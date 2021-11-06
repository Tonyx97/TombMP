#include <specific/standard.h>

#include "lara.h"
#include "control.h"
#include "sphere.h"
#include "effect2.h"

#define CLEANER_TURN	(0x400)
#define CLEANER_SPEED	(STEP_L / 4)
#define CLEANER_TOUCH	(0xfffc)

uint8_t elecspark[3] = { 0, 0, 0 };

void TriggerElectricSparksCleaner(PHD_VECTOR* pos, int16_t item_number, int16_t Node);

void InitialiseCleaner(int16_t item_number)
{
	auto item = &items[item_number];

	item->pos.x_pos &= ~(1023);
	item->pos.z_pos &= ~(1023);
	item->pos.x_pos |= 512;
	item->pos.z_pos |= 512;
	item->item_flags[0] = CLEANER_TURN;
	item->item_flags[1] = 0;
	item->item_flags[2] = CLEANER_SPEED;
	item->collidable = 1;
	item->data = nullptr;
}

void CleanerControl(int16_t item_number)
{
	if (!CreatureActive(item_number) || items[item_number].item_flags[2] == 0)
		return;

	auto item = &items[item_number];
	auto cleaner = (CREATURE_INFO*)item->data;

	int16_t angle = 0;

	if ((item->pos.y_rot & 0x3fff) == 0)
	{
		if (((item->pos.z_pos & 0x3ff) == 512 && (item->pos.y_rot == 0x0 || item->pos.y_rot == -0x8000)) ||
			((item->pos.x_pos & 0x3ff) == 512 && (item->pos.y_rot == 0x4000 || item->pos.y_rot == -0x4000)))
		{
			int16_t room_number = item->room_number;

			if (item->item_flags[1] == 1)
			{
				int x = item->pos.x_pos + ((1024 * phd_sin(item->pos.y_rot + 0x8000)) >> W2V_SHIFT),
					z = item->pos.z_pos + ((1024 * phd_cos(item->pos.y_rot + 0x8000)) >> W2V_SHIFT);

				GetFloor(x, item->pos.y_pos, z, &room_number);

				auto r = &room[room_number];

				r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size].stopper = 0;

				item->item_flags[1] = 0;
			}

			room_number = item->room_number;

			switch (item->pos.y_rot)
			{
			case 0x0:
			{
				auto floor = GetFloor(item->pos.x_pos - WALL_L, item->pos.y_pos, item->pos.z_pos, &room_number);
				auto r = &room[room_number];

				int height = GetHeight(floor, item->pos.x_pos - WALL_L, item->pos.y_pos, item->pos.z_pos),
					stop = (r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - WALL_L - r->x) >> WALL_SHIFT) * r->x_size].stopper);

				bool left_ok = (height == item->pos.y_pos && stop == 0);

				room_number = item->room_number;
				floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos + WALL_L, &room_number);
				height = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos + WALL_L);
				r = &room[room_number];
				stop = (r->floor[((item->pos.z_pos + WALL_L - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size].stopper);
				
				bool forward_ok = (height == item->pos.y_pos && stop == 0);
				
				if (!forward_ok && !left_ok && item->item_flags[0] > 0)
				{
					item->pos.y_rot += CLEANER_TURN;
					item->item_flags[0] = CLEANER_TURN;
				}
				else if (!forward_ok && !left_ok && item->item_flags[0] < 0)
				{
					item->pos.y_rot -= CLEANER_TURN;
					item->item_flags[0] = -CLEANER_TURN;
				}
				else if (left_ok && item->item_flags[0] > 0)
				{
					item->pos.y_rot -= CLEANER_TURN;
					item->item_flags[0] = -CLEANER_TURN;
				}
				else
				{
					item->item_flags[0] = CLEANER_TURN;
					item->pos.z_pos += item->item_flags[2];
					r->floor[((item->pos.z_pos + WALL_L - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size].stopper = 1;
					item->item_flags[1] = 1;
				}

				break;
			}
			case 0x4000:
			{
				auto floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos + WALL_L, &room_number);
				auto r = &room[room_number];

				int height = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos + WALL_L),
					stop = (r->floor[((item->pos.z_pos + WALL_L - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size].stopper);
				
				bool left_ok = (height == item->pos.y_pos && stop == 0);

				room_number = item->room_number;
				floor = GetFloor(item->pos.x_pos + WALL_L, item->pos.y_pos, item->pos.z_pos, &room_number);
				height = GetHeight(floor, item->pos.x_pos + WALL_L, item->pos.y_pos, item->pos.z_pos);
				r = &room[room_number];
				stop = (r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + ((item->pos.x_pos + WALL_L - r->x) >> WALL_SHIFT) * r->x_size].stopper);
				
				bool forward_ok = (height == item->pos.y_pos && stop == 0);

				if (!forward_ok && !left_ok && item->item_flags[0] > 0)
				{
					item->pos.y_rot += CLEANER_TURN;
					item->item_flags[0] = CLEANER_TURN;
				}
				else if (!forward_ok && !left_ok && item->item_flags[0] < 0)
				{
					item->pos.y_rot -= CLEANER_TURN;
					item->item_flags[0] = -CLEANER_TURN;
				}
				else if (left_ok && item->item_flags[0] > 0)
				{
					item->pos.y_rot -= CLEANER_TURN;
					item->item_flags[0] = -CLEANER_TURN;
				}
				else
				{
					item->item_flags[0] = CLEANER_TURN;
					item->pos.x_pos += item->item_flags[2];
					r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + ((item->pos.x_pos + WALL_L - r->x) >> WALL_SHIFT) * r->x_size].stopper = 1;
					item->item_flags[1] = 1;
				}

				break;
			}
			case -0x8000:
			{
				auto floor = GetFloor(item->pos.x_pos + WALL_L, item->pos.y_pos, item->pos.z_pos, &room_number);
				auto r = &room[room_number];

				int height = GetHeight(floor, item->pos.x_pos + WALL_L, item->pos.y_pos, item->pos.z_pos),
					stop = (r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + ((item->pos.x_pos + WALL_L - r->x) >> WALL_SHIFT) * r->x_size].stopper);
				
				bool left_ok = (height == item->pos.y_pos && stop == 0);

				room_number = item->room_number;
				floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos - WALL_L, &room_number);
				height = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos - WALL_L);
				r = &room[room_number];
				stop = (r->floor[((item->pos.z_pos - WALL_L - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size].stopper);
				
				bool forward_ok = (height == item->pos.y_pos && stop == 0);

				if (!forward_ok && !left_ok && item->item_flags[0] > 0)
				{
					item->pos.y_rot += CLEANER_TURN;
					item->item_flags[0] = CLEANER_TURN;
				}
				else if (!forward_ok && !left_ok && item->item_flags[0] < 0)
				{
					item->pos.y_rot -= CLEANER_TURN;
					item->item_flags[0] = -CLEANER_TURN;
				}
				else if (left_ok && item->item_flags[0] > 0)
				{
					item->pos.y_rot -= CLEANER_TURN;
					item->item_flags[0] = -CLEANER_TURN;
				}
				else
				{
					item->item_flags[0] = CLEANER_TURN;
					item->pos.z_pos -= item->item_flags[2];
					r->floor[((item->pos.z_pos - WALL_L - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size].stopper = 1;
					item->item_flags[1] = 1;
				}

				break;
			}
			case -0x4000:
			{
				auto floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos - WALL_L, &room_number);
				auto r = &room[room_number];
				
				int height = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos - WALL_L),
					stop = (r->floor[((item->pos.z_pos - WALL_L - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size].stopper);
				
				bool left_ok = (height == item->pos.y_pos && stop == 0);

				room_number = item->room_number;
				floor = GetFloor(item->pos.x_pos - WALL_L, item->pos.y_pos, item->pos.z_pos, &room_number);
				height = GetHeight(floor, item->pos.x_pos - WALL_L, item->pos.y_pos, item->pos.z_pos);
				r = &room[room_number];
				stop = (r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - WALL_L - r->x) >> WALL_SHIFT) * r->x_size].stopper);
				
				bool forward_ok = (height == item->pos.y_pos && stop == 0);

				if (!forward_ok && !left_ok && item->item_flags[0] > 0)
				{
					item->pos.y_rot += CLEANER_TURN;
					item->item_flags[0] = CLEANER_TURN;
				}
				else if (!forward_ok && !left_ok && item->item_flags[0] < 0)
				{
					item->pos.y_rot -= CLEANER_TURN;
					item->item_flags[0] = -CLEANER_TURN;
				}
				else if (left_ok && item->item_flags[0] > 0)
				{
					item->pos.y_rot -= CLEANER_TURN;
					item->item_flags[0] = -CLEANER_TURN;
				}
				else
				{
					item->item_flags[0] = CLEANER_TURN;
					item->pos.x_pos -= item->item_flags[2];
					r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - WALL_L - r->x) >> WALL_SHIFT) * r->x_size].stopper = 1;
					item->item_flags[1] = 1;
				}
			}
			}

			TestTriggers(trigger_index, 1);

			if (HeavyTriggered)
			{
				item->item_flags[2] = 0;

				g_audio->play_sound(131, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
			}
		}
		else
		{
			switch (item->pos.y_rot)
			{
			case 0x0:	  item->pos.z_pos += item->item_flags[2]; break;
			case 0x4000:  item->pos.x_pos += item->item_flags[2]; break;
			case -0x8000: item->pos.z_pos -= item->item_flags[2]; break;
			case -0x4000: item->pos.x_pos -= item->item_flags[2]; break;
			}
		}
	}
	else item->pos.y_rot += item->item_flags[0];

	if ((item->touch_bits & CLEANER_TOUCH) && !lara.electric)
	{
		lara.electric = 1;
		lara_item->hit_points = 0;
		item->item_flags[2] = 0;

		g_audio->play_sound(131, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
	}

	AnimateItem(item);

	auto room_number = item->room_number;

	GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	if (room_number != item->room_number)
		ItemNewRoom(item_number, room_number);

	g_audio->play_sound(191, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

	if (((GetRandomControl() & 7) == 0 && elecspark[0] == 0) || elecspark[0])
	{
		if (elecspark[0] == 0)
			elecspark[0] = (GetRandomControl() & 7) + 4;
		else --elecspark[0];

		PHD_VECTOR pos { -160, -8, 16 };

		GetJointAbsPosition(item, &pos, 5);
		TriggerElectricSparksCleaner(&pos, item_number, SPN_CLEANER5);

		pos.x += (GetRandomControl() & 31) - 16;
		pos.y += (GetRandomControl() & 31) - 16;
		pos.z += (GetRandomControl() & 31) - 16;

		int b = (GetRandomControl() & 15) + 16,
			r = b >> 2,
			g = b >> 1;

		TriggerDynamicLight(pos.x, pos.y, pos.z, 10, r, g, b);
	}

	if (((GetRandomControl() & 7) == 0 && elecspark[1] == 0) || elecspark[1])
	{
		if (elecspark[1] == 0)
			elecspark[1] = (GetRandomControl() & 7) + 4;
		else --elecspark[1];

		PHD_VECTOR pos { -160, -8, 16 };

		GetJointAbsPosition(item, &pos, 9);
		TriggerElectricSparksCleaner(&pos, item_number, SPN_CLEANER9);

		pos.x += (GetRandomControl() & 31) - 16;
		pos.y += (GetRandomControl() & 31) - 16;
		pos.z += (GetRandomControl() & 31) - 16;
		
		int b = (GetRandomControl() & 15) + 16,
			r = b >> 2,
			g = b >> 1;

		TriggerDynamicLight(pos.x, pos.y, pos.z, 10, r, g, b);
	}

	if (((GetRandomControl() & 7) == 0 && elecspark[2] == 0) || elecspark[2])
	{
		if (elecspark[2] == 0)
			elecspark[2] = (GetRandomControl() & 7) + 4;
		else --elecspark[2];

		PHD_VECTOR pos { -160, -8, 16 };

		GetJointAbsPosition(item, &pos, 13);
		TriggerElectricSparksCleaner(&pos, item_number, SPN_CLEANER13);

		pos.x += (GetRandomControl() & 31) - 16;
		pos.y += (GetRandomControl() & 31) - 16;
		pos.z += (GetRandomControl() & 31) - 16;
		
		int b = (GetRandomControl() & 15) + 16,
			r = b >> 2,
			g = b >> 1;

		TriggerDynamicLight(pos.x, pos.y, pos.z, 10, r, g, b);
	}
}

void TriggerElectricSparksCleaner(PHD_VECTOR* pos, int16_t item_number, int16_t Node)
{
	int dx = lara_item->pos.x_pos - pos->x,
		dz = lara_item->pos.z_pos - pos->z;

	if (dx < -0x5000 || dx > 0x5000 || dz < -0x5000 || dz > 0x5000)
		return;

	auto sptr = &spark[GetFreeSpark()];

	sptr->On = 1;
	sptr->sB = (GetRandomControl() & 63) + 192;
	sptr->sR = sptr->sB;
	sptr->sG = sptr->sB;
	sptr->dB = (GetRandomControl() & 63) + 192;
	sptr->dR = sptr->sB >> 2;
	sptr->dG = sptr->sB >> 1;
	sptr->ColFadeSpeed = 8;
	sptr->FadeToBlack = 8;
	sptr->sLife = sptr->Life = 20 + (GetRandomControl() & 7);
	sptr->TransType = COLADD;
	sptr->Dynamic = -1;
	sptr->x = (GetRandomControl() & 31) - 16;
	sptr->y = (GetRandomControl() & 31) - 16;
	sptr->z = (GetRandomControl() & 31) - 16;
	sptr->Xvel = ((GetRandomControl() & 255) - 128) << 2;
	sptr->Yvel = (GetRandomControl() & 7) - 4;
	sptr->Zvel = ((GetRandomControl() & 255) - 128) << 2;
	sptr->Friction = 4;
	sptr->Flags = SP_SCALE | SP_ITEM | SP_NODEATTATCH;
	sptr->FxObj = item_number;
	sptr->NodeNumber = Node;
	sptr->Scalar = 1;
	sptr->Width = sptr->sWidth = (GetRandomControl() & 3) + 4;
	sptr->dWidth = sptr->sWidth >> 1;
	sptr->Height = sptr->sHeight = sptr->Width;
	sptr->dHeight = sptr->dWidth;
	sptr->Gravity = 4 + (GetRandomControl() & 3);
	sptr->MaxYvel = 0;
}