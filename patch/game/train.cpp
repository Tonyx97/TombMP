#include <specific/standard.h>

#include "objects.h"
#include "lara.h"
#include "laraanim.h"
#include "sphere.h"
#include "control.h"
#include "camera.h"
#include "effect2.h"

#define TRAIN_VEL 260

int32_t TestHeight(ITEM_INFO* item, int32_t x, int32_t z, int16_t* room_number)
{
	int c = phd_cos(item->pos.y_rot),
		s = phd_sin(item->pos.y_rot);

	PHD_VECTOR pos
	{
		item->pos.x_pos + (((z * s) + (x * c)) >> W2V_SHIFT),
		item->pos.y_pos - (z * phd_sin(item->pos.x_rot) >> W2V_SHIFT) + (x * phd_sin(item->pos.z_rot) >> W2V_SHIFT) ,
		item->pos.z_pos + (((z * c) - (x * s)) >> W2V_SHIFT)
	};

	*room_number = item->room_number;

	return GetHeight(GetFloor(pos.x, pos.y, pos.z, room_number), pos.x, pos.y, pos.z);
}

void TrainControl(int16_t train_number)
{
	auto t = &items[train_number];

	if (!TriggerActive(t))
		return;

	if (t->item_flags[0] == 0)
		t->item_flags[0] = t->item_flags[1] = TRAIN_VEL;

	int c = phd_cos(t->pos.y_rot),
		s = phd_sin(t->pos.y_rot);

	t->pos.x_pos += ((t->item_flags[1] * s) >> W2V_SHIFT);
	t->pos.z_pos += ((t->item_flags[1] * c) >> W2V_SHIFT);

	int16_t room_number;

	int rh = TestHeight(t, 0, 5120, &room_number),
		fh = t->pos.y_pos = TestHeight(t, 0, 0, &room_number);
	
	if (fh == NO_HEIGHT)
	{
		KillItem(train_number);
		return;
	}

	t->pos.y_pos -= 32;

	room_number = t->room_number;

	GetFloor(t->pos.x_pos, t->pos.y_pos, t->pos.z_pos, &room_number);

	if (room_number != t->room_number)
		ItemNewRoom(train_number, room_number);

	t->pos.x_rot = -(rh - fh) << 1;

	TriggerDynamicLight(t->pos.x_pos + ((3072 * s) >> W2V_SHIFT), t->pos.y_pos, t->pos.z_pos + ((3072 * c) >> W2V_SHIFT), 16, 31, 31, 31);

	if (t->item_flags[1] != TRAIN_VEL)
	{
		if ((t->item_flags[1] -= 48) < 0)
			t->item_flags[1] = 0;

		if (!UseForcedFixedCamera && lara_item->hit_points <= 0)
		{
			ForcedFixedCamera.x = t->pos.x_pos + ((8192 * s) >> W2V_SHIFT);
			ForcedFixedCamera.z = t->pos.z_pos + ((8192 * c) >> W2V_SHIFT);
			
			room_number = t->room_number;
			auto floor = GetFloor(ForcedFixedCamera.x, t->pos.y_pos - 512, ForcedFixedCamera.z, &room_number);

			ForcedFixedCamera.y = GetHeight(floor, ForcedFixedCamera.x, t->pos.y_pos - 512, ForcedFixedCamera.z);
			ForcedFixedCamera.room_number = room_number;

			UseForcedFixedCamera = 1;
		}
	}
	else g_audio->play_sound(148, { t->pos.x_pos, t->pos.y_pos, t->pos.z_pos });
}

void TrainCollision(int16_t train_number, ITEM_INFO* l, COLL_INFO* coll)
{
	auto t = &items[train_number];

	if (!TestBoundsCollide(t, l, coll->radius) || !TestCollision(t, l))
		return;

	g_audio->play_sound(41, { l->pos.x_pos, l->pos.y_pos, l->pos.z_pos });
	g_audio->play_sound(53, { l->pos.x_pos, l->pos.y_pos, l->pos.z_pos });
	g_audio->stop_sound(148);

	l->anim_number = objects[LARA_EXTRA].anim_index;
	l->frame_number = anims[l->anim_number].frame_base;
	l->current_anim_state = EXTRA_TRAINKILL;
	l->goal_anim_state = EXTRA_TRAINKILL;
	l->hit_points = 0;
	l->pos.y_rot = t->pos.y_rot;
	l->fallspeed = 0;
	l->gravity_status = 0;
	l->speed = 0;

	AnimateItem(l);

	lara.extra_anim = 1;
	lara.gun_status = LG_HANDSBUSY;
	lara.gun_type = LG_UNARMED;
	lara.hit_direction = -1;
	lara.air = -1;

	t->item_flags[1] = 160;

	int c = phd_cos(t->pos.y_rot),
		s = phd_sin(t->pos.y_rot),
		x = l->pos.x_pos + ((256 * s) >> W2V_SHIFT),
		z = l->pos.z_pos + ((256 * c) >> W2V_SHIFT);

	DoLotsOfBlood(x, l->pos.y_pos - 512, z, 1024, t->pos.y_rot, l->room_number, 15);
}