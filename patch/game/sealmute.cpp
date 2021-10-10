#include "objects.h"
#include "lara.h"
#include "control.h"
#include "people.h"
#include "sphere.h"
#include "effect2.h"
#include "lot.h"

#include <specific/fn_stubs.h>

#define SEAL_BITE_DAMAGE		100
#define SEAL_SLASH_TOUCH		(0x80)
#define SEAL_KICK_TOUCH			(0x4000)
#define SEAL_DIE_ANIM			5
#define SEAL_WALK_TURN			(3 * ONE_DEGREE)
#define SEAL_RUN_TURN			(6 * ONE_DEGREE)
#define SEAL_ATTACK1_RANGE		SQUARE(WALL_L)
#define SEAL_ATTACK2_RANGE		SQUARE(WALL_L * 2)
#define SEAL_ATTACK3_RANGE		SQUARE(WALL_L * 4 / 3)
#define SEAL_FIRE_RANGE			SQUARE(WALL_L * 2)
#define SEAL_ROAR_CHANCE		0x60
#define SEAL_WALK_CHANCE		(SEAL_ROAR_CHANCE + 0x400)
#define SEAL_AWARE_DISTANCE		SQUARE(WALL_L)
#define SEALMUTE_FLAME_LIMIT	80

enum seal_anims
{
	SEAL_STOP,
	SEAL_WALK,
	SEAL_BURP,
	SEAL_DEATH
};

int16_t TriggerSealmuteGasThrower2(ITEM_INFO* item, BITE_INFO* bite, int16_t speed);
void TriggerSealmuteGas2(long x, long y, long z, long xv, long yv, long zv, long fxnum);

BITE_INFO seal_gas2 = { 0, 48, 140, 10 };

void SealmuteControl(int16_t item_number)
{
	if (!CreatureActive(item_number))
		return;

	auto item = &items[item_number];
	auto seal = (CREATURE_INFO*)item->data;

	int16_t head = 0,
		   torso_y = 0,
		   torso_x = 0,
		   angle = 0,
		   tilt = 0;

	if (item->item_flags[0] > SEALMUTE_FLAME_LIMIT)
		item->hit_points = 0;

	if (!seal)
		return;

	if (item->hit_points <= 0)
	{
		if (item->current_anim_state != SEAL_DEATH)
		{
			item->anim_number = objects[item->object_number].anim_index + SEAL_DIE_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = SEAL_DEATH;
			seal->flags = 0;
		}
		else if (item->item_flags[0] > SEALMUTE_FLAME_LIMIT)
		{
			for (int r = 9; r < 17; ++r)
			{
				if ((wibble & 4) == 0)
				{
					PHD_VECTOR pos {};

					GetJointAbsPosition(item, &pos, r);
					TriggerFireFlame(pos.x, pos.y, pos.z, -1, 255);
				}
			}

			int bright = item->frame_number - anims[item->anim_number].frame_base;

			if (bright > 16)
				if ((bright = anims[item->anim_number].frame_end - item->frame_number) > 16)
					bright = 16;

			int rnd = GetRandomControl(),
				r = ((31 - ((rnd >> 4) & 3)) * bright) >> 4,
				g = ((24 - ((rnd >> 6) & 7)) * bright) >> 4,
				b = ((rnd & 7) * bright) >> 4;

			TriggerDynamicLight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 12, r, g, b);
		}
		else
		{
			if (item->frame_number >= anims[item->anim_number].frame_base + 1 &&
				item->frame_number <= anims[item->anim_number].frame_end - 8)
			{
				uint32_t length = item->frame_number - anims[item->anim_number].frame_base + 1;

				if (length > 24)
				{
					if ((length = anims[item->anim_number].frame_end - item->frame_number - 8) <= 0)
						length = 1;

					if (length > 24)
						length = (GetRandomControl() & 15) + 8;
				}

				TriggerSealmuteGasThrower2(item, &seal_gas2, length);
			}
		}
	}
	else
	{
		AI_INFO info,
				lara_info;

		if (item->ai_bits)
			GetAITarget(seal);
		else
		{
			int lara_dz = lara_item->pos.z_pos - item->pos.z_pos,
				lara_dx = lara_item->pos.x_pos - item->pos.x_pos;

			seal->enemy = lara_item;

			lara_info.distance = lara_dz * lara_dz + lara_dx * lara_dx;

			auto cinfo = baddie_slots;

			for (int i = 0; i < NUM_SLOTS; ++i, ++cinfo)
			{
				if (cinfo->item_num == NO_ITEM || cinfo->item_num == item_number)
					continue;

				auto target = &items[cinfo->item_num];
				if ((target->object_number != LARA && target->object_number != FLAMETHROWER_BLOKE) || target->hit_points <= 0)
					continue;

				int x = target->pos.x_pos - item->pos.x_pos,
					z = target->pos.z_pos - item->pos.z_pos,
					distance = x * x + z * z;

				if (distance < lara_info.distance)
					seal->enemy = target;
			}

		}

		CreatureAIInfo(item, &info);

		if (seal->enemy == lara_item)
		{
			lara_info.angle = info.angle;
			lara_info.distance = info.distance;
		}
		else
		{
			int lara_dz = lara_item->pos.z_pos - item->pos.z_pos,
				lara_dx = lara_item->pos.x_pos - item->pos.x_pos;

			lara_info.angle = phd_atan(lara_dz, lara_dx) - item->pos.y_rot;
			lara_info.distance = lara_dz * lara_dz + lara_dx * lara_dx;
		}

		if (info.zone_number == info.enemy_zone)
		{
			GetCreatureMood(item, &info, VIOLENT);
			if (seal->enemy == lara_item && lara.poisoned >= 0x100)
				seal->mood = ESCAPE_MOOD;
			CreatureMood(item, &info, VIOLENT);
		}
		else
		{
			GetCreatureMood(item, &info, TIMID);
			if (seal->enemy == lara_item && lara.poisoned >= 0x100)
				seal->mood = ESCAPE_MOOD;
			CreatureMood(item, &info, TIMID);
		}

		angle = CreatureTurn(item, seal->maximum_turn);

		auto real_enemy = seal->enemy;

		seal->enemy = lara_item;

		if (lara_info.distance < SEAL_AWARE_DISTANCE || item->hit_status || TargetVisible(item, &lara_info))
			AlertAllGuards(item_number);

		seal->enemy = real_enemy;

		switch (item->current_anim_state)
		{
		case SEAL_STOP:
		{
			seal->maximum_turn = 0;
			seal->flags = 0;

			head = info.angle;

			if (!(item->ai_bits & GUARD))
			{
				if (item->ai_bits & PATROL1)
				{
					item->goal_anim_state = SEAL_WALK;
					head = 0;
				}
				else if (seal->mood == ESCAPE_MOOD)
					item->goal_anim_state = SEAL_WALK;
				else if (Targetable(item, &info) && info.distance < SEAL_FIRE_RANGE)
					item->goal_anim_state = SEAL_BURP;
				else if (item->required_anim_state)
					item->goal_anim_state = item->required_anim_state;
				else item->goal_anim_state = SEAL_WALK;
			}
			else
			{
				head = AIGuard(seal);
				item->goal_anim_state = SEAL_STOP;
			}

			break;
		}
		case SEAL_WALK:
		{
			seal->maximum_turn = SEAL_WALK_TURN;

			if (info.ahead)
				head = info.angle;
			if (item->ai_bits & PATROL1)
			{
				item->goal_anim_state = SEAL_WALK;
				head = 0;
			}
			else if (Targetable(item, &info) && info.distance < SEAL_FIRE_RANGE)
				item->goal_anim_state = SEAL_STOP;

			break;
		}
		case SEAL_BURP:
		{
			if (abs(info.angle) < SEAL_WALK_TURN)
				item->pos.y_rot += info.angle;
			else if (info.angle < 0)
				item->pos.y_rot -= SEAL_WALK_TURN;
			else item->pos.y_rot += SEAL_WALK_TURN;

			if (info.ahead)
			{
				torso_y = info.angle >> 1;
				torso_x = info.x_angle;
			}

			if (item->frame_number >= anims[item->anim_number].frame_base + 35 &&
				item->frame_number <= anims[item->anim_number].frame_base + 58)
			{
				if (seal->flags < 24)
					seal->flags += 3;

				if (seal->flags < 24)
					TriggerSealmuteGasThrower2(item, &seal_gas2, seal->flags);
				else TriggerSealmuteGasThrower2(item, &seal_gas2, (GetRandomControl() & 15) + 8);

				if (seal->enemy != lara_item)
					seal->enemy->hit_status = 1;
			}
		}
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso_x);
	CreatureJoint(item, 1, torso_y);
	CreatureJoint(item, 2, head);

	CreatureAnimation(item_number, angle, tilt);
}

int16_t TriggerSealmuteGasThrower2(ITEM_INFO* item, BITE_INFO* bite, int16_t speed)
{
	auto fx_number = CreateEffect(item->room_number);

	if (fx_number != NO_ITEM)
	{
		auto fx = &effects[fx_number];

		PHD_VECTOR pos1 { bite->x, bite->y, bite->z },
				   pos2 { bite->x, bite->y << 1, bite->z << 3 };

		GetJointAbsPosition(item, &pos1, bite->mesh_num);
		GetJointAbsPosition(item, &pos2, bite->mesh_num);

		const auto angles = phd_GetVectorAngles({ pos2.x - pos1.x,pos2.y - pos1.y,pos2.z - pos1.z });

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
		fx->flag1 = 1;

		TriggerSealmuteGas2(0, 0, 0, 0, 0, 0, fx_number);

		for (int i = 0; i < 2; ++i)
		{
			int spd = (GetRandomControl() % (speed << 2)) + 32,
				vel = (spd * phd_cos(fx->pos.x_rot)) >> W2V_SHIFT,
				zv = (vel * phd_cos(fx->pos.y_rot)) >> W2V_SHIFT,
				xv = (vel * phd_sin(fx->pos.y_rot)) >> W2V_SHIFT,
				yv = -((spd * phd_sin(fx->pos.x_rot)) >> W2V_SHIFT);

			TriggerSealmuteGas2(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, xv << 5, yv << 5, zv << 5, -1);
		}

		int vel = ((speed << 1) * phd_cos(fx->pos.x_rot)) >> W2V_SHIFT,
			zv = (vel * phd_cos(fx->pos.y_rot)) >> W2V_SHIFT,
			xv = (vel * phd_sin(fx->pos.y_rot)) >> W2V_SHIFT,
			yv = -(((speed << 1) * phd_sin(fx->pos.x_rot)) >> W2V_SHIFT);

		TriggerSealmuteGas2(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, xv << 5, yv << 5, zv << 5, -2);
	}

	return fx_number;
}

void TriggerSealmuteGas2(long x, long y, long z, long xv, long yv, long zv, long fxnum)
{
	auto sptr = &spark[GetFreeSpark()];

	int size = (GetRandomControl() & 31) + 48;

	sptr->On = 1;
	sptr->sR = 128 + (GetRandomControl() & 63);
	sptr->sG = 128 + (GetRandomControl() & 63);
	sptr->sB = 32;
	sptr->dR = 32 + (GetRandomControl() & 15);
	sptr->dG = 32 + (GetRandomControl() & 15);
	sptr->dB = 0;

	if (xv || yv || zv)
	{
		sptr->ColFadeSpeed = 6;
		sptr->FadeToBlack = 2;
		sptr->sLife = sptr->Life = (GetRandomControl() & 1) + 16;
	}
	else
	{
		sptr->ColFadeSpeed = 8;
		sptr->FadeToBlack = 16;
		sptr->sLife = sptr->Life = (GetRandomControl() & 3) + 28;
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
		sptr->RotAng = GetRandomControl() & 4095;
		sptr->RotAdd = ((GetRandomControl() & 1) ? -(GetRandomControl() & 15) - 16 : (GetRandomControl() & 15) + 16);
		sptr->Flags = (fxnum >= 0 ? (SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF | SP_FX)
								  : (SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF));
	}
	else sptr->Flags = (fxnum >= 0 ? (SP_SCALE | SP_DEF | SP_EXPDEF | SP_FX) : (SP_SCALE | SP_DEF | SP_EXPDEF));

	sptr->FxObj = fxnum;
	sptr->Gravity = sptr->MaxYvel = 0;
	sptr->Def = objects[EXPLOSION1].mesh_index;

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