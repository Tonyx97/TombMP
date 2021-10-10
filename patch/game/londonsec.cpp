#include "objects.h"
#include "lara.h"
#include "control.h"
#include "sphere.h"
#include "people.h"
#include "effect2.h"

#include <specific/standard.h>
#include <specific/fn_stubs.h>

#define LONDSEC_SHOT_DAMAGE			32
#define LONDSEC_WALK_TURN			(ONE_DEGREE * 5)
#define LONDSEC_RUN_TURN			(ONE_DEGREE * 10)
#define LONDSEC_WALK_RANGE			SQUARE(WALL_L * 2)
#define LONDSEC_FEELER_DISTANCE		WALL_L
#define LONDSEC_DIE_ANIM			14
#define LONDSEC_AIM1_ANIM			12
#define LONDSEC_AIM2_ANIM			13
#define LONDSEC_WT1_SHT1_ANIM		1
#define LONDSEC_WT1_SHT2_ANIM		4
#define LONDSEC_WLK_SHT4A_ANIM		18
#define LONDSEC_WLK_SHT4B_ANIM		19
#define LONDSEC_WALK_WAIT_ANIM		17
#define LONDSEC_RUN_WAIT1_ANIM		27
#define LONDSEC_RUN_WAIT2_ANIM		28
#define LONDSEC_DEATH_SHOT_ANGLE	0x2000
#define LONDSEC_AWARE_DISTANCE		SQUARE(WALL_L)

enum londsec_anims
{
	LONDSEC_EMPTY,
	LONDSEC_WAIT,
	LONDSEC_WALK,
	LONDSEC_RUN,
	LONDSEC_AIM1,
	LONDSEC_SHOOT1,
	LONDSEC_AIM2,
	LONDSEC_SHOOT2,
	LONDSEC_SHOOT3A,
	LONDSEC_SHOOT3B,
	LONDSEC_SHOOT4A,
	LONDSEC_AIM3,
	LONDSEC_AIM4,
	LONDSEC_DEATH,
	LONDSEC_SHOOT4B,
	LONDSEC_DUCK,
	LONDSEC_DUCKED,
	LONDSEC_DUCKAIM,
	LONDSEC_DUCKSHOT,
	LONDSEC_DUCKWALK,
	LONDSEC_STAND
};

BITE_INFO londsec_gun = { 0, 160, 40, 13 };

void LondSecControl(int16_t item_number)
{
	if (!CreatureActive(item_number))
		return;

	auto item = &items[item_number];
	auto londsec = (CREATURE_INFO*)item->data;

	if (!londsec)
		return;

	int16_t torso_y = 0,
		   torso_x = 0,
		   head = 0,
		   angle = 0,
		   tilt = 0;

	if (item->fired_weapon)
	{
		phd_PushMatrix();
		{
			PHD_VECTOR pos { londsec_gun.x, londsec_gun.y, londsec_gun.z };

			GetJointAbsPosition(item, &pos, londsec_gun.mesh_num);
			TriggerDynamicLight(pos.x, pos.y, pos.z, (item->fired_weapon << 1) + 4, 24, 16, 4);
		}
		phd_PopMatrix();
	}

	AI_INFO info;

	if (item->hit_points <= 0)
	{
		item->hit_points = 0;

		if (item->current_anim_state != LONDSEC_DEATH)
		{
			item->anim_number = objects[item->object_number].anim_index + LONDSEC_DIE_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = LONDSEC_DEATH;
		}
		else if ((GetRandomControl() & 1) && (item->frame_number == anims[item->anim_number].frame_base + 3 || item->frame_number == anims[item->anim_number].frame_base + 28))
		{
			CreatureAIInfo(item, &info);

			if (Targetable(item, &info))
			{
				if (info.angle > -LONDSEC_DEATH_SHOT_ANGLE && info.angle < LONDSEC_DEATH_SHOT_ANGLE)
				{
					torso_y = info.angle;
					head = info.angle;

					ShotLara(item, &info, &londsec_gun, torso_y, LONDSEC_SHOT_DAMAGE * 2);
					g_audio->play_sound(305, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
				}
			}

		}
	}
	else
	{
		AI_INFO lara_info;

		if (item->ai_bits)
			GetAITarget(londsec);
		else londsec->enemy = lara_item;

		CreatureAIInfo(item, &info);

		if (londsec->enemy == lara_item)
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

		int meta_mood = (londsec->enemy != lara_item ? VIOLENT : TIMID);

		GetCreatureMood(item, &info, meta_mood);
		CreatureMood(item, &info, meta_mood);

		angle = CreatureTurn(item, londsec->maximum_turn);

		auto room_number = item->room_number;

		int x = item->pos.x_pos + (LONDSEC_FEELER_DISTANCE * phd_sin(item->pos.y_rot + lara_info.angle) >> W2V_SHIFT),
			y = item->pos.y_pos,
			z = item->pos.z_pos + (LONDSEC_FEELER_DISTANCE * phd_cos(item->pos.y_rot + lara_info.angle) >> W2V_SHIFT);

		auto floor = GetFloor(x, y, z, &room_number);

		int height = GetHeight(floor, x, y, z),
			near_cover = (item->pos.y_pos > (height + (STEP_L * 3 / 2)) && item->pos.y_pos < (height + (STEP_L * 9 / 2)) && lara_info.distance > LONDSEC_AWARE_DISTANCE);

		auto real_enemy = londsec->enemy;

		londsec->enemy = lara_item;

		if (item->hit_status || ((lara_info.distance < LONDSEC_AWARE_DISTANCE || TargetVisible(item, &lara_info)) && (abs(lara_item->pos.y_pos - item->pos.y_pos) < (WALL_L * 2))))
		{
			if (!londsec->alerted)
				g_audio->play_sound(299, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

			AlertAllGuards(item_number);
		}

		londsec->enemy = real_enemy;

		switch (item->current_anim_state)
		{
		case LONDSEC_WAIT:
		{
			londsec->maximum_turn = 0;

			head = lara_info.angle;

			if (item->anim_number == objects[item->object_number].anim_index + LONDSEC_WALK_WAIT_ANIM ||
				item->anim_number == objects[item->object_number].anim_index + LONDSEC_RUN_WAIT1_ANIM ||
				item->anim_number == objects[item->object_number].anim_index + LONDSEC_RUN_WAIT2_ANIM)
			{
				if (abs(info.angle) < LONDSEC_RUN_TURN)
					item->pos.y_rot += info.angle;
				else if (info.angle < 0)
					item->pos.y_rot -= LONDSEC_RUN_TURN;
				else item->pos.y_rot += LONDSEC_RUN_TURN;
			}

			if (item->ai_bits & GUARD)
			{
				head = AIGuard(londsec);
				item->goal_anim_state = LONDSEC_WAIT;
			}
			else if (item->ai_bits & PATROL1)
			{
				item->goal_anim_state = LONDSEC_WALK;
				head = 0;
			}
			else if (near_cover && (lara.target == item || item->hit_status))
				item->goal_anim_state = LONDSEC_DUCK;
			else if (item->required_anim_state == LONDSEC_DUCK)
				item->goal_anim_state = LONDSEC_DUCK;
			else if (londsec->mood == ESCAPE_MOOD)
				item->goal_anim_state = LONDSEC_RUN;
			else if (Targetable(item, &info))
			{
				if (info.distance > LONDSEC_WALK_RANGE)
					item->goal_anim_state = LONDSEC_WALK;
				else
				{
					if (int random = GetRandomControl(); random < 0x2000)
						item->goal_anim_state = LONDSEC_SHOOT1;
					else if (random < 0x4000)
						item->goal_anim_state = LONDSEC_SHOOT2;
					else item->goal_anim_state = LONDSEC_AIM3;
				}
			}
			else if (londsec->mood == BORED_MOOD || ((item->ai_bits & FOLLOW) && (londsec->reached_goal || lara_info.distance > SQUARE(WALL_L * 2))))
				item->goal_anim_state = (info.ahead ? LONDSEC_WAIT : LONDSEC_WALK);
			else item->goal_anim_state = LONDSEC_RUN;

			break;
		}
		case LONDSEC_WALK:
		{
			londsec->maximum_turn = LONDSEC_WALK_TURN;

			head = lara_info.angle;

			if (item->ai_bits & PATROL1)
			{
				item->goal_anim_state = LONDSEC_WALK;
				head = 0;
			}
			else if (near_cover && (lara.target == item || item->hit_status))
			{
				item->required_anim_state = LONDSEC_DUCK;
				item->goal_anim_state = LONDSEC_WAIT;
			}
			else if (londsec->mood == ESCAPE_MOOD)
				item->goal_anim_state = LONDSEC_RUN;
			else if (Targetable(item, &info))
				item->goal_anim_state = (info.distance > LONDSEC_WALK_RANGE && info.zone_number == info.enemy_zone ? LONDSEC_AIM4 : LONDSEC_WAIT);
			else if (londsec->mood == BORED_MOOD)
				item->goal_anim_state = (info.ahead ? LONDSEC_WALK : LONDSEC_WAIT);
			else item->goal_anim_state = LONDSEC_RUN;

			break;
		}
		case LONDSEC_RUN:
		{
			londsec->maximum_turn = LONDSEC_RUN_TURN;

			if (info.ahead)
				head = info.angle;

			tilt = angle / 2;

			if (item->ai_bits & GUARD)
				item->goal_anim_state = LONDSEC_WAIT;

			else if (near_cover && (lara.target == item || item->hit_status))
			{
				item->required_anim_state = LONDSEC_DUCK;
				item->goal_anim_state = LONDSEC_WAIT;
			}
			else if (londsec->mood == ESCAPE_MOOD)
				break;
			else if (Targetable(item, &info) || ((item->ai_bits & FOLLOW) && (londsec->reached_goal || lara_info.distance > SQUARE(WALL_L * 2))))
				item->goal_anim_state = LONDSEC_WAIT;
			else if (londsec->mood == BORED_MOOD)
				item->goal_anim_state = LONDSEC_WALK;

			break;
		}
		case LONDSEC_AIM1:
		{
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			if ((item->anim_number == objects[SECURITY_GUARD].anim_index + LONDSEC_AIM1_ANIM) || (item->anim_number == objects[SECURITY_GUARD].anim_index + LONDSEC_WT1_SHT1_ANIM && item->frame_number == anims[item->anim_number].frame_base + 10))
			{
				if (!ShotLara(item, &info, &londsec_gun, torso_y, LONDSEC_SHOT_DAMAGE))
					item->required_anim_state = LONDSEC_WAIT;
			}
			else if (item->hit_status && !(GetRandomControl() & 0x3) && near_cover)
			{
				item->required_anim_state = LONDSEC_DUCK;
				item->goal_anim_state = LONDSEC_WAIT;
			}

			break;
		}
		case LONDSEC_SHOOT1:
		{
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			if (item->required_anim_state == LONDSEC_WAIT)
				item->goal_anim_state = LONDSEC_WAIT;

			break;
		}
		case LONDSEC_SHOOT2:
		{
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			if (item->frame_number == anims[item->anim_number].frame_base)
			{
				if (!ShotLara(item, &info, &londsec_gun, torso_y, LONDSEC_SHOT_DAMAGE))
					item->goal_anim_state = LONDSEC_WAIT;
			}
			else if (item->hit_status && !(GetRandomControl() & 0x3) && near_cover)
			{
				item->required_anim_state = LONDSEC_DUCK;
				item->goal_anim_state = LONDSEC_WAIT;
			}

			break;
		}
		case LONDSEC_SHOOT3A:
		case LONDSEC_SHOOT3B:
		{
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			if (item->frame_number == anims[item->anim_number].frame_base || (item->frame_number == anims[item->anim_number].frame_base + 11))
			{
				if (!ShotLara(item, &info, &londsec_gun, torso_y, LONDSEC_SHOT_DAMAGE))
					item->goal_anim_state = LONDSEC_WAIT;
			}
			else if (item->hit_status && !(GetRandomControl() & 0x3) && near_cover)
			{
				item->required_anim_state = LONDSEC_DUCK;
				item->goal_anim_state = LONDSEC_WAIT;
			}

			break;
		}
		case LONDSEC_AIM4:
		{
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			if ((item->anim_number == objects[SECURITY_GUARD].anim_index + LONDSEC_WLK_SHT4A_ANIM && item->frame_number == anims[item->anim_number].frame_base + 17) || (item->anim_number == objects[SECURITY_GUARD].anim_index + LONDSEC_WLK_SHT4B_ANIM && item->frame_number == anims[item->anim_number].frame_base + 6))
			{
				if (!ShotLara(item, &info, &londsec_gun, torso_y, LONDSEC_SHOT_DAMAGE))
					item->required_anim_state = LONDSEC_WALK;
			}
			else if (item->hit_status && !(GetRandomControl() & 0x3) && near_cover)
			{
				item->required_anim_state = LONDSEC_DUCK;
				item->goal_anim_state = LONDSEC_WAIT;
			}

			if (info.distance < LONDSEC_WALK_RANGE)
				item->required_anim_state = LONDSEC_WALK;

			break;
		}
		case LONDSEC_SHOOT4A:
		case LONDSEC_SHOOT4B:
		{
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			if (item->required_anim_state == LONDSEC_WALK)
				item->goal_anim_state = LONDSEC_WALK;

			if (item->frame_number == anims[item->anim_number].frame_base + 16 && !ShotLara(item, &info, &londsec_gun, torso_y, LONDSEC_SHOT_DAMAGE))
				item->goal_anim_state = LONDSEC_WALK;

			if (info.distance < LONDSEC_WALK_RANGE)
				item->goal_anim_state = LONDSEC_WALK;

			break;
		}
		case LONDSEC_DUCKED:
		{
			londsec->maximum_turn = 0;

			if (info.ahead)
				head = info.angle;

			if (Targetable(item, &info))
				item->goal_anim_state = LONDSEC_DUCKAIM;
			else if (item->hit_status || !near_cover || (info.ahead && !(GetRandomControl() & 0x1F)))
				item->goal_anim_state = LONDSEC_STAND;
			else item->goal_anim_state = LONDSEC_DUCKWALK;

			break;
		}
		case LONDSEC_DUCKAIM:
		{
			londsec->maximum_turn = ONE_DEGREE;

			if (info.ahead)
				torso_y = info.angle;

			item->goal_anim_state = (Targetable(item, &info) ? LONDSEC_DUCKSHOT : LONDSEC_DUCKED);

			break;
		}
		case LONDSEC_DUCKSHOT:
		{
			if (info.ahead)
				torso_y = info.angle;

			if (item->frame_number == anims[item->anim_number].frame_base && !ShotLara(item, &info, &londsec_gun, torso_y, LONDSEC_SHOT_DAMAGE) || !(GetRandomControl() & 0x7))
				item->goal_anim_state = LONDSEC_DUCKED;

			break;
		}
		case LONDSEC_DUCKWALK:
		{
			londsec->maximum_turn = LONDSEC_WALK_TURN;

			if (info.ahead)
				head = info.angle;

			if (Targetable(item, &info) || item->hit_status || !near_cover || (info.ahead && !(GetRandomControl() & 0x1F)))
				item->goal_anim_state = LONDSEC_DUCKED;
		}
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso_y);
	CreatureJoint(item, 1, torso_x);
	CreatureJoint(item, 2, head);

	CreatureAnimation(item_number, angle, 0);
}