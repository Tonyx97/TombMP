#include "objects.h"
#include "lara.h"
#include "laraanim.h"
#include "effect2.h"
#include "control.h"
#include "camera.h"
#include "lara2gun.h"

#include <specific/standard.h>
#include <specific/input.h>

#define AIMGUNS_F	0
#define DRAW1_F		5
#define DRAW2_F		13
#define RECOIL_F	24

struct PISTOL_DEF
{
	int16_t ObjectNum;
	char Draw1Anim2;
	char Draw1Anim;
	char Draw2Anim;
	char RecoilAnim;
};

PISTOL_DEF PistolTable[4] =
{
	{        0,  0,  0,  0,  0 },		// LG_UNARMED
	{  PISTOLS,  4,  5, 13, 24 },		// LG_PISTOLS
	{   MAGNUM,  7,  8, 15, 29 },		// LG_MAGNUMS
	{      UZI,  4,  5, 13, 24 }		// LG_UZIS
};

void set_arm_info(LARA_ARM* arm, int frame)
{
	auto p = &PistolTable[lara.gun_type];
	auto anim_base = objects[p->ObjectNum].anim_index;

	if (frame < p->Draw1Anim)		arm->anim_number = anim_base;
	else if (frame < p->Draw2Anim)  arm->anim_number = anim_base + 1;
	else if (frame < p->RecoilAnim) arm->anim_number = anim_base + 2;
	else							arm->anim_number = anim_base + 3;

	arm->frame_base = anims[arm->anim_number].frame_ptr;
	arm->frame_number = frame;
}

void draw_pistols(int weapon_type)
{
	auto p = &PistolTable[lara.gun_type];
	auto ani = lara.left_arm.frame_number + 1;

	if (ani < p->Draw1Anim || ani > p->RecoilAnim - 1)
		ani = p->Draw1Anim;
	else if (ani == p->Draw2Anim)
	{
		draw_pistol_meshes(weapon_type);
		g_audio->play_sound(6, { lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos });
	}
	else if (ani == p->RecoilAnim - 1)
	{
		ready_pistols(weapon_type);
		ani = 0;
	}

	set_arm_info(&lara.right_arm, ani);
	set_arm_info(&lara.left_arm, ani);
}

void undraw_pistols(int weapon_type)
{
	auto p = &PistolTable[lara.gun_type];
	auto anil = lara.left_arm.frame_number;

	if (anil >= p->RecoilAnim)
		anil = p->Draw1Anim2;

	else if (anil > 0 && anil < p->Draw1Anim)
	{
		lara.left_arm.x_rot -= lara.left_arm.x_rot / anil;
		lara.left_arm.y_rot -= lara.left_arm.y_rot / anil;

		--anil;
	}
	else if (anil == 0)
	{
		lara.left_arm.x_rot = lara.left_arm.y_rot = lara.left_arm.z_rot = 0;
		anil = p->RecoilAnim - 1;
	}
	else if (anil > p->Draw1Anim && anil < p->RecoilAnim)
	{
		if (--anil == p->Draw2Anim - 1)
		{
			undraw_pistol_mesh_left(weapon_type);
			g_audio->play_sound(7, { lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos });
		}
	}

	set_arm_info(&lara.left_arm, anil);

	auto anir = lara.right_arm.frame_number;

	if (anir >= p->RecoilAnim)
		anir = p->Draw1Anim2;

	else if ((anir > 0) && (anir < p->Draw1Anim))
	{
		lara.right_arm.x_rot -= lara.right_arm.x_rot / anir;
		lara.right_arm.y_rot -= lara.right_arm.y_rot / anir;

		--anir;
	}
	else if (anir == 0)
	{
		lara.right_arm.x_rot = lara.right_arm.y_rot = lara.right_arm.z_rot = 0;
		anir = p->RecoilAnim - 1;
	}
	else if ((anir > p->Draw1Anim) && (anir < p->RecoilAnim))
	{
		if (--anir == p->Draw2Anim - 1)
		{
			undraw_pistol_mesh_right(weapon_type);
			g_audio->play_sound(7, { lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos });
		}
	}

	set_arm_info(&lara.right_arm, anir);

	if ((anil == p->Draw1Anim) && (anir == p->Draw1Anim))
	{
		lara.gun_status = LG_ARMLESS;
		lara.right_arm.frame_number = lara.left_arm.frame_number = 0;
		lara.target = nullptr;
		lara.left_arm.lock = lara.right_arm.lock = 0;
	}

	if (!(input & IN_LOOK))
	{
		lara.torso_y_rot = lara.head_y_rot = (lara.left_arm.y_rot + lara.right_arm.y_rot) / 4;
		lara.torso_x_rot = lara.head_x_rot = (lara.left_arm.x_rot + lara.right_arm.x_rot) / 4;
	}
}

void ready_pistols(int weapon_type)
{
	lara.gun_status = LG_READY;
	lara.left_arm.x_rot = lara.left_arm.y_rot = lara.left_arm.z_rot = 0;
	lara.right_arm.x_rot = lara.right_arm.y_rot = lara.right_arm.z_rot = 0;
	lara.left_arm.frame_number = lara.right_arm.frame_number = 0;
	lara.target = nullptr;
	lara.left_arm.lock = lara.right_arm.lock = 0;
	lara.left_arm.frame_base = lara.right_arm.frame_base = objects[WeaponObject(weapon_type)].frame_base;
}

void draw_pistol_meshes(int weapon_type)
{
	int obj = WeaponObject(weapon_type);

	lara.mesh_ptrs[HAND_R] = *(objects[obj].mesh_ptr + HAND_R);
	lara.mesh_ptrs[THIGH_R] = *(objects[LARA].mesh_ptr + THIGH_R);

	if (weapon_type != LG_MAGNUMS)
	{
		lara.mesh_ptrs[HAND_L] = *(objects[obj].mesh_ptr + HAND_L);
		lara.mesh_ptrs[THIGH_L] = *(objects[LARA].mesh_ptr + THIGH_L);
	}
}

void undraw_pistol_mesh_left(int weapon_type)
{
	if (weapon_type != LG_MAGNUMS)
	{
		int obj = WeaponObject(weapon_type);

		lara.mesh_ptrs[THIGH_L] = *(objects[obj].mesh_ptr + THIGH_L);
		lara.mesh_ptrs[HAND_L] = *(objects[LARA].mesh_ptr + HAND_L);
	}
}

void undraw_pistol_mesh_right(int weapon_type)
{
	int obj = WeaponObject(weapon_type);

	lara.mesh_ptrs[THIGH_R] = *(objects[obj].mesh_ptr + THIGH_R);
	lara.mesh_ptrs[HAND_R] = *(objects[LARA].mesh_ptr + HAND_R);
}

void PistolHandler(int weapon_type)
{
	auto winfo = &weapons[weapon_type];

	if (input & IN_ACTION)
		LaraTargetInfo(winfo);
	else lara.target = nullptr;

	LaraGetNewTarget(winfo);

	AimWeapon(winfo, &lara.left_arm);
	AimWeapon(winfo, &lara.right_arm);

	if (lara.left_arm.lock && !lara.right_arm.lock)
	{
		lara.torso_y_rot = lara.left_arm.y_rot / 2;
		lara.torso_x_rot = lara.left_arm.x_rot / 2;

		if (camera.old_type != LOOK_CAMERA)
		{
			lara.head_y_rot = lara.torso_y_rot;
			lara.head_x_rot = lara.torso_x_rot;
		}
	}
	else if (lara.right_arm.lock && !lara.left_arm.lock)
	{
		lara.torso_y_rot = lara.right_arm.y_rot / 2;
		lara.torso_x_rot = lara.right_arm.x_rot / 2;

		if (camera.old_type != LOOK_CAMERA)
		{
			lara.head_y_rot = lara.torso_y_rot;
			lara.head_x_rot = lara.torso_x_rot;
		}
	}
	else if (lara.left_arm.lock && lara.right_arm.lock)
	{
		lara.torso_y_rot = (lara.left_arm.y_rot + lara.right_arm.y_rot) / 4;
		lara.torso_x_rot = (lara.left_arm.x_rot + lara.right_arm.x_rot) / 4;

		if (camera.old_type != LOOK_CAMERA)
		{
			lara.head_y_rot = lara.torso_y_rot;
			lara.head_x_rot = lara.torso_x_rot;
		}
	}

	AnimatePistols(weapon_type);

	if (lara.left_arm.flash_gun || lara.right_arm.flash_gun)
	{
		PHD_VECTOR pos { (GetRandomControl() & 255) - 128, (GetRandomControl() & 127) - 63, (GetRandomControl() & 255) - 128 };

		get_lara_bone_pos(lara_item, &pos, (lara.left_arm.flash_gun) ? HAND_L : HAND_R);
		TriggerDynamicLight(pos.x, pos.y, pos.z, 10, (GetRandomControl() & 7) + 24, (GetRandomControl() & 3) + 16, GetRandomControl() & 7);
	}
}

void AnimatePistols(int weapon_type)
{
	static bool uzi_left = false,
				uzi_right = false;

	bool sound_already = false;

	draw_weapon_smoke(lara_item, smoke_weapon, smoke_count_l, smoke_count_r);

	auto winfo = &weapons[weapon_type];
	auto p = &PistolTable[lara.gun_type];

	auto anir = lara.right_arm.frame_number,
		 anil = lara.left_arm.frame_number;

	if (lara.right_arm.lock || (input & IN_ACTION && !lara.target))
	{
		if (anir >= 0 && anir < p->Draw1Anim2)
			++anir;
		else if (anir == p->Draw1Anim2)
		{
			if (input & IN_ACTION)
			{
				if (weapon_type != LG_MAGNUMS)
				{
					PHD_ANGLE angles[] { lara.right_arm.y_rot + lara_item->pos.y_rot, lara.right_arm.x_rot };

					if (FireWeapon(weapon_type, lara.target, lara_item, angles))
					{
						smoke_count_r = 28;
						smoke_weapon = weapon_type;

						auto pos = get_gun_shell_pos(HAND_R, weapon_type);

						TriggerGunShell(pos.x, pos.y, pos.z, lara_item->pos.y_rot, GUNSHELL, weapon_type, false, lara_item->room_number, true);

						lara.right_arm.flash_gun = winfo->flash_time;

						g_audio->play_sound((int)winfo->sample_num, { lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos });

						sound_already = true;

						if (weapon_type == LG_UZIS)
							uzi_right = true;
					}
				}

				anir = p->RecoilAnim;
			}
			else if (uzi_right)
			{
				g_audio->stop_sound((int)winfo->sample_num);
				g_audio->play_sound((int)winfo->sample_num + 1, { lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos });

				uzi_right = false;
			}
		}

		else if (anir >= p->RecoilAnim)
		{
			if (weapon_type == LG_UZIS)
			{
				g_audio->play_sound((int)winfo->sample_num, { lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos });

				uzi_right = true;
			}

			if (++anir == (p->RecoilAnim + winfo->recoil_frame))
				anir = p->Draw1Anim2;
		}
	}
	else
	{
		if (anir >= p->RecoilAnim)
			anir = p->Draw1Anim2;
		else if ((anir > 0) && (anir <= p->Draw1Anim2))
			--anir;

		if (uzi_right)
		{
			g_audio->stop_sound((int)winfo->sample_num);
			g_audio->play_sound((int)winfo->sample_num + 1, { lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos });
			uzi_right = false;
		}
	}

	set_arm_info(&lara.right_arm, anir);

	if (lara.left_arm.lock || (input & IN_ACTION && !lara.target))
	{
		if (anil >= 0 && anil < p->Draw1Anim2)
			++anil;
		else if (anil == p->Draw1Anim2)
		{
			if (input & IN_ACTION)
			{
				PHD_ANGLE angles[] { lara.left_arm.y_rot + lara_item->pos.y_rot, lara.left_arm.x_rot };

				if (FireWeapon(weapon_type, lara.target, lara_item, angles))
				{
					if (weapon_type == LG_MAGNUMS)
					{
						smoke_count_r = 28;
						smoke_weapon = weapon_type;

						auto pos = get_gun_shell_pos(HAND_R, weapon_type);

						TriggerGunShell(pos.x, pos.y, pos.z, lara_item->pos.y_rot, GUNSHELL, weapon_type, false, lara_item->room_number, true);

						lara.right_arm.flash_gun = winfo->flash_time;
					}
					else
					{
						smoke_count_l = 28;
						smoke_weapon = weapon_type;

						auto pos = get_gun_shell_pos(HAND_L, weapon_type);

						TriggerGunShell(pos.x, pos.y, pos.z, lara_item->pos.y_rot, GUNSHELL, weapon_type, true, lara_item->room_number, true);

						lara.left_arm.flash_gun = winfo->flash_time;
					}

					if (!sound_already)
						g_audio->play_sound((int)winfo->sample_num, { lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos });

					if (weapon_type == LG_UZIS)
						uzi_left = true;
				}

				anil = p->RecoilAnim;
			}
			else if (uzi_left)
			{
				g_audio->stop_sound((int)winfo->sample_num);
				g_audio->play_sound((int)winfo->sample_num + 1, { lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos });

				uzi_left = false;
			}
		}
		else if (anil >= p->RecoilAnim)
		{
			if (weapon_type == LG_UZIS)
			{
				g_audio->play_sound((int)winfo->sample_num, { lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos });

				uzi_left = true;
			}

			if (++anil == (p->RecoilAnim + winfo->recoil_frame))
				anil = p->Draw1Anim2;
		}
	}
	else
	{
		if (anil >= p->RecoilAnim)
			anil = p->Draw1Anim2;
		else if ((anil > 0) && (anil <= p->Draw1Anim2))
			--anil;

		if (uzi_left)
		{
			g_audio->stop_sound((int)winfo->sample_num);
			g_audio->play_sound((int)winfo->sample_num + 1, { lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos });
			uzi_left = 0;
		}
	}

	set_arm_info(&lara.left_arm, anil);
}