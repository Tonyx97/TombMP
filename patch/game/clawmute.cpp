#include "objects.h"
#include "lara.h"
#include "control.h"
#include "effect2.h"
#include "sphere.h"
#include "people.h"
#include "traps.h"

#include <specific/standard.h>
#include <specific/fn_stubs.h>

#define CLAW_PLASMA_DAMAGE 200
#define CLAW_BITE_DAMAGE 100
#define CLAW_TOUCH (0x90)
#define CLAW_DIE_ANIM 20
#define CLAW_WALK_TURN (3*ONE_DEGREE)
#define CLAW_RUN_TURN  (4*ONE_DEGREE)
#define CLAW_ATTACK1_RANGE SQUARE(WALL_L)
#define CLAW_ATTACK2_RANGE SQUARE(WALL_L*2)
#define CLAW_ATTACK3_RANGE SQUARE(WALL_L*4/3)
#define CLAW_FIRE_RANGE SQUARE(WALL_L*3)
#define CLAW_ROAR_CHANCE 0x60
#define CLAW_WALK_CHANCE (CLAW_ROAR_CHANCE + 0x400)
#define CLAW_AWARE_DISTANCE SQUARE(WALL_L)

enum claw_anims
{
	CLAW_STOP,
	CLAW_WALK,
	CLAW_RUN,
	CLAW_RUN_ATAK,
	CLAW_WALK_ATAK1,
	CLAW_WALK_ATAK2,
	CLAW_SLASH_LEFT,
	CLAW_SLASH_RIGHT,
	CLAW_DEATH,
	CLAW_CLAW_ATAK,
	CLAW_FIRE_ATAK
};

void TriggerPlasma(int16_t item_number);
void TriggerPlasmaBallFlame(int16_t fx_number, long type, long xv, long yv, long zv);
void TriggerPlasmaBall(ITEM_INFO* item, long type, PHD_VECTOR* pos1, int16_t room_number, int16_t angle);

BITE_INFO claw_bite_left = { 19, -13, 3, 7 },
		  claw_bite_right = { 19, -13, 3, 4 };

void ClawmuteControl(int16_t item_number)
{
	if (!CreatureActive(item_number))
		return;

	auto item = &items[item_number];
	auto claw = (CREATURE_INFO*)item->data;

	if (!claw)
		return;

	int16_t head = 0,
		   torso_y = 0,
		   torso_x = 0,
		   angle = 0,
		   tilt = 0;

	if (item->hit_points <= 0)
	{
		if (item->current_anim_state != CLAW_DEATH)
		{
			item->anim_number = objects[item->object_number].anim_index + CLAW_DIE_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = CLAW_DEATH;
		}

		if (item->frame_number == anims[item->anim_number].frame_end - 1)
		{
			CreatureDie(item_number, true);
			TriggerExplosionSparks(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 3, -2, 2, 0);

			for (int i = 0; i < 2; ++i)
				TriggerExplosionSparks(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 3, -1, 2, 0);

			g_audio->play_sound(106, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

			return;
		}
	}
	else
	{
		AI_INFO info,
				lara_info;

		if (item->ai_bits)
			GetAITarget(claw);

		CreatureAIInfo(item, &info);

		if (claw->enemy == lara_item)
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
			CreatureMood(item, &info, VIOLENT);
		}
		else
		{
			GetCreatureMood(item, &info, TIMID);
			CreatureMood(item, &info, TIMID);
		}

		angle = CreatureTurn(item, claw->maximum_turn);

		auto real_enemy = claw->enemy;

		claw->enemy = lara_item;

		if (lara_info.distance < CLAW_AWARE_DISTANCE || item->hit_status || TargetVisible(item, &lara_info))
			AlertAllGuards(item_number);

		claw->enemy = real_enemy;

		switch (item->current_anim_state)
		{
		case CLAW_STOP:
		{
			claw->maximum_turn = 0;
			claw->flags = 0;

			head = info.angle;

			if (item->ai_bits & GUARD)
			{
				head = AIGuard(claw);
				item->goal_anim_state = CLAW_STOP;
			}
			else if (item->ai_bits & PATROL1)
			{
				item->goal_anim_state = CLAW_WALK;
				head = 0;
			}
			else if (claw->mood == ESCAPE_MOOD)
				item->goal_anim_state = CLAW_RUN;
			else if (info.bite && info.distance < CLAW_ATTACK1_RANGE)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;

				item->goal_anim_state = (info.angle < 0 ? CLAW_SLASH_LEFT : CLAW_SLASH_RIGHT);
			}
			else if (info.bite && info.distance < CLAW_ATTACK3_RANGE)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;

				item->goal_anim_state = CLAW_CLAW_ATAK;
			}
			else if (Targetable(item, &info) && ((info.distance > CLAW_FIRE_RANGE && !item->item_flags[0]) || info.zone_number != info.enemy_zone))
				item->goal_anim_state = CLAW_FIRE_ATAK;
			else if (claw->mood == BORED_MOOD)
				item->goal_anim_state = CLAW_WALK;
			else if (item->required_anim_state)
				item->goal_anim_state = item->required_anim_state;
			else item->goal_anim_state = CLAW_RUN;

			break;
		}
		case CLAW_WALK:
		{
			claw->maximum_turn = CLAW_WALK_TURN;

			if (info.ahead)
				head = info.angle;

			if (item->ai_bits & PATROL1)
			{
				item->goal_anim_state = CLAW_WALK;
				head = 0;
			}
			else if (info.bite && info.distance < CLAW_ATTACK3_RANGE)
			{
				claw->maximum_turn = CLAW_WALK_TURN;
				item->goal_anim_state = (info.angle < 0 ? CLAW_WALK_ATAK1 : CLAW_WALK_ATAK2);
			}
			else if (Targetable(item, &info) && ((info.distance > CLAW_FIRE_RANGE && !item->item_flags[0]) || info.zone_number != info.enemy_zone))
			{
				claw->maximum_turn = CLAW_WALK_TURN;
				item->goal_anim_state = CLAW_STOP;
			}
			else if (claw->mood == ESCAPE_MOOD || claw->mood == ATTACK_MOOD)
				item->goal_anim_state = CLAW_RUN;

			break;
		}
		case CLAW_RUN:
		{
			claw->maximum_turn = CLAW_RUN_TURN;

			if (info.ahead)
				head = info.angle;
			if (item->ai_bits & GUARD)
				item->goal_anim_state = CLAW_STOP;
			else if (claw->mood == BORED_MOOD)
				item->goal_anim_state = CLAW_STOP;
			else if (claw->flags && info.ahead)
				item->goal_anim_state = CLAW_STOP;
			else if (info.bite && info.distance < CLAW_ATTACK2_RANGE)
				item->goal_anim_state = (lara_item->speed == 0 ? CLAW_STOP : CLAW_RUN_ATAK);
			else if (Targetable(item, &info) && ((info.distance > CLAW_FIRE_RANGE && !item->item_flags[0]) || info.zone_number != info.enemy_zone))
			{
				claw->maximum_turn = CLAW_WALK_TURN;
				item->goal_anim_state = CLAW_STOP;
			}

			claw->flags = 0;

			break;
		}
		case CLAW_WALK_ATAK1:
		case CLAW_WALK_ATAK2:
		case CLAW_SLASH_LEFT:
		case CLAW_SLASH_RIGHT:
		case CLAW_CLAW_ATAK:
		case CLAW_RUN_ATAK:
		{
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			if (!claw->flags && (item->touch_bits & CLAW_TOUCH))
			{
				lara_item->hit_status = 1;
				lara_item->hit_points -= CLAW_BITE_DAMAGE;

				CreatureEffect(item, &claw_bite_left, DoBloodSplat);
				CreatureEffect(item, &claw_bite_right, DoBloodSplat);

				claw->flags = 1;
			}

			item->item_flags[0] = 0;

			break;
		}
		case CLAW_FIRE_ATAK:
		{
			if (abs(info.angle) < CLAW_WALK_TURN)
				item->pos.y_rot += info.angle;
			else if (info.angle < 0)
				item->pos.y_rot -= CLAW_WALK_TURN;
			else item->pos.y_rot += CLAW_WALK_TURN;

			if (info.ahead)
			{
				torso_y = info.angle >> 1;
				torso_x = info.x_angle;
			}

			if (item->frame_number == anims[item->anim_number].frame_base && !(GetRandomControl() & 0x3))
				item->item_flags[0] = 1;

			if (item->frame_number - anims[item->anim_number].frame_base < 28)
				TriggerPlasma(item_number);
			else if (item->frame_number - anims[item->anim_number].frame_base == 28)
				TriggerPlasmaBall(item, 0, nullptr, item->room_number, item->pos.y_rot);

			{
				int bright = item->frame_number - anims[item->anim_number].frame_base;

				if (bright > 16)
					if ((bright = anims[item->anim_number].frame_base + 28 + 16 - item->frame_number) > 16)
						bright = 16;

				if (bright > 0)
				{
					int rnd = GetRandomControl(),
						r = ((rnd & 7) * bright) >> 4,
						g = ((24 - ((rnd >> 6) & 3)) * bright) >> 4,
						b = ((31 - ((rnd >> 4) & 3)) * bright) >> 4;

					PHD_VECTOR pos { -32, -16, -192 };

					GetJointAbsPosition(item, &pos, 13);
					TriggerDynamicLight(pos.x, pos.y, pos.z, 13, r, g, b);
				}
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

void TriggerPlasma(int16_t item_number)
{
	int dx = lara_item->pos.x_pos - items[item_number].pos.x_pos,
		dz = lara_item->pos.z_pos - items[item_number].pos.z_pos;

	if (g_effects_draw_distance != -1 && (dx < -g_effects_draw_distance || dx > g_effects_draw_distance || dz < -g_effects_draw_distance || dz > g_effects_draw_distance))
		return;

	auto sptr = &spark[GetFreeSpark()];

	int size = (GetRandomControl() & 31) + 64;

	sptr->On = 1;
	sptr->sB = 255;
	sptr->sG = 48 + (GetRandomControl() & 31);
	sptr->sR = 48;
	sptr->dB = 192 + (GetRandomControl() & 63);
	sptr->dG = 128 + (GetRandomControl() & 63);
	sptr->dR = 32;
	sptr->ColFadeSpeed = 12 + (GetRandomControl() & 3);
	sptr->FadeToBlack = 8;
	sptr->sLife = sptr->Life = (GetRandomControl() & 7) + 24;
	sptr->TransType = COLADD;
	sptr->extras = 0;
	sptr->Dynamic = -1;
	sptr->x = ((GetRandomControl() & 15) - 8);
	sptr->y = 0;
	sptr->z = ((GetRandomControl() & 15) - 8);
	sptr->Xvel = ((GetRandomControl() & 31) - 16);
	sptr->Yvel = (GetRandomControl() & 15) + 16;
	sptr->Zvel = ((GetRandomControl() & 31) - 16);
	sptr->Friction = 3;

	if (GetRandomControl() & 1)
	{
		sptr->Flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF | SP_ITEM | SP_NODEATTATCH;
		sptr->RotAng = GetRandomControl() & 4095;
		sptr->RotAdd = ((GetRandomControl() & 1) ? -(GetRandomControl() & 15) - 16 : (GetRandomControl() & 15) + 16);
	}
	else sptr->Flags = SP_SCALE | SP_DEF | SP_EXPDEF | SP_ITEM | SP_NODEATTATCH;

	sptr->Gravity = (GetRandomControl() & 31) + 16;
	sptr->MaxYvel = (GetRandomControl() & 7) + 16;
	sptr->FxObj = item_number;
	sptr->NodeNumber = SPN_CLAWMUTEPLASMA;
	sptr->Def = (PHDSPRITESTRUCT*)objects[EXPLOSION1].mesh_ptr;
	sptr->Scalar = 1;
	sptr->Width = sptr->sWidth = size;
	sptr->Height = sptr->sHeight = size;
	sptr->dWidth = size >> 2;
	sptr->dHeight = size >> 2;
}

void TriggerPlasmaBallFlame(int16_t fx_number, long type, long xv, long yv, long zv)
{
	int dx = lara_item->pos.x_pos - effects[fx_number].pos.x_pos,
		dz = lara_item->pos.z_pos - effects[fx_number].pos.z_pos;

	if (g_effects_draw_distance != -1 && (dx < -g_effects_draw_distance || dx > g_effects_draw_distance || dz < -g_effects_draw_distance || dz > g_effects_draw_distance))
		return;

	auto sptr = &spark[GetFreeSpark()];

	int size = (GetRandomControl() & 31) + 64;
	
	sptr->On = 1;
	sptr->sB = 255;
	sptr->sG = 48 + (GetRandomControl() & 31);
	sptr->sR = 48;
	sptr->dB = 192 + (GetRandomControl() & 63);
	sptr->dG = 128 + (GetRandomControl() & 63);
	sptr->dR = 32;
	sptr->ColFadeSpeed = 12 + (GetRandomControl() & 3);
	sptr->FadeToBlack = 8;
	sptr->sLife = sptr->Life = (GetRandomControl() & 7) + 24;
	sptr->TransType = COLADD;
	sptr->extras = 0;
	sptr->Dynamic = -1;
	sptr->x = ((GetRandomControl() & 15) - 8);
	sptr->y = 0;
	sptr->z = ((GetRandomControl() & 15) - 8);
	sptr->Xvel = xv + (GetRandomControl() & 255) - 128;
	sptr->Yvel = yv;
	sptr->Zvel = zv + (GetRandomControl() & 255) - 128;
	sptr->Friction = 5;

	if (GetRandomControl() & 1)
	{
		sptr->Flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF | SP_FX;
		sptr->RotAng = GetRandomControl() & 4095;
		sptr->RotAdd = ((GetRandomControl() & 1) ? -(GetRandomControl() & 15) - 16 : (GetRandomControl() & 15) + 16);
	}
	else sptr->Flags = SP_SCALE | SP_DEF | SP_EXPDEF | SP_FX;

	sptr->FxObj = fx_number;

	sptr->Def = (PHDSPRITESTRUCT*)objects[EXPLOSION1].mesh_ptr;
	sptr->Scalar = 1;
	sptr->Width = sptr->sWidth = size;
	sptr->Height = sptr->sHeight = size;
	sptr->dWidth = size >> 2;
	sptr->dHeight = size >> 2;
	sptr->Gravity = sptr->MaxYvel = 0;

	if (type == 0)
	{
		sptr->Yvel = (GetRandomControl() & 511) - 256;
		sptr->Xvel <<= 1;
		sptr->Zvel <<= 1;
		sptr->Scalar = 2;
		sptr->Friction = 5 | (5 << 4);
		sptr->dWidth >>= 1;
		sptr->dHeight >>= 1;
	}
}

void TriggerPlasmaBall(ITEM_INFO* item, long type, PHD_VECTOR* pos1, int16_t room_number, int16_t angle)
{
	PHD_VECTOR pos;
	PHD_ANGLE_VEC angles;

	int speed;

	if (type == 0)
	{
		pos = { -32, -16, -192 };

		GetJointAbsPosition(item, &pos, 13);

		speed = (GetRandomControl() & 7) + 8;
		angles = phd_GetVectorAngles({ lara_item->pos.x_pos - pos.x, lara_item->pos.y_pos - pos.y - 256, lara_item->pos.z_pos - pos.z });
		angles.x = item->pos.y_rot;
	}
	else
	{
		pos = { pos1->x, pos1->y, pos1->z };
		speed = (GetRandomControl() & 15) + 16;
		angles.x = GetRandomControl() << 1;
		angles.y = 0x2000;
	}

	if (auto fx_number = CreateEffect(room_number); fx_number != NO_ITEM)
	{
		auto fx = &effects[fx_number];

		fx->pos.x_pos = pos.x;
		fx->pos.y_pos = pos.y;
		fx->pos.z_pos = pos.z;
		fx->pos.y_rot = angles.x;
		fx->pos.x_rot = angles.y;
		fx->object_number = EXTRAFX1;
		fx->speed = speed;
		fx->fallspeed = 0;
		fx->flag1 = type;
	}
}

void ControlClawmutePlasmaBall(int16_t fx_number)
{
	auto fx = &effects[fx_number];

	int old_x = fx->pos.x_pos,
		old_y = fx->pos.y_pos,
		old_z = fx->pos.z_pos;

	if (fx->speed < 384 && fx->flag1 == 0)
		fx->speed += (fx->speed >> 3) + 4;

	if (fx->flag1 == 1)
	{
		fx->fallspeed++;
		if (fx->speed > 8)
			fx->speed -= 2;
		if (fx->pos.x_rot > -0x3c00)
			fx->pos.x_rot -= 0x100;
	}

	int speed = (fx->speed * phd_cos(fx->pos.x_rot)) >> W2V_SHIFT;

	fx->pos.z_pos += (speed * phd_cos(fx->pos.y_rot)) >> W2V_SHIFT;
	fx->pos.x_pos += (speed * phd_sin(fx->pos.y_rot)) >> W2V_SHIFT;
	fx->pos.y_pos += -((fx->speed * phd_sin(fx->pos.x_rot)) >> W2V_SHIFT) + fx->fallspeed;

	if (wibble & 4)
		TriggerPlasmaBallFlame(fx_number, fx->flag1, 0, fx->flag1 == 0 ? (abs(old_y - fx->pos.y_pos) << 3) : 0, 0);

	auto room_number = fx->room_number;
	auto floor = GetFloor(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, &room_number);

	if (fx->pos.y_pos >= GetHeight(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos) ||
		fx->pos.y_pos < GetCeiling(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos))
	{
		if (fx->flag1 == 0)
		{
			PHD_VECTOR pos { old_x, old_y, old_z };

			for (int i = 0; i < (5 + (GetRandomControl() & 3)); ++i)
				TriggerPlasmaBall(nullptr, 1, &pos, fx->room_number, fx->pos.y_rot);
		}

		KillEffect(fx_number);

		return;
	}

	if (room[room_number].flags & UNDERWATER)
	{
		KillEffect(fx_number);
		return;
	}

	if (ItemNearLara(lara_item, &fx->pos, 200) && fx->flag1 == 0)
	{
		PHD_VECTOR pos { fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos };

		for (int i = 0; i < (3 + (GetRandomControl() & 1)); ++i)
			TriggerPlasmaBall(nullptr, 1, &pos, fx->room_number, fx->pos.y_rot);

		lara_item->hit_points -= CLAW_PLASMA_DAMAGE;
		lara_item->hit_status = 1;

		KillEffect(fx_number);

		return;
	}

	if (room_number != fx->room_number)
		EffectNewRoom(fx_number, lara_item->room_number);

	uint8_t radtab[2] = { 13, 7 };

	if (radtab[fx->flag1])
	{
		int rnd = GetRandomControl(),
			b = 31 - ((rnd >> 4) & 3),
			g = 24 - ((rnd >> 6) & 3),
			r = rnd & 7;

		TriggerDynamicLight(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, radtab[fx->flag1], r, g, b);
	}
}