#include "effect2.h"
#include "objects.h"
#include "lara.h"
#include "control.h"
#include "sphere.h"
#include "traps.h"
#include "people.h"
#include "game.h"

#include <specific/standard.h>
#include <specific/fn_stubs.h>

#define PUNK_HIT_DAMAGE		80
#define PUNK_SWIPE_DAMAGE	100
#define PUNK_WALK_TURN		(ONE_DEGREE * 5)
#define PUNK_RUN_TURN		(ONE_DEGREE * 6)
#define PUNK_ATTACK0_RANGE	SQUARE(WALL_L / 2)
#define PUNK_ATTACK1_RANGE	SQUARE(WALL_L)
#define PUNK_ATTACK2_RANGE	SQUARE(WALL_L * 5 / 4)
#define PUNK_WALK_RANGE		SQUARE(WALL_L)
#define PUNK_WALK_CHANCE	0x100
#define PUNK_WAIT_CHANCE	0x100
#define PUNK_DIE_ANIM		26
#define PUNK_STOP_ANIM		6
#define PUNK_CLIMB1_ANIM	28
#define PUNK_CLIMB2_ANIM	29
#define PUNK_CLIMB3_ANIM	27
#define PUNK_FALL3_ANIM		30
#define PUNK_TOUCH			0x2400
#define PUNK_VAULT_SHIFT	260
#define PUNK_AWARE_DISTANCE SQUARE(WALL_L)

enum punk_anims
{
	PUNK_EMPTY,
	PUNK_STOP,
	PUNK_WALK,
	PUNK_PUNCH2,
	PUNK_AIM2,
	PUNK_WAIT,
	PUNK_AIM1,
	PUNK_AIM0,
	PUNK_PUNCH1,
	PUNK_PUNCH0,
	PUNK_RUN,
	PUNK_DEATH,
	PUNK_CLIMB3,
	PUNK_CLIMB1,
	PUNK_CLIMB2,
	PUNK_FALL3
};

BITE_INFO punk_hit = { 16, 48, 320, 13 };

void TriggerPunkFlame(int16_t item_number, PHD_VECTOR* pos)
{
	int dx = lara_item->pos.x_pos - items[item_number].pos.x_pos,
		dz = lara_item->pos.z_pos - items[item_number].pos.z_pos;

	if (g_effects_draw_distance != -1 && (dx < -g_effects_draw_distance || dx > g_effects_draw_distance || dz < -g_effects_draw_distance || dz > g_effects_draw_distance))
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
	sptr->x = pos->x + ((GetRandomControl() & 15) - 8);
	sptr->y = pos->y;
	sptr->z = pos->z + ((GetRandomControl() & 15) - 8);
	sptr->Xvel = ((GetRandomControl() & 255) - 128);
	sptr->Yvel = -(GetRandomControl() & 15) - 16;
	sptr->Zvel = ((GetRandomControl() & 255) - 128);
	sptr->Friction = 5;

	if (GetRandomControl() & 1)
	{
		sptr->Gravity = -(GetRandomControl() & 31) - 16;
		sptr->MaxYvel = -(GetRandomControl() & 7) - 16;
		sptr->Flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
		sptr->RotAng = GetRandomControl() & 4095;
		sptr->RotAdd = ((GetRandomControl() & 1) ? -(GetRandomControl() & 15) - 16 : (GetRandomControl() & 15) + 16);
	}
	else
	{
		sptr->Flags = SP_SCALE | SP_DEF | SP_EXPDEF;
		sptr->Gravity = -(GetRandomControl() & 31) - 16;
		sptr->MaxYvel = -(GetRandomControl() & 7) - 16;
	}

	sptr->FxObj = item_number;
	sptr->NodeNumber = SPN_PUNKFLAME;
	sptr->Def = (PHDSPRITESTRUCT*)objects[EXPLOSION1].mesh_ptr;
	sptr->Scalar = 1;
	sptr->Width = sptr->sWidth = size;
	sptr->Height = sptr->sHeight = size;
	sptr->dWidth = size >> 2;
	sptr->dHeight = size >> 2;
}

void InitialisePunk(int16_t item_number)
{
	InitialiseCreature(item_number);

	auto item = &items[item_number];

	item->anim_number = objects[PUNK1].anim_index + PUNK_STOP_ANIM;
	item->frame_number = anims[item->anim_number].frame_base;
	item->current_anim_state = item->goal_anim_state = PUNK_STOP;
}

void PunkControl(int16_t item_number)
{
	PHD_VECTOR	pos1;
	int16_t angle, torso_y, torso_x, head, tilt;
	int32_t lara_dx, lara_dz, rnd;
	AI_INFO info, lara_info;

	if (!CreatureActive(item_number))
		return;

	auto item = &items[item_number];
	auto punk = (CREATURE_INFO*)item->data;

	torso_y = torso_x = head = angle = tilt = 0;

	if (!punk)
		return;

	if (item->item_flags[2])
	{
		rnd = GetRandomControl();
		pos1.x = punk_hit.x + (rnd & 15) - 8;
		pos1.y = punk_hit.y + ((rnd >> 4) & 15) - 8;
		pos1.z = punk_hit.z + ((rnd >> 8) & 15) - 8;

		GetJointAbsPosition(&items[item_number], &pos1, punk_hit.mesh_num);
		TriggerDynamicLight(pos1.x, pos1.y, pos1.z, 13, 31 - ((rnd >> 4) & 3), 24 - ((rnd >> 6) & 3), rnd & 7);
		TriggerPunkFlame(item_number, &pos1);
	}

	if (item->hit_points <= 0)
	{
		if (item->current_anim_state != PUNK_DEATH)
		{
			item->anim_number = objects[PUNK1].anim_index + PUNK_DIE_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = PUNK_DEATH;
			punk->LOT.step = STEP_L;
		}
	}
	else
	{
		if (item->ai_bits)
			GetAITarget(punk);
		else punk->enemy = lara_item;

		CreatureAIInfo(item, &info);

		if (punk->enemy == lara_item)
		{
			lara_info.angle = info.angle;
			lara_info.distance = info.distance;
		}
		else
		{
			lara_dz = lara_item->pos.z_pos - item->pos.z_pos;
			lara_dx = lara_item->pos.x_pos - item->pos.x_pos;
			lara_info.angle = phd_atan(lara_dz, lara_dx) - item->pos.y_rot; //only need to fill out the bits of lara_info that will be needed by TargetVisible
			lara_info.distance = lara_dz * lara_dz + lara_dx * lara_dx;
		}

		if (!punk->alerted && punk->enemy == lara_item)
			punk->enemy = nullptr;

		GetCreatureMood(item, &info, VIOLENT);

		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, punk->maximum_turn);

		auto real_enemy = punk->enemy;

		punk->enemy = lara_item;

		if (item->hit_status || ((lara_info.distance < PUNK_AWARE_DISTANCE || TargetVisible(item, &lara_info)) && abs(lara_item->pos.y_pos - item->pos.y_pos) < STEP_L * 5 && enable_punks_friendly && !(item->ai_bits & FOLLOW)))
		{
			if (!punk->alerted)
				g_audio->play_sound(299, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

			AlertAllGuards(item_number);
		}

		punk->enemy = real_enemy;

		switch (item->current_anim_state)
		{
		case PUNK_WAIT:
		{
			if (punk->alerted || item->goal_anim_state == PUNK_RUN)
			{
				item->goal_anim_state = PUNK_STOP;
				break;
			}
		}
		case PUNK_STOP:
		{
			punk->flags = 0;
			punk->maximum_turn = 0;

			head = lara_info.angle;

			if (item->ai_bits & GUARD)
			{
				head = AIGuard(punk);

				if (!(GetRandomControl() & 0xFF))
					item->goal_anim_state = (item->current_anim_state == PUNK_STOP ? PUNK_WAIT : PUNK_STOP);
			}
			else if (item->ai_bits & PATROL1)
				item->goal_anim_state = PUNK_WALK;
			else if (punk->mood == ESCAPE_MOOD)
				item->goal_anim_state = (lara.target != item && info.ahead && !item->hit_status ? PUNK_STOP : PUNK_RUN);
			else if (punk->mood == BORED_MOOD || ((item->ai_bits & FOLLOW) && (punk->reached_goal || lara_info.distance > SQUARE(WALL_L * 2))))
			{
				if (item->required_anim_state)
					item->goal_anim_state = item->required_anim_state;
				else if (info.ahead)
					item->goal_anim_state = PUNK_STOP;
				else item->goal_anim_state = PUNK_RUN;
			}
			else if (info.bite && info.distance < PUNK_ATTACK0_RANGE)
				item->goal_anim_state = PUNK_AIM0;
			else if (info.bite && info.distance < PUNK_ATTACK1_RANGE)
				item->goal_anim_state = PUNK_AIM1;
			else if (info.bite && info.distance < PUNK_WALK_RANGE)
				item->goal_anim_state = PUNK_WALK;
			else item->goal_anim_state = PUNK_RUN;

			break;
		}
		case PUNK_WALK:
		{
			punk->maximum_turn = PUNK_WALK_TURN;

			head = lara_info.angle;

			if (item->ai_bits & PATROL1)
			{
				item->goal_anim_state = PUNK_WALK;
				head = 0;
			}
			else if (punk->mood == ESCAPE_MOOD)
				item->goal_anim_state = PUNK_RUN;
			else if (punk->mood == BORED_MOOD)
			{
				if (GetRandomControl() < PUNK_WAIT_CHANCE)
				{
					item->required_anim_state = PUNK_WAIT;
					item->goal_anim_state = PUNK_STOP;
				}
			}
			else if (info.bite && info.distance < PUNK_ATTACK0_RANGE)
				item->goal_anim_state = PUNK_STOP;
			else if (info.bite && info.distance < PUNK_ATTACK2_RANGE)
				item->goal_anim_state = PUNK_AIM2;
			else item->goal_anim_state = PUNK_RUN;

			break;
		}
		case PUNK_RUN:
		{
			punk->maximum_turn = PUNK_RUN_TURN;

			if (info.ahead)
				head = info.angle;

			tilt = angle / 2;

			if (item->ai_bits & GUARD)
				item->goal_anim_state = PUNK_WAIT;
			else if (punk->mood == ESCAPE_MOOD)
			{
				if (lara.target != item && info.ahead)
					item->goal_anim_state = PUNK_STOP;
			}
			else if ((item->ai_bits & FOLLOW) && (punk->reached_goal || lara_info.distance > SQUARE(WALL_L * 2)))
				item->goal_anim_state = PUNK_STOP;
			else if (punk->mood == BORED_MOOD)
				item->goal_anim_state = PUNK_WALK;
			else if (info.ahead && info.distance < PUNK_WALK_RANGE)
				item->goal_anim_state = PUNK_WALK;

			break;
		}
		case PUNK_AIM0:
		{
			punk->maximum_turn = PUNK_WALK_TURN;
			punk->flags = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			item->goal_anim_state = (info.bite && info.distance < PUNK_ATTACK0_RANGE ? PUNK_PUNCH0 : PUNK_STOP);

			break;
		}
		case PUNK_AIM1:
		{
			punk->maximum_turn = PUNK_WALK_TURN;
			punk->flags = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			item->goal_anim_state = (info.ahead && info.distance < PUNK_ATTACK1_RANGE ? PUNK_PUNCH1 : PUNK_STOP);

			break;
		}
		case PUNK_AIM2:
		{
			punk->maximum_turn = PUNK_WALK_TURN;
			punk->flags = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			item->goal_anim_state = (info.bite && info.distance < PUNK_ATTACK2_RANGE ? PUNK_PUNCH2 : PUNK_WALK);

			break;
		}
		case PUNK_PUNCH0:
		{
			punk->maximum_turn = PUNK_WALK_TURN;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			if (!punk->flags && (item->touch_bits & PUNK_TOUCH))
			{
				lara_item->hit_points -= PUNK_HIT_DAMAGE;
				lara_item->hit_status = 1;

				CreatureEffect(item, &punk_hit, DoBloodSplat);

				g_audio->play_sound(70, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

				if (item->item_flags[2] == 1)
					LaraBurn();
				else if (item->item_flags[2])
					--item->item_flags[2];

				punk->flags = 1;
			}

			break;
		}
		case PUNK_PUNCH1:
		{
			punk->maximum_turn = PUNK_WALK_TURN;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			if (!punk->flags && (item->touch_bits & PUNK_TOUCH))
			{
				lara_item->hit_points -= PUNK_HIT_DAMAGE;
				lara_item->hit_status = 1;

				CreatureEffect(item, &punk_hit, DoBloodSplat);

				g_audio->play_sound(70, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

				if (item->item_flags[2] == 1)
					LaraBurn();
				else if (item->item_flags[2])
					--item->item_flags[2];

				punk->flags = 1;
			}

			if (info.ahead && info.distance > PUNK_ATTACK1_RANGE && info.distance < PUNK_ATTACK2_RANGE)
				item->goal_anim_state = PUNK_PUNCH2;

			break;
		}
		case PUNK_PUNCH2:
		{
			punk->maximum_turn = PUNK_WALK_TURN;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			if (punk->flags != 2 && (item->touch_bits & PUNK_TOUCH))
			{
				lara_item->hit_points -= PUNK_SWIPE_DAMAGE;
				lara_item->hit_status = 1;

				CreatureEffect(item, &punk_hit, DoBloodSplat);

				g_audio->play_sound(70, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

				if (item->item_flags[2] == 1)
					LaraBurn();
				else if (item->item_flags[2])
					--item->item_flags[2];

				punk->flags = 2;
			}
		}
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso_y);
	CreatureJoint(item, 1, torso_x);
	CreatureJoint(item, 2, head);

	if (item->current_anim_state < PUNK_DEATH)
	{
		switch (CreatureVault(item_number, angle, 2, PUNK_VAULT_SHIFT))
		{
		case 2:
			punk->maximum_turn = 0;
			item->anim_number = objects[PUNK1].anim_index + PUNK_CLIMB1_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = PUNK_CLIMB1;
			break;

		case 3:
			punk->maximum_turn = 0;
			item->anim_number = objects[PUNK1].anim_index + PUNK_CLIMB2_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = PUNK_CLIMB2;
			break;

		case 4:
			punk->maximum_turn = 0;
			item->anim_number = objects[PUNK1].anim_index + PUNK_CLIMB3_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = PUNK_CLIMB3;
			break;
		case -4:
			punk->maximum_turn = 0;
			item->anim_number = objects[PUNK1].anim_index + PUNK_FALL3_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = PUNK_FALL3;
			break;
		}
	}
	else
	{
		punk->maximum_turn = 0;
		CreatureAnimation(item_number, angle, 0);
	}
}