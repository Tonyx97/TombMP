#include "objects.h"
#include "lara.h"
#include "control.h"
#include "people.h"
#include "effect2.h"
#include "sphere.h"

#include <specific/standard.h>
#include <specific/fn_stubs.h>

#define SWAT_SHOT_DAMAGE		28
#define SWAT_WALK_TURN			(ONE_DEGREE * 6)
#define SWAT_RUN_TURN			(ONE_DEGREE * 9)
#define SWAT_RUN_RANGE			SQUARE(WALL_L * 2)
#define SWAT_SHOOT1_RANGE		SQUARE(WALL_L * 3)
#define SWAT_DIE_ANIM			19
#define SWAT_STOP_ANIM			12
#define SWAT_WALK_STOP_ANIM		17
#define SWAT_DEATH_SHOT_ANGLE	0x2000
#define SWAT_AWARE_DISTANCE		SQUARE(WALL_L)

enum swat_anims
{
	SWAT_EMPTY,
	SWAT_STOP,
	SWAT_WALK,
	SWAT_RUN,
	SWAT_WAIT,
	SWAT_SHOOT1,
	SWAT_SHOOT2,
	SWAT_DEATH,
	SWAT_AIM1,
	SWAT_AIM2,
	SWAT_AIM3,
	SWAT_SHOOT3
};

BITE_INFO swat_gun = { 0, 300, 64, 7 };

void InitialiseSwat(int16_t item_number)
{
	auto item = &items[item_number];
	
	InitialiseCreature(item_number);

	item->anim_number = objects[item->object_number].anim_index + SWAT_STOP_ANIM;
	item->frame_number = anims[item->anim_number].frame_base;
	item->current_anim_state = item->goal_anim_state = SWAT_STOP;
}

void SwatControl(int16_t item_number)
{
	if (!CreatureActive(item_number))
		return;

	auto item = &items[item_number];
	auto swat = (CREATURE_INFO*)item->data;

	if (!swat)
		return;

	int16_t angle = 0,
		   torso_y = 0,
		   torso_x = 0,
		   head = 0,
		   tilt = 0;

	if (item->fired_weapon)
	{
		phd_PushMatrix();
		{
			PHD_VECTOR pos { swat_gun.x, swat_gun.y, swat_gun.z };

			GetJointAbsPosition(item, &pos, swat_gun.mesh_num);
			TriggerDynamicLight(pos.x, pos.y, pos.z, (item->fired_weapon << 1) + 8, 24, 16, 4);
		}
		phd_PopMatrix();
	}

	if (boxes[item->box_number].overlap_index & BLOCKED)
	{
		DoLotsOfBloodD(item->pos.x_pos, item->pos.y_pos - (GetRandomControl() & 255) - 32, item->pos.z_pos, (GetRandomControl() & 127) + 128, GetRandomControl() << 1, item->room_number, 3);
		item->hit_points -= 20;
	}

	AI_INFO info,
			lara_info;

	if (item->hit_points <= 0)
	{
		if (item->current_anim_state != SWAT_DEATH)
		{
			item->anim_number = objects[item->object_number].anim_index + SWAT_DIE_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = SWAT_DEATH;

			swat->flags = !(GetRandomControl() & 1);
		}
		else if (swat->flags && item->frame_number > anims[item->anim_number].frame_base + 44 && item->frame_number < anims[item->anim_number].frame_base + 52 && !(item->frame_number & 0x3))
		{
			CreatureAIInfo(item, &info);

			if (Targetable(item, &info))
			{
				if (info.angle > -SWAT_DEATH_SHOT_ANGLE && info.angle < SWAT_DEATH_SHOT_ANGLE)
				{
					torso_y = info.angle;
					head = info.angle;

					ShotLara(item, &info, &swat_gun, torso_y, SWAT_SHOT_DAMAGE * 3);
				
					g_audio->play_sound(item->object_number == LON_MERCENARY1 ? 72 : 137, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
				}
			}
		}
	}
	else
	{
		if (item->ai_bits)
			GetAITarget(swat);
		else swat->enemy = lara_item;

		CreatureAIInfo(item, &info);

		if (swat->enemy == lara_item)
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

		int meta_mood = (swat->enemy != lara_item ? VIOLENT : TIMID);

		GetCreatureMood(item, &info, meta_mood);
		CreatureMood(item, &info, meta_mood);

		angle = CreatureTurn(item, swat->maximum_turn);

		auto real_enemy = swat->enemy;

		swat->enemy = lara_item;

		if (item->hit_status || ((lara_info.distance < SWAT_AWARE_DISTANCE || TargetVisible(item, &lara_info)) && abs(lara_item->pos.y_pos - item->pos.y_pos) < (WALL_L * 2)))
		{
			if (!swat->alerted)
				g_audio->play_sound(item->object_number == SWAT_GUN ? 300 : 299, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

			AlertAllGuards(item_number);
		}

		swat->enemy = real_enemy;
		
		switch (item->current_anim_state)
		{
		case SWAT_STOP:
		{
			head = lara_info.angle;

			swat->flags = 0;
			swat->maximum_turn = 0;

			if (item->anim_number == objects[item->object_number].anim_index + SWAT_WALK_STOP_ANIM)
			{
				if (abs(info.angle) < SWAT_RUN_TURN)
					item->pos.y_rot += info.angle;
				else if (info.angle < 0)
					item->pos.y_rot -= SWAT_RUN_TURN;
				else item->pos.y_rot += SWAT_RUN_TURN;
			}

			if (!(item->ai_bits & GUARD))
			{
				if (item->ai_bits & PATROL1)
				{
					item->goal_anim_state = SWAT_WALK;
					head = 0;
				}
				else if (swat->mood == ESCAPE_MOOD)
					item->goal_anim_state = SWAT_RUN;
				else if (Targetable(item, &info))
				{
					if (info.distance < SWAT_SHOOT1_RANGE || info.zone_number != info.enemy_zone)
						item->goal_anim_state = (GetRandomControl() < 0x4000 ? SWAT_AIM1 : SWAT_AIM3);
					else item->goal_anim_state = SWAT_WALK;
				}
				else if (swat->mood == BORED_MOOD || ((item->ai_bits & FOLLOW) && (swat->reached_goal || lara_info.distance > SQUARE(WALL_L * 2))))
					item->goal_anim_state = SWAT_STOP;
				else if (swat->mood != BORED_MOOD && info.distance > SWAT_RUN_RANGE)
					item->goal_anim_state = SWAT_RUN;
				else item->goal_anim_state = SWAT_WALK;
			}
			else
			{
				head = AIGuard(swat);

				if (!(GetRandomControl() & 0xFF))
					item->goal_anim_state = (item->current_anim_state == SWAT_STOP ? SWAT_WAIT : SWAT_STOP);
			}

			break;
		}
		case SWAT_WAIT:
		{
			head = lara_info.angle;

			swat->flags = 0;
			swat->maximum_turn = 0;

			if (!(item->ai_bits & GUARD))
			{
				if (Targetable(item, &info))
					item->goal_anim_state = SWAT_SHOOT1;
				else if (swat->mood != BORED_MOOD || !info.ahead)
					item->goal_anim_state = SWAT_STOP;
			}
			else
			{
				head = AIGuard(swat);

				if (!(GetRandomControl() & 0xFF))
					item->goal_anim_state = (item->current_anim_state == SWAT_STOP ? SWAT_WAIT : SWAT_STOP);
			}

			break;
		}
		case SWAT_WALK:
		{
			head = lara_info.angle;

			swat->flags = 0;
			swat->maximum_turn = SWAT_WALK_TURN;

			if (item->ai_bits & PATROL1)
			{
				head = 0;
				item->goal_anim_state = SWAT_WALK;
			}
			else if (swat->mood == ESCAPE_MOOD)
				item->goal_anim_state = SWAT_RUN;
			else if ((item->ai_bits & GUARD) || ((item->ai_bits & FOLLOW) && (swat->reached_goal || lara_info.distance > SQUARE(WALL_L * 2))))
				item->goal_anim_state = SWAT_STOP;
			else if (Targetable(item, &info))
				item->goal_anim_state = (info.distance < SWAT_SHOOT1_RANGE || info.zone_number != info.enemy_zone ? SWAT_STOP : SWAT_AIM2);
			else if (swat->mood == BORED_MOOD && info.ahead)
				item->goal_anim_state = SWAT_STOP;
			else if (swat->mood != BORED_MOOD && info.distance > SWAT_RUN_RANGE)
				item->goal_anim_state = SWAT_RUN;

			break;
		}
		case SWAT_RUN:
		{
			if (info.ahead)
				head = info.angle;

			swat->maximum_turn = SWAT_RUN_TURN;
			tilt = angle / 2;

			if ((item->ai_bits & GUARD) || ((item->ai_bits & FOLLOW) && (swat->reached_goal || lara_info.distance > SQUARE(WALL_L * 2))))
				item->goal_anim_state = SWAT_WALK;
			else if (swat->mood == ESCAPE_MOOD)
				break;
			else if (Targetable(item, &info))
				item->goal_anim_state = SWAT_WALK;
			else if (swat->mood == BORED_MOOD || (swat->mood == STALK_MOOD && !(item->ai_bits & FOLLOW) && info.distance < SWAT_RUN_RANGE))
				item->goal_anim_state = SWAT_WALK;

			break;
		}
		case SWAT_AIM1:
		case SWAT_AIM3:
		{
			swat->flags = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;

				item->goal_anim_state = (Targetable(item, &info) ? (item->current_anim_state == SWAT_AIM1) ? SWAT_SHOOT1 : SWAT_SHOOT3
																 : SWAT_STOP);
			}

			break;
		}
		case SWAT_AIM2:
		{
			swat->flags = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;

				item->goal_anim_state = (Targetable(item, &info) ? SWAT_SHOOT2 : SWAT_WALK);
			}
			break;
		}
		case SWAT_SHOOT3:
		{
			if (item->goal_anim_state != SWAT_STOP && (swat->mood == ESCAPE_MOOD || info.distance > SWAT_SHOOT1_RANGE || !Targetable(item, &info)))
				item->goal_anim_state = SWAT_STOP;
		}
		case SWAT_SHOOT2:
		case SWAT_SHOOT1:
		{
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			if (!swat->flags)
			{
				ShotLara(item, &info, &swat_gun, torso_y, SWAT_SHOT_DAMAGE);

				swat->flags = 5;
			}
			else --swat->flags;
		}
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso_y);
	CreatureJoint(item, 1, torso_x);
	CreatureJoint(item, 2, head);

	CreatureAnimation(item_number, angle, 0);
}