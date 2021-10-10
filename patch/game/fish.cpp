#include "objects.h"
#include "lara.h"
#include "fish.h"
#include "control.h"
#include "gameflow.h"
#include "game.h"

#include <specific/fn_stubs.h>

#define PIRAHNA_DAMAGE	4

int FishNearLara(PHD_3DPOS* pos, int32_t distance, ITEM_INFO* item);

uint8_t jungle_fish_ranges[1][3] =
{
	{ 2 << 2,5 << 2,3 }
};

uint8_t temple_fish_ranges[3][3] =
{
	{ 1 << 2, 1 << 2, 2 },
	{ 1 << 2, 4 << 2, 2 },
	{ 1 << 2, 7 << 2, 4 },
};

uint8_t quadchase_fish_ranges[8][3] =
{
	{ 1 << 2, 3 << 2, 1 },
	{ 0 << 2, 3 << 2, 2 },
	{ 2 << 2, 1 << 2, 2 },
	{ 1 << 2, 2 << 2, 1 },
	{ 1 << 2, 4 << 2, 2 },
	{ 1 << 2, 6 << 2, 1 },
	{ 3 << 2, 1 << 2, 1 },
	{ 4 << 2, 1 << 2, 1 },
};

uint8_t house_fish_ranges[7][3] =
{
	{ 1 << 2, 1 << 2, 1 },
	{ 4 << 2, 2 << 2, 2 },
	{ 6 << 2, 2 << 2, 2 },
	{ 2 << 2, 4 << 2, 2 },
	{ 2 << 2, 3 << 2, 1 },
	{ 5 << 2, 2 << 2, 2 },
	{ 4 << 2, 2 << 2, 1 },
};

uint8_t shore_fish_ranges[3][3] =
{
	{ 3 << 2, 3 << 2, 6 },
	{ 3 << 2, 5 << 2, 6 },
	{ 5 << 2, 1 << 2, 8 },
};

uint8_t crash_fish_ranges[1][3] =
{
	{ 5 << 2, 1 << 2, 6 },
};

uint8_t rapids_fish_ranges[2][3] =
{
	{ 4 << 2, 4 << 2, 8 },
	{ 1 << 2, 2 << 2, 5 },
};

int PirahnaHitWait = 0;

void SetupShoal(long shoal_number)
{
	switch (fish_shoal_type)
	{
	case 0:
		lead_info[shoal_number].Xrange = (jungle_fish_ranges[shoal_number][0] + 2) << 8;
		lead_info[shoal_number].Zrange = (jungle_fish_ranges[shoal_number][1] + 2) << 8;
		lead_info[shoal_number].Yrange = (jungle_fish_ranges[shoal_number][2]) << 8;
	case 1:
		lead_info[shoal_number].Xrange = (temple_fish_ranges[shoal_number][0] + 2) << 8;
		lead_info[shoal_number].Zrange = (temple_fish_ranges[shoal_number][1] + 2) << 8;
		lead_info[shoal_number].Yrange = (temple_fish_ranges[shoal_number][2]) << 8;
	case 2:
		lead_info[shoal_number].Xrange = (quadchase_fish_ranges[shoal_number][0] + 2) << 8;
		lead_info[shoal_number].Zrange = (quadchase_fish_ranges[shoal_number][1] + 2) << 8;
		lead_info[shoal_number].Yrange = (quadchase_fish_ranges[shoal_number][2]) << 8;
	case 3:
		lead_info[shoal_number].Xrange = (house_fish_ranges[shoal_number][0] + 2) << 8;
		lead_info[shoal_number].Zrange = (house_fish_ranges[shoal_number][1] + 2) << 8;
		lead_info[shoal_number].Yrange = (house_fish_ranges[shoal_number][2]) << 8;
	case 4:
		lead_info[shoal_number].Xrange = (shore_fish_ranges[shoal_number][0] + 2) << 8;
		lead_info[shoal_number].Zrange = (shore_fish_ranges[shoal_number][1] + 2) << 8;
		lead_info[shoal_number].Yrange = (shore_fish_ranges[shoal_number][2]) << 8;
	case 5:
		lead_info[shoal_number].Xrange = (crash_fish_ranges[shoal_number][0] + 2) << 8;
		lead_info[shoal_number].Zrange = (crash_fish_ranges[shoal_number][1] + 2) << 8;
		lead_info[shoal_number].Yrange = (crash_fish_ranges[shoal_number][2]) << 8;
	case 6:
		lead_info[shoal_number].Xrange = (rapids_fish_ranges[shoal_number][0] + 2) << 8;
		lead_info[shoal_number].Zrange = (rapids_fish_ranges[shoal_number][1] + 2) << 8;
		lead_info[shoal_number].Yrange = (rapids_fish_ranges[shoal_number][2]) << 8;
	default:
		lead_info[shoal_number].Xrange = 1 << 8;
		lead_info[shoal_number].Zrange = 1 << 8;
		lead_info[shoal_number].Yrange = 1 << 8;
	}
}

void SetupFish(long leader, ITEM_INFO* item)
{
	if (leader >= 7)
		return;

	int fish_Xrange = lead_info[leader].Xrange,
		fish_Yrange = lead_info[leader].Yrange,
		fish_Zrange = lead_info[leader].Zrange;

	fish[leader].x = 0;
	fish[leader].y = 0;
	fish[leader].z = 0;
	fish[leader].angle = 0;
	fish[leader].speed = (GetRandomControl() & 63) + 8;
	fish[leader].swim = GetRandomControl() & 63;

	for (int i = 0; i < 24; ++i)
	{
		fish[MAX_FISH + (leader * 24) + i].x = (GetRandomControl() % (fish_Xrange << 1)) - fish_Xrange;
		fish[MAX_FISH + (leader * 24) + i].y = (GetRandomControl() % fish_Yrange);
		fish[MAX_FISH + (leader * 24) + i].z = (GetRandomControl() % (fish_Zrange << 1)) - fish_Zrange;
		fish[MAX_FISH + (leader * 24) + i].desty = (GetRandomControl() % fish_Yrange);
		fish[MAX_FISH + (leader * 24) + i].angle = GetRandomControl() & 4095;
		fish[MAX_FISH + (leader * 24) + i].speed = (GetRandomControl() & 31) + 32;
		fish[MAX_FISH + (leader * 24) + i].swim = GetRandomControl() & 63;
	}

	lead_info[leader].on = 1;
	lead_info[leader].angle = 0;
	lead_info[leader].speed = (GetRandomControl() & 127) + 32;
	lead_info[leader].angle_time = 0;
	lead_info[leader].speed_time = 0;
}

void ControlFish(short item_number)
{
	auto enemy = &items[item_number];
	auto item = enemy;

	if (!TriggerActive(item))
		return;

	int leader = item->hit_points;

	if (!lead_info[leader].on)
		SetupFish(leader, item);

	int pirahna_attack = 0;

	if (item->object_number == PIRAHNAS)
		pirahna_attack = (lara_item->room_number == item->room_number);
	else pirahna_attack = 0;

	if (PirahnaHitWait)
		--PirahnaHitWait;

	auto fptr = (FISH_INFO*)&fish[leader];

	if (pirahna_attack)
	{
		enemy = (pirahna_attack == 1 ? lara_item : &items[carcass_item]);

		lead_info[leader].angle = fptr->angle = (-((long)m_atan2(fptr->x + item->pos.x_pos, fptr->z + item->pos.z_pos, enemy->pos.x_pos, enemy->pos.z_pos) + 0x4000) >> 4) & 4095;
		lead_info[leader].speed = (GetRandomControl() & 63) + 192;
	}

	int diff = fptr->angle - lead_info[leader].angle,
		angadd;

	if (diff > 2048)		diff -= 4096;
	else if (diff < -2048)  diff += 4096;

	if (diff > 128)
	{
		fptr->angadd -= 4;

		if (fptr->angadd < -120)
			fptr->angadd = -120;
	}
	else if (diff < -128)
	{
		fptr->angadd += 4;

		if (fptr->angadd > 120)
			fptr->angadd = 120;
	}
	else
	{
		fptr->angadd -= fptr->angadd >> 2;

		if (abs(fptr->angadd) < 4)
			fptr->angadd = 0;
	}

	fptr->angle += fptr->angadd;

	if (diff > 1024)
		fptr->angle += fptr->angadd >> 2;

	fptr->angle &= 4095;

	diff = fptr->speed - lead_info[leader].speed;

	if (diff < -4)
	{
		if ((diff = fptr->speed + (GetRandomControl() & 3) + 1) < 0)
			diff = 0;
		fptr->speed = diff;

	}
	else if (diff > 4)
	{
		if ((diff = fptr->speed - (GetRandomControl() & 3) - 1) > 255)
			diff = 255;

		fptr->speed = diff;
	}

	fptr->swim += fptr->speed >> 4;
	fptr->swim &= 63;

	int z = fptr->z,
		x = fptr->x;

	z += (m_cos(fptr->angle << 1) * fptr->speed) >> 13;
	x += -((m_sin(fptr->angle << 1) * fptr->speed) >> 13);

	if (pirahna_attack == 0)
	{
		int fish_Xrange = lead_info[leader].Xrange,
			fish_Zrange = lead_info[leader].Zrange;

		if (z < -fish_Zrange)
		{
			z = -fish_Zrange;

			lead_info[leader].angle = (fptr->angle < 2048 ? fptr->angle - ((GetRandomControl() & 127) + 128) : fptr->angle + ((GetRandomControl() & 127) + 128));
			lead_info[leader].angle_time = (GetRandomControl() & 15) + 8;
			lead_info[leader].speed_time = 0;
		}
		else if (z > fish_Zrange)
		{
			z = fish_Zrange;

			lead_info[leader].angle = (fptr->angle > 3072 ? fptr->angle - ((GetRandomControl() & 127) + 128) : fptr->angle + ((GetRandomControl() & 127) + 128));
			lead_info[leader].angle_time = (GetRandomControl() & 15) + 8;
			lead_info[leader].speed_time = 0;
		}

		if (x < -fish_Xrange)
		{
			x = -fish_Xrange;

			lead_info[leader].angle = (fptr->angle < 1024 ? fptr->angle - ((GetRandomControl() & 127) + 128) : fptr->angle + ((GetRandomControl() & 127) + 128));
			lead_info[leader].angle_time = (GetRandomControl() & 15) + 8;
			lead_info[leader].speed_time = 0;
		}
		else if (x > fish_Xrange)
		{
			x = fish_Xrange;

			lead_info[leader].angle = (fptr->angle < 3072 ? fptr->angle - ((GetRandomControl() & 127) + 128) : fptr->angle + ((GetRandomControl() & 127) + 128));
			lead_info[leader].angle_time = (GetRandomControl() & 15) + 8;
			lead_info[leader].speed_time = 0;
		}

		if ((GetRandomControl() & 15) == 0)
			lead_info[leader].angle_time = 0;

		if (lead_info[leader].angle_time)
			--lead_info[leader].angle_time;
		else
		{
			angadd = ((GetRandomControl() & 63) + 16) - 8 - 32;

			lead_info[leader].angle_time = (GetRandomControl() & 15) + 8;
			lead_info[leader].angle += ((GetRandomControl() & 3) == 0 ? angadd << 5 : angadd);
			lead_info[leader].angle &= 4095;
		}

		if (lead_info[leader].speed_time)
			--lead_info[leader].speed_time;
		else
		{
			lead_info[leader].speed_time = (GetRandomControl() & 31) + 32;

			if ((GetRandomControl() & 7) == 0)
				lead_info[leader].speed = (GetRandomControl() & 127) + 128;
			else if ((GetRandomControl() & 3) == 0)
				lead_info[leader].speed += (GetRandomControl() & 127) + 32;
			else if (lead_info[leader].speed > 140)
				lead_info[leader].speed -= (GetRandomControl() & 31) + 48;
			else
			{
				lead_info[leader].speed_time = (GetRandomControl() & 3) + 4;
				lead_info[leader].speed += (GetRandomControl() & 31) - 15;
			}
		}
	}

	int ftx = fptr->x = x,
		fty = fptr->y,
		ftz = fptr->z = z;

	fptr = (FISH_INFO*)&fish[MAX_FISH + (leader * 24)];

	for (int i = 0; i < 24; ++i)
	{
		if (item->object_number == PIRAHNAS)
		{
			PHD_3DPOS pos { item->pos.x_pos + fptr->x, item->pos.y_pos + fptr->y, item->pos.z_pos + fptr->z };
			
			if (FishNearLara(&pos, 256, (pirahna_attack < 2) ? lara_item : enemy))
			{
				if (PirahnaHitWait == 0)
				{
					DoBloodSplat(item->pos.x_pos + fptr->x, item->pos.y_pos + fptr->y, item->pos.z_pos + fptr->z, 0, 0, (pirahna_attack < 2) ? lara_item->room_number : enemy->room_number);
					PirahnaHitWait = 8;
				}

				if (pirahna_attack != 2)
					lara_item->hit_points -= PIRAHNA_DAMAGE;
			}
		}

		int angle = (-((long)m_atan2(fptr->x, fptr->z, ftx, ftz) + 0x4000) >> 4) & 4095,
			dx = fptr->x - ftx + ((24 - i) << 7),
			dz = fptr->z - ftz - ((24 - i) << 7);

		dx *= dx;
		dz *= dz;

		diff = fptr->angle - angle;

		if (diff > 2048)		diff -= 4096;
		else if (diff < -2048)  diff += 4096;

		if (diff > 128)
		{
			fptr->angadd -= 4;

			if (fptr->angadd < -92 - (i >> 1))
				fptr->angadd = -92 - (i >> 1);
		}
		else	if (diff < -128)
		{
			fptr->angadd += 4;

			if (fptr->angadd > 92 + (i >> 1))
				fptr->angadd = 92 + (i >> 1);
		}
		else
		{
			fptr->angadd -= fptr->angadd >> 2;

			if (abs(fptr->angadd) < 4)
				fptr->angadd = 0;
		}

		fptr->angle += fptr->angadd;

		if (diff > 1024)
			fptr->angle += fptr->angadd >> 2;

		fptr->angle &= 4095;

		if ((dx + dz) < (0x100000 + ((i << 7) * (i << 7))))
		{
			if (fptr->speed > 32 + (i << 1))
				fptr->speed -= fptr->speed >> 5;
		}
		else
		{
			if (fptr->speed < 160 + (i >> 1))
				fptr->speed += (GetRandomControl() & 3) + 1 + (i >> 1);

			if (fptr->speed > 160 + (i >> 1) - (i << 2))
				fptr->speed = 160 + (i >> 1) - (i << 2);
		}

		fptr->speed += ((GetRandomControl() & 1) ? -(GetRandomControl() & 1) : (GetRandomControl() & 1));

		if (fptr->speed < 32)		fptr->speed = 32;
		else if (fptr->speed > 200) fptr->speed = 200;

		fptr->swim += (fptr->speed >> 4) + (fptr->speed >> 5);
		fptr->swim &= 63;

		z = fptr->z + ((m_cos(fptr->angle << 1) * fptr->speed) >> 13);
		x = fptr->x - ((m_sin(fptr->angle << 1) * fptr->speed) >> 13);

		if (z < -32000)		z = -32000;
		else if (z > 32000) z = 32000;

		if (x < -32000)		x = -32000;
		else if (x > 32000) x = 32000;

		fptr->x = x;
		fptr->z = z;

		if (pirahna_attack == 0)
		{
			if (abs(fptr->y - fptr->desty) < 16 && lead_info[leader].Yrange > 0)
				fptr->desty = GetRandomControl() % lead_info[leader].Yrange;
		}
		else
		{
			int y = enemy->pos.y_pos - item->pos.y_pos;

			if (abs(fptr->y - fptr->desty) < 16)
				fptr->desty = y + (GetRandomControl() & 255);
		}

		fptr->y += (fptr->desty - fptr->y) >> 4;

		++fptr;
	}
}

int FishNearLara(PHD_3DPOS* pos, int32_t distance, ITEM_INFO* item)
{
	int x = pos->x_pos - item->pos.x_pos,
		y = abs(pos->y_pos - item->pos.y_pos),
		z = pos->z_pos - item->pos.z_pos;

	if (x < -distance || x > distance || z < -distance || z > distance || y < -WALL_L * 3 || y > WALL_L * 3)
		return (0);

	if (x * x + z * z > SQUARE(distance))
		return 0;

	if (y > distance)
		return 0;

	return 1;
}