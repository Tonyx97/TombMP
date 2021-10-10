#include "objects.h"
#include "lara.h"
#include "control.h"
#include "people.h"
#include "sphere.h"
#include "effect2.h"

#include <specific/standard.h>
#include <specific/fn_stubs.h>

#define OILSMG_SHOT_DAMAGE		28
#define OILSMG_WALK_TURN		(ONE_DEGREE * 5)
#define OILSMG_RUN_TURN			(ONE_DEGREE * 10)
#define OILSMG_RUN_RANGE		SQUARE(WALL_L * 2)
#define OILSMG_SHOOT1_RANGE		SQUARE(WALL_L * 3)
#define OILSMG_DIE_ANIM			19
#define OILSMG_STOP_ANIM		12
#define OILSMG_WALK_STOP_ANIM	17
#define OILSMG_DEATH_SHOT_ANGLE 0x2000
#define OILSMG_AWARE_DISTANCE	SQUARE(WALL_L)

enum oilsmg_anims
{
	OILSMG_EMPTY,
	OILSMG_STOP,
	OILSMG_WALK,
	OILSMG_RUN,
	OILSMG_WAIT,
	OILSMG_SHOOT1,
	OILSMG_SHOOT2,
	OILSMG_DEATH,
	OILSMG_AIM1,
	OILSMG_AIM2,
	OILSMG_AIM3,
	OILSMG_SHOOT3
};

BITE_INFO oilsmg_gun = { 0, 400, 64, 7 };

void InitialiseOilSMG(int16_t item_number)
{
	auto item = &items[item_number];

	InitialiseCreature(item_number);

	item->anim_number = objects[WHITE_SOLDIER].anim_index + OILSMG_STOP_ANIM;
	item->frame_number = anims[item->anim_number].frame_base;
	item->current_anim_state = item->goal_anim_state = OILSMG_STOP;
}

void OilSMGControl(int16_t item_number)
{
	if (!CreatureActive(item_number))
		return;

	auto item = &items[item_number];
	auto oilsmg = (CREATURE_INFO*)item->data;

	int16_t torso_y = 0,
		   torso_x = 0,
		   head = 0,
		   angle = 0,
		   tilt = 0;

	if (item->fired_weapon)
	{
		phd_PushMatrix();
		{
			PHD_VECTOR pos { oilsmg_gun.x, oilsmg_gun.y, oilsmg_gun.z };

			GetJointAbsPosition(item, &pos, oilsmg_gun.mesh_num);
			TriggerDynamicLight(pos.x, pos.y, pos.z, (item->fired_weapon << 1) + 8, 24, 16, 4);
		}
		phd_PopMatrix();
	}

	AI_INFO info;

	if (item->hit_points <= 0)
	{
		if (item->current_anim_state != OILSMG_DEATH)
		{
			item->anim_number = objects[item->object_number].anim_index + OILSMG_DIE_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = OILSMG_DEATH;

			if (oilsmg)
				oilsmg->flags = !(GetRandomControl() & 3);
		}
		else if (oilsmg->flags && (item->frame_number > anims[item->anim_number].frame_base + 3 && item->frame_number < anims[item->anim_number].frame_base + 31) && !(item->frame_number & 0x3))
		{
			CreatureAIInfo(item, &info);
	
			torso_y = info.angle;
			head = info.angle;

			ShotLara(item, &info, &oilsmg_gun, 0, 0);
			g_audio->play_sound(72, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
		}
	}
	else
	{
		AI_INFO lara_info;

		if (item->ai_bits)
			GetAITarget(oilsmg);
		else oilsmg->enemy = lara_item;

		CreatureAIInfo(item, &info);

		if (oilsmg->enemy == lara_item)
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

		int meta_mood = oilsmg->enemy != lara_item ? VIOLENT : TIMID;

		GetCreatureMood(item, &info, meta_mood);
		CreatureMood(item, &info, meta_mood);

		angle = CreatureTurn(item, oilsmg->maximum_turn);

		auto real_enemy = oilsmg->enemy;

		oilsmg->enemy = lara_item;

		if ((lara_info.distance < OILSMG_AWARE_DISTANCE || item->hit_status || TargetVisible(item, &lara_info)) && !(item->ai_bits & FOLLOW))
		{
			if (!oilsmg->alerted)
				g_audio->play_sound(300, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

			AlertAllGuards(item_number);
		}

		oilsmg->enemy = real_enemy;

		switch (item->current_anim_state)
		{
		case OILSMG_STOP:
		{
			head = lara_info.angle;

			oilsmg->flags = 0;
			oilsmg->maximum_turn = 0;

			if (item->anim_number == objects[item->object_number].anim_index + OILSMG_WALK_STOP_ANIM)
			{
				if (abs(info.angle) < OILSMG_RUN_TURN)  item->pos.y_rot += info.angle;
				else if (info.angle < 0)				item->pos.y_rot -= OILSMG_RUN_TURN;
				else									item->pos.y_rot += OILSMG_RUN_TURN;
			}

			if (!(item->ai_bits & GUARD))
			{
				if (item->ai_bits & PATROL1)
					item->goal_anim_state = OILSMG_WALK;
				else if (oilsmg->mood == ESCAPE_MOOD)
					item->goal_anim_state = OILSMG_RUN;
				else if (Targetable(item, &info))
				{
					if (info.distance < OILSMG_SHOOT1_RANGE || info.zone_number != info.enemy_zone)
						item->goal_anim_state = (GetRandomControl() < 0x4000 ? OILSMG_AIM1 : OILSMG_AIM3);
					else item->goal_anim_state = OILSMG_WALK;
				}
				else if (oilsmg->mood == BORED_MOOD || ((item->ai_bits & FOLLOW) && (oilsmg->reached_goal || lara_info.distance > SQUARE(WALL_L * 2))))
					item->goal_anim_state = OILSMG_STOP;
				else if (oilsmg->mood != BORED_MOOD && info.distance > OILSMG_RUN_RANGE)
					item->goal_anim_state = OILSMG_RUN;
				else item->goal_anim_state = OILSMG_WALK;
			}
			else
			{
				head = AIGuard(oilsmg);

				if (!(GetRandomControl() & 0xFF))
					item->goal_anim_state = (item->current_anim_state == OILSMG_STOP ? OILSMG_WAIT : OILSMG_STOP);
			}

			break;
		}
		case OILSMG_WAIT:
		{
			head = lara_info.angle;

			oilsmg->flags = 0;
			oilsmg->maximum_turn = 0;

			if (!(item->ai_bits & GUARD))
			{
				if (Targetable(item, &info))
					item->goal_anim_state = OILSMG_SHOOT1;
				else if (oilsmg->mood != BORED_MOOD || !info.ahead)
					item->goal_anim_state = OILSMG_STOP;
			}
			else
			{
				head = AIGuard(oilsmg);

				if (!(GetRandomControl() & 0xFF))
					item->goal_anim_state = (item->current_anim_state == OILSMG_STOP ? OILSMG_WAIT : OILSMG_STOP);
			}

			break;
		}
		case OILSMG_WALK:
		{
			head = lara_info.angle;

			oilsmg->flags = 0;
			oilsmg->maximum_turn = OILSMG_WALK_TURN;

			if (item->ai_bits & PATROL1)
			{
				item->goal_anim_state = OILSMG_WALK;
				head = 0;
			}
			else if (oilsmg->mood == ESCAPE_MOOD)
				item->goal_anim_state = OILSMG_RUN;
			else if ((item->ai_bits & GUARD) || ((item->ai_bits & FOLLOW) && (oilsmg->reached_goal || lara_info.distance > SQUARE(WALL_L * 2))))
				item->goal_anim_state = OILSMG_STOP;
			else if (Targetable(item, &info))
				item->goal_anim_state = (info.distance < OILSMG_SHOOT1_RANGE || info.zone_number != info.enemy_zone ? OILSMG_STOP : OILSMG_AIM2);
			else if (oilsmg->mood == BORED_MOOD && info.ahead)
				item->goal_anim_state = OILSMG_STOP;
			else if (oilsmg->mood != BORED_MOOD && info.distance > OILSMG_RUN_RANGE)
				item->goal_anim_state = OILSMG_RUN;

			break;
		}
		case OILSMG_RUN:
		{
			if (info.ahead)
				head = info.angle;

			oilsmg->maximum_turn = OILSMG_RUN_TURN;
			tilt = angle / 2;

			if ((item->ai_bits & GUARD) || ((item->ai_bits & FOLLOW) && (oilsmg->reached_goal || lara_info.distance > SQUARE(WALL_L * 2))))
				item->goal_anim_state = OILSMG_WALK;
			else if (oilsmg->mood == ESCAPE_MOOD)
				break;
			else if (Targetable(item, &info))
				item->goal_anim_state = OILSMG_WALK;
			else if (oilsmg->mood == BORED_MOOD || (oilsmg->mood == STALK_MOOD && !(item->ai_bits & FOLLOW) && info.distance < OILSMG_RUN_RANGE))
				item->goal_anim_state = OILSMG_WALK;

			break;
		}
		case OILSMG_AIM1:
		case OILSMG_AIM3:
		{
			oilsmg->flags = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;

				if (Targetable(item, &info))
					item->goal_anim_state = (item->current_anim_state == OILSMG_AIM1 ? OILSMG_SHOOT1 : OILSMG_SHOOT3);
				else item->goal_anim_state = OILSMG_STOP;
			}

			break;
		}
		case OILSMG_AIM2:
		{
			oilsmg->flags = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;

				item->goal_anim_state = (Targetable(item, &info) ? OILSMG_SHOOT2 : OILSMG_WALK);
			}

			break;
		}
		case OILSMG_SHOOT3:
		{
			if (item->goal_anim_state != OILSMG_STOP && (oilsmg->mood == ESCAPE_MOOD || info.distance > OILSMG_SHOOT1_RANGE || !Targetable(item, &info)))
				item->goal_anim_state = OILSMG_STOP;
		}
		case OILSMG_SHOOT2:
		case OILSMG_SHOOT1:
		{
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			if (!oilsmg->flags)
			{
				ShotLara(item, &info, &oilsmg_gun, torso_y, OILSMG_SHOT_DAMAGE);

				oilsmg->flags = 5;
			}
			else --oilsmg->flags;

			break;
		}
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso_y);
	CreatureJoint(item, 1, torso_x);
	CreatureJoint(item, 2, head);
	CreatureAnimation(item_number, angle, 0);
}