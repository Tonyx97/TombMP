#include "laraanim.h"
#include "laramisc.h"
#include "objects.h"
#include "lot.h"
#include "game.h"
#include "lara2gun.h"
#include "camera.h"

#define DINO_TOUCH_DAMAGE	1
#define DINO_TRAMPLE_DAMAGE 10
#define DINO_BITE_DAMAGE	10000
#define DINO_TOUCH			(0x3000)
#define DINO_ROAR_CHANCE	0x200
#define DINO_RUN_TURN		(ONE_DEGREE*4)
#define DINO_WALK_TURN		(ONE_DEGREE*2)
#define DINO_RUN_RANGE		SQUARE(WALL_L*5)
#define DINO_ATTACK_RANGE	SQUARE(WALL_L*4)
#define DINO_BITE_RANGE		SQUARE(1500)

enum dino_anims
{
	DINO_LINK,
	DINO_STOP,
	DINO_WALK,
	DINO_RUN,
	DINO_ATTACK1,
	DINO_DEATH,
	DINO_ROAR,
	DINO_ATTACK2,
	DINO_KILL
};

void DinoControl(int16_t item_number)
{
	auto item = &items[item_number];

	if (item->status == INVISIBLE)
	{
		if (!EnableBaddieAI(item_number, 0))
			return;

		item->status = ACTIVE;
	}

	auto dino = (CREATURE_INFO*)item->data;
	
	int16_t angle = 0;

	if (item->hit_points <= 0)
		item->goal_anim_state = (item->current_anim_state == DINO_STOP ? DINO_DEATH : DINO_STOP);
	else
	{
		if (!dino)
			return;

		AI_INFO info;

		CreatureAIInfo(item, &info);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, dino->maximum_turn);

		if (item->touch_bits)
			lara_item->hit_points -= (item->current_anim_state == DINO_RUN) ? DINO_TRAMPLE_DAMAGE : DINO_TOUCH_DAMAGE;

		dino->flags = (dino->mood != ESCAPE_MOOD && !info.ahead && info.enemy_facing > -FRONT_ARC && info.enemy_facing < FRONT_ARC);

		if (!dino->flags && info.distance > DINO_BITE_RANGE && info.distance < DINO_ATTACK_RANGE && info.bite)
			dino->flags = 1;

		switch (item->current_anim_state)
		{
		case DINO_STOP:
		{
			if (item->required_anim_state)
				item->goal_anim_state = item->required_anim_state;
			else if (info.distance < DINO_BITE_RANGE && info.bite)
				item->goal_anim_state = DINO_ATTACK2;
			else if (dino->mood == BORED_MOOD || dino->flags)
				item->goal_anim_state = DINO_WALK;
			else item->goal_anim_state = DINO_RUN;

			break;
		}
		case DINO_WALK:
		{
			dino->maximum_turn = DINO_WALK_TURN;

			if (dino->mood != BORED_MOOD || !dino->flags)
				item->goal_anim_state = DINO_STOP;
			else if (info.ahead && GetRandomControl() < DINO_ROAR_CHANCE)
			{
				item->required_anim_state = DINO_ROAR;
				item->goal_anim_state = DINO_STOP;
			}

			break;
		}
		case DINO_RUN:
		{
			dino->maximum_turn = DINO_RUN_TURN;

			if (info.distance < DINO_RUN_RANGE && info.bite)
				item->goal_anim_state = DINO_STOP;
			else if (dino->flags)
				item->goal_anim_state = DINO_STOP;
			else if (dino->mood != ESCAPE_MOOD && info.ahead && GetRandomControl() < DINO_ROAR_CHANCE)
			{
				item->required_anim_state = DINO_ROAR;
				item->goal_anim_state = DINO_STOP;
			}
			else if (dino->mood == BORED_MOOD)
				item->goal_anim_state = DINO_STOP;

			break;
		}
		case DINO_ATTACK2:
		{
			if (item->touch_bits & DINO_TOUCH)
			{
				lara_item->hit_points -= DINO_BITE_DAMAGE;
				lara_item->hit_status = 1;
				
				item->goal_anim_state = DINO_KILL;

				if (lara_item == lara_item)
					LaraDinoDeath(item);
			}

			item->required_anim_state = DINO_WALK;
		}
		}
	}

	CreatureAnimation(item_number, angle, 0);

	item->collidable = 1;
}

void LaraDinoDeath(ITEM_INFO* item)
{
	item->goal_anim_state = DINO_KILL;

	auto litem = lara_item;

	if (litem->room_number != item->room_number)
		ItemNewRoom(lara.item_number, item->room_number);

	litem->pos.x_pos = item->pos.x_pos;
	litem->pos.y_pos = item->pos.y_pos;
	litem->pos.z_pos = item->pos.z_pos;
	litem->pos.y_rot = item->pos.y_rot;
	litem->pos.x_rot = litem->pos.z_rot = 0;
	litem->gravity_status = 0;
	litem->anim_number = objects[LARA_EXTRA].anim_index + 1;
	litem->frame_number = anims[litem->anim_number].frame_base;
	litem->current_anim_state = AS_SPECIAL;
	litem->goal_anim_state = AS_SPECIAL;

	LaraSwapMeshExtra();

	litem->hit_points = -1;
	lara.air = -1;
	lara.gun_status = LG_HANDSBUSY;
	lara.gun_type = LG_UNARMED;

	camera.flags = FOLLOW_CENTRE;
	camera.target_angle = 170 * ONE_DEGREE;
	camera.target_elevation = -25 * ONE_DEGREE;
}