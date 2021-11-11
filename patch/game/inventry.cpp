import prof;

#include <specific/standard.h>
#include <specific/global.h>
#include <specific/input.h>
#include <specific/output.h>
#include <specific/picture.h>
#include <specific/frontend.h>

#include "objects.h"
#include "lara.h"
#include "camera.h"
#include "control.h"
#include "inventry.h"
#include "invdata.h"
#include "invfunc.h"
#include "laramisc.h"
#include "health.h"
#include "draw.h"

#define MENU_BACKGROUND 0
#define LIT_MESHES		0x7e

int16_t Inventory_Chosen = -1;
int32_t Inventory_ExtraData[8] = { 0 };
int32_t Inventory_Mode = INV_GAME_MODE;

void DoInventoryBackground();
void Construct_Inventory();
void SelectMeshes(INVENTORY_ITEM* inv_item);
int32_t AnimateInventoryItem(INVENTORY_ITEM* inv_item);
void DrawInventoryItem(INVENTORY_ITEM* inv_item);

IMOTION_INFO imo = { 0 };
RING_INFO ring = { 0 };

INVENTORY_ITEM* inv_item = nullptr;

int busy = 0,
	inv_nframes = 2;

bool init_inventory(int inventory_mode)
{
	if (Inventory_Displaying)
		return true;

	inv_item = nullptr;
	imo = { 0 };
	ring = { 0 };
	busy = 0;

	if (inventory_mode == INV_KEYS_MODE && !inv_keys_objects)
	{
		Inventory_Chosen = -1;
		return false;
	}

	T_RemovePrint(ammo_text);
	ammo_text = nullptr;

	Inventory_Mode = inventory_mode;

	Construct_Inventory();

	switch (inventory_mode)
	{
	case INV_KEYS_MODE:
		Inv_RingInit(&ring, KEYS_RING, inv_keys_list, inv_keys_objects, inv_main_current, &imo);
		break;
	default:
		if (inv_main_objects) Inv_RingInit(&ring, MAIN_RING, inv_main_list, inv_main_objects, inv_main_current, &imo);
		else				  Inv_RingInit(&ring, OPTION_RING, inv_option_list, inv_option_objects, inv_option_current, &imo);
	}

	g_audio->play_sound(111);

	return true;
}

int32_t display_inventory(int32_t inventory_mode)
{
	if (!Inventory_Displaying)
		return 0;

	Inv_RingCalcAdders(&ring, ROTATE_DURATION);

	for (int frm = 0; frm < inv_nframes; ++frm)
	{
		if (idelay && --idcount == 0)
			idelay = 0;

		Inv_RingDoMotions(&ring);
	}

	ring.camera.z_pos = ring.radius + CAMERA_2_RING;

	PHD_3DPOS viewer;

	Inv_RingGetView(&ring, &viewer);
	phd_GenerateW2V(viewer);
	Inv_RingLight(&ring, 0);

	phd_PushMatrix();
	phd_TranslateAbs(ring.ringpos.x_pos, ring.ringpos.y_pos, ring.ringpos.z_pos);
	phd_RotYXZ(ring.ringpos.y_rot, ring.ringpos.x_rot, ring.ringpos.z_rot);

	for (int i = 0, angle = 0; i < ring.number_of_objects; ++i)
	{
		inv_item = *(ring.list + i);

		if (i == ring.current_object)
		{
			for (int frm = 0; frm < inv_nframes; ++frm)
			{
				if (ring.rotating)
				{
					if (inv_item->y_rot != 0)
						inv_item->y_rot += (inv_item->y_rot < 0 ? 0x200 : -0x200);
				}
				else if (imo.status == RNG_SELECTED ||
						 imo.status == RNG_DESELECTING ||
						 imo.status == RNG_SELECTING ||
						 imo.status == RNG_DESELECT ||
						 imo.status == RNG_CLOSING_ITEM)
				{
					if (inv_item->y_rot != inv_item->y_rot_sel)
					{
						int tempSint32 = inv_item->y_rot_sel - inv_item->y_rot;

						inv_item->y_rot += (tempSint32 > 0 && tempSint32 < 0x8000 ? 0x400 : -0x400);
						inv_item->y_rot &= 0xfc00;
					}
				}
				else if (ring.number_of_objects == 1 || !(inv_input & IN_LEFT || inv_input & IN_RIGHT))
					inv_item->y_rot += 0x100;
			}

			if ((imo.status == RNG_OPEN || imo.status == RNG_SELECTING ||
				 imo.status == RNG_SELECTED || imo.status == RNG_DESELECTING ||
				 imo.status == RNG_DESELECT || imo.status == RNG_CLOSING_ITEM) &&
				!ring.rotating && !(inv_input & IN_LEFT || inv_input & IN_RIGHT))
			{
				RingNotActive(inv_item);
			}
		}
		else
		{
			for (int frm = 0; frm < inv_nframes; ++frm)
				if (inv_item->y_rot != 0)
					inv_item->y_rot = (inv_item->y_rot < 0 ? 0x100 : -0x100);
		}

		if (imo.status == RNG_OPEN || imo.status == RNG_SELECTING ||
			imo.status == RNG_SELECTED || imo.status == RNG_DESELECTING ||
			imo.status == RNG_DESELECT || imo.status == RNG_CLOSING_ITEM)
		{
			RingIsOpen(&ring);
		}
		else RingIsNotOpen(&ring);

		if (imo.status == RNG_OPENING || imo.status == RNG_CLOSING ||
			imo.status == RNG_MAIN2OPTION || imo.status == RNG_OPTION2MAIN ||
			imo.status == RNG_EXITING_INVENTORY || imo.status == RNG_DONE ||
			ring.rotating)
		{
			RingActive();
		}

		phd_PushMatrix();
		{
			phd_RotYXZ(angle, 0, 0);
			phd_TranslateRel(ring.radius, 0, 0);
			phd_RotYXZ(0x4000, (int16_t)(ITEM_TILT + inv_item->pt_xrot), 0);

			DrawInventoryItem(inv_item);
		}
		phd_PopMatrix();

		angle += ring.angle_adder;
		angle &= 0xffff;
	}

	phd_PopMatrix();

	if (!ring.rotating)
	{
		switch (imo.status)
		{
		case RNG_OPEN:
		{
			if ((inv_input & ROTATE_RIGHT) && ring.number_of_objects > 1)
			{
				Inv_RingRotateLeft(&ring);
				g_audio->play_sound(108);
				break;
			}

			if ((inv_input & ROTATE_LEFT) && ring.number_of_objects > 1)
			{
				Inv_RingRotateRight(&ring);
				g_audio->play_sound(108);
				break;
			}

			if (inv_input & EXIT_INVENTORY)
			{
				g_audio->play_sound(112);

				Inventory_Chosen = -1;

				if (ring.type == MAIN_RING)
					inv_main_current = ring.current_object;
				else inv_option_current = ring.current_object;

				Inv_RingMotionSetup(&ring, RNG_CLOSING, RNG_DONE, CLOSE_FRAMES);
				Inv_RingMotionRadius(&ring, 0);
				Inv_RingMotionCameraPos(&ring, CAMERA_STARTHEIGHT);
				Inv_RingMotionRotation(&ring, CLOSE_ROTATION, (int16_t)(ring.ringpos.y_rot - CLOSE_ROTATION));

				inv_input = 0;
			}

			if (inv_input & SELECT_ITEM)
			{
				item_data = 0;

				if (ring.type == MAIN_RING)
				{
					inv_main_current = ring.current_object;
					inv_item = inv_main_list[ring.current_object];
				}
				else if (ring.type == OPTION_RING)
				{
					inv_option_current = ring.current_object;
					inv_item = inv_option_list[ring.current_object];
				}
				else
				{
					inv_keys_current = ring.current_object;
					inv_item = inv_keys_list[ring.current_object];
				}

				inv_item->goal_frame = inv_item->open_frame;
				inv_item->anim_direction = 1;

				Inv_RingMotionSetup(&ring, RNG_SELECTING, RNG_SELECTED, SELECTING_FRAMES);
				Inv_RingMotionRotation(&ring, 0, (int16_t)(0xc000 - (ring.current_object * ring.angle_adder)));
				Inv_RingMotionItemSelect(&ring, inv_item);

				inv_input = 0;

				switch (inv_item->object_number)
				{
				case MAP_OPTION:
					g_audio->play_sound(113);
					break;
				case SHOTGUN_OPTION:
				case MAGNUM_OPTION:
				case UZI_OPTION:
				case HARPOON_OPTION:
				case M16_OPTION:
				case ROCKET_OPTION:
				case GRENADE_OPTION:
					g_audio->play_sound(114);
					break;
				default:
					g_audio->play_sound(109);
				}
			}

			if (inv_input & IN_FORWARD && inventory_mode != INV_KEYS_MODE)
			{
				if (ring.type == MAIN_RING)
				{
					if (inv_keys_objects)
					{
						Inv_RingMotionSetup(&ring, RNG_CLOSING, RNG_MAIN2KEYS, RINGSWITCH_FRAMES / 2);
						Inv_RingMotionRadius(&ring, 0);
						Inv_RingMotionRotation(&ring, CLOSE_ROTATION, (int16_t)(ring.ringpos.y_rot - CLOSE_ROTATION));
						Inv_RingMotionCameraPitch(&ring, 0x2000);

						imo.misc = 0x2000;
					}

					inv_input = 0;
				}
				else if (ring.type == OPTION_RING)
				{
					if (inv_main_objects)
					{
						Inv_RingMotionSetup(&ring, RNG_CLOSING, RNG_OPTION2MAIN, RINGSWITCH_FRAMES / 2);
						Inv_RingMotionRadius(&ring, 0);
						Inv_RingMotionRotation(&ring, CLOSE_ROTATION, (int16_t)(ring.ringpos.y_rot - CLOSE_ROTATION));
						Inv_RingMotionCameraPitch(&ring, 0x2000);

						imo.misc = 0x2000;
					}
				}
			}
			else if (inv_input & IN_BACK && inventory_mode != INV_KEYS_MODE)
			{
				if (ring.type == KEYS_RING)
				{
					if (inv_main_objects)
					{
						Inv_RingMotionSetup(&ring, RNG_CLOSING, RNG_KEYS2MAIN, RINGSWITCH_FRAMES / 2);
						Inv_RingMotionRadius(&ring, 0);
						Inv_RingMotionRotation(&ring, CLOSE_ROTATION, (int16_t)(ring.ringpos.y_rot - CLOSE_ROTATION));
						Inv_RingMotionCameraPitch(&ring, 0 - 0x2000);

						imo.misc = 0 - 0x2000;
					}

					inv_input = 0;
				}
				else if (ring.type == MAIN_RING)
				{
					if (inv_option_objects && !gameflow.lockout_optionring)
					{
						Inv_RingMotionSetup(&ring, RNG_CLOSING, RNG_MAIN2OPTION, RINGSWITCH_FRAMES / 2);
						Inv_RingMotionRadius(&ring, 0);
						Inv_RingMotionRotation(&ring, CLOSE_ROTATION, (int16_t)(ring.ringpos.y_rot - CLOSE_ROTATION));
						Inv_RingMotionCameraPitch(&ring, 0 - 0x2000);

						imo.misc = 0 - 0x2000;
					}
				}
			}

			break;
		}
		case RNG_MAIN2OPTION:
		{
			Inv_RingMotionSetup(&ring, RNG_OPENING, RNG_OPEN, RINGSWITCH_FRAMES / 2);
			Inv_RingMotionRadius(&ring, RING_RADIUS);

			imo.camera_pitch_target = 0;
			ring.camera_pitch = -(int16_t)imo.misc;
			imo.camera_pitch_rate = imo.misc / (RINGSWITCH_FRAMES / 2);
			ring.list = inv_option_list;
			ring.type = OPTION_RING;

			inv_main_current = ring.current_object;

			ring.number_of_objects = inv_option_objects;
			ring.current_object = inv_option_current;

			Inv_RingCalcAdders(&ring, ROTATE_DURATION);
			Inv_RingMotionRotation(&ring, OPEN_ROTATION, (int16_t)(0xc000 - (ring.current_object * ring.angle_adder)));

			ring.ringpos.y_rot = imo.rotate_target + OPEN_ROTATION;

			break;
		}
		case RNG_OPTION2MAIN:
		{
			Inv_RingMotionSetup(&ring, RNG_OPENING, RNG_OPEN, RINGSWITCH_FRAMES / 2);
			Inv_RingMotionRadius(&ring, RING_RADIUS);

			imo.camera_pitch_target = 0;
			ring.camera_pitch = -(int16_t)imo.misc;
			imo.camera_pitch_rate = imo.misc / (RINGSWITCH_FRAMES / 2);
			ring.list = inv_main_list;
			ring.type = MAIN_RING;

			inv_option_objects = ring.number_of_objects;
			inv_option_current = ring.current_object;

			ring.number_of_objects = inv_main_objects;
			ring.current_object = inv_main_current;

			Inv_RingCalcAdders(&ring, ROTATE_DURATION);
			Inv_RingMotionRotation(&ring, OPEN_ROTATION, (int16_t)(0xc000 - (ring.current_object * ring.angle_adder)));

			ring.ringpos.y_rot = imo.rotate_target + OPEN_ROTATION;

			break;
		}
		case RNG_MAIN2KEYS:
		{
			Inv_RingMotionSetup(&ring, RNG_OPENING, RNG_OPEN, RINGSWITCH_FRAMES / 2);
			Inv_RingMotionRadius(&ring, RING_RADIUS);

			imo.camera_pitch_target = 0;
			ring.camera_pitch = -(int16_t)imo.misc;
			imo.camera_pitch_rate = imo.misc / (RINGSWITCH_FRAMES / 2);
			ring.list = inv_keys_list;
			ring.type = KEYS_RING;

			inv_main_objects = ring.number_of_objects;
			inv_main_current = ring.current_object;

			ring.number_of_objects = inv_keys_objects;
			ring.current_object = inv_keys_current;

			Inv_RingCalcAdders(&ring, ROTATE_DURATION);
			Inv_RingMotionRotation(&ring, OPEN_ROTATION, (int16_t)(0xc000 - (ring.current_object * ring.angle_adder)));

			ring.ringpos.y_rot = imo.rotate_target + OPEN_ROTATION;

			break;
		}
		case RNG_KEYS2MAIN:
		{
			Inv_RingMotionSetup(&ring, RNG_OPENING, RNG_OPEN, RINGSWITCH_FRAMES / 2);
			Inv_RingMotionRadius(&ring, RING_RADIUS);

			imo.camera_pitch_target = 0;
			ring.camera_pitch = -(int16_t)imo.misc;
			imo.camera_pitch_rate = imo.misc / (RINGSWITCH_FRAMES / 2);
			ring.list = inv_main_list;
			ring.type = MAIN_RING;

			inv_keys_current = ring.current_object;

			ring.number_of_objects = inv_main_objects;
			ring.current_object = inv_main_current;

			Inv_RingCalcAdders(&ring, ROTATE_DURATION);
			Inv_RingMotionRotation(&ring, OPEN_ROTATION, (int16_t)(0xc000 - (ring.current_object * ring.angle_adder)));

			ring.ringpos.y_rot = imo.rotate_target + OPEN_ROTATION;

			break;
		}
		case RNG_CLOSING_ITEM:
		{
			inv_item = *(ring.list + ring.current_object);

			for (int frm = 0; frm < inv_nframes; ++frm)
			{
				if (!AnimateInventoryItem(inv_item))
				{
					imo.count = SELECTING_FRAMES;
					imo.status = imo.status_target;

					Inv_RingMotionItemDeselect(&ring, inv_item);

					break;
				}
			}

			break;
		}
		case RNG_DESELECT:
		{
			g_audio->play_sound(112);

			Inv_RingMotionSetup(&ring, RNG_DESELECTING, RNG_OPEN, SELECTING_FRAMES);
			Inv_RingMotionRotation(&ring, 0, (int16_t)(0xc000 - (ring.current_object * ring.angle_adder)));

			inv_input = 0;

			break;
		}
		case RNG_SELECTED:
		{
			inv_item = *(ring.list + ring.current_object);

			for (int frm = 0; frm < inv_nframes; ++frm)
			{
				busy = 0;

				if (inv_item->y_rot == inv_item->y_rot_sel)
					busy = AnimateInventoryItem(inv_item);
			}

			if (!busy && !idelay)
			{
				do_inventory_options(inv_item);

				if ((inv_input & DESELECT_ITEM))
				{
					inv_item->sprlist = nullptr;

					Inv_RingMotionSetup(&ring, RNG_CLOSING_ITEM, RNG_DESELECT, 0);

					inv_input = 0;
				}

				if (inv_input & SELECT_ITEM)
				{
					inv_item->sprlist = nullptr;

					Inventory_Chosen = inv_item->object_number;

					if (ring.type == MAIN_RING)
						inv_main_current = ring.current_object;
					else inv_option_current = ring.current_object;

					Inv_RingMotionSetup(&ring, RNG_CLOSING_ITEM, RNG_EXITING_INVENTORY, 0);

					inv_input = 0;
				}
			}

			break;
		}
		case RNG_EXITING_INVENTORY:
		{
			if (!imo.count)
			{
				Inv_RingMotionSetup(&ring, RNG_CLOSING, RNG_DONE, CLOSE_FRAMES);
				Inv_RingMotionRadius(&ring, 0);
				Inv_RingMotionCameraPos(&ring, CAMERA_STARTHEIGHT);
				Inv_RingMotionRotation(&ring, CLOSE_ROTATION, (int16_t)(ring.ringpos.y_rot - CLOSE_ROTATION));
			}
		}
		}
	}

	if (imo.status == RNG_DONE)
	{
		Inventory_Displaying = 0;
		RemoveInventoryText();
	}

	if (Inventory_Chosen == -1)
		return 0;

	switch (Inventory_Chosen)
	{
	case DETAIL_OPTION:
	case SOUND_OPTION:
	case CONTROL_OPTION:
	case GAMMA_OPTION:		break;
	case GUN_OPTION:		UseItem(GUN_OPTION);		break;
	case SHOTGUN_OPTION:	UseItem(SHOTGUN_OPTION);	break;
	case MAGNUM_OPTION:		UseItem(MAGNUM_OPTION);		break;
	case UZI_OPTION:		UseItem(UZI_OPTION);		break;
	case HARPOON_OPTION:	UseItem(HARPOON_OPTION);	break;
	case M16_OPTION:		UseItem(M16_OPTION);		break;
	case ROCKET_OPTION:		UseItem(ROCKET_OPTION);		break;
	case GRENADE_OPTION:	UseItem(GRENADE_OPTION);	break;
	case FLAREBOX_OPTION:	UseItem(FLAREBOX_OPTION);	break;
	case MEDI_OPTION:		UseItem(MEDI_OPTION);		break;
	case BIGMEDI_OPTION:	UseItem(BIGMEDI_OPTION);	break;
	}

	return 0;
}

void Construct_Inventory()
{
	S_SetupAboveWater(0);

	phd_left = 0;
	phd_top = 0;
	phd_right = phd_winxmax;
	phd_bottom = phd_winymax;

	Inventory_Displaying = 1;
	Inventory_Chosen = 0;

	for (int i = 0; i < 8; ++i)
		Inventory_ExtraData[i] = 0;

	for (int i = 0; i < inv_main_objects; ++i)
	{
		auto inv_item = inv_main_list[i];

		inv_item->drawn_meshes = inv_item->which_meshes;
		inv_item->current_frame = 0;
		inv_item->goal_frame = inv_item->current_frame;
		inv_item->anim_count = 0;
		inv_item->y_rot = 0;
	}

	for (int i = 0; i < inv_option_objects; ++i)
	{
		auto inv_item = inv_option_list[i];

		inv_item->current_frame = 0;
		inv_item->goal_frame = inv_item->current_frame;
		inv_item->anim_count = 0;
		inv_item->y_rot = 0;
	}

	inv_main_current = 0;
	inv_option_current = 0;
	item_data = 0;
}

void SelectMeshes(INVENTORY_ITEM* inv_item)
{
	if (inv_item->object_number == MAP_OPTION)
	{
		int frame = inv_item->current_frame;

		inv_item->drawn_meshes = (frame && frame < 18 ? 0xffffffff : inv_item->which_meshes);
	}
	else if (inv_item->object_number != GAMMA_OPTION)
		inv_item->drawn_meshes = 0xffffffff;
}

int32_t AnimateInventoryItem(INVENTORY_ITEM* inv_item)
{
	if (inv_item->current_frame != inv_item->goal_frame)
	{
		if (inv_item->anim_count)
			--inv_item->anim_count;
		else
		{
			inv_item->anim_count = inv_item->anim_speed;
			inv_item->current_frame += inv_item->anim_direction;

			if (inv_item->current_frame >= inv_item->frames_total)
				inv_item->current_frame = 0;
			else if (inv_item->current_frame < 0)
				inv_item->current_frame = inv_item->frames_total - 1;
		}

		SelectMeshes(inv_item);

		return 1;
	}

	SelectMeshes(inv_item);

	return 0;
}

void DrawInventoryItem(INVENTORY_ITEM* inv_item)
{
	int hours,
		minutes,
		seconds;

	if (inv_item->object_number == MAP_OPTION)
	{
		auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		auto td = std::localtime(&t);

		seconds = td->tm_sec * -1092;
		minutes = (td->tm_min * -1092);
		hours = ((td->tm_hour % 12) * -2730) + (td->tm_min * -45);
	}

	phd_TranslateRel(0, inv_item->ytrans, inv_item->ztrans);
	phd_RotYXZ(inv_item->y_rot, inv_item->x_rot, 0);

	auto object = &objects[inv_item->object_number];

	if (!object->loaded)
	{
		prof::print(RED, "Object {} Not Loaded", inv_item->object_number);
		return;
	}

	if (object->nmeshes < 0)
		return S_DrawSprite(SPRITE_REL, 0, 0, 0, (int16_t*)object->mesh_ptr, 0, 0);

	if (auto isprite_list = inv_item->sprlist)
	{
		int zv = *(phd_mxptr + M23),
			xv = *(phd_mxptr + M03),
			yv = *(phd_mxptr + M13),
			zp = zv / phd_persp,
			sx = (int32_t)(xv / zp + phd_centerx),
			sy = (int32_t)(yv / zp + phd_centery);

		auto isprite = *isprite_list++;

		do {
			if (zv < phd_znear || zv > phd_zfar)
				break;

			while (isprite->shape)
			{
				switch (isprite->shape)
				{
				case SHAPE_SPRITE:	S_DrawScreenSprite(sx + isprite->x, sy + isprite->y, isprite->z, isprite->param1, isprite->param2, (int16_t)(static_objects[ALPHABET].mesh_number + isprite->sprnum), 16 * 256, 0); break;
				case SHAPE_LINE:	S_DrawScreenLine(sx + isprite->x, sy + isprite->y, isprite->z, isprite->param1, isprite->param2, isprite->sprnum, isprite->grdptr, 0); break;
				case SHAPE_BOX:		S_DrawScreenBox(sx + isprite->x, sy + isprite->y, isprite->z, isprite->param1, isprite->param2, isprite->sprnum, isprite->grdptr, 0); break;
				case SHAPE_FBOX:	S_DrawScreenFBox(sx + isprite->x, sy + isprite->y, isprite->z, isprite->param1, isprite->param2, isprite->sprnum, isprite->grdptr, 0); break;
				}

				++isprite;
			}
		} while (isprite = *isprite_list++);
	}

	auto frame = object->frame_base + inv_item->current_frame * (anims[object->anim_index].interpolation >> 8);

	phd_PushMatrix();

	if (int clip = S_GetObjectBounds(frame))
	{
		phd_TranslateRel((int32_t)*(frame + 6), (int32_t)*(frame + 7), (int32_t)*(frame + 8));

		auto rotation = frame + 9;

		gar_RotYXZsuperpack(&rotation, 0);

		auto mesh = object->mesh_ptr;
		auto bone = object->bone_ptr;

		int16_t mesh_num = 1;

		if (mesh_num & inv_item->drawn_meshes)
			phd_PutPolygons(*mesh, clip);

		for (int i = object->nmeshes - 1; i > 0; --i, bone += 3)
		{
			++mesh;

			mesh_num <<= 1;

			int poppush = *(bone++);

			if (poppush & 1) phd_PopMatrix();
			if (poppush & 2) phd_PushMatrix();

			phd_TranslateRel(*bone, *(bone + 1), *(bone + 2));
			gar_RotYXZsuperpack(&rotation, 0);

			if (inv_item->object_number == MAP_OPTION)
			{
				if (i == 1)
				{
					phd_RotZ((PHD_ANGLE)seconds);

					inv_item->misc_data[1] = inv_item->misc_data[0];
					inv_item->misc_data[0] = seconds;
				}

				if (i == 2) phd_RotZ((PHD_ANGLE)minutes);
				if (i == 3) phd_RotZ((PHD_ANGLE)hours);
			}

			if (mesh_num & inv_item->drawn_meshes)
				phd_PutPolygons(*mesh, clip);
		}
	}

	phd_PopMatrix();
}

void DoInventoryBackground()
{
	static int bz = 12288;

	if (!objects[MENU_BACKGROUND].loaded)
		return;

	auto angles = phd_GetVectorAngles({ 0, 0x1000, 0 });

	PHD_3DPOS viewer { 0, -0x200, 0, (PHD_ANGLE)angles.y, (PHD_ANGLE)angles.x, (PHD_ANGLE)0 };

	phd_GenerateW2V(viewer);

	angles = phd_GetVectorAngles({ -0x600, 0x100, 0x400 });

	phd_RotateLight(angles.y, angles.x);

	//S_SetupAboveWater(0);

	phd_PushMatrix();
	{
		phd_TranslateAbs(0, bz, 0);
		phd_RotYXZ(0, 0x4000, (short)0x8000);

		auto frame = anims[objects[MENU_BACKGROUND].anim_index].frame_ptr + 9;

		gar_RotYXZsuperpack(&frame, 0);
		phd_RotYXZ((int16_t)0x8000, 0, 0);
	}

	phd_PopMatrix();
}