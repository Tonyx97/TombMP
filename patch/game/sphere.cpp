#include <vcruntime_string.h>

#include "anim.h"
#include "draw.h"
#include "sphere.h"

extern int16_t null_rotations[];
extern int32_t* IMptr;
extern int32_t IMstack[12 * 64];
extern int32_t IM_frac;
extern int32_t IM_rate;

void InitInterpolate2(int frac, int rate)
{
	IM_frac = frac;
	IM_rate = rate;
	IMptr = &IMstack[12 * 32];

	memcpy(IMptr, phd_mxptr, 12 * 4);
}

bool TestCollision(ITEM_INFO* item, ITEM_INFO* laraitem)
{
	SPHERE slist_baddie[34],
		   slist_lara[34];

	uint32_t flags = 0;

	int num1 = GetSpheres(item, &slist_baddie[0], 1),
		num2 = GetSpheres(laraitem, &slist_lara[0], 1);

	auto ptr1 = &slist_baddie[0];

	for (int i = 0; i < num1; ++i, ++ptr1)
	{
		int x1 = ptr1->x,
			y1 = ptr1->y,
			z1 = ptr1->z,
			r1 = ptr1->r;

		if (r1 > 0)
		{
			auto ptr2 = &slist_lara[0];

			for (int j = 0; j < num2; ++j, ++ptr2)
			{
				if (ptr2->r > 0)
				{
					int x = ptr2->x - x1,
						y = ptr2->y - y1,
						z = ptr2->z - z1,
						r = ptr2->r + r1;

					if ((x * x + y * y + z * z) < (r * r))
					{
						flags |= (1 << i);
						break;
					}
				}
			}
		}
	}

	return !!(item->touch_bits = flags);
}

int GetSpheres(ITEM_INFO* item, SPHERE* ptr, int WorldSpace)
{
	if (!item)
		return 0;

	PHD_VECTOR pos {};

	if (WorldSpace)
	{
		pos = { item->pos.x_pos, item->pos.y_pos, item->pos.z_pos };

		phd_PushUnitMatrix();

		*(phd_mxptr + M03) = 0;
		*(phd_mxptr + M13) = 0;
		*(phd_mxptr + M23) = 0;
	}
	else
	{
		phd_PushMatrix();
		phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
	}

	phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);

	auto frame = GetBestFrame(item);

	phd_TranslateRel((int32_t)*(frame + 6), (int32_t)*(frame + 7), (int32_t)*(frame + 8));

	auto rotation = frame + 9;

	gar_RotYXZsuperpack(&rotation, 0);

	auto object = &objects[item->object_number];
	auto meshpp = &meshes[object->mesh_index];
	auto bone = bones + object->bone_index;
	auto objptr = *(meshpp++);

	phd_PushMatrix();
	{
		phd_TranslateRel((int32_t)*(objptr), (int32_t)*(objptr + 1), (int32_t)*(objptr + 2));

		ptr->x = pos.x + (*(phd_mxptr + M03) >> W2V_SHIFT);
		ptr->y = pos.y + (*(phd_mxptr + M13) >> W2V_SHIFT);
		ptr->z = pos.z + (*(phd_mxptr + M23) >> W2V_SHIFT);
		ptr->r = (int32_t) * (objptr + 3);

		++ptr;
	}
	phd_PopMatrix();

	auto extra_rotation = (int16_t*)item->data;

	for (int i = object->nmeshes - 1; i > 0; --i, bone += 3)
	{
		int poppush = *(bone++);

		if (poppush & 1) phd_PopMatrix();
		if (poppush & 2) phd_PushMatrix();

		phd_TranslateRel(*(bone), *(bone + 1), *(bone + 2));
		gar_RotYXZsuperpack(&rotation, 0);

		if ((poppush & (ROT_X | ROT_Y | ROT_Z)) && extra_rotation)
		{
			if (poppush & ROT_Y) phd_RotY(*(extra_rotation++));
			if (poppush & ROT_X) phd_RotX(*(extra_rotation++));
			if (poppush & ROT_Z) phd_RotZ(*(extra_rotation++));
		}

		objptr = *(meshpp++);

		phd_PushMatrix();
		{
			phd_TranslateRel((int32_t) * (objptr), (int32_t) * (objptr + 1), (int32_t) * (objptr + 2));

			ptr->x = pos.x + (*(phd_mxptr + M03) >> W2V_SHIFT);
			ptr->y = pos.y + (*(phd_mxptr + M13) >> W2V_SHIFT);
			ptr->z = pos.z + (*(phd_mxptr + M23) >> W2V_SHIFT);
			ptr->r = (int32_t) * (objptr + 3);

			++ptr;
		}
		phd_PopMatrix();

	}

	phd_PopMatrix();

	return object->nmeshes;
}

void GetJointAbsPosition(ITEM_INFO* item, PHD_VECTOR* vec, int joint)
{
	auto MatrixStash = phd_mxptr;
	auto IMStash = IMptr;

	auto object = &objects[item->object_number];

	int16_t* frmptr[2];

	int rate,
		frac = GetFrames(item, frmptr, &rate);

	phd_PushUnitMatrix();

	*(phd_mxptr + M03) = 0;
	*(phd_mxptr + M13) = 0;
	*(phd_mxptr + M23) = 0;

	phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);

	auto extra_rotation = (item->data ? (int16_t*)item->data : null_rotations);
	auto bone = bones + object->bone_index;

	if (!frac)
	{
		phd_TranslateRel((int32_t) * (frmptr[0] + 6), (int32_t) * (frmptr[0] + 7), (int32_t) * (frmptr[0] + 8));

		auto rotation1 = frmptr[0] + 9;

		gar_RotYXZsuperpack(&rotation1, 0);

		for (int i = 0; i < joint; ++i, bone += 4)
		{
			int poppush = *bone;

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
		}

		phd_TranslateRel(vec->x, vec->y, vec->z);

		vec->x = (*(phd_mxptr + M03) >> W2V_SHIFT) + item->pos.x_pos;
		vec->y = (*(phd_mxptr + M13) >> W2V_SHIFT) + item->pos.y_pos;
		vec->z = (*(phd_mxptr + M23) >> W2V_SHIFT) + item->pos.z_pos;
	}
	else
	{
		InitInterpolate2(frac, rate);

		auto rotation1 = frmptr[0] + 9,
			 rotation2 = frmptr[1] + 9;

		phd_TranslateRel_ID(
			(int32_t)*(frmptr[0] + 6),
			(int32_t)*(frmptr[0] + 7),
			(int32_t)*(frmptr[0] + 8),
			(int32_t)*(frmptr[1] + 6),
			(int32_t)*(frmptr[1] + 7),
			(int32_t)*(frmptr[1] + 8));

		gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);

		for (int i = 0; i < joint; ++i, bone += 4)
		{
			int poppush = *bone;

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
		}

		phd_TranslateRel_I(vec->x, vec->y, vec->z);
		InterpolateMatrix();

		vec->x = (*(phd_mxptr + M03) >> W2V_SHIFT) + item->pos.x_pos;
		vec->y = (*(phd_mxptr + M13) >> W2V_SHIFT) + item->pos.y_pos;
		vec->z = (*(phd_mxptr + M23) >> W2V_SHIFT) + item->pos.z_pos;
	}

	phd_mxptr = MatrixStash;
	IMptr = IMStash;
}