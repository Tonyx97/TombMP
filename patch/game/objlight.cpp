#include <specific/standard.h>
#include <specific/stypes.h>
#include <specific/fn_stubs.h>

#include "objlight.h"
#include "effect2.h"
#include "items.h"
#include "control.h"
#include "gameflow.h"
#include "game.h"

void ControlStrobeLight(int16_t item_number)
{
	auto item = &items[item_number];

	if (TriggerActive(item))
	{
		if (!item->really_active)
			return;

		item->pos.y_rot += ONE_DEGREE << 4;

		int angle = ((item->pos.y_rot + 0x5800) >> 4) & 4095,
			sin = m_sin(angle << 1, 4),
			cos = m_cos(angle << 1, 4);

		TriggerAlertLight(item->pos.x_pos, item->pos.y_pos - 512, item->pos.z_pos, 31, 8, 0, angle, item->room_number);
		TriggerDynamicLight(item->pos.x_pos + sin, item->pos.y_pos - 768, item->pos.z_pos + cos, 6, 31, 12, 0);

		if ((wibble & 127) == 0)
			g_audio->play_sound(208, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

		++item->item_flags[0];

		if (item->item_flags[0] > 1800)
		{
			item->really_active = 0;
			item->item_flags[0] = 0;
		}
	}
}

void ControlPulseLight(int16_t item_number)
{
	auto item = &items[item_number];

	if (TriggerActive(item))
	{
		item->item_flags[0] += ONE_DEGREE << 2;

		int angle = (item->item_flags[0] >> 4) & 4095,
			sin = abs(m_sin(angle << 1, 7));

		if (sin > 31)
			sin = 31;
		else if (sin < 8)
		{
			sin = 8;
			item->item_flags[0] += 2048;
		}

		TriggerDynamicLight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, sin, 31, 12, 0);
	}
}

void ControlOnOffLight(int16_t item_number)
{
	if (auto item = &items[item_number]; TriggerActive(item))
		TriggerDynamicLight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 16, 31, 31, 31);
}

void ControlElectricalLight(int16_t item_number)
{
	if (auto item = &items[item_number]; TriggerActive(item))
	{
		int r, g, b;

		if (item->item_flags[0] < 16)
		{
			r = g = (GetRandomControl() & 7) << 2;
			b = r + (GetRandomControl() & 3);
			++item->item_flags[0];
		}
		else if (item->item_flags[0] < 96)
		{
			if ((wibble & 63) == 0 || (GetRandomControl() & 7) == 0)
			{
				r = g = 24 - (GetRandomControl() & 7);
				b = r + (GetRandomControl() & 3);
			}
			else
			{
				r = g = GetRandomControl() & 7;
				b = r + (GetRandomControl() & 3);
			}
			++item->item_flags[0];
		}
		else if (item->item_flags[0] < 160)
		{
			r = g = 12 - (GetRandomControl() & 3);
			b = r + (GetRandomControl() & 3);

			item->item_flags[0] = ((GetRandomControl() & 31) == 0 && item->item_flags[0] > 128 ? 160 : item->item_flags[0] + 1);
		}
		else
		{
			r = g = 31 - (GetRandomControl() & 3);
			b = 31 - (GetRandomControl() & 1);
		}

		TriggerDynamicLight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 16, r, g, b);
	}
	else item->item_flags[0] = 0;
}

void ControlBeaconLight(int16_t item_number)
{
	if (auto item = &items[item_number]; TriggerActive(item))
	{
		++item->item_flags[0];
		item->item_flags[0] &= 63;

		if (item->item_flags[0] < 3)
		{
			int r = 31 - (GetRandomControl() & 1),
				g = r,
				b = 31 - (GetRandomControl() & 3);

			TriggerDynamicLight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 16, r, g, b);
		}
	}
}