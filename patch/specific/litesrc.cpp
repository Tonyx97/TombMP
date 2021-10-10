#include "standard.h"
#include "global.h"
#include "litesrc.h"
#include "drawprimitive.h"
#include "init.h"

#include <game/effect2.h>
#include <game/lara.h>

#include <3dsystem/3d_gen.h>

enum
{
	SUN,
	OMN,
	DYN
};

void ApplyMatrix(int32_t* mptr, PHD_VECTOR* objptr, int32_t& x, int32_t& y, int32_t& z)
{
	x = (mptr[M00] * objptr->x + mptr[M01] * objptr->y + mptr[M02] * objptr->z) >> W2V_SHIFT;
	y = (mptr[M10] * objptr->x + mptr[M11] * objptr->y + mptr[M12] * objptr->z) >> W2V_SHIFT;
	z = (mptr[M20] * objptr->x + mptr[M21] * objptr->y + mptr[M22] * objptr->z) >> W2V_SHIFT;
}

void ApplyTransposeMatrix(int32_t* mptr, PHD_VECTOR* objptr, int32_t& x, int32_t& y, int32_t& z)
{
	x = (mptr[M00] * objptr->x + mptr[M10] * objptr->y + mptr[M20] * objptr->z) >> W2V_SHIFT;
	y = (mptr[M01] * objptr->x + mptr[M11] * objptr->y + mptr[M21] * objptr->z) >> W2V_SHIFT;
	z = (mptr[M02] * objptr->x + mptr[M12] * objptr->y + mptr[M22] * objptr->z) >> W2V_SHIFT;
}

int16_t* calc_vertice_light(int16_t* objptr, int16_t* objptr1)
{
	double fconv;

	auto dest = vbuf;
	auto numvert = int16_t(*objptr);

	++objptr;

	if (numvert > 0)
	{
		auto mptr = phd_mxptr;

		ApplyTransposeMatrix(mptr, &LPos[SUN], LightPos[M00], LightPos[M01], LightPos[M02]);
		ApplyTransposeMatrix(mptr, &LPos[OMN], LightPos[M10], LightPos[M11], LightPos[M12]);
		ApplyTransposeMatrix(mptr, &LPos[DYN], LightPos[M20], LightPos[M21], LightPos[M22]);

		for (; numvert > 0; --numvert, ++dest, objptr += 3)
		{
			int32_t sdot, odot, ddot;

			float v1 = float(objptr[0]),
				  v2 = float(objptr[1]),
				  v3 = float(objptr[2]);

			float fsdot = (v1 * float(LightPos[M00])) + (v2 * float(LightPos[M01])) + (v3 * float(LightPos[M02]));

			sdot = FTOL(fsdot);

			float fodot = (v1 * float(LightPos[M10])) + (v2 * float(LightPos[M11])) + (v3 * float(LightPos[M12]));

			odot = FTOL(fodot);

			float fddot = (v1 * float(LightPos[M20])) + (v2 * float(LightPos[M21])) + (v3 * float(LightPos[M22]));

			ddot = FTOL(fddot);

			sdot >>= 16;
			odot >>= 16;
			ddot >>= 16;

			if (sdot < 0) sdot = 0;
			if (odot < 0) odot = 0;
			if (ddot < 0) ddot = 0;

			int r = (smcr << 13) + (LightCol[M00] * sdot) + (LightCol[M01] * odot) + (LightCol[M02] * ddot),
				g = (smcg << 13) + (LightCol[M10] * sdot) + (LightCol[M11] * odot) + (LightCol[M12] * ddot),
				b = (smcb << 13) + (LightCol[M20] * sdot) + (LightCol[M21] * odot) + (LightCol[M22] * ddot);

			r >>= 16 + 3;
			g >>= 16 + 3;
			b >>= 16 + 3;

			if (r > 0x1f) r = 0x1f;
			if (g > 0x1f) g = 0x1f;
			if (b > 0x1f) b = 0x1f;

			if (r < 0) r = 0;
			if (g < 0) g = 0;
			if (b < 0) b = 0;

			// Apply DepthQ

			if (dest->z > DPQ_S)
			{
				int v = 2048 - ((dest->z - DPQ_S) >> 16);

				r <<= 3;
				g <<= 3;
				b <<= 3;

				r *= v;
				g *= v;
				b *= v;

				r >>= 14;
				g >>= 14;
				b >>= 14;

				if (r < 0) r = 0;
				if (g < 0) g = 0;
				if (b < 0) b = 0;
			}

			dest->g = r << 10 | g << 5 | b;
		}
	}
	else if (numvert < 0)
	{
		// skip vertex count on vertex data

		objptr1 += 2;

		// calculate lighting for static mesh objects

		for (; numvert < 0; ++numvert, ++dest, ++objptr, objptr1 += 3)
		{
			// get static mesh light

			int r = smcr,
				g = smcg,
				b = smcb;

			// apply tint

			auto shade = 8192 - *(objptr);
			r *= shade;
			g *= shade;
			b *= shade;
			r >>= 13 + 3;
			g >>= 13 + 3;
			b >>= 13 + 3;

			// apply dynamic lights

			if (number_dynamics > 0 && CurrentMesh != nullptr)
			{
				int dx, dy, dz;
				int x1, y1, z1;
				int mod, dist;

				if (CurrentMesh->y_rot != 0)
				{
					phd_PushUnitMatrix();
					phd_RotY(CurrentMesh->y_rot);
					dx = phd_mxptr[M00] * objptr1[0] + phd_mxptr[M01] * objptr1[1] + phd_mxptr[M02] * objptr1[2];
					dz = phd_mxptr[M20] * objptr1[0] + phd_mxptr[M21] * objptr1[1] + phd_mxptr[M22] * objptr1[2];
					dx >>= 14;
					dz >>= 14;
					dx += CurrentMesh->x;
					dy = objptr1[1] + CurrentMesh->y;
					dz += CurrentMesh->z;
					phd_PopMatrix();
				}
				else
				{
					dx = CurrentMesh->x + objptr1[0];
					dy = CurrentMesh->y + objptr1[1];
					dz = CurrentMesh->z + objptr1[2];
				}

				for (int i = 0; i < number_dynamics; ++i)
				{
					if (dynamics[i].on)
					{
						z1 = (dynamics[i].z - dz) >> 7;

						mod = (z1 * z1);

						if (mod >= 0 && mod < 1024)
						{
							x1 = (dynamics[i].x - dx) >> 7;

							mod += (x1 * x1);

							if (mod >= 0 && mod < 1024)
							{
								y1 = (dynamics[i].y - dy) >> 7;

								mod += (y1 * y1);

								if (mod >= 0 && mod < 1024)
								{
									dist = SqrTable[mod];

									if (dist < dynamics[i].falloff >> 8)
									{
										r += RColorTable[dynamics[i].falloff >> 8][dist][dynamics[i].r] >> 10;
										g += GColorTable[dynamics[i].falloff >> 8][dist][dynamics[i].g] >> 5;
										b += BColorTable[dynamics[i].falloff >> 8][dist][dynamics[i].b];
									}
								}
							}
						}
					}
				}

			}

			if (r > 0x1f) r = 0x1f;
			if (g > 0x1f) g = 0x1f;
			if (b > 0x1f) b = 0x1f;

			// apply DepthQ

			if (dest->z > DPQ_S)
			{
				auto v = 2048 - ((dest->z - DPQ_S) >> 16);
				
				r <<= 3;
				g <<= 3;
				b <<= 3;

				r *= v;
				g *= v;
				b *= v;

				r >>= 14;
				g >>= 14;
				b >>= 14;

				if (r < 0) r = 0;
				if (g < 0) g = 0;
				if (b < 0) b = 0;
			}

			dest->g = r << 10 | g << 5 | b;
		}
	}

	return objptr;
}

void S_CalculateLight(int32_t x, int32_t y, int32_t z, int16_t room_number, ITEM_LIGHT* il)
{
	ITEM_LIGHT dummy;

	if (!il)
	{
		il = &dummy;
		il->init = 0;
	}

	LightPos[M00] = LightPos[M01] = LightPos[M02] =
					LightPos[M10] = LightPos[M11] = LightPos[M12] =
					LightPos[M20] = LightPos[M21] = LightPos[M22] = 0;

	LightCol[M00] = LightCol[M01] = LightCol[M02] =
					LightCol[M10] = LightCol[M11] = LightCol[M12] =
					LightCol[M20] = LightCol[M21] = LightCol[M22] = 0;

	LPos[0].x = LPos[0].y = LPos[0].z = 0;
	LPos[1].x = LPos[1].y = LPos[1].z = 0;
	LPos[2].x = LPos[2].y = LPos[2].z = 0;

	// determine brightest room light

	PHD_VECTOR ls, lc;

	auto r = &room[room_number];

	LIGHT_INFO* l = nullptr,
			  * sl = nullptr;

	auto light = r->light;

	int32_t brightest = -1;

	uint16_t ambience = (0x1FFF - (r->ambient >> 5)) + 1;

	bool sun = false;

	for (int i = r->num_lights; i > 0; --i, ++light)
	{
		// process sun bulb

		if (light->type)
		{
			sun = true;
			sl = light;
		}
		// process omni bulb
		else
		{
			lc.x = light->x - x;
			lc.y = light->y - y;
			lc.z = light->z - z;

			auto distance = (int32_t)phd_sqrt(SQUARE(lc.x) + SQUARE(lc.y) + SQUARE(lc.z));

			if (distance > light->l.spot.falloff)
				continue;

			auto shade = 8191 - (((light->l.spot.intensity * distance) / light->l.spot.falloff));

			ambience += (uint16_t)(shade >> (5 + 3));

			if (shade > brightest)
			{
				brightest = shade;
				l = light;
				ls = lc;
			}
		}
	}

	// interpolate sun bulb

	LIGHT_INFO sunlight;

	if (!sun && (il->init & 1))
	{
		sunlight.l.sun.nx = (int16_t)il->sun.x;
		sunlight.l.sun.ny = (int16_t)il->sun.y;
		sunlight.l.sun.nz = (int16_t)il->sun.z;

		sunlight.r = sunlight.g = sunlight.b = r->ambient >> 5;

		sl = &sunlight;
		sun = true;
	}

	if (sun)
	{
		if (il->init & 1)
		{
			il->sun.x += (sl->l.sun.nx - il->sun.x) >> 3;
			il->sun.y += (sl->l.sun.ny - il->sun.y) >> 3;
			il->sun.z += (sl->l.sun.nz - il->sun.z) >> 3;

			il->sunr += (sl->r - il->sunr) >> 3;
			il->sung += (sl->g - il->sung) >> 3;
			il->sunb += (sl->b - il->sunb) >> 3;
		}
		else
		{
			il->sun.x = sl->l.sun.nx;
			il->sun.y = sl->l.sun.ny;
			il->sun.z = sl->l.sun.nz;

			il->sunr = sl->r;
			il->sung = sl->g;
			il->sunb = sl->b;
			il->init |= 1;
		}

		LPos[0].x = il->sun.x;
		LPos[0].y = il->sun.y;
		LPos[0].z = il->sun.z;

		LightCol[M00] = il->sunr << 4;
		LightCol[M10] = il->sung << 4;
		LightCol[M20] = il->sunb << 4;
	}

	// interpolate omni bulb

	if ((brightest == -1) && (il->init & 2))
	{
		ls.x = il->bulb.x;
		ls.y = il->bulb.y;
		ls.z = il->bulb.z;

		sunlight.r = sunlight.g = sunlight.b = r->ambient >> 5;
		l = &sunlight;

		brightest = 8191;
	}

	if (brightest != -1)
	{
		phd_NormaliseVector(ls.x, ls.y, ls.z, (int32_t*)&ls);

		if (il->init & 2)
		{
			il->bulb.x += (ls.x - il->bulb.x) >> 3;
			il->bulb.y += (ls.y - il->bulb.y) >> 3;
			il->bulb.z += (ls.z - il->bulb.z) >> 3;

			il->bulbr += (uint8_t)((((l->r * brightest) >> 13) - il->bulbr) >> 3);
			il->bulbg += (uint8_t)((((l->g * brightest) >> 13) - il->bulbg) >> 3);
			il->bulbb += (uint8_t)((((l->b * brightest) >> 13) - il->bulbb) >> 3);
		}
		else
		{
			il->bulb.x = ls.x;
			il->bulb.y = ls.y;
			il->bulb.z = ls.z;

			il->bulbr = (uint8_t)((l->r * brightest) >> 13);
			il->bulbg = (uint8_t)((l->g * brightest) >> 13);
			il->bulbb = (uint8_t)((l->b * brightest) >> 13);

			il->init |= 2;
		}

		LPos[1].x = il->bulb.x >> 2;
		LPos[1].y = il->bulb.y >> 2;
		LPos[1].z = il->bulb.z >> 2;

		LightCol[M01] = il->bulbr << 4;
		LightCol[M11] = il->bulbg << 4;
		LightCol[M21] = il->bulbb << 4;
	}

	// determine brightest dynamic light

	DYNAMIC* dl2 = nullptr;

	auto dl = dynamics;

	// reset brightest for dynamics.

	brightest = -1;

	for (int i = 0; i < number_dynamics; ++i, ++dl)
	{
		lc.x = dl->x - x;
		lc.y = dl->y - y;
		lc.z = dl->z - z;

		if (abs(lc.x) > 8192 ||
			abs(lc.y) > 8192 ||
			abs(lc.z) > 8192)
			continue;

		auto distance = (int32_t)phd_sqrt(SQUARE(lc.x) + SQUARE(lc.y) + SQUARE(lc.z));

		if (distance > (dl->falloff >> 1))
			continue;

		auto shade = 8191 - (((8191 * distance) / (dl->falloff >> 1)));

		ambience += (uint16_t)(shade >> (5 + 2));

		if (shade > brightest)
		{
			brightest = shade;
			ls = lc;
			dl2 = dl;
		}
	}

	if (brightest != -1)
	{
		phd_NormaliseVector(ls.x, ls.y, ls.z, (int32_t*)&ls);

		if (il->init & 4)
		{
			il->dynamic.x += (ls.x - il->dynamic.x) >> 1;
			il->dynamic.y += (ls.y - il->dynamic.y) >> 1;
			il->dynamic.z += (ls.z - il->dynamic.z) >> 1;

			il->dynamicr += (uint8_t)((((dl2->r * brightest) >> (13 - 3)) - il->dynamicr) >> 1);
			il->dynamicg += (uint8_t)((((dl2->g * brightest) >> (13 - 3)) - il->dynamicg) >> 1);
			il->dynamicb += (uint8_t)((((dl2->b * brightest) >> (13 - 3)) - il->dynamicb) >> 1);
		}
		else
		{
			il->dynamic.x = ls.x;
			il->dynamic.y = ls.y;
			il->dynamic.z = ls.z;

			il->dynamicr = (uint8_t)((dl2->r * brightest) >> (13 - 3));
			il->dynamicg = (uint8_t)((dl2->g * brightest) >> (13 - 3));
			il->dynamicb = (uint8_t)((dl2->b * brightest) >> (13 - 3));
		}

		LPos[2].x = il->dynamic.x >> 2;
		LPos[2].y = il->dynamic.y >> 2;
		LPos[2].z = il->dynamic.z >> 2;

		LightCol[M02] = il->dynamicr << 6;
		LightCol[M12] = il->dynamicg << 6;
		LightCol[M22] = il->dynamicb << 6;
	}

	// determine ambience

	if (ambience > 255)
		ambience = 255;

	if (il->init & 8)
		il->ambient += (ambience - il->ambient) >> 3;
	else
	{
		il->ambient = (uint8_t)ambience;
		il->init |= 8;
	}

	smcr = il->ambient;
	smcg = il->ambient;
	smcb = il->ambient;
	
	LPos[0].x <<= 2;
	LPos[0].y <<= 2;
	LPos[0].z <<= 2;

	LPos[1].x <<= 2;
	LPos[1].y <<= 2;
	LPos[1].z <<= 2;

	LPos[2].x <<= 2;
	LPos[2].y <<= 2;
	LPos[2].z <<= 2;

	ApplyMatrix(w2v_matrix, &LPos[0], x, y, z);

	LPos[0].x = x;
	LPos[0].y = y;
	LPos[0].z = z;

	ApplyMatrix(w2v_matrix, &LPos[1], x, y, z);

	LPos[1].x = x;
	LPos[1].y = y;
	LPos[1].z = z;

	ApplyMatrix(w2v_matrix, &LPos[2], x, y, z);

	LPos[2].x = x;
	LPos[2].y = y;
	LPos[2].z = z;
}

void S_CalculateStaticLight(int16_t adder)
{
	smcr = (adder & 0x1f) << 3;
	smcg = (adder & 0x3e0) >> 2;
	smcb = (adder & 0x7c00) >> 7;
}