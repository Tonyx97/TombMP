#include "lara.h"
#include "people.h"

#define HYBRID_SLASH_DAMAGE		100
#define HYBRID_KICK_DAMAGE		80
#define HYBRID_JUMP_DAMAGE		20
#define HYBRID_SLASH_TOUCH		(0x80)
#define HYBRID_KICK_TOUCH		(0x800)
#define HYBRID_DIE_ANIM			18
#define HYBRID_WALK_TURN		(3 * ONE_DEGREE)
#define HYBRID_RUN_TURN			(6 * ONE_DEGREE)
#define HYBRID_ATTACK1_RANGE	SQUARE(WALL_L)
#define HYBRID_ATTACK2_RANGE	SQUARE(WALL_L * 2)
#define HYBRID_ATTACK3_RANGE	SQUARE(WALL_L * 4 / 3)
#define HYBRID_FIRE_RANGE		SQUARE(WALL_L * 3)
#define HYBRID_ROAR_CHANCE		0x60
#define HYBRID_WALK_CHANCE		(HYBRID_ROAR_CHANCE + 0x400)
#define HYBRID_AWARE_DISTANCE	SQUARE(WALL_L)

enum hybrid_anims
{
	HYBRID_EMPTY,
	HYBRID_STOP,
	HYBRID_WALK,
	HYBRID_RUN,
	HYBRID_JUMP_STR,
	HYBRID_JUMP_MID,
	HYBRID_JUMP_END,
	HYBRID_SLASH_LEFT,
	HYBRID_KICK,
	HYBRID_RUN_ATAK,
	HYBRID_WALK_ATAK,
	HYBRID_DEATH
};

BITE_INFO hybrid_bite_left  = { 19, -13, 3, 7 },
		  hybrid_bite_right = { 19, -13, 3, 14 };

void HybridControl(int16_t item_number)
{
	if (!CreatureActive(item_number))
		return;

	auto item = &items[item_number];
	auto hybrid = (CREATURE_INFO*)item->data;

	int16_t head = 0,
		   torso_y = 0,
		   torso_x = 0,
		   angle = 0,
		   tilt = 0;

	if (item->hit_points <= 0)
	{
		if (item->current_anim_state != HYBRID_DEATH)
		{
			item->anim_number = objects[item->object_number].anim_index + HYBRID_DIE_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = HYBRID_DEATH;
		}
	}
	else
	{
		if (item->ai_bits)
			GetAITarget(hybrid);

		AI_INFO info,
				lara_info;

		CreatureAIInfo(item, &info);

		if (hybrid->enemy == lara_item)
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

		auto real_enemy = hybrid->enemy;

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, hybrid->maximum_turn);

		hybrid->enemy = lara_item;

		if (lara_info.distance < HYBRID_AWARE_DISTANCE || item->hit_status || TargetVisible(item, &lara_info))
			AlertAllGuards(item_number);

		hybrid->enemy = real_enemy;

		switch (item->current_anim_state)
		{
		case HYBRID_STOP:
		{
			hybrid->maximum_turn = 0;
			hybrid->flags = 0;

			head = info.angle;

			if (!(item->ai_bits & GUARD))
			{
				if (item->ai_bits & PATROL1)
				{
					item->goal_anim_state = HYBRID_WALK;
					head = 0;
				}
				else if (hybrid->mood == ESCAPE_MOOD)
					item->goal_anim_state = (lara.target != item && info.ahead ? HYBRID_STOP : HYBRID_RUN);
				else if (info.angle < 0x2000 && info.angle > -0x2000 && info.distance > HYBRID_ATTACK1_RANGE)
					item->goal_anim_state = HYBRID_JUMP_STR;
				else if (info.bite && info.distance < HYBRID_ATTACK1_RANGE)
				{
					torso_y = info.angle;
					torso_x = info.x_angle;
					item->goal_anim_state = (info.angle < 0 ? HYBRID_SLASH_LEFT : HYBRID_KICK);
				}
				else if (item->required_anim_state)
					item->goal_anim_state = item->required_anim_state;
				else if (info.distance < HYBRID_ATTACK2_RANGE)
					item->goal_anim_state = HYBRID_WALK;
				else item->goal_anim_state = HYBRID_RUN;
			}
			else
			{
				head = AIGuard(hybrid);
				item->goal_anim_state = HYBRID_STOP;
			}

			break;
		}
		case HYBRID_WALK:
		{
			hybrid->maximum_turn = HYBRID_WALK_TURN;

			if (info.ahead)
				head = info.angle;

			if (item->ai_bits & PATROL1)
			{
				item->goal_anim_state = HYBRID_WALK;
				head = 0;
			}
			else if (info.bite && info.distance < HYBRID_ATTACK3_RANGE)
			{
				hybrid->maximum_turn = HYBRID_WALK_TURN;
				item->goal_anim_state = HYBRID_WALK_ATAK;
			}
			else if (hybrid->mood == ESCAPE_MOOD || hybrid->mood == ATTACK_MOOD)
				item->goal_anim_state = HYBRID_RUN;

			break;
		}
		case HYBRID_RUN:
		{
			hybrid->maximum_turn = HYBRID_RUN_TURN;

			if (info.ahead)
				head = info.angle;
			if (item->ai_bits & GUARD)
				item->goal_anim_state = HYBRID_STOP;
			else if (hybrid->mood == BORED_MOOD || (hybrid->mood == ESCAPE_MOOD && lara.target != item && info.ahead))
				item->goal_anim_state = HYBRID_STOP;
			else if (hybrid->flags && info.ahead)
				item->goal_anim_state = HYBRID_STOP;
			else if (info.bite && info.distance < HYBRID_ATTACK2_RANGE)
				item->goal_anim_state = (lara_item->speed == 0 ? HYBRID_STOP : HYBRID_RUN_ATAK);
			else if (info.distance < HYBRID_ATTACK2_RANGE)
				item->goal_anim_state = HYBRID_WALK;

			hybrid->flags = 0;

			break;
		}
		case HYBRID_WALK_ATAK:
		case HYBRID_RUN_ATAK:
		case HYBRID_SLASH_LEFT:
		{
			hybrid->maximum_turn = HYBRID_WALK_TURN;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			if (!hybrid->flags && (item->touch_bits & HYBRID_SLASH_TOUCH))
			{
				lara_item->hit_status = 1;
				lara_item->hit_points -= HYBRID_SLASH_DAMAGE;
				CreatureEffect(item, &hybrid_bite_left, DoBloodSplat);
				hybrid->flags = 1;
			}

			if (!(info.bite && info.distance < HYBRID_ATTACK1_RANGE))
				item->goal_anim_state = HYBRID_STOP;

			if (item->frame_number == anims[item->anim_number].frame_end)
				hybrid->flags = 0;

			break;
		}
		case HYBRID_KICK:
		{
			hybrid->maximum_turn = HYBRID_WALK_TURN;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			if (!hybrid->flags && (item->touch_bits & HYBRID_KICK_TOUCH))
			{
				lara_item->hit_status = 1;
				lara_item->hit_points -= HYBRID_KICK_DAMAGE;
				CreatureEffect(item, &hybrid_bite_right, DoBloodSplat);
				hybrid->flags = 1;
			}

			if (!(info.bite && info.distance < HYBRID_ATTACK1_RANGE))
				item->goal_anim_state = HYBRID_STOP;

			if (item->frame_number == anims[item->anim_number].frame_end)
				hybrid->flags = 0;

			break;
		}
		case HYBRID_JUMP_STR:
			hybrid->maximum_turn = HYBRID_WALK_TURN;
			break;
		case HYBRID_JUMP_MID:
		case HYBRID_JUMP_END:
		{
			hybrid->maximum_turn = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			if ((item->touch_bits & HYBRID_SLASH_TOUCH))
			{
				lara_item->hit_status = 1;
				lara_item->hit_points -= HYBRID_JUMP_DAMAGE;
				CreatureEffect(item, &hybrid_bite_left, DoBloodSplat);
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