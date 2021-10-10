#include <specific/standard.h>
#include <specific/stypes.h>

#include "objects.h"
#include "lara.h"
#include "people.h"
#include "control.h"
#include "effect2.h"
#include "game.h"

void TriggerFenceSparks(long x, long y, long z, long kill);

#define CIVVY_HIT_DAMAGE		40
#define CIVVY_SWIPE_DAMAGE		50
#define CIVVY_WALK_TURN			(ONE_DEGREE * 5)
#define CIVVY_RUN_TURN			(ONE_DEGREE * 6)
#define CIVVY_ATTACK0_RANGE		SQUARE(WALL_L / 3)
#define CIVVY_ATTACK1_RANGE		SQUARE(WALL_L * 2 / 3)
#define CIVVY_ATTACK2_RANGE		SQUARE(WALL_L)
#define CIVVY_WALK_RANGE		SQUARE(WALL_L)
#define CIVVY_ESCAPE_RANGE		SQUARE(WALL_L * 3)
#define CIVVY_WALK_CHANCE		0x100
#define CIVVY_WAIT_CHANCE		0x100
#define CIVVY_DIE_ANIM			26
#define CIVVY_STOP_ANIM			6
#define CIVVY_CLIMB1_ANIM		28
#define CIVVY_CLIMB2_ANIM		29
#define CIVVY_CLIMB3_ANIM		27
#define CIVVY_FALL3_ANIM		30
#define CIVVY_TOUCH				0x2400
#define CIVVY_VAULT_SHIFT		260
#define CIVVY_AWARE_DISTANCE	SQUARE(WALL_L)
#define FENCE_WIDTH				128
#define FENCE_LENGTH			1024 + 32

enum civvy_anims
{
	CIVVY_EMPTY,
	CIVVY_STOP,
	CIVVY_WALK,
	CIVVY_PUNCH2,
	CIVVY_AIM2,
	CIVVY_WAIT,
	CIVVY_AIM1,
	CIVVY_AIM0,
	CIVVY_PUNCH1,
	CIVVY_PUNCH0,
	CIVVY_RUN,
	CIVVY_DEATH,
	CIVVY_CLIMB3,
	CIVVY_CLIMB1,
	CIVVY_CLIMB2,
	CIVVY_FALL3
};

BITE_INFO civvy_hit = { 0, 0, 0, 13 };

void InitialiseCivvy(int16_t item_number)
{
	InitialiseCreature(item_number);

	auto item = &items[item_number];

	item->anim_number = objects[CIVVIE].anim_index + CIVVY_STOP_ANIM;
	item->frame_number = anims[item->anim_number].frame_base;
	item->current_anim_state = item->goal_anim_state = CIVVY_STOP;
}

void CivvyControl(int16_t item_number)
{
	if (!CreatureActive(item_number))
		return;

	auto item = &items[item_number];
	auto civvy = (CREATURE_INFO*)item->data;

	int torso_y = 0,
		torso_x = 0,
		head = 0,
		angle = 0,
		tilt = 0;

	if (boxes[item->box_number].overlap_index & BLOCKED)
	{
		DoLotsOfBloodD(item->pos.x_pos, item->pos.y_pos - (GetRandomControl() & 255) - 32, item->pos.z_pos, (GetRandomControl() & 127) + 128, (PHD_ANGLE)GetRandomControl() << 1, item->room_number, 3);
		item->hit_points -= 20;
	}

	if (item->hit_points <= 0)
	{
		if (item->current_anim_state != CIVVY_DEATH)
		{
			item->anim_number = objects[CIVVIE].anim_index + CIVVY_DIE_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = CIVVY_DEATH;
			civvy->LOT.step = STEP_L;
		}
	}
	else
	{
		if (item->ai_bits)
			GetAITarget(civvy);
		else civvy->enemy = lara_item;

		AI_INFO info,
				lara_info;

		CreatureAIInfo(item, &info);

		if (civvy->enemy == lara_item)
		{
			lara_info.angle = info.angle;
			lara_info.distance = info.distance;
		}
		else
		{
			int lara_dz = lara_item->pos.z_pos - item->pos.z_pos,
				lara_dx = lara_item->pos.x_pos - item->pos.x_pos;

			lara_info.angle = phd_atan(lara_dz, lara_dx) - item->pos.y_rot;
			lara_info.distance = lara_dz * lara_dz + lara_dx * lara_dx;
		}

		GetCreatureMood(item, &info, VIOLENT);

		if (civvy->enemy == lara_item && info.distance > CIVVY_ESCAPE_RANGE && info.enemy_facing < 0x3000 && info.enemy_facing > -0x3000)
			civvy->mood = ESCAPE_MOOD;

		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, civvy->maximum_turn);

		auto real_enemy = civvy->enemy;

		civvy->enemy = lara_item;

		if ((lara_info.distance < CIVVY_AWARE_DISTANCE || item->hit_status || TargetVisible(item, &lara_info)) && !(item->ai_bits & FOLLOW))
		{
			if (!civvy->alerted)
				g_audio->play_sound(300, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

			AlertAllGuards(item_number);
		}

		civvy->enemy = real_enemy;

		switch (item->current_anim_state)
		{
		case CIVVY_WAIT:
		{
			if (civvy->alerted || item->goal_anim_state == CIVVY_RUN)
			{
				item->goal_anim_state = CIVVY_STOP;
				break;
			}
		}
		case CIVVY_STOP:
		{
			civvy->flags = 0;
			civvy->maximum_turn = 0;

			head = lara_info.angle;

			if (item->ai_bits & GUARD)
			{
				head = AIGuard(civvy);

				if (!(GetRandomControl() & 0xFF))
					item->goal_anim_state = (item->current_anim_state == CIVVY_STOP ? CIVVY_WAIT : CIVVY_STOP);
			}
			else if (item->ai_bits & PATROL1)
				item->goal_anim_state = CIVVY_WALK;
			else if (civvy->mood == ESCAPE_MOOD)
				item->goal_anim_state = (lara.target != item && info.ahead ? CIVVY_STOP : CIVVY_RUN);
			else if (civvy->mood == BORED_MOOD || ((item->ai_bits & FOLLOW) && (civvy->reached_goal || lara_info.distance > SQUARE(WALL_L * 2))))
			{
				if (item->required_anim_state)
					item->goal_anim_state = item->required_anim_state;
				else if (info.ahead)
					item->goal_anim_state = CIVVY_STOP;
				else item->goal_anim_state = CIVVY_RUN;
			}
			else if (info.bite && info.distance < CIVVY_ATTACK0_RANGE)
				item->goal_anim_state = CIVVY_AIM0;
			else if (info.bite && info.distance < CIVVY_ATTACK1_RANGE)
				item->goal_anim_state = CIVVY_AIM1;
			else if (info.bite && info.distance < CIVVY_WALK_RANGE)
				item->goal_anim_state = CIVVY_WALK;
			else item->goal_anim_state = CIVVY_RUN;

			break;
		}
		case CIVVY_WALK:
		{
			head = lara_info.angle;

			civvy->maximum_turn = CIVVY_WALK_TURN;

			if (item->ai_bits & PATROL1)
			{
				item->goal_anim_state = CIVVY_WALK;
				head = 0;
			}
			else if (civvy->mood == ESCAPE_MOOD)
				item->goal_anim_state = CIVVY_RUN;
			else if (civvy->mood == BORED_MOOD)
			{
				if (GetRandomControl() < CIVVY_WAIT_CHANCE)
				{
					item->required_anim_state = CIVVY_WAIT;
					item->goal_anim_state = CIVVY_STOP;
				}
			}
			else if (info.bite && info.distance < CIVVY_ATTACK0_RANGE)
				item->goal_anim_state = CIVVY_STOP;
			else if (info.bite && info.distance < CIVVY_ATTACK2_RANGE)
				item->goal_anim_state = CIVVY_AIM2;
			else item->goal_anim_state = CIVVY_RUN;

			break;
		}
		case CIVVY_RUN:
		{
			if (info.ahead)
				head = info.angle;

			civvy->maximum_turn = CIVVY_RUN_TURN;
			tilt = angle / 2;

			if (item->ai_bits & GUARD)
				item->goal_anim_state = CIVVY_WAIT;
			else if (civvy->mood == ESCAPE_MOOD)
			{
				if (lara.target != item && info.ahead)
					item->goal_anim_state = CIVVY_STOP;
			}
			else if ((item->ai_bits & FOLLOW) && (civvy->reached_goal || lara_info.distance > SQUARE(WALL_L * 2)))
				item->goal_anim_state = CIVVY_STOP;
			else if (civvy->mood == BORED_MOOD)
				item->goal_anim_state = CIVVY_WALK;
			else if (info.ahead && info.distance < CIVVY_WALK_RANGE)
				item->goal_anim_state = CIVVY_WALK;

			break;
		}
		case CIVVY_AIM0:
		{
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			civvy->maximum_turn = CIVVY_WALK_TURN;
			civvy->flags = 0;

			item->goal_anim_state = (info.bite && info.distance < CIVVY_ATTACK0_RANGE ? CIVVY_PUNCH0 : CIVVY_STOP);

			break;
		}
		case CIVVY_AIM1:
		{
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			civvy->maximum_turn = CIVVY_WALK_TURN;
			civvy->flags = 0;

			item->goal_anim_state = (info.ahead && info.distance < CIVVY_ATTACK1_RANGE ? CIVVY_PUNCH1 : CIVVY_STOP);

			break;
		}
		case CIVVY_AIM2:
		{
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			civvy->maximum_turn = CIVVY_WALK_TURN;
			civvy->flags = 0;

			item->goal_anim_state = (info.bite && info.distance < CIVVY_ATTACK2_RANGE ? CIVVY_PUNCH2 : CIVVY_WALK);

			break;
		}
		case CIVVY_PUNCH0:
		{
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			civvy->maximum_turn = CIVVY_WALK_TURN;

			if (!civvy->flags && (item->touch_bits & CIVVY_TOUCH))
			{
				lara_item->hit_points -= CIVVY_HIT_DAMAGE;
				lara_item->hit_status = 1;

				CreatureEffect(item, &civvy_hit, DoBloodSplat);
				g_audio->play_sound(70, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

				civvy->flags = 1;
			}

			break;
		}
		case CIVVY_PUNCH1:
		{
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			civvy->maximum_turn = CIVVY_WALK_TURN;

			if (!civvy->flags && (item->touch_bits & CIVVY_TOUCH))
			{
				lara_item->hit_points -= CIVVY_HIT_DAMAGE;
				lara_item->hit_status = 1;

				CreatureEffect(item, &civvy_hit, DoBloodSplat);
				g_audio->play_sound(70, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

				civvy->flags = 1;
			}

			if (info.ahead && info.distance > CIVVY_ATTACK1_RANGE && info.distance < CIVVY_ATTACK2_RANGE)
				item->goal_anim_state = CIVVY_PUNCH2;

			break;
		}
		case CIVVY_PUNCH2:
		{
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			civvy->maximum_turn = CIVVY_WALK_TURN;

			if (civvy->flags != 2 && (item->touch_bits & CIVVY_TOUCH))
			{
				lara_item->hit_points -= CIVVY_SWIPE_DAMAGE;
				lara_item->hit_status = 1;

				CreatureEffect(item, &civvy_hit, DoBloodSplat);
				g_audio->play_sound(70, { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos });

				civvy->flags = 2;
			}

			break;
		}
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso_y);
	CreatureJoint(item, 1, torso_x);
	CreatureJoint(item, 2, head);

	if (item->current_anim_state < CIVVY_DEATH)
	{
		switch (CreatureVault(item_number, angle, 2, CIVVY_VAULT_SHIFT))
		{
		case 2:
			civvy->maximum_turn = 0;
			item->anim_number = objects[CIVVIE].anim_index + CIVVY_CLIMB1_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = CIVVY_CLIMB1;
			break;
		case 3:
			civvy->maximum_turn = 0;
			item->anim_number = objects[CIVVIE].anim_index + CIVVY_CLIMB2_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = CIVVY_CLIMB2;
			break;
		case 4:
			civvy->maximum_turn = 0;
			item->anim_number = objects[CIVVIE].anim_index + CIVVY_CLIMB3_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = CIVVY_CLIMB3;
			break;
		case -4:
			civvy->maximum_turn = 0;
			item->anim_number = objects[CIVVIE].anim_index + CIVVY_FALL3_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = CIVVY_FALL3;
			break;
		}
	}
	else
	{
		civvy->maximum_turn = 0;
		CreatureAnimation(item_number, angle, 0);
	}
}

void ControlElectricFence(int16_t item_number)
{
	auto item = &items[item_number];

	if (!TriggerActive(item))
		return;

	int dx = lara_item->pos.x_pos - item->pos.x_pos,
		dz = lara_item->pos.z_pos - item->pos.z_pos;

	if (dx < -0x5000 || dx > 0x5000 || dz < -0x5000 || dz > 0x5000)
		return;

	int x, z, xsize, zsize,
		tx, tz, xand, zand;

	switch (item->pos.y_rot)
	{
	case 0:
	{
		x = item->pos.x_pos + 512;
		z = item->pos.z_pos + 512;
		tx = x - FENCE_LENGTH;
		tz = z - 256;
		xand = 2047;
		zand = 0;
		xsize = FENCE_LENGTH;
		zsize = FENCE_WIDTH;

		break;
	}
	case 16384:
	{
		x = item->pos.x_pos + 512;
		z = item->pos.z_pos - 512;
		tx = x - 256;
		tz = z - FENCE_LENGTH;
		xand = 0;
		zand = 2047;
		xsize = FENCE_WIDTH;
		zsize = FENCE_LENGTH;

		break;
	}
	case -32768:
	{
		x = item->pos.x_pos - 512;
		z = item->pos.z_pos - 512;
		tx = x - FENCE_LENGTH;
		tz = z + 256;
		xand = 2047;
		zand = 0;
		xsize = FENCE_LENGTH;
		zsize = FENCE_WIDTH;

		break;
	}
	case -16384:
	{
		x = item->pos.x_pos - 512;
		z = item->pos.z_pos + 512;
		tx = x + 256;
		tz = z - FENCE_LENGTH;
		xand = 0;
		zand = 2047;
		xsize = FENCE_WIDTH;
		zsize = FENCE_LENGTH;

		break;
	}
	default:
		x = z = xsize = zsize = tx = tz = xand = zand = 0;
		break;
	}

	if ((GetRandomControl() & 63) == 0)
	{
		if (xand)
			tx += (GetRandomControl() & xand);
		else tz += (GetRandomControl() & zand);

		int ty = item->pos.y_pos - (GetRandomControl() & 0x1F);

		for (int i = 0; i < (GetRandomControl() & 3) + 3; ++i)
		{
			TriggerFenceSparks(tx, ty, tz, 0);

			if (xand)
				tx += ((GetRandomControl() & xand) & 7) - 4;
			else tz += ((GetRandomControl() & zand) & 7) - 4;

			ty += (GetRandomControl() & 7) - 4;
		}
	}

	if (lara.electric ||
		lara_item->pos.x_pos < x - xsize ||
		lara_item->pos.x_pos > x + xsize ||
		lara_item->pos.z_pos < z - zsize ||
		lara_item->pos.z_pos > z + zsize ||
		lara_item->pos.y_pos > item->pos.y_pos + 32 ||
		lara_item->pos.y_pos < item->pos.y_pos - 3072)
		return;

	int sx = tx,
		sz = tz;

	int cnt = (GetRandomControl() & 15) + 3;

	for (int i = 0; i < cnt; ++i)
	{
		if (xand)
			tx = lara_item->pos.x_pos + (GetRandomControl() & 511) - 256;
		else tz = lara_item->pos.z_pos + (GetRandomControl() & 511) - 256;

		int ty = lara_item->pos.y_pos - (GetRandomControl() % 768),
			cnt2 = (GetRandomControl() & 3) + 6;

		for (int lp2 = 0; lp2 < cnt2; ++lp2)
		{
			TriggerFenceSparks(tx, ty, tz, 1);

			if (xand)
				tx += ((GetRandomControl() & xand) & 7) - 4;
			else tz += ((GetRandomControl() & zand) & 7) - 4;

			ty += (GetRandomControl() & 7) - 4;
		}

		tx = sx;
		tz = sz;
	}

	lara.electric = 1;
	lara_item->hit_points = 0;
}

void TriggerFenceSparks(long x, long y, long z, long kill)
{
	auto sptr = &spark[GetFreeSpark()];

	sptr->On = 1;
	sptr->sB = (GetRandomControl() & 63) + 192;
	sptr->sR = sptr->sB;
	sptr->sG = sptr->sB;
	sptr->dB = (GetRandomControl() & 63) + 192;
	sptr->dR = sptr->sB >> 2;
	sptr->dG = sptr->sB >> 1;
	sptr->ColFadeSpeed = 8;
	sptr->FadeToBlack = 16;
	sptr->sLife = sptr->Life = 32 + (GetRandomControl() & 7);
	sptr->TransType = COLADD;
	sptr->Dynamic = -1;
	sptr->x = x;
	sptr->y = y;
	sptr->z = z;
	sptr->Xvel = ((GetRandomControl() & 255) - 128) << 1;
	sptr->Yvel = (short)((GetRandomControl() & 15) - 8 - (kill << 5));
	sptr->Zvel = ((GetRandomControl() & 255) - 128) << 1;
	sptr->Friction = 4;
	sptr->Flags = SP_SCALE;
	sptr->Scalar = (uint8_t)(1 + kill);
	sptr->Width = sptr->sWidth = (GetRandomControl() & 3) + 4;
	sptr->dWidth = sptr->sWidth = 1;
	sptr->Height = sptr->sHeight = sptr->Width;
	sptr->dHeight = sptr->dWidth;
	sptr->Gravity = 16 + (GetRandomControl() & 15);
	sptr->MaxYvel = 0;
}