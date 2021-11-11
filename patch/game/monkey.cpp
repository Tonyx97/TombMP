#include <specific/stypes.h>
#include <specific/output.h>
#include <specific/fn_stubs.h>

#include "objects.h"
#include "lara.h"
#include "control.h"
#include "lot.h"

void InitialiseMonkey(int16_t item_number);
void MonkeyControl(int16_t item_number);
void DrawMonkey(ITEM_INFO* item);

#define MONKEY_HIT_DAMAGE		40
#define MONKEY_SWIPE_DAMAGE		50
#define MONKEY_WALK_TURN		(ONE_DEGREE * 7)
#define MONKEY_RUN_TURN			(ONE_DEGREE * 11)
#define MONKEY_ATTACK_RANGE		SQUARE(WALL_L / 3)
#define MONKEY_JUMP_RANGE		SQUARE(WALL_L * 2 / 3)
#define MONKEY_WALK_RANGE		SQUARE(WALL_L * 2 / 3)
#define MONKEY_ROLL_RANGE		SQUARE(WALL_L)
#define MONKEY_WALK_CHANCE		0x100
#define MONKEY_WAIT_CHANCE		0x100
#define MONKEY_DIE_ANIM			14
#define MONKEY_SIT_ANIM			2
#define MONKEY_CLIMB2_ANIM		19
#define MONKEY_CLIMB3_ANIM		18
#define MONKEY_CLIMB4_ANIM		17
#define MONKEY_DOWN2_ANIM		22
#define MONKEY_DOWN3_ANIM		21
#define MONKEY_DOWN4_ANIM		20
#define MONKEY_TOUCH			0x2400
#define MONKEY_VAULT_SHIFT		128
#define MONKEY_AWARE_DISTANCE	SQUARE(WALL_L)
#define MONKEY_HIT_RADIUS		(STEP_L)

BITE_INFO monkey_hit = { 10, 10, 11, 13 };

enum monkey_anims
{
	MONKEY_EMPTY,
	MONKEY_STOP,
	MONKEY_WALK,
	MONKEY_STAND,
	MONKEY_RUN,
	MONKEY_PICKUP,
	MONKEY_SIT,
	MONKEY_EAT,
	MONKEY_SCRATCH,
	MONKEY_ROLL,
	MONKEY_ANGRY,
	MONKEY_DEATH,
	MONKEY_ATAK_LOW,
	MONKEY_ATAK_HIGH,
	MONKEY_ATAK_JUMP,
	MONKEY_CLIMB4,
	MONKEY_CLIMB3,
	MONKEY_CLIMB2,
	MONKEY_DOWN4,
	MONKEY_DOWN3,
	MONKEY_DOWN2
};

void InitialiseMonkey(int16_t item_number)
{
	auto item = &items[item_number];

	InitialiseCreature(item_number);

	item->anim_number = objects[MONKEY].anim_index + MONKEY_SIT_ANIM;
	item->frame_number = anims[item->anim_number].frame_base;
	item->current_anim_state = item->goal_anim_state = MONKEY_SIT;
}

void DrawMonkey(ITEM_INFO* item)
{
	int16_t* frmptr[2] = { nullptr, nullptr };
	int rate;

	int frac = GetFrames(item, frmptr, &rate);

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

		auto extra_rotation = (int16_t*)item->data;

		auto meshpp = objects[MONKEY].mesh_ptr,
			 swappp = objects[item->ai_bits != MODIFY ? MESHSWAP2 : MESHSWAP3].mesh_ptr;

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

				phd_PutPolygons(bit & item->mesh_bits ? *meshpp : *swappp, clip);
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

			auto rotation1 = frmptr[0] + 9;
			auto rotation2 = frmptr[1] + 9;

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

void MonkeyControl(int16_t item_number)
{
	if (!CreatureActive(item_number))
		return;

	auto item = &items[item_number];
	auto monkey = (CREATURE_INFO*)item->data;

	if (!monkey)
		return;
	
	int16_t torso_y = 0,
		    torso_x = 0,
		    head = 0,
		    angle = 0,
		    tilt = 0;

	if (item->hit_points <= 0)
	{
		if (item->current_anim_state != MONKEY_DEATH)
		{
			item->mesh_bits = 0xffffffff;
			item->anim_number = objects[MONKEY].anim_index + MONKEY_DIE_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = MONKEY_DEATH;
		}
	}
	else
	{
		GetAITarget(monkey);

		if (monkey->hurt_by_lara && !monkey->enemy)
			monkey->enemy = lara_item;
		
		if (item->ai_bits != MODIFY)
			item->mesh_bits = (item->carried_item != NO_ITEM ? 0xfffffeff : 0xffffffff);
		else item->mesh_bits = (item->carried_item != NO_ITEM ? 0xffff6e6f : 0xffff6f6f);

		AI_INFO info,
				lara_info;

		CreatureAIInfo(item, &info);

		if (monkey->enemy == lara_item)
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

		if (lara.skidoo != NO_ITEM)
			monkey->mood = ESCAPE_MOOD;

		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, monkey->maximum_turn);

		auto enemy = monkey->enemy;

		monkey->enemy = lara_item;

		if (item->hit_status)
			AlertAllGuards(item_number);

		monkey->enemy = enemy;

		enemy = monkey->enemy;

		switch (item->current_anim_state)
		{
		case MONKEY_SIT:
		{
			monkey->flags = 0;
			monkey->maximum_turn = 0;

			head = lara_info.angle;

			if (item->ai_bits & GUARD)
			{
				head = AIGuard(monkey);

				if (!(GetRandomControl() & 0xF))
					item->goal_anim_state = ((GetRandomControl() & 0x1) ? MONKEY_SCRATCH : MONKEY_EAT);
			}
			else if (item->ai_bits & PATROL1)
				item->goal_anim_state = MONKEY_WALK;
			else if (monkey->mood == ESCAPE_MOOD)
				item->goal_anim_state = MONKEY_STAND;
			else if (monkey->mood == BORED_MOOD)
			{
				if (item->required_anim_state)
					item->goal_anim_state = item->required_anim_state;
				else if (!(GetRandomControl() & 0xF))
					item->goal_anim_state = MONKEY_WALK;
				else if (!(GetRandomControl() & 0xF))
					item->goal_anim_state = ((GetRandomControl() & 0x1) ? MONKEY_SCRATCH : MONKEY_EAT);
			}
			else if ((item->ai_bits & FOLLOW) && (monkey->reached_goal || lara_info.distance > SQUARE(WALL_L * 2)))
			{
				if (item->required_anim_state)
					item->goal_anim_state = item->required_anim_state;
				else if (info.ahead)
					item->goal_anim_state = MONKEY_SIT;
				else item->goal_anim_state = MONKEY_STAND;
			}
			else if (info.bite && info.distance < MONKEY_JUMP_RANGE)
				item->goal_anim_state = MONKEY_STAND;
			else if (info.bite && info.distance < MONKEY_WALK_RANGE)
				item->goal_anim_state = MONKEY_WALK;
			else item->goal_anim_state = MONKEY_STAND;

			break;
		}
		case MONKEY_STAND:
		{
			monkey->flags = 0;
			monkey->maximum_turn = 0;

			head = lara_info.angle;

			if (item->ai_bits & GUARD)
			{
				head = AIGuard(monkey);

				if (!(GetRandomControl() & 0xF))
					item->goal_anim_state = ((GetRandomControl() & 0x1) ? MONKEY_ANGRY : MONKEY_SIT);
			}
			else if (item->ai_bits & PATROL1)
				item->goal_anim_state = MONKEY_WALK;
			else if (monkey->mood == ESCAPE_MOOD)
				item->goal_anim_state = (lara.target != item && info.ahead ? MONKEY_STAND : MONKEY_RUN);
			else if (monkey->mood == BORED_MOOD)
			{
				if (item->required_anim_state)
					item->goal_anim_state = item->required_anim_state;
				else if (!(GetRandomControl() & 0xF))
					item->goal_anim_state = MONKEY_WALK;
				else if (!(GetRandomControl() & 0xF))
					item->goal_anim_state = ((GetRandomControl() & 0x1) ? MONKEY_ANGRY : MONKEY_SIT);
			}
			else if ((item->ai_bits & FOLLOW) && (monkey->reached_goal || lara_info.distance > SQUARE(WALL_L * 2)))
			{
				if (item->required_anim_state)
					item->goal_anim_state = item->required_anim_state;
				else if (info.ahead)
					item->goal_anim_state = MONKEY_SIT;
				else item->goal_anim_state = MONKEY_RUN;
			}
			else if (info.bite && info.distance < MONKEY_ATTACK_RANGE)
				item->goal_anim_state = (lara_item->pos.y_pos < item->pos.y_pos ? MONKEY_ATAK_HIGH : MONKEY_ATAK_LOW);
			else if (info.bite && info.distance < MONKEY_JUMP_RANGE)
				item->goal_anim_state = MONKEY_ATAK_JUMP;
			else if (info.bite && info.distance < MONKEY_WALK_RANGE)
				item->goal_anim_state = MONKEY_WALK;
			else if (info.distance < MONKEY_WALK_RANGE && monkey->enemy != lara_item && monkey->enemy != nullptr &&
					 monkey->enemy->object_number != AI_PATROL1 && monkey->enemy->object_number != AI_PATROL2 &&
					 abs(item->pos.y_pos - monkey->enemy->pos.y_pos) < STEP_L)
			{
				item->goal_anim_state = MONKEY_PICKUP;
			}
			else if (info.bite && info.distance < MONKEY_ROLL_RANGE)
				item->goal_anim_state = MONKEY_ROLL;
			else item->goal_anim_state = MONKEY_RUN;

			break;
		}
		case MONKEY_PICKUP:
		{
			monkey->reached_goal = 1;

			if (monkey->enemy)
			{
				if ((monkey->enemy->object_number == MEDI_ITEM || monkey->enemy->object_number == KEY_ITEM4) && item->frame_number == anims[item->anim_number].frame_base + 12)
				{
					if (monkey->enemy->room_number == NO_ROOM || monkey->enemy->status == INVISIBLE || monkey->enemy->flags & KILLED_ITEM)
						monkey->enemy = nullptr;
					else
					{
						int i = monkey->enemy - items;

						item->carried_item = i;

						RemoveDrawnItem(i);

						monkey->enemy->room_number = NO_ROOM;
						monkey->enemy->carried_item = NO_ITEM;

						auto cinfo = baddie_slots;

						for (int j = 0; j < NUM_SLOTS; ++j, ++cinfo)
						{
							if (cinfo->item_num == NO_ITEM || cinfo->item_num == item_number)
								continue;

							auto target = &items[cinfo->item_num];

							if (cinfo->enemy == monkey->enemy)
								cinfo->enemy = nullptr;
						}

						monkey->enemy = nullptr;

						if (item->ai_bits != MODIFY)
						{
							item->ai_bits |= AMBUSH;
							item->ai_bits |= MODIFY;
						}
					}
				}
				else if (monkey->enemy->object_number == AI_AMBUSH && item->frame_number == anims[item->anim_number].frame_base + 12)
				{
					item->ai_bits = 0;

					auto pickup = &items[item->carried_item];

					pickup->pos.x_pos = item->pos.x_pos;
					pickup->pos.y_pos = item->pos.y_pos;
					pickup->pos.z_pos = item->pos.z_pos;

					ItemNewRoom(item->carried_item, item->room_number);

					item->carried_item = NO_ITEM;
					pickup->ai_bits = 1;
				}
				else
				{
					monkey->enemy = nullptr;
					monkey->maximum_turn = 0;

					if (abs(info.angle) < MONKEY_WALK_TURN)
						item->pos.y_rot += info.angle;
					else if (info.angle < 0)
						item->pos.y_rot -= MONKEY_WALK_TURN;
					else item->pos.y_rot += MONKEY_WALK_TURN;
				}
			}

			break;
		}
		case MONKEY_WALK:
		{
			monkey->maximum_turn = MONKEY_WALK_TURN;

			head = lara_info.angle;

			if (item->ai_bits & PATROL1)
			{
				item->goal_anim_state = MONKEY_WALK;
				head = 0;
			}
			else if (monkey->mood == ESCAPE_MOOD)
				item->goal_anim_state = MONKEY_RUN;
			else if (monkey->mood == BORED_MOOD)
			{
				if (GetRandomControl() < MONKEY_WAIT_CHANCE)
					item->goal_anim_state = MONKEY_SIT;
			}
			else if (info.bite && info.distance < MONKEY_JUMP_RANGE)
				item->goal_anim_state = MONKEY_STAND;
			else item->goal_anim_state = MONKEY_STAND;

			break;
		}
		case MONKEY_RUN:
		{
			monkey->maximum_turn = MONKEY_RUN_TURN;

			if (info.ahead)
				head = info.angle;

			tilt = angle / 2;

			if (item->ai_bits & GUARD)
				item->goal_anim_state = MONKEY_STAND;
			else if (monkey->mood == ESCAPE_MOOD)
			{
				if (lara.target != item && info.ahead)
					item->goal_anim_state = MONKEY_STAND;
			}
			else if ((item->ai_bits & FOLLOW) && (monkey->reached_goal || lara_info.distance > SQUARE(WALL_L * 2)))
				item->goal_anim_state = MONKEY_STAND;
			else if (monkey->mood == BORED_MOOD)
				item->goal_anim_state = MONKEY_ROLL;
			else if (info.distance < MONKEY_WALK_RANGE)
				item->goal_anim_state = MONKEY_STAND;
			else if (info.bite && info.distance < MONKEY_ROLL_RANGE)
				item->goal_anim_state = MONKEY_ROLL;

			break;
		}
		case MONKEY_ATAK_LOW:
		{
			monkey->maximum_turn = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			if (abs(info.angle) < MONKEY_WALK_TURN)
				item->pos.y_rot += info.angle;
			else if (info.angle < 0)
				item->pos.y_rot -= MONKEY_WALK_TURN;
			else item->pos.y_rot += MONKEY_WALK_TURN;

			if (enemy == lara_item)
			{
				if (!monkey->flags && (item->touch_bits & MONKEY_TOUCH))
				{
					lara_item->hit_points -= MONKEY_HIT_DAMAGE;
					lara_item->hit_status = 1;

					CreatureEffect(item, &monkey_hit, DoBloodSplat);

					monkey->flags = 1;
				}
			}
			else
			{
				if (!monkey->flags && enemy)
				{
					if (ABS(enemy->pos.x_pos - item->pos.x_pos) < MONKEY_HIT_RADIUS &&
						ABS(enemy->pos.y_pos - item->pos.y_pos) <= MONKEY_HIT_RADIUS &&
						ABS(enemy->pos.z_pos - item->pos.z_pos) < MONKEY_HIT_RADIUS)
					{
						enemy->hit_points -= MONKEY_HIT_DAMAGE >> 1;
						enemy->hit_status = 1;
						monkey->flags = 1;

						CreatureEffect(item, &monkey_hit, DoBloodSplat);
					}
				}
			}

			break;
		}
		case MONKEY_ATAK_HIGH:
		{
			monkey->maximum_turn = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			if (abs(info.angle) < MONKEY_WALK_TURN)
				item->pos.y_rot += info.angle;
			else if (info.angle < 0)
				item->pos.y_rot -= MONKEY_WALK_TURN;
			else item->pos.y_rot += MONKEY_WALK_TURN;

			if (enemy == lara_item)
			{
				if (!monkey->flags && (item->touch_bits & MONKEY_TOUCH))
				{
					lara_item->hit_points -= MONKEY_HIT_DAMAGE;
					lara_item->hit_status = 1;

					CreatureEffect(item, &monkey_hit, DoBloodSplat);

					monkey->flags = 1;
				}
			}
			else if (!monkey->flags && enemy)
			{
				if (ABS(enemy->pos.x_pos - item->pos.x_pos) < MONKEY_HIT_RADIUS &&
					ABS(enemy->pos.y_pos - item->pos.y_pos) <= MONKEY_HIT_RADIUS &&
					ABS(enemy->pos.z_pos - item->pos.z_pos) < MONKEY_HIT_RADIUS)
				{
					enemy->hit_points -= MONKEY_HIT_DAMAGE >> 1;
					enemy->hit_status = 1;
					monkey->flags = 1;

					CreatureEffect(item, &monkey_hit, DoBloodSplat);
				}
			}
		}
		case MONKEY_ATAK_JUMP:
		{
			monkey->maximum_turn = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.x_angle;
			}

			if (abs(info.angle) < MONKEY_WALK_TURN)
				item->pos.y_rot += info.angle;
			else if (info.angle < 0)
				item->pos.y_rot -= MONKEY_WALK_TURN;
			else item->pos.y_rot += MONKEY_WALK_TURN;

			if (enemy == lara_item)
			{
				if (monkey->flags != 1 && (item->touch_bits & MONKEY_TOUCH))
				{
					lara_item->hit_points -= MONKEY_SWIPE_DAMAGE;
					lara_item->hit_status = 1;

					CreatureEffect(item, &monkey_hit, DoBloodSplat);

					monkey->flags = 1;
				}
			}
			else
			{
				if (monkey->flags != 1 && enemy)
				{
					if (ABS(enemy->pos.x_pos - item->pos.x_pos) < MONKEY_HIT_RADIUS &&
						ABS(enemy->pos.y_pos - item->pos.y_pos) <= MONKEY_HIT_RADIUS &&
						ABS(enemy->pos.z_pos - item->pos.z_pos) < MONKEY_HIT_RADIUS)
					{
						enemy->hit_points -= MONKEY_SWIPE_DAMAGE >> 1;
						enemy->hit_status = 1;
						monkey->flags = 1;

						CreatureEffect(item, &monkey_hit, DoBloodSplat);
					}
				}
			}
		}
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso_y);

	if (item->current_anim_state < MONKEY_CLIMB4)
	{
		switch (CreatureVault(item_number, angle, 2, MONKEY_VAULT_SHIFT))
		{
		case 2:
			monkey->maximum_turn = 0;
			item->anim_number = objects[MONKEY].anim_index + MONKEY_CLIMB2_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = MONKEY_CLIMB2;
			break;
		case 3:
			monkey->maximum_turn = 0;
			item->anim_number = objects[MONKEY].anim_index + MONKEY_CLIMB3_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = MONKEY_CLIMB3;
			break;
		case 4:
			monkey->maximum_turn = 0;
			item->anim_number = objects[MONKEY].anim_index + MONKEY_CLIMB4_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = MONKEY_CLIMB4;
			break;
		case -2:
			monkey->maximum_turn = 0;
			item->anim_number = objects[MONKEY].anim_index + MONKEY_DOWN2_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = MONKEY_DOWN2;
			break;
		case -3:
			monkey->maximum_turn = 0;
			item->anim_number = objects[MONKEY].anim_index + MONKEY_DOWN3_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = MONKEY_DOWN3;
			break;
		case -4:
			monkey->maximum_turn = 0;
			item->anim_number = objects[MONKEY].anim_index + MONKEY_DOWN4_ANIM;
			item->frame_number = anims[item->anim_number].frame_base;
			item->current_anim_state = MONKEY_DOWN4;
		}
	}
	else
	{
		monkey->maximum_turn = 0;
		CreatureAnimation(item_number, angle, 0);
	}
}