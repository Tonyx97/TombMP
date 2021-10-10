#include <specific/stypes.h>
#include <specific/standard.h>

#include "objects.h"
#include "lara.h"
#include "laraanim.h"
#include "control.h"
#include "lot.h"
#include "people.h"

#include <audio/audio_system.h>

#define BATON_HIT_DAMAGE		80
#define BATON_SWIPE_DAMAGE		100
#define BATON_KICK_DAMAGE		150
#define BATON_HIT_RADIUS		(STEP_L)
#define BATON_WALK_TURN			(ONE_DEGREE * 6)
#define BATON_RUN_TURN			(ONE_DEGREE * 7)
#define BATON_ATTACK0_RANGE		SQUARE(WALL_L / 2)
#define BATON_ATTACK1_RANGE		SQUARE(WALL_L)
#define BATON_ATTACK2_RANGE		SQUARE(WALL_L * 5 / 4)
#define BATON_KICK_RANGE		SQUARE(WALL_L * 3 / 2)
#define BATON_WALK_RANGE		SQUARE(WALL_L)
#define BATON_WALK_CHANCE		0x100
#define BATON_WAIT_CHANCE		0x100
#define BATON_DIE_ANIM			26
#define BATON_STOP_ANIM			6
#define BATON_CLIMB1_ANIM		28
#define BATON_CLIMB2_ANIM		29
#define BATON_CLIMB3_ANIM		27
#define BATON_FALL3_ANIM		30
#define BATON_TOUCH				0x2400
#define BATON_KICK_TOUCH		0x60
#define BATON_VAULT_SHIFT		260
#define BATON_AWARE_DISTANCE	SQUARE(WALL_L)

enum baton_anims
{
	BATON_EMPTY,
	BATON_STOP,
	BATON_WALK,
	BATON_PUNCH2,
	BATON_AIM2,
	BATON_WAIT,
	BATON_AIM1,
	BATON_AIM0,
	BATON_PUNCH1,
	BATON_PUNCH0,
	BATON_RUN,
	BATON_DEATH,
	BATON_KICK,
	BATON_CLIMB3,
	BATON_CLIMB1,
	BATON_CLIMB2,
	BATON_FALL3
};

BITE_INFO baton_hit = { 247, 10, 11, 13 },
		  baton_kick = { 0, 0, 100, 6 };

void InitialiseBaton(int16_t item_number)
{
	auto item = &items[item_number];

	InitialiseCreature(item_number);

	item->anim_number = objects[MP1].anim_index + BATON_STOP_ANIM;
	item->frame_number = anims[item->anim_number].frame_base;
	item->current_anim_state = item->goal_anim_state = BATON_STOP;
}

void BatonControl(int16_t item_number)
{
	if (!CreatureActive(item_number))
		return;

	auto item = &items[item_number];
	auto baton = (CREATURE_INFO*)item->data;

	if (!baton)
		return;

	int16_t torso_y = 0,
		   torso_x = 0,
		   head = 0,
		   angle = 0,
		   tilt = 0;

	if (boxes[item->box_number].overlap_index & BLOCKED)
	{
		DoLotsOfBloodD(item->pos.x_pos, item->pos.y_pos - (GetRandomControl() & 255) - 32, item->pos.z_pos, (GetRandomControl() & 127) + 128, GetRandomControl() << 1, item->room_number, 3);

		item->hit_points -= 20;
	}

	if (item->hit_points <= 0)
	{
		if (item->current_anim_state != BATON_DEATH)
		{
			item->anim_number = objects[MP1].anim_index + BATON_DIE_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = BATON_DEATH;

			baton->LOT.step = STEP_L;
		}
	}
	else
	{
		AI_INFO lara_info;

		if (item->ai_bits)
			GetAITarget(baton);
		else
		{
			int lara_dz = lara_item->pos.z_pos - item->pos.z_pos,
				lara_dx = lara_item->pos.x_pos - item->pos.x_pos,
				best_distance = 0x7fffffff;

			lara_info.distance = lara_dz * lara_dz + lara_dx * lara_dx;

			baton->enemy = lara_item;

			auto cinfo = baddie_slots;

			for (int slot = 0; slot < NUM_SLOTS; ++slot, ++cinfo)
			{
				if (cinfo->item_num == NO_ITEM || cinfo->item_num == item_number)
					continue;

				auto target = &items[cinfo->item_num];
				if (target->object_number != LARA && target->object_number != BOB)
					continue;

				int x = target->pos.x_pos - item->pos.x_pos,
					z = target->pos.z_pos - item->pos.z_pos;

				if (z > 32000 || z < -32000 || x > 32000 || x < -32000)
					continue;

				if (int distance = x * x + z * z; distance < best_distance && distance < lara_info.distance)
				{
					baton->enemy = target;
					best_distance = distance;
				}
			}
		}

		AI_INFO info;

		CreatureAIInfo(item, &info);

		if (baton->enemy == lara_item)
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

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, baton->maximum_turn);

		auto enemy = baton->enemy;

		baton->enemy = lara_item;

		if (item->hit_status || ((lara_info.distance < BATON_AWARE_DISTANCE || TargetVisible(item, &lara_info)) && (ABS(lara_item->pos.y_pos - item->pos.y_pos) < WALL_L)))
		{
			if (!baton->alerted)
				g_audio->play_sound(300, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

			AlertAllGuards(item_number);
		}

		baton->enemy = enemy;

		switch (item->current_anim_state)
		{
		case BATON_WAIT:
		{
			if (baton->alerted || item->goal_anim_state == BATON_RUN)
			{
				item->goal_anim_state = BATON_STOP;
				break;
			}
		}
		case BATON_STOP:
		{
			baton->flags = 0;
			baton->maximum_turn = 0;

			head = lara_info.angle;

			if (item->ai_bits & GUARD)
			{
				head = AIGuard(baton);

				if (!(GetRandomControl() & 0xFF))
					item->goal_anim_state = (item->current_anim_state == BATON_STOP ? BATON_WAIT : BATON_STOP);

				break;
			}

			else if (item->ai_bits & PATROL1)
				item->goal_anim_state = BATON_WALK;
			else if (baton->mood == ESCAPE_MOOD)
				item->goal_anim_state = (lara.target != item && info.ahead && !item->hit_status) ? BATON_STOP : BATON_RUN;
			else if (baton->mood == BORED_MOOD || ((item->ai_bits & FOLLOW) && (baton->reached_goal || lara_info.distance > SQUARE(WALL_L * 2))))
			{
				if (item->required_anim_state)	item->goal_anim_state = item->required_anim_state;
				else if (info.ahead)			item->goal_anim_state = BATON_STOP;
				else							item->goal_anim_state = BATON_RUN;
			}
			else if (info.bite && info.distance < BATON_ATTACK0_RANGE)
				item->goal_anim_state = BATON_AIM0;
			else if (info.bite && info.distance < BATON_ATTACK1_RANGE)
				item->goal_anim_state = BATON_AIM1;
			else if (info.bite && info.distance < BATON_WALK_RANGE)
				item->goal_anim_state = BATON_WALK;
			else item->goal_anim_state = BATON_RUN;

			break;
		}
		case BATON_WALK:
		{
			head = lara_info.angle;
			baton->flags = 0;

			baton->maximum_turn = BATON_WALK_TURN;

			if (item->ai_bits & PATROL1)
			{
				item->goal_anim_state = BATON_WALK;
				head = 0;
			}
			else if (baton->mood == ESCAPE_MOOD)
				item->goal_anim_state = BATON_RUN;
			else if (baton->mood == BORED_MOOD)
			{
				if (GetRandomControl() < BATON_WAIT_CHANCE)
				{
					item->required_anim_state = BATON_WAIT;
					item->goal_anim_state = BATON_STOP;
				}
			}
			else if (info.bite && info.distance < BATON_KICK_RANGE && info.x_angle < 0)
				item->goal_anim_state = BATON_KICK;
			else if (info.bite && info.distance < BATON_ATTACK0_RANGE)
				item->goal_anim_state = BATON_STOP;
			else if (info.bite && info.distance < BATON_ATTACK2_RANGE)
				item->goal_anim_state = BATON_AIM2;
			else item->goal_anim_state = BATON_RUN;

			break;
		}
		case BATON_RUN:
		{
			if (info.ahead)
				head = info.angle;

			baton->maximum_turn = BATON_RUN_TURN;
			tilt = angle / 2;

			if (item->ai_bits & GUARD)
				item->goal_anim_state = BATON_WAIT;
			else if (baton->mood == ESCAPE_MOOD)
			{
				if (lara.target != item && info.ahead)
					item->goal_anim_state = BATON_STOP;

				break;
			}
			else if ((item->ai_bits & FOLLOW) && (baton->reached_goal || lara_info.distance > SQUARE(WALL_L * 2)))
				item->goal_anim_state = BATON_STOP;
			else if (baton->mood == BORED_MOOD)
				item->goal_anim_state = BATON_WALK;
			else if (info.ahead && info.distance < BATON_WALK_RANGE)
				item->goal_anim_state = BATON_WALK;

			break;
		}
		case BATON_AIM0:
		{
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			baton->maximum_turn = BATON_WALK_TURN;
			baton->flags = 0;

			item->goal_anim_state = (info.bite && info.distance < BATON_ATTACK0_RANGE) ? BATON_PUNCH0 : BATON_STOP;

			break;
		}
		case BATON_AIM1:
		{
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			baton->maximum_turn = BATON_WALK_TURN;
			baton->flags = 0;

			item->goal_anim_state = (info.ahead && info.distance < BATON_ATTACK1_RANGE) ? BATON_PUNCH1 : BATON_STOP;

			break;
		}
		case BATON_AIM2:
		{
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			baton->maximum_turn = BATON_WALK_TURN;
			baton->flags = 0;

			item->goal_anim_state = (info.bite && info.distance < BATON_ATTACK2_RANGE ? BATON_PUNCH2 : BATON_WALK);

			break;
		}
		case BATON_PUNCH0:
		{
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			baton->maximum_turn = BATON_WALK_TURN;

			if (enemy == lara_item)
			{
				if (!baton->flags && (item->touch_bits & BATON_TOUCH))
				{
					lara_item->hit_points -= BATON_HIT_DAMAGE;
					lara_item->hit_status = 1;

					CreatureEffect(item, &baton_hit, DoBloodSplat);
					g_audio->play_sound(70, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

					baton->flags = 1;
				}
			}
			else
			{
				if (!baton->flags && enemy)
				{
					if (ABS(enemy->pos.x_pos - item->pos.x_pos) < BATON_HIT_RADIUS &&
						ABS(enemy->pos.y_pos - item->pos.y_pos) <= BATON_HIT_RADIUS &&
						ABS(enemy->pos.z_pos - item->pos.z_pos) < BATON_HIT_RADIUS)
					{
						enemy->hit_points -= BATON_HIT_DAMAGE >> 4;
						enemy->hit_status = 1;
						baton->flags = 1;

						CreatureEffect(item, &baton_hit, DoBloodSplat);
						g_audio->play_sound(70, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
					}
				}
			}

			break;
		}
		case BATON_PUNCH1:
		{
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			baton->maximum_turn = BATON_WALK_TURN;

			if (enemy == lara_item)
			{
				if (!baton->flags && (item->touch_bits & BATON_TOUCH))
				{
					lara_item->hit_points -= BATON_HIT_DAMAGE;
					lara_item->hit_status = 1;

					CreatureEffect(item, &baton_hit, DoBloodSplat);
					g_audio->play_sound(70, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

					baton->flags = 1;
				}
			}
			else
			{
				if (!baton->flags && enemy)
				{
					if (ABS(enemy->pos.x_pos - item->pos.x_pos) < BATON_HIT_RADIUS &&
						ABS(enemy->pos.y_pos - item->pos.y_pos) <= BATON_HIT_RADIUS &&
						ABS(enemy->pos.z_pos - item->pos.z_pos) < BATON_HIT_RADIUS)
					{
						enemy->hit_points -= BATON_HIT_DAMAGE >> 4;
						enemy->hit_status = 1;
						baton->flags = 1;

						CreatureEffect(item, &baton_hit, DoBloodSplat);
						g_audio->play_sound(70, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
					}
				}
			}

			if (info.ahead && info.distance > BATON_ATTACK1_RANGE && info.distance < BATON_ATTACK2_RANGE)
				item->goal_anim_state = BATON_PUNCH2;

			break;
		}
		case BATON_PUNCH2:
		{
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}
			baton->maximum_turn = BATON_WALK_TURN;

			if (enemy == lara_item)
			{
				if (baton->flags != 2 && (item->touch_bits & BATON_TOUCH))
				{
					lara_item->hit_points -= BATON_SWIPE_DAMAGE;
					lara_item->hit_status = 1;

					CreatureEffect(item, &baton_hit, DoBloodSplat);
					g_audio->play_sound(70, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

					baton->flags = 2;
				}
			}
			else
			{
				if (baton->flags != 2 && enemy)
				{
					if (ABS(enemy->pos.x_pos - item->pos.x_pos) < BATON_HIT_RADIUS &&
						ABS(enemy->pos.y_pos - item->pos.y_pos) <= BATON_HIT_RADIUS &&
						ABS(enemy->pos.z_pos - item->pos.z_pos) < BATON_HIT_RADIUS)
					{
						enemy->hit_points -= BATON_SWIPE_DAMAGE >> 4;
						enemy->hit_status = 1;
						baton->flags = 2;

						CreatureEffect(item, &baton_hit, DoBloodSplat);
						g_audio->play_sound(70, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
					}
				}
			}

			break;
		}
		case BATON_KICK:
		{
			if (info.ahead)
				torso_y = info.angle;

			baton->maximum_turn = BATON_WALK_TURN;

			if (enemy == lara_item)
			{
				if (baton->flags != 1 && (item->touch_bits & BATON_KICK_TOUCH) && (item->frame_number > anims[item->anim_number].frame_base + 8))
				{
					lara_item->hit_points -= BATON_KICK_DAMAGE;
					lara_item->hit_status = 1;

					CreatureEffect(item, &baton_kick, DoBloodSplat);
					g_audio->play_sound(70, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

					baton->flags = 1;
				}
			}
			else
			{
				if (!baton->flags != 1 && enemy && (item->frame_number > anims[item->anim_number].frame_base + 8))
				{
					if (ABS(enemy->pos.x_pos - item->pos.x_pos) < BATON_HIT_RADIUS &&
						ABS(enemy->pos.y_pos - item->pos.y_pos) <= BATON_HIT_RADIUS &&
						ABS(enemy->pos.z_pos - item->pos.z_pos) < BATON_HIT_RADIUS)
					{
						enemy->hit_points -= BATON_KICK_DAMAGE >> 4;
						enemy->hit_status = 1;
						baton->flags = 1;

						CreatureEffect(item, &baton_kick, DoBloodSplat);
						g_audio->play_sound(70, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
					}
				}
			}

			break;
		}
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso_y);
	CreatureJoint(item, 1, torso_x);
	CreatureJoint(item, 2, head);

	if (item->current_anim_state < BATON_DEATH)
	{
		switch (CreatureVault(item_number, angle, 2, BATON_VAULT_SHIFT))
		{
		case 2:
			baton->maximum_turn = 0;
			item->anim_number = objects[MP1].anim_index + BATON_CLIMB1_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = BATON_CLIMB1;
			break;
		case 3:
			baton->maximum_turn = 0;
			item->anim_number = objects[MP1].anim_index + BATON_CLIMB2_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = BATON_CLIMB2;
			break;
		case 4:
			baton->maximum_turn = 0;
			item->anim_number = objects[MP1].anim_index + BATON_CLIMB3_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = BATON_CLIMB3;
			break;
		case -4:
			baton->maximum_turn = 0;
			item->anim_number = objects[MP1].anim_index + BATON_FALL3_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = BATON_FALL3;
			break;
		}
	}
	else
	{
		baton->maximum_turn = 0;

		CreatureAnimation(item_number, angle, 0);
	}
}