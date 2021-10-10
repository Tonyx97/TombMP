#include "objects.h"
#include "lara.h"
#include "control.h"
#include "sphere.h"
#include "effect2.h"
#include "people.h"

#include <specific/standard.h>
#include <specific/fn_stubs.h>

#define BLOW_BIFF_DAMAGE		100
#define BLOW_SHOOT_DAMAGE		20
#define BLOW_BIFF_ENEMY_DAMAGE	5
#define BLOWPIPE_SPEED			150
#define BLOW_WALK_TURN			(ONE_DEGREE * 9)
#define BLOW_RUN_TURN			(ONE_DEGREE * 6)
#define BLOW_WAIT_TURN			(ONE_DEGREE * 2)
#define BLOW_PIPE_RANGE			SQUARE(WALL_L * 8)
#define BLOW_CLOSE_RANGE		SQUARE(WALL_L / 2)
#define BLOW_WALK_RANGE			SQUARE(WALL_L * 2)
#define BLOW_AWARE_DISTANCE		(WALL_L)
#define BLOW_HIT_RANGE			(STEP_L * 2)
#define BLOW_SHIFT_CHANCE		0x200
#define BLOW_TOUCH				0x2400
#define BLOW_KNEELING_DIE_ANIM	21
#define BLOW_STANDING_DIE_ANIM	20

enum blow_anims
{
	BLOW_EMPTY,
	BLOW_WAIT1,
	BLOW_WALK,
	BLOW_RUN,
	BLOW_ATTACK1,
	BLOW_ATTACK2,
	BLOW_ATTACK3,
	BLOW_ATTACK4,
	BLOW_AIM3,
	BLOW_DEATH,
	BLOW_ATTACK5,
	BLOW_WAIT2
};

BITE_INFO blow_biff_hit  = { 0, 0, -200, 13 },
		  blow_shoot_hit = { 8, 40, -248, 13 };

void BlowpipeControl(int16_t item_number)
{
	if (!CreatureActive(item_number))
		return;

	auto r = &room[lara_item->room_number];
	auto item = &items[item_number];
	auto blow = (CREATURE_INFO*)item->data;

	int16_t head_x = 0,
		   head_y = 0,
		   torso_x = 0,
		   torso_y = 0,
		   angle = 0,
		   tilt = 0;

	if (item->hit_points <= 0)
	{
		if (item->current_anim_state != BLOW_DEATH)
		{
			item->anim_number = (item->current_anim_state == BLOW_WAIT1 || item->current_anim_state == BLOW_ATTACK1
								 ? objects[item->object_number].anim_index + BLOW_KNEELING_DIE_ANIM
								 : objects[item->object_number].anim_index + BLOW_STANDING_DIE_ANIM);

			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = BLOW_DEATH;
		}
	}
	else
	{
		if (item->ai_bits)
			GetAITarget(blow);

		AI_INFO info;

		CreatureAIInfo(item, &info);

		auto meta_mood = (info.zone_number == info.enemy_zone ? VIOLENT : TIMID);

		GetCreatureMood(item, &info, meta_mood);

		if (item->hit_status && lara.poisoned >= 0x100 && blow->mood == BORED_MOOD)
			blow->mood = ESCAPE_MOOD;

		CreatureMood(item, &info, TIMID);

		angle = CreatureTurn(item, blow->mood == BORED_MOOD ? BLOW_WAIT_TURN : blow->maximum_turn);

		if (info.ahead)
		{
			head_y = info.angle >> 1;
			torso_y = info.angle >> 1;
		}

		if (item->hit_status || (blow->enemy == lara_item && (info.distance < BLOW_AWARE_DISTANCE || TargetVisible(item, &info)) && (abs(lara_item->pos.y_pos - item->pos.y_pos) < (WALL_L * 2)))) //Maybe move this into LONDSEC_WAIT case?
			AlertAllGuards(item_number);

		switch (item->current_anim_state)
		{
		case BLOW_WAIT1:
		{
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle >> 1;
			}

			blow->flags &= 0x0fff;
			blow->maximum_turn = BLOW_WAIT_TURN;

			if (!(item->ai_bits & GUARD))
			{
				if (blow->mood == ESCAPE_MOOD)
					item->goal_anim_state = (lara.target != item && info.ahead && !item->hit_status ? BLOW_WAIT1 : BLOW_RUN);
				else if (info.bite && info.distance < BLOW_CLOSE_RANGE)
					item->goal_anim_state = BLOW_WAIT2;
				else if (info.bite && info.distance < BLOW_WALK_RANGE)
					item->goal_anim_state = BLOW_WALK;
				else if (Targetable(item, &info) && info.distance < BLOW_PIPE_RANGE)
					item->goal_anim_state = BLOW_ATTACK1;
				else if (blow->mood == BORED_MOOD)
				{
					if (GetRandomControl() < BLOW_SHIFT_CHANCE)
						item->goal_anim_state = BLOW_WALK;
				}
				else item->goal_anim_state = BLOW_RUN;
			}
			else
			{
				head_y = AIGuard(blow);
				torso_x = torso_y = 0;

				blow->maximum_turn = 0;

				if (!(GetRandomControl() & 0xFF))
					item->goal_anim_state = BLOW_WAIT2;
			}

			break;
		}
		case BLOW_WAIT2:
		{
			blow->flags &= 0x0fff;
			blow->maximum_turn = BLOW_WAIT_TURN;

			if (!(item->ai_bits & GUARD))
			{
				if (blow->mood == ESCAPE_MOOD)
					item->goal_anim_state = (lara.target != item && info.ahead && !item->hit_status ? BLOW_WAIT1 : BLOW_RUN);
				else if (info.bite && info.distance < BLOW_CLOSE_RANGE)
					item->goal_anim_state = BLOW_ATTACK3;
				else if (info.bite && info.distance < BLOW_WALK_RANGE)
					item->goal_anim_state = BLOW_WALK;
				else if (Targetable(item, &info) && info.distance < BLOW_PIPE_RANGE)
					item->goal_anim_state = BLOW_WAIT1;
				else if (blow->mood == BORED_MOOD && GetRandomControl() < BLOW_SHIFT_CHANCE)
					item->goal_anim_state = BLOW_WALK;
				else item->goal_anim_state = BLOW_RUN;
			}
			else
			{
				head_y = AIGuard(blow);
				torso_x = torso_y = 0;

				blow->maximum_turn = 0;

				if (!(GetRandomControl() & 0xFF))
					item->goal_anim_state = BLOW_WAIT1;
			}

			break;
		}
		case BLOW_WALK:
		{
			blow->maximum_turn = BLOW_WALK_TURN;

			if (info.bite && info.distance < BLOW_CLOSE_RANGE)
				item->goal_anim_state = BLOW_WAIT2;
			else if (info.bite && info.distance < BLOW_WALK_RANGE)
				item->goal_anim_state = BLOW_WALK;
			else if (Targetable(item, &info) && info.distance < BLOW_PIPE_RANGE)
				item->goal_anim_state = BLOW_WAIT1;
			else if (blow->mood == ESCAPE_MOOD)
				item->goal_anim_state = BLOW_RUN;
			else if (blow->mood == BORED_MOOD)
			{
				if (GetRandomControl() > BLOW_SHIFT_CHANCE)
					item->goal_anim_state = BLOW_WALK;
				else if (GetRandomControl() > BLOW_SHIFT_CHANCE)
					item->goal_anim_state = BLOW_WAIT2;
				else item->goal_anim_state = BLOW_WAIT1;
			}
			else if (info.distance > BLOW_WALK_RANGE)
				item->goal_anim_state = BLOW_RUN;

			break;
		}
		case BLOW_RUN:
		{
			blow->flags &= 0x0fff;
			blow->maximum_turn = BLOW_RUN_TURN;

			tilt = angle >> 2;

			if (info.bite && info.distance < BLOW_CLOSE_RANGE)
				item->goal_anim_state = BLOW_WAIT2;
			else if (Targetable(item, &info) && info.distance < BLOW_PIPE_RANGE)
				item->goal_anim_state = BLOW_WAIT1;
			if (item->ai_bits & GUARD)
				item->goal_anim_state = BLOW_WAIT2;
			else if (blow->mood == ESCAPE_MOOD && lara.target != item && info.ahead)
				item->goal_anim_state = BLOW_WAIT2;
			else if (blow->mood == BORED_MOOD)
				item->goal_anim_state = BLOW_WAIT1;

			break;
		}
		case BLOW_AIM3:
			item->goal_anim_state = (!info.bite || info.distance > BLOW_CLOSE_RANGE ? BLOW_WAIT2 : BLOW_ATTACK3);
			break;
		case BLOW_ATTACK1:
		{
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			blow->maximum_turn = 0;

			if (abs(info.angle) < BLOW_WAIT_TURN)
				item->pos.y_rot += info.angle;
			else if (info.angle < 0)
				item->pos.y_rot -= BLOW_WAIT_TURN;
			else item->pos.y_rot += BLOW_WAIT_TURN;

			if (item->frame_number == anims[item->anim_number].frame_base + 15)
			{
				if (auto dart_num = CreateItem(); dart_num != NO_ITEM)
				{
					auto dart = &items[dart_num];

					dart->object_number = DARTS;
					dart->room_number = item->room_number;

					auto bite = &blow_shoot_hit;

					PHD_VECTOR pos1 { bite->x, bite->y, bite->z },
							   pos2 { bite->x, bite->y, bite->z << 1 };

					GetJointAbsPosition(item, &pos1, bite->mesh_num);
					GetJointAbsPosition(item, &pos2, bite->mesh_num);

					auto angles = phd_GetVectorAngles({ pos2.x - pos1.x, pos2.y - pos1.y, pos2.z - pos1.z });

					dart->pos.x_pos = pos1.x;
					dart->pos.y_pos = pos1.y;
					dart->pos.z_pos = pos1.z;

					InitialiseItem(dart_num);

					dart->pos.x_rot = angles.y;
					dart->pos.y_rot = angles.x;
					dart->speed = 256;

					AddActiveItem(dart_num);

					dart->status = ACTIVE;

					pos1 = { bite->x, bite->y, bite->z + 96};

					GetJointAbsPosition(item, &pos1, bite->mesh_num);

					for (int i = 0; i < 2; ++i)
						TriggerDartSmoke(pos1.x, pos1.y, pos1.z, 0, 0, 1);
				}

				item->goal_anim_state = BLOW_WAIT1;
			}

			break;
		}
		case BLOW_ATTACK3:
		{
			if (auto enemy = blow->enemy; enemy == lara_item)
			{
				if (!(blow->flags & 0xf000) && (item->touch_bits & BLOW_TOUCH))
				{
					lara_item->hit_points -= BLOW_BIFF_DAMAGE;
					lara_item->hit_status = 1;

					blow->flags |= 0x1000;

					g_audio->play_sound(70, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
					CreatureEffect(item, &blow_biff_hit, DoBloodSplat);
				}
			}
			else
			{
				if (!(blow->flags & 0xf000) && enemy)
				{
					if (ABS(enemy->pos.x_pos - item->pos.x_pos) < BLOW_HIT_RANGE &&
						 ABS(enemy->pos.y_pos - item->pos.y_pos) < BLOW_HIT_RANGE &&
						 ABS(enemy->pos.z_pos - item->pos.z_pos) < BLOW_HIT_RANGE)
					{
						enemy->hit_points -= BLOW_BIFF_ENEMY_DAMAGE;
						enemy->hit_status = 1;

						blow->flags |= 0x1000;

						g_audio->play_sound(70, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
					}
				}
			}
		}
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso_y);
	CreatureJoint(item, 1, torso_x);
	CreatureJoint(item, 2, head_y - torso_y);
	CreatureJoint(item, 3, head_x);

	CreatureAnimation(item_number, angle, 0);
}