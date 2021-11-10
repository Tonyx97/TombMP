#include "objects.h"
#include "lara.h"
#include "effect2.h"
#include "lot.h"
#include "sphere.h"
#include "laraelec.h"
#include "pickup.h"
#include "control.h"
#include "setup.h"
#include "triboss.h"

#include <specific/standard.h>
#include <specific/fn_stubs.h>

#define SIGN_BIT			0x80000000
#define	MAX_ELEC_POS		256
#define	MAX_ELEC_POS2		MAX_ELEC_POS + (MAX_ELEC_POS >> 1)
#define TRIBEBOSS_TURN 		(ONE_DEGREE * 3)
#define TRIBEBOSS_DIE_ANIM 	3
#define TRIBEBOSS_HAND_R 	14
#define MAX_HEAD_ATTACKS	4

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
	short life;
	short speed;
	short radius;
	short xrot;
	short zrot;
	long x;
	long y;
	long z;
	EXPLOSION_VERTS	verts[16];
};

enum
{
	ATTACK_HEAD,
	ATTACK_HAND1,
	ATTACK_HAND2
};

enum tribeboss_anims
{
	TRIBEBOSS_WAIT,
	TRIBEBOSS_ATTACK_HEAD,
	TRIBEBOSS_ATTACK_HAND,
	TRIBEBOSS_DEATH
};

long death_radii_triboss[5];
long death_heights_triboss[5];
long radii_triboss[5] = { 200,400,500,500,475 };
long heights_triboss[5] = { -1536,-1280,-832,-384,0 };
long dradii_triboss[5] = { 100 << 4,350 << 4,400 << 4,350 << 4,100 << 4 };
long dheights1_triboss[5] = { -1536 - (768 << 3),-1152 - (384 << 3),-768,-384 + (384 << 3),0 + (768 << 3) };
long dheights2_triboss[5] = { -1536,-1152,-768,-384,0 };

long lizman_summon_coords[2][4] =
{
	{ 0xf200, -0x1500, 0xf600, 0xe000 },
	{ 0xf200, -0x1500, 0xce00, 0xa000 }
};

BITE_INFO tribeboss_hit[6] =
{
	{ 120, 68, 136, 8 },
	{ 128, -64, 136, 8 },
	{ 8, -120, 136, 8 },
	{ -128, -64, 136, 8 },
	{ -124, 64, 136, 8 },
	{ 8, 32, 400, 8 }
};

inline PHD_VECTOR TrigDynamics[3];

SHIELD_POINTS TribeBossShield[40];
EXPLOSION_RING ExpRingsTriboss[7];

uint8_t turned = 0,
	  shield_active;

void TriggerElectricSparks(GAME_VECTOR* pos, long shield);
void TribeBossDie(int16_t item_number);
void ExplodeTribeBoss(ITEM_INFO* item);
void RotateHeadXAngle(ITEM_INFO* item);
void TriggerLizardMan();
void TriggerSummonSmoke(long x, long y, long z);
int16_t FindLizardManItemNumber(int16_t room_number);

void TribeBossControl(int16_t item_number)
{
	for (int i = 0; i < 3; ++i)
	{
		if (TrigDynamics[i].x != 0)
		{
			if (i == 0)
				TriggerDynamicLight(TrigDynamics[0].x, TrigDynamics[0].y, TrigDynamics[0].z, (GetRandomControl() & 3) + 8, 0, (GetRandomControl() & 7) + 8, (GetRandomControl() & 7) + 16);
			else if (i == 1)
			{
				if (bossdata.attack_type == ATTACK_HEAD)
					TriggerDynamicLight(TrigDynamics[1].x, TrigDynamics[1].y, TrigDynamics[1].z, (GetRandomControl() & 7) + 8, 0, (GetRandomControl() & 7) + 8, (GetRandomControl() & 7) + 16);
				else TriggerDynamicLight(TrigDynamics[1].x, TrigDynamics[1].y, TrigDynamics[1].z, (GetRandomControl() & 7) + 8, 0, (GetRandomControl() & 7) + 16, (GetRandomControl() & 7) + 8);
			}
			else
			{
				if (bossdata.attack_count)
				{
					int16_t dynsize = (128 - bossdata.attack_count) >> 1;

					if (dynsize > 31)
						dynsize = 31;

					if (dynsize > 0)
					{
						if (bossdata.attack_type == ATTACK_HEAD)
							TriggerDynamicLight(TrigDynamics[2].x, TrigDynamics[2].y, TrigDynamics[2].z, dynsize, dynsize >> 2, (GetRandomControl() & 7) + 16, (GetRandomControl() & 7) + 24);
						else TriggerDynamicLight(TrigDynamics[2].x, TrigDynamics[2].y, TrigDynamics[2].z, dynsize, dynsize >> 2, (GetRandomControl() & 7) + 24, (GetRandomControl() & 7) + 16);
					}
				}
			}
		}
	}

	if (!CreatureActive(item_number))
		return;

	auto item = &items[item_number];
	auto tribeboss = (CREATURE_INFO*)item->data;

	int16_t head = 0,
		   angle = 0,
		   tilt = 0;

	uint16_t ty, tyrot;

	if (item->hit_points <= 0)
	{
		if (item->current_anim_state != TRIBEBOSS_DEATH)
		{
			item->anim_number = objects[item->object_number].anim_index + TRIBEBOSS_DIE_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = TRIBEBOSS_DEATH;

			bossdata.death_count = 1;
		}

		if (item->frame_number - anims[item->anim_number].frame_base > 119)
		{
			item->frame_number = anims[item->anim_number].frame_base + 120;
			item->mesh_bits = 0;

			bossdata.death_count = -1;

			if (bossdata.explode_count == 0)
			{
				bossdata.ring_count = 0;

				g_audio->play_sound(106, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

				for (int i = 0; i < 6; ++i)
				{
					ExpRingsTriboss[i].on = 0;
					ExpRingsTriboss[i].life = 32;
					ExpRingsTriboss[i].radius = 512;
					ExpRingsTriboss[i].speed = 128 + (i << 5);
					ExpRingsTriboss[i].xrot = ((GetRandomControl() & 511) - 256) & 4095;
					ExpRingsTriboss[i].zrot = ((GetRandomControl() & 511) - 256) & 4095;
				}

				if (bossdata.dropped_icon == 0)
				{
					BossDropIcon(item_number);
					bossdata.dropped_icon = 1;
				}
			}

			if (bossdata.explode_count < 256)
				bossdata.explode_count++;

			if (bossdata.explode_count > 64 && bossdata.ring_count == 6/* && ExpRingsTriboss[5].life == 0*/)
			{
				TribeBossDie(item_number);
				bossdata.dead = 1;
			}
			else ExplodeTribeBoss(item);

			return;
		}

		item->pos.z_rot = (GetRandomControl() % bossdata.death_count) - (bossdata.death_count >> 1);

		if (bossdata.death_count < 2048)
			bossdata.death_count += 32;
	}
	else
	{
		AI_INFO info;

		CreatureAIInfo(item, &info);

		if (item->hit_status)
			g_audio->play_sound(361, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

		tyrot = item->pos.y_rot;

		if (!bossdata.attack_flag)
		{
			int dx = item->pos.x_pos - lara_item->pos.x_pos,
				dz = item->pos.z_pos - lara_item->pos.z_pos;

			if (SQUARE(dx) + SQUARE(dz) < 2048 * 2048)
				bossdata.attack_flag = 1;
		}

		tribeboss->target.x = lara_item->pos.x_pos;
		tribeboss->target.z = lara_item->pos.z_pos;

		if (lizard_man_active && item->current_anim_state == TRIBEBOSS_WAIT)
		{
			ty = (uint16_t)item->pos.y_rot;

			if (abs(0xc000 - ty) > ONE_DEGREE)
				item->pos.y_rot += (0xc000 - ty) >> 3;
			else item->pos.y_rot = PHD_ANGLE(0xc000);
		}
		else angle = CreatureTurn(item, tribeboss->maximum_turn);

		RotateHeadXAngle(item);

		if (info.ahead)
			head = info.angle;

		switch (item->current_anim_state)
		{
		case TRIBEBOSS_WAIT:
		{
			bossdata.attack_count = 0;

			if (item->goal_anim_state != TRIBEBOSS_ATTACK_HEAD &&
				item->goal_anim_state != TRIBEBOSS_ATTACK_HAND)
				TribeBossShieldOn = 1;

			if (lizard_man_active)
				CreatureJoint(item, 1, head);	// Rotate head to watch Lara.
			else CreatureJoint(item, 1, 0);		// Rotate head to point forward.

			if (bossdata.attack_flag && !lizard_man_active)
				tribeboss->maximum_turn = TRIBEBOSS_TURN;
			else tribeboss->maximum_turn = 0;

			if (item->goal_anim_state != TRIBEBOSS_ATTACK_HEAD && info.angle > -128 && info.angle < 128
				&& lara_item->hit_points > 0 && bossdata.attack_head_count < MAX_HEAD_ATTACKS && !lizard_man_active
				&& !shield_active)
			{
				PHD_VECTOR	pos;

				pos.x = pos.y = pos.z = 0;
				GetJointAbsPosition(lara_item, &pos, 0);	// Get position of Lara's hips.
				bossdata.BeamTarget.x = pos.x;
				bossdata.BeamTarget.y = pos.y;
				bossdata.BeamTarget.z = pos.z;
				item->goal_anim_state = TRIBEBOSS_ATTACK_HEAD;
				tribeboss->maximum_turn = 0;
				TribeBossShieldOn = 0;
				bossdata.attack_head_count++;
				break;
			}
			else if (item->goal_anim_state != TRIBEBOSS_ATTACK_HAND && bossdata.attack_head_count >= MAX_HEAD_ATTACKS
				&& lara_item->hit_points > 0)
			{
				tribeboss->maximum_turn = 0;

				if (bossdata.attack_type == ATTACK_HEAD)
				{
					long dxz1, dxz2;

					dxz1 = SQUARE(lara_item->pos.x_pos - lizman_summon_coords[0][0]);
					dxz1 += SQUARE(lara_item->pos.z_pos - lizman_summon_coords[0][2]);
					dxz2 = SQUARE(lara_item->pos.x_pos - lizman_summon_coords[1][0]);
					dxz2 += SQUARE(lara_item->pos.z_pos - lizman_summon_coords[1][2]);

					// Get furthest lizard man summon point from Lara.

					if (dxz1 > dxz2)
						bossdata.attack_type = ATTACK_HAND1;
					else
						bossdata.attack_type = ATTACK_HAND2;
				}

				ty = (uint16_t)item->pos.y_rot;
				if (abs(lizman_summon_coords[bossdata.attack_type - 1][3] - ty) < ONE_DEGREE)
				{
					item->pos.y_rot = lizman_summon_coords[bossdata.attack_type - 1][3];
					if (!shield_active)
					{
						item->goal_anim_state = TRIBEBOSS_ATTACK_HAND;
						bossdata.BeamTarget.x = lizman_summon_coords[bossdata.attack_type - 1][0];
						bossdata.BeamTarget.y = lizman_summon_coords[bossdata.attack_type - 1][1];
						bossdata.BeamTarget.z = lizman_summon_coords[bossdata.attack_type - 1][2];
						tribeboss->maximum_turn = 0;
						bossdata.attack_head_count = 0;
						TribeBossShieldOn = 0;
						break;
					}
				}
				else
					item->pos.y_rot += (lizman_summon_coords[bossdata.attack_type - 1][3] - ty) >> 4;
			}
			break;
		}
		case TRIBEBOSS_ATTACK_HEAD:
		{
			tribeboss->maximum_turn = 0;

			bossdata.attack_count += 3;
			bossdata.attack_type = ATTACK_HEAD;

			CreatureJoint(item, 1, 0);

			break;
		}
		case TRIBEBOSS_ATTACK_HAND:
		{
			tribeboss->maximum_turn = 0;

			bossdata.attack_count += (bossdata.attack_count < 64 ? 2 : 3);

			CreatureJoint(item, 1, 0);
		}
		}
	}

	if (bossdata.attack_count && bossdata.attack_type == ATTACK_HEAD && bossdata.attack_count < 64)
		tribeboss_hit[5].z = 136 + (bossdata.attack_count << 2);

	tribeboss->joint_rotation[0] += ONE_DEGREE * 7;

	CreatureAnimation(item_number, angle, 0);

	if (tyrot != item->pos.y_rot && turned == 0)
	{
		turned = 1;
		g_audio->play_sound(362, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
	}
	else if (tyrot == item->pos.y_rot)
		turned = 0;
}

void S_DrawTribeBoss(ITEM_INFO* item)
{
	DrawAnimatingItem(item);
	UpdateElectricityPoints();

	TrigDynamics[0].x = 0;
	TrigDynamics[1].x = 0;
	TrigDynamics[2].x = 0;
}

void TriggerElectricSparks(GAME_VECTOR* pos, long shield)
{
	int dx = lara_item->pos.x_pos - pos->x,
		dz = lara_item->pos.z_pos - pos->z;

	if (dx < -0x5000 || dx > 0x5000 || dz < -0x5000 || dz > 0x5000)
		return;

	TrigDynamics[1] = { pos->x, pos->y, pos->z };

	auto sptr = &spark[GetFreeSpark()];

	sptr->On = 1;
	sptr->sR = 255;
	sptr->sG = 255;
	sptr->sB = 255;

	if (shield)
	{
		sptr->dR = 255;
		sptr->dG = 64 + (GetRandomControl() & 127);
		sptr->dB = 0;
	}
	else if (bossdata.attack_type == ATTACK_HEAD)
	{
		sptr->dR = 0;
		sptr->dG = (GetRandomControl() & 127) + 64;
		sptr->dB = (sptr->dG >> 1) + 128;
	}
	else
	{
		sptr->dR = 0;
		sptr->dB = (GetRandomControl() & 127) + 64;
		sptr->dG = (sptr->dB >> 1) + 128;
	}


	sptr->ColFadeSpeed = 3;
	sptr->FadeToBlack = 8;
	sptr->sLife = sptr->Life = 16;
	sptr->TransType = COLADD;
	sptr->Dynamic = -1;
	sptr->x = pos->x + (GetRandomControl() & 31) - 16;
	sptr->y = pos->y + (GetRandomControl() & 31) - 16;
	sptr->z = pos->z + (GetRandomControl() & 31) - 16;
	sptr->Xvel = ((GetRandomControl() & 511) - 256) << 2;
	sptr->Yvel = ((GetRandomControl() & 511) - 256) << 1;
	sptr->Zvel = ((GetRandomControl() & 511) - 256) << 2;

	if (shield)
	{
		sptr->Xvel >>= 1;
		sptr->Yvel >>= 1;
		sptr->Zvel >>= 1;
	}

	sptr->Friction = 4;
	sptr->Flags = SP_SCALE;
	sptr->Scalar = 3;
	sptr->Width = sptr->sWidth = (GetRandomControl() & 1) + 1;
	sptr->dWidth = (GetRandomControl() & 3) + 4;
	sptr->Height = sptr->sHeight = (GetRandomControl() & 1) + 1;
	sptr->dHeight = (GetRandomControl() & 3) + 4;
	sptr->Gravity = 15;
	sptr->MaxYvel = 0;
}

void TribeBossDie(int16_t item_number)
{
	auto item = &items[item_number];

	item->collidable = 0;
	item->hit_points = DONT_TARGET;

	KillItem(item_number);
	DisableBaddieAI(item_number);

	item->flags |= ONESHOT;
}

void InitialiseTribeBoss(int16_t item_number)
{
	bossdata.LizmanItem = FindLizardManItemNumber(items[item_number].room_number);
	bossdata.LizmanRoom = items[bossdata.LizmanItem].room_number;

	for (int i = 0; i < 3; ++i)
		TrigDynamics[i].x = 0;

	bossdata.attack_count = bossdata.attack_flag = bossdata.death_count = bossdata.attack_head_count =
		lizard_man_active = TribeBossShieldOn = shield_active = bossdata.explode_count = bossdata.ring_count =
		bossdata.dead = bossdata.dropped_icon = 0;

	auto shptr = &TribeBossShield[0];

	for (int i = 0; i < 5; ++i)
	{
		int y = heights_triboss[i],
			angle = 0;

		for (int j = 0; j < 8; ++j, ++shptr)
		{
			shptr->x = (m_sin(angle << 1) * radii_triboss[i]) >> 11;
			shptr->y = y;
			shptr->z = (m_cos(angle << 1) * radii_triboss[i]) >> 11;
			shptr->rgb = 0;

			angle += 512;
		}
	}
}

void FindClosestShieldPoint(long x, long y, long z, ITEM_INFO* item)
{
	auto shptr = &TribeBossShield[0];

	int closestdist = 0x7fffffff,
		closestvert = 0;

	for (int i = 0; i < 40; ++i, ++shptr)
	{
		if (i >= 16 && i <= 23)
		{
			int dx = (shptr->x + item->pos.x_pos) - x,
				dy = (shptr->y + item->pos.y_pos) - y,
				dz = (shptr->z + item->pos.z_pos) - z;

			dx = SQUARE(dx) + SQUARE(dy) + SQUARE(dz);

			if (dx < closestdist)
			{
				closestdist = dx;
				closestvert = i;
			}
		}
	}

	int coladd = 0;

	switch (lara.gun_type)
	{
	case LG_PISTOLS: coladd = 144; break;
	case LG_MAGNUMS: coladd = 200; break;
	case LG_UZIS:	 coladd = 144; break;
	case LG_SHOTGUN: coladd = 192; break;
	case LG_HARPOON: coladd = 200; break;
	case LG_ROCKET:  coladd = 224; break;
	case LG_GRENADE: coladd = 224; break;
	case LG_M16:	 coladd = 192; break;
	}

	long affected[5] = { 0, -1, 1, -8, 8 };

	for (int i = 0; i < 5; ++i)
	{
		int vert = closestvert + affected[i];

		if ((vert & 7) == 7 && affected[i] == -1) vert += 8;
		if ((vert & 7) == 0 && affected[i] == 1)  vert -= 8;

		shptr = &TribeBossShield[vert];

		int rgb = shptr->rgb,
			r = rgb & 255,
			g = ((rgb) >> 8) & 255,
			b = ((rgb) >> 16) & 255;

		if (i == 0)
		{
			if (coladd >= 200)
				r = coladd;
			else
			{
				r += coladd >> 2;

				if (r > coladd)
					r = coladd;
			}
		}
		else
		{
			if (coladd >= 200)
				r = coladd >> 1;
			else
			{
				r += coladd >> 3;

				if (r > coladd >> 1)
					r = coladd >> 1;
			}
		}

		if (i == 0)
		{
			if (coladd >= 200)
				g = coladd;
			else
			{
				g += coladd >> 2;

				if (g > coladd)
					g = coladd;
			}
		}
		else
		{
			if (coladd >= 200)
				g = coladd >> 1;
			else
			{
				g += coladd >> 3;

				if (g > coladd >> 1)
					g = coladd >> 1;
			}
		}

		if (i == 0)
		{
			if (coladd >= 200)
				b = coladd;
			else
			{
				b += coladd >> 2;

				if (b > coladd)
					b = coladd;
			}
		}
		else
		{
			if (coladd >= 200)
				b = coladd >> 1;
			else
			{
				b += coladd >> 3;

				if (b > coladd >> 1)
					b = coladd >> 1;
			}
		}

		shptr->rsub = (GetRandomControl() & 7) + 8;
		shptr->gsub = (GetRandomControl() & 7) + 8;
		shptr->bsub = (GetRandomControl() & 7) + 8;

		if (lara.gun_type == LG_ROCKET || lara.gun_type == LG_GRENADE)
		{
			shptr->rsub >>= 1;
			shptr->gsub >>= 1;
			shptr->bsub >>= 1;
		}

		shptr->rgb = r | (g << 8) | (b << 16);
	}

	GAME_VECTOR	pos { x, y, z };

	for (int i = 0; i < 7; ++i)
		TriggerElectricSparks(&pos, 1);
}

void RotateHeadXAngle(ITEM_INFO* item)
{
	PHD_VECTOR pos1 {}, pos2 {};

	GetJointAbsPosition(lara_item, &pos1, 0);
	GetJointAbsPosition(item, &pos2, 0);

	int x = abs(pos2.x - pos1.x),
		y = pos2.y - pos1.y,
		z = abs(pos2.z - pos1.z);

	x = phd_sqrt(SQUARE(x) + SQUARE(z));

	auto angle = phd_atan(x, y);

	CreatureJoint(item, 2, abs(angle) < 0x200 ? angle : 0);
}

void TriggerLizardMan()
{
	auto item = &items[bossdata.LizmanItem];

	item->object_number = LIZARD_MAN;
	item->room_number = bossdata.LizmanRoom;
	item->pos.x_pos = lizman_summon_coords[bossdata.attack_type - 1][0];
	item->pos.y_pos = lizman_summon_coords[bossdata.attack_type - 1][1];
	item->pos.z_pos = lizman_summon_coords[bossdata.attack_type - 1][2];
	item->anim_number = objects[item->object_number].anim_index;
	item->frame_number = anims[item->anim_number].frame_base;
	item->current_anim_state = item->goal_anim_state = anims[item->anim_number].current_anim_state;
	item->required_anim_state = 0;
	item->pos.y_rot = (bossdata.attack_type == ATTACK_HAND1) ? 0x8000 : 0;
	item->pos.x_rot = item->pos.z_rot = 0;
	item->flags = item->timer = 0;
	item->data = nullptr;
	item->mesh_bits = 0xffffffff;
	item->hit_points = objects[item->object_number].hit_points;
	item->collidable = 1;
	item->active = 0;
	item->status = ACTIVE;

	AddActiveItem(bossdata.LizmanItem);
	EnableBaddieAI(bossdata.LizmanItem, 1);

	auto room_number = item->room_number;

	GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	if (item->room_number != room_number)
		ItemNewRoom(bossdata.LizmanItem, room_number);

	lizard_man_active = 1;

	RemoveDrawnItem(bossdata.LizmanItem);

	auto r = &room[item->room_number];

	item->next_item = r->item_number;

	r->item_number = bossdata.LizmanItem;
}

void ExplodeTribeBoss(ITEM_INFO* item)
{
	TribeBossShieldOn = 0;

	if (bossdata.explode_count == 1 ||
		bossdata.explode_count == 15 ||
		bossdata.explode_count == 25 ||
		bossdata.explode_count == 35 ||
		bossdata.explode_count == 45 ||
		bossdata.explode_count == 55)
	{
		int x = item->pos.x_pos + ((GetRandomDraw() & 1023) - 512),
			y = item->pos.y_pos - (GetRandomDraw() & 1023) - 256,
			z = item->pos.z_pos + ((GetRandomDraw() & 1023) - 512);

		ExpRingsTriboss[bossdata.ring_count].x = x;
		ExpRingsTriboss[bossdata.ring_count].y = y;
		ExpRingsTriboss[bossdata.ring_count].z = z;
		ExpRingsTriboss[bossdata.ring_count++].on = 1;

		TriggerExplosionSparks(x, y, z, 3, -2, 2, 0);

		for (int i = 0; i < 2; ++i)
			TriggerExplosionSparks(x, y, z, 3, -1, 2, 0);

		g_audio->play_sound(76, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
	}

	for (int i = 0; i < 5; ++i)
	{
		if (bossdata.explode_count < 128)
		{
			death_radii_triboss[i] = (dradii_triboss[i] >> 4) + (((dradii_triboss[i]) * bossdata.explode_count) >> 7);
			death_heights_triboss[i] = dheights2_triboss[i] + (((dheights1_triboss[i] - dheights2_triboss[i]) * bossdata.explode_count) >> 7);
		}
	}

	auto shptr = &TribeBossShield[0];

	for (int i = 0; i < 5; ++i)
	{
		int y = death_heights_triboss[i],
			angle = (wibble << 3) & 511;

		for (int j = 0; j < 8; ++j, ++shptr)
		{
			shptr->x = (m_sin(angle << 1) * death_radii_triboss[i]) >> 11;
			shptr->y = y;
			shptr->z = (m_cos(angle << 1) * death_radii_triboss[i]) >> 11;

			if (i != 0 && i != 4 && bossdata.explode_count < 64)
			{
				int r = (GetRandomDraw() & 31),
					b = (GetRandomDraw() & 63) + 224,
					g = (b >> 2) + (GetRandomDraw() & 63);

				r = (r * (64 - bossdata.explode_count)) >> 6;
				g = (g * (64 - bossdata.explode_count)) >> 6;
				b = (b * (64 - bossdata.explode_count)) >> 6;

				shptr->rgb = r | (g << 8) | (b << 16);
			}
			else shptr->rgb = 0;

			angle += 512;
			angle &= 4095;
		}
	}
}

void TriggerSummonSmoke(long x, long y, long z)
{
	auto sptr = &spark[GetFreeSpark()];

	int size = (GetRandomControl() & 31) + 128;

	sptr->On = 1;
	sptr->sR = 16;
	sptr->sG = 64;
	sptr->sB = 0;
	sptr->dR = 8;
	sptr->dG = 32;
	sptr->dB = 0;
	sptr->ColFadeSpeed = 16 + (GetRandomControl() & 7);
	sptr->FadeToBlack = 64;
	sptr->sLife = sptr->Life = (GetRandomControl() & 15) + 96;
	sptr->TransType = COLADD;
	sptr->extras = 0;
	sptr->Dynamic = -1;
	sptr->x = x + ((GetRandomControl() & 127) - 64);
	sptr->y = y - (GetRandomControl() & 31);
	sptr->z = z + ((GetRandomControl() & 127) - 64);
	sptr->Xvel = ((GetRandomControl() & 255) - 128);
	sptr->Yvel = -(GetRandomControl() & 15) - 16;
	sptr->Zvel = ((GetRandomControl() & 255) - 128);
	sptr->Friction = 0;
	
	if (GetRandomControl() & 1)
	{
		sptr->Flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF | SP_WIND;
		sptr->RotAng = GetRandomControl() & 4095;
		sptr->RotAdd = ((GetRandomControl() & 1) ? -(GetRandomControl() & 7) - 4 : (GetRandomControl() & 7) + 4);
	}
	else sptr->Flags = SP_SCALE | SP_DEF | SP_EXPDEF | SP_WIND;

	sptr->Def = uint8_t(objects[EXPLOSION1].mesh_ptr);
	sptr->Scalar = 3;
	sptr->Gravity = -(GetRandomControl() & 7) - 8;
	sptr->MaxYvel = -(GetRandomControl() & 7) - 4;
	sptr->Width = sptr->sWidth = size >> 1;
	sptr->dWidth = size;

	size += (GetRandomControl() & 31) + 32;

	sptr->Height = sptr->sHeight = size >> 1;
	sptr->dHeight = size;
}

int16_t FindLizardManItemNumber(int16_t room_number)
{
	for (int target_number = 0; target_number < level_items; ++target_number)
		if (auto target = &items[target_number]; target->object_number == LIZARD_MAN && target->room_number == room_number)
			return (target_number);

	return -1;
}