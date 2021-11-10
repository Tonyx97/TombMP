#include "objects.h"
#include "lara.h"
#include "laraanim.h"
#include "effect2.h"
#include "control.h"
#include "sphere.h"

#include <specific/standard.h>
#include <specific/output.h>

#define SHIVA_PINCER_DAMAGE		150
#define SHIVA_CHOPPER_DAMAGE	180
#define SHIVA_FEELER_DISTANCE	WALL_L
#define SHIVA_WALK_TURN			(ONE_DEGREE * 4)
#define SHIVA_WAIT_CHANCE		0x200
#define SHIVA_WALK_CHANCE		(SHIVA_WAIT_CHANCE + 0x200)
#define SHIVA_PINCER_RANGE		SQUARE(WALL_L * 5 / 4)
#define SHIVA_CHOPPER_RANGE		SQUARE(WALL_L * 4 / 3)
#define SHIVA_WALK_RANGE		SQUARE(WALL_L * 2)
#define SHIVA_DIE_ANIM			22
#define SHIVA_START_ANIM		14
#define SHIVA_KILL_ANIM			18
#define SHIVA_TOUCHL			0x2400
#define SHIVA_TOUCHR			0x2400000
#define SHIVA_HURT_FLAG			4

BITE_INFO shiva_left  = { 0, 0, 920, 13 },
		  shiva_right = { 0, 0, 920, 22 };

int effect_mesh = 0;

enum shiva_anims
{
	SHIVA_WAIT,
	SHIVA_WALK,
	SHIVA_WAIT_DEF,
	SHIVA_WALK_DEF,
	SHIVA_START,
	SHIVA_PINCER,
	SHIVA_KILL,
	SHIVA_CHOPPER,
	SHIVA_WALKBACK,
	SHIVA_DEATH
};

void ShivaDamage(ITEM_INFO* item, CREATURE_INFO* shiva, int damage)
{
	if (!shiva->flags)
	{
		if (item->touch_bits & SHIVA_TOUCHR)
		{
			lara_item->hit_points -= damage;
			lara_item->hit_status = 1;

			CreatureEffect(item, &shiva_right, DoBloodSplat);

			shiva->flags = 1;

			g_audio->play_sound(318, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
		}

		if (item->touch_bits & SHIVA_TOUCHL)
		{
			lara_item->hit_points -= damage;
			lara_item->hit_status = 1;

			CreatureEffect(item, &shiva_left, DoBloodSplat);

			shiva->flags = 1;

			g_audio->play_sound(318, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });
		}
	}
}

void TriggerShivaSmoke(long x, long y, long z, long uw)
{
	int dx = lara_item->pos.x_pos - x,
		dz = lara_item->pos.z_pos - z;

	if (dx < -0x4000 || dx > 0x4000 || dz < -0x4000 || dz > 0x4000)
		return;

	auto sptr = &spark[GetFreeSpark()];

	int size = (GetRandomControl() & 31) + 128;
	
	sptr->On = 1;

	if (uw)
	{
		sptr->sR = 0;
		sptr->sG = 0;
		sptr->sB = 0;
		sptr->dR = 192;
		sptr->dG = 192;
		sptr->dB = 208;
	}
	else
	{
		sptr->sR = 144;
		sptr->sG = 144;
		sptr->sB = 144;
		sptr->dR = 64;
		sptr->dG = 64;
		sptr->dB = 64;
	}

	sptr->ColFadeSpeed = 8;
	sptr->FadeToBlack = 64;
	sptr->sLife = sptr->Life = (GetRandomControl() & 31) + 96;
	sptr->TransType = (uw ? COLADD : COLSUB);
	sptr->extras = 0;
	sptr->Dynamic = -1;
	sptr->x = x + (GetRandomControl() & 31) - 16;
	sptr->y = y + (GetRandomControl() & 31) - 16;
	sptr->z = z + (GetRandomControl() & 31) - 16;
	sptr->Xvel = ((GetRandomControl() & 4095) - 2048) >> 2;
	sptr->Yvel = (GetRandomControl() & 255) - 128;
	sptr->Zvel = ((GetRandomControl() & 4095) - 2048) >> 2;

	if (uw)
	{
		sptr->Yvel >>= 4;
		sptr->y += 32;
		sptr->Friction = 4 | (1 << 4);
	}
	else sptr->Friction = 6;

	sptr->Flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
	sptr->RotAng = GetRandomControl() & 4095;
	sptr->RotAdd = ((GetRandomControl() & 1) ? -(GetRandomControl() & 15) - 16 : (GetRandomControl() & 15) + 16);
	sptr->Def = uint8_t(objects[EXPLOSION1].mesh_ptr);
	sptr->Scalar = 3;

	if (uw)
		sptr->Gravity = sptr->MaxYvel = 0;
	else
	{
		sptr->Gravity = -(GetRandomControl() & 3) - 3;
		sptr->MaxYvel = -(GetRandomControl() & 3) - 4;
	}

	sptr->Width = sptr->sWidth = size >> 2;
	sptr->dWidth = size;

	size += (GetRandomControl() & 31) + 32;

	sptr->Height = sptr->sHeight = size >> 3;
	sptr->dHeight = size;
}

void DrawShiva(ITEM_INFO* item)
{
	int16_t* frmptr[2] = { nullptr, nullptr };
	int rate;

	int frac = GetFrames(item, frmptr, &rate);

	if (!frmptr[0] || !frmptr[1])
		return;

	if (item->hit_points <= 0 && item->status != ACTIVE && item->mesh_bits != 0)
	{
		item->mesh_bits = item->mesh_bits >> 1;

		PHD_VECTOR pos { 0, 0, 256 };

		GetJointAbsPosition(item, &pos, effect_mesh++);
		TriggerShivaSmoke(pos.x, pos.y, pos.z, 1);
	}

	auto object = &objects[item->object_number];

	if (object->shadow_size)
		S_PrintShadow(object->shadow_size, frmptr[0], item, 0);

	phd_PushMatrix();
	phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
	phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);

	if (int clip = S_GetObjectBounds(frmptr[0]))
	{
		CalculateObjectLighting(item, frmptr[0]);

		auto extra_rotation = (int16_t*)item->data;
		auto meshpp = &meshes[objects[SHIVA].mesh_ptr],
			 swappp = &meshes[objects[MESHSWAP1].mesh_ptr];

		auto bone = object->bone_ptr;

		uint32_t bit = 1;

		if (!frac)
		{
			phd_TranslateRel((int32_t) * (frmptr[0] + 6), (int32_t) * (frmptr[0] + 7), (int32_t) * (frmptr[0] + 8));

			auto rotation1 = frmptr[0] + 9;

			gar_RotYXZsuperpack(&rotation1, 0);
			phd_PutPolygons(bit & item->mesh_bits ? *meshpp : *swappp, clip);

			++meshpp;
			++swappp;

			for (int i = object->nmeshes - 1; i > 0; --i, bone += 4, ++meshpp, ++swappp)
			{
				int poppush = *(bone);

				if (poppush & 1) phd_PopMatrix();
				if (poppush & 2) phd_PushMatrix();

				phd_TranslateRel(*(bone + 1), *(bone + 2), *(bone + 3));
				gar_RotYXZsuperpack(&rotation1, 0);

				if (extra_rotation && (poppush & (ROT_X | ROT_Y | ROT_Z)))
				{
					if (poppush & ROT_Y) phd_RotY(*(extra_rotation++));
					if (poppush & ROT_X) phd_RotX(*(extra_rotation++));
					if (poppush & ROT_Z) phd_RotZ(*(extra_rotation++));
				}

				bit <<= 1;

				phd_PutPolygons(bit& item->mesh_bits ? *meshpp : *swappp, clip);
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
			phd_PutPolygons_I(bit & item->mesh_bits ? *meshpp : *swappp, clip);

			++meshpp;
			++swappp;

			for (int i = object->nmeshes - 1; i > 0; --i, bone += 4, ++meshpp, ++swappp)
			{
				int poppush = *(bone);

				if (poppush & 1) phd_PopMatrix_I();
				if (poppush & 2) phd_PushMatrix_I();

				phd_TranslateRel_I(*(bone + 1), *(bone + 2), *(bone + 3));
				gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);

				if (extra_rotation && (poppush & (ROT_X | ROT_Y | ROT_Z)))
				{
					if (poppush & ROT_Y) phd_RotY_I(*(extra_rotation++));
					if (poppush & ROT_X) phd_RotX_I(*(extra_rotation++));
					if (poppush & ROT_Z) phd_RotZ_I(*(extra_rotation++));
				}

				bit <<= 1;

				if (bit & item->mesh_bits)
					phd_PutPolygons_I(*meshpp, clip);
				else if (item->hit_points > 0 || item->status == ACTIVE || bit != 0x400 || item->carried_item == -1)
					phd_PutPolygons_I(*swappp, clip);
			}
		}
	}

	phd_PopMatrix();
}

void InitialiseShiva(int16_t item_number)
{
	auto item = &items[item_number];

	if (item->object_number == SHIVA)
	{
		item->anim_number = objects[SHIVA].anim_index + SHIVA_START_ANIM;
		item->frame_number = anims[item->anim_number].frame_base;
		item->current_anim_state = item->goal_anim_state = SHIVA_START;
	}

	item->status = NOT_ACTIVE;
	item->mesh_bits = 0;
}

void ShivaControl(int16_t item_number)
{
	if (!CreatureActive(item_number))
		return;

	auto item = &items[item_number];
	auto shiva = (CREATURE_INFO*)item->data;

	int16_t head_x = 0,
		   head_y = 0,
		   torso_x = 0,
		   torso_y = 0,
		   angle = 0,
		   tilt = 0;

	bool lara_alive = (lara_item->hit_points > 0);

	if (item->hit_points <= 0)
	{
		if (item->current_anim_state != SHIVA_DEATH)
		{
			item->anim_number = objects[item->object_number].anim_index + SHIVA_DIE_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = SHIVA_DEATH;
		}
	}
	else
	{
		AI_INFO info;

		CreatureAIInfo(item, &info);
		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		if (shiva->mood == ESCAPE_MOOD)
		{
			shiva->target.x = lara_item->pos.x_pos;
			shiva->target.z = lara_item->pos.z_pos;
		}

		angle = CreatureTurn(item, shiva->maximum_turn);

		if (item->current_anim_state != SHIVA_START)
			item->mesh_bits = 0xffffffff;

		switch (item->current_anim_state)
		{
		case SHIVA_START:
		{
			shiva->maximum_turn = 0;

			if (!shiva->flags)
			{
				if (item->mesh_bits == 0)
					effect_mesh = 0;

				item->mesh_bits = (item->mesh_bits << 1) + 1;
				shiva->flags = 1;

				PHD_VECTOR pos{ 0, 0, 256 };

				GetJointAbsPosition(item, &pos, effect_mesh++);

				TriggerExplosionSparks(pos.x, pos.y, pos.z, 2, 0, 0, item->room_number);
				TriggerShivaSmoke(pos.x, pos.y, pos.z, 1);
			}
			else --shiva->flags;

			if (item->mesh_bits == 0x7fffffff)
			{
				item->goal_anim_state = SHIVA_WAIT;
				effect_mesh = 0;
				shiva->flags = -45;
			}

			break;
		}
		case SHIVA_WAIT:
		{
			if (info.ahead)
				head_y = info.angle;

			if (shiva->flags < 0)
			{
				++shiva->flags;

				TriggerShivaSmoke(item->pos.x_pos + (GetRandomControl() & 0x5FF) - 0x300, item->pos.y_pos - (GetRandomControl() & 0x5FF), item->pos.z_pos + (GetRandomControl() & 0x5FF) - 0x300, 1);

				break;
			}

			if (shiva->flags == 1)
				shiva->flags = 0;

			shiva->maximum_turn = 0;

			if (shiva->mood == ESCAPE_MOOD)
			{
				auto room_number = item->room_number;

				int x = item->pos.x_pos + (SHIVA_FEELER_DISTANCE * phd_sin(item->pos.y_rot + 0x8000) >> W2V_SHIFT),
					z = item->pos.z_pos + (SHIVA_FEELER_DISTANCE * phd_cos(item->pos.y_rot + 0x8000) >> W2V_SHIFT);

				auto floor = GetFloor(x, item->pos.y_pos, z, &room_number);

				item->goal_anim_state = (!shiva->flags && floor->box != NO_BOX && !(boxes[floor->box].overlap_index & BLOCKABLE) ? SHIVA_WALKBACK : SHIVA_WAIT_DEF);
			}
			else if (shiva->mood == BORED_MOOD)
			{
				if (GetRandomControl() < SHIVA_WALK_CHANCE)
					item->goal_anim_state = SHIVA_WALK;
			}
			else if (info.bite && info.distance < SHIVA_PINCER_RANGE)
			{
				item->goal_anim_state = SHIVA_PINCER;
				shiva->flags = 0;
			}
			else if (info.bite && info.distance < SHIVA_CHOPPER_RANGE)
			{
				item->goal_anim_state = SHIVA_CHOPPER;
				shiva->flags = 0;
			}
			else if (item->hit_status && info.ahead)
			{
				shiva->flags = SHIVA_HURT_FLAG;
				item->goal_anim_state = SHIVA_WAIT_DEF;
			}
			else item->goal_anim_state = SHIVA_WALK;

			break;
		}
		case SHIVA_WAIT_DEF:
		{
			shiva->maximum_turn = 0;

			if (info.ahead)
				head_y = info.angle;

			if (item->hit_status || shiva->mood == ESCAPE_MOOD)
				shiva->flags = SHIVA_HURT_FLAG;

			if ((info.bite && info.distance < SHIVA_CHOPPER_RANGE) || (item->frame_number == anims[item->anim_number].frame_base && !shiva->flags) || !info.ahead)
			{
				item->goal_anim_state = SHIVA_WAIT;
				shiva->flags = 0;
			}
			else if (shiva->flags)
				item->goal_anim_state = SHIVA_WAIT_DEF;

			if (item->frame_number == anims[item->anim_number].frame_base && shiva->flags > 1)
				shiva->flags -= 2;

			break;
		}
		case SHIVA_WALK:
		{
			shiva->maximum_turn = SHIVA_WALK_TURN;

			if (info.ahead)
				head_y = info.angle;

			if (shiva->mood == ESCAPE_MOOD)
				item->goal_anim_state = SHIVA_WAIT;
			else if (shiva->mood == BORED_MOOD)
			{
				item->goal_anim_state = SHIVA_WAIT;
			}
			else if (info.bite && info.distance < SHIVA_CHOPPER_RANGE)
			{
				item->goal_anim_state = SHIVA_WAIT;
				shiva->flags = 0;
			}
			else if (item->hit_status)
			{
				shiva->flags = SHIVA_HURT_FLAG;
				item->goal_anim_state = SHIVA_WALK_DEF;
			}

			break;
		}
		case SHIVA_WALK_DEF:
		{
			shiva->maximum_turn = SHIVA_WALK_TURN;

			if (info.ahead)
				head_y = info.angle;

			if (item->hit_status)
				shiva->flags = SHIVA_HURT_FLAG;

			if ((info.bite && info.distance < SHIVA_PINCER_RANGE) || (item->frame_number == anims[item->anim_number].frame_base && !shiva->flags))
			{
				item->goal_anim_state = SHIVA_WALK;
				shiva->flags = 0;
			}
			else if (shiva->flags)
				item->goal_anim_state = SHIVA_WALK_DEF;

			if (item->frame_number == anims[item->anim_number].frame_base)
				shiva->flags = 0;

			break;
		}
		case SHIVA_WALKBACK:
		{
			shiva->maximum_turn = SHIVA_WALK_TURN;

			if (info.ahead)
				head_y = info.angle;

			if ((info.ahead && info.distance < SHIVA_CHOPPER_RANGE) || (item->frame_number == anims[item->anim_number].frame_base && !shiva->flags))
				item->goal_anim_state = SHIVA_WAIT;
			else if (item->hit_status)
			{
				shiva->flags = SHIVA_HURT_FLAG;
				item->goal_anim_state = SHIVA_WAIT;
			}

			break;
		}
		case SHIVA_PINCER:
		{
			shiva->maximum_turn = SHIVA_WALK_TURN;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
				head_y = info.angle;
			}

			ShivaDamage(item, shiva, SHIVA_PINCER_DAMAGE);

			break;
		}
		case SHIVA_CHOPPER:
		{
			shiva->maximum_turn = SHIVA_WALK_TURN;

			if (info.ahead)
			{
				torso_y = info.angle;
				head_y = info.angle;

				if (info.x_angle > 0)
					torso_x = info.x_angle;
			}

			ShivaDamage(item, shiva, SHIVA_CHOPPER_DAMAGE);

			break;
		}
		case SHIVA_KILL:
		{
			shiva->maximum_turn = torso_y = torso_x = head_x = head_y = 0;

			if (item->frame_number == anims[item->anim_number].frame_base + 10 ||
				item->frame_number == anims[item->anim_number].frame_base + 21 ||
				item->frame_number == anims[item->anim_number].frame_base + 33)
			{
				CreatureEffect(item, &shiva_right, DoBloodSplat);
				CreatureEffect(item, &shiva_left, DoBloodSplat);
			}
		}
		}
	}

	if (lara_alive && lara_item->hit_points <= 0)
	{
		CreatureKill(item, SHIVA_KILL_ANIM, SHIVA_KILL, EXTRA_YETIKILL);
		return;
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso_y);
	CreatureJoint(item, 1, torso_x);
	CreatureJoint(item, 2, head_y - torso_y);
	CreatureJoint(item, 3, head_x);

	CreatureAnimation(item_number, angle, 0);
}