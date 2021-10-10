#include "objects.h"
#include "lara.h"
#include "control.h"
#include "people.h"
#include "sphere.h"
#include "effect2.h"
#include "lot.h"

#include <specific/standard.h>
#include <specific/fn_stubs.h>

#define ARMY_SHOT_DAMAGE		28
#define ARMY_WALK_TURN			(ONE_DEGREE * 5)
#define ARMY_RUN_TURN			(ONE_DEGREE * 10)
#define ARMY_RUN_RANGE			SQUARE(WALL_L * 2)
#define ARMY_SHOOT1_RANGE		SQUARE(WALL_L * 3)
#define ARMY_DIE_ANIM			19
#define ARMY_STOP_ANIM			12
#define ARMY_WALK_STOP_ANIM		17
#define ARMY_DEATH_SHOT_ANGLE	0x2000
#define ARMY_AWARE_DISTANCE		SQUARE(WALL_L)

enum army_anims
{
	ARMY_EMPTY,
	ARMY_STOP,
	ARMY_WALK,
	ARMY_RUN,
	ARMY_WAIT,
	ARMY_SHOOT1,
	ARMY_SHOOT2,
	ARMY_DEATH,
	ARMY_AIM1,
	ARMY_AIM2,
	ARMY_AIM3,
	ARMY_SHOOT3
};

BITE_INFO army_gun = { 0, 300, 64, 7 };

void InitialiseArmySMG(int16_t item_number)
{
	auto item = &items[item_number];

	InitialiseCreature(item_number);

	item->anim_number = objects[STHPAC_MERCENARY].anim_index + ARMY_STOP_ANIM;
	item->frame_number = anims[item->anim_number].frame_base;
	item->current_anim_state = item->goal_anim_state = ARMY_STOP;
}

void ArmySMGControl(int16_t item_number)
{
	int16_t angle, torso_y, torso_x, head, tilt;

	if (!CreatureActive(item_number))
		return;

	auto item = &items[item_number];
	auto army = (CREATURE_INFO*)item->data;

	if (!army)
		return;

	torso_y = torso_x = head = angle = tilt = 0;

	if (item->fired_weapon)
	{
		phd_PushMatrix();
		{
			PHD_VECTOR pos { army_gun.x, army_gun.y, army_gun.z };
			
			GetJointAbsPosition(item, &pos, army_gun.mesh_num);
			TriggerDynamicLight(pos.x, pos.y, pos.z, (item->fired_weapon << 1) + 8, 24, 16, 4);
		}
		phd_PopMatrix();
	}

	if (item->hit_points <= 0)
	{
		if (item->current_anim_state != ARMY_DEATH)
		{
			item->anim_number = objects[item->object_number].anim_index + ARMY_DIE_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = ARMY_DEATH;

			if (army)
				army->flags = !(GetRandomControl() & 3);
		}
	}
	else
	{
		AI_INFO info,
				lara_info;

		if (item->ai_bits)
			GetAITarget(army);
		else if (army->hurt_by_lara)
			army->enemy = lara_item;
		else
		{
			int best_distance = 0x7fffffff;

			army->enemy = nullptr;

			auto cinfo = baddie_slots;

			for (int i = 0; i < NUM_SLOTS; ++i, ++cinfo)
			{
				if (cinfo->item_num == NO_ITEM || cinfo->item_num == item_number)
					continue;

				auto target = &items[cinfo->item_num];

				if (target->object_number == LARA || target->object_number == STHPAC_MERCENARY || (target == lara_item && !army->hurt_by_lara))
					continue;

				int x = target->pos.x_pos - item->pos.x_pos,
					z = target->pos.z_pos - item->pos.z_pos,
					distance = x * x + z * z;

				if (distance < best_distance)
				{
					army->enemy = target;
					best_distance = distance;
				}
			}
		}

		CreatureAIInfo(item, &info);

		if (army->enemy == lara_item)
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

		if (!army->hurt_by_lara && army->enemy == lara_item)
			army->enemy = nullptr;

		int meta_mood = (army->enemy != lara_item ? VIOLENT : TIMID);

		GetCreatureMood(item, &info, meta_mood);
		CreatureMood(item, &info, meta_mood);

		angle = CreatureTurn(item, army->maximum_turn);

		auto real_enemy = army->enemy;

		if (item->hit_status)
		{
			if (!army->alerted)
				g_audio->play_sound(300, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

			AlertAllGuards(item_number);
		}
		
		switch (item->current_anim_state)
		{
		case ARMY_STOP:
		{
			head = lara_info.angle;
			army->flags = 0;

			army->maximum_turn = 0;

			if (item->anim_number == objects[item->object_number].anim_index + ARMY_WALK_STOP_ANIM)
			{
				if (abs(info.angle) < ARMY_RUN_TURN)
					item->pos.y_rot += info.angle;
				else if (info.angle < 0)
					item->pos.y_rot -= ARMY_RUN_TURN;
				else
					item->pos.y_rot += ARMY_RUN_TURN;
			}

			if (!(item->ai_bits & GUARD))
			{
				if (item->ai_bits & PATROL1)
				{
					item->goal_anim_state = ARMY_WALK;
					head = 0;
				}
				else if (army->mood == ESCAPE_MOOD)
					item->goal_anim_state = ARMY_RUN;
				else if (Targetable(item, &info))
				{
					if (info.distance < ARMY_SHOOT1_RANGE || info.zone_number != info.enemy_zone)
						item->goal_anim_state = (GetRandomControl() < 0x4000 ? ARMY_AIM1 : ARMY_AIM3);
					else item->goal_anim_state = ARMY_WALK;
				}
				else if ((!army->alerted && army->mood == BORED_MOOD) || ((item->ai_bits & FOLLOW) && (army->reached_goal || lara_info.distance > SQUARE(WALL_L * 2))))
					item->goal_anim_state = ARMY_STOP;
				else if (army->mood != BORED_MOOD && info.distance > ARMY_RUN_RANGE)
					item->goal_anim_state = ARMY_RUN;
				else item->goal_anim_state = ARMY_WALK;
			}
			else
			{
				head = AIGuard(army);

				if (!(GetRandomControl() & 0xFF))
					item->goal_anim_state = (item->current_anim_state == ARMY_STOP ? ARMY_WAIT : ARMY_STOP);
			}

			break;
		}
		case ARMY_WAIT:
		{
			head = lara_info.angle;

			army->flags = 0;
			army->maximum_turn = 0;

			if (!(item->ai_bits & GUARD))
			{
				if (Targetable(item, &info))
					item->goal_anim_state = ARMY_SHOOT1;
				else if (army->mood != BORED_MOOD || !info.ahead)
					item->goal_anim_state = ARMY_STOP;
			}
			else
			{
				head = AIGuard(army);

				if (!(GetRandomControl() & 0xFF))
					item->goal_anim_state = ARMY_STOP;
			}

			break;
		}
		case ARMY_WALK:
		{
			head = lara_info.angle;

			army->flags = 0;
			army->maximum_turn = ARMY_WALK_TURN;

			if (item->ai_bits & PATROL1)
				item->goal_anim_state = ARMY_WALK;
			else if (army->mood == ESCAPE_MOOD)
				item->goal_anim_state = ARMY_RUN;
			else if ((item->ai_bits & GUARD) || ((item->ai_bits & FOLLOW) && (army->reached_goal || lara_info.distance > SQUARE(WALL_L * 2))))
				item->goal_anim_state = ARMY_STOP;
			else if (Targetable(item, &info))
				item->goal_anim_state = (info.distance < ARMY_SHOOT1_RANGE || info.zone_number != info.enemy_zone ? ARMY_STOP : ARMY_AIM2);
			else if (army->mood == BORED_MOOD && info.ahead)
				item->goal_anim_state = ARMY_STOP;
			else if (army->mood != BORED_MOOD && info.distance > ARMY_RUN_RANGE)
				item->goal_anim_state = ARMY_RUN;

			break;
		}
		case ARMY_RUN:
		{
			if (info.ahead)
				head = info.angle;

			army->maximum_turn = ARMY_RUN_TURN;

			tilt = angle / 2;

			if ((item->ai_bits & GUARD) || ((item->ai_bits & FOLLOW) && (army->reached_goal || lara_info.distance > SQUARE(WALL_L * 2))))
				item->goal_anim_state = ARMY_WALK;
			else if (army->mood == ESCAPE_MOOD)
				break;
			else if (Targetable(item, &info))
				item->goal_anim_state = ARMY_WALK;
			else if (army->mood == BORED_MOOD || (army->mood == STALK_MOOD && !(item->ai_bits & FOLLOW) && info.distance < ARMY_RUN_RANGE))
				item->goal_anim_state = ARMY_WALK;

			break;
		}
		case ARMY_AIM1:
		case ARMY_AIM3:
		{
			army->flags = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;

				item->goal_anim_state = (Targetable(item, &info) ? (item->current_anim_state == ARMY_AIM1) ? ARMY_SHOOT1 : ARMY_SHOOT3
																 : ARMY_STOP);
			}

			break;
		}
		case ARMY_AIM2:
		{
			army->flags = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;

				item->goal_anim_state = (Targetable(item, &info) ? ARMY_SHOOT2 : ARMY_WALK);
			}

			break;
		}
		case ARMY_SHOOT3:
		{
			if (item->goal_anim_state != ARMY_STOP && (army->mood == ESCAPE_MOOD || info.distance > ARMY_SHOOT1_RANGE || !Targetable(item, &info)))
				item->goal_anim_state = ARMY_STOP;
		}
		case ARMY_SHOOT2:
		case ARMY_SHOOT1:
		{
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			if (!army->flags)
			{
				ShotLara(item, &info, &army_gun, torso_y, ARMY_SHOT_DAMAGE);

				army->flags = 5;
			}
			else --army->flags;
		}
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso_y);
	CreatureJoint(item, 1, torso_x);
	CreatureJoint(item, 2, head);

	CreatureAnimation(item_number, angle, 0);
}