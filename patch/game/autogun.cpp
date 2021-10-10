#include <specific/standard.h>

#include "objects.h"
#include "lara.h"
#include "control.h"
#include "sphere.h"
#include "effect2.h"
#include "lot.h"
#include "people.h"
#include "missile.h"

#define AUTOGUN_DAMAGE		10
#define AUTOGUN_STILL_ANIM	1
#define AUTOGUN_TURN		(ONE_DEGREE * 10)
#define AUTOGUN_RANGE		SQUARE(WALL_L * 5)
#define	AUTOGUN_LEFT_BITE	4
#define	AUTOGUN_RIGHT_BITE	5

enum autogun_anims
{
	AUTOGUN_FIRE,
	AUTOGUN_STILL
};

BITE_INFO autogun_left  = { 110, -30, -530, 2 },
		  autogun_right = { -110, -30, -530, 2 };

void InitialiseAutogun(int16_t item_number)
{
	auto item = &items[item_number];

	item->anim_number = objects[ROBOT_SENTRY_GUN].anim_index + AUTOGUN_STILL_ANIM;
	item->frame_number = anims[item->anim_number].frame_base;
	item->current_anim_state = item->goal_anim_state = AUTOGUN_STILL;
	item->item_flags[0] = 0;
}

void AutogunControl(int16_t item_number)
{
	auto item = &items[item_number];

	if (item->fired_weapon > 1)
	{
		--item->fired_weapon;

		if (GetRandomControl() & 1)
		{
			objects[item->object_number].bite_offset = AUTOGUN_LEFT_BITE;

			phd_PushMatrix();
			{
				PHD_VECTOR pos{ autogun_left.x, autogun_left.y, autogun_left.z };

				GetJointAbsPosition(item, &pos, autogun_left.mesh_num);
				TriggerDynamicLight(pos.x, pos.y, pos.z, (item->fired_weapon << 1) + 8, 24, 16, 4);
			}
			phd_PopMatrix();
		}
		else
		{
			objects[item->object_number].bite_offset = AUTOGUN_RIGHT_BITE;

			phd_PushMatrix();
			{
				PHD_VECTOR pos{ autogun_right.x, autogun_right.y, autogun_right.z };

				GetJointAbsPosition(item, &pos, autogun_right.mesh_num);
				TriggerDynamicLight(pos.x, pos.y, pos.z, (item->fired_weapon << 1) + 8, 24, 16, 4);
			}
			phd_PopMatrix();
		}
	}

	if (!CreatureActive(item_number) || item->data == 0)
		return;

	if (item->hit_status)
		item->really_active = 1;

	auto autogun = (CREATURE_INFO*)item->data;

	if (item->hit_points <= 0)
	{
		ExplodingDeath(item_number, 0xffffffff, 0);
		DisableBaddieAI(item_number);
		KillItem(item_number);

		item->status = DEACTIVATED;
		item->flags |= ONESHOT;
	}

	if (!items[item_number].really_active)
		return;

	AI_INFO info;

	CreatureAIInfo(item, &info);

	int tilt_x = -info.x_angle;

	switch (item->current_anim_state)
	{
	case AUTOGUN_FIRE:
	{
		if (Targetable(item, &info))
		{
			if (item->frame_number == anims[item->anim_number].frame_base)
			{
				item->item_flags[0] = 1;

				if (objects[item->object_number].bite_offset == AUTOGUN_RIGHT_BITE)
					ShotLara(item, &info, &autogun_left, autogun->joint_rotation[0], AUTOGUN_DAMAGE);
				else ShotLara(item, &info, &autogun_right, autogun->joint_rotation[0], AUTOGUN_DAMAGE);

				item->fired_weapon = 10;

				g_audio->play_sound(44, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
			}
		}
		else item->goal_anim_state = AUTOGUN_STILL;

		break;
	}
	case AUTOGUN_STILL:
	{
		if (Targetable(item, &info) && item->item_flags[0] == 0)
			item->goal_anim_state = AUTOGUN_FIRE;
		else if (item->item_flags[0])
		{
			if (item->ai_bits == MODIFY)
			{
				item->item_flags[0] = 1;
				item->goal_anim_state = AUTOGUN_FIRE;
			}
			else
			{
				item->item_flags[0] = 0;
				item->really_active = 0;
			}
		}

		break;
	}
	}

	int change = info.angle - autogun->joint_rotation[0];

	if (change > AUTOGUN_TURN)		 change = AUTOGUN_TURN;
	else if (change < -AUTOGUN_TURN) change = -AUTOGUN_TURN;

	autogun->joint_rotation[0] += change;

	CreatureJoint(item, 1, tilt_x);
	AnimateItem(item);

	if (info.angle > 0x4000)
	{
		item->pos.y_rot -= 0x8000;

		if (info.angle > 0)
			autogun->joint_rotation[0] -= 0x8000;
		else if (info.angle < 0)
			autogun->joint_rotation[0] += 0x8000;
	}
	else if (info.angle < -0x4000)
	{
		item->pos.y_rot += 0x8000;

		if (info.angle > 0)
			autogun->joint_rotation[0] -= 0x8000;
		else if (info.angle < 0)
			autogun->joint_rotation[0] += 0x8000;
	}
}