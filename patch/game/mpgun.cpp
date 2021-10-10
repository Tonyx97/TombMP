#include "objects.h"
#include "lara.h"
#include "control.h"
#include "sphere.h"
#include "people.h"
#include "effect2.h"
#include "lot.h"

#include <specific/standard.h>
#include <specific/fn_stubs.h>

#define MPGUN_SHOT_DAMAGE		32
#define MPGUN_WALK_TURN			(ONE_DEGREE * 6)
#define MPGUN_RUN_TURN			(ONE_DEGREE * 10)
#define MPGUN_WALK_RANGE		SQUARE(WALL_L * 3/ 2)
#define MPGUN_FEELER_DISTANCE	WALL_L
#define MPGUN_DIE_ANIM			14
#define MPGUN_AIM1_ANIM			12
#define MPGUN_AIM2_ANIM			13
#define MPGUN_WT1_SHT1_ANIM		1
#define MPGUN_WT1_SHT2_ANIM		4
#define MPGUN_WLK_SHT4A_ANIM	18
#define MPGUN_WLK_SHT4B_ANIM	19
#define MPGUN_WALK_WAIT_ANIM	17
#define MPGUN_RUN_WAIT1_ANIM	27
#define MPGUN_RUN_WAIT2_ANIM	28
#define MPGUN_DEATH_SHOT_ANGLE	0x2000
#define MPGUN_AWARE_DISTANCE	SQUARE(WALL_L)

enum mpgun_anims
{
	MPGUN_EMPTY,
	MPGUN_WAIT,
	MPGUN_WALK,
	MPGUN_RUN,
	MPGUN_AIM1,
	MPGUN_SHOOT1,
	MPGUN_AIM2,
	MPGUN_SHOOT2,
	MPGUN_SHOOT3A,
	MPGUN_SHOOT3B,
	MPGUN_SHOOT4A,
	MPGUN_AIM3,
	MPGUN_AIM4,
	MPGUN_DEATH,
	MPGUN_SHOOT4B,
	MPGUN_DUCK,
	MPGUN_DUCKED,
	MPGUN_DUCKAIM,
	MPGUN_DUCKSHOT,
	MPGUN_DUCKWALK,
	MPGUN_STAND
};

BITE_INFO mpgun_gun = { 0, 160, 40, 13 };

void MPGunControl(int16_t item_number)
{
	if (!CreatureActive(item_number))
		return;

	auto item = &items[item_number];
	auto mpgun = (CREATURE_INFO*)item->data;

	if (!mpgun)
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
			PHD_VECTOR pos { mpgun_gun.x, mpgun_gun.y, mpgun_gun.z };

			GetJointAbsPosition(item, &pos, mpgun_gun.mesh_num);
			TriggerDynamicLight(pos.x, pos.y, pos.z, (item->fired_weapon << 1) + 4, 24, 16, 4);
		}
		phd_PopMatrix();
	}

	if (boxes[item->box_number].overlap_index & BLOCKED)
	{
		DoLotsOfBloodD(item->pos.x_pos, item->pos.y_pos - (GetRandomControl() & 255) - 32, item->pos.z_pos, (GetRandomControl() & 127) + 128, GetRandomControl() << 1, item->room_number, 3);
		item->hit_points -= 20;
	}

	AI_INFO info;

	if (item->hit_points <= 0)
	{
		item->hit_points = 0;

		if (item->current_anim_state != MPGUN_DEATH)
		{
			item->anim_number = objects[item->object_number].anim_index + MPGUN_DIE_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = MPGUN_DEATH;
		}
		else if (!(GetRandomControl() & 3) && item->frame_number == anims[item->anim_number].frame_base + 1)
		{
			CreatureAIInfo(item, &info);

			if (Targetable(item, &info))
			{
				if (info.angle > -MPGUN_DEATH_SHOT_ANGLE && info.angle < MPGUN_DEATH_SHOT_ANGLE)
				{
					torso_y = info.angle;
					head = info.angle;

					ShotLara(item, &info, &mpgun_gun, torso_y, MPGUN_SHOT_DAMAGE);
					g_audio->play_sound(72, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
				}
			}
		}
	}
	else
	{
		AI_INFO lara_info;

		if (item->ai_bits)
			GetAITarget(mpgun);
		else
		{
			mpgun->enemy = lara_item;
			
			int lara_dz = lara_item->pos.z_pos - item->pos.z_pos,
				lara_dx = lara_item->pos.x_pos - item->pos.x_pos;

			lara_info.distance = lara_dz * lara_dz + lara_dx * lara_dx;

			auto cinfo = baddie_slots;

			for (int i = 0; i < NUM_SLOTS; ++i, ++cinfo)
			{
				if (cinfo->item_num == NO_ITEM || cinfo->item_num == item_number)
					continue;

				auto target = &items[cinfo->item_num];

				if (target->object_number != LARA && target->object_number != BOB)
					continue;

				int x = target->pos.x_pos - item->pos.x_pos,
					z = target->pos.z_pos - item->pos.z_pos,
					distance = x * x + z * z;

				if (distance < lara_info.distance)
					mpgun->enemy = target;
			}

		}

		CreatureAIInfo(item, &info);

		if (mpgun->enemy == lara_item)
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

		int meta_mood = (mpgun->enemy != lara_item ? VIOLENT : TIMID);

		GetCreatureMood(item, &info, meta_mood);
		CreatureMood(item, &info, meta_mood);

		angle = CreatureTurn(item, mpgun->maximum_turn);

		auto room_number = item->room_number;
		
		int x = item->pos.x_pos + (MPGUN_FEELER_DISTANCE * phd_sin(item->pos.y_rot + lara_info.angle) >> W2V_SHIFT),
			y = item->pos.y_pos,
			z = item->pos.z_pos + (MPGUN_FEELER_DISTANCE * phd_cos(item->pos.y_rot + lara_info.angle) >> W2V_SHIFT);

		auto floor = GetFloor(x, y, z, &room_number);

		int height = GetHeight(floor, x, y, z),
			near_cover = (item->pos.y_pos > (height + (STEP_L * 3 / 2)) && item->pos.y_pos < (height + (STEP_L * 9 / 2)) && lara_info.distance > MPGUN_AWARE_DISTANCE);

		auto real_enemy = mpgun->enemy;

		mpgun->enemy = lara_item;

		if (lara_info.distance < MPGUN_AWARE_DISTANCE || item->hit_status || TargetVisible(item, &lara_info))
		{
			if (!mpgun->alerted)
				g_audio->play_sound(300, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

			AlertAllGuards(item_number);
		}

		mpgun->enemy = real_enemy;

		switch (item->current_anim_state)
		{
		case MPGUN_WAIT:
		{
			mpgun->maximum_turn = 0;

			head = lara_info.angle;

			if (item->anim_number == objects[item->object_number].anim_index + MPGUN_WALK_WAIT_ANIM ||
				item->anim_number == objects[item->object_number].anim_index + MPGUN_RUN_WAIT1_ANIM ||
				item->anim_number == objects[item->object_number].anim_index + MPGUN_RUN_WAIT2_ANIM)
			{
				if (abs(info.angle) < MPGUN_RUN_TURN)
					item->pos.y_rot += info.angle;
				else if (info.angle < 0)
					item->pos.y_rot -= MPGUN_RUN_TURN;
				else item->pos.y_rot += MPGUN_RUN_TURN;
			}

			if (item->ai_bits & GUARD)
			{
				head = AIGuard(mpgun);
				item->goal_anim_state = MPGUN_WAIT;
			}
			else if (item->ai_bits & PATROL1)
			{
				item->goal_anim_state = MPGUN_WALK;
				head = 0;
			}
			else if (near_cover && (lara.target == item || item->hit_status))
				item->goal_anim_state = MPGUN_DUCK;
			else if (item->required_anim_state == MPGUN_DUCK)
				item->goal_anim_state = MPGUN_DUCK;
			else if (mpgun->mood == ESCAPE_MOOD)
				item->goal_anim_state = MPGUN_RUN;
			else if (Targetable(item, &info))
			{
				if (int random = GetRandomControl(); random < 0x2000)
					item->goal_anim_state = MPGUN_SHOOT1;
				else if (random < 0x4000)
					item->goal_anim_state = MPGUN_SHOOT2;
				else item->goal_anim_state = MPGUN_AIM3;
			}
			else if (mpgun->mood == BORED_MOOD || ((item->ai_bits & FOLLOW) && (mpgun->reached_goal || lara_info.distance > SQUARE(WALL_L * 2))))
				item->goal_anim_state = (info.ahead ? MPGUN_WAIT : MPGUN_WALK);
			else item->goal_anim_state = MPGUN_RUN;

			break;
		}
		case MPGUN_WALK:
		{
			mpgun->maximum_turn = MPGUN_WALK_TURN;

			head = lara_info.angle;

			if (item->ai_bits & PATROL1)
			{
				item->goal_anim_state = MPGUN_WALK;
				head = 0;
			}
			else if (near_cover && (lara.target == item || item->hit_status))
			{
				item->required_anim_state = MPGUN_DUCK;
				item->goal_anim_state = MPGUN_WAIT;
			}
			else if (mpgun->mood == ESCAPE_MOOD)
				item->goal_anim_state = MPGUN_RUN;
			else if (Targetable(item, &info))
				item->goal_anim_state = (info.distance > MPGUN_WALK_RANGE && info.zone_number == info.enemy_zone ? MPGUN_AIM4 : MPGUN_WAIT);
			else if (mpgun->mood == BORED_MOOD)
				item->goal_anim_state = (info.ahead ? MPGUN_WALK : MPGUN_WAIT);
			else item->goal_anim_state = MPGUN_RUN;

			break;
		}
		case MPGUN_RUN:
		{
			mpgun->maximum_turn = MPGUN_RUN_TURN;

			if (info.ahead)
				head = info.angle;

			tilt = angle / 2;

			if (item->ai_bits & GUARD)
				item->goal_anim_state = MPGUN_WAIT;
			else if (near_cover && (lara.target == item || item->hit_status))
			{
				item->required_anim_state = MPGUN_DUCK;
				item->goal_anim_state = MPGUN_WAIT;
			}
			else if (mpgun->mood == ESCAPE_MOOD)
				break;
			else if (Targetable(item, &info) || ((item->ai_bits & FOLLOW) && (mpgun->reached_goal || lara_info.distance > SQUARE(WALL_L * 2))))
				item->goal_anim_state = MPGUN_WAIT;
			else if (mpgun->mood == BORED_MOOD)
				item->goal_anim_state = MPGUN_WALK;

			break;
		}
		case MPGUN_AIM1:
		{
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			if ((item->anim_number == objects[MP2].anim_index + MPGUN_AIM1_ANIM) || (item->anim_number == objects[MP2].anim_index + MPGUN_WT1_SHT1_ANIM && item->frame_number == anims[item->anim_number].frame_base + 10))
			{
				if (!ShotLara(item, &info, &mpgun_gun, torso_y, MPGUN_SHOT_DAMAGE))
					item->required_anim_state = MPGUN_WAIT;
			}
			else if (item->hit_status && !(GetRandomControl() & 0x3) && near_cover)
			{
				item->required_anim_state = MPGUN_DUCK;
				item->goal_anim_state = MPGUN_WAIT;
			}

			break;
		}
		case MPGUN_SHOOT1:
		{
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			if (item->required_anim_state == MPGUN_WAIT)
				item->goal_anim_state = MPGUN_WAIT;

			break;
		}
		case MPGUN_SHOOT2:
		{
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			if (item->frame_number == anims[item->anim_number].frame_base)
			{
				if (!ShotLara(item, &info, &mpgun_gun, torso_y, MPGUN_SHOT_DAMAGE))
					item->goal_anim_state = MPGUN_WAIT;
			}
			else if (item->hit_status && !(GetRandomControl() & 0x3) && near_cover)
			{
				item->required_anim_state = MPGUN_DUCK;
				item->goal_anim_state = MPGUN_WAIT;
			}

			break;
		}
		case MPGUN_SHOOT3A:
		case MPGUN_SHOOT3B:
		{
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			if (item->frame_number == anims[item->anim_number].frame_base || (item->frame_number == anims[item->anim_number].frame_base + 11))
			{
				if (!ShotLara(item, &info, &mpgun_gun, torso_y, MPGUN_SHOT_DAMAGE))
					item->goal_anim_state = MPGUN_WAIT;
			}
			else if (item->hit_status && !(GetRandomControl() & 0x3) && near_cover)
			{
				item->required_anim_state = MPGUN_DUCK;
				item->goal_anim_state = MPGUN_WAIT;
			}

			break;
		}
		case MPGUN_AIM4:
		{
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			if ((item->anim_number == objects[MP2].anim_index + MPGUN_WLK_SHT4A_ANIM && item->frame_number == anims[item->anim_number].frame_base + 17) || (item->anim_number == objects[MP2].anim_index + MPGUN_WLK_SHT4B_ANIM && item->frame_number == anims[item->anim_number].frame_base + 6)) // Cheers, Phil :)
			{
				if (!ShotLara(item, &info, &mpgun_gun, torso_y, MPGUN_SHOT_DAMAGE))
					item->required_anim_state = MPGUN_WALK;
			}
			else if (item->hit_status && !(GetRandomControl() & 0x3) && near_cover)
			{
				item->required_anim_state = MPGUN_DUCK;
				item->goal_anim_state = MPGUN_WAIT;
			}

			if (info.distance < MPGUN_WALK_RANGE)
				item->required_anim_state = MPGUN_WALK;

			break;
		}
		case MPGUN_SHOOT4A:
		case MPGUN_SHOOT4B:
		{
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			if (item->required_anim_state == MPGUN_WALK)
				item->goal_anim_state = MPGUN_WALK;

			if (item->frame_number == anims[item->anim_number].frame_base + 16 && !ShotLara(item, &info, &mpgun_gun, torso_y, MPGUN_SHOT_DAMAGE))
				item->goal_anim_state = MPGUN_WALK;

			if (info.distance < MPGUN_WALK_RANGE)
				item->goal_anim_state = MPGUN_WALK;

			break;
		}
		case MPGUN_DUCKED:
		{
			mpgun->maximum_turn = 0;

			if (info.ahead)
				head = info.angle;

			if (Targetable(item, &info))
				item->goal_anim_state = MPGUN_DUCKAIM;
			else if (item->hit_status || !near_cover || (info.ahead && !(GetRandomControl() & 0x1F)))
				item->goal_anim_state = MPGUN_STAND;
			else item->goal_anim_state = MPGUN_DUCKWALK;

			break;
		}
		case MPGUN_DUCKAIM:
		{
			mpgun->maximum_turn = ONE_DEGREE;

			if (info.ahead)
				torso_y = info.angle;

			item->goal_anim_state = (Targetable(item, &info) ? MPGUN_DUCKSHOT : MPGUN_DUCKED);

			break;
		}
		case MPGUN_DUCKSHOT:
		{
			if (info.ahead)
				torso_y = info.angle;

			if (item->frame_number == anims[item->anim_number].frame_base && !ShotLara(item, &info, &mpgun_gun, torso_y, MPGUN_SHOT_DAMAGE) || !(GetRandomControl() & 0x7))
				item->goal_anim_state = MPGUN_DUCKED;

			break;
		}
		case MPGUN_DUCKWALK:
		{
			mpgun->maximum_turn = MPGUN_WALK_TURN;

			if (info.ahead)
				head = info.angle;

			if (Targetable(item, &info) || item->hit_status || !near_cover || (info.ahead && !(GetRandomControl() & 0x1F)))
				item->goal_anim_state = MPGUN_DUCKED;
		}
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso_y);
	CreatureJoint(item, 1, torso_x);
	CreatureJoint(item, 2, head);

	CreatureAnimation(item_number, angle, 0);
}