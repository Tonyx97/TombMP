#include "objects.h"
#include "lara.h"
#include "effect2.h"
#include "lot.h"
#include "sphere.h"
#include "control.h"
#include "traps.h"
#include "pickup.h"
#include "setup.h"

#include <specific/standard.h>
#include <specific/fn_stubs.h>

#define TONYBOSS_TURN 			(ONE_DEGREE * 2)
#define TONYBOSS_DIE_ANIM 		6
#define TONYBOSS_HITS			100
#define MAX_TONY_TRIGGER_RANGE	0x4000

enum
{
	ROCKZAPPL,		// To ceiling from left hand
	ROCKZAPPR,		// To ceiling from right hand
	ZAPP,			// From right hand.
	DROPPER,		// From ceiling.
	ROCKZAPPDEBRIS,	// Small bits from ceiling explosions.
	ZAPPDEBRIS,		// Small bits from hand flame explosions.
	DROPPERDEBRIS,	// Small bits from droppers explosions.
};

enum tonyboss_anims
{
	TONYBOSS_WAIT,
	TONYBOSS_RISE,
	TONYBOSS_FLOAT,
	TONYBOSS_ZAPP,
	TONYBOSS_ROCKZAPP,
	TONYBOSS_BIGBOOM,
	TONYBOSS_DEATH
};

struct SHIELD_POINTS
{
	short x;
	short y;
	short z;
	uint8_t rsub;
	uint8_t gsub;
	uint8_t bsub;
	uint8_t pad[3];
	long rgb;
};

struct EXPLOSION_VERTS
{
	short x;
	short z;
	long rgb;
};

struct EXPLOSION_RING
{
	short on;
	short life;		// 0 - 32.
	short speed;
	short radius;	// Width is 1/4 of radius.
	short xrot;
	short zrot;
	long x;
	long y;
	long z;
	EXPLOSION_VERTS	verts[16];
};

SHIELD_POINTS TonyBossShield[40];
EXPLOSION_RING tb_ExpRings[7];

long tb_radii[5]	 = { 200, 400, 500, 500, 475 };
long tb_heights[5]	 = { -1536,-1280, -832, -384, 0 };
long tb_dradii[5]	 = { 100 << 4, 350 << 4, 400 << 4, 350 << 4, 100 << 4 };
long tb_dheights1[5] = { -1536 - (768 << 3), -1152 - (384 << 3), -768, -384 + (384 << 3), 0 + (768 << 3) };
long tb_dheights2[5] = { -1536, -1152, -768, -384, 0 };
long tb_death_radii[5];
long tb_death_heights[5];

void TriggerFireBallFlame(int16_t fx_number, long type, long xv, long yv, long zv);
void TriggerFireBall(ITEM_INFO* item, long type, PHD_VECTOR* pos1, int16_t room_number, int16_t angle, long zdspeed);
void TriggerTonyFlame(int16_t item_number, long hand);

void TonyBossDie(int16_t item_number);
void ExplodeTonyBoss(ITEM_INFO* item);

void TonyBossControl(int16_t item_number)
{
	if (!CreatureActive(item_number))
		return;

	auto item = &items[item_number];
	auto tonyboss = (CREATURE_INFO*)item->data;

	int16_t head = 0,
		   torso_y = 0,
		   torso_x = 0,
		   angle = 0,
		   tilt = 0;

	if (item->hit_points <= 0)
	{
		if (item->current_anim_state != TONYBOSS_DEATH)
		{
			item->anim_number = objects[item->object_number].anim_index + TONYBOSS_DIE_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = TONYBOSS_DEATH;
		}

		if (item->frame_number - anims[item->anim_number].frame_base > 110)
		{
			item->frame_number = anims[item->anim_number].frame_base + 110;
			item->mesh_bits = 0;

			if (bossdata.explode_count == 0)
			{
				bossdata.ring_count = 0;

				for (int i = 0; i < 6; ++i)
				{
					tb_ExpRings[i].on = 0;
					tb_ExpRings[i].life = 32;
					tb_ExpRings[i].radius = 512;
					tb_ExpRings[i].speed = 128 + (i << 5);
					tb_ExpRings[i].xrot = ((GetRandomControl() & 511) - 256) & 4095;
					tb_ExpRings[i].zrot = ((GetRandomControl() & 511) - 256) & 4095;
				}

				if (bossdata.dropped_icon == 0)
				{
					BossDropIcon(item_number);
					bossdata.dropped_icon = 1;
				}
			}

			if (bossdata.explode_count < 256)
				++bossdata.explode_count;

			if (bossdata.explode_count > 64 && bossdata.ring_count == 6/* && tb_ExpRings[5].life == 0*/)
			{
				TonyBossDie(item_number);
				bossdata.dead = 1;
			}
			else ExplodeTonyBoss(item);

			return;
		}
	}
	else
	{
		if (item->item_flags[3] != 2)
			item->hit_points = TONYBOSS_HITS;

		AI_INFO info;

		CreatureAIInfo(item, &info);

		if (!item->item_flags[3])
		{
			int dx = item->pos.x_pos - lara_item->pos.x_pos,
				dz = item->pos.z_pos - lara_item->pos.z_pos;

			if ((SQUARE(dx) + SQUARE(dz)) < SQUARE(5120))
				item->item_flags[3] = 1;

			angle = 0;
		}
		else
		{
			tonyboss->target.x = lara_item->pos.x_pos;
			tonyboss->target.z = lara_item->pos.z_pos;
			angle = CreatureTurn(item, tonyboss->maximum_turn);
		}

		if (info.ahead)
			head = info.angle;

		switch (item->current_anim_state)
		{
		case TONYBOSS_WAIT:
		{
			tonyboss->maximum_turn = 0;

			if (item->goal_anim_state != TONYBOSS_RISE && item->item_flags[3])
				item->goal_anim_state = TONYBOSS_RISE;

			break;
		}
		case TONYBOSS_RISE:
			tonyboss->maximum_turn = (item->frame_number - anims[item->anim_number].frame_base > 16 ? TONYBOSS_TURN : 0);
			break;
		case TONYBOSS_FLOAT:
		{
			tonyboss->maximum_turn = TONYBOSS_TURN;

			torso_y = info.angle;
			torso_x = info.x_angle;

			if (!bossdata.explode_count)
			{
				if (item->goal_anim_state != TONYBOSS_BIGBOOM && item->item_flags[3] != 2)
				{
					item->goal_anim_state = TONYBOSS_BIGBOOM;
					tonyboss->maximum_turn = 0;
				}

				if (item->goal_anim_state != TONYBOSS_ROCKZAPP && item->item_flags[3] == 2)
				{
					if (!(wibble & 255) && item->item_flags[0] == 0)
					{
						item->goal_anim_state = TONYBOSS_ROCKZAPP;
						item->item_flags[0] = 1;
					}
				}

				if (item->goal_anim_state != TONYBOSS_ZAPP && item->goal_anim_state != TONYBOSS_ROCKZAPP && item->item_flags[3] == 2)
				{
					if (!(wibble & 255) && item->item_flags[0] == 1)
					{
						item->goal_anim_state = TONYBOSS_ZAPP;
						item->item_flags[0] = 0;
					}
				}
			}

			break;
		}
		case TONYBOSS_ROCKZAPP:
		{
			tonyboss->maximum_turn = 0;

			torso_y = info.angle;
			torso_x = info.x_angle;

			if (item->frame_number - anims[item->anim_number].frame_base == 40)
			{
				TriggerFireBall(item, ROCKZAPPL, nullptr, item->room_number, 0, 0);
				TriggerFireBall(item, ROCKZAPPR, nullptr, item->room_number, 0, 0);
			}

			break;
		}
		case TONYBOSS_ZAPP:
		{
			tonyboss->maximum_turn = TONYBOSS_TURN >> 1;

			torso_y = info.angle;
			torso_x = info.x_angle;

			if (item->frame_number - anims[item->anim_number].frame_base == 28)
				TriggerFireBall(item, ZAPP, nullptr, item->room_number, item->pos.y_rot, 0);

			break;
		}
		case TONYBOSS_BIGBOOM:
		{
			tonyboss->maximum_turn = 0;

			if (item->frame_number - anims[item->anim_number].frame_base == 56)
			{
				item->item_flags[3] = 2;
				bossdata.explode_count = 1;
			}
		}
		}
	}

	if (item->current_anim_state == TONYBOSS_ROCKZAPP || item->current_anim_state == TONYBOSS_ZAPP || item->current_anim_state == TONYBOSS_BIGBOOM)
	{
		int bright = item->frame_number - anims[item->anim_number].frame_base;

		if (bright > 16)
			if ((bright = anims[item->anim_number].frame_end - item->frame_number) > 16)
				bright = 16;

		int rnd = GetRandomControl(),
			r = ((31 - ((rnd >> 4) & 3)) * bright) >> 4,
			g = ((24 - ((rnd >> 6) & 3)) * bright) >> 4,
			b = ((rnd & 7) * bright) >> 4;

		{
			PHD_VECTOR pos {};

			GetJointAbsPosition(&items[item_number], &pos, 10);

			TriggerDynamicLight(pos.x, pos.y, pos.z, 12, r, g, b);
			TriggerTonyFlame(item_number, SPN_TONYHANDRFLAME);
		}

		if (item->current_anim_state == TONYBOSS_ROCKZAPP || item->current_anim_state == TONYBOSS_BIGBOOM)
		{
			PHD_VECTOR pos {};

			GetJointAbsPosition(&items[item_number], &pos, 13);

			TriggerDynamicLight(pos.x, pos.y, pos.z, 12, r, g, b);
			TriggerTonyFlame(item_number, SPN_TONYHANDLFLAME);
		}
	}

	if (bossdata.explode_count && item->hit_points > 0)
	{
		ExplodeTonyBoss(item);

		if (++bossdata.explode_count == 32)
			FlipMap();

		if (bossdata.explode_count > 64)
			bossdata.explode_count = bossdata.ring_count = 0;
	}

	CreatureJoint(item, 0, torso_y >> 1);
	CreatureJoint(item, 1, torso_x);
	CreatureJoint(item, 2, torso_y >> 1);

	CreatureAnimation(item_number, angle, 0);
}

void S_DrawTonyBoss(ITEM_INFO* item)
{
	DrawAnimatingItem(item);
}

void TonyBossDie(int16_t item_number)
{
	auto item = &items[item_number];

	item->collidable = 0;
	item->hit_points = DONT_TARGET;

	KillItem(item_number);
	DisableBaddieAI(item_number);

	item->flags |= ONESHOT;
}

void InitialiseTonyBoss(int16_t item_number)
{
	items[item_number].item_flags[3] =
		bossdata.explode_count =
		bossdata.ring_count =
		bossdata.dropped_icon =
		bossdata.dead = 0;

	auto shptr = &TonyBossShield[0];

	for (int i = 0; i < 5; ++i)
	{
		int y = tb_heights[i],
			angle = 0;

		for (int j = 0; j < 8; ++j, ++shptr)
		{
			shptr->x = (m_sin(angle << 1) * tb_radii[i]) >> 11;
			shptr->y = y;
			shptr->z = (m_cos(angle << 1) * tb_radii[i]) >> 11;
			shptr->rgb = 0;
			angle += 512;
		}
	}
}

void ExplodeTonyBoss(ITEM_INFO* item)
{
	if (item->hit_points <= 0 &&
		(bossdata.explode_count == 1 ||
		bossdata.explode_count == 15 ||
		bossdata.explode_count == 25 ||
		bossdata.explode_count == 35 ||
		bossdata.explode_count == 45 ||
		bossdata.explode_count == 55))
	{
		int x = item->pos.x_pos + ((GetRandomDraw() & 1023) - 512),
			y = item->pos.y_pos - (GetRandomDraw() & 1023) - 256,
			z = item->pos.z_pos + ((GetRandomDraw() & 1023) - 512);

		tb_ExpRings[bossdata.ring_count].x = x;
		tb_ExpRings[bossdata.ring_count].y = y;
		tb_ExpRings[bossdata.ring_count].z = z;
		tb_ExpRings[bossdata.ring_count++].on = 1;

		TriggerExplosionSparks(x, y, z, 3, -2, 0, 0);

		for (int i = 0; i < 2; ++i)
			TriggerExplosionSparks(x, y, z, 3, -1, 0, 0);

		g_audio->play_sound(76, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
	}

	for (int i = 0; i < 5; ++i)
		if (bossdata.explode_count < 128)
		{
			tb_death_radii[i] = (tb_dradii[i] >> 4) + (((tb_dradii[i]) * bossdata.explode_count) >> 7);
			tb_death_heights[i] = tb_dheights2[i] + (((tb_dheights1[i] - tb_dheights2[i]) * bossdata.explode_count) >> 7);
		}

	if (bossdata.explode_count <= 64)
	{
		auto shptr = &TonyBossShield[0];

		for (int i = 0; i < 5; ++i)
		{
			int y = tb_death_heights[i],
				angle = (wibble << 3) & 511;

			for (int j = 0; j < 8; ++j, ++shptr)
			{
				shptr->x = (m_sin(angle << 1) * tb_death_radii[i]) >> 11;
				shptr->y = y;
				shptr->z = (m_cos(angle << 1) * tb_death_radii[i]) >> 11;

				if (i != 0 && i != 4 && bossdata.explode_count < 64)
				{
					int r = (GetRandomDraw() & 31) + 224,
						g = (r >> 2) + (GetRandomDraw() & 63),
						b = (GetRandomDraw() & 63);

					if (item->hit_points > 0)
					{
						r = (r * (128 - bossdata.explode_count)) >> 7;
						g = (g * (128 - bossdata.explode_count)) >> 7;
						b = (b * (128 - bossdata.explode_count)) >> 7;
					}
					else
					{
						r = (r * (64 - bossdata.explode_count)) >> 6;
						g = (g * (64 - bossdata.explode_count)) >> 6;
						b = (b * (64 - bossdata.explode_count)) >> 6;
					}

					shptr->rgb = r | (g << 8) | (b << 16);
				}
				else shptr->rgb = 0;

				angle += 512;
				angle &= 4095;
			}
		}
	}
}

void TriggerTonyFlame(int16_t item_number, long hand)
{
	int dx = lara_item->pos.x_pos - items[item_number].pos.x_pos,
		dz = lara_item->pos.z_pos - items[item_number].pos.z_pos;

	if (dx < -MAX_TONY_TRIGGER_RANGE || dx > MAX_TONY_TRIGGER_RANGE || dz < -MAX_TONY_TRIGGER_RANGE || dz > MAX_TONY_TRIGGER_RANGE)
		return;

	auto sptr = &spark[GetFreeSpark()];

	int size = (GetRandomControl() & 31) + 64;
	
	sptr->On = 1;
	sptr->sR = 255;
	sptr->sG = 48 + (GetRandomControl() & 31);
	sptr->sB = 48;
	sptr->dR = 192 + (GetRandomControl() & 63);
	sptr->dG = 128 + (GetRandomControl() & 63);
	sptr->dB = 32;
	sptr->ColFadeSpeed = 12 + (GetRandomControl() & 3);
	sptr->FadeToBlack = 8;
	sptr->sLife = sptr->Life = (GetRandomControl() & 7) + 24;
	sptr->TransType = COLADD;
	sptr->extras = 0;
	sptr->Dynamic = -1;
	sptr->x = ((GetRandomControl() & 15) - 8);
	sptr->y = 0;
	sptr->z = ((GetRandomControl() & 15) - 8);
	sptr->Xvel = ((GetRandomControl() & 255) - 128);
	sptr->Yvel = -(GetRandomControl() & 15) - 16;
	sptr->Zvel = ((GetRandomControl() & 255) - 128);
	sptr->Friction = 5;

	if (GetRandomControl() & 1)
	{
		sptr->Flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF | SP_ITEM | SP_NODEATTATCH;
		sptr->RotAng = GetRandomControl() & 4095;
		sptr->RotAdd = ((GetRandomControl() & 1) ? -(GetRandomControl() & 15) - 16 : (GetRandomControl() & 15) + 16);
	}
	else sptr->Flags = SP_SCALE | SP_DEF | SP_EXPDEF | SP_ITEM | SP_NODEATTATCH;

	sptr->Gravity = -(GetRandomControl() & 31) - 16;
	sptr->MaxYvel = -(GetRandomControl() & 7) - 16;
	sptr->FxObj = item_number;
	sptr->NodeNumber = hand;
	sptr->Def = objects[EXPLOSION1].mesh_index;
	sptr->Scalar = 1;
	sptr->Width = sptr->sWidth = size;
	sptr->Height = sptr->sHeight = size;
	sptr->dWidth = size >> 2;
	sptr->dHeight = size >> 2;
}

void TriggerFireBallFlame(int16_t fx_number, long type, long xv, long yv, long zv)
{
	int dx = lara_item->pos.x_pos - effects[fx_number].pos.x_pos,
		dz = lara_item->pos.z_pos - effects[fx_number].pos.z_pos;

	if (dx < -MAX_TONY_TRIGGER_RANGE || dx > MAX_TONY_TRIGGER_RANGE || dz < -MAX_TONY_TRIGGER_RANGE || dz > MAX_TONY_TRIGGER_RANGE)
		return;

	auto sptr = &spark[GetFreeSpark()];

	int size = (GetRandomControl() & 31) + 64;
	
	sptr->On = 1;
	sptr->sR = 255;
	sptr->sG = 48 + (GetRandomControl() & 31);
	sptr->sB = 48;
	sptr->dR = 192 + (GetRandomControl() & 63);
	sptr->dG = 128 + (GetRandomControl() & 63);
	sptr->dB = 32;
	sptr->ColFadeSpeed = 12 + (GetRandomControl() & 3);
	sptr->FadeToBlack = 8;
	sptr->sLife = sptr->Life = (GetRandomControl() & 7) + 24;
	sptr->TransType = COLADD;
	sptr->extras = 0;
	sptr->Dynamic = -1;
	sptr->x = ((GetRandomControl() & 15) - 8);
	sptr->y = 0;
	sptr->z = ((GetRandomControl() & 15) - 8);
	sptr->Xvel = xv + ((GetRandomControl() & 255) - 128);
	sptr->Yvel = yv;
	sptr->Zvel = zv + ((GetRandomControl() & 255) - 128);
	sptr->Friction = 5;

	if (GetRandomControl() & 1)
	{
		sptr->Flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF | SP_FX;
		sptr->RotAng = GetRandomControl() & 4095;
		sptr->RotAdd = ((GetRandomControl() & 1) ? -(GetRandomControl() & 15) - 16 : (GetRandomControl() & 15) + 16);
	}
	else sptr->Flags = SP_SCALE | SP_DEF | SP_EXPDEF | SP_FX;

	sptr->FxObj = fx_number;
	sptr->Def = objects[EXPLOSION1].mesh_index;
	sptr->Scalar = 1;
	sptr->Width = sptr->sWidth = size;
	sptr->Height = sptr->sHeight = size;
	sptr->dWidth = size >> 2;
	sptr->dHeight = size >> 2;

	if (type == ROCKZAPPL || type == ROCKZAPPR)
	{
		sptr->Gravity = (GetRandomControl() & 31) + 16;
		sptr->MaxYvel = (GetRandomControl() & 15) + 48;
		sptr->Yvel = -sptr->Yvel << 4;
		sptr->Scalar = 2;
	}
	else if (type == ROCKZAPPDEBRIS || type == ZAPPDEBRIS || type == DROPPERDEBRIS)
		sptr->Gravity = sptr->MaxYvel = 0;
	else if (type == DROPPER)
	{
		sptr->Gravity = -(GetRandomControl() & 31) - 16;
		sptr->MaxYvel = -(GetRandomControl() & 31) - 64;
		sptr->Yvel = sptr->Yvel << 4;
		sptr->Scalar = 2;
	}
	else if (type == ZAPP)
	{
		sptr->Gravity = sptr->MaxYvel = 0;
		sptr->Scalar = 2;
	}
}

void TriggerFireBall(ITEM_INFO* item, long type, PHD_VECTOR* pos1, int16_t room_number, int16_t angle, long zdspeed)
{
	PHD_VECTOR pos;

	int speed, fallspeed;

	if (type == ROCKZAPPL)
	{
		pos.x = pos.y = pos.z = speed = 0;
		GetJointAbsPosition(item, &pos, 10);
		angle = item->pos.y_rot;
		fallspeed = -16;
	}
	else if (type == ROCKZAPPR)
	{
		pos.x = pos.y = pos.z = speed = 0;
		GetJointAbsPosition(item, &pos, 13);
		angle = item->pos.y_rot;
		fallspeed = -16;
	}
	else if (type == ZAPP)
	{
		pos.x = pos.y = pos.z = 0;

		GetJointAbsPosition(item, &pos, 13);

		speed = 160;
		fallspeed = -(GetRandomControl() & 7) - 32;
	}
	else if (type == ROCKZAPPDEBRIS)
	{
		pos = *pos1;
		speed = zdspeed + (GetRandomControl() & 3);
		angle = GetRandomControl() << 1;
		fallspeed = (GetRandomControl() & 3) - 2;
	}
	else if (type == ZAPPDEBRIS)
	{
		pos = *pos1;
		speed = (GetRandomControl() & 7) + 48;
		angle += (GetRandomControl() & 0x1fff) - 0x9000;
		fallspeed = -(GetRandomControl() & 15) - 16;
	}
	else if (type == DROPPER)
	{
		pos = *pos1;
		speed = 0;
		fallspeed = (GetRandomControl() & 3) + 4;
	}
	else
	{
		pos.x = pos1->x;
		pos.y = pos1->y;
		pos.z = pos1->z;
		speed = (GetRandomControl() & 31) + 32;
		angle = GetRandomControl() << 1;
		fallspeed = -(GetRandomControl() & 31) - 32;
	}

	if (auto fx_number = CreateEffect(room_number); fx_number != NO_ITEM)
	{
		auto fx = &effects[fx_number];

		fx->pos.x_pos = pos.x;
		fx->pos.y_pos = pos.y;
		fx->pos.z_pos = pos.z;
		fx->pos.y_rot = angle;
		fx->object_number = TONYFIREBALL;
		fx->speed = speed;
		fx->fallspeed = fallspeed;
		fx->flag1 = type;
		fx->flag2 = (GetRandomControl() & 3) + 1;

		if (type == ZAPPDEBRIS) fx->flag2 <<= 1;
		else if (type == ZAPP)  fx->flag2 = 0;
	}
}

void ControlTonyFireBall(int16_t fx_number)
{
	auto fx = &effects[fx_number];

	int old_x = fx->pos.x_pos,
		old_y = fx->pos.y_pos,
		old_z = fx->pos.z_pos;

	if (fx->flag1 == ROCKZAPPL || fx->flag1 == ROCKZAPPR)
	{
		fx->fallspeed += (fx->fallspeed >> 3) + 1;

		if (fx->fallspeed < -4096)
			fx->fallspeed = -4096;

		fx->pos.y_pos += fx->fallspeed;

		if (wibble & 4)
			TriggerFireBallFlame(fx_number, fx->flag1, 0, 0, 0);
	}
	else if (fx->flag1 == DROPPER)
	{
		fx->fallspeed += 2;
		fx->pos.y_pos += fx->fallspeed;

		if (wibble & 4)
			TriggerFireBallFlame(fx_number, fx->flag1, 0, 0, 0);
	}
	else
	{
		if (fx->flag1 != ZAPP && fx->speed > 48)
			--fx->speed;

		fx->fallspeed += fx->flag2;

		if (fx->fallspeed > 512)
			fx->fallspeed = 512;

		fx->pos.y_pos += fx->fallspeed >> 1;
		fx->pos.z_pos += (fx->speed * phd_cos(fx->pos.y_rot) >> W2V_SHIFT);
		fx->pos.x_pos += (fx->speed * phd_sin(fx->pos.y_rot) >> W2V_SHIFT);

		if (wibble & 4)
			TriggerFireBallFlame(fx_number, fx->flag1, (old_x - fx->pos.x_pos) << 3, (old_y - fx->pos.y_pos) << 3, (old_z - fx->pos.z_pos) << 3);
	}

	auto room_number = fx->room_number;
	auto floor = GetFloor(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, &room_number);

	if (fx->pos.y_pos >= GetHeight(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos) ||
		fx->pos.y_pos < GetCeiling(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos))
	{
		if (fx->flag1 == ROCKZAPPL || fx->flag1 == ROCKZAPPR || fx->flag1 == ZAPP || fx->flag1 == DROPPER)
		{
			TriggerExplosionSparks(old_x, old_y, old_z, 3, -2, 0, fx->room_number);

			if (fx->flag1 == ROCKZAPPL || fx->flag1 == ROCKZAPPR)
				for (int x = 0; x < 2; ++x)
					TriggerExplosionSparks(old_x, old_y, old_z, 3, -1, 0, fx->room_number);

			int r = (fx->flag1 == ZAPP ? 7 : 3),
				g;

			if (fx->flag1 == ZAPP)
				g = ZAPPDEBRIS;
			else if (fx->flag1 == DROPPER)
				g = DROPPERDEBRIS;
			else g = ROCKZAPPDEBRIS;

			PHD_VECTOR pos { old_x, old_y, old_z };

			for (int x = 0; x < r; ++x)
				TriggerFireBall(nullptr, g, &pos, fx->room_number, fx->pos.y_rot, 32 + (x << 2));

			if (fx->flag1 == ROCKZAPPL || fx->flag1 == ROCKZAPPR)
			{
				room_number = lara_item->room_number;
				floor = GetFloor(lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos, &room_number);
				pos.y = GetCeiling(floor, lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos) + 256;
				pos.x = lara_item->pos.x_pos + (GetRandomControl() & 1023) - 512;
				pos.z = lara_item->pos.z_pos + (GetRandomControl() & 1023) - 512;

				TriggerExplosionSparks(pos.x, pos.y, pos.z, 3, -2, 0, room_number);
				TriggerFireBall(nullptr, DROPPER, &pos, room_number, 0, 0);
			}
		}

		return KillEffect(fx_number);
	}

	if (room[room_number].flags & UNDERWATER)
		return KillEffect(fx_number);

	if (!lara.burn)
	{
		if (ItemNearLara(lara_item, &fx->pos, 200))
		{
			lara_item->hit_status = 1;

			KillEffect(fx_number);

			lara_item->hit_points -= 200;

			LaraBurn();

			return;
		}
	}

	if (room_number != fx->room_number)
		EffectNewRoom(fx_number, lara_item->room_number);

	static constexpr uint8_t radtab[7] = { 16, 0, 14, 9, 7, 7, 7 };
	
	if (radtab[fx->flag1])
	{
		int rnd = GetRandomControl(),
			r = 31 - ((rnd >> 4) & 3),
			g = 24 - ((rnd >> 6) & 3),
			b = rnd & 7;

		TriggerDynamicLight(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, radtab[fx->flag1], r, g, b);
	}
}

void TonyBossFlipMap()
{
	auto r = room;

	for (int i = 0; i < number_rooms; ++i, ++r)
	{
		if (r->flipped_room < 0)
			continue;

		auto flipped = &room[r->flipped_room];

		ROOM_INFO temp;

		memcpy(&temp, r, sizeof(ROOM_INFO));
		memcpy(r, flipped, sizeof(ROOM_INFO));
		memcpy(flipped, &temp, sizeof(ROOM_INFO));

		r->flipped_room = flipped->flipped_room;
		r->item_number = flipped->item_number;
		r->fx_number = flipped->fx_number;

		flipped->flipped_room = -1;
	}
}