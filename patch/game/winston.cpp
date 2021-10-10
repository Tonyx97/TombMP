#include <specific/standard.h>

#include "objects.h"
#include "lara.h"
#include "control.h"
#include "lot.h"

#define WINSTON_FALLDOWN_ANIM	16
#define WINSTON_STOP_RANGE		SQUARE(WALL_L*3/2)
#define WINSTON_HP_AFTER_KO		16
#define WINSTON_KO_TIME			150
#define WINSTON_TURN			(ONE_DEGREE * 2)
#define JUST_GOT_UP				999

void WinstonControl(int16_t item_number);
void OldWinstonControl(int16_t item_number);

enum winston_anim
{
	WINSTON_EMPTY,
	WINSTON_STOP,
	WINSTON_WALK,
	WINSTON_DEF1,
	WINSTON_DEF2,
	WINSTON_DEF3,
	WINSTON_HIT1,
	WINSTON_HIT2,
	WINSTON_HIT3,
	WINSTON_HITDOWN,
	WINSTON_FALLDOWN,
	WINSTON_GETUP,
	WINSTON_BRUSHOFF,
	WINSTON_ONFLOOR
};

void WinstonControl(int16_t item_number)
{
	int16_t angle;
	AI_INFO info;

	if (!CreatureActive(item_number))
		return;

	auto item = &items[item_number];
	auto winston = (CREATURE_INFO*)item->data;

	CreatureAIInfo(item, &info);
	GetCreatureMood(item, &info, VIOLENT);
	CreatureMood(item, &info, VIOLENT);

	angle = CreatureTurn(item, winston->maximum_turn);

	// kill Old Winston, bring on the one in uniform

	if (!item->item_flags[1])
	{
		auto cinfo = baddie_slots;

		for (int i = 0; i < NUM_SLOTS; ++i, ++cinfo)
		{
			if (items[cinfo->item_num].object_number != WINSTON)
				continue;

			items[cinfo->item_num].status = INVISIBLE;

			CreatureDie(cinfo->item_num, 0);

			break;
		}
	}

	if (item->hit_points <= 0)
	{
		winston->maximum_turn = 0;
		switch (item->current_anim_state)
		{
		case WINSTON_FALLDOWN:
		{
			if (item->hit_status)				item->goal_anim_state = WINSTON_HITDOWN;
			else if (item->item_flags[0]-- < 0) item->goal_anim_state = WINSTON_ONFLOOR;
			break;
		}
		case WINSTON_HITDOWN:
		{
			if (item->hit_status)				item->goal_anim_state = WINSTON_HITDOWN;
			else if (item->item_flags[0]-- < 0) item->goal_anim_state = WINSTON_ONFLOOR;
			break;
		}
		case WINSTON_ONFLOOR:
			if (item->hit_status)				item->goal_anim_state = WINSTON_HITDOWN;
			else if (item->item_flags[0]-- < 0) item->goal_anim_state = WINSTON_GETUP;
			break;
		case WINSTON_GETUP:
		{
			item->hit_points = WINSTON_HP_AFTER_KO;

			if (GetRandomControl() & 1)
				winston->flags = JUST_GOT_UP;

			break;
		}
		default:
		{
			item->anim_number = objects[ARMY_WINSTON].anim_index + WINSTON_FALLDOWN_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = WINSTON_FALLDOWN;
			item->goal_anim_state = WINSTON_FALLDOWN;
			item->item_flags[0] = WINSTON_KO_TIME;
		}
		}
	}
	else
	{
		switch (item->current_anim_state)
		{
		case WINSTON_STOP:
		{
			winston->maximum_turn = WINSTON_TURN;

			if (winston->flags == JUST_GOT_UP)
				item->goal_anim_state = WINSTON_BRUSHOFF;
			else if (lara.target == item)
				item->goal_anim_state = WINSTON_DEF1;
			else if (info.distance > WINSTON_STOP_RANGE || !info.ahead)
			{
				if (item->goal_anim_state != WINSTON_WALK)
				{
					item->goal_anim_state = WINSTON_WALK;

					g_audio->play_sound(345, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
				}
			}

			break;
		}
		case WINSTON_WALK:
		{
			winston->maximum_turn = WINSTON_TURN;

			if (lara.target == item)
				item->goal_anim_state = WINSTON_STOP;
			else if (info.distance < WINSTON_STOP_RANGE)
			{
				if (info.ahead)
				{
					item->goal_anim_state = WINSTON_STOP;

					if (winston->flags & 1)
						--winston->flags;
				}
				else if (!(winston->flags & 1))
				{
					g_audio->play_sound(344, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
					g_audio->play_sound(347, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

					winston->flags |= 1;
				}
			}

			break;
		}
		case WINSTON_BRUSHOFF:
		{
			winston->maximum_turn = 0;
			winston->flags = 0;
			break;
		}
		case WINSTON_DEF1:
		{
			winston->maximum_turn = WINSTON_TURN;

			if (item->required_anim_state)  item->goal_anim_state = item->required_anim_state;
			if (item->hit_status)			item->goal_anim_state = WINSTON_HIT1;
			else if (lara.target != item)	item->goal_anim_state = WINSTON_STOP;

			break;
		}
		case WINSTON_DEF2:
		{
			winston->maximum_turn = WINSTON_TURN;

			if (item->required_anim_state)  item->goal_anim_state = item->required_anim_state;
			if (item->hit_status)			item->goal_anim_state = WINSTON_HIT2;
			else if (lara.target != item)	item->goal_anim_state = WINSTON_DEF1;

			break;
		}
		case WINSTON_DEF3:
		{
			winston->maximum_turn = WINSTON_TURN;

			if (item->required_anim_state)  item->goal_anim_state = item->required_anim_state;
			if (item->hit_status)			item->goal_anim_state = WINSTON_HIT3;
			else if (lara.target != item)   item->goal_anim_state = WINSTON_DEF1;

			break;
		}
		case WINSTON_HIT1:
			item->required_anim_state = ((GetRandomControl() & 1) ? WINSTON_DEF3 : WINSTON_DEF2);
			break;
		case WINSTON_HIT2:
			item->required_anim_state = WINSTON_DEF1;
			break;
		case WINSTON_HIT3:
			item->required_anim_state = WINSTON_DEF1;
			break;
		}
	}

	if (GetRandomControl() < 0x100)
		g_audio->play_sound(347, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

	CreatureAnimation(item_number, angle, 0);
}

void OldWinstonControl(int16_t item_number)
{
	if (!CreatureActive(item_number))
		return;

	auto item = &items[item_number];
	auto winston = (CREATURE_INFO*)item->data;

	AI_INFO info;

	CreatureAIInfo(item, &info);
	GetCreatureMood(item, &info, VIOLENT);
	CreatureMood(item, &info, VIOLENT);

	auto angle = CreatureTurn(item, winston->maximum_turn);

	if (item->current_anim_state == WINSTON_STOP)
	{
		if (info.distance > WINSTON_STOP_RANGE || !info.ahead)
		{
			if (item->goal_anim_state != WINSTON_WALK)
			{
				item->goal_anim_state = WINSTON_WALK;

				g_audio->play_sound(345, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
			}
		}
	}
	else
	{
		if (info.distance < WINSTON_STOP_RANGE)
		{
			if (info.ahead)
			{
				item->goal_anim_state = WINSTON_STOP;

				if (winston->flags & 1)
					--winston->flags;
			}
			else if (!(winston->flags & 1))
			{
				g_audio->play_sound(344, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
				g_audio->play_sound(347, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

				winston->flags |= 1;
			}
		}
	}

	if (item->touch_bits && !(winston->flags & 2))
	{
		g_audio->play_sound(346, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
		g_audio->play_sound(347, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

		winston->flags |= 2;
	}
	else if (!item->touch_bits && winston->flags & 2)
		winston->flags -= 2;

	if (GetRandomDraw() < 0x100)
		g_audio->play_sound(347, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

	CreatureAnimation(item_number, angle, 0);
}