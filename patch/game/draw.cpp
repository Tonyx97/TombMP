import prof;
import utils;

#include <specific/standard.h>
#include <specific/global.h>
#include <specific/output.h>
#include <specific/input.h>
#include <specific/hwrender.h>
#include <specific/litesrc.h>
#include <specific/frontend.h>

#include <3dsystem/3d_gen.h>

#include <scripting/events.h>

#include <shared/scripting/resource.h>
#include <shared/scripting/resource_system.h>

#include <mp/client.h>
#include <mp/game/entity.h>
#include <mp/game/player.h>
#include <mp/game/level.h>

#include <ui/ui.h>

#include "game.h"
#include "objects.h"
#include "lara.h"
#include "laraanim.h"
#include "camera.h"
#include "control.h"
#include "laraflar.h"
#include "effect2.h"
#include "invfunc.h"
#include "inventry.h"
#include "text.h"
#include "hair.h"
#include "rain.h"
#include "footprint.h"
#include "gameflow.h"
#include "triboss.h"
#include "lasers.h"
#include "laraelec.h"
#include "sphere.h"

#define CLIP_OUTSIDE
#define PutPolyLara(A, B)	phd_PutPolygons(lara.mesh_ptrs[A], B)
#define PutPolyLara_I(A, B)	phd_PutPolygons_I(lara.mesh_ptrs[A], B)

struct door_vbuf
{
	int32_t xv, yv, zv;
} vbufdoor[4];

int16_t null_rotations[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

int box_lines[12][2] =
{
	{ 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 },
	{ 4, 5 }, { 5, 6 }, { 6, 7 }, { 7, 4 },
	{ 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 }
};

int16_t interpolated_bounds[6];

#ifdef CLIP_OUTSIDE
int32_t outside_left,
	   outside_right,
	   outside_top,
	   outside_bottom;
#endif

BITE_INFO EnemyBites[16] =
{
	// X, Y, Z, Mesh.
	{ 0, 192, 40, 13 },		// OILRED
	{ 0, 400, 64, 7 },		// WHITE_SOLDIER
	{ 0, 300, 56, 7 },		// SWAT_GUN
	{ 0, 200, 106, 7 },		// SWAT_GUN_laser
	{ 110, -30, -530, 2 },	// AUTOGUN left
	{ -110, -30, -530, 2 },	// AUTOGUN right
	{ 0, 300, 56, 7 },		// ARMYSMG_GUN
};

uint8_t EnemyWeapon[16] =
{
	// Pistol or SGM ? (0 or 1) - Draws a different gun flash.
	0,		// OILRED
	1, 		// WHITE_SOLDIER
	0x81,	// SWAT_GUN (set high bit to draw laser).
	0, 		// SWAT_GUN_LASER
	1, 		// AUTOGUN left
	1, 		// AUTOGUN right
	1,		// ARMYSMG_GUN
};

int32_t* IMptr,
		 IMstack[12 * 64],
		 IM_frac,
		 IM_rate;

int bound_list[128],
	bound_start = 0,
	bound_end = 0;

void DrawRoomBox(int room_num);
void DrawBox(int16_t* bptr, int colour);

void PrintRooms(int16_t room_number)
{
	CurrentRoom = room_number;

	auto r = &room[room_number];

	if (r->flags & UNDERWATER)
		S_SetupBelowWater(camera_underwater);
	else S_SetupAboveWater(camera_underwater);

#ifdef BETTER_ROOM_SORT
	mid_sort = r->bound_active >> 8;
#endif

	phd_TranslateAbs(r->x, r->y, r->z);

	phd_left = r->left;
	phd_right = r->right;
	phd_top = r->top;
	phd_bottom = r->bottom;

	S_LightRoom(r);

	if (outside == 0)
		ClipRoom(r);

	S_InsertRoom(r->data, outside > 0);
}

void PrintObjects(int16_t room_number)
{
	CurrentRoom = room_number;

	auto r = &room[room_number];

	if (r->flags & UNDERWATER)
		S_SetupBelowWater(camera_underwater);
	else S_SetupAboveWater(camera_underwater);

#ifdef BETTER_ROOM_SORT
	mid_sort = r->bound_active >> 8;
#endif

	r->bound_active = 0;

	phd_PushMatrix();
	phd_TranslateAbs(r->x, r->y, r->z);

	phd_left = r->left;
	phd_right = r->right;
	phd_top = r->top;
	phd_bottom = r->bottom;

	auto mesh = r->mesh;

	for (int i = r->num_meshes; i > 0; --i, ++mesh)
	{
		CurrentMesh = mesh;

		if (static_objects[mesh->static_number].flags & 2)
		{
			phd_PushMatrix();
			{
				phd_TranslateAbs(mesh->x, mesh->y, mesh->z);
				phd_RotY(mesh->y_rot);

				if (int clip = S_GetObjectBounds(&static_objects[mesh->static_number].x_minp))
				{
					S_CalculateStaticMeshLight(mesh->x, mesh->y, mesh->z, mesh->shade, mesh->shadeB, r);

					if (bObjectOn)
						phd_PutPolygons(meshes[static_objects[mesh->static_number].mesh_number], clip);
				}
			}
			phd_PopMatrix();
		}
	}

	phd_left = 0;
	phd_top = 0;
	phd_right = phd_winxmax + 1;
	phd_bottom = phd_winymax + 1;

	for (auto item_num = r->item_number; item_num != NO_ITEM; item_num = items[item_num].next_item)
	{
		auto item = &items[item_num];

		if (item->status != INVISIBLE && bAObjectOn)	// remove the invisible check or make it more tolerant to a large amount of items
			(*objects[item->object_number].draw_routine)(item);

		if (item->after_death < 32 && item->after_death > 0)
		{
			if (++item->after_death == 2 ||
				item->after_death == 5 ||
				item->after_death == 11 ||
				item->after_death == 20 ||
				item->after_death == 27 ||
				item->after_death == 32 ||
				(GetRandomDraw() & 7) == 0)
			{
				DoLotsOfBloodD(item->pos.x_pos, item->pos.y_pos - 64, item->pos.z_pos, 0, GetRandomDraw() << 1, item->room_number, 1);
			}
		}
	}

	for (auto item_num = r->fx_number; item_num != NO_ITEM; item_num = effects[item_num].next_fx)
		DrawEffect(item_num);

	phd_PopMatrix();

	r->left = phd_winxmax;
	r->top = phd_winymax;
	r->right = r->bottom = 0;
}

void SetRoomBounds(int16_t* objptr, int room_number, ROOM_INFO* parent)
{
	auto r = &room[room_number];

	if (r->left <= parent->test_left && r->right >= parent->test_right && r->top <= parent->test_top && r->bottom >= parent->test_bottom)
		return;

	int left = parent->test_right,
		right = parent->test_left,
		top = parent->test_bottom,
		bottom = parent->test_top;

	objptr += 3;

	auto mptr = phd_mxptr;
	auto dest = vbufdoor;

	int z_behind = 0,
		z_toofar = 0;

	for (int i = 0; i < 4; ++i, ++dest, objptr += 3)
	{
		int xv = dest->xv = (int32_t)(*(mptr + M00) * *(objptr)+*(mptr + M01) * *(objptr + 1) + *(mptr + M02) * *(objptr + 2) + *(mptr + M03)),
			yv = dest->yv = (int32_t)(*(mptr + M10) * *(objptr)+*(mptr + M11) * *(objptr + 1) + *(mptr + M12) * *(objptr + 2) + *(mptr + M13)),
			zv = dest->zv = (int32_t)(*(mptr + M20) * *(objptr)+*(mptr + M21) * *(objptr + 1) + *(mptr + M22) * *(objptr + 2) + *(mptr + M23));

		if (zv > 0)
		{
			if (zv > phd_zfar)
				++z_toofar;

			zv /= phd_persp;

			int xs, ys;

			if (zv)
			{
				xs = xv / zv + phd_centerx;
				ys = yv / zv + phd_centery;
			}
			else
			{
				xs = (xv >= 0) ? phd_right : phd_left;
				ys = (yv >= 0) ? phd_bottom : phd_top;
			}

			if (xs - 1 < left)   left = xs - 1;
			if (xs + 1 > right)  right = xs + 1;
			if (ys - 1 < top)	 top = ys - 1;
			if (ys + 1 > bottom) bottom = ys + 1;
		}
		else ++z_behind;
	}

	if (z_behind == 4)				   return;
	if (z_toofar == 4 && outside == 0) return;

	if (z_behind > 0)
	{
		dest = vbufdoor;

		auto last = dest + 3;

		for (int i = 0; i < 4; ++i, last = dest, ++dest)
		{
			if ((dest->zv <= 0) ^ (last->zv <= 0))
			{
				if (dest->xv < 0 && last->xv < 0)	   left = 0;
				else if (dest->xv > 0 && last->xv > 0) right = phd_winxmax;
				else
				{
					left = 0;
					right = phd_winxmax;
				}

				if (dest->yv < 0 && last->yv < 0)	   top = 0;
				else if (dest->yv > 0 && last->yv > 0) bottom = phd_winymax;
				else
				{
					top = 0;
					bottom = phd_winymax;
				}
			}
		}
	}

	if (left < parent->test_left)	  left = parent->test_left;
	if (right > parent->test_right)   right = parent->test_right;
	if (top < parent->test_top)		  top = parent->test_top;
	if (bottom > parent->test_bottom) bottom = parent->test_bottom;

	if (left >= right || top >= bottom)
		return;

	if (r->bound_active & 2)
	{
		if (left < r->test_left)	 r->test_left = (int16_t)left;
		if (top < r->test_top)		 r->test_top = (int16_t)top;
		if (right > r->test_right)   r->test_right = (int16_t)right;
		if (bottom > r->test_bottom) r->test_bottom = (int16_t)bottom;
	}
	else
	{
		bound_list[(bound_end++) % 128] = room_number;

		r->bound_active |= 2;
		r->bound_active += (int16_t)(mid_sort << 8);
		r->test_left = (int16_t)left;
		r->test_right = (int16_t)right;
		r->test_top = (int16_t)top;
		r->test_bottom = (int16_t)bottom;
	}
}

void GetRoomBounds()
{
	while (bound_start != bound_end)
	{
		auto room_number = bound_list[(bound_start++) % 128];
		auto r = &room[room_number];

		r->bound_active -= 2;
		mid_sort = (r->bound_active >> 8) + 1;

		if (r->test_left < r->left)		r->left = r->test_left;
		if (r->test_top < r->top)		r->top = r->test_top;
		if (r->test_right > r->right)	r->right = r->test_right;
		if (r->test_bottom > r->bottom) r->bottom = r->test_bottom;

		if (!(r->bound_active & 1))
		{
			rooms_to_draw[number_draw_rooms++] = room_number;

			r->bound_active |= 1;

			if (r->flags & OUTSIDE)
				outside = OUTSIDE;
		}

#ifdef CLIP_OUTSIDE
		if (r->flags & OUTSIDE)
		{
			if (r->left < outside_left)		outside_left = r->left;
			if (r->right > outside_right)	outside_right = r->right;
			if (r->top < outside_top)		outside_top = r->top;
			if (r->bottom > outside_bottom) outside_bottom = r->bottom;
		}
#endif

		phd_PushMatrix();
		phd_TranslateAbs(r->x, r->y, r->z);

		if (auto door = r->door)
		{
			for (int i = *(door++); i > 0; --i, door += 15)
			{
				room_number = *(door++);

				if ((int32_t)(*(door + 0) * (r->x + *(door + 3) - *(w2v_matrix + M03))) +
					(int32_t)(*(door + 1) * (r->y + *(door + 4) - *(w2v_matrix + M13))) +
					(int32_t)(*(door + 2) * (r->z + *(door + 5) - *(w2v_matrix + M23))) >= 0)
					continue;

				SetRoomBounds(door, room_number, r);
			}
		}

		phd_PopMatrix();
	}
}

int32_t draw_phase_game()
{
	draw_rooms(camera.pos.room_number);
	draw_game_info();
	display_inventory(0);
	output_polylist();
	animate_textures(camera.number_frames = dump_screen());

	return camera.number_frames;
}

void draw_rooms(int16_t current_room)
{
	CurrentRoom = current_room;

	auto r = &room[current_room];

	phd_left = r->test_left = 0;
	phd_top = r->test_top = 0;
	phd_right = r->test_right = phd_winxmax;
	phd_bottom = r->test_bottom = phd_winymax;

	outside = r->flags & OUTSIDE;
	camera_underwater = r->flags & UNDERWATER;

	r->bound_active = 2;

	bound_list[0] = current_room;
	bound_start = 0;
	bound_end = 1;

	number_draw_rooms = 0;

#ifdef CLIP_OUTSIDE
	if (outside)
	{
		outside_left = outside_top = 0;
		outside_right = phd_winxmax;
		outside_bottom = phd_winymax;
	}
	else
	{
		outside_left = phd_winxmax;
		outside_top = phd_winymax;
		outside_right = outside_bottom = 0;
	}
#endif

	GetRoomBounds();

	mid_sort = 0;

	if (outside)
	{
#ifdef CLIP_OUTSIDE
		phd_left = outside_left;
		phd_right = outside_right;
		phd_top = outside_top;
		phd_bottom = outside_bottom;
#endif
		if (objects[HORIZON].loaded)
		{
			S_SetupAboveWater(camera_underwater);

			phd_PushMatrix();
			{
				*(phd_mxptr + M03) = *(phd_mxptr + M13) = *(phd_mxptr + M23) = 0;

				auto frame = anims[objects[HORIZON].anim_index].frame_ptr + 9;

				gar_RotYXZsuperpack(&frame, 0);

				S_InitialisePolyList(SIPL_DONT_CLEAR_SCREEN);
				S_InsertBackground(*objects[HORIZON].mesh_ptr);
			}
			phd_PopMatrix();
		}
		else
		{
			S_InitialisePolyList(SIPL_CLEAR_SCREEN);

			outside = -1;
		}
	}
	else S_InitialisePolyList(SIPL_DONT_CLEAR_SCREEN);

	bool electric_points_updated = false;

	if (objects[LARA].loaded)
	{
		if (lara.underwater = (room[lara_item->room_number].flags & UNDERWATER))
			S_SetupBelowWater(camera_underwater);
		else S_SetupAboveWater(camera_underwater);

		if (mid_sort = room[lara_item->room_number].bound_active >> 8)
			--mid_sort;

		// render localplayer

		auto localplayer = game_level::LOCALPLAYER();

		if (lara.spawned && !(localplayer->get_entity_flags() & ENTITY_FLAG_INVISIBLE) && g_resource->trigger_event(events::renderer::ON_PLAYER_RENDER, localplayer))
			draw_lara(lara_item, (vec3d*)g_hair, lara.weapon_item != NO_ITEM ? items[lara.weapon_item].current_anim_state : -1, false);

		// render all players

		decltype(lara) saved_lara;

		bool saved_blue_effect = g_blue_effect,
			 trigger_local_player_fire = false,
			 any_electric_player = false;

		memcpy(&saved_lara, &lara, sizeof(lara));

		auto local_cam_pos = int_vec3(camera.pos.x, camera.pos.y, camera.pos.z);

		g_level->for_each_player([&](game_player* player)
		{
			auto player_item = player->get_item();

			if (player->is_spawned() &&
				!(player->get_entity_flags() & ENTITY_FLAG_INVISIBLE) &&
				!(player_item->ai_bits & EXPLODED) &&	// this flag is set when a player explodes locally and it's not synced so we don't get flickers
				g_resource->trigger_event(events::renderer::ON_PLAYER_RENDER, player))
			{
				// fix delayed position of player while in vehicle by updating the pos of the vehicle
				
				auto vehicle = player->get_vehicle();

				if (vehicle)
				{
					player->set_position(vehicle->get_position());
					player->set_rotation(vehicle->get_rotation());
				}

				// dispatch everything needed to render a remote lara

				const auto& left_arm = player->get_left_arm_info(),
							right_arm = player->get_right_arm_info();

				const auto& player_name = player->get_name();

				auto player_pos = player->get_position();
				auto player_rot = player->get_rotation();
				auto health = player->get_health();
				auto meshes_offsets = player->get_meshes_offsets();
				auto hair_data = player->get_hair_data();
				auto hair_vel = player->get_hair_vel();

				// interpolate player variables

				if (!vehicle)
				{
					player->set_position(player_pos.interpolate(player->get_next_position(), 0.7f));
					
					//player->set_anim_frame(int16_t(std::lerp(float(player->get_anim_frame()), float(player->get_next_frame()), 0.1f)));
				}

				utils::mem::move(lara.head_y_rot, player->get_head_rotation());
				utils::mem::move(lara.torso_y_rot, player->get_torso_rotation());
				utils::mem::move(lara.left_arm.y_rot, left_arm.rotation);
				utils::mem::move(lara.right_arm.y_rot, right_arm.rotation);

				for (int i = 0; i < MAX_LARA_MESHES; ++i)
					lara.mesh_ptrs[i] = meshes_base + meshes_offsets[i];
				
				lara.flare_age = player->get_flare_age();
				lara.flare_control_left = player->is_flare_in_hand();
				lara.back_gun = player->get_back_gun();
				lara.hit_direction = player->get_hit_direction();
				lara.hit_frame = player->get_hit_frame();
				lara.gun_type = player->get_gun_type();
				lara.gun_status = player->get_gun_status();
				lara.is_ducked = player->is_ducked();
				lara.water_status = player->get_water_status();
				lara.left_arm.frame_base = frames + left_arm.frame_base;
				lara.left_arm.anim_number = left_arm.anim;
				lara.left_arm.frame_number = left_arm.frame;
				lara.left_arm.flash_gun = left_arm.flash_gun;
				lara.right_arm.frame_base = frames + right_arm.frame_base;
				lara.right_arm.anim_number = right_arm.anim;
				lara.right_arm.frame_number = right_arm.frame;
				lara.right_arm.flash_gun = right_arm.flash_gun;

				HairControl(player_item, hair_data, hair_vel);

				g_blue_effect = player->is_underwater();

				// create fire on the player if it's not created

				if (player->is_burning())
				{
					const auto fire_color = player->get_fire_color();

					draw_lara_fire(player_item, fire_color.x, fire_color.y, fire_color.z);

					if (!trigger_local_player_fire)
					{
						PHD_3DPOS fire_pos { player_pos.x, player_pos.y, player_pos.z };

						if (ItemNearLara(lara_item, &fire_pos, 600))
						{
							int x = lara_item->pos.x_pos - player_pos.x,
								z = lara_item->pos.z_pos - player_pos.z,
								distance = x * x + z * z;

							lara_item->hit_points -= FLAME_TOONEAR_DAMAGE;
							lara_item->hit_status = 1;

							if (distance < SQUARE(450))
								trigger_local_player_fire = true;
						}
					}
				}

				// render electric points on the player

				if (player->is_electric())
				{
					any_electric_player = true;

					LaraElectricDeath(0, player_item);
					LaraElectricDeath(1, player_item);

					TriggerDynamicLight(player_pos.x, player_pos.y, player_pos.z, (GetRandomControl() & 3) + 8, 0, (GetRandomControl() & 7) + 8, (GetRandomControl() & 7) + 16);
				}

				// draw player info on top of the head

				if (g_client->get_game_settings().player_info)
				{
					if (auto distance_to_player = float(local_cam_pos.distance(player_pos)); distance_to_player > 0.f && distance_to_player < 13470.f)
					{
						PHD_VECTOR head_pos {};

						get_lara_bone_pos(player_item, &head_pos, HEAD);

						if (int ox, oy; world_to_screen(head_pos.x, head_pos.y - 128, head_pos.z, ox, oy) && ox > 0 && oy > 0)
						{
							float name_size_adjust = 1.f,
								  hp_bar_size_adjust = 1.f;

							if (distance_to_player > 1.f)
							{
								name_size_adjust = 1.f - (distance_to_player / 31430.f);
								hp_bar_size_adjust = 1.f - (distance_to_player / 15715.f);
							}

							const float hp = float(std::clamp<int16_t>(health, 0, LARA_HITPOINTS) / 10) * hp_bar_size_adjust,
										hp_color = hp / (100.f * hp_bar_size_adjust);

							const float hp_bar_size_x = 100.f * hp_bar_size_adjust,
										hp_border_size = 2.f;

							g_ui->draw_filled_rect(
								ox - hp_border_size - (hp_bar_size_x / 2.f + hp_border_size / 2.f),
								oy - 2.f,
								hp_bar_size_x + hp_border_size * 2.f,
								5.f * hp_bar_size_adjust + hp_border_size * 2.f,
								{ 0.f, 0.f, 0.f, 1.f });

							g_ui->draw_filled_rect(
								ox - (hp_bar_size_x / 2.f + hp_border_size / 2.f),
								oy,
								hp,
								5.f * hp_bar_size_adjust,
								{ 1.f - hp_color, hp_color, 0.f, 1.f });

							g_ui->add_text(player_name.c_str(), ox, oy - 15.f, 18.f * name_size_adjust, { 1.f, 1.f, 1.f, 1.f }, true);
						}
					}
				}

				// draw player's flare light

				if (lara.mesh_ptrs[HAND_L] == *(objects[FLARE].mesh_ptr + HAND_L) && lara.flare_age > 0 && lara.flare_age < FLARE_DEAD)
				{
					PHD_VECTOR flare_pos;

					get_lara_bone_pos(player_item, &flare_pos, HAND_L);

					do_flare_light(&flare_pos, lara.flare_age);
				}

				// draw player's weapon smokes

				draw_weapon_smoke(player_item, player->get_smoke_weapon(), player->get_smoke_count_l(), player->get_smoke_count_r());

				// draw player's weapon muzzle light

				if (right_arm.flash_gun)
				{
					if (auto gun_type = player->get_gun_type(); is_1gun(gun_type))
					{
						TriggerDynamicLight(
							player_pos.x + (phd_sin(player_rot.y) >> (W2V_SHIFT - 10)) + ((GetRandomControl() & 255) - 128),
							player_pos.y - WALL_L / 2 + ((GetRandomControl() & 127) - 63),
							player_pos.z + (phd_cos(player_rot.y) >> (W2V_SHIFT - 10)) + ((GetRandomControl() & 255) - 128),
							(gun_type == LG_SHOTGUN) ? 12 : 11,
							(GetRandomControl() & 7) + 24,
							(GetRandomControl() & 3) + 16,
							GetRandomControl() & 7);
					}
					else if (is_2guns(gun_type))
					{
						PHD_VECTOR pos { (GetRandomControl() & 255) - 128, (GetRandomControl() & 127) - 63, (GetRandomControl() & 255) - 128 };

						get_lara_bone_pos(player_item, &pos, HAND_R);

						TriggerDynamicLight(pos.x, pos.y, pos.z, 12, (GetRandomControl() & 7) + 24, (GetRandomControl() & 3) + 16, GetRandomControl() & 7);
					}
				}

				// draw player's lara

				draw_lara(player_item, hair_data, player->get_weapon_item_current_anim_state(), true);
			}
		});

		memcpy(&lara, &saved_lara, sizeof(lara));

		g_blue_effect = saved_blue_effect;

		if (trigger_local_player_fire)
			LaraBurn();

		if (electric_points_updated = any_electric_player)
			UpdateElectricityPoints();
	}

	// interpolate vehicles
	
	/*g_level->for_each_level_entity([&](game_entity* entity)
	{
		if (!entity->is_spawned())
			return;

		if (entity->get_subtype() == ENTITY_LEVEL_TYPE_VEHICLE && !g_level->is_entity_streamed(entity))
			entity->set_position(entity->get_position().interpolate(entity->get_next_position(), 0.25f));
	});*/

	if (bRoomOn)
		for (int i = 0; i < number_draw_rooms; ++i)
		{
			auto r = rooms_to_draw[i];

			PrintRooms(r);
			PrintObjects(r);
		}

	if (bEffectOn)
	{
		S_DrawSparks();
		S_DrawSplashes();
		S_DrawBat();
		//DoRain();

		if (enable_footprints)
			S_DrawFootPrints();
	}
	
	if (lara.electric)
	{
		if (lara.electric < 16)
			++lara.electric;

		if (!electric_points_updated)
			UpdateElectricityPoints();

		LaraElectricDeath(0, lara_item);
		LaraElectricDeath(1, lara_item);
	}
}

void ClipRoom(ROOM_INFO* r)
{
	int32_t xv[8], yv[8], zv[8], clip[8];

	xv[0] = xv[3] = xv[4] = xv[7] = WALL_L;							// min x
	yv[0] = yv[1] = yv[2] = yv[3] = r->maxceiling - r->y;
	zv[0] = zv[1] = zv[4] = zv[5] = WALL_L;							// min z

	xv[1] = xv[2] = xv[5] = xv[6] = (r->y_size - 1) * WALL_L;		// max x
	yv[4] = yv[5] = yv[6] = yv[7] = r->minfloor - r->y;				// max y
	zv[2] = zv[3] = zv[6] = zv[7] = (r->x_size - 1) * WALL_L;		// max z

	auto mptr = phd_mxptr;

	int clip_room = 0;

	for (int i = 0; i < 8; ++i)
	{
		int x = xv[i],
			y = yv[i],
			z = zv[i];

		xv[i] = (*(mptr + M00) * x) + (*(mptr + M01) * y) + (*(mptr + M02) * z) + *(mptr + M03);
		yv[i] = (*(mptr + M10) * x) + (*(mptr + M11) * y) + (*(mptr + M12) * z) + *(mptr + M13);
		zv[i] = (*(mptr + M20) * x) + (*(mptr + M21) * y) + (*(mptr + M22) * z) + *(mptr + M23);

		clip[i] = (zv[i] > phd_zfar ? (clip_room = 1) : 0);
	}

	if (!clip_room)
		return;

	int min_x = 0x10000000,
		min_y = min_x,
		max_x = -0x10000000,
		max_y = max_x;

	for (int i = 0; i < 12; ++i)
	{
		auto p1 = box_lines[i][0],
			 p2 = box_lines[i][1];

		if (clip[p1] ^ clip[p2])
		{
			int zdiv = (zv[p2] - zv[p1]) >> W2V_SHIFT;

			if (zdiv)
			{
				int znom = (phd_zfar - zv[p1]) >> W2V_SHIFT,
					x = xv[p1] + ((((xv[p2] - xv[p1]) >> W2V_SHIFT) * znom / zdiv) << W2V_SHIFT),
					y = yv[p1] + ((((yv[p2] - yv[p1]) >> W2V_SHIFT) * znom / zdiv) << W2V_SHIFT);

				if (x < min_x) min_x = x;
				if (x > max_x) max_x = x;
				if (y < min_y) min_y = y;
				if (y > max_y) max_y = y;
			}
			else
			{
				if (xv[p1] < min_x) min_x = xv[p1];
				if (xv[p2] < min_x) min_x = xv[p2];
				if (xv[p1] > max_x) max_x = xv[p1];
				if (xv[p2] > max_x) max_x = xv[p2];
				if (yv[p1] < min_y) min_y = yv[p1];
				if (yv[p2] < min_y) min_y = yv[p2];
				if (yv[p1] > max_y) max_y = yv[p1];
				if (yv[p2] > max_y) max_y = yv[p2];
			}
		}
	}

	int z = phd_zfar / phd_persp;

	min_x = min_x / z + phd_centerx;
	min_y = min_y / z + phd_centery;
	max_x = max_x / z + phd_centerx;
	max_y = max_y / z + phd_centery;

	if ((min_x > phd_right) || (min_y > phd_bottom) || (max_x < phd_left) || (max_y < phd_top))
		return;

	if (min_x < phd_left)   min_x = phd_left;
	if (min_y < phd_top)	min_y = phd_top;
	if (max_x > phd_right)  max_x = phd_right;
	if (max_y > phd_bottom) max_y = phd_bottom;

	S_InsertBackPolygon(min_x, min_y, max_x, max_y, 0x00800000);
}

void DrawEffect(int16_t fx_num)
{
	auto fx = &effects[fx_num];

	if (objects[fx->object_number].draw_routine == DrawDummyItem)
		return;

	auto object = &objects[fx->object_number];

	if (!object->loaded)
	{
		prof::print(RED, "Object not Loaded: {}", fx->object_number);
		return;
	}

	if (fx->object_number == GLOW)
		return S_DrawSprite(
			((uint16_t)fx->pos.x_rot) | (fx->pos.y_rot << 16),
			fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos,
			(int16_t*)objects[GLOW].mesh_ptr,
			fx->shade,
			fx->frame_number);

	if (object->nmeshes < 0)
		S_DrawSprite(SPRITE_ABS | (object->semi_transparent ? SPRITE_SEMITRANS : 0) | SPRITE_SHADE, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, (int16_t*)(object->mesh_ptr - fx->frame_number), fx->shade, 0);
	else
	{
		phd_PushMatrix();
		{
			phd_TranslateAbs(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos);

			if (*(phd_mxptr + M23) > phd_znear && *(phd_mxptr + M23) < phd_zfar)
			{
				phd_RotYXZ(fx->pos.y_rot, fx->pos.x_rot, fx->pos.z_rot);

				if (object->nmeshes)
				{
					S_CalculateLight(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, fx->room_number, NULL);
					phd_PutPolygons(*object->mesh_ptr, -1);
				}
				else
				{
					S_CalculateLight(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, fx->room_number, NULL);
					phd_PutPolygons(meshes[fx->frame_number], -1);
				}
			}
		}
		phd_PopMatrix();
	}
}

void DrawDummyItem(ITEM_INFO* item) {}

void DrawAnimatingItem(ITEM_INFO* item)
{
	int16_t* frmptr[2] = { nullptr, nullptr };

	int rate;

	auto frac = GetFrames(item, frmptr, &rate);

	if (!frmptr[0] || !frmptr[1])
		return;

	auto object = &objects[item->object_number];

	if (object->shadow_size)
		S_PrintShadow(object->shadow_size, frmptr[0], item, 0);

	phd_PushMatrix();
	phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
	phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);

	if (int clip = S_GetObjectBounds(frmptr[0]))
	{
		CalculateObjectLighting(item, frmptr[0]);

		auto extra_rotation = (item->data ? (int16_t*)item->data : null_rotations);
		auto meshpp = object->mesh_ptr;
		auto bone = object->bone_ptr;

		uint32_t bit = 1;

		if (!frac)
		{
			phd_TranslateRel((int32_t)*(frmptr[0] + 6), (int32_t)*(frmptr[0] + 7), (int32_t)*(frmptr[0] + 8));

			auto rotation1 = frmptr[0] + 9;

			gar_RotYXZsuperpack(&rotation1, 0);

			if (bit & item->mesh_bits)
				phd_PutPolygons(*meshpp, clip);

			++meshpp;

			for (int i = 0; i < object->nmeshes - 1; ++i, bone += 4, ++meshpp)
			{
				int poppush = *(bone);

				if (poppush & 1) phd_PopMatrix();
				if (poppush & 2) phd_PushMatrix();

				phd_TranslateRel(*(bone + 1), *(bone + 2), *(bone + 3));
				gar_RotYXZsuperpack(&rotation1, 0);

				if (poppush & (ROT_X | ROT_Y | ROT_Z))
				{
					if (poppush & ROT_Y) phd_RotY(*(extra_rotation++));
					if (poppush & ROT_X) phd_RotX(*(extra_rotation++));
					if (poppush & ROT_Z) phd_RotZ(*(extra_rotation++));
				}

				bit <<= 1;

				if (bit & item->mesh_bits)
					phd_PutPolygons(*meshpp, clip);

				if (item->fired_weapon && i == (EnemyBites[object->bite_offset].mesh_num - 1))
				{
					auto bi = &EnemyBites[object->bite_offset];

					int rnd = GetRandomDraw();

					phd_PushMatrix();
					phd_TranslateRel(bi->x, bi->y, bi->z);
					phd_RotYXZ(0, item->object_number == ROBOT_SENTRY_GUN ? 180 * ONE_DEGREE : -90 * ONE_DEGREE, (int16_t)((rnd & 3) << 14) + (rnd >> 2) - 4096);

					S_DrawSprite(SPRITE_REL | SPRITE_SEMITRANS | SPRITE_TRANS_ADD | SPRITE_TINT | SPRITE_COLOUR(0x3f, 0x38, 0x8) | SPRITE_SCALE, 0, 0, 0, (int16_t*)objects[GLOW].mesh_ptr, 0, 0xc0);

					S_CalculateStaticLight(24 | (18 << 5));

					phd_PutPolygons(*objects[(EnemyWeapon[object->bite_offset] & 1) ? M16_FLASH : GUN_FLASH].mesh_ptr, clip);

					phd_PopMatrix();

					--item->fired_weapon;
				}

				if (i == EnemyBites[object->bite_offset].mesh_num - 1 && (EnemyWeapon[object->bite_offset] & 0x80))
				{
					auto bi = &EnemyBites[object->bite_offset + 1];

					PHD_VECTOR pos { bi->x, bi->y, bi->z };

					GetJointAbsPosition(item, &pos, bi->mesh_num);

					GAME_VECTOR	src { pos.x, pos.y, pos.z, item->room_number };

					pos = { bi->x, bi->y << 5, bi->z };

					GetJointAbsPosition(item, &pos, bi->mesh_num);

					GAME_VECTOR	dest { pos.x, pos.y, pos.z };

					LOS(&src, &dest);

					S_DrawLaserBeam(&src, &dest, 255, 2, 3);
				}
			}
		}
		else
		{
			InitInterpolate(frac, rate);

			phd_TranslateRel_ID(
				(int32_t)*(frmptr[0] + 6),
				(int32_t)*(frmptr[0] + 7),
				(int32_t)*(frmptr[0] + 8),
				(int32_t)*(frmptr[1] + 6),
				(int32_t)*(frmptr[1] + 7),
				(int32_t)*(frmptr[1] + 8));

			auto rotation1 = frmptr[0] + 9,
				 rotation2 = frmptr[1] + 9;

			gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);

			if (bit & item->mesh_bits)
				phd_PutPolygons_I(*meshpp, clip);

			++meshpp;

			for (int i = 0; i < object->nmeshes - 1; ++i, bone += 4, ++meshpp)
			{
				int poppush = *(bone);

				if (poppush & 1) phd_PopMatrix_I();
				if (poppush & 2) phd_PushMatrix_I();

				phd_TranslateRel_I(*(bone + 1), *(bone + 2), *(bone + 3));
				gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);

				if (poppush & (ROT_X | ROT_Y | ROT_Z))
				{
					if (poppush & ROT_Y) phd_RotY_I(*(extra_rotation++));
					if (poppush & ROT_X) phd_RotX_I(*(extra_rotation++));
					if (poppush & ROT_Z) phd_RotZ_I(*(extra_rotation++));
				}

				bit <<= 1;

				if (bit & item->mesh_bits)
					phd_PutPolygons_I(*meshpp, clip);

				if (item->fired_weapon && i == (EnemyBites[object->bite_offset].mesh_num - 1))
				{
					auto bi = &EnemyBites[object->bite_offset];

					int16_t rnd = GetRandomDraw();

					phd_PushMatrix_I();

					phd_TranslateRel_I(bi->x, bi->y, bi->z);
					phd_RotYXZ_I(0, -90 * ONE_DEGREE, (int16_t)((rnd & 3) << 14) + (rnd >> 2) - 4096);
					InterpolateMatrix();

					S_DrawSprite(SPRITE_REL | SPRITE_SEMITRANS | SPRITE_TRANS_ADD | SPRITE_TINT | SPRITE_COLOUR(0x3f, 0x38, 0x8) | SPRITE_SCALE, 0, 0, 0, (int16_t*)objects[GLOW].mesh_ptr, 0, 0xc0);
					S_CalculateStaticLight(24 | (18 << 5));
					phd_PutPolygons(*objects[(EnemyWeapon[object->bite_offset] & 1) ? M16_FLASH : GUN_FLASH].mesh_ptr, clip);

					phd_PopMatrix_I();

					--item->fired_weapon;
				}
			}
		}
	}
	else if (item->clear_body && item->hit_points <= 0 && item->after_death >= 32)
		KillItem(item - items);

	phd_PopMatrix();
}

void draw_lara(ITEM_INFO* item, vec3d* hair_data, short weapon_current_anim_state, bool remote)
{
	int left = phd_left,
		right = phd_right,
		top = phd_top,
		bottom = phd_bottom;

	phd_left = phd_top = 0;
	phd_bottom = phd_winymax;
	phd_right = phd_winxmax;

	PHD_VECTOR pos {};

	if (!GotJointPos[FOOT_L]) get_lara_bone_pos(lara_item, &pos, FOOT_L);
	if (!GotJointPos[FOOT_R]) get_lara_bone_pos(lara_item, &pos, FOOT_R);
	if (!GotJointPos[HAND_L]) get_lara_bone_pos(lara_item, &pos, HAND_L);
	if (!GotJointPos[HAND_R]) get_lara_bone_pos(lara_item, &pos, HAND_R);
	if (!GotJointPos[HEAD])   get_lara_bone_pos(lara_item, &pos, HEAD);

	int16_t* frmptr[2] = { nullptr, nullptr };

	int	rate,
		frac = GetFrames(item, frmptr, &rate);

	if (!frmptr[0] || !frmptr[1])
		return;

	if (lara.hit_direction < 0 && frac)
	{
		draw_lara_interpolation(item, hair_data, frmptr[0], frmptr[1], frac, rate, weapon_current_anim_state, remote);

		phd_left = left;
		phd_right = right;
		phd_top = top;
		phd_bottom = bottom;

		return;
	}

	auto object = &objects[item->object_number];

	int16_t* frame;

	int size;

	if (lara.hit_direction >= 0)
	{
		int16_t spaz;

		switch (lara.hit_direction)
		{
		case NORTH: spaz = (lara.is_ducked ? SPAZ_DUCKF_A : SPAZ_FORWARD_A); break;
		case SOUTH: spaz = (lara.is_ducked ? SPAZ_DUCKB_A : SPAZ_BACK_A);    break;
		case EAST:  spaz = (lara.is_ducked ? SPAZ_DUCKR_A : SPAZ_RIGHT_A);   break;
		default:    spaz = (lara.is_ducked ? SPAZ_DUCKL_A : SPAZ_LEFT_A);    break;
		}

		auto anim = &anims[spaz];

		size = anim->interpolation >> 8;

		frame = (int16_t*)anim->frame_ptr + (lara.hit_frame * size);
	}
	else frame = frmptr[0];

	if (lara.skidoo == NO_ITEM || (lara.extra_anim && item->current_anim_state == EXTRA_RAPIDSDROWN))
		S_PrintShadow(object->shadow_size, frame, item, 0);

	int32_t savemox[12];

	memcpy(savemox, phd_mxptr, 12 * 4);

	phd_PushMatrix();
	phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
	phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);

	int clip = S_GetObjectBounds(frame);

	if (!clip)
		clip = -1;

	phd_PushMatrix();

	CalculateObjectLightingLara();

	auto bone = object->bone_ptr;
	auto rotation = frame + 9;

	phd_TranslateRel((int32_t)*(frame + 6), (int32_t)*(frame + 7), (int32_t)*(frame + 8));
	gar_RotYXZsuperpack(&rotation, 0);

	PutPolyLara(HIPS, clip);

	phd_PushMatrix();

	phd_TranslateRel(*(bone + 1), *(bone + 2), *(bone + 3));
	gar_RotYXZsuperpack(&rotation, 0);
	PutPolyLara(THIGH_L, clip);

	phd_TranslateRel(*(bone + 1 + 4), *(bone + 2 + 4), *(bone + 3 + 4));
	gar_RotYXZsuperpack(&rotation, 0);
	PutPolyLara(CALF_L, clip);

	phd_TranslateRel(*(bone + 1 + 8), *(bone + 2 + 8), *(bone + 3 + 8));
	gar_RotYXZsuperpack(&rotation, 0);
	PutPolyLara(FOOT_L, clip);

	phd_PopMatrix();

	phd_PushMatrix();

	phd_TranslateRel(*(bone + 1 + 12), *(bone + 2 + 12), *(bone + 3 + 12));
	gar_RotYXZsuperpack(&rotation, 0);
	PutPolyLara(THIGH_R, clip);

	phd_TranslateRel(*(bone + 1 + 16), *(bone + 2 + 16), *(bone + 3 + 16));
	gar_RotYXZsuperpack(&rotation, 0);
	PutPolyLara(CALF_R, clip);

	phd_TranslateRel(*(bone + 1 + 20), *(bone + 2 + 20), *(bone + 3 + 20));
	gar_RotYXZsuperpack(&rotation, 0);
	PutPolyLara(FOOT_R, clip);

	phd_PopMatrix();

	phd_TranslateRel(*(bone + 1 + 24), *(bone + 2 + 24), *(bone + 3 + 24));

	if (lara.gun_type == LG_M16 &&
		(weapon_current_anim_state == 0 ||
		 weapon_current_anim_state == 2 ||
		 weapon_current_anim_state == 4))
	{
		rotation = lara.right_arm.frame_base + lara.right_arm.frame_number * (anims[lara.right_arm.anim_number].interpolation >> 8) + 9;
		gar_RotYXZsuperpack(&rotation, 7);
	}
	else gar_RotYXZsuperpack(&rotation, 0);

	phd_RotYXZ(lara.torso_y_rot, lara.torso_x_rot, lara.torso_z_rot);

	PutPolyLara(TORSO, clip);

	phd_PushMatrix();

	phd_TranslateRel(*(bone + 1 + 52), *(bone + 2 + 52), *(bone + 3 + 52));

	auto rotationw = rotation;

	gar_RotYXZsuperpack(&rotationw, 6);
	phd_RotYXZ(lara.head_y_rot, lara.head_x_rot, lara.head_z_rot);
	PutPolyLara(HEAD, clip);

	memcpy(phd_mxptr, savemox, 12 * 4);

	DrawHair(hair_data);

	phd_PopMatrix();

	if (lara.back_gun)
	{
		phd_PushMatrix();
		{
			auto bonew = objects[lara.back_gun].bone_ptr;

			phd_TranslateRel(*(bonew + 1 + 52), *(bonew + 2 + 52), *(bonew + 3 + 52));

			rotationw = objects[lara.back_gun].frame_base + 9;

			gar_RotYXZsuperpack(&rotationw, 14);
			phd_PutPolygons(*(objects[lara.back_gun].mesh_ptr + HEAD), clip);
		}
		phd_PopMatrix();
	}

	int16_t fire_arms = LG_UNARMED;

	if (lara.gun_status == LG_READY || lara.gun_status == LG_SPECIAL || lara.gun_status == LG_DRAW || lara.gun_status == LG_UNDRAW)
		fire_arms = lara.gun_type;

	bool do_flare = false;

	switch (fire_arms)
	{
	case LG_UNARMED:
	case LG_FLARE:
	{
		phd_PushMatrix();

		phd_TranslateRel(*(bone + 1 + 28), *(bone + 2 + 28), *(bone + 3 + 28));
		gar_RotYXZsuperpack(&rotation, 0);
		PutPolyLara(UARM_R, clip);

		phd_TranslateRel(*(bone + 1 + 32), *(bone + 2 + 32), *(bone + 3 + 32));
		gar_RotYXZsuperpack(&rotation, 0);
		PutPolyLara(LARM_R, clip);

		phd_TranslateRel(*(bone + 1 + 36), *(bone + 2 + 36), *(bone + 3 + 36));
		gar_RotYXZsuperpack(&rotation, 0);
		PutPolyLara(HAND_R, clip);

		phd_PopMatrix();

		phd_PushMatrix();

		phd_TranslateRel(*(bone + 1 + 40), *(bone + 2 + 40), *(bone + 3 + 40));

		if (lara.flare_control_left)
		{
			rotation = lara.left_arm.frame_base + (lara.left_arm.frame_number - anims[lara.left_arm.anim_number].frame_base) * (anims[lara.left_arm.anim_number].interpolation >> 8) + 9;

			gar_RotYXZsuperpack(&rotation, 11);
		}
		else gar_RotYXZsuperpack(&rotation, 0);

		PutPolyLara(UARM_L, clip);

		phd_TranslateRel(*(bone + 1 + 44), *(bone + 2 + 44), *(bone + 3 + 44));
		gar_RotYXZsuperpack(&rotation, 0);
		PutPolyLara(LARM_L, clip);

		phd_TranslateRel(*(bone + 1 + 48), *(bone + 2 + 48), *(bone + 3 + 48));
		gar_RotYXZsuperpack(&rotation, 0);
		PutPolyLara(HAND_L, clip);

		if (lara.gun_type == LG_FLARE && lara.left_arm.flash_gun)
		{
			do_flare = true;

			DrawGunFlash(LG_FLARE, clip);
		}

		break;
	}
	case LG_PISTOLS:
	case LG_UZIS:
	{
		phd_PushMatrix();
		phd_TranslateRel(*(bone + 1 + 28), *(bone + 2 + 28), *(bone + 3 + 28));

		*(phd_mxptr + M00) = *(phd_mxptr + M00 - 24);
		*(phd_mxptr + M01) = *(phd_mxptr + M01 - 24);
		*(phd_mxptr + M02) = *(phd_mxptr + M02 - 24);
		*(phd_mxptr + M10) = *(phd_mxptr + M10 - 24);
		*(phd_mxptr + M11) = *(phd_mxptr + M11 - 24);
		*(phd_mxptr + M12) = *(phd_mxptr + M12 - 24);
		*(phd_mxptr + M20) = *(phd_mxptr + M20 - 24);
		*(phd_mxptr + M21) = *(phd_mxptr + M21 - 24);
		*(phd_mxptr + M22) = *(phd_mxptr + M22 - 24);

		phd_RotYXZ(lara.right_arm.y_rot, lara.right_arm.x_rot, lara.right_arm.z_rot);

		rotation = lara.right_arm.frame_base + (lara.right_arm.frame_number - anims[lara.right_arm.anim_number].frame_base) * (anims[lara.right_arm.anim_number].interpolation >> 8) + 9;

		gar_RotYXZsuperpack(&rotation, 8);
		PutPolyLara(UARM_R, clip);

		phd_TranslateRel(*(bone + 1 + 32), *(bone + 2 + 32), *(bone + 3 + 32));
		gar_RotYXZsuperpack(&rotation, 0);
		PutPolyLara(LARM_R, clip);

		phd_TranslateRel(*(bone + 1 + 36), *(bone + 2 + 36), *(bone + 3 + 36));
		gar_RotYXZsuperpack(&rotation, 0);
		PutPolyLara(HAND_R, clip);

		if (lara.right_arm.flash_gun)
			memcpy(savemox, phd_mxptr, 12 * 4);

		phd_PopMatrix();

		phd_PushMatrix();
		phd_TranslateRel(*(bone + 1 + 40), *(bone + 2 + 40), *(bone + 3 + 40));

		*(phd_mxptr + M00) = *(phd_mxptr + M00 - 24);
		*(phd_mxptr + M01) = *(phd_mxptr + M01 - 24);
		*(phd_mxptr + M02) = *(phd_mxptr + M02 - 24);
		*(phd_mxptr + M10) = *(phd_mxptr + M10 - 24);
		*(phd_mxptr + M11) = *(phd_mxptr + M11 - 24);
		*(phd_mxptr + M12) = *(phd_mxptr + M12 - 24);
		*(phd_mxptr + M20) = *(phd_mxptr + M20 - 24);
		*(phd_mxptr + M21) = *(phd_mxptr + M21 - 24);
		*(phd_mxptr + M22) = *(phd_mxptr + M22 - 24);

		phd_RotYXZ(lara.left_arm.y_rot, lara.left_arm.x_rot, lara.left_arm.z_rot);

		rotation = lara.left_arm.frame_base + (lara.left_arm.frame_number - anims[lara.left_arm.anim_number].frame_base) * (anims[lara.left_arm.anim_number].interpolation >> 8) + 9;

		gar_RotYXZsuperpack(&rotation, 11);
		PutPolyLara(UARM_L, clip);

		phd_TranslateRel(*(bone + 1 + 44), *(bone + 2 + 44), *(bone + 3 + 44));
		gar_RotYXZsuperpack(&rotation, 0);
		PutPolyLara(LARM_L, clip);

		phd_TranslateRel(*(bone + 1 + 48), *(bone + 2 + 48), *(bone + 3 + 48));
		gar_RotYXZsuperpack(&rotation, 0);
		PutPolyLara(HAND_L, clip);

		if (lara.left_arm.flash_gun)
			DrawGunFlash(fire_arms, clip);

		if (lara.right_arm.flash_gun)
		{
			memcpy(phd_mxptr, savemox, 12 * 4);
			DrawGunFlash(fire_arms, clip);
		}

		break;
	}
	case LG_MAGNUMS:
	{
		phd_PushMatrix();
		phd_TranslateRel(*(bone + 1 + 28), *(bone + 2 + 28), *(bone + 3 + 28));

		*(phd_mxptr + M00) = *(phd_mxptr + M00 - 24);
		*(phd_mxptr + M01) = *(phd_mxptr + M01 - 24);
		*(phd_mxptr + M02) = *(phd_mxptr + M02 - 24);
		*(phd_mxptr + M10) = *(phd_mxptr + M10 - 24);
		*(phd_mxptr + M11) = *(phd_mxptr + M11 - 24);
		*(phd_mxptr + M12) = *(phd_mxptr + M12 - 24);
		*(phd_mxptr + M20) = *(phd_mxptr + M20 - 24);
		*(phd_mxptr + M21) = *(phd_mxptr + M21 - 24);
		*(phd_mxptr + M22) = *(phd_mxptr + M22 - 24);

		phd_RotYXZ(lara.torso_y_rot, lara.torso_x_rot, lara.torso_z_rot);

		rotation = lara.right_arm.frame_base + (lara.right_arm.frame_number - anims[lara.right_arm.anim_number].frame_base) * (anims[lara.right_arm.anim_number].interpolation >> 8) + 9;

		gar_RotYXZsuperpack(&rotation, 8);
		PutPolyLara(UARM_R, clip);

		phd_TranslateRel(*(bone + 1 + 32), *(bone + 2 + 32), *(bone + 3 + 32));
		gar_RotYXZsuperpack(&rotation, 0);
		PutPolyLara(LARM_R, clip);

		phd_TranslateRel(*(bone + 1 + 36), *(bone + 2 + 36), *(bone + 3 + 36));
		gar_RotYXZsuperpack(&rotation, 0);
		PutPolyLara(HAND_R, clip);

		if (lara.right_arm.flash_gun)
			memcpy(savemox, phd_mxptr, 12 * 4);

		phd_PopMatrix();
		phd_PushMatrix();
		phd_TranslateRel(*(bone + 1 + 40), *(bone + 2 + 40), *(bone + 3 + 40));

		*(phd_mxptr + M00) = *(phd_mxptr + M00 - 24);
		*(phd_mxptr + M01) = *(phd_mxptr + M01 - 24);
		*(phd_mxptr + M02) = *(phd_mxptr + M02 - 24);
		*(phd_mxptr + M10) = *(phd_mxptr + M10 - 24);
		*(phd_mxptr + M11) = *(phd_mxptr + M11 - 24);
		*(phd_mxptr + M12) = *(phd_mxptr + M12 - 24);
		*(phd_mxptr + M20) = *(phd_mxptr + M20 - 24);
		*(phd_mxptr + M21) = *(phd_mxptr + M21 - 24);
		*(phd_mxptr + M22) = *(phd_mxptr + M22 - 24);

		phd_RotYXZ(lara.torso_y_rot, lara.torso_x_rot, lara.torso_z_rot);

		rotation = lara.left_arm.frame_base + (lara.left_arm.frame_number - anims[lara.left_arm.anim_number].frame_base) * (anims[lara.left_arm.anim_number].interpolation >> 8) + 9;

		gar_RotYXZsuperpack(&rotation, 11);
		PutPolyLara(UARM_L, clip);

		phd_TranslateRel(*(bone + 1 + 44), *(bone + 2 + 44), *(bone + 3 + 44));
		gar_RotYXZsuperpack(&rotation, 0);
		PutPolyLara(LARM_L, clip);

		phd_TranslateRel(*(bone + 1 + 48), *(bone + 2 + 48), *(bone + 3 + 48));
		gar_RotYXZsuperpack(&rotation, 0);
		PutPolyLara(HAND_L, clip);

		if (lara.left_arm.flash_gun)
			DrawGunFlash(fire_arms, clip);

		if (lara.right_arm.flash_gun)
		{
			memcpy(phd_mxptr, savemox, 12 * 4);
			DrawGunFlash(fire_arms, clip);
		}

		break;
	}
	case LG_SHOTGUN:
	case LG_HARPOON:
	case LG_ROCKET:
	case LG_GRENADE:
	case LG_M16:
	{
		phd_PushMatrix();
		phd_TranslateRel(*(bone + 1 + 28), *(bone + 2 + 28), *(bone + 3 + 28));

		rotation = lara.right_arm.frame_base + lara.right_arm.frame_number * (anims[lara.right_arm.anim_number].interpolation >> 8) + 9;
		gar_RotYXZsuperpack(&rotation, 8);
		PutPolyLara(UARM_R, clip);

		phd_TranslateRel(*(bone + 1 + 32), *(bone + 2 + 32), *(bone + 3 + 32));
		gar_RotYXZsuperpack(&rotation, 0);
		PutPolyLara(LARM_R, clip);

		phd_TranslateRel(*(bone + 1 + 36), *(bone + 2 + 36), *(bone + 3 + 36));
		gar_RotYXZsuperpack(&rotation, 0);
		PutPolyLara(HAND_R, clip);

		if (lara.right_arm.flash_gun)
			memcpy(savemox, phd_mxptr, 12 * 4);

		phd_PopMatrix();
		phd_PushMatrix();
		phd_TranslateRel(*(bone + 1 + 40), *(bone + 2 + 40), *(bone + 3 + 40));
		gar_RotYXZsuperpack(&rotation, 0);
		PutPolyLara(UARM_L, clip);

		phd_TranslateRel(*(bone + 1 + 44), *(bone + 2 + 44), *(bone + 3 + 44));
		gar_RotYXZsuperpack(&rotation, 0);
		PutPolyLara(LARM_L, clip);

		phd_TranslateRel(*(bone + 1 + 48), *(bone + 2 + 48), *(bone + 3 + 48));
		gar_RotYXZsuperpack(&rotation, 0);

		PutPolyLara(HAND_L, clip);

		if (lara.right_arm.flash_gun)
		{
			memcpy(phd_mxptr, savemox, 12 * 4);

			DrawGunFlash(fire_arms, clip);
		}
	}
	}

	phd_PopMatrix();
	phd_PopMatrix();
	phd_PopMatrix();

	phd_left = left;
	phd_right = right;
	phd_top = top;
	phd_bottom = bottom;

	if (do_flare)
	{
		PHD_VECTOR pos { 8, 36, 32 };

		get_lara_bone_pos(item, &pos, HAND_L);

		int xv = pos.x,
			yv = pos.y,
			zv = pos.z;

		pos = { 8, 36, 1024 + (GetRandomDraw() & 255) };

		get_lara_bone_pos(item, &pos, HAND_L);

		for (int i = 0; i < (GetRandomDraw() & 3) + 4; ++i)
			TriggerFlareSparks(xv, yv, zv, pos.x - xv, pos.y - yv, pos.z - zv, i >> 2);
	}
}

void draw_lara_interpolation(ITEM_INFO* item, vec3d* hair_data, int16_t* frame1, int16_t* frame2, int frac, int rate, short weapon_current_anim_state, bool remote)
{
	auto object = &objects[item->object_number];
	auto bounds = GetBoundsAccurate(item);

	if (lara.skidoo == NO_ITEM)
		S_PrintShadow(object->shadow_size, bounds, item, 0);

	int32_t savemox1[12];

	memcpy(savemox1, phd_mxptr, 12 * 4);

	phd_PushMatrix();
	phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
	phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);

	int clip = S_GetObjectBounds(frame1);
	if (!clip)
		clip = -1;

	phd_PushMatrix();

	CalculateObjectLightingLara();

	auto bone = object->bone_ptr;
	auto rotation1 = frame1 + 9;
	auto rotation2 = frame2 + 9;

	InitInterpolate(frac, rate);

	phd_TranslateRel_ID(
		(int32_t)*(frame1 + 6),
		(int32_t)*(frame1 + 7),
		(int32_t)*(frame1 + 8),
		(int32_t)*(frame2 + 6),
		(int32_t)*(frame2 + 7),
		(int32_t)*(frame2 + 8));

	gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
	PutPolyLara_I(HIPS, clip);

	phd_PushMatrix_I();
	phd_TranslateRel_I(*(bone + 1), *(bone + 2), *(bone + 3));
	gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
	PutPolyLara_I(THIGH_L, clip);

	phd_TranslateRel_I(*(bone + 1 + 4), *(bone + 2 + 4), *(bone + 3 + 4));
	gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
	PutPolyLara_I(CALF_L, clip);

	phd_TranslateRel_I(*(bone + 1 + 8), *(bone + 2 + 8), *(bone + 3 + 8));
	gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
	PutPolyLara_I(FOOT_L, clip);

	phd_PopMatrix_I();

	phd_PushMatrix_I();
	phd_TranslateRel_I(*(bone + 1 + 12), *(bone + 2 + 12), *(bone + 3 + 12));
	gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
	PutPolyLara_I(THIGH_R, clip);

	phd_TranslateRel_I(*(bone + 1 + 16), *(bone + 2 + 16), *(bone + 3 + 16));
	gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
	PutPolyLara_I(CALF_R, clip);

	phd_TranslateRel_I(*(bone + 1 + 20), *(bone + 2 + 20), *(bone + 3 + 20));
	gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
	PutPolyLara_I(FOOT_R, clip);

	phd_PopMatrix_I();

	phd_TranslateRel_I(*(bone + 1 + 24), *(bone + 2 + 24), *(bone + 3 + 24));

	if (lara.gun_type == LG_M16 &&
		(weapon_current_anim_state == 0 ||
		 weapon_current_anim_state == 2 ||
		 weapon_current_anim_state == 4))
	{
		rotation1 = rotation2 = lara.right_arm.frame_base + lara.right_arm.frame_number * (anims[lara.right_arm.anim_number].interpolation >> 8) + 9;
		gar_RotYXZsuperpack_I(&rotation1, &rotation2, 7);
	}
	else gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);

	phd_RotYXZ_I(lara.torso_y_rot, lara.torso_x_rot, lara.torso_z_rot);
	PutPolyLara_I(TORSO, clip);

	phd_PushMatrix_I();
	phd_TranslateRel_I(*(bone + 1 + 52), *(bone + 2 + 52), *(bone + 3 + 52));

	auto rotationw1 = rotation1,
		 rotationw2 = rotation2;

	gar_RotYXZsuperpack_I(&rotationw1, &rotationw2, 6);
	phd_RotYXZ_I(lara.head_y_rot, lara.head_x_rot, lara.head_z_rot);
	PutPolyLara_I(HEAD, clip);

	memcpy(phd_mxptr, savemox1, 12 * 4);

	DrawHair(hair_data);

	phd_PopMatrix_I();

	if (lara.back_gun)
	{
		phd_PushMatrix_I();

		auto bonew = objects[lara.back_gun].bone_ptr;

		phd_TranslateRel_I(*(bonew + 1 + 52), *(bonew + 2 + 52), *(bonew + 3 + 52));

		rotationw1 = rotationw2 = objects[lara.back_gun].frame_base + 9;

		gar_RotYXZsuperpack_I(&rotationw1, &rotationw2, 14);
		phd_PutPolygons_I(*(objects[lara.back_gun].mesh_ptr + HEAD), clip);
		phd_PopMatrix_I();
	}

	int16_t fire_arms = LG_UNARMED;

	if (lara.gun_status == LG_READY || lara.gun_status == LG_SPECIAL || lara.gun_status == LG_DRAW || lara.gun_status == LG_UNDRAW)
		fire_arms = lara.gun_type;

	bool do_flare = false;

	switch (fire_arms)
	{
	case LG_UNARMED:
	case LG_FLARE:
	{
		phd_PushMatrix_I();

		phd_TranslateRel_I(*(bone + 1 + 28), *(bone + 2 + 28), *(bone + 3 + 28));
		gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
		PutPolyLara_I(UARM_R, clip);

		phd_TranslateRel_I(*(bone + 1 + 32), *(bone + 2 + 32), *(bone + 3 + 32));
		gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
		PutPolyLara_I(LARM_R, clip);

		phd_TranslateRel_I(*(bone + 1 + 36), *(bone + 2 + 36), *(bone + 3 + 36));
		gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
		PutPolyLara_I(HAND_R, clip);

		phd_PopMatrix_I();

		phd_PushMatrix_I();

		phd_TranslateRel_I(*(bone + 1 + 40), *(bone + 2 + 40), *(bone + 3 + 40));

		if (lara.flare_control_left)
		{
			rotation1 = rotation2 = lara.left_arm.frame_base + (lara.left_arm.frame_number - anims[lara.left_arm.anim_number].frame_base) * (anims[lara.left_arm.anim_number].interpolation >> 8) + 9;

			gar_RotYXZsuperpack_I(&rotation1, &rotation2, 11);
		}
		else gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);

		PutPolyLara_I(UARM_L, clip);

		phd_TranslateRel_I(*(bone + 1 + 44), *(bone + 2 + 44), *(bone + 3 + 44));
		gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
		PutPolyLara_I(LARM_L, clip);

		phd_TranslateRel_I(*(bone + 1 + 48), *(bone + 2 + 48), *(bone + 3 + 48));
		gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
		PutPolyLara_I(HAND_L, clip);

		if (lara.gun_type == LG_FLARE && lara.left_arm.flash_gun)
		{
			phd_TranslateRel_I(11, 32, 80);
			phd_RotX_I(-90 * ONE_DEGREE);

			do_flare = true;
		}

		break;
	}
	case LG_PISTOLS:
	case LG_UZIS:
	{
		phd_PushMatrix_I();
		phd_TranslateRel_I(*(bone + 1 + 28), *(bone + 2 + 28), *(bone + 3 + 28));

		InterpolateArmMatrix();

		phd_RotYXZ(lara.right_arm.y_rot, lara.right_arm.x_rot, lara.right_arm.z_rot);

		rotation1 = lara.right_arm.frame_base + (lara.right_arm.frame_number - anims[lara.right_arm.anim_number].frame_base) * (anims[lara.right_arm.anim_number].interpolation >> 8) + 9;

		gar_RotYXZsuperpack(&rotation1, 8);
		PutPolyLara(UARM_R, clip);

		phd_TranslateRel(*(bone + 1 + 32), *(bone + 2 + 32), *(bone + 3 + 32));
		gar_RotYXZsuperpack(&rotation1, 0);
		PutPolyLara(LARM_R, clip);

		phd_TranslateRel(*(bone + 1 + 36), *(bone + 2 + 36), *(bone + 3 + 36));
		gar_RotYXZsuperpack(&rotation1, 0);
		PutPolyLara(HAND_R, clip);

		if (lara.right_arm.flash_gun)
			memcpy(savemox1, phd_mxptr, 12 * 4);

		phd_PopMatrix_I();
		phd_PushMatrix_I();
		phd_TranslateRel_I(*(bone + 1 + 40), *(bone + 2 + 40), *(bone + 3 + 40));

		InterpolateArmMatrix();

		phd_RotYXZ(lara.left_arm.y_rot, lara.left_arm.x_rot, lara.left_arm.z_rot);

		rotation1 = lara.left_arm.frame_base + (lara.left_arm.frame_number - anims[lara.left_arm.anim_number].frame_base) * (anims[lara.left_arm.anim_number].interpolation >> 8) + 9;

		gar_RotYXZsuperpack(&rotation1, 11);
		PutPolyLara(UARM_L, clip);

		phd_TranslateRel(*(bone + 1 + 44), *(bone + 2 + 44), *(bone + 3 + 44));
		gar_RotYXZsuperpack(&rotation1, 0);
		PutPolyLara(LARM_L, clip);

		phd_TranslateRel(*(bone + 1 + 48), *(bone + 2 + 48), *(bone + 3 + 48));
		gar_RotYXZsuperpack(&rotation1, 0);
		PutPolyLara(HAND_L, clip);

		if (lara.left_arm.flash_gun)
			DrawGunFlash(fire_arms, clip);

		if (lara.right_arm.flash_gun)
		{
			memcpy(phd_mxptr, savemox1, 12 * 4);
			DrawGunFlash(fire_arms, clip);
		}

		break;
	}
	case LG_MAGNUMS:
	{
		phd_PushMatrix_I();
		phd_TranslateRel_I(*(bone + 1 + 28), *(bone + 2 + 28), *(bone + 3 + 28));

		InterpolateArmMatrix();

		phd_RotYXZ_I(lara.torso_y_rot, lara.torso_x_rot, lara.torso_z_rot);

		rotation1 = lara.right_arm.frame_base + (lara.right_arm.frame_number - anims[lara.right_arm.anim_number].frame_base) * (anims[lara.right_arm.anim_number].interpolation >> 8) + 9;

		gar_RotYXZsuperpack(&rotation1, 8);
		PutPolyLara(UARM_R, clip);

		phd_TranslateRel(*(bone + 1 + 32), *(bone + 2 + 32), *(bone + 3 + 32));
		gar_RotYXZsuperpack(&rotation1, 0);
		PutPolyLara(LARM_R, clip);

		phd_TranslateRel(*(bone + 1 + 36), *(bone + 2 + 36), *(bone + 3 + 36));
		gar_RotYXZsuperpack(&rotation1, 0);
		PutPolyLara(HAND_R, clip);

		if (lara.right_arm.flash_gun)
			memcpy(savemox1, phd_mxptr, 12 * 4);

		phd_PopMatrix_I();
		phd_PushMatrix_I();

		phd_TranslateRel_I(*(bone + 1 + 40), *(bone + 2 + 40), *(bone + 3 + 40));
		InterpolateArmMatrix();

		phd_RotYXZ_I(lara.torso_y_rot, lara.torso_x_rot, lara.torso_z_rot);

		rotation1 = lara.left_arm.frame_base + (lara.left_arm.frame_number - anims[lara.left_arm.anim_number].frame_base) * (anims[lara.left_arm.anim_number].interpolation >> 8) + 9;

		gar_RotYXZsuperpack(&rotation1, 11);
		PutPolyLara(UARM_L, clip);

		phd_TranslateRel(*(bone + 1 + 44), *(bone + 2 + 44), *(bone + 3 + 44));
		gar_RotYXZsuperpack(&rotation1, 0);
		PutPolyLara(LARM_L, clip);

		phd_TranslateRel(*(bone + 1 + 48), *(bone + 2 + 48), *(bone + 3 + 48));
		gar_RotYXZsuperpack(&rotation1, 0);
		PutPolyLara(HAND_L, clip);

		if (lara.left_arm.flash_gun)
			DrawGunFlash(fire_arms, clip);

		if (lara.right_arm.flash_gun)
		{
			memcpy(phd_mxptr, savemox1, 12 * 4);
			DrawGunFlash(fire_arms, clip);
		}

		break;
	}
	case LG_SHOTGUN:
	case LG_HARPOON:
	case LG_ROCKET:
	case LG_GRENADE:
	case LG_M16:
		phd_PushMatrix_I();
		phd_TranslateRel_I(*(bone + 1 + 28), *(bone + 2 + 28), *(bone + 3 + 28));
		rotation1 = rotation2 = lara.right_arm.frame_base + lara.right_arm.frame_number * (anims[lara.right_arm.anim_number].interpolation >> 8) + 9;
		gar_RotYXZsuperpack_I(&rotation1, &rotation2, 8);
		PutPolyLara_I(UARM_R, clip);

		phd_TranslateRel_I(*(bone + 1 + 32), *(bone + 2 + 32), *(bone + 3 + 32));
		gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
		PutPolyLara_I(LARM_R, clip);

		phd_TranslateRel_I(*(bone + 1 + 36), *(bone + 2 + 36), *(bone + 3 + 36));
		gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
		PutPolyLara_I(HAND_R, clip);

		if (lara.right_arm.flash_gun)
			memcpy(savemox1, phd_mxptr, 12 * 4);

		phd_PopMatrix_I();

		phd_PushMatrix_I();
		phd_TranslateRel_I(*(bone + 1 + 40), *(bone + 2 + 40), *(bone + 3 + 40));
		gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
		PutPolyLara_I(UARM_L, clip);

		phd_TranslateRel_I(*(bone + 1 + 44), *(bone + 2 + 44), *(bone + 3 + 44));
		gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
		PutPolyLara_I(LARM_L, clip);

		phd_TranslateRel_I(*(bone + 1 + 48), *(bone + 2 + 48), *(bone + 3 + 48));
		gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
		PutPolyLara_I(HAND_L, clip);

		if (lara.right_arm.flash_gun)
		{
			memcpy(phd_mxptr, savemox1, 12 * 4);
			DrawGunFlash(fire_arms, clip);
		}

		break;
	}

	phd_PopMatrix();
	phd_PopMatrix();
	phd_PopMatrix();

	if (do_flare)
	{
		PHD_VECTOR pos { 8, 36, 32 };

		get_lara_bone_pos(item, &pos, HAND_L);

		int xv = pos.x,
			yv = pos.y,
			zv = pos.z;

		pos = { 8, 36, 1024 + (GetRandomDraw() & 255) };

		get_lara_bone_pos(item, &pos, HAND_L);

		for (int i = 0; i < (GetRandomDraw() & 3) + 4; ++i)
			TriggerFlareSparks(xv, yv, zv, pos.x - xv, pos.y - yv, pos.z - zv, i >> 2);
	}
}

void InitInterpolate(int frac, int rate)
{
	IM_frac = frac;
	IM_rate = rate;
	IMptr = IMstack;

	memcpy(IMstack, phd_mxptr, 12 * 4);
}

void phd_PopMatrix_I()
{
	phd_PopMatrix();
	IMptr -= 12;
}

void phd_PushMatrix_I()
{
	phd_PushMatrix();
	memcpy(IMptr + 12, IMptr, 12 * 4);
	IMptr += 12;
}

void phd_RotY_I(int16_t ang)
{
	phd_RotY(ang);

	auto oldm = phd_mxptr;

	phd_mxptr = IMptr;
	phd_RotY(ang);
	phd_mxptr = oldm;
}

void phd_RotX_I(int16_t ang)
{
	phd_RotX(ang);

	auto oldm = phd_mxptr;

	phd_mxptr = IMptr;
	phd_RotX(ang);
	phd_mxptr = oldm;
}

void phd_RotZ_I(int16_t ang)
{
	phd_RotZ(ang);

	auto oldm = phd_mxptr;

	phd_mxptr = IMptr;
	phd_RotZ(ang);
	phd_mxptr = oldm;
}

void phd_TranslateRel_I(int32_t x, int32_t y, int32_t z)
{
	phd_TranslateRel(x, y, z);

	auto oldm = phd_mxptr;

	phd_mxptr = IMptr;
	phd_TranslateRel(x, y, z);
	phd_mxptr = oldm;
}

void phd_TranslateRel_ID(int32_t x, int32_t y, int32_t z, int32_t x2, int32_t y2, int32_t z2)
{
	phd_TranslateRel(x, y, z);

	auto oldm = phd_mxptr;

	phd_mxptr = IMptr;
	phd_TranslateRel(x2, y2, z2);
	phd_mxptr = oldm;
}

void phd_RotYXZ_I(int16_t y, int16_t x, int16_t z)
{
	phd_RotYXZ(y, x, z);

	auto oldm = phd_mxptr;

	phd_mxptr = IMptr;
	phd_RotYXZ(y, x, z);
	phd_mxptr = oldm;
}

void gar_RotYXZsuperpack_I(int16_t** pprot1, int16_t** pprot2, int skip)
{
	gar_RotYXZsuperpack(pprot1, skip);

	auto oldm = phd_mxptr;

	phd_mxptr = IMptr;
	gar_RotYXZsuperpack(pprot2, skip);
	phd_mxptr = oldm;
}

void gar_RotYXZsuperpack(int16_t** pprot, int skip)
{
	while (skip)
	{
		auto prot = (uint16_t*)*pprot;

 		(*pprot) += ((*prot & (3 << 14)) ? 1 : 2);

		--skip;
	}

	auto prot = (uint16_t*)*pprot;

	switch (*prot >> 14)
	{
	case 0:
		phd_RotYXZpack((*prot << 16) + *(prot + 1));
		(*pprot) += 2;
		return;
	case 1:
		phd_RotX((int16_t)((*prot & 1023) << 6));
		break;
	case 2:
		phd_RotY((int16_t)((*prot & 1023) << 6));
		break;
	default:
		phd_RotZ((int16_t)((*prot & 1023) << 6));
		break;
	}

	(*pprot) += 1;
}

void phd_PutPolygons_I(int16_t* ptr, int clip)
{
	phd_PushMatrix();
	InterpolateMatrix();
	phd_PutPolygons(ptr, clip);
	phd_PopMatrix();
}

void InterpolateMatrix()
{
	auto mptr = phd_mxptr,
		 iptr = IMptr;

	int frac = IM_frac,
		rate = IM_rate;

	if (rate == 2 || (frac == 2 && rate == 4))
	{
		*(mptr + M00) = (*(iptr + M00) + *(mptr + M00)) >> 1;
		*(mptr + M01) = (*(iptr + M01) + *(mptr + M01)) >> 1;
		*(mptr + M02) = (*(iptr + M02) + *(mptr + M02)) >> 1;
		*(mptr + M03) = (*(iptr + M03) + *(mptr + M03)) >> 1;

		*(mptr + M10) = (*(iptr + M10) + *(mptr + M10)) >> 1;
		*(mptr + M11) = (*(iptr + M11) + *(mptr + M11)) >> 1;
		*(mptr + M12) = (*(iptr + M12) + *(mptr + M12)) >> 1;
		*(mptr + M13) = (*(iptr + M13) + *(mptr + M13)) >> 1;

		*(mptr + M20) = (*(iptr + M20) + *(mptr + M20)) >> 1;
		*(mptr + M21) = (*(iptr + M21) + *(mptr + M21)) >> 1;
		*(mptr + M22) = (*(iptr + M22) + *(mptr + M22)) >> 1;
		*(mptr + M23) = (*(iptr + M23) + *(mptr + M23)) >> 1;
	}
	else if (frac == 1)
	{
		*(mptr + M00) += (*(iptr + M00) - *(mptr + M00)) >> 2;
		*(mptr + M01) += (*(iptr + M01) - *(mptr + M01)) >> 2;
		*(mptr + M02) += (*(iptr + M02) - *(mptr + M02)) >> 2;
		*(mptr + M03) += (*(iptr + M03) - *(mptr + M03)) >> 2;

		*(mptr + M10) += (*(iptr + M10) - *(mptr + M10)) >> 2;
		*(mptr + M11) += (*(iptr + M11) - *(mptr + M11)) >> 2;
		*(mptr + M12) += (*(iptr + M12) - *(mptr + M12)) >> 2;
		*(mptr + M13) += (*(iptr + M13) - *(mptr + M13)) >> 2;

		*(mptr + M20) += (*(iptr + M20) - *(mptr + M20)) >> 2;
		*(mptr + M21) += (*(iptr + M21) - *(mptr + M21)) >> 2;
		*(mptr + M22) += (*(iptr + M22) - *(mptr + M22)) >> 2;
		*(mptr + M23) += (*(iptr + M23) - *(mptr + M23)) >> 2;
	}
	else
	{
		*(mptr + M00) = *(iptr + M00) - ((*(iptr + M00) - *(mptr + M00)) >> 2);
		*(mptr + M01) = *(iptr + M01) - ((*(iptr + M01) - *(mptr + M01)) >> 2);
		*(mptr + M02) = *(iptr + M02) - ((*(iptr + M02) - *(mptr + M02)) >> 2);
		*(mptr + M03) = *(iptr + M03) - ((*(iptr + M03) - *(mptr + M03)) >> 2);

		*(mptr + M10) = *(iptr + M10) - ((*(iptr + M10) - *(mptr + M10)) >> 2);
		*(mptr + M11) = *(iptr + M11) - ((*(iptr + M11) - *(mptr + M11)) >> 2);
		*(mptr + M12) = *(iptr + M12) - ((*(iptr + M12) - *(mptr + M12)) >> 2);
		*(mptr + M13) = *(iptr + M13) - ((*(iptr + M13) - *(mptr + M13)) >> 2);

		*(mptr + M20) = *(iptr + M20) - ((*(iptr + M20) - *(mptr + M20)) >> 2);
		*(mptr + M21) = *(iptr + M21) - ((*(iptr + M21) - *(mptr + M21)) >> 2);
		*(mptr + M22) = *(iptr + M22) - ((*(iptr + M22) - *(mptr + M22)) >> 2);
		*(mptr + M23) = *(iptr + M23) - ((*(iptr + M23) - *(mptr + M23)) >> 2);
	}
}

void InterpolateArmMatrix()
{
	int32_t* mptr, * iptr;
	int32_t frac, rate;

	mptr = phd_mxptr;
	iptr = IMptr;
	frac = IM_frac;
	rate = IM_rate;

	*(mptr + M00) = *(mptr + M00 - 24);
	*(mptr + M01) = *(mptr + M01 - 24);
	*(mptr + M02) = *(mptr + M02 - 24);
	*(mptr + M10) = *(mptr + M10 - 24);
	*(mptr + M11) = *(mptr + M11 - 24);
	*(mptr + M12) = *(mptr + M12 - 24);
	*(mptr + M20) = *(mptr + M20 - 24);
	*(mptr + M21) = *(mptr + M21 - 24);
	*(mptr + M22) = *(mptr + M22 - 24);

	if (rate == 2 || (frac == 2 && rate == 4))
	{
		*(mptr + M03) = (*(iptr + M03) + *(mptr + M03)) >> 1;
		*(mptr + M13) = (*(iptr + M13) + *(mptr + M13)) >> 1;
		*(mptr + M23) = (*(iptr + M23) + *(mptr + M23)) >> 1;
	}
	else if (frac == 1)
	{
		*(mptr + M03) += (*(iptr + M03) - *(mptr + M03)) >> 2;
		*(mptr + M13) += (*(iptr + M13) - *(mptr + M13)) >> 2;
		*(mptr + M23) += (*(iptr + M23) - *(mptr + M23)) >> 2;
	}
	else
	{
		*(mptr + M03) = *(iptr + M03) - ((*(iptr + M03) - *(mptr + M03)) >> 2);
		*(mptr + M13) = *(iptr + M13) - ((*(iptr + M13) - *(mptr + M13)) >> 2);
		*(mptr + M23) = *(iptr + M23) - ((*(iptr + M23) - *(mptr + M23)) >> 2);
	}
}

void DrawGunFlash(int weapon_type, int clip)
{
	int g_len, nZOff;

	int16_t light;

	switch (weapon_type)
	{
	case LG_FLARE:
		return;
	case LG_MAGNUMS:
		light = 24 | (18 << 5);
		g_len = 215;
		nZOff = 65;
		break;
	case LG_UZIS:
		light = 24 | (18 << 5);
		g_len = 150;
		nZOff = 50;
		break;
	case LG_M16:
	{
		if (g_silenced_hk)
			return;
		
		phd_TranslateRel(0, 332, 96);
		phd_RotYXZ(0, -85 * ONE_DEGREE, (int16_t)((GetRandomDraw() * 2 & 0x4000) + 0x2000 + (GetRandomDraw() & 4095) - 2048));

		S_CalculateStaticLight(24 | (18 << 5));

		phd_PutPolygons(*objects[M16_FLASH].mesh_ptr, clip);

		return S_DrawSprite(SPRITE_REL | SPRITE_SEMITRANS | SPRITE_TRANS_ADD | SPRITE_TINT | SPRITE_COLOUR(0x3f, 0x30, 0x08) | SPRITE_SCALE, 0, 0, -65, (int16_t*)objects[GLOW].mesh_ptr, 0, 0xc0);
	}
	case LG_SHOTGUN:

		return;
	default:
		light = 24 | (18 << 5);
		g_len = 150;
		nZOff = 40;
		break;
	}

	phd_TranslateRel(0, g_len, nZOff);
	phd_RotYXZ(0, -90 * ONE_DEGREE, (int16_t)(wibble << 8));

	S_CalculateStaticLight(light);

	phd_PutPolygons(*objects[GUN_FLASH].mesh_ptr, clip);

	S_DrawSprite(SPRITE_REL | SPRITE_SEMITRANS | SPRITE_TRANS_ADD | SPRITE_TINT | SPRITE_COLOUR(0x3f, 0x38, 0x8) | SPRITE_SCALE, 0, 0, 0, (int16_t*)objects[GLOW].mesh_ptr, 0, 0xc0);
}

void CalculateObjectLighting(ITEM_INFO* item, int16_t* frame)
{
	if (item->shade < 0)
	{
		if (item->object_number == TRAIN)
			S_CalculateLight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number, &item->il);
		else
		{
			phd_PushUnitMatrix();

			auto mptr = phd_mxptr;

			*(mptr + M03) = *(mptr + M13) = *(mptr + M23) = 0;

			phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);

			int x = (*(phd_mxptr + M03) >> W2V_SHIFT) + item->pos.x_pos,
				y = (*(phd_mxptr + M13) >> W2V_SHIFT) + item->pos.y_pos,
				z = (*(phd_mxptr + M23) >> W2V_SHIFT) + item->pos.z_pos;

			phd_PopMatrix();

			S_CalculateLight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number, &item->il);
		}
	}
	else S_CalculateStaticMeshLight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->shade & 0x7fff, item->shadeB, &room[item->room_number]);
}

void CalculateObjectLightingLara()
{
	PHD_VECTOR pos {};

	get_lara_bone_pos(lara_item, &pos, FOOT_L);

	auto room_no = lara_item->room_number;

	GetFloor(pos.x, pos.y, pos.z, &room_no);

	S_CalculateLight(pos.x, pos.y, pos.z, room_no, &lara_item->il);
}

int GetFrames(ITEM_INFO* item, int16_t* frmptr[], int* rate)
{
	if (!is_valid_anim(item->anim_number))
		return 0;
	
	auto anim = &anims[item->anim_number];

	frmptr[0] = frmptr[1] = anim->frame_ptr;

	int rat = *rate = anim->interpolation & 0x00ff,
		frame_size = anim->interpolation >> 8,
		frm = item->frame_number - anim->frame_base;

	int first = frm / rat,
		interp = frm % rat;

	frmptr[0] += first * frame_size;
	frmptr[1] = frmptr[0] + frame_size;

	if (interp == 0)
		return 0;

	if (int second = first * rat + rat; second > anim->frame_end)
		*rate = anim->frame_end - (second - rat);

	return interp;
}

int16_t* GetBoundsAccurate(ITEM_INFO* item)
{
	int16_t* frmptr[2];

	int rate;

	if (int frac = GetFrames(item, frmptr, &rate); frac == 0)
		return frmptr[0];
	else
	{
		auto bptr = interpolated_bounds;

		for (int i = 0; i < 6; ++i, ++bptr, ++frmptr[0], ++frmptr[1])
			*bptr = *(frmptr[0]) + ((*(frmptr[1]) - *(frmptr[0])) * frac) / rate;

		return interpolated_bounds;
	}
}

int16_t* GetBestFrame(ITEM_INFO* item)
{
	int16_t* frmptr[2];

	int rate,
		frac = GetFrames(item, frmptr, &rate);

	return (frac <= (rate >> 1) ? frmptr[0] : frmptr[1]);
}

#if defined(GAMEDEBUG)
SG_COL RedBox[] =
{
	S_GOURAUD_COL(255, 0, 0, 128),
	S_GOURAUD_COL(255, 0, 0, 128),
	S_GOURAUD_COL(255, 0, 0, 128),
	S_GOURAUD_COL(255, 0, 0, 128)
};

SG_COL GreenBox[] =
{
	S_GOURAUD_COL(0, 255, 0, 128),
	S_GOURAUD_COL(0, 255, 0, 128),
	S_GOURAUD_COL(0, 255, 0, 128),
	S_GOURAUD_COL(0, 255, 0, 128)
};

SG_COL BlueBox[] =
{
	S_GOURAUD_COL(0, 0, 255, 128),
	S_GOURAUD_COL(0, 0, 255, 128),
	S_GOURAUD_COL(0, 0, 255, 128),
	S_GOURAUD_COL(0, 0, 255, 128)
};

SG_COL YellowBox[] =
{
	S_GOURAUD_COL(255, 255, 0, 128),
	S_GOURAUD_COL(255, 255, 0, 128),
	S_GOURAUD_COL(255, 255, 0, 128),
	S_GOURAUD_COL(255, 255, 0, 128)
};

void DrawRoomBox(int room_num)
{
	auto r = &room[room_num];

	phd_PushMatrix();
	phd_TranslateAbs(r->x, r->y, r->z);

	phd_left = r->left;
	phd_right = r->right;
	phd_top = r->top;
	phd_bottom = r->bottom;

	int16_t bounds[6]
	{
		WALL_L,
		(r->y_size - 1) * WALL_L,
		(int16_t)(r->maxceiling - r->y),
		(int16_t)(r->minfloor - r->y),
		WALL_L,
		(r->x_size - 1) * WALL_L
	};

	DrawBox(bounds, room_num == lara_item->room_number ? C_WHITE : C_BLUE);

	S_DrawScreenBox(phd_left, phd_top, 0, phd_right - phd_left, phd_bottom - phd_top, C_RED, RedBox, 0);

	phd_PopMatrix();
}

void DrawBox(int16_t* bptr, int colour)
{
	int32_t xv[8], yv[8], zv[8], xs[8], ys[8];

	phd_left = phd_top = 0;
	phd_right = phd_winxmax;
	phd_bottom = phd_winymax;

	xv[0] = xv[3] = xv[4] = xv[7] = *bptr;
	yv[0] = yv[1] = yv[2] = yv[3] = *(bptr + 2);
	zv[0] = zv[1] = zv[4] = zv[5] = *(bptr + 4);

	xv[1] = xv[2] = xv[5] = xv[6] = *(bptr + 1);
	yv[4] = yv[5] = yv[6] = yv[7] = *(bptr + 3);
	zv[2] = zv[3] = zv[6] = zv[7] = *(bptr + 5);

	auto mptr = phd_mxptr;

	for (int i = 0; i < 8; ++i)
	{
		int x = xv[i],
			y = yv[i],
			z = zv[i];

		xv[i] = (*(mptr + M00) * x) + (*(mptr + M01) * y) + (*(mptr + M02) * z) + *(mptr + M03);
		yv[i] = (*(mptr + M10) * x) + (*(mptr + M11) * y) + (*(mptr + M12) * z) + *(mptr + M13);
		zv[i] = (*(mptr + M20) * x) + (*(mptr + M21) * y) + (*(mptr + M22) * z) + *(mptr + M23);

		z = zv[i] / phd_persp;

		xs[i] = xv[i] / z + phd_centerx;
		ys[i] = yv[i] / z + phd_centery;
	}

	for (int i = 0; i < 12; ++i)
	{
		int p1 = box_lines[i][0],
			p2 = box_lines[i][1];

		if (zv[p1] > phd_znear && zv[p2] > phd_znear && zv[p1] < phd_zfar && zv[p2] < phd_zfar)
			S_DrawLine(xs[p1], ys[p1], 0, xs[p2], ys[p2], colour, YellowBox, 0);
	}
}
#endif