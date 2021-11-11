#include "objects.h"
#include "lara.h"
#include "effect2.h"
#include "sphere.h"
#include "control.h"
#include "traps.h"

#include <specific/standard.h>
#include <specific/fn_stubs.h>

#define	DEADLYRANGE_ADD		32

enum strut_anims
{
	FIREHEAD_STILL,
	FIREHEAD_REAR,
	FIREHEAD_BLOW
};

enum
{
	FH_BLOWLOOPS,
	FH_SPEED,
	FH_STOP,
	FH_DEADLYRANGE
};

void TriggerFireHeadFlame(long x, long y, long z, long angle, long speed);

void InitialiseFireHead(int16_t item_number)
{
	auto item = &items[item_number];

	item->anim_number = objects[item->object_number].anim_index + 1;
	item->frame_number = anims[item->anim_number].frame_base;
	item->current_anim_state = FIREHEAD_REAR;
	item->goal_anim_state = FIREHEAD_REAR;
}

void ControlFireHead(int16_t item_number)
{
	auto item = &items[item_number];

	if (!TriggerActive(item))
		return;

	if (item->current_anim_state != FIREHEAD_STILL)
	{
		int angle = (((item->pos.y_rot >> 4) + 1024) & 4095) << 1,
			r = 24 + (GetRandomControl() & 7),
			g = 12 + (GetRandomControl() & 3);

		if (item->current_anim_state == FIREHEAD_REAR)
		{
			PHD_VECTOR pos { 0, 128, 0 };

			GetJointAbsPosition(item, &pos, 7);

			item->item_flags[FH_SPEED] = 0;
			item->item_flags[FH_STOP] = 0;
			item->item_flags[FH_DEADLYRANGE] = 0;
			item->item_flags[FH_BLOWLOOPS] = item->hit_points;

			TriggerDynamicLight(pos.x, pos.y, pos.z, 8, r, g, 0);
		}
		else
		{
			PHD_VECTOR pos {};

			GetJointAbsPosition(item, &pos, 7);

			if (item->item_flags[FH_STOP] == 0)
			{
				g_audio->play_sound(204, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

				if (item->item_flags[FH_SPEED] < 2048)
					item->item_flags[FH_SPEED] += 64;

				if (item->item_flags[FH_DEADLYRANGE] < 2048)
					item->item_flags[FH_DEADLYRANGE] += DEADLYRANGE_ADD;
			}
			else
			{
				if (item->item_flags[FH_SPEED])
					item->item_flags[FH_SPEED] -= 64;

				if (item->item_flags[FH_DEADLYRANGE])
				{
					item->item_flags[FH_DEADLYRANGE] -= DEADLYRANGE_ADD;

					if (item->item_flags[FH_DEADLYRANGE] < 0)
						item->item_flags[FH_DEADLYRANGE] = 0;
				}
			}

			if (item->item_flags[FH_BLOWLOOPS])
				--item->item_flags[FH_BLOWLOOPS];

			if (wibble & 4)
				TriggerFireHeadFlame(pos.x, pos.y, pos.z, angle, item->item_flags[FH_SPEED]);

			if (item->item_flags[FH_BLOWLOOPS] == 0 && item->item_flags[FH_STOP] == 0 && item->frame_number == anims[item->anim_number].frame_base)
			{
				item->item_flags[FH_STOP] = 1;
				item->goal_anim_state = FIREHEAD_REAR;
			}

			TriggerDynamicLight(pos.x, pos.y, pos.z, 8 + (item->item_flags[FH_SPEED] >> 7), r, g, 0);

			if (!lara.burn)
			{
				int ydmin = item->pos.y_pos - 1024,
					ydmax = item->pos.y_pos + 256,
					xdmin,
					zdmin,
					xdmax,
					zdmax;

				switch (item->pos.y_rot)
				{
				case 0:
					xdmin = pos.x;
					xdmax = pos.x + item->item_flags[FH_DEADLYRANGE];
					zdmin = pos.z - 512;
					zdmax = pos.z + 512;
					break;
				case 16384:
					zdmax = pos.z;
					zdmin = pos.z - item->item_flags[FH_DEADLYRANGE];
					xdmin = pos.x - 512;
					xdmax = pos.x + 512;
					break;
				case -32768:
					xdmax = pos.x;
					xdmin = pos.x - item->item_flags[FH_DEADLYRANGE];
					zdmin = pos.z - 512;
					zdmax = pos.z + 512;
					break;
				default:
					zdmin = pos.z;
					zdmax = pos.z + item->item_flags[FH_DEADLYRANGE];
					xdmin = pos.x - 512;
					xdmax = pos.x + 512;
					break;
				}

				int lx = lara_item->pos.x_pos,
					ly = lara_item->pos.y_pos,
					lz = lara_item->pos.z_pos;

				if (lx > xdmin && lx < xdmax && ly > ydmin && ly < ydmax && lz > zdmin && lz < zdmax)
					LaraBurn();
			}
		}
	}
	else item->goal_anim_state = FIREHEAD_REAR;

	AnimateItem(item);
}

void TriggerFireHeadFlame(long x, long y, long z, long angle, long speed)
{
	int dx = lara_item->pos.x_pos - x,
		dz = lara_item->pos.z_pos - z;

	if (g_effects_draw_distance != -1 && (dx < -g_effects_draw_distance || dx > g_effects_draw_distance || dz < -g_effects_draw_distance || dz > g_effects_draw_distance))
		return;

	speed -= (GetRandomControl() % ((speed >> 3) + 1));

	int size = (GetRandomControl() & 15) + (speed >> 4);

	auto sptr = &spark[GetFreeSpark()];

	sptr->On = 1;
	sptr->sR = 48 + (GetRandomControl() & 31);
	sptr->sG = sptr->sR;
	sptr->sB = 192 + (GetRandomControl() & 63);

	sptr->dR = 192 + (GetRandomControl() & 63);
	sptr->dG = 128 + (GetRandomControl() & 63);
	sptr->dB = 32;

	sptr->ColFadeSpeed = 12 + (GetRandomControl() & 3);
	sptr->FadeToBlack = 8;
	sptr->sLife = sptr->Life = (GetRandomControl() & 7) + 28;
	sptr->TransType = COLADD;
	sptr->extras = 0;
	sptr->Dynamic = -1;
	sptr->x = x + ((GetRandomControl() & 31) - 16);
	sptr->y = y + ((GetRandomControl() & 31) - 16);
	sptr->z = z + ((GetRandomControl() & 31) - 16);

	int zv = (m_cos(angle) * speed) >> 11,
		xv = (m_sin(angle) * speed) >> 11;

	sptr->Xvel = xv + ((GetRandomControl() & 127) - 64);
	sptr->Yvel = (GetRandomControl() & 7) + 6;
	sptr->Zvel = zv + ((GetRandomControl() & 127) - 64);
	sptr->Friction = 4;
	sptr->Flags = SP_SCALE | SP_DEF | SP_EXPDEF;

	if (GetRandomControl() & 1)
	{
		sptr->Flags |= SP_ROTATE;
		sptr->RotAng = GetRandomControl() & 4095;
		sptr->RotAdd = (GetRandomControl() & 63) - 32;
	}

	sptr->Gravity = -(GetRandomControl() & 15) - 8;
	sptr->MaxYvel = -(GetRandomControl() & 7) - 8;
	sptr->Def = (PHDSPRITESTRUCT*)objects[EXPLOSION1].mesh_ptr;
	sptr->Scalar = 3;
	sptr->Width = sptr->sWidth = size >> 2;
	sptr->Height = sptr->sHeight = size >> 2;
	sptr->dWidth = size;
	sptr->dHeight = size;
}

void ControlRotateyThing(int16_t item_number)
{
	if (auto item = &items[item_number]; TriggerActive(item))
		AnimateItem(item);
	else KillItem(item_number);
}