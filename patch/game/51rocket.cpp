#include <specific/standard.h>

#include "objects.h"
#include "lara.h"
#include "laraanim.h"
#include "effect2.h"
#include "sphere.h"
#include "control.h"
#include "camera.h"

#include <specific/fn_stubs.h>

#define FIRE_ROOM		52
#define SMOKE_START		0
#define SMOKE_END		512
#define	MAX_ELEC_POS	256
#define	MAX_ELEC_POS2	MAX_ELEC_POS + (MAX_ELEC_POS >> 1)

enum strut_anims
{
	STRUT_EMPTY,
	STRUT_WAIT,
	STRUT_FALL
};

void Trigger51RocketSmoke(long x, long y, long z, long yv, long fire);
void Trigger51BlastFire(long x, long y, long z, long smoke, long end);

uint8_t strut_fall;

void ControlArea51Rocket(int16_t item_number)
{
	auto item = &items[item_number];

	if (!TriggerActive(item))
		return;

	if (item->object_number == SPECIAL_FX1)
	{
		if (item->required_anim_state < SMOKE_END)
		{
			item->required_anim_state += 8;

			if (item->required_anim_state >= SMOKE_END)
			{
				item->required_anim_state += 2048;
				item->goal_anim_state = 64;

				g_audio->play_sound(106);
			}
			else
			{
				item->goal_anim_state = item->current_anim_state = 0;

				g_audio->play_sound(12, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
			}
		}
		else if (item->current_anim_state)
		{
			g_audio->play_sound(257);

			item->required_anim_state += 32;

			if (item->required_anim_state > 16000)
			{
				g_audio->stop_sound(257);
				KillItem(item_number);
				return;
			}

			int r = 28 + (GetRandomControl() & 3),
				g = 12 + (GetRandomControl() & 7),
				b = GetRandomControl() & 3;

			r = (r * (16384 - item->required_anim_state)) >> 12;
			g = (g * (16384 - item->required_anim_state)) >> 12;
			b = (b * (16384 - item->required_anim_state)) >> 12;

			int y = -(GetRandomControl() & 511) + room[FIRE_ROOM].minfloor - 256;

			TriggerDynamicLight(item->pos.x_pos - 512 - 8192 + 1024, y, item->pos.z_pos - 1024, 24, r, g, b);

			camera.bounce = -((16384 - item->required_anim_state) >> 6);

			return;
		}
		else
		{
			if (!lara.burn)
			{
				int rad = (item->goal_anim_state < 8192) ? item->goal_anim_state : 8192,
					x = item->pos.x_pos - 512 - rad - 1024;

				if (lara_item->pos.x_pos > x)
				{
					lara_item->hit_points = 0;
					LaraBurn();
				}

				item->goal_anim_state += 80;
			}

			item->required_anim_state += 32;
		}

		if (item->required_anim_state < 4096 + 512)
		{
			if (item->goal_anim_state > 768)
				g_audio->play_sound(257);

			if (item->goal_anim_state > 1024)
				item->goal_anim_state = 1024;
		}
		else
		{
			g_audio->play_sound(257, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

			if (item->required_anim_state > 12288)
				item->required_anim_state = 12288;

			if (item->goal_anim_state > 20480 + 2048)
			{
				for (int i = 0; i < 64; ++i)
				{
					int dist = (GetRandomControl() & 4095),
						x = item->pos.x_pos - 512 - 5120 - dist,
						z = item->pos.z_pos - 1024 + (GetRandomControl() & 2047) - 1024,
						y = -(GetRandomControl() & 2047) + room[FIRE_ROOM].minfloor;

					Trigger51BlastFire(x, y, z, 0, i);
				}

				for (int i = 64; i < MAX_SPARKS >> 1; ++i)
				{
					int dist = (GetRandomControl() & 4095),
						x = item->pos.x_pos - 512 - 5120 - dist,
						z = item->pos.z_pos - 1024 + (GetRandomControl() & 2047) - 1024,
						y = -(GetRandomControl() & 2047) + room[FIRE_ROOM].minfloor;

					Trigger51BlastFire(x, y, z, 1, i);
				}

				item->current_anim_state = 1;

				camera.bounce = -((16384 - item->required_anim_state) >> 6);

				return;
			}

			--item->fallspeed;

			if (item->fallspeed < -1024)
				item->fallspeed = -1024;

			if (item->fallspeed < -72)
				strut_fall = 1;

			item->pos.y_pos += item->fallspeed >> 2;

			auto room_number = item->room_number;
			auto floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

			if (room_number != item->room_number)
				ItemNewRoom(item_number, room_number);
		}

		if (item->required_anim_state < 8192 && item->required_anim_state > 64)
			camera.bounce = -(item->required_anim_state >> 6);
		else if (item->required_anim_state >= 8192)
			camera.bounce = -((16384 - item->required_anim_state) >> 6);
		else camera.bounce = -1;
	}

	if ((wibble & 12) == 0)
	{
		int yvel = (item->required_anim_state < (SMOKE_END >> 2) ? GetRandomControl() & 31
																 : (item->required_anim_state - (SMOKE_END >> 2) + (GetRandomControl() & 31)) << 2);

		if (yvel > (192 << 5))
			yvel = 192 << 5;

		int f = (item->required_anim_state < SMOKE_END) ? 0 : 1;

		Trigger51RocketSmoke(item->pos.x_pos - 512 - 256 - 128, item->pos.y_pos - 64, item->pos.z_pos - 512, yvel, f);
		Trigger51RocketSmoke(item->pos.x_pos - 512 + 256 + 128, item->pos.y_pos - 64, item->pos.z_pos - 512, yvel, f);
		Trigger51RocketSmoke(item->pos.x_pos - 512, item->pos.y_pos - 64, item->pos.z_pos - 512 - 256 - 128, yvel, f);
		Trigger51RocketSmoke(item->pos.x_pos - 512, item->pos.y_pos - 64, item->pos.z_pos - 512 + 256 + 128, yvel, f);
	}

	if (item->goal_anim_state)
	{
		TriggerDynamicLight(item->pos.x_pos - 512, item->pos.y_pos, item->pos.z_pos - 512, 31, 28 + (GetRandomControl() & 3), 12 + (GetRandomControl() & 7), GetRandomControl() & 3);

		int rad = (item->goal_anim_state < 8192) ? item->goal_anim_state : 8192,
			dist = GetRandomControl() & 2047,
			x = item->pos.x_pos - 512 - rad - dist + 1024,
			z = item->pos.z_pos - 1024 + (GetRandomControl() & 2047) - 1024;

		int yand = (item->goal_anim_state < 1024 ? 255 : 2047),
			y = -(GetRandomControl() & yand) + room[FIRE_ROOM].minfloor;

		if (wibble & 4)
			Trigger51BlastFire(x, y, z, 0, -1);

		TriggerDynamicLight(item->pos.x_pos - 512 - rad + 1024, y, item->pos.z_pos - 1024, 24, 28 + (GetRandomControl() & 3), 12 + (GetRandomControl() & 7), GetRandomControl() & 3);

		if (wibble & 4)
		{
			dist = GetRandomControl() & 1023;
			x = item->pos.x_pos - 512 - rad - dist;
			z = item->pos.z_pos - 1024 + (GetRandomControl() & 2047) - 1024;

			yand = (item->goal_anim_state < 1024 ? 255 : 2047);
			y = -(GetRandomControl() & yand) + room[FIRE_ROOM].minfloor;

			Trigger51BlastFire(x, y, z, 1, -1);
		}
	}
}

void Trigger51RocketSmoke(long x, long y, long z, long yv, long fire)
{
	int size = std::clamp((int)((GetRandomControl() & 63) + 64 + (yv >> 5)), 0, 255);

	auto sptr = &spark[GetFreeSpark()];

	sptr->On = 1;

	if (fire)
	{
		sptr->sR = 48 + (GetRandomControl() & 31);
		sptr->sG = sptr->sR;
		sptr->sB = 192 + (GetRandomControl() & 63);

		sptr->dR = 192 + (GetRandomControl() & 63);
		sptr->dG = 128 + (GetRandomControl() & 63);
		sptr->dB = 32;
	}
	else
	{
		sptr->sR = 0;
		sptr->sG = 0;
		sptr->sB = 0;
		sptr->dR = 32 + (yv >> 5);
		sptr->dG = 32 + (yv >> 5);
		sptr->dB = 32 + (yv >> 5);
	}

	sptr->ColFadeSpeed = 16 - (fire ? (yv >> 9) : 0);
	sptr->FadeToBlack = fire ? 0 : 16;
	sptr->sLife = sptr->Life = (GetRandomControl() & 3) + 60 - (fire ? (yv >> 8) : 0);
	sptr->TransType = COLADD;
	sptr->extras = 0;
	sptr->Dynamic = -1;
	sptr->x = x + ((GetRandomControl() & 15) - 8);
	sptr->y = y;
	sptr->z = z + ((GetRandomControl() & 15) - 8);
	sptr->Xvel = ((GetRandomControl() & 255) - 128);
	sptr->Yvel = yv + (GetRandomControl() & 15);
	sptr->Zvel = ((GetRandomControl() & 255) - 128);
	sptr->Friction = 4;

	if (GetRandomControl() & 1)
	{
		sptr->Flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
		sptr->RotAng = GetRandomControl() & 4095;
		sptr->RotAdd = ((GetRandomControl() & 1) ? -(GetRandomControl() & 15) - 16 : (GetRandomControl() & 15) + 16);
	}
	else sptr->Flags = SP_SCALE | SP_DEF | SP_EXPDEF;

	sptr->Def = objects[EXPLOSION1].mesh_index;
	sptr->Scalar = 3;
	sptr->Gravity = sptr->MaxYvel = 0;
	sptr->Width = sptr->sWidth = size >> 2;
	sptr->dWidth = size;
	sptr->Height = sptr->sHeight = size >> 2;
	sptr->dHeight = size;
}

void Trigger51BlastFire(long x, long y, long z, long smoke, long end)
{
	auto sptr = (end >= 0 ? &spark[end] : &spark[GetFreeSpark()]);
	
	sptr->On = 1;

	if (smoke)
	{
		sptr->sR = 0;
		sptr->sG = 0;
		sptr->sB = 0;
		sptr->dR = 64;
		sptr->dG = 64;
		sptr->dB = 64;
	}
	else
	{
		sptr->sR = 128 + (GetRandomControl() & 31);
		sptr->sG = 64 + (GetRandomControl() & 31);
		sptr->sB = 32;
		sptr->dR = 224 + (GetRandomControl() & 31);
		sptr->dG = 160 + (GetRandomControl() & 31);
		sptr->dB = 32;
	}

	sptr->ColFadeSpeed = 16;

	if (end)
	{
		sptr->FadeToBlack = 32 + (GetRandomControl() & 31);
		sptr->sLife = sptr->Life = 72 + (end >> 1);
	}
	else
	{
		sptr->FadeToBlack = 8 + (smoke ? 24 : 0);
		sptr->sLife = sptr->Life = (GetRandomControl() & 7) + 32 + (smoke ? 32 : 0);
	}

	int size = (GetRandomControl() & 63) + 64;

	sptr->TransType = COLADD;
	sptr->extras = 0;
	sptr->Dynamic = -1;
	sptr->x = x + ((GetRandomControl() & 15) - 8);
	sptr->y = y;
	sptr->z = z + ((GetRandomControl() & 15) - 8);
	sptr->Xvel = ((GetRandomControl() & 255) - 128);
	sptr->Yvel = -(GetRandomControl() & 7);
	sptr->Zvel = ((GetRandomControl() & 255) - 128);
	sptr->Friction = 4;
	sptr->Flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
	sptr->RotAng = GetRandomControl() & 4095;
	sptr->RotAdd = ((GetRandomControl() & 1) ? -(GetRandomControl() & 15) - 16 : (GetRandomControl() & 15) + 16);
	sptr->Def = objects[EXPLOSION1].mesh_index;
	sptr->Scalar = 4;
	sptr->Gravity = sptr->MaxYvel = 0;

	if (end)
		sptr->Width = sptr->sWidth = sptr->dWidth = sptr->Height = sptr->sHeight = sptr->dHeight = size;
	else
	{
		sptr->Width = sptr->sWidth = size >> 1;
		sptr->dWidth = size;
		sptr->Height = sptr->sHeight = size >> 1;
		sptr->dHeight = size;
	}
}

void InitialiseArea51Struts(int16_t item_number)
{
	strut_fall = 0;
}

void ControlArea51Struts(int16_t item_number)
{
	auto item = &items[item_number];

	if (!strut_fall)
		item->current_anim_state = STRUT_WAIT;
	else if (item->goal_anim_state != STRUT_FALL && item->current_anim_state != STRUT_FALL)
		item->goal_anim_state = STRUT_FALL;

	AnimateItem(item);
}