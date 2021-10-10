#include "objects.h"
#include "lara.h"
#include "control.h"
#include "lot.h"

#define RAPTOR_LUNGE_DAMAGE			100
#define RAPTOR_BITE_DAMAGE			100
#define RAPTOR_CHARGE_DAMAGE		100
#define RAPTOR_TOUCH				(0xff7c00)
#define RAPTOR_DIE1_ANIM			9
#define RAPTOR_DIE2_ANIM			10
#define RAPTOR_ROAR_CHANCE			0x80
#define RAPTOR_LUNGE_RANGE			SQUARE(WALL_L * 3 / 2)
#define RAPTOR_ATTACK_RANGE			SQUARE(WALL_L * 3 / 2)
#define RAPTOR_CLOSE_RANGE			SQUARE(WALL_L * 4 / 7)
#define RAPTOR_ESCAPE_RANGE			SQUARE(WALL_L * 3)
#define RAPTOR_HIT_RADIUS			(STEP_L * 2)
#define RAPTOR_RUN_TURN				(ONE_DEGREE * 4)
#define RAPTOR_WALK_TURN			(ONE_DEGREE * 2)
#define RAPTOR_INFIGHTING_CHANCE	0x400
#define RAPTOR_INFIGHTING_RANGE		SQUARE(WALL_L * 2)
#define HIT_FLAG					1
#define KILL_FLAG					2
#define AMBUSH_FLAG					4
#define RAPTOR_HIT_POINTS			100

enum raptor_anims
{
	RAPTOR_EMPTY,
	RAPTOR_STOP,
	RAPTOR_WALK,
	RAPTOR_RUN,
	RAPTOR_ATTACK1,
	RAPTOR_DEATH,
	RAPTOR_WARNING,
	RAPTOR_ATTACK2,
	RAPTOR_ATTACK3
};

BITE_INFO raptor_bite = { 0, 66, 318, 22 };

void RaptorControl(int16_t item_number)
{
	ITEM_INFO* best, * target;

	best = nullptr;

	auto item = &items[item_number];

	if (item->status == INVISIBLE)
	{
		if (!EnableBaddieAI(item_number, 0))
			return;

		item->status = ACTIVE;
	}

	auto raptor = (CREATURE_INFO*)item->data;

	int16_t head = 0,
		    neck = 0,
		    angle = 0,
		    tilt = 0;

	if (item->hit_points <= 0)
	{
		if (item->current_anim_state != RAPTOR_DEATH)
		{
			item->anim_number = (GetRandomControl() > 0x4000 ? objects[item->object_number].anim_index + RAPTOR_DIE1_ANIM
															 : objects[item->object_number].anim_index + RAPTOR_DIE2_ANIM);
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = RAPTOR_DEATH;
		}
	}
	else
	{
		if (!raptor)
			return;

		if (!raptor->enemy || !(GetRandomControl() & 0x7F))
		{
			int best_distance = 0x7fffffff;

			auto cinfo = baddie_slots;

			for (int i = 0; i < NUM_SLOTS; ++i, ++cinfo)
			{
				if (cinfo->item_num == NO_ITEM || cinfo->item_num == item_number)
					continue;

				target = &items[cinfo->item_num];

				int x = (target->pos.x_pos - item->pos.x_pos) >> 6,
					y = (target->pos.y_pos - item->pos.y_pos) >> 6,
					z = (target->pos.z_pos - item->pos.z_pos) >> 6,
					distance = x * x + y * y + z * z;

				if (distance < best_distance && item->hit_points > 0)
				{
					best = target;
					best_distance = distance;
				}
			}

			if ((best && best->object_number != RAPTOR) || (GetRandomControl() < RAPTOR_INFIGHTING_CHANCE && best_distance < RAPTOR_INFIGHTING_RANGE))
				raptor->enemy = best;

			int x = (lara_item->pos.x_pos - item->pos.x_pos) >> 6,
				y = (lara_item->pos.y_pos - item->pos.y_pos) >> 6,
				z = (lara_item->pos.z_pos - item->pos.z_pos) >> 6,
				distance = x * x + y * y + z * z;

			if (distance <= best_distance)
				raptor->enemy = lara_item;
		}

		if (item->ai_bits)
			GetAITarget(raptor);

		AI_INFO info;

		CreatureAIInfo(item, &info);

		if (info.ahead)
			head = info.angle;

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		if (raptor->mood == BORED_MOOD)
			raptor->maximum_turn >>= 1;

		angle = CreatureTurn(item, raptor->maximum_turn);
		neck = -(angle * 6);

		auto enemy = raptor->enemy;

		switch (item->current_anim_state)
		{
		case RAPTOR_STOP:
		{
			raptor->maximum_turn = 0;
			raptor->flags &= ~HIT_FLAG;

			if (item->required_anim_state)
				item->goal_anim_state = item->required_anim_state;
			else if (raptor->flags & KILL_FLAG)
			{
				raptor->flags &= ~KILL_FLAG;
				item->goal_anim_state = RAPTOR_WARNING;
			}
			else if ((item->touch_bits & RAPTOR_TOUCH) || (info.distance < RAPTOR_CLOSE_RANGE && info.bite))
				item->goal_anim_state = RAPTOR_ATTACK3;
			else if (info.bite && info.distance < RAPTOR_LUNGE_RANGE)
				item->goal_anim_state = RAPTOR_ATTACK1;
			else if (raptor->mood == ESCAPE_MOOD && lara.target != item && info.ahead && !item->hit_status)
				item->goal_anim_state = RAPTOR_STOP;
			else if (raptor->mood == BORED_MOOD)
				item->goal_anim_state = RAPTOR_WALK;
			else item->goal_anim_state = RAPTOR_RUN;

			break;
		}
		case RAPTOR_WALK:
		{
			raptor->maximum_turn = RAPTOR_WALK_TURN;
			raptor->flags &= ~HIT_FLAG;

			if (raptor->mood != BORED_MOOD)
				item->goal_anim_state = RAPTOR_STOP;
			else if (info.ahead && GetRandomControl() < RAPTOR_ROAR_CHANCE)
			{
				item->required_anim_state = RAPTOR_WARNING;
				item->goal_anim_state = RAPTOR_STOP;

				raptor->flags &= ~KILL_FLAG;
			}

			break;
		}
		case RAPTOR_RUN:
		{
			tilt = angle;
			raptor->maximum_turn = RAPTOR_RUN_TURN;
			raptor->flags &= ~HIT_FLAG;

			if (item->touch_bits & RAPTOR_TOUCH)
				item->goal_anim_state = RAPTOR_STOP;
			else if (raptor->flags & KILL_FLAG)
			{
				item->required_anim_state = RAPTOR_WARNING;
				item->goal_anim_state = RAPTOR_STOP;
				raptor->flags &= ~KILL_FLAG;
			}
			else if (info.bite && info.distance < RAPTOR_ATTACK_RANGE)
			{
				if (item->goal_anim_state == RAPTOR_RUN)
					item->goal_anim_state = (GetRandomControl() < 0x2000 ? RAPTOR_STOP : RAPTOR_ATTACK2);
			}
			else if (info.ahead && raptor->mood != ESCAPE_MOOD && GetRandomControl() < RAPTOR_ROAR_CHANCE && enemy->object_number != ANIMATING6)
			{
				item->required_anim_state = RAPTOR_WARNING;
				item->goal_anim_state = RAPTOR_STOP;
			}
			else if (raptor->mood == BORED_MOOD || (raptor->mood == ESCAPE_MOOD && lara.target != item && info.ahead))
				item->goal_anim_state = RAPTOR_STOP;

			break;
		}
		case RAPTOR_ATTACK1:
		{
			raptor->maximum_turn = RAPTOR_WALK_TURN;

			tilt = angle;

			if (enemy == lara_item)
			{
				if (!(raptor->flags & HIT_FLAG) && (item->touch_bits & RAPTOR_TOUCH))
				{
					raptor->flags |= HIT_FLAG;

					CreatureEffect(item, &raptor_bite, DoBloodSplat);

					if (lara_item->hit_points <= 0)
						raptor->flags |= KILL_FLAG;

					lara_item->hit_points -= RAPTOR_LUNGE_DAMAGE;
					lara_item->hit_status = 1;

					item->required_anim_state = RAPTOR_STOP;
				}
			}
			else
			{
				if (!(raptor->flags & HIT_FLAG) && enemy)
				{
					if (ABS(enemy->pos.x_pos - item->pos.x_pos) < RAPTOR_HIT_RADIUS &&
						ABS(enemy->pos.y_pos - item->pos.y_pos) < RAPTOR_HIT_RADIUS &&
						ABS(enemy->pos.z_pos - item->pos.z_pos) < RAPTOR_HIT_RADIUS)
					{
						enemy->hit_points -= RAPTOR_LUNGE_DAMAGE >> 2;
						enemy->hit_status = 1;

						if (enemy->hit_points <= 0)
							raptor->flags |= KILL_FLAG;

						raptor->flags |= HIT_FLAG;

						CreatureEffect(item, &raptor_bite, DoBloodSplat);
					}
				}
			}

			break;
		}
		case RAPTOR_ATTACK3:
		{
			tilt = angle;
			raptor->maximum_turn = RAPTOR_WALK_TURN;

			if (enemy == lara_item)
			{
				if (!(raptor->flags & HIT_FLAG) && (item->touch_bits & RAPTOR_TOUCH))
				{
					raptor->flags |= HIT_FLAG;
					CreatureEffect(item, &raptor_bite, DoBloodSplat);

					if (lara_item->hit_points <= 0)
						raptor->flags |= KILL_FLAG;

					lara_item->hit_points -= RAPTOR_BITE_DAMAGE;
					lara_item->hit_status = 1;

					item->required_anim_state = RAPTOR_STOP;
				}
			}
			else
			{
				if (!(raptor->flags & HIT_FLAG) && enemy)
				{
					if (ABS(enemy->pos.x_pos - item->pos.x_pos) < RAPTOR_HIT_RADIUS &&
						ABS(enemy->pos.y_pos - item->pos.y_pos) < RAPTOR_HIT_RADIUS &&
						ABS(enemy->pos.z_pos - item->pos.z_pos) < RAPTOR_HIT_RADIUS)
					{
						enemy->hit_points -= RAPTOR_BITE_DAMAGE >> 2;
						enemy->hit_status = 1;

						if (enemy->hit_points <= 0)
							raptor->flags |= KILL_FLAG;

						raptor->flags |= HIT_FLAG;

						CreatureEffect(item, &raptor_bite, DoBloodSplat);

					}
				}
			}

			break;
		}

		case RAPTOR_ATTACK2:
		{
			tilt = angle;
			raptor->maximum_turn = RAPTOR_WALK_TURN;

			if (enemy == lara_item)
			{
				if (!(raptor->flags & HIT_FLAG) && (item->touch_bits & RAPTOR_TOUCH))
				{
					raptor->flags |= HIT_FLAG;
					CreatureEffect(item, &raptor_bite, DoBloodSplat);

					lara_item->hit_points -= RAPTOR_CHARGE_DAMAGE;
					lara_item->hit_status = 1;
					if (lara_item->hit_points <= 0)
						raptor->flags |= KILL_FLAG;
					item->required_anim_state = RAPTOR_RUN;
				}
			}
			else
			{
				if (!(raptor->flags & HIT_FLAG) && enemy)
				{
					if (ABS(enemy->pos.x_pos - item->pos.x_pos) < RAPTOR_HIT_RADIUS &&
						ABS(enemy->pos.y_pos - item->pos.y_pos) < RAPTOR_HIT_RADIUS &&
						ABS(enemy->pos.z_pos - item->pos.z_pos) < RAPTOR_HIT_RADIUS)
					{
						enemy->hit_points -= RAPTOR_CHARGE_DAMAGE >> 2;
						enemy->hit_status = 1;

						if (enemy->hit_points <= 0)
							raptor->flags |= KILL_FLAG;

						raptor->flags |= HIT_FLAG;

						CreatureEffect(item, &raptor_bite, DoBloodSplat);
					}
				}
			}
		}
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, head >> 1);
	CreatureJoint(item, 1, head >> 1);
	CreatureJoint(item, 2, neck);
	CreatureJoint(item, 3, neck);

	CreatureAnimation(item_number, angle, tilt);
}