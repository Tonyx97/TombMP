#include "objects.h"
#include "lara.h"
#include "control.h"
#include "sphere.h"
#include "effect2.h"

#include <specific/fn_stubs.h>

#define	MAX_WINGMUTE_TRIGGER_RANGE	0x4000
#define WING_DAMAGE					50
#define WING_TOUCH					(0x1000)
#define WING_TURN					(ONE_DEGREE * 3)
#define WING_ATTACK_RANGE			SQUARE(WALL_L / 2)
#define WING_TAKEOFF_RANGE			SQUARE(WALL_L * 3)
#define WING_LAND_SPEED				(WALL_L / 20)
#define WING_TAKEOFF_CHANCE			0x80
#define WING_FALL_ANIM				5
#define WING_START_ANIM				2

void TriggerWingMuteParticles(int16_t item_number);

enum wing_anims
{
	WING_HOVER,
	WING_LAND,
	WING_WAIT,
	WING_TAKEOFF,
	WING_ATTACK,
	WING_FALL,
	WING_DEATH,
	WING_MOVE
};

BITE_INFO wing_bite = { 0, 0, 0, 12 };

void InitialiseWingmute(int16_t item_number)
{
	InitialiseCreature(item_number);

	auto item = &items[item_number];

	item->anim_number = objects[MUTANT1].anim_index + WING_START_ANIM;
	item->frame_number = anims[item->anim_number].frame_base;
	item->current_anim_state = item->goal_anim_state = WING_WAIT;
	item->item_flags[1] = GetRandomControl() & 0x7F;
}

void WingmuteControl(int16_t item_number)
{
	if (!CreatureActive(item_number))
		return;

	auto item = &items[item_number];
	auto wing = (CREATURE_INFO*)item->data;

	if (!wing)
		return;

	int16_t angle = 0;

	if (item->hit_points <= 0)
	{
		switch (item->current_anim_state)
		{
		case WING_FALL:
		{
			if (item->pos.y_pos > item->floor)
			{
				item->pos.y_pos = item->floor;
				item->gravity_status = 0;
				item->fallspeed = 0;
				item->goal_anim_state = WING_DEATH;
			}

			break;
		}
		case WING_DEATH:
			item->pos.y_pos = item->floor;
			break;
		default:
		{
			item->anim_number = objects[MUTANT1].anim_index + WING_FALL_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = WING_FALL;
			item->gravity_status = 1;
			item->speed = 0;
			break;
		}
		}

		item->pos.x_rot = 0;
	}
	else
	{
		AI_INFO info;

		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, wing->maximum_turn);

		switch (item->current_anim_state)
		{
		case WING_WAIT:
		{
			item->pos.y_pos = item->floor;
			wing->maximum_turn = ONE_DEGREE;

			if (item->hit_status || info.distance < WING_TAKEOFF_RANGE || wing->hurt_by_lara || item->ai_bits == MODIFY)
				item->goal_anim_state = WING_TAKEOFF;

			break;
		}
		case WING_LAND:
		{
			item->pos.y_pos = item->pos.y_pos + WING_LAND_SPEED;

			if (item->pos.y_pos > item->floor)
				item->pos.y_pos = item->floor;
			
			break;
		}
		case WING_HOVER:
		{
			wing->flags = 0;
			wing->maximum_turn = WING_TURN;

			if (item->required_anim_state)
				item->goal_anim_state = item->required_anim_state;
			else if (item->hit_status || GetRandomControl() < WING_TAKEOFF_CHANCE * 3 || item->ai_bits == MODIFY)
				item->goal_anim_state = WING_MOVE;
			else if ((wing->mood == BORED_MOOD || GetRandomControl() < WING_TAKEOFF_CHANCE) && !wing->hurt_by_lara && !(item->ai_bits == MODIFY))
				item->goal_anim_state = WING_LAND;
			else if (info.ahead && info.distance < WING_ATTACK_RANGE)
				item->goal_anim_state = WING_ATTACK;

			break;
		}
		case WING_MOVE:
		{
			wing->flags = 0;
			wing->maximum_turn = WING_TURN;

			if (item->required_anim_state)
				item->goal_anim_state = item->required_anim_state;
			else if ((wing->mood == BORED_MOOD || GetRandomControl() < WING_TAKEOFF_CHANCE) && !wing->hurt_by_lara && !(item->ai_bits == MODIFY))
				item->goal_anim_state = WING_HOVER;
			else if (info.ahead && info.distance < WING_ATTACK_RANGE)
				item->goal_anim_state = WING_ATTACK;

			break;
		}
		case WING_ATTACK:
		{
			wing->maximum_turn = WING_TURN;

			if (info.ahead && info.distance < WING_ATTACK_RANGE)
				item->goal_anim_state = WING_ATTACK;
			else if (info.distance < WING_ATTACK_RANGE)
				item->goal_anim_state = WING_HOVER;
			else
			{
				item->goal_anim_state = WING_HOVER;
				item->required_anim_state = WING_MOVE;
			}

			if (!wing->flags && (item->touch_bits & WING_TOUCH))
			{
				lara_item->hit_points -= WING_DAMAGE;
				lara_item->hit_status = 1;

				CreatureEffect(item, &wing_bite, DoBloodSplat);

				wing->flags = 1;
			}

			break;
		}
		}
	}

	PHD_VECTOR pos {};

	GetJointAbsPosition(item, &pos, 10);
	TriggerDynamicLight(pos.x, pos.y, pos.z, 10, 0, std::clamp(abs(m_sin(item->item_flags[1] << 7, 7)), 0, 31), 0);

	++item->item_flags[1];
	item->item_flags[1] &= 63;

	TriggerWingMuteParticles(item_number);
	TriggerWingMuteParticles(item_number);

	CreatureAnimation(item_number, angle, 0);
}

void TriggerWingMuteParticles(int16_t item_number)
{
	int dx = lara_item->pos.x_pos - items[item_number].pos.x_pos,
		dz = lara_item->pos.z_pos - items[item_number].pos.z_pos;

	if (dx < -MAX_WINGMUTE_TRIGGER_RANGE || dx > MAX_WINGMUTE_TRIGGER_RANGE || dz < -MAX_WINGMUTE_TRIGGER_RANGE || dz > MAX_WINGMUTE_TRIGGER_RANGE)
		return;

	auto sptr = &spark[GetFreeSpark()];

	auto size = (GetRandomControl() & 3) + 3;

	sptr->On = 1;
	sptr->sG = (GetRandomControl() & 63) + 32;
	sptr->sB = sptr->sG >> 1;
	sptr->sR = sptr->sG >> 2;

	sptr->dG = (GetRandomControl() & 31) + 224;
	sptr->dB = sptr->dG >> 1;
	sptr->dR = sptr->dG >> 2;

	sptr->ColFadeSpeed = 4;
	sptr->FadeToBlack = 2;
	sptr->sLife = sptr->Life = 8;

	sptr->TransType = COLADD;

	sptr->extras = 0;
	sptr->Dynamic = -1;

	sptr->x = (GetRandomControl() & 15) - 8;
	sptr->y = (GetRandomControl() & 15) - 8;
	sptr->z = (GetRandomControl() & 127) - 64;

	sptr->Xvel = (GetRandomControl() & 31) - 16;
	sptr->Yvel = (GetRandomControl() & 31) - 16;
	sptr->Zvel = (GetRandomControl() & 31) - 16;
	sptr->Friction = 2 | (2 << 4);

	sptr->Flags = SP_SCALE | SP_ITEM | SP_NODEATTATCH | SP_DEF;
	sptr->Gravity = sptr->MaxYvel = 0;

	sptr->FxObj = item_number;
	sptr->NodeNumber = SPN_WINGMUTEPARTICLES;

	sptr->Def = objects[EXPLOSION1].mesh_index;
	sptr->Scalar = 3;
	sptr->Width = sptr->sWidth = size;
	sptr->Height = sptr->sHeight = size;
	sptr->dWidth = size >> 1;
	sptr->dHeight = size >> 1;
}