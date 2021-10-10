#include <specific/standard.h>

#include "objects.h"
#include "lara.h"
#include "control.h"
#include "lot.h"

#define BOB_HIT_DAMAGE		40
#define BOB_SWIPE_DAMAGE	50
#define BOB_WALK_TURN		(ONE_DEGREE * 7)
#define BOB_RUN_TURN		(ONE_DEGREE * 11)
#define BOB_ATTACK0_RANGE	SQUARE(WALL_L / 3)
#define BOB_ATTACK1_RANGE	SQUARE(WALL_L * 2 / 3)
#define BOB_ATTACK2_RANGE	SQUARE(WALL_L * 3 / 4)
#define BOB_WALK_RANGE		SQUARE(WALL_L)
#define BOB_WALK_CHANCE		0x100
#define BOB_WAIT_CHANCE		0x100
#define BOB_DIE_ANIM		26
#define BOB_STOP_ANIM		6
#define BOB_CLIMB1_ANIM		28
#define BOB_CLIMB2_ANIM		29
#define BOB_CLIMB3_ANIM		27
#define BOB_FALL3_ANIM		30
#define BOB_TOUCH			0x2400
#define BOB_VAULT_SHIFT		260
#define BOB_AWARE_DISTANCE	SQUARE(WALL_L)
#define BOB_HIT_RADIUS		(STEP_L)

enum bob_anims
{
	BOB_EMPTY,
	BOB_STOP,
	BOB_WALK,
	BOB_PUNCH2,
	BOB_AIM2,
	BOB_WAIT,
	BOB_AIM1,
	BOB_AIM0,
	BOB_PUNCH1,
	BOB_PUNCH0,
	BOB_RUN,
	BOB_DEATH,
	BOB_CLIMB3,
	BOB_CLIMB1,
	BOB_CLIMB2,
	BOB_FALL3
};

BITE_INFO bob_hit = { 10, 10, 11, 13 };

void InitialisePrisoner(int16_t item_number)
{
	InitialiseCreature(item_number);

	auto item = &items[item_number];

	item->anim_number = objects[BOB].anim_index + BOB_STOP_ANIM;
	item->frame_number = anims[item->anim_number].frame_base;
	item->current_anim_state = item->goal_anim_state = BOB_STOP;
}

void PrisonerControl(int16_t item_number)
{
	if (!CreatureActive(item_number))
		return;

	auto item = &items[item_number];
	auto bob = (CREATURE_INFO*)item->data;

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
	
	if (!bob)
		return;

	if (item->hit_points <= 0)
	{
		if (item->current_anim_state != BOB_DEATH)
		{
			item->anim_number = objects[BOB].anim_index + BOB_DIE_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = BOB_DEATH;
			bob->LOT.step = STEP_L;
		}
	}
	else
	{
		if (item->ai_bits && item->ai_bits != MODIFY)
			GetAITarget(bob);
		else if (bob->hurt_by_lara)
			bob->enemy = lara_item;
		else
		{
			bob->enemy = nullptr;

			int best_distance = 0x7fffffff;

			auto cinfo = baddie_slots;

			for (int i = 0; i < NUM_SLOTS; ++i, ++cinfo)
			{
				if (cinfo->item_num == NO_ITEM || cinfo->item_num == item_number)
					continue;

				auto target = &items[cinfo->item_num];

				if (target->object_number == LARA || target->object_number == BOB || target->object_number == ROBOT_SENTRY_GUN || target->hit_points <= 0)
					continue;

				int x = target->pos.x_pos - item->pos.x_pos,
					z = target->pos.z_pos - item->pos.z_pos;

				if (z > 32000 || z < -32000 || x>32000 || x < -32000)
					continue;
				
				if (int distance = x * x + z * z; distance < best_distance)
				{
					bob->enemy = target;
					best_distance = distance;
				}
			}
		}

		if (item->ai_bits == MODIFY)
			item->hit_points = 200;

		AI_INFO info,
				lara_info;

		CreatureAIInfo(item, &info);

		if (!bob->hurt_by_lara && bob->enemy == lara_item)
			bob->enemy = nullptr;

		if (bob->enemy == lara_item)
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

		angle = CreatureTurn(item, bob->maximum_turn);

		if (bob->hurt_by_lara)
		{
			if (!bob->alerted)
				g_audio->play_sound(300, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

			AlertAllGuards(item_number);
		}

		auto enemy = bob->enemy;

		switch (item->current_anim_state)
		{
		case BOB_WAIT:
		{
			if (bob->alerted || item->goal_anim_state == BOB_RUN)
			{
				item->goal_anim_state = BOB_STOP;
				break;
			}
		}
		case BOB_STOP:
		{
			bob->flags = 0;
			bob->maximum_turn = 0;

			head = lara_info.angle;

			if (item->ai_bits & GUARD)
			{
				head = AIGuard(bob);

				if (!(GetRandomControl() & 0xFF))
					item->goal_anim_state = (item->current_anim_state == BOB_STOP ? BOB_WAIT : BOB_STOP);
			}
			else if (item->ai_bits & PATROL1)
				item->goal_anim_state = BOB_WALK;
			else if (bob->mood == ESCAPE_MOOD)
				item->goal_anim_state = (lara.target != item && info.ahead && !item->hit_status ? BOB_STOP : BOB_RUN);
			else if (bob->mood == BORED_MOOD || ((item->ai_bits & FOLLOW) && (bob->reached_goal || lara_info.distance > SQUARE(WALL_L * 2))))
			{
				if (item->required_anim_state)
					item->goal_anim_state = item->required_anim_state;
				else if (info.ahead)
					item->goal_anim_state = BOB_STOP;
				else item->goal_anim_state = BOB_RUN;
			}
			else if (info.bite && info.distance < BOB_ATTACK0_RANGE)
				item->goal_anim_state = BOB_AIM0;
			else if (info.bite && info.distance < BOB_ATTACK1_RANGE)
				item->goal_anim_state = BOB_AIM1;
			else if (info.bite && info.distance < BOB_WALK_RANGE)
				item->goal_anim_state = BOB_WALK;
			else item->goal_anim_state = BOB_RUN;

			break;
		}
		case BOB_WALK:
		{
			bob->maximum_turn = BOB_WALK_TURN;

			head = lara_info.angle;

			if (item->ai_bits & PATROL1)
			{
				item->goal_anim_state = BOB_WALK;
				head = 0;
			}
			else if (bob->mood == ESCAPE_MOOD)
				item->goal_anim_state = BOB_RUN;
			else if (bob->mood == BORED_MOOD)
			{
				if (GetRandomControl() < BOB_WAIT_CHANCE)
				{
					item->required_anim_state = BOB_WAIT;
					item->goal_anim_state = BOB_STOP;
				}
			}
			else if (info.bite && info.distance < BOB_ATTACK0_RANGE)
				item->goal_anim_state = BOB_STOP;
			else if (info.bite && info.distance < BOB_ATTACK2_RANGE)
				item->goal_anim_state = BOB_AIM2;
			else item->goal_anim_state = BOB_RUN;

			break;
		}
		case BOB_RUN:
		{
			bob->maximum_turn = BOB_RUN_TURN;

			if (info.ahead)
				head = info.angle;

			tilt = angle / 2;

			if (item->ai_bits & GUARD)
				item->goal_anim_state = BOB_STOP;
			else if (bob->mood == ESCAPE_MOOD)
			{
				if (lara.target != item && info.ahead)
					item->goal_anim_state = BOB_STOP;
			}
			else if ((item->ai_bits & FOLLOW) && (bob->reached_goal || lara_info.distance > SQUARE(WALL_L * 2)))
				item->goal_anim_state = BOB_STOP;
			else if (bob->mood == BORED_MOOD)
				item->goal_anim_state = BOB_WALK;
			else if (info.ahead && info.distance < BOB_WALK_RANGE)
				item->goal_anim_state = BOB_WALK;

			break;
		}
		case BOB_AIM0:
		{
			bob->maximum_turn = BOB_WALK_TURN;
			bob->flags = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			item->goal_anim_state = (info.bite && info.distance < BOB_ATTACK0_RANGE ? BOB_PUNCH0 : BOB_STOP);

			break;
		}
		case BOB_AIM1:
		{
			bob->maximum_turn = BOB_WALK_TURN;
			bob->flags = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			item->goal_anim_state = (info.ahead && info.distance < BOB_ATTACK1_RANGE ? BOB_PUNCH1 : BOB_STOP);

			break;
		}
		case BOB_AIM2:
		{
			bob->maximum_turn = BOB_WALK_TURN;
			bob->flags = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			item->goal_anim_state = (info.bite && info.distance < BOB_ATTACK2_RANGE ? BOB_PUNCH2 : BOB_WALK);

			break;
		}
		case BOB_PUNCH0:
		{
			bob->maximum_turn = BOB_WALK_TURN;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			if (enemy == lara_item)
			{
				if (!bob->flags && (item->touch_bits & BOB_TOUCH))
				{
					lara_item->hit_points -= BOB_HIT_DAMAGE;
					lara_item->hit_status = 1;

					CreatureEffect(item, &bob_hit, DoBloodSplat);
					g_audio->play_sound(70, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

					bob->flags = 1;
				}
			}
			else
			{
				if (!bob->flags && enemy)
				{
					if (ABS(enemy->pos.x_pos - item->pos.x_pos) < BOB_HIT_RADIUS &&
						ABS(enemy->pos.y_pos - item->pos.y_pos) <= BOB_HIT_RADIUS &&
						ABS(enemy->pos.z_pos - item->pos.z_pos) < BOB_HIT_RADIUS)
					{
						enemy->hit_points -= BOB_HIT_DAMAGE >> 1;
						enemy->hit_status = 1;

						bob->flags = 1;

						g_audio->play_sound(70, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
						CreatureEffect(item, &bob_hit, DoBloodSplat);
					}
				}
			}

			break;
		}
		case BOB_PUNCH1:
		{
			bob->maximum_turn = BOB_WALK_TURN;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			if (enemy == lara_item)
			{
				if (!bob->flags && (item->touch_bits & BOB_TOUCH))
				{
					lara_item->hit_points -= BOB_HIT_DAMAGE;
					lara_item->hit_status = 1;

					bob->flags = 1;

					CreatureEffect(item, &bob_hit, DoBloodSplat);
					g_audio->play_sound(70, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
				}
			}
			else
			{
				if (!bob->flags && enemy)
				{
					if (ABS(enemy->pos.x_pos - item->pos.x_pos) < BOB_HIT_RADIUS &&
						ABS(enemy->pos.y_pos - item->pos.y_pos) <= BOB_HIT_RADIUS &&
						ABS(enemy->pos.z_pos - item->pos.z_pos) < BOB_HIT_RADIUS)
					{
						enemy->hit_points -= BOB_HIT_DAMAGE >> 1;
						enemy->hit_status = 1;

						bob->flags = 1;

						CreatureEffect(item, &bob_hit, DoBloodSplat);
						g_audio->play_sound(70, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
					}
				}
			}

			if (info.ahead && info.distance > BOB_ATTACK1_RANGE && info.distance < BOB_ATTACK2_RANGE)
				item->goal_anim_state = BOB_PUNCH2;

			break;
		}
		case BOB_PUNCH2:
		{
			bob->maximum_turn = BOB_WALK_TURN;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			if (enemy == lara_item)
			{
				if (bob->flags != 2 && (item->touch_bits & BOB_TOUCH))
				{
					lara_item->hit_points -= BOB_SWIPE_DAMAGE;
					lara_item->hit_status = 1;

					bob->flags = 2;

					CreatureEffect(item, &bob_hit, DoBloodSplat);
					g_audio->play_sound(70, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
				}
			}
			else
			{
				if (bob->flags != 2 && enemy)
				{
					if (ABS(enemy->pos.x_pos - item->pos.x_pos) < BOB_HIT_RADIUS &&
						ABS(enemy->pos.y_pos - item->pos.y_pos) <= BOB_HIT_RADIUS &&
						ABS(enemy->pos.z_pos - item->pos.z_pos) < BOB_HIT_RADIUS)
					{
						enemy->hit_points -= BOB_SWIPE_DAMAGE >> 1;
						enemy->hit_status = 1;

						bob->flags = 2;

						g_audio->play_sound(70, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
						CreatureEffect(item, &bob_hit, DoBloodSplat);
					}
				}
			}
		}
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso_y);
	CreatureJoint(item, 1, torso_x);
	CreatureJoint(item, 2, head);

	if (item->current_anim_state < BOB_DEATH)
	{
		switch (CreatureVault(item_number, angle, 2, BOB_VAULT_SHIFT))
		{
		case 2:
			bob->maximum_turn = 0;
			item->anim_number = objects[BOB].anim_index + BOB_CLIMB1_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = BOB_CLIMB1;
			break;
		case 3:
			bob->maximum_turn = 0;
			item->anim_number = objects[BOB].anim_index + BOB_CLIMB2_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = BOB_CLIMB2;
			break;
		case 4:
			bob->maximum_turn = 0;
			item->anim_number = objects[BOB].anim_index + BOB_CLIMB3_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = BOB_CLIMB3;
			break;
		case -4:
			bob->maximum_turn = 0;
			item->anim_number = objects[BOB].anim_index + BOB_FALL3_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = BOB_FALL3;
			break;
		}
	}
	else
	{
		bob->maximum_turn = 0;
		CreatureAnimation(item_number, angle, 0);
	}
}