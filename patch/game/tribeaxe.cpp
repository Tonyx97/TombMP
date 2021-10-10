#include <specific/standard.h>

#include "lara.h"
#include "control.h"

#define TRIBEAXE_AXE_DAMAGE			16
#define TRIBEAXE_AXE_ENEMY_DAMAGE	2
#define TRIBEAXE_WALK_TURN			(ONE_DEGREE * 9)
#define TRIBEAXE_RUN_TURN			(ONE_DEGREE * 6)
#define TRIBEAXE_OTHER_TURN			(ONE_DEGREE * 4)
#define TRIBEAXE_CLOSE_RANGE		SQUARE(WALL_L * 2 / 3)
#define TRIBEAXE_LONG_RANGE			SQUARE(WALL_L)
#define TRIBEAXE_ATTACK5_RANGE		SQUARE(WALL_L * 3)
#define TRIBEAXE_WALK_RANGE			SQUARE(WALL_L * 2)
#define TRIBE_ESCAPE_RANGE			SQUARE(WALL_L * 3)
#define TRIBEAXE_HIT_RANGE			(STEP_L * 2)
#define TRIBEAXE_TOUCH				(1 << 13)
#define TRIBEAXE_DIE_ANIM_STAND		20
#define TRIBEAXE_DIE_ANIM_DOWN		21

enum tribeaxe_anims
{
	TRIBEAXE_EMPTY,
	TRIBEAXE_WAIT1,
	TRIBEAXE_WALK,
	TRIBEAXE_RUN,
	TRIBEAXE_ATTACK1,
	TRIBEAXE_ATTACK2,
	TRIBEAXE_ATTACK3,
	TRIBEAXE_ATTACK4,
	TRIBEAXE_AIM3,
	TRIBEAXE_DEATH,
	TRIBEAXE_ATTACK5,
	TRIBEAXE_WAIT2,
	TRIBEAXE_ATTACK6
};

BITE_INFO tribeaxe_hit = { 0, 16, 265, 13 };

static constexpr unsigned char tribeaxe_hitframes[13][3] =
{
	{ 0, 0, 0 },	//	0 	TRIBEAXE_EMPTY
	{ 0, 0, 0 },  	//	1 	TRIBEAXE_WAIT1
	{ 0, 0, 0 },  	//	2 	TRIBEAXE_WALK
	{ 0, 0, 0 },  	//	3 	TRIBEAXE_RUN
	{ 0, 0, 0 },  	//	4 	TRIBEAXE_ATTACK1
	{ 2, 12, 8 }, 	//	5 	TRIBEAXE_ATTACK2
	{ 8, 9, 32 },  	//	6 	TRIBEAXE_ATTACK3
	{ 19, 28, 8 },	//	7 	TRIBEAXE_ATTACK4
	{ 0, 0, 0 },  	//	8 	TRIBEAXE_AIM3
	{ 0, 0, 0 },  	//	9 	TRIBEAXE_DEATH
	{ 7, 14, 8 }, 	//	10	TRIBEAXE_ATTACK5
	{ 0, 0, 0 },  	//	11	TRIBEAXE_WAIT2
	{ 15, 19, 32 } 	//	12	TRIBEAXE_ATTACK6
};

void TribeAxeControl(int16_t item_number)
{
	if (!CreatureActive(item_number))
		return;

	auto item = &items[item_number];
	auto tribeaxe = (CREATURE_INFO*)item->data;

	int16_t head = 0,
		   angle = 0,
		   tilt = 0;

	if (item->hit_points <= 0)
	{
		if (item->current_anim_state != TRIBEAXE_DEATH)
		{
			if (item->current_anim_state == TRIBEAXE_WAIT1 ||
				item->current_anim_state == TRIBEAXE_ATTACK4)
				item->anim_number = objects[item->object_number].anim_index + TRIBEAXE_DIE_ANIM_DOWN;
			else
				item->anim_number = objects[item->object_number].anim_index + TRIBEAXE_DIE_ANIM_STAND;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = TRIBEAXE_DEATH;
		}
	}
	else
	{
		AI_INFO info;

		CreatureAIInfo(item, &info);
		GetCreatureMood(item, &info, VIOLENT);

		if (tribeaxe->enemy == lara_item && tribeaxe->hurt_by_lara && info.distance > TRIBE_ESCAPE_RANGE && info.enemy_facing < 0x3000 && info.enemy_facing > -0x3000)
			tribeaxe->mood = ESCAPE_MOOD;

		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, tribeaxe->maximum_turn);

		if (info.ahead)
			head = info.angle;

		switch (item->current_anim_state)
		{
		case TRIBEAXE_WAIT1:
		{
			tribeaxe->maximum_turn = TRIBEAXE_OTHER_TURN;
			tribeaxe->flags = 0;

			if (tribeaxe->mood == BORED_MOOD)
			{
				tribeaxe->maximum_turn = 0;

				if (GetRandomControl() < 0x100)
					item->goal_anim_state = TRIBEAXE_WALK;
			}
			else if (tribeaxe->mood == ESCAPE_MOOD)
				item->goal_anim_state = (lara.target != item && info.ahead && !item->hit_status ? TRIBEAXE_WAIT1 : TRIBEAXE_RUN);
			else if (item->item_flags[0])
			{
				item->item_flags[0] = 0;
				item->goal_anim_state = TRIBEAXE_WAIT2;
			}
			else if (info.ahead && info.distance < TRIBEAXE_CLOSE_RANGE)
				item->goal_anim_state = TRIBEAXE_ATTACK4;
			else if (info.ahead && info.distance < TRIBEAXE_LONG_RANGE)
				item->goal_anim_state = (GetRandomControl() < 0x4000 ? TRIBEAXE_WALK : TRIBEAXE_ATTACK4);
			else if (info.ahead && info.distance < TRIBEAXE_WALK_RANGE)
				item->goal_anim_state = TRIBEAXE_WALK;
			else item->goal_anim_state = TRIBEAXE_RUN;

			break;
		}
		case TRIBEAXE_WAIT2:
		{
			tribeaxe->maximum_turn = TRIBEAXE_OTHER_TURN;
			tribeaxe->flags = 0;

			if (tribeaxe->mood == BORED_MOOD)
			{
				tribeaxe->maximum_turn = 0;

				if (GetRandomControl() < 0x100)
					item->goal_anim_state = TRIBEAXE_WALK;
			}
			else if (tribeaxe->mood == ESCAPE_MOOD)
				item->goal_anim_state = (lara.target != item && info.ahead && !item->hit_status ? TRIBEAXE_WAIT1 : TRIBEAXE_RUN);
			else if (info.ahead && info.distance < TRIBEAXE_CLOSE_RANGE)
				item->goal_anim_state = (GetRandomControl() < 0x800 ? TRIBEAXE_ATTACK2 : TRIBEAXE_AIM3);
			else if (info.distance < TRIBEAXE_WALK_RANGE)
				item->goal_anim_state = TRIBEAXE_WALK;
			else item->goal_anim_state = TRIBEAXE_RUN;

			break;
		}
		case TRIBEAXE_WALK:
		{
			tribeaxe->flags = 0;
			tribeaxe->maximum_turn = TRIBEAXE_WALK_TURN;
			tilt = angle >> 3;

			if (tribeaxe->mood == BORED_MOOD)
			{
				tribeaxe->maximum_turn >>= 2;

				if (GetRandomControl() < 0x100)
					item->goal_anim_state = (GetRandomControl() < 0x2000 ? TRIBEAXE_WAIT1 : TRIBEAXE_WAIT2);
			}
			else if (tribeaxe->mood == ESCAPE_MOOD)
				item->goal_anim_state = TRIBEAXE_RUN;
			else if (info.ahead && info.distance < TRIBEAXE_CLOSE_RANGE)
				item->goal_anim_state = (GetRandomControl() < 0x2000 ? TRIBEAXE_WAIT1 : TRIBEAXE_WAIT2);
			else if (info.distance > TRIBEAXE_WALK_RANGE)
				item->goal_anim_state = TRIBEAXE_RUN;

			break;
		}
		case TRIBEAXE_RUN:
		{
			tribeaxe->flags = 0;
			tribeaxe->maximum_turn = TRIBEAXE_RUN_TURN;
			tilt = angle >> 2;

			if (tribeaxe->mood == BORED_MOOD)
			{
				tribeaxe->maximum_turn >>= 2;

				if (GetRandomControl() < 0x100)
					item->goal_anim_state = (GetRandomControl() < 0x4000 ? TRIBEAXE_WAIT1 : TRIBEAXE_WAIT2);
			}
			else if (tribeaxe->mood == ESCAPE_MOOD && lara.target != item && info.ahead)
				item->goal_anim_state = TRIBEAXE_WAIT2;
			else if (info.bite || info.distance < TRIBEAXE_WALK_RANGE)
			{
				if (GetRandomControl() < 0x4000)
					item->goal_anim_state = TRIBEAXE_ATTACK6;
				else if (GetRandomControl() < 0x2000)
					item->goal_anim_state = TRIBEAXE_ATTACK5;
				else item->goal_anim_state = TRIBEAXE_WALK;
			}

			break;
		}
		case TRIBEAXE_AIM3:
		{
			tribeaxe->maximum_turn = TRIBEAXE_OTHER_TURN;
			item->goal_anim_state = (info.bite || info.distance < TRIBEAXE_CLOSE_RANGE ? TRIBEAXE_ATTACK3 : TRIBEAXE_WAIT2);
			break;
		}
		case TRIBEAXE_ATTACK2:
		case TRIBEAXE_ATTACK3:
		case TRIBEAXE_ATTACK4:
		case TRIBEAXE_ATTACK5:
		case TRIBEAXE_ATTACK6:
		{
			auto enemy = tribeaxe->enemy;

			item->item_flags[0] = 1;
			tribeaxe->maximum_turn = TRIBEAXE_OTHER_TURN;
			tribeaxe->flags = item->frame_number - anims[item->anim_number].frame_base;

			if (enemy == lara_item)
			{
				if ((item->touch_bits & TRIBEAXE_TOUCH) &&
					tribeaxe->flags >= tribeaxe_hitframes[item->current_anim_state][0] &&
					tribeaxe->flags <= tribeaxe_hitframes[item->current_anim_state][1])
				{
					lara_item->hit_points -= tribeaxe_hitframes[item->current_anim_state][2];
					lara_item->hit_status = 1;

					for (int i = 0; i < tribeaxe_hitframes[item->current_anim_state][2]; i += 8)
						CreatureEffect(item, &tribeaxe_hit, DoBloodSplat);

					g_audio->play_sound(70, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
				}
			}
			else
			{
				if (enemy)
				{
					if (ABS(enemy->pos.x_pos - item->pos.x_pos) < TRIBEAXE_HIT_RANGE &&
						ABS(enemy->pos.y_pos - item->pos.y_pos) < TRIBEAXE_HIT_RANGE &&
						ABS(enemy->pos.z_pos - item->pos.z_pos) < TRIBEAXE_HIT_RANGE &&
						tribeaxe->flags >= tribeaxe_hitframes[item->current_anim_state][0] &&
						tribeaxe->flags <= tribeaxe_hitframes[item->current_anim_state][1])
					{
						enemy->hit_points -= TRIBEAXE_AXE_ENEMY_DAMAGE;
						enemy->hit_status = 1;

						CreatureEffect(item, &tribeaxe_hit, DoBloodSplat);
						g_audio->play_sound(70, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
					}
				}
			}

			break;
		}
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, head >> 1);
	CreatureJoint(item, 1, head >> 1);

	CreatureAnimation(item_number, angle, 0);
}