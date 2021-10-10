#include "lara.h"
#include "control.h"

#define LUNGE_RANGE				SQUARE(WALL_L)
#define LUNGE_TOUCH				0x6648
#define LUNGE_DAMAGE			50
#define STALK_RANGE				SQUARE(WALL_L + (WALL_L / 2))
#define STALK_TURN				(3 * ONE_DEGREE)
#define BITE_RANGE				SQUARE(WALL_L * 5 / 12)
#define BITE_TOUCH 				0x48
#define BITE_DAMAGE 			12
#define DOG_RUN_TURN  			(6 * ONE_DEGREE)
#define DOG_STAT_TURN			(1 * ONE_DEGREE)
#define DOG_WALK_TURN			(3 * ONE_DEGREE)
#define MINIMUM_SLEEP_TIME		(30 * 10)
#define SLEEP_CHANCE			0x100
#define SLEEP_2_STAND_CHANCE	0x80
#define STAT_CHANCE				0x100
#define WALK_CHANCE				0x1000
#define DOG_AWARE_DISTANCE		SQUARE(WALL_L * 3)
#define DOG_FASTTURN_TURN		(ONE_DEGREE * 6)
#define DOG_FASTTURN_ANGLE		1000
#define DOG_FASTTURN_RANGE		SQUARE(WALL_L * 3)

#define RUN_ANIM	7
#define STOP_ANIM	8
#define DIE_ANIM1	20
#define DIE_ANIM2	21
#define DIE_ANIM3	22

enum dog_anims
{
	DOG_EMPTY,
	DOG_STOP,
	DOG_WALK,
	DOG_RUN,
	DOG_JUMP,
	DOG_STALK,
	DOG_ATTACK1,
	DOG_HOWL,
	DOG_SLEEP,
	DOG_CROUCH,
	DOG_TURN,
	DOG_DEATH,
	DOG_ATTACK2
};

BITE_INFO dog_bite = { 0, 0, 100, 3 };

void InitialiseDog(int16_t item_number)
{
	auto item = &items[item_number];

	item->anim_number = objects[item->object_number].anim_index + STOP_ANIM;
	item->frame_number = anims[item->anim_number].frame_base;
	item->current_anim_state = DOG_STOP;
}

void DogControl(int16_t item_number)
{
	if (!CreatureActive(item_number))
		return;

	auto item = &items[item_number];
	auto dog = (CREATURE_INFO*)item->data;

	int16_t head = 0,
		   x_head = 0,
		   angle = 0;

	if (item->hit_points <= 0)
	{
		if (item->current_anim_state != DOG_DEATH)
		{
			static constexpr char DeathAnims[] =
			{
				DIE_ANIM1,
				DIE_ANIM2,
				DIE_ANIM3,
				DIE_ANIM1
			};

			item->anim_number = objects[item->object_number].anim_index + DeathAnims[GetRandomControl() & 3];
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = DOG_DEATH;
		}
	}
	else if (dog)
	{
		AI_INFO info,
				lara_info;

		if (item->ai_bits)
			GetAITarget(dog);
		else dog->enemy = lara_item;

		CreatureAIInfo(item, &info);

		if (dog->enemy == lara_item)
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

		if (info.ahead)
		{
			head = info.angle;
			x_head = info.x_angle;
		}

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		if (dog->mood == BORED_MOOD)
			dog->maximum_turn >>= 1;

		angle = CreatureTurn(item, dog->maximum_turn);

		if (dog->hurt_by_lara || (lara_info.distance < DOG_AWARE_DISTANCE && !(item->ai_bits & MODIFY)))
		{
			AlertAllGuards(item_number);
			item->ai_bits &= ~MODIFY;
		}

		auto random = GetRandomControl();
		auto frame = item->frame_number - anims[item->anim_number].frame_base;

		switch (item->current_anim_state)
		{
		case DOG_SLEEP:
		case DOG_EMPTY:
		{
			head = 0;
			x_head = 0;

			if (dog->mood == BORED_MOOD || item->ai_bits == MODIFY)
			{
				dog->maximum_turn = 0;
				++dog->flags;

				if (dog->flags > MINIMUM_SLEEP_TIME && random < SLEEP_2_STAND_CHANCE)
					item->goal_anim_state = DOG_STOP;
			}
			else item->goal_anim_state = DOG_STOP;

			break;
		}
		case DOG_CROUCH:
		{
			if (item->required_anim_state)
			{
				item->goal_anim_state = item->required_anim_state;
				break;
			}
		}
		case DOG_STOP:
		{
			dog->maximum_turn = 0;

			if (item->ai_bits & GUARD)
			{
				head = AIGuard(dog);
				
				if (!(GetRandomControl() & 0xFF))
					item->goal_anim_state = (item->current_anim_state == DOG_STOP ? DOG_CROUCH : DOG_STOP);

				break;
			}
			else if (item->current_anim_state == DOG_CROUCH && random < SLEEP_2_STAND_CHANCE)
			{
				item->goal_anim_state = DOG_STOP;
				break;
			}
			else if (item->ai_bits & PATROL1)
				item->goal_anim_state = (item->current_anim_state == DOG_STOP ? DOG_WALK : DOG_STOP);
			else if (dog->mood == ESCAPE_MOOD)
			{
				if (lara.target != item && info.ahead && !item->hit_status)
					item->goal_anim_state = DOG_STOP;
				else
				{
					item->required_anim_state = DOG_RUN;
					item->goal_anim_state = DOG_CROUCH;
				}
			}
			else if (dog->mood == BORED_MOOD)
			{
				dog->flags = 0;
				dog->maximum_turn = DOG_STAT_TURN;

				if (random < SLEEP_CHANCE && (item->ai_bits & MODIFY) && item->current_anim_state == DOG_STOP)
				{
					item->goal_anim_state = DOG_SLEEP;
					dog->flags = 0;
				}
				else if (random < WALK_CHANCE)
					item->goal_anim_state = (item->current_anim_state == DOG_STOP ? DOG_WALK : DOG_STOP);
				else if (!(random & 0x1F))
					item->goal_anim_state = DOG_HOWL;
			}
			else
			{
				if (item->current_anim_state == DOG_STOP)
				{
					item->required_anim_state = DOG_RUN;
					item->goal_anim_state = DOG_CROUCH;
				}
				else item->required_anim_state = DOG_RUN;
			}

			break;
		}
		case DOG_WALK:
		{
			dog->maximum_turn = DOG_WALK_TURN;

			if (item->ai_bits & PATROL1)
				item->goal_anim_state = DOG_WALK;
			else if (dog->mood == BORED_MOOD && random < STAT_CHANCE)
				item->goal_anim_state = DOG_STOP;
			else item->goal_anim_state = DOG_STALK;

			break;
		}
		case DOG_RUN:
		{
			dog->maximum_turn = DOG_RUN_TURN;

			if (dog->mood != ESCAPE_MOOD)
			{
				if (dog->mood == BORED_MOOD)
					item->goal_anim_state = DOG_CROUCH;
				else if ((info.bite) && (info.distance < LUNGE_RANGE))
					item->goal_anim_state = DOG_ATTACK1;
				else if (info.distance < STALK_RANGE)
				{
					item->required_anim_state = DOG_STALK;
					item->goal_anim_state = DOG_CROUCH;
				}
			}
			else if (lara.target != item && info.ahead)
				item->goal_anim_state = DOG_CROUCH;

			break;
		}
		case DOG_STALK:
		{
			dog->maximum_turn = STALK_TURN;

			if (dog->mood == BORED_MOOD)
				item->goal_anim_state = DOG_CROUCH;
			else if (dog->mood == ESCAPE_MOOD)
				item->goal_anim_state = DOG_RUN;
			else if (info.bite && info.distance < BITE_RANGE)
			{
				item->goal_anim_state = DOG_ATTACK2;
				item->required_anim_state = DOG_STALK;
			}
			else if (info.distance > STALK_RANGE || item->hit_status)
				item->goal_anim_state = DOG_RUN;

			break;
		}
		case DOG_ATTACK1:
		{
			if (info.bite && (item->touch_bits & LUNGE_TOUCH) && frame >= 4 && frame <= 14)
			{
				CreatureEffect(item, &dog_bite, DoBloodSplat);
				lara_item->hit_points -= LUNGE_DAMAGE;
				lara_item->hit_status = 1;
			}

			item->goal_anim_state = DOG_RUN;

			break;
		}
		case DOG_ATTACK2:
		{
			if (info.bite && (item->touch_bits & BITE_TOUCH) && ((frame >= 9 && frame <= 12) || (frame >= 22 && frame <= 25)))
			{
				CreatureEffect(item, &dog_bite, DoBloodSplat);
				lara_item->hit_points -= BITE_DAMAGE;
				lara_item->hit_status = 1;
			}

			break;
		}
		case DOG_HOWL:
			head = x_head = 0;
			break;
		}
	}

	CreatureTilt(item, 0);
	CreatureJoint(item, 0, head);
	CreatureJoint(item, 1, x_head);

	CreatureAnimation(item_number, angle, 0);
}