#include <shared/defs.h>

#include <sol/sol.hpp>

#include <specific/standard.h>
#include <specific/output.h>
#include <specific/global.h>

#include <game/effect2.h>

#include <window/window.h>

#include <mp/chat.h>
#include <ui/ui.h>

#include "cf_defs.h"

void cf_dx::register_functions(sol::state* vm)
{
	/*class PlayerT {
	public:
		int get_hp() const {
			return hp;
		}

		void set_hp(int value) {
			hp = value;
		}

		int get_max_hp() const {
			return hp;
		}

		void set_max_hp(int value) {
			maxhp = value;
		}

	private:
		int hp = 50;
		int maxhp = 50;
	};

	vm->new_usertype<PlayerT>("Player",
		"hp", sol::property(&PlayerT::get_hp, &PlayerT::set_hp),
		"maxHp", sol::property(&PlayerT::get_max_hp, &PlayerT::set_max_hp)
		);*/

	vm->set_function("dxWorldToScreen", [&](int x, int y, int z) -> std::tuple<int, int, bool>
	{
		int ox = 0, oy = 0;

		if (!world_to_screen(x, y, z, ox, oy))
			return { 0, 0, false };

		return { ox, oy, true};
	});

	vm->set_function("dxDrawText", [&](const char* txt, float x, float y, float size, float r, float g, float b, float a, bool center)
	{
		g_ui->add_text(txt, x, y, size, { r, g, b, a }, center);
	});

	vm->set_function("dxDrawLine", [&](float x, float y, float x1, float y1, float r, float g, float b, float a, float thickness)
	{
		g_ui->draw_line(x, y, x1, y1, thickness, { r, g, b, a });
	});

	vm->set_function("dxDrawCircle", [&](float x, float y, float radius, float r, float g, float b, float a, float thickness)
	{
		g_ui->draw_circle(x, y, radius, thickness, { r, g, b, a });
	});

	vm->set_function("dxDrawFilledCircle", [&](float x, float y, float radius, float r, float g, float b, float a)
	{
		g_ui->draw_filled_circle(x, y, radius, { r, g, b, a });
	});

	vm->set_function("dxDrawRect", [&](float x, float y, float w, float h, float t, float r, float g, float b, float a)
	{
		g_ui->draw_rect(x, y, w, h, t, { r, g, b, a });
	});

	vm->set_function("dxDrawFilledRect", [&](float x, float y, float w, float h, float r, float g, float b, float a)
	{
		g_ui->draw_filled_rect(x, y, w, h, { r, g, b, a });
	});

	vm->set_function("dxGetScreenSize", [&]() -> std::tuple<float, float>
	{
		auto size = g_ui->get_screen_size();
		return { size.x, size.y };
	});

	vm->set_function("outputChat", [&](const char* text)
	{
		g_chat->add_chat_msg(text);
	});
	
	vm->set_function("setEffectsDrawDistance", [&](int distance)
	{
		g_effects_draw_distance = distance;
	});
}