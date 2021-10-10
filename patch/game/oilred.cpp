#include "objects.h"
#include "lara.h"
#include "control.h"
#include "people.h"
#include "sphere.h"
#include "effect2.h"

#include <specific/standard.h>
#include <specific/fn_stubs.h>

#define OILRED_SHOT_DAMAGE		35
#define OILRED_WALK_TURN		(ONE_DEGREE * 6)
#define OILRED_RUN_TURN			(ONE_DEGREE * 10)
#define OILRED_WALK_RANGE		SQUARE(WALL_L * 2)
#define OILRED_FEELER_DISTANCE	WALL_L
#define OILRED_DIE_ANIM			14
#define OILRED_AIM1_ANIM		12
#define OILRED_AIM2_ANIM		13
#define OILRED_WT1_SHT1_ANIM	1
#define OILRED_WT1_SHT2_ANIM	4
#define OILRED_WALK_WAIT_ANIM	17
#define OILRED_RUN_WAIT1_ANIM	27
#define OILRED_RUN_WAIT2_ANIM	28
#define OILRED_WLK_SHT4A_ANIM	18
#define OILRED_WLK_SHT4B_ANIM	19
#define OILRED_DEATH_SHOT_ANGLE 0x2000
#define OILRED_AWARE_DISTANCE	SQUARE(WALL_L)

enum oilred_anims
{
	OILRED_EMPTY,
	OILRED_WAIT,
	OILRED_WALK,
	OILRED_RUN,
	OILRED_AIM1,
	OILRED_SHOOT1,
	OILRED_AIM2,
	OILRED_SHOOT2,
	OILRED_SHOOT3A,
	OILRED_SHOOT3B,
	OILRED_SHOOT4A,
	OILRED_AIM3,
	OILRED_AIM4,
	OILRED_DEATH,
	OILRED_SHOOT4B,
	OILRED_DUCK,
	OILRED_DUCKED,
	OILRED_DUCKAIM,
	OILRED_DUCKSHOT,
	OILRED_DUCKWALK,
	OILRED_STAND
};

BITE_INFO oilred_gun = { 0, 160, 40, 13 };

void OilRedControl(int16_t item_number)
{
	if (!CreatureActive(item_number))
		return;

	auto item = &items[item_number];
	auto oilred = (CREATURE_INFO*)item->data;

	int16_t torso_y = 0,
		   torso_x = 0,
		   head = 0,
		   angle = 0,
		   tilt = 0;

	if (item->fired_weapon)
	{
		phd_PushMatrix();
		{
			PHD_VECTOR pos { oilred_gun.x, oilred_gun.y, oilred_gun.z };

			GetJointAbsPosition(item, &pos, oilred_gun.mesh_num);
			TriggerDynamicLight(pos.x, pos.y, pos.z, (item->fired_weapon << 1) + 4, 24, 16, 4);
		}
		phd_PopMatrix();
	}

	AI_INFO info;

	if (item->hit_points <= 0)
	{
		item->hit_points = 0;

		if (item->current_anim_state != OILRED_DEATH)
		{
			item->anim_number = objects[OILRED].anim_index + OILRED_DIE_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = OILRED_DEATH;
		}
		else if (item->frame_number == anims[item->anim_number].frame_base + 47)
		{
			CreatureAIInfo(item, &info);

			if (Targetable(item, &info))
			{
				if (info.angle > -OILRED_DEATH_SHOT_ANGLE && info.angle < OILRED_DEATH_SHOT_ANGLE)
				{
					torso_y = info.angle;
					head = info.angle;

					ShotLara(item, &info, &oilred_gun, torso_y, OILRED_SHOT_DAMAGE * 3);
					g_audio->play_sound(72, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
				}
			}
		}
	}
	else
	{
		if (!oilred)
			return;

		AI_INFO lara_info;

		if (item->ai_bits)
			GetAITarget(oilred);
		else oilred->enemy = lara_item;

		CreatureAIInfo(item, &info);

		if (oilred->enemy == lara_item)
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

		int meta_mood = (oilred->enemy != lara_item ? VIOLENT : TIMID);

		GetCreatureMood(item, &info, meta_mood);
		CreatureMood(item, &info, meta_mood);

		angle = CreatureTurn(item, oilred->maximum_turn);

		int x = item->pos.x_pos + (OILRED_FEELER_DISTANCE * phd_sin(item->pos.y_rot + lara_info.angle) >> W2V_SHIFT),
			y = item->pos.y_pos,
			z = item->pos.z_pos + (OILRED_FEELER_DISTANCE * phd_cos(item->pos.y_rot + lara_info.angle) >> W2V_SHIFT);

		auto room_number = item->room_number;
		auto floor = GetFloor(x, y, z, &room_number);

		int height = GetHeight(floor, x, y, z),
			near_cover = (item->pos.y_pos > (height + (STEP_L * 3 / 2)) && item->pos.y_pos < (height + (STEP_L * 9 / 2)) && lara_info.distance > OILRED_AWARE_DISTANCE);

		auto real_enemy = oilred->enemy;

		oilred->enemy = lara_item;

		if ((lara_info.distance < OILRED_AWARE_DISTANCE || item->hit_status || TargetVisible(item, &lara_info)) && !(item->ai_bits & FOLLOW))
		{
			if (!oilred->alerted)
				g_audio->play_sound(300, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

			AlertAllGuards(item_number);
		}

		oilred->enemy = real_enemy;

		switch (item->current_anim_state)
		{
		case OILRED_WAIT:
		{
			head = lara_info.angle;

			if (item->anim_number == objects[OILRED].anim_index + OILRED_WALK_WAIT_ANIM ||
				item->anim_number == objects[OILRED].anim_index + OILRED_RUN_WAIT1_ANIM ||
				item->anim_number == objects[OILRED].anim_index + OILRED_RUN_WAIT2_ANIM)
			{
				if (abs(info.angle) < OILRED_RUN_TURN)
					item->pos.y_rot += info.angle;
				else if (info.angle < 0)
					item->pos.y_rot -= OILRED_RUN_TURN;
				else item->pos.y_rot += OILRED_RUN_TURN;
			}

			oilred->maximum_turn = 0;

			if (item->ai_bits & GUARD)
			{
				head = AIGuard(oilred);
				item->goal_anim_state = OILRED_WAIT;
				break;
			}
			else if (item->ai_bits & PATROL1)
				item->goal_anim_state = OILRED_WALK;
			else if (near_cover && (lara.target == item || item->hit_status))
				item->goal_anim_state = OILRED_DUCK;
			else if (item->required_anim_state == OILRED_DUCK)
				item->goal_anim_state = OILRED_DUCK;
			else if (oilred->mood == ESCAPE_MOOD)
				item->goal_anim_state = OILRED_RUN;
			else if (Targetable(item, &info))
			{
				if (info.distance > OILRED_WALK_RANGE)
					item->goal_anim_state = OILRED_WALK;
				else
				{
					if (int random = GetRandomControl(); random < 0x2000)
						item->goal_anim_state = OILRED_SHOOT1;
					else if (random < 0x4000)
						item->goal_anim_state = OILRED_SHOOT2;
					else item->goal_anim_state = OILRED_AIM3;
				}
			}
			else if (oilred->mood == BORED_MOOD || ((item->ai_bits & FOLLOW) && (oilred->reached_goal || lara_info.distance > SQUARE(WALL_L * 2))))
				item->goal_anim_state = (info.ahead ? OILRED_WAIT : OILRED_WALK);
			else item->goal_anim_state = OILRED_RUN;

			break;
		}
		case OILRED_WALK:
		{
			oilred->maximum_turn = OILRED_WALK_TURN;

			head = lara_info.angle;

			if (item->ai_bits & PATROL1)
			{
				item->goal_anim_state = OILRED_WALK;
				head = 0;
			}
			else if (near_cover && (lara.target == item || item->hit_status))
			{
				item->required_anim_state = OILRED_DUCK;
				item->goal_anim_state = OILRED_WAIT;
			}
			else if (oilred->mood == ESCAPE_MOOD)
				item->goal_anim_state = OILRED_RUN;
			else if (Targetable(item, &info))
				item->goal_anim_state = (info.distance > OILRED_WALK_RANGE && info.zone_number == info.enemy_zone ? OILRED_AIM4 : OILRED_WAIT);
			else if (oilred->mood == BORED_MOOD)
				item->goal_anim_state = (info.ahead ? OILRED_WALK : OILRED_WAIT);
			else item->goal_anim_state = OILRED_RUN;

			break;
		}
		case OILRED_RUN:
		{
			oilred->maximum_turn = OILRED_RUN_TURN;

			if (info.ahead)
				head = info.angle;

			tilt = angle / 2;

			if (item->ai_bits & GUARD)
				item->goal_anim_state = OILRED_WAIT;
			else if (near_cover && (lara.target == item || item->hit_status))
			{
				item->required_anim_state = OILRED_DUCK;
				item->goal_anim_state = OILRED_WAIT;
			}
			else if (oilred->mood == ESCAPE_MOOD)
				break;
			else if (Targetable(item, &info) || ((item->ai_bits & FOLLOW) && (oilred->reached_goal || lara_info.distance > SQUARE(WALL_L * 2))))
				item->goal_anim_state = OILRED_WAIT;
			else if (oilred->mood == BORED_MOOD)
				item->goal_anim_state = OILRED_WALK;

			break;
		}
		case OILRED_AIM1:
		{
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			if ((item->anim_number == objects[OILRED].anim_index + OILRED_AIM1_ANIM) || (item->anim_number == objects[OILRED].anim_index + OILRED_WT1_SHT1_ANIM && item->frame_number == anims[item->anim_number].frame_base + 10))
			{
				if (!ShotLara(item, &info, &oilred_gun, torso_y, OILRED_SHOT_DAMAGE))
					item->required_anim_state = OILRED_WAIT;
			}
			else if (item->hit_status && !(GetRandomControl() & 0x3) && near_cover)
			{
				item->required_anim_state = OILRED_DUCK;
				item->goal_anim_state = OILRED_WAIT;
			}

			break;
		}
		case OILRED_SHOOT1:
		{
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			if (item->required_anim_state == OILRED_WAIT)
				item->goal_anim_state = OILRED_WAIT;

			break;
		}
		case OILRED_SHOOT2:
		{
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			if (item->frame_number == anims[item->anim_number].frame_base)
			{
				if (!ShotLara(item, &info, &oilred_gun, torso_y, OILRED_SHOT_DAMAGE))
					item->goal_anim_state = OILRED_WAIT;
			}
			else if (item->hit_status && !(GetRandomControl() & 0x3) && near_cover)
			{
				item->required_anim_state = OILRED_DUCK;
				item->goal_anim_state = OILRED_WAIT;
			}

			break;
		}
		case OILRED_SHOOT3A:
		case OILRED_SHOOT3B:
		{
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			if (item->frame_number == anims[item->anim_number].frame_base || (item->frame_number == anims[item->anim_number].frame_base + 11))
			{
				if (!ShotLara(item, &info, &oilred_gun, torso_y, OILRED_SHOT_DAMAGE))
					item->goal_anim_state = OILRED_WAIT;
			}
			else if (item->hit_status && !(GetRandomControl() & 0x3) && near_cover)
			{
				item->required_anim_state = OILRED_DUCK;
				item->goal_anim_state = OILRED_WAIT;
			}

			break;
		}
		case OILRED_AIM4:
		{
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			if ((item->anim_number == objects[OILRED].anim_index + OILRED_WLK_SHT4A_ANIM && item->frame_number == anims[item->anim_number].frame_base + 17) || (item->anim_number == objects[OILRED].anim_index + OILRED_WLK_SHT4B_ANIM && item->frame_number == anims[item->anim_number].frame_base + 6)) // Cheers, Phil :)
			{
				if (!ShotLara(item, &info, &oilred_gun, torso_y, OILRED_SHOT_DAMAGE))
					item->required_anim_state = OILRED_WALK;
			}
			else if (item->hit_status && !(GetRandomControl() & 0x3) && near_cover)
			{
				item->required_anim_state = OILRED_DUCK;
				item->goal_anim_state = OILRED_WAIT;
			}

			if (info.distance < OILRED_WALK_RANGE)
				item->required_anim_state = OILRED_WALK;

			break;
		}
		case OILRED_SHOOT4A:
		case OILRED_SHOOT4B:
		{
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			if (item->required_anim_state == OILRED_WALK)
				item->goal_anim_state = OILRED_WALK;

			if (item->frame_number == anims[item->anim_number].frame_base + 16 && !ShotLara(item, &info, &oilred_gun, torso_y, OILRED_SHOT_DAMAGE))
				item->goal_anim_state = OILRED_WALK;
			
			if (info.distance < OILRED_WALK_RANGE)
				item->goal_anim_state = OILRED_WALK;

			break;
		}
		case OILRED_DUCKED:
		{
			oilred->maximum_turn = 0;

			if (info.ahead)
				head = info.angle;

			if (Targetable(item, &info))
				item->goal_anim_state = OILRED_DUCKAIM;
			else if (item->hit_status || !near_cover || (info.ahead && !(GetRandomControl() & 0x1F)))
				item->goal_anim_state = OILRED_STAND;
			else item->goal_anim_state = OILRED_DUCKWALK;

			break;
		}
		case OILRED_DUCKAIM:
		{
			oilred->maximum_turn = ONE_DEGREE;

			if (info.ahead)
				torso_y = info.angle;

			item->goal_anim_state = (Targetable(item, &info) ? OILRED_DUCKSHOT : OILRED_DUCKED);

			break;
		}
		case OILRED_DUCKSHOT:
		{
			if (info.ahead)
				torso_y = info.angle;

			if (item->frame_number == anims[item->anim_number].frame_base && !ShotLara(item, &info, &oilred_gun, torso_y, OILRED_SHOT_DAMAGE) || !(GetRandomControl() & 0x7))
				item->goal_anim_state = OILRED_DUCKED;

			break;
		}
		case OILRED_DUCKWALK:
		{
			if (info.ahead)
				head = info.angle;

			oilred->maximum_turn = OILRED_WALK_TURN;

			if (Targetable(item, &info) || item->hit_status || !near_cover || (info.ahead && !(GetRandomControl() & 0x1F)))
				item->goal_anim_state = OILRED_DUCKED;

			break;
		}
		case OILRED_STAND:
		{
			if (abs(info.angle) < OILRED_WALK_TURN)
				item->pos.y_rot += info.angle;
			else if (info.angle < 0)
				item->pos.y_rot -= OILRED_WALK_TURN;
			else item->pos.y_rot += OILRED_WALK_TURN;
		}
		}
	}

	torso_y = std::clamp<int16_t>(torso_y, -0x2000, 0x2000);

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso_y);
	CreatureJoint(item, 1, torso_x);
	CreatureJoint(item, 2, head);

	CreatureAnimation(item_number, angle, 0);
}