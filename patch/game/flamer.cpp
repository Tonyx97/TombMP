#include <specific/standard.h>

#include "effect2.h"
#include "objects.h"
#include "lara.h"
#include "control.h"
#include "people.h"
#include "sphere.h"
#include "lot.h"
#include "game.h"

int16_t TriggerFlameThrower(ITEM_INFO* item, BITE_INFO* bite, int16_t speed);
void TriggerFlamethrowerFlame(long x, long y, long z, long xv, long yv, long zv, long fxnum);
void TriggerPilotFlame(long itemnum);

#define FLAMER_SHOT_DAMAGE		1
#define FLAMER_WALK_TURN		(ONE_DEGREE * 5)
#define FLAMER_RUN_TURN			(ONE_DEGREE * 10)
#define FLAMER_RUN_RANGE		SQUARE(WALL_L * 2)
#define FLAMER_SHOOT1_RANGE		SQUARE(WALL_L * 4)
#define FLAMER_AWARE_DISTANCE	SQUARE(WALL_L)
#define FLAMER_DIE_ANIM			19
#define FLAMER_STOP_ANIM		12
#define SEAL_BURP_STATE			2
#define SEAL_X_OFFSET			0x800

enum flamer_anims
{
	FLAMER_EMPTY,
	FLAMER_STOP,
	FLAMER_WALK,
	FLAMER_RUN,
	FLAMER_WAIT,
	FLAMER_SHOOT1,
	FLAMER_SHOOT2,
	FLAMER_DEATH,
	FLAMER_AIM1,
	FLAMER_AIM2,
	FLAMER_AIM3,
	FLAMER_SHOOT3
};

BITE_INFO flamer_gun = { 0, 340, 64, 7 };

void FlamerControl(int16_t item_number)
{
	if (!CreatureActive(item_number))
		return;

	auto item = &items[item_number];
	auto flamer = (CREATURE_INFO*)item->data;

	int16_t torso_y = 0,
		   torso_x = 0,
		   head = 0,
		   angle = 0,
		   tilt = 0;

	PHD_VECTOR pos { flamer_gun.x, flamer_gun.y, flamer_gun.z };

	GetJointAbsPosition(item, &pos, flamer_gun.mesh_num);

	int rnd = GetRandomControl();

	if (item->current_anim_state != FLAMER_SHOOT2 && item->current_anim_state != FLAMER_SHOOT3)
	{
		TriggerDynamicLight(pos.x, pos.y, pos.z, (rnd & 3) + 6, 24 - ((rnd >> 4) & 3), 16 - ((rnd >> 6) & 3), rnd & 3);
		TriggerPilotFlame(item_number);
	}
	else TriggerDynamicLight(pos.x, pos.y, pos.z, (rnd & 3) + 10, 31 - ((rnd >> 4) & 3), 24 - ((rnd >> 6) & 3), rnd & 7);

	if (!flamer)
		return;

	if (item->hit_points <= 0)
	{
		if (item->current_anim_state != FLAMER_DEATH)
		{
			item->anim_number = objects[item->object_number].anim_index + FLAMER_DIE_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = FLAMER_DEATH;
		}
	}
	else
	{
		AI_INFO info, lara_info;

		if (item->ai_bits)
			GetAITarget(flamer);
		else if (flamer->hurt_by_lara || !enable_flamer_friendly)
			flamer->enemy = lara_item;
		else
		{
			flamer->enemy = nullptr;

			int best_distance = 0x7fffffff;

			auto cinfo = baddie_slots;

			for (int i = 0; i < NUM_SLOTS; ++i, ++cinfo)
			{
				if (cinfo->item_num == NO_ITEM || cinfo->item_num == item_number)
					continue;

				auto target = &items[cinfo->item_num];
				if (target->object_number == LARA || target->object_number == WHITE_SOLDIER || target->object_number == FLAMETHROWER_BLOKE || target->hit_points <= 0)
					continue;

				int x = target->pos.x_pos - item->pos.x_pos,
					z = target->pos.z_pos - item->pos.z_pos,
					distance = x * x + z * z;

				if (distance < best_distance)
				{
					flamer->enemy = target;
					best_distance = distance;
				}
			}
		}

		CreatureAIInfo(item, &info);

		if (flamer->enemy == lara_item)
		{
			lara_info.angle = info.angle;
			lara_info.distance = info.distance;

			if (!flamer->hurt_by_lara && enable_flamer_friendly)
				flamer->enemy = nullptr;
		}
		else
		{
			int lara_dz = lara_item->pos.z_pos - item->pos.z_pos,
				lara_dx = lara_item->pos.x_pos - item->pos.x_pos;

			lara_info.angle = phd_atan(lara_dz, lara_dx) - item->pos.y_rot;
			lara_info.distance = lara_dz * lara_dz + lara_dx * lara_dx;
			info.x_angle -= SEAL_X_OFFSET;
		}

		GetCreatureMood(item, &info, TIMID);
		CreatureMood(item, &info, TIMID);

		angle = CreatureTurn(item, flamer->maximum_turn);

		auto real_enemy = flamer->enemy;

		if (item->hit_status || ((lara_info.distance < FLAMER_AWARE_DISTANCE || TargetVisible(item, &lara_info)) && !enable_flamer_friendly))
		{
			if (!flamer->alerted)
				g_audio->play_sound(300, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

			AlertAllGuards(item_number);
		}

		switch (item->current_anim_state)
		{
		case FLAMER_STOP:
		{
			flamer->flags = 0;
			flamer->maximum_turn = 0;

			head = lara_info.angle;

			if (item->ai_bits & GUARD)
			{
				head = AIGuard(flamer);

				if (!(GetRandomControl() & 0xFF))
					item->goal_anim_state = FLAMER_WAIT;
			}
			else if (item->ai_bits & PATROL1)
				item->goal_anim_state = FLAMER_WALK;
			else if (flamer->mood == ESCAPE_MOOD)
				item->goal_anim_state = FLAMER_WALK;
			else if (Targetable(item, &info) && (real_enemy != lara_item || flamer->hurt_by_lara || !enable_flamer_friendly))
				item->goal_anim_state = (info.distance < FLAMER_SHOOT1_RANGE ? FLAMER_AIM3 : FLAMER_WALK);
			else if (flamer->mood == BORED_MOOD && info.ahead && !(GetRandomControl() & 0xFF))
				item->goal_anim_state = FLAMER_WAIT;
			else if (flamer->mood == ATTACK_MOOD || !(GetRandomControl() & 0xFF))
				item->goal_anim_state = FLAMER_WALK;

			break;
		}
		case FLAMER_WAIT:
		{
			head = lara_info.angle;

			if (item->ai_bits & GUARD)
			{
				head = AIGuard(flamer);

				if (!(GetRandomControl() & 0xFF))
					item->goal_anim_state = FLAMER_STOP;
			}
			else if ((Targetable(item, &info) && info.distance < FLAMER_SHOOT1_RANGE && (real_enemy != lara_item || flamer->hurt_by_lara || !enable_flamer_friendly)) || flamer->mood != BORED_MOOD || !(GetRandomControl() & 0xFF))
				item->goal_anim_state = FLAMER_STOP;

			break;
		}
		case FLAMER_WALK:
		{
			flamer->flags = 0;
			flamer->maximum_turn = FLAMER_WALK_TURN;

			head = lara_info.angle;

			if (item->ai_bits & GUARD)
			{
				item->anim_number = objects[item->object_number].anim_index + FLAMER_STOP_ANIM;
				item->frame_number = anims[item->anim_number].frame_base;
				item->current_anim_state = FLAMER_STOP;
				item->goal_anim_state = FLAMER_STOP;
			}
			else if (item->ai_bits & PATROL1)
				item->goal_anim_state = FLAMER_WALK;
			else if (flamer->mood == ESCAPE_MOOD)
				item->goal_anim_state = FLAMER_WALK;
			else if (Targetable(item, &info) && (real_enemy != lara_item || flamer->hurt_by_lara || !enable_flamer_friendly))
				item->goal_anim_state = (info.distance < FLAMER_SHOOT1_RANGE ? FLAMER_STOP : FLAMER_AIM2);
			else if (flamer->mood == BORED_MOOD && info.ahead)
				item->goal_anim_state = FLAMER_STOP;
			else item->goal_anim_state = FLAMER_WALK;

			break;
		}
		case FLAMER_AIM3:
		{
			flamer->flags = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;

				item->goal_anim_state = (Targetable(item, &info) && info.distance < FLAMER_SHOOT1_RANGE && (real_enemy != lara_item || flamer->hurt_by_lara || !enable_flamer_friendly) ? FLAMER_SHOOT3
																																														: FLAMER_STOP);
			}

			break;
		}
		case FLAMER_AIM2:
		{
			flamer->flags = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;

				item->goal_anim_state = (Targetable(item, &info) && info.distance < FLAMER_SHOOT1_RANGE && (real_enemy != lara_item || flamer->hurt_by_lara || !enable_flamer_friendly) ? FLAMER_SHOOT2
																																														: FLAMER_WALK);
			}

			break;
		}
		case FLAMER_SHOOT3:
		{
			if (flamer->flags < 40)
				flamer->flags += (flamer->flags >> 2) + 1;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
				item->goal_anim_state = (Targetable(item, &info) && info.distance < FLAMER_SHOOT1_RANGE && (real_enemy != lara_item || flamer->hurt_by_lara || !enable_flamer_friendly) ? FLAMER_SHOOT3
																																														: FLAMER_STOP);
			}
			else
				item->goal_anim_state = FLAMER_STOP;

			if (flamer->flags < 40)
				TriggerFlameThrower(item, &flamer_gun, flamer->flags);
			else
			{
				TriggerFlameThrower(item, &flamer_gun, (GetRandomControl() & 31) + 12);

				if (real_enemy && real_enemy->object_number == BURNT_MUTANT)
					++real_enemy->item_flags[0];
			}

			g_audio->play_sound(204, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

			break;
		}
		case FLAMER_SHOOT2:
		{
			if (flamer->flags < 40)
				flamer->flags += (flamer->flags >> 2) + 1;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
				item->goal_anim_state = (Targetable(item, &info) && info.distance < FLAMER_SHOOT1_RANGE && (real_enemy != lara_item || flamer->hurt_by_lara || !enable_flamer_friendly) ? FLAMER_SHOOT2
																																														: FLAMER_WALK);
			}
			else item->goal_anim_state = FLAMER_WALK;

			if (flamer->flags < 40)
				TriggerFlameThrower(item, &flamer_gun, flamer->flags);
			else
			{
				TriggerFlameThrower(item, &flamer_gun, (GetRandomControl() & 31) + 12);

				if (real_enemy && real_enemy->object_number == BURNT_MUTANT)
					++real_enemy->item_flags[0];
			}

			g_audio->play_sound(204, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
		}
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso_y);
	CreatureJoint(item, 1, torso_x);
	CreatureJoint(item, 2, head);

	CreatureAnimation(item_number, angle, 0);
}

int16_t TriggerFlameThrower(ITEM_INFO* item, BITE_INFO* bite, int16_t speed)
{
	PHD_VECTOR	pos1, pos2;

	auto fx_number = CreateEffect(item->room_number);

	if (fx_number != NO_ITEM)
	{
		auto fx = &effects[fx_number];

		pos1.x = bite->x;
		pos1.y = bite->y;
		pos1.z = bite->z;

		GetJointAbsPosition(item, &pos1, bite->mesh_num);

		pos2.x = bite->x;
		pos2.y = bite->y << 1;
		pos2.z = bite->z;

		GetJointAbsPosition(item, &pos2, bite->mesh_num);

		auto angles = phd_GetVectorAngles({ pos2.x - pos1.x,pos2.y - pos1.y,pos2.z - pos1.z });

		fx->pos.x_pos = pos1.x;
		fx->pos.y_pos = pos1.y;
		fx->pos.z_pos = pos1.z;
		fx->room_number = item->room_number;
		fx->pos.x_rot = angles.y;
		fx->pos.z_rot = 0;
		fx->pos.y_rot = angles.x;
		fx->speed = speed << 2;
		fx->object_number = DRAGON_FIRE;
		fx->counter = 20;
		fx->flag1 = 0;

		TriggerFlamethrowerFlame(0, 0, 0, 0, 0, 0, fx_number);

		for (int i = 0; i < 2; ++i)
		{
			int spd = (GetRandomControl() % (speed << 2)) + 32,
				vel = (spd * phd_cos(fx->pos.x_rot)) >> W2V_SHIFT,
				zv = (vel * phd_cos(fx->pos.y_rot)) >> W2V_SHIFT,
				xv = (vel * phd_sin(fx->pos.y_rot)) >> W2V_SHIFT,
				yv = -((spd * phd_sin(fx->pos.x_rot)) >> W2V_SHIFT);

			TriggerFlamethrowerFlame(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, xv << 5, yv << 5, zv << 5, -1);
		}

		int vel = ((speed << 1) * phd_cos(fx->pos.x_rot)) >> W2V_SHIFT,
			zv = (vel * phd_cos(fx->pos.y_rot)) >> W2V_SHIFT,
			xv = (vel * phd_sin(fx->pos.y_rot)) >> W2V_SHIFT,
			yv = -(((speed << 1) * phd_sin(fx->pos.x_rot)) >> W2V_SHIFT);

		TriggerFlamethrowerFlame(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, xv << 5, yv << 5, zv << 5, -2);
	}

	return fx_number;
}

void TriggerFlamethrowerFlame(long x, long y, long z, long xv, long yv, long zv, long fxnum)
{
	auto sptr = &spark[GetFreeSpark()];

	int size = (GetRandomControl() & 31) + 64;

	sptr->On = 1;
	sptr->sR = 48 + (GetRandomControl() & 31);
	sptr->sG = sptr->sR;
	sptr->sB = 192 + (GetRandomControl() & 63);
	sptr->dR = 192 + (GetRandomControl() & 63);
	sptr->dG = 128 + (GetRandomControl() & 63);
	sptr->dB = 32;

	if (xv || yv || zv)
	{
		sptr->ColFadeSpeed = 6;
		sptr->FadeToBlack = 2;
		sptr->sLife = sptr->Life = (GetRandomControl() & 1) + 12;
	}
	else
	{
		sptr->ColFadeSpeed = 8;
		sptr->FadeToBlack = 16;
		sptr->sLife = sptr->Life = (GetRandomControl() & 3) + 20;
	}

	sptr->TransType = COLADD;
	sptr->extras = 0;
	sptr->Dynamic = -1;
	sptr->x = x + ((GetRandomControl() & 31) - 16);
	sptr->y = y;
	sptr->z = z + ((GetRandomControl() & 31) - 16);
	sptr->Xvel = ((GetRandomControl() & 15) - 16) + xv;
	sptr->Yvel = yv;
	sptr->Zvel = ((GetRandomControl() & 15) - 16) + zv;
	sptr->Friction = 0;

	if (GetRandomControl() & 1)
	{
		sptr->Flags = (fxnum >= 0 ? (SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF | SP_FX) : (SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF));
		sptr->RotAng = GetRandomControl() & 4095;
		sptr->RotAdd = ((GetRandomControl() & 1) ? -(GetRandomControl() & 15) - 16 : (GetRandomControl() & 15) + 16);
	}
	else sptr->Flags = (fxnum >= 0 ? (SP_SCALE | SP_DEF | SP_EXPDEF | SP_FX) : (SP_SCALE | SP_DEF | SP_EXPDEF));

	sptr->FxObj = fxnum;
	sptr->Gravity = sptr->MaxYvel = 0;
	sptr->Def = (PHDSPRITESTRUCT*)objects[EXPLOSION1].mesh_ptr;

	if (xv || yv || zv)
	{
		sptr->sWidth = sptr->Width = size >> 5;
		sptr->sHeight = sptr->Height = size >> 5;
		sptr->Scalar = (fxnum == -2 ? 2 : 3);
	}
	else
	{
		sptr->sWidth = sptr->Width = size >> 4;
		sptr->sHeight = sptr->Height = size >> 4;
		sptr->Scalar = 4;
	}

	sptr->dWidth = size >> 1;
	sptr->dHeight = size >> 1;
}

void TriggerPilotFlame(long itemnum)
{
	int dx = lara_item->pos.x_pos - items[itemnum].pos.x_pos,
		dz = lara_item->pos.z_pos - items[itemnum].pos.z_pos;

	if (g_effects_draw_distance != -1 && (dx < -g_effects_draw_distance || dx > g_effects_draw_distance || dz < -g_effects_draw_distance || dz > g_effects_draw_distance))
		return;

	auto sptr = &spark[GetFreeSpark()];

	int size = (GetRandomControl() & 7) + 32;

	sptr->On = 1;
	sptr->sR = 48 + (GetRandomControl() & 31);
	sptr->sG = sptr->sR;
	sptr->sB = 192 + (GetRandomControl() & 63);
	sptr->dR = 192 + (GetRandomControl() & 63);
	sptr->dG = 128 + (GetRandomControl() & 63);
	sptr->dB = 32;
	sptr->ColFadeSpeed = 12 + (GetRandomControl() & 3);
	sptr->FadeToBlack = 4;
	sptr->sLife = sptr->Life = (GetRandomControl() & 3) + 20;
	sptr->TransType = COLADD;
	sptr->extras = 0;
	sptr->Dynamic = -1;
	sptr->x = (GetRandomControl() & 31) - 16;
	sptr->y = (GetRandomControl() & 31) - 16;
	sptr->z = (GetRandomControl() & 31) - 16;
	sptr->Xvel = (GetRandomControl() & 31) - 16;
	sptr->Yvel = -(GetRandomControl() & 3);
	sptr->Zvel = (GetRandomControl() & 31) - 16;
	sptr->Flags = SP_SCALE | SP_DEF | SP_EXPDEF | SP_ITEM | SP_NODEATTATCH;
	sptr->FxObj = itemnum;
	sptr->NodeNumber = SPN_PILOTFLAME;
	sptr->Friction = 4;
	sptr->Gravity = -(GetRandomControl() & 3) - 2;
	sptr->MaxYvel = -(GetRandomControl() & 3) - 4;
	sptr->Def = (PHDSPRITESTRUCT*)objects[EXPLOSION1].mesh_ptr;
	sptr->Scalar = 0;
	sptr->Width = sptr->sWidth = size >> 1;
	sptr->Height = sptr->sHeight = size >> 1;
	sptr->dWidth = size;
	sptr->dHeight = size;
}