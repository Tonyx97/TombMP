#include "effect2.h"
#include "objects.h"
#include "lara.h"
#include "control.h"
#include "sphere.h"
#include "people.h"
#include "triboss.h"

#include <specific/fn_stubs.h>

#define LIZMAN_BITE_DAMAGE		100
#define LIZMAN_SWIPE_DAMAGE		120
#define LIZMAN_SPIT_DAMAGE		10
#define LIZMAN_WALK_TURN		(ONE_DEGREE*10)
#define LIZMAN_RUN_TURN			(ONE_DEGREE*4)
#define LIZMAN_ATTACK0_RANGE	SQUARE(WALL_L*5/2)
#define LIZMAN_ATTACK1_RANGE	SQUARE(WALL_L*3/4)
#define LIZMAN_ATTACK2_RANGE	SQUARE(WALL_L*3/2)
#define LIZMAN_WALK_RANGE		SQUARE(WALL_L*2)
#define LIZMAN_WALK_CHANCE		0x100
#define LIZMAN_WAIT_CHANCE		0x100
#define LIZMAN_DIE_ANIM			26
#define LIZMAN_CLIMB1_ANIM		28
#define LIZMAN_CLIMB2_ANIM		29
#define LIZMAN_CLIMB3_ANIM		27
#define LIZMAN_FALL3_ANIM		30
#define LIZMAN_SLIDE1_ANIM		23
#define LIZMAN_SLIDE2_ANIM		31
#define LIZMAN_BITE_TOUCH		0xc00
#define LIZMAN_SWIPE_TOUCH		0x20
#define LIZMAN_VAULT_SHIFT		260

enum lizman_anims
{
	LIZMAN_EMPTY,
	LIZMAN_STOP,
	LIZMAN_WALK,
	LIZMAN_PUNCH2,
	LIZMAN_AIM2,
	LIZMAN_WAIT,
	LIZMAN_AIM1,
	LIZMAN_AIM0,
	LIZMAN_PUNCH1,
	LIZMAN_PUNCH0,
	LIZMAN_RUN,
	LIZMAN_DEATH,
	LIZMAN_CLIMB3,
	LIZMAN_CLIMB1,
	LIZMAN_CLIMB2,
	LIZMAN_FALL3
};

int16_t TriggerLizmanGasThrower(ITEM_INFO* item, BITE_INFO* bite, int16_t speed);
void TriggerLizmanGas(long x, long y, long z, long xv, long yv, long zv, long fxnum);

BITE_INFO lizman_bite_hit  = { 0, -120, 120, 10 },
		  lizman_swipe_hit = { 0, 0, 0, 5 },
		  lizman_gas	   = { 0, -64, 56, 9 };

void LizManControl(int16_t item_number)
{
	if (!CreatureActive(item_number))
		return;

	auto item = &items[item_number];
	auto lizman = (CREATURE_INFO*)item->data;

	int16_t head = 0,
		   neck = 0,
		   angle = 0,
		   tilt = 0;

	if (item->hit_points <= 0)
	{
		if (item->current_anim_state != LIZMAN_DEATH)
		{
			item->anim_number = objects[LIZARD_MAN].anim_index + LIZMAN_DIE_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = LIZMAN_DEATH;
		}
		else if (lizard_man_active)
		{
			if (item->frame_number - anims[item->anim_number].frame_base == 50)
			{
				CreatureDie(item_number, true);

				lizard_man_active = 0;
			}
		}
	}
	else
	{
		AI_INFO info;

		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		if (boxes[lizman->enemy->box_number].overlap_index & BLOCKABLE)
			lizman->mood = ATTACK_MOOD;

		angle = CreatureTurn(item, lizman->maximum_turn);

		switch (item->current_anim_state)
		{
		case LIZMAN_STOP:
		{
			lizman->flags = 0;
			lizman->maximum_turn = 0;

			if (info.ahead)
				neck = info.angle;

			if (lizman->mood == ESCAPE_MOOD)
				item->goal_anim_state = LIZMAN_RUN;
			else if (lizman->mood == BORED_MOOD)
			{
				if (item->required_anim_state)
					item->goal_anim_state = item->required_anim_state;
				else if (GetRandomControl() < 0x4000)
					item->goal_anim_state = LIZMAN_WALK;
				else item->goal_anim_state = LIZMAN_WAIT;
			}
			else if (info.bite && info.distance < LIZMAN_ATTACK1_RANGE)
				item->goal_anim_state = LIZMAN_AIM1;
			else if (Targetable(item, &info) && info.bite && info.distance < LIZMAN_ATTACK0_RANGE && (lara.poisoned < 0x100 || (boxes[lizman->enemy->box_number].overlap_index & BLOCKABLE)))
				item->goal_anim_state = LIZMAN_AIM0;
			else item->goal_anim_state = LIZMAN_RUN;

			break;
		}
		case LIZMAN_WAIT:
		{
			lizman->maximum_turn = 0;

			if (info.ahead)
				neck = info.angle;

			if (lizman->mood != BORED_MOOD)
				item->goal_anim_state = LIZMAN_STOP;
			else if (GetRandomControl() < LIZMAN_WALK_CHANCE)
			{
				item->required_anim_state = LIZMAN_WALK;
				item->goal_anim_state = LIZMAN_STOP;
			}

			break;
		}
		case LIZMAN_WALK:
		{
			if (info.ahead)
				neck = info.angle;

			if (item->anim_number == objects[LIZARD_MAN].anim_index + LIZMAN_SLIDE1_ANIM ||
				item->anim_number == objects[LIZARD_MAN].anim_index + LIZMAN_SLIDE2_ANIM)
				lizman->maximum_turn = 0;
			else lizman->maximum_turn = LIZMAN_WALK_TURN;

			if (lizman->mood == ESCAPE_MOOD)
				item->goal_anim_state = LIZMAN_RUN;
			else if (lizman->mood == BORED_MOOD)
			{
				if (GetRandomControl() < LIZMAN_WAIT_CHANCE)
				{
					item->required_anim_state = LIZMAN_WAIT;
					item->goal_anim_state = LIZMAN_STOP;
				}
			}
			else if (info.bite && info.distance < LIZMAN_ATTACK1_RANGE)
				item->goal_anim_state = LIZMAN_STOP;
			else if (info.bite && info.distance < LIZMAN_ATTACK2_RANGE)
				item->goal_anim_state = LIZMAN_AIM2;
			else if (Targetable(item, &info) && info.distance < LIZMAN_ATTACK0_RANGE && (lara.poisoned < 0x100 || (boxes[lizman->enemy->box_number].overlap_index & BLOCKABLE)))
				item->goal_anim_state = LIZMAN_STOP;
			else if (info.distance > LIZMAN_WALK_RANGE)
				item->goal_anim_state = LIZMAN_RUN;

			break;
		}
		case LIZMAN_RUN:
		{
			lizman->maximum_turn = LIZMAN_RUN_TURN;

			tilt = angle / 2;

			if (info.ahead)
				neck = info.angle;

			if (lizman->mood == ESCAPE_MOOD)
				break;
			else if (lizman->mood == BORED_MOOD)
				item->goal_anim_state = LIZMAN_WALK;
			else if (info.bite && info.distance < LIZMAN_ATTACK1_RANGE)
				item->goal_anim_state = LIZMAN_STOP;
			else if (Targetable(item, &info) && info.distance < LIZMAN_ATTACK0_RANGE && (lara.poisoned < 0x100 || (boxes[lizman->enemy->box_number].overlap_index & BLOCKABLE)))
				item->goal_anim_state = LIZMAN_STOP;
			else if (info.ahead && info.distance < LIZMAN_WALK_RANGE)
				item->goal_anim_state = LIZMAN_WALK;

			break;
		}
		case LIZMAN_AIM0:
		{
			lizman->maximum_turn = 0;
			lizman->flags = 0;

			if (info.ahead)
				neck = info.angle;

			if (abs(info.angle) < LIZMAN_RUN_TURN)
				item->pos.y_rot += info.angle;
			else if (info.angle < 0)
				item->pos.y_rot -= LIZMAN_RUN_TURN;
			else item->pos.y_rot += LIZMAN_RUN_TURN;

			if (info.bite && info.distance < LIZMAN_ATTACK0_RANGE && (lara.poisoned < 0x100 || (boxes[lizman->enemy->box_number].overlap_index & BLOCKABLE)))
				item->goal_anim_state = LIZMAN_PUNCH0;
			else item->goal_anim_state = LIZMAN_STOP;

			break;
		}
		case LIZMAN_AIM1:
		{
			if (info.ahead)
				neck = info.angle;

			lizman->maximum_turn = LIZMAN_WALK_TURN;
			lizman->flags = 0;

			item->goal_anim_state = (info.ahead && info.distance < LIZMAN_ATTACK1_RANGE ? LIZMAN_PUNCH1 : LIZMAN_STOP);
			
			break;
		}
		case LIZMAN_AIM2:
		{
			if (info.ahead)
				neck = info.angle;

			lizman->maximum_turn = LIZMAN_WALK_TURN;
			lizman->flags = 0;

			item->goal_anim_state = (info.bite && info.distance < LIZMAN_ATTACK2_RANGE ? LIZMAN_PUNCH2 : LIZMAN_WALK);

			break;
		}
		case LIZMAN_PUNCH0:
		{
			if (info.ahead)
				neck = info.angle;

			if (abs(info.angle) < LIZMAN_RUN_TURN)
				item->pos.y_rot += info.angle;
			else if (info.angle < 0)
				item->pos.y_rot -= LIZMAN_RUN_TURN;
			else item->pos.y_rot += LIZMAN_RUN_TURN;

			if (item->frame_number >= anims[item->anim_number].frame_base + 7 &&
				item->frame_number <= anims[item->anim_number].frame_base + 28)
			{
				if (lizman->flags < 24)
					lizman->flags += 2;

				if (lizman->flags < 24)
					TriggerLizmanGasThrower(item, &lizman_gas, lizman->flags);
				else TriggerLizmanGasThrower(item, &lizman_gas, (GetRandomControl() & 15) + 8);
			}

			if (item->frame_number > anims[item->anim_number].frame_base + 28)
				lizman->flags = 0;

			break;
		}
		case LIZMAN_PUNCH1:
		{
			if (info.ahead)
				neck = info.angle;

			if (!lizman->flags && (item->touch_bits & LIZMAN_SWIPE_TOUCH))
			{
				lara_item->hit_points -= LIZMAN_SWIPE_DAMAGE;
				lara_item->hit_status = 1;

				CreatureEffect(item, &lizman_swipe_hit, DoBloodSplat);

				lizman->flags = 1;
			}

			if (info.distance < LIZMAN_ATTACK2_RANGE)
				item->goal_anim_state = LIZMAN_PUNCH2;

			break;
		}
		case LIZMAN_PUNCH2:
		{
			if (info.ahead)
				neck = info.angle;

			if (lizman->flags != 2 && (item->touch_bits & LIZMAN_BITE_TOUCH))
			{
				lara_item->hit_points -= LIZMAN_BITE_DAMAGE;
				lara_item->hit_status = 1;

				CreatureEffect(item, &lizman_bite_hit, DoBloodSplat);

				lizman->flags = 2;
			}
		}
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, head);
	CreatureJoint(item, 1, neck);

	if (item->current_anim_state < LIZMAN_DEATH)
	{
		switch (CreatureVault(item_number, angle, 2, LIZMAN_VAULT_SHIFT))
		{
		case 2:
			lizman->maximum_turn = 0;
			item->anim_number = objects[LIZARD_MAN].anim_index + LIZMAN_CLIMB1_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = LIZMAN_CLIMB1;
			break;
		case 3:
			lizman->maximum_turn = 0;
			item->anim_number = objects[LIZARD_MAN].anim_index + LIZMAN_CLIMB2_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = LIZMAN_CLIMB2;
			break;
		case 4:
			lizman->maximum_turn = 0;
			item->anim_number = objects[LIZARD_MAN].anim_index + LIZMAN_CLIMB3_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = LIZMAN_CLIMB3;
			break;
		case -4:
			lizman->maximum_turn = 0;
			item->anim_number = objects[LIZARD_MAN].anim_index + LIZMAN_FALL3_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = LIZMAN_FALL3;
			break;
		}
	}
	else CreatureAnimation(item_number, angle, 0);
}

int16_t TriggerLizmanGasThrower(ITEM_INFO* item, BITE_INFO* bite, int16_t speed)
{
	auto fx_number = CreateEffect(item->room_number);

	if (fx_number != NO_ITEM)
	{
		auto fx = &effects[fx_number];

		PHD_VECTOR pos1 { bite->x, bite->y,  bite->z },
				   pos2 { bite->x, bite->y << 3, bite->z << 2 };

		GetJointAbsPosition(item, &pos1, bite->mesh_num);
		GetJointAbsPosition(item, &pos2, bite->mesh_num);

		const auto angles = phd_GetVectorAngles({ pos2.x - pos1.x,pos2.y - pos1.y,pos2.z - pos1.z });

		fx->pos.x_pos = pos1.x;
		fx->pos.y_pos = pos1.y;
		fx->pos.z_pos = pos1.z;
		fx->room_number = item->room_number;
		fx->pos.x_rot = angles.y;
		fx->pos.z_rot = 0;
		fx->pos.y_rot = angles.x;
		fx->speed = speed << 2;
		fx->object_number = DRAGON_FIRE;
		fx->counter = 20;
		fx->flag1 = 1;

		TriggerLizmanGas(0, 0, 0, 0, 0, 0, fx_number);

		for (int i = 0; i < 2; ++i)
		{
			int spd = (GetRandomControl() % (speed << 2)) + 32,
				vel = (spd * phd_cos(fx->pos.x_rot)) >> W2V_SHIFT,
				zv = (vel * phd_cos(fx->pos.y_rot)) >> W2V_SHIFT,
				xv = (vel * phd_sin(fx->pos.y_rot)) >> W2V_SHIFT,
				yv = -((spd * phd_sin(fx->pos.x_rot)) >> W2V_SHIFT);

			TriggerLizmanGas(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, xv << 5, yv << 5, zv << 5, -1);
		}

		int vel = ((speed << 1) * phd_cos(fx->pos.x_rot)) >> W2V_SHIFT,
			zv = (vel * phd_cos(fx->pos.y_rot)) >> W2V_SHIFT,
			xv = (vel * phd_sin(fx->pos.y_rot)) >> W2V_SHIFT,
			yv = -(((speed << 1) * phd_sin(fx->pos.x_rot)) >> W2V_SHIFT);

		TriggerLizmanGas(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, xv << 5, yv << 5, zv << 5, -2);
	}

	return fx_number;
}

void TriggerLizmanGas(long x, long y, long z, long xv, long yv, long zv, long fxnum)
{
	auto sptr = &spark[GetFreeSpark()];

	int size = (GetRandomControl() & 31) + 48;

	sptr->On = 1;
	sptr->sR = 0;
	sptr->sG = 128 + (GetRandomControl() & 63);
	sptr->sB = 32;
	sptr->dR = 0;
	sptr->dG = 32 + (GetRandomControl() & 15);
	sptr->dB = 0;

	if (xv || yv || zv)
	{
		sptr->ColFadeSpeed = 6;
		sptr->FadeToBlack = 2;
		sptr->sLife = sptr->Life = (GetRandomControl() & 1) + 12;
	}
	else
	{
		sptr->ColFadeSpeed = 8;
		sptr->FadeToBlack = 16;
		sptr->sLife = sptr->Life = (GetRandomControl() & 3) + 20;
	}

	sptr->TransType = COLADD;
	sptr->extras = 0;
	sptr->Dynamic = -1;
	sptr->x = x + ((GetRandomControl() & 31) - 16);
	sptr->y = y;
	sptr->z = z + ((GetRandomControl() & 31) - 16);
	sptr->Xvel = ((GetRandomControl() & 15) - 16) + xv;
	sptr->Yvel = yv;
	sptr->Zvel = ((GetRandomControl() & 15) - 16) + zv;
	sptr->Friction = 0;

	if (GetRandomControl() & 1)
	{
		sptr->Flags = (fxnum >= 0 ? (SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF | SP_FX) : (SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF));
		sptr->RotAng = GetRandomControl() & 4095;
		sptr->RotAdd = ((GetRandomControl() & 1) ? -(GetRandomControl() & 15) - 16 : (GetRandomControl() & 15) + 16);
	}
	else sptr->Flags = (fxnum >= 0 ? (SP_SCALE | SP_DEF | SP_EXPDEF | SP_FX) : (SP_SCALE | SP_DEF | SP_EXPDEF));

	sptr->FxObj = fxnum;
	sptr->Gravity = sptr->MaxYvel = 0;
	sptr->Def = uint8_t(objects[EXPLOSION1].mesh_ptr);

	if (xv || yv || zv)
	{
		sptr->sWidth = sptr->Width = size >> 5;
		sptr->sHeight = sptr->Height = size >> 5;
		sptr->Scalar = (fxnum == -2 ? 2 : 3);
	}
	else
	{
		sptr->sWidth = sptr->Width = size >> 4;
		sptr->sHeight = sptr->Height = size >> 4;
		sptr->Scalar = 4;
	}

	sptr->dWidth = size >> 1;
	sptr->dHeight = size >> 1;
}