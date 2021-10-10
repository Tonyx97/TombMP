#include <specific/standard.h>
#include <specific/global.h>
#include <specific/hwrender.h>
#include <specific/init.h>
#include <specific/drawprimitive.h>
#include <specific/fn_stubs.h>

#include <game/room.h>
#include <game/draw.h>
#include <game/effect2.h>
#include <game/lara.h>
#include <game/camera.h>

#include <main.h>

#include "hwinsert.h"
#include "3d_gen.h"
#include "3dglodef.h"

#define MINZR	(0.005f)
#define MAXZR	(0.995f)
#define ZRANGE	(MAXZR - MINZR)

float g_aspect_correction = 1.f;

void phd_InitBuffers()
{
	matrix_stack = new int32_t[MAX_MATRICES * 12]();
	vbuf = new PHD_VBUF[MAX_VERTICES]();
	phdtextinfo = new PHDTEXTURESTRUCT[MAX_TEXTURES]();
	info3d_bufferbf = new int16_t[MAX_POLYGONS * 30]();
}

void phd_DestroyBuffers()
{
	delete[] info3d_bufferbf;
	delete[] phdtextinfo;
	delete[] vbuf;
	delete[] matrix_stack;
}

/*
* Generate World-To-View Matrix given Position and Rotations
* W2V is generated at Base Position on matrix stack...
*/
void phd_GenerateW2V(const PHD_3DPOS& viewpos)
{
    PHD_VECTOR s { phd_sin(viewpos.x_rot), phd_sin(viewpos.y_rot), phd_sin(viewpos.z_rot) };
    PHD_VECTOR c { phd_cos(viewpos.x_rot), phd_cos(viewpos.y_rot), phd_cos(viewpos.z_rot) };
    int32_t* mptr, * nptr;

    mptr = phd_mxptr = matrix_stack;    /* Put W2V at Top Of matrix Stack */
    nptr = w2v_matrix;

    *(nptr + M00) = *(mptr + M00) = (TRIGMULT3(s.x, s.y, s.z) + TRIGMULT2(c.y, c.z));
    *(nptr + M01) = *(mptr + M01) = (TRIGMULT2(c.x, s.z));
    *(nptr + M02) = *(mptr + M02) = (TRIGMULT3(s.x, c.y, s.z) - TRIGMULT2(s.y, c.z));
    *(nptr + M10) = *(mptr + M10) = (TRIGMULT3(s.x, s.y, c.z) - TRIGMULT2(c.y, s.z));
    *(nptr + M11) = *(mptr + M11) = (TRIGMULT2(c.x, c.z));
    *(nptr + M12) = *(mptr + M12) = (TRIGMULT3(s.x, c.y, c.z) + TRIGMULT2(s.y, s.z));
    *(nptr + M20) = *(mptr + M20) = (TRIGMULT2(c.x, s.y));
    *(nptr + M21) = *(mptr + M21) = -(s.x);
    *(nptr + M22) = *(mptr + M22) = (TRIGMULT2(c.x, c.y));
    *(nptr + M03) = *(mptr + M03) = viewpos.x_pos;
    *(nptr + M13) = *(mptr + M13) = viewpos.y_pos;
    *(nptr + M23) = *(mptr + M23) = viewpos.z_pos;

    *(nptr + M10) = *(mptr + M10) = (int32_t)((float)(*(mptr + M10)) * g_aspect_correction);
    *(nptr + M11) = *(mptr + M11) = (int32_t)((float)(*(mptr + M11)) * g_aspect_correction);
    *(nptr + M12) = *(mptr + M12) = (int32_t)((float)(*(mptr + M12)) * g_aspect_correction);
}

/*
* Setup w2v to look at specific target
*/
void phd_LookAt(const PHD_VECTOR& src, const PHD_VECTOR& tar, PHD_ANGLE roll)
{
    PHD_3DPOS viewer;
	
    auto angles = phd_GetVectorAngles({ tar.x - src.x, tar.y - src.y , tar.z - src.z });

	viewer.x_pos = src.x;
	viewer.y_pos = src.y;
	viewer.z_pos = src.z;
	viewer.x_rot = angles.y;
	viewer.y_rot = angles.x;
	viewer.z_rot = roll;
	
    PHD_VECTOR d { src.x - tar.x , src.y - tar.y, src.z - tar.z };

    int len = (int)sqrtf((float)((d.x * d.x) + (d.z * d.z)));

    CamRot = { (m_atan2(0, 0, len, d.y) >> 4) & 4095, (m_atan2(src.z, src.x, tar.z, tar.x) >> 4) & 4095, 0 };
	
	phd_GenerateW2V(viewer);
}

/**
* Get Angles In 3D Space from Source to Dest
* get Yaw angle at pos 0
* Pitch angle at pos 1
*/
PHD_ANGLE_VEC phd_GetVectorAngles(const PHD_VECTOR& src)
{
	PHD_VECTOR lsrc = src;
	PHD_ANGLE_VEC dest;
    PHD_ANGLE pitch;

    dest.x = phd_atan(lsrc.z, lsrc.x);

    while ((int16_t)lsrc.x != lsrc.x || (int16_t)lsrc.y != lsrc.y || (int16_t)lsrc.z != lsrc.z)
    {
		// scale coords into word size

        lsrc.x >>= 2;
        lsrc.y >>= 2;
        lsrc.z >>= 2;
    }

    pitch = phd_atan(phd_sqrt(lsrc.x * lsrc.x + lsrc.z * lsrc.z), lsrc.y);

    if ((lsrc.y > 0 && pitch > 0) || (lsrc.y < 0 && pitch < 0))
        pitch = -pitch;

    dest.y = pitch;
	dest.z = 0;

    return dest;
}

/**
* Normalize a vector to length 16384
*/
void phd_NormaliseVector(int32_t x, int32_t y, int32_t z, int32_t* dest)
{
    int32_t len;

    if (x || y || z)
    {
        while ((int16_t)x != x || (int16_t)y != y || (int16_t)z != z)
        {
			// scale coords into word size vals

            x >>= 2;
            y >>= 2;
            z >>= 2;
        }

        len = phd_sqrt(x * x + y * y + z * z);

        *(dest + 0) = (x << W2V_SHIFT) / len;
        *(dest + 1) = (y << W2V_SHIFT) / len;
        *(dest + 2) = (z << W2V_SHIFT) / len;
    }
    else
    {
        *(dest + 0) = 0;		/* If Vector Is Zero Length*/
        *(dest + 1) = 0;      /* then return Null Vector*/
        *(dest + 2) = 0;
    }
}

/**
* Normalize and regenerate current matrix
*/
void phd_NormaliseMatrix()
{
	auto angles = phd_GetMatrixAngles();

	phd_UnitMatrix();
	phd_RotYXZ(angles.y, angles.x, angles.z);
}

/**
* Get rotation angles from current matrix in rot. order (z, x, y)
* saves inn order rot_x, rot_y, rot_z
*/
PHD_ANGLE_VEC phd_GetMatrixAngles()
{
	auto mptr = phd_mxptr;

	PHD_ANGLE_VEC angles;

	angles.y = phd_atan(*(mptr + M22), *(mptr + M02));

	auto sy = phd_sin(angles.y),
		 cy = phd_cos(angles.y);

	angles.x = phd_atan(phd_sqrt(*(mptr + M22) * *(mptr + M22) + *(mptr + M02) * *(mptr + M02)), *(mptr + M12));
	angles.z = phd_atan(((cy * *(mptr + M00)) - (sy * *(mptr + M20))), ((sy * *(mptr + M21)) - (cy * *(mptr + M01))));

	if ((*(mptr + M12) >= 0 && angles.x > 0) || (*(mptr + M12) < 0 && angles.x < 0))
		angles.x = -angles.x;

	return angles;
}

/**
* Get rotation position from current matrix
*/
PHD_VECTOR phd_GetMatrixPosition()
{
	return { *(phd_mxptr + M03) >> W2V_SHIFT,
			 *(phd_mxptr + M13) >> W2V_SHIFT,
			 *(phd_mxptr + M23) >> W2V_SHIFT };
}

/**
* Rotate current matrix around its x-axis
*/
void phd_RotX(PHD_ANGLE rx)
{
	if (rx == 0)
		return;

	auto mptr = phd_mxptr;

	auto sx = phd_sin(rx),
		 cx = phd_cos(rx);

	auto r0 = *(mptr + M01) * cx + *(mptr + M02) * sx,
		 r1 = *(mptr + M02) * cx - *(mptr + M01) * sx;

	*(mptr + M01) = r0 >> W2V_SHIFT;
	*(mptr + M02) = r1 >> W2V_SHIFT;

	r0 = *(mptr + M11) * cx + *(mptr + M12) * sx;
	r1 = *(mptr + M12) * cx - *(mptr + M11) * sx;

	*(mptr + M11) = r0 >> W2V_SHIFT;
	*(mptr + M12) = r1 >> W2V_SHIFT;

	r0 = *(mptr + M21) * cx + *(mptr + M22) * sx;
	r1 = *(mptr + M22) * cx - *(mptr + M21) * sx;

	*(mptr + M21) = r0 >> W2V_SHIFT;
	*(mptr + M22) = r1 >> W2V_SHIFT;
}

/**
* Rotate current matrix around its y-axis
*/
void phd_RotY(PHD_ANGLE ry)
{
	if (ry == 0)
		return;

	auto mptr = phd_mxptr;

	auto sy = phd_sin(ry),
		 cy = phd_cos(ry);

	auto r0 = *(mptr + M00) * cy - *(mptr + M02) * sy,
		 r1 = *(mptr + M02) * cy + *(mptr + M00) * sy;

	*(mptr + M00) = r0 >> W2V_SHIFT;
	*(mptr + M02) = r1 >> W2V_SHIFT;

	r0 = *(mptr + M10) * cy - *(mptr + M12) * sy;
	r1 = *(mptr + M12) * cy + *(mptr + M10) * sy;

	*(mptr + M10) = r0 >> W2V_SHIFT;
	*(mptr + M12) = r1 >> W2V_SHIFT;

	r0 = *(mptr + M20) * cy - *(mptr + M22) * sy;
	r1 = *(mptr + M22) * cy + *(mptr + M20) * sy;

	*(mptr + M20) = r0 >> W2V_SHIFT;
	*(mptr + M22) = r1 >> W2V_SHIFT;
}

/**
* Rotate current matrix around its z-axis
*/
void phd_RotZ(PHD_ANGLE rz)
{
	if (rz == 0)
		return;

	auto mptr = phd_mxptr;
		
	auto sz = phd_sin(rz),
		 cz = phd_cos(rz);

	auto r0 = *(mptr + M00) * cz + *(mptr + M01) * sz,
		 r1 = *(mptr + M01) * cz - *(mptr + M00) * sz;

	*(mptr + M00) = r0 >> W2V_SHIFT;
	*(mptr + M01) = r1 >> W2V_SHIFT;

	r0 = *(mptr + M10) * cz + *(mptr + M11) * sz;
	r1 = *(mptr + M11) * cz - *(mptr + M10) * sz;

	*(mptr + M10) = r0 >> W2V_SHIFT;
	*(mptr + M11) = r1 >> W2V_SHIFT;

	r0 = *(mptr + M20) * cz + *(mptr + M21) * sz;
	r1 = *(mptr + M21) * cz - *(mptr + M20) * sz;

	*(mptr + M20) = r0 >> W2V_SHIFT;
	*(mptr + M21) = r1 >> W2V_SHIFT;
}

/**
* Do successive rotations in order y-x-z
*/
void phd_RotYXZ(PHD_ANGLE ry, PHD_ANGLE rx, PHD_ANGLE rz)
{
	auto mptr = phd_mxptr;

	if (ry)
	{
		auto sina = phd_sin(ry),
			 cosa = phd_cos(ry);

		auto r0 = *(mptr + M00) * cosa - *(mptr + M02) * sina,
			 r1 = *(mptr + M02) * cosa + *(mptr + M00) * sina;

		*(mptr + M00) = r0 >> W2V_SHIFT;
		*(mptr + M02) = r1 >> W2V_SHIFT;

		r0 = *(mptr + M10) * cosa - *(mptr + M12) * sina;
		r1 = *(mptr + M12) * cosa + *(mptr + M10) * sina;

		*(mptr + M10) = r0 >> W2V_SHIFT;
		*(mptr + M12) = r1 >> W2V_SHIFT;

		r0 = *(mptr + M20) * cosa - *(mptr + M22) * sina;
		r1 = *(mptr + M22) * cosa + *(mptr + M20) * sina;

		*(mptr + M20) = r0 >> W2V_SHIFT;
		*(mptr + M22) = r1 >> W2V_SHIFT;
	}

	if (rx)
	{
		auto sina = phd_sin(rx),
			 cosa = phd_cos(rx);

		auto r0 = *(mptr + M01) * cosa + *(mptr + M02) * sina,
			 r1 = *(mptr + M02) * cosa - *(mptr + M01) * sina;

		*(mptr + M01) = r0 >> W2V_SHIFT;
		*(mptr + M02) = r1 >> W2V_SHIFT;

		r0 = *(mptr + M11) * cosa + *(mptr + M12) * sina;
		r1 = *(mptr + M12) * cosa - *(mptr + M11) * sina;

		*(mptr + M11) = r0 >> W2V_SHIFT;
		*(mptr + M12) = r1 >> W2V_SHIFT;

		r0 = *(mptr + M21) * cosa + *(mptr + M22) * sina;
		r1 = *(mptr + M22) * cosa - *(mptr + M21) * sina;

		*(mptr + M21) = r0 >> W2V_SHIFT;
		*(mptr + M22) = r1 >> W2V_SHIFT;
	}

	if (rz)
	{
		auto sina = phd_sin(rz),
			 cosa = phd_cos(rz);

		auto r0 = *(mptr + M00) * cosa + *(mptr + M01) * sina,
			 r1 = *(mptr + M01) * cosa - *(mptr + M00) * sina;

		*(mptr + M00) = r0 >> W2V_SHIFT;
		*(mptr + M01) = r1 >> W2V_SHIFT;

		r0 = *(mptr + M10) * cosa + *(mptr + M11) * sina;
		r1 = *(mptr + M11) * cosa - *(mptr + M10) * sina;

		*(mptr + M10) = r0 >> W2V_SHIFT;
		*(mptr + M11) = r1 >> W2V_SHIFT;

		r0 = *(mptr + M20) * cosa + *(mptr + M21) * sina;
		r1 = *(mptr + M21) * cosa - *(mptr + M20) * sina;

		*(mptr + M20) = r0 >> W2V_SHIFT;
		*(mptr + M21) = r1 >> W2V_SHIFT;
	}
}

/**
* Do successive rotations in order y-x-z
* RRotations are packed in long word, 10 bits each
* RotX = bits 29..20
* RotY = bits 19..10
* RotZ = bits 09..00
*/
void phd_RotYXZpack(int32_t rots)
{
	int32_t		sina, cosa, r0, r1;

	auto mptr = phd_mxptr;
	auto ang = (((rots >> 10) & 1023) << 6);		// extract RotY

	if (ang)
	{
		auto sina = phd_sin(ang),
			 cosa = phd_cos(ang);

		r0 = *(mptr + M00) * cosa - *(mptr + M02) * sina;
		r1 = *(mptr + M02) * cosa + *(mptr + M00) * sina;

		*(mptr + M00) = r0 >> W2V_SHIFT;
		*(mptr + M02) = r1 >> W2V_SHIFT;

		r0 = *(mptr + M10) * cosa - *(mptr + M12) * sina;
		r1 = *(mptr + M12) * cosa + *(mptr + M10) * sina;

		*(mptr + M10) = r0 >> W2V_SHIFT;
		*(mptr + M12) = r1 >> W2V_SHIFT;

		r0 = *(mptr + M20) * cosa - *(mptr + M22) * sina;
		r1 = *(mptr + M22) * cosa + *(mptr + M20) * sina;

		*(mptr + M20) = r0 >> W2V_SHIFT;
		*(mptr + M22) = r1 >> W2V_SHIFT;
	}

	if (ang = (((rots >> 20) & 1023) << 6); ang)	// extract RotX
	{
		sina = phd_sin(ang);
		cosa = phd_cos(ang);
		r0 = *(mptr + M01) * cosa + *(mptr + M02) * sina;
		r1 = *(mptr + M02) * cosa - *(mptr + M01) * sina;
		*(mptr + M01) = r0 >> W2V_SHIFT;
		*(mptr + M02) = r1 >> W2V_SHIFT;
		r0 = *(mptr + M11) * cosa + *(mptr + M12) * sina;
		r1 = *(mptr + M12) * cosa - *(mptr + M11) * sina;
		*(mptr + M11) = r0 >> W2V_SHIFT;
		*(mptr + M12) = r1 >> W2V_SHIFT;
		r0 = *(mptr + M21) * cosa + *(mptr + M22) * sina;
		r1 = *(mptr + M22) * cosa - *(mptr + M21) * sina;
		*(mptr + M21) = r0 >> W2V_SHIFT;
		*(mptr + M22) = r1 >> W2V_SHIFT;
	}

	if (ang = ((rots & 1023) << 6); ang)			// extract RotZ
	{
		sina = phd_sin(ang);
		cosa = phd_cos(ang);
		r0 = *(mptr + M00) * cosa + *(mptr + M01) * sina;
		r1 = *(mptr + M01) * cosa - *(mptr + M00) * sina;
		*(mptr + M00) = r0 >> W2V_SHIFT;
		*(mptr + M01) = r1 >> W2V_SHIFT;
		r0 = *(mptr + M10) * cosa + *(mptr + M11) * sina;
		r1 = *(mptr + M11) * cosa - *(mptr + M10) * sina;
		*(mptr + M10) = r0 >> W2V_SHIFT;
		*(mptr + M11) = r1 >> W2V_SHIFT;
		r0 = *(mptr + M20) * cosa + *(mptr + M21) * sina;
		r1 = *(mptr + M21) * cosa - *(mptr + M20) * sina;
		*(mptr + M20) = r0 >> W2V_SHIFT;
		*(mptr + M21) = r1 >> W2V_SHIFT;
	}
}

/**
* Translate matrix to relative position
*/
bool phd_TranslateRel(int32_t x, int32_t y, int32_t z)
{
	auto mptr = phd_mxptr;

	*(mptr + M03) += *(mptr + M00) * x + *(mptr + M01) * y + *(mptr + M02) * z;
	*(mptr + M13) += *(mptr + M10) * x + *(mptr + M11) * y + *(mptr + M12) * z;
	*(mptr + M23) += *(mptr + M20) * x + *(mptr + M21) * y + *(mptr + M22) * z;

	return !((x = ABS(*(mptr + M03))) > phd_zfar ||
			 (y = ABS(*(mptr + M13))) > phd_zfar ||
			 (z = ABS(*(mptr + M23))) > phd_zfar);
}

/**
* Translate matrix to absolute position
* Returns 0 if out of range
*/
void phd_TranslateAbs(int32_t x, int32_t y, int32_t z)
{
	auto mptr = phd_mxptr;

	// subtract position of w2v matrix
	x -= *(w2v_matrix + M03);
	y -= *(w2v_matrix + M13);
	z -= *(w2v_matrix + M23);

	*(mptr + M03) = *(mptr + M00) * x + *(mptr + M01) * y + *(mptr + M02) * z;
	*(mptr + M13) = *(mptr + M10) * x + *(mptr + M11) * y + *(mptr + M12) * z;
	*(mptr + M23) = *(mptr + M20) * x + *(mptr + M21) * y + *(mptr + M22) * z;
}

/**
* Insert polygon object polygons into draw list
* using current matrix pointed to by phd_mxptr
*/
void phd_PutPolygons(int16_t* objptr, int clipstatus)
{
	// skip x, y, z and radius

	objptr += 4;

	phd_leftfloat = (float)phd_winxmin;
	phd_topfloat = (float)phd_winymin;
	phd_rightfloat = (float)(phd_winxmin + phd_winxmax + 1);
	phd_bottomfloat = (float)(phd_winymin + phd_winymax + 1);
	f_centerx = (float)(phd_winxmin + phd_centerx);
	f_centery = (float)(phd_winymin + phd_centery);

	auto objptr1 = objptr;

	if (objptr = calc_object_vertices(objptr))
	{
		objptr = calc_vertice_light(objptr, objptr1);
		objptr = InsertObjectGT4(objptr + 1, *objptr, MID_SORT);
		objptr = InsertObjectGT3(objptr + 1, *objptr, MID_SORT);
		objptr = InsertObjectG4(objptr + 1, *objptr, MID_SORT);
		objptr = InsertObjectG3(objptr + 1, *objptr, MID_SORT);
	}

#ifdef GAMEDEBUG
	++PolysPushed;
#endif
}

/**
* Do room insertion
*/
void S_InsertRoom(int16_t* objptr, int outside)
{
	phd_leftfloat = (float)(phd_winxmin + phd_left);
	phd_topfloat = (float)(phd_winymin + phd_top);
	phd_rightfloat = (float)(phd_winxmin + phd_right + 1);
	phd_bottomfloat = (float)(phd_winymin + phd_bottom + 1);
	f_centerx = (float)(phd_winxmin + phd_centerx);
	f_centery = (float)(phd_winymin + phd_centery);

	objptr = calc_roomvert(objptr, outside ? 0 : 16);
	objptr = InsertObjectGT4(objptr + 1, *objptr, FAR_SORT);
	objptr = InsertObjectGT3(objptr + 1, *objptr, FAR_SORT);
	objptr = ins_room_sprite(objptr + 1, *objptr);
}

/**
* Calculate background object
*/
int16_t* calc_back_light(int16_t* objptr)
{
	// alow for prelit or dynamic lit info (but ignore either way)

	auto numvert = (int)*(objptr++);
	if (numvert > 0)
		objptr += numvert * 3;
	else if (numvert < 0)
	{
		numvert = -numvert;
		objptr += numvert;
	}

	// set shade to mid-shade for moment...
	for (PHD_VBUF* dest = vbuf; numvert > 0; --numvert, ++dest)
		dest->g = 16 << 10 | 16 << 5 | 16;
	
	return objptr;
}

void S_InsertBackground(int16_t* objptr)
{
	objptr += 4;

	phd_leftfloat = (float)(phd_winxmin + phd_left);
	phd_topfloat = (float)(phd_winymin + phd_top);
	phd_rightfloat = (float)(phd_winxmin + phd_right + 1);
	phd_bottomfloat = (float)(phd_winymin + phd_bottom + 1);
	f_centerx = (float)(phd_winxmin + phd_centerx);
	f_centery = (float)(phd_winymin + phd_centery);

	if (objptr = calc_object_vertices(objptr))
	{
		HWR_EnableZBuffer(false, false);

		objptr = calc_back_light(objptr);
		objptr = InsertObjectGT4(objptr + 1, *objptr, BACK_SORT);
		objptr = InsertObjectGT3(objptr + 1, *objptr, BACK_SORT);
		objptr = InsertObjectG4(objptr + 1, *objptr, BACK_SORT);
		
		if (fix_skybox)
		{
			auto v6 = (int)(objptr + 0x41);
			auto v7 = *objptr - 16;
			if (v7 > 0)
			{
				auto v8 = v6 + 6;
				do
				{
					v8 += 8;
					*(uint16_t*)(v8 - 8) = 0x700;
					--v7;
				} while (v7);
			}

			fix_skybox = false;
		}

		objptr = InsertObjectG3(objptr + 1, *(objptr), BACK_SORT);

		HWR_EnableColorKey(false);
		DrawBuckets();
		InitBuckets();

		phd_InitPolyList();

		HWR_EnableZBuffer(true, true);
	}
}

/**
* Position global light vector in world using angles
*/
void phd_RotateLight(PHD_ANGLE pitch, PHD_ANGLE yaw)
{
	auto cx = phd_cos(pitch);

	// x = calc world frame
	// y = light vector

	PHD_VECTOR ls_world { TRIGMULT2(cx, (int32_t)phd_sin(yaw)), -phd_sin(pitch), TRIGMULT2(cx, (int32_t)phd_cos(yaw)) };

	auto mptr = w2v_matrix;

	ls_vector_view.x = (*(mptr + M00) * ls_world.x
					   + *(mptr + M01) * ls_world.y 
					   + *(mptr + M02) * ls_world.z) >> W2V_SHIFT;

	ls_vector_view.y = (*(mptr + M10) * ls_world.x
					   + *(mptr + M11) * ls_world.y
					   + *(mptr + M12) * ls_world.z) >> W2V_SHIFT;

	ls_vector_view.z = (*(mptr + M20) * ls_world.x
					   + *(mptr + M21) * ls_world.y
					   + *(mptr + M22) * ls_world.z) >> W2V_SHIFT;
}

/**
* Position global light vector in world using destination position
* and position of light
*/
void phd_PointLight(PHD_3DPOS* destpos, PHD_3DPOS* lightpos)
{
	const auto angles = phd_GetVectorAngles({ destpos->x_pos - lightpos->x_pos,
											  destpos->y_pos - lightpos->y_pos,
											  destpos->z_pos - lightpos->z_pos });

	phd_RotateLight(angles.y, angles.x);
}

/**
* Initialise polygon output stuff
*/
void phd_InitPolyList()
{
	surfacenumbf = 0;

	info3dptrbf = (int16_t*)info3d_bufferbf;
	sort3dptrbf = (int32_t*)sort3d_bufferbf;

	HWR_InitVertexList();
}

/**
* Sort the polygon draw list in back to front order
*/
//void phd_SortPolyList(int number, int32_t** buffer)
void phd_SortPolyList(int number, int32_t buffer[][10])
{
	if (!number)
		return;

	// eliminate polygon flicker

	for (int i = 0; i < number; ++i)
		buffer[i][1] += i;

	do_quickysorty(0, number - 1, buffer);
}

/**
* Quick sort routine (pisses the shell sort, but uses recursion!!!!)
*/
//void do_quickysorty(int left, int right, int32_t** buffer)
void do_quickysorty(int left, int right, int32_t buffer[][10])
{
	// middle value
	auto compare = buffer[(left + right) / 2][1];

	auto i = left, j = right;

	do
	{
		while (buffer[i][1] > compare && i < right) ++i;	// was < x
		while (compare > buffer[j][1] && j > left)	--j;	// was x <

		if (i <= j)
		{
			auto swap = buffer[i][1];
			buffer[i][1] = buffer[j][1];
			buffer[j][1] = swap;

			swap = buffer[i][0];
			buffer[i][0] = buffer[j][0];
			buffer[j][0] = swap;

			++i;
			--j;
		}
	} while (i <= j);

	if (left < j)  do_quickysorty(left, j, buffer);
	if (i < right) do_quickysorty(i, right, buffer);
}

/**
* Alter FOV while retainning other screen attributes
* Angle is in PHD_ANGLE format
*/
void AlterFOV(PHD_ANGLE fov)
{
	auto [aspect_x, aspect_y] = g_window->get_aspect();

	g_window->set_fov(int(camera.fov = fov));

	int new_fov = (fov * ONE_DEGREE) / 2;

	phd_persp = ((phd_winwidth / 2) * phd_cos(new_fov)) / phd_sin(new_fov);
	phd_oopersp = (int32_t)one / phd_persp;
	f_persp = (float)phd_persp;
	f_oneopersp = one / f_persp;
	f_perspoznear = f_persp / f_znear;

	float aspect = float(phd_winwidth) / float(phd_winheight),
		  desired_aspect = float(aspect_x) / float(aspect_y);

	g_aspect_correction = desired_aspect / aspect;

	InitZTable();
}

void SetupZRange()
{
	f_b = (ZRANGE * f_znear * f_zfar) / (f_znear - f_zfar);
	f_a = MINZR - (f_b / f_znear);
	f_b = -f_b;
	f_boo = f_b / one;
}

void SetZNear(int nNewZNear)
{
	phd_znear = nNewZNear;
	f_znear = (float)phd_znear;
	f_oneoznear = one / f_znear;
	f_perspoznear = f_persp / f_znear;

	SetupZRange();
}

void SetZFar(int nNewZFar)
{
	phd_zfar = nNewZFar;
	f_zfar = (float)phd_zfar;

	SetupZRange();
}

/**
* Initialize 3D window
*/
void phd_InitWindow(int x, int y,
					int width, int height,
					int nearz, int farz,
					int scrwidth,
					int	scrheight)
{
	phd_winxmin = x;
	phd_winymin = y;
	phd_winxmax = width;
	phd_winymax = height;
	phd_winwidth = width;
	phd_winheight = height;
	phd_centerx = width / 2;
	phd_centery = height / 2;
	phd_znear = nearz << W2V_SHIFT;
	phd_zfar = farz << W2V_SHIFT;
	phd_viewdist = farz;
	phd_scrwidth = scrwidth;
	phd_scrheight = scrheight;

	f_centerx = (float)phd_centerx;
	f_centery = (float)phd_centery;

	AlterFOV(g_window->get_fov());
	SetZNear(phd_znear);
	SetZFar(phd_zfar);

	InitZTable();

	phd_left = 0;
	phd_top = 0;
	phd_right = phd_winxmax;
	phd_bottom = phd_winymax;
	phd_mxptr = matrix_stack;

	grc_dump_window.left = phd_winxmin;
	grc_dump_window.top = phd_winymin;
	grc_dump_window.right = phd_winxmin + phd_winwidth;
	grc_dump_window.bottom = phd_winymin + phd_winheight;
}