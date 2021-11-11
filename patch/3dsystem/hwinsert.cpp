#include <specific/standard.h>
#include <specific/drawprimitive.h>
#include <specific/hwrender.h>
#include <specific/global.h>

#include <main.h>

#include "hwinsert.h"

#define UV_MUL			(1.f / 65536.f)
#define SHADOW_BIAS		131072
#define RED_SHADE		128
#define GREEN_SHADE		224
#define ALPHA_OPAQUE	0xff000000
//#define NOLIGHTING

#define LimitBuffer() { if (CurrentTLVertex - VertexBuffer > MAX_TLVERTICES - 32) return; }

/**
* Set correct sort buffer ptrs for poly and increments
*/
#define	SetBufferPtrs(sort,info){\
									LimitBuffer();\
									sort    = sort3dptrbf;\
									info    = info3dptrbf;\
									surfacenumbf++;\
									sort3dptrbf += 10;\
									info3dptrbf += 5;\
								};

#define CLIP_GUV(OUT, B, A) {\
	OUT.ooz = B->ooz + (A->ooz - B->ooz) * clipper;\
	OUT.vr = FTOL(B->vr + (A->vr - B->vr) * clipper);\
	OUT.u = B->u + (A->u - B->u) * clipper;\
	OUT.vg = FTOL(B->vg + (A->vg - B->vg) * clipper);\
	OUT.v = B->v + (A->v - B->v) * clipper;\
	OUT.vb = FTOL(B->vb + (A->vb - B->vb) * clipper);}


#define CLIP_G(OUT, B, A) {\
	OUT.ooz = B->ooz + (A->ooz - B->ooz) * clipper;\
	OUT.vr = FTOL(B->vr + (A->vr - B->vr) * clipper);\
	OUT.vg = FTOL(B->vg + (A->vg - B->vg) * clipper);\
	OUT.vb = FTOL(B->vb + (A->vb - B->vb) * clipper);}

template <typename T, auto new_value>
struct temp_var
{
	T* value;
	T old_val;

	temp_var(T& v)	{ old_val = v; value = &(v = new_value); }
	~temp_var()		{ *value = old_val; }
};

template <typename T> inline void swap(T& a, T& b) { a ^= b; b ^= a; a ^= b; };

VERTEX_INFO XYG_output[8],
			XYGUV_output[8];

float UVTable[65536];

uint8_t g_gamma_ramp[256];

void refresh_gamma_ramp()
{
	auto v = 1.0 / (f_gamma * 0.1 * 4.0);

	for (int i = 0; i < 256; ++i)
	{
		auto v2 = (signed __int64)(std::pow((double)i * 0.00390625, v) * 256.0);
		g_gamma_ramp[i++] = uint8_t(v2);
	}
}

void InitUVTable()
{
	for (int i = 0; i < 65536; ++i)
		UVTable[i] = (float)(i + 1) * UV_MUL;

	refresh_gamma_ramp();
}

void InitBuckets()
{
	for (int i = 0; i < MAXBUCKETS; ++i)
		Bucket[i].init();
}

int FindBucket(DWORD tpage)
{
	// find bucket already in use

	for (int i = 0; i < MAXBUCKETS; ++i)
	{
		auto& b = Bucket[i];

		if (b.tpage == tpage && b.cnt < (VERTSPERBUCKET - 32))
			return i;
	}

	// no bucket in use/or full find next available

	for (int i = 0; i < MAXBUCKETS; ++i)
	{
		auto& b = Bucket[i];

		if (b.tpage == -1)
		{
			b.tpage = tpage;
			return i;
		}
	}

	return -1;
}

void DrawBuckets()
{
	for (int i = 0; i < MAXBUCKETS; ++i)
	{
		auto& b = Bucket[i];

		if (b.cnt != 0)
		{
			HWR_SetCurrentTexture(b.tpage);
			DrawPrimitive(D3DPT_TRIANGLELIST, D3DVT_TLVERTEX, &b.Vertex[0], b.cnt, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);

#ifdef _DEBUG
			++DrawPrimitiveCnt;
#endif
		}
	}
}

/**
* Cross product of point 1 and 3 with the origin to get plane through origin
* then see which side of this plane point 2 lies on 
*/
bool visible_zclip(PHD_VBUF *vn1, PHD_VBUF *vn2, PHD_VBUF *vn3)
{
	float a = (vn1->yv * vn3->zv) - (vn3->yv * vn1->zv),
		  b = (vn1->zv * vn3->xv) - (vn3->zv * vn1->xv),
		  c = (vn1->xv * vn3->yv) - (vn3->xv * vn1->yv);
	
	return ((a * vn2->xv) + (b * vn2->yv) + (c * vn2->zv) < 0.f);
}

DWORD ColorFromRGB(int nR, int nG, int nB)
{
	if (g_blue_effect)
	{
		nR *= RED_SHADE;
		nR >>= 8;
		nG *= GREEN_SHADE;
		nG >>= 8;
	}

	return RGB_MAKE(nR, nG, nB) | ALPHA_OPAQUE;
}

int GETR(int rgb) 
{
	if (!g_lighting_on)
		return 0xff;

	if (g_forced_red)
		return g_forced_red;

	int r = g_gamma_ramp[(rgb >> 7) & 0xF8];

	if (g_blue_effect)
	{
		r *= RED_SHADE;
		r >>= 8;
	}

	return r;
}

int GETG(int rgb) 
{
	if (!g_lighting_on)
		return 0xff;

	if (g_forced_green)
		return g_forced_green;

	int g = g_gamma_ramp[(rgb >> 2) & 0xF8];

	if (g_blue_effect)
	{
		g *= GREEN_SHADE;
		g >>= 8;
	}

	return g;
}

int GETB(int rgb)
{
	if (!g_lighting_on)
		return 0xff;

	if (g_forced_blue)
		return g_forced_blue;

	return g_gamma_ramp[8 * (rgb & 0x1F)];
}

unsigned long VertexRGB(int rgb)
{  
	return RGBA_MAKE(GETR(rgb), GETG(rgb), GETB(rgb), 0xff);
}

/**
* Add a textured poly from v_buffer to sort list
*/
void HWI_InsertClippedPoly_Textured(int nPoints, float zdepth, int nDrawType, int nTPage)
{
	auto pVI = v_buffer;
	
	if (App.lpZBuffer == 0 || nDrawType == DRAW_TLV_GTA || nDrawType == DRAW_TLV_WGT)
	{
		double fconv;
		int32_t* sort;
		int16_t* info;

		// add verts to sort list

		SetBufferPtrs(sort, info);

		sort[0] = (int32_t)info;
		sort[1] = (int32_t)FTOL(zdepth);
		info[0] = nDrawType;
		info[1] = nTPage;
		info[2] = nPoints;

		auto pTLV = CurrentTLVertex;

		*((D3DTLVERTEX**)(info + 3)) = pTLV;

		for (int nPoint = nPoints; nPoint--;)
		{
			pTLV->sx = pVI->x;
			pTLV->sy = pVI->y;
			pTLV->sz = f_a - f_boo * pVI->ooz;
			pTLV->rhw = pVI->ooz;
			pTLV->color = RGB_MAKE(pVI->vr, pVI->vg, pVI->vb);

			float newz = UV_MUL / pVI->ooz;

			pTLV->tu = pVI->u * newz;
			pTLV->tv = pVI->v * newz;

			++pVI;
			++pTLV;
		}
	
		CurrentTLVertex = pTLV;
	}
	else
	{
		// put verts into texture bucket
		// fans so do the first 3 verts then add remainder
		
		int b = FindBucket(GahTextureHandle[nTPage]);
		if (b == -1)
			return;

		for (int i = 0; i < 3; ++i)
		{
			const int& v = Bucket[b].cnt;			

			Bucket[b].Vertex[v].sx = pVI->x;
			Bucket[b].Vertex[v].sy = pVI->y;
			Bucket[b].Vertex[v].sz = f_a - f_boo * pVI->ooz;
			Bucket[b].Vertex[v].rhw = pVI->ooz;
			Bucket[b].Vertex[v].color = RGB_MAKE(pVI->vr, pVI->vg, pVI->vb);

			float newz = UV_MUL / pVI->ooz;

			Bucket[b].Vertex[v].tu = pVI->u * newz;
			Bucket[b].Vertex[v].tv = pVI->v * newz;
			
			++pVI;
			++Bucket[b].cnt;
		}

		nPoints -= 3;
		
		--pVI;

		for (int i = nPoints; i > 0; --i)
		{
			const int& v = Bucket[b].cnt;

			Bucket[b].Vertex[v + 0].sx = v_buffer->x;
			Bucket[b].Vertex[v + 0].sy = v_buffer->y;
			Bucket[b].Vertex[v + 0].sz = f_a - f_boo * v_buffer->ooz;
			Bucket[b].Vertex[v + 0].rhw = v_buffer->ooz;
			Bucket[b].Vertex[v + 0].color = RGB_MAKE(v_buffer->vr, v_buffer->vg, v_buffer->vb);

			float newz = UV_MUL / v_buffer->ooz;

			Bucket[b].Vertex[v + 0].tu = v_buffer->u * newz;
			Bucket[b].Vertex[v + 0].tv = v_buffer->v * newz;

			Bucket[b].Vertex[v + 1].sx = pVI->x;
			Bucket[b].Vertex[v + 1].sy = pVI->y;
			Bucket[b].Vertex[v + 1].sz = f_a - f_boo * pVI->ooz;
			Bucket[b].Vertex[v + 1].rhw = pVI->ooz;

			newz = UV_MUL / pVI->ooz;

			Bucket[b].Vertex[v + 1].color = RGB_MAKE(pVI->vr, pVI->vg, pVI->vb);
			Bucket[b].Vertex[v + 1].tu = pVI->u * newz;
			Bucket[b].Vertex[v + 1].tv = pVI->v * newz;

			++pVI;

			Bucket[b].Vertex[v + 2].sx = pVI->x;
			Bucket[b].Vertex[v + 2].sy = pVI->y;
			Bucket[b].Vertex[v + 2].sz = f_a - f_boo * pVI->ooz;
			Bucket[b].Vertex[v + 2].rhw = pVI->ooz;

			newz = UV_MUL / pVI->ooz;

			Bucket[b].Vertex[v + 2].color = RGB_MAKE(pVI->vr, pVI->vg, pVI->vb);
			Bucket[b].Vertex[v + 2].tu = pVI->u * newz;
			Bucket[b].Vertex[v + 2].tv = pVI->v * newz;

			Bucket[b].cnt += 3;
		}
	}
}

void PHD_VBUF_To_POINT_INFO(PHD_VBUF* pV, POINT_INFO* p, uint16_t* pUV)
{
	p->xv = pV->xv;
	p->yv = pV->yv;
	p->zv = pV->zv;
	p->ooz = pV->ooz;
	p->xs = pV->xs;
	p->ys = pV->ys;
	p->vr = GETR(pV->g);
	p->vg = GETG(pV->g);
	p->vb = GETB(pV->g);
	p->u = (float)pUV[0];
	p->v = (float)pUV[1];
}

void PHD_VBUF_To_POINT_INFO(PHD_VBUF* pV, POINT_INFO* p)
{
	p->xv = pV->xv;
	p->yv = pV->yv;
	p->zv = pV->zv;
	p->ooz = pV->ooz;
	p->xs = pV->xs;
	p->ys = pV->ys;
	p->vr = GETR(pV->g);
	p->vg = GETG(pV->g);
	p->vb = GETB(pV->g);
}

void PHD_VBUF_To_VERTEX_INFO(PHD_VBUF* pV, VERTEX_INFO* p, uint16_t* pUV)
{
	p->x = pV->xs;
	p->y = pV->ys;
	p->ooz = pV->ooz;
	p->u = pUV[0] * pV->ooz;
	p->v = pUV[1] * pV->ooz;
	p->vr = GETR(pV->g);
	p->vg = GETG(pV->g);
	p->vb = GETB(pV->g);
}

void PHD_VBUF_To_VERTEX_INFO(PHD_VBUF* pV, VERTEX_INFO* p)
{
	p->x = pV->xs;
	p->y = pV->ys;
	p->ooz = pV->ooz;
	p->vr = GETR(pV->g);
	p->vg = GETG(pV->g);
	p->vb = GETB(pV->g);
}

void PHD_VBUF_To_TLVERT(PHD_VBUF* pV, D3DTLVERTEX* p, uint16_t* pUV)
{
	p->sx = pV->xs;
	p->sy = pV->ys;
	p->sz = f_a - f_boo * pV->ooz;
	p->rhw = pV->ooz;
	p->color = VertexRGB(pV->g);
	p->tu = UVTable[pUV[0]]; //pUV[0]*UV_MUL;
	p->tv = UVTable[pUV[1]]; //pUV[1]*UV_MUL;
}

void PHD_VBUF_To_TLVERT(PHD_VBUF* pV, D3DTLVERTEX* p)
{
	p->sx = pV->xs;
	p->sy = pV->ys;
	p->sz = f_a - f_boo * pV->ooz;
	p->rhw = pV->ooz;
	p->color = VertexRGB(pV->g);
}

void InsertGT3(PHD_VBUF* pV1, PHD_VBUF* pV2, PHD_VBUF* pV3, PHDTEXTURESTRUCT* pTex, uint16_t* pUV1, uint16_t* pUV2, uint16_t* pUV3, sort_type nSortType, unsigned short dbl)
{
	int nPoints = 0;

	if ((pV1->clip | pV2->clip | pV3->clip) < 0)
	{
		POINT_INFO aPoint[3];

		PHD_VBUF_To_POINT_INFO(pV1, &aPoint[0], pUV1);
		PHD_VBUF_To_POINT_INFO(pV2, &aPoint[1], pUV2);
		PHD_VBUF_To_POINT_INFO(pV3, &aPoint[2], pUV3);

		if ((nPoints = RoomZedClipper(3, aPoint, v_buffer)) != 0)
			goto doxyclip;

		return;
	}

	IF_NOT_VIS(pV1, pV2, pV3)
	{
		if (!dbl)
			return;

		swap((long&)pV2, (long&)pV3);
		swap((long&)pUV2, (long&)pUV3);
	}

	float zdepth;

	if (pV1->clip | pV2->clip | pV3->clip)
	{
		PHD_VBUF_To_VERTEX_INFO(pV1, &v_buffer[0], pUV1);
		PHD_VBUF_To_VERTEX_INFO(pV2, &v_buffer[1], pUV2);
		PHD_VBUF_To_VERTEX_INFO(pV3, &v_buffer[2], pUV3);

		nPoints = 3;

doxyclip:

		nPoints = RoomXYGUVClipper(nPoints, v_buffer);

		if (nPoints == 0)
			return;

		switch (nSortType)
		{
		case FAR_SORT: TRIA_DEPTH_FAR(pV1, pV2, pV3); break;
		case MID_SORT: TRIA_DEPTH_MID(pV1, pV2, pV3); break;
		default:	   zdepth = BACK_DEPTH;
		}

		HWI_InsertClippedPoly_Textured(nPoints, zdepth, pTex->drawtype <= 1 ? pTex->drawtype + DRAW_TLV_GT : DRAW_TLV_GTA, pTex->tpage);
	}
	else
	{
		switch (nSortType)
		{
		case FAR_SORT: TRIA_DEPTH_FAR(pV1, pV2, pV3); break;
		case MID_SORT: TRIA_DEPTH_MID(pV1, pV2, pV3); break;
		default:	   zdepth = BACK_DEPTH;
		}

		int drawtype = (pTex->drawtype <= 1 ? pTex->drawtype + DRAW_TLV_GT : DRAW_TLV_GTA);

		if (App.lpZBuffer == 0 || drawtype == DRAW_TLV_GTA || drawtype == DRAW_TLV_WGT)
		{
			int32_t* sort;
			int16_t* info;
			double fconv;

			SetBufferPtrs(sort, info);

			sort[0] = (int32_t)info;
			sort[1] = (int32_t)FTOL(zdepth);
			info[0] = drawtype;
			info[1] = pTex->tpage;
			info[2] = 3;

			auto pTLV = CurrentTLVertex;

			*((D3DTLVERTEX**)(info + 3)) = pTLV;

			PHD_VBUF_To_TLVERT(pV1, pTLV, pUV1); ++pTLV;
			PHD_VBUF_To_TLVERT(pV2, pTLV, pUV2); ++pTLV;
			PHD_VBUF_To_TLVERT(pV3, pTLV, pUV3); ++pTLV;

			CurrentTLVertex = pTLV;
		}
		else
		{
			if (int b = FindBucket(GahTextureHandle[pTex->tpage]); b != -1)
			{
				auto& v = Bucket[b];

				PHD_VBUF_To_TLVERT(pV1, &v.Vertex[Bucket[b].cnt], pUV1); ++v.cnt;
				PHD_VBUF_To_TLVERT(pV2, &v.Vertex[Bucket[b].cnt], pUV2); ++v.cnt;
				PHD_VBUF_To_TLVERT(pV3, &v.Vertex[Bucket[b].cnt], pUV3); ++v.cnt;
			}
		}
	}
}

void InsertGT4(PHD_VBUF* pV1, PHD_VBUF* pV2, PHD_VBUF* pV3, PHD_VBUF* pV4, PHDTEXTURESTRUCT* pTex, sort_type nSortType, unsigned short dbl)
{
	if (pV1->clip & pV2->clip & pV3->clip & pV4->clip)
		return;

	if ((pV1->clip | pV2->clip | pV3->clip | pV4->clip) < 0)
	{
		if (!dbl)
		{
			if (visible_zclip(pV1, pV2, pV3))
			{
				InsertGT3(pV1, pV2, pV3, pTex, &pTex->u1, &pTex->u2, &pTex->u3, nSortType, 0);
				InsertGT3(pV1, pV3, pV4, pTex, &pTex->u1, &pTex->u3, &pTex->u4, nSortType, 0);
			}
		}
		else
		{
			if (visible_zclip(pV1, pV2, pV3))
			{
				InsertGT3(pV1, pV2, pV3, pTex, &pTex->u1, &pTex->u2, &pTex->u3, nSortType, 0);
				InsertGT3(pV1, pV3, pV4, pTex, &pTex->u1, &pTex->u3, &pTex->u4, nSortType, 0);
			}

			swap((long&)pV1, (long&)pV3);

			if (visible_zclip(pV1, pV2, pV3))
			{
				InsertGT3(pV1, pV2, pV3, pTex, &pTex->u3, &pTex->u2, &pTex->u1, nSortType, 0);
				InsertGT3(pV1, pV3, pV4, pTex, &pTex->u3, &pTex->u1, &pTex->u4, nSortType, 0);
			}
		}

		return;
	}

	IF_NOT_VIS(pV1, pV2, pV3)
	{
		if (!dbl)
			return;

		swap((long&)pV1, (long&)pV3);

		if (pV1->clip | pV2->clip | pV3->clip | pV4->clip)
		{
			InsertGT3(pV1, pV2, pV3, pTex, &pTex->u3, &pTex->u2, &pTex->u1, nSortType, 0);
			InsertGT3(pV1, pV3, pV4, pTex, &pTex->u3, &pTex->u1, &pTex->u4, nSortType, 0);

			return;
		}
	}
	else dbl = 0;

	if (pV1->clip | pV2->clip | pV3->clip | pV4->clip)
	{
		InsertGT3(pV1, pV2, pV3, pTex, &pTex->u1, &pTex->u2, &pTex->u3, nSortType, dbl);
		InsertGT3(pV1, pV3, pV4, pTex, &pTex->u1, &pTex->u3, &pTex->u4, nSortType, dbl);
		return;
	}

	float zdepth;

	switch (nSortType)
	{
	case FAR_SORT: QUAD_DEPTH_FAR(pV1, pV2, pV3, pV4); break;
	case MID_SORT: QUAD_DEPTH_MID(pV1, pV2, pV3, pV4); break;
	default:	   zdepth = BACK_DEPTH;
	}

	int drawtype = (pTex->drawtype <= 1 ? pTex->drawtype + DRAW_TLV_GT : DRAW_TLV_GTA);

	if (App.lpZBuffer == 0 || drawtype == DRAW_TLV_GTA || drawtype == DRAW_TLV_WGT)
	{
		int32_t* sort;
		int16_t* info;
		double fconv;

		SetBufferPtrs(sort, info);

		sort[0] = (int32_t)info;
		sort[1] = (int32_t)FTOL(zdepth);

		info[0] = drawtype;
		info[1] = pTex->tpage;
		info[2] = 4;

		auto pTLV = CurrentTLVertex;

		*((D3DTLVERTEX**)(info + 3)) = pTLV;

		if (!dbl)
		{
			PHD_VBUF_To_TLVERT(pV1, pTLV);
			pTLV->tu = UVTable[pTex->u1]; //*UV_MUL;
			pTLV->tv = UVTable[pTex->v1];//*UV_MUL;
			++pTLV;
			PHD_VBUF_To_TLVERT(pV2, pTLV);
			pTLV->tu = UVTable[pTex->u2];//*UV_MUL;
			pTLV->tv = UVTable[pTex->v2];//*UV_MUL;
			++pTLV;
			PHD_VBUF_To_TLVERT(pV3, pTLV);
			pTLV->tu = UVTable[pTex->u3];//*UV_MUL;
			pTLV->tv = UVTable[pTex->v3];//*UV_MUL;
		}
		else
		{
			PHD_VBUF_To_TLVERT(pV1, pTLV);
			pTLV->tu = UVTable[pTex->u3];//*UV_MUL;
			pTLV->tv = UVTable[pTex->v3];//*UV_MUL;
			++pTLV;
			PHD_VBUF_To_TLVERT(pV2, pTLV);
			pTLV->tu = UVTable[pTex->u2];//*UV_MUL;
			pTLV->tv = UVTable[pTex->v2];//*UV_MUL;
			++pTLV;
			PHD_VBUF_To_TLVERT(pV3, pTLV);
			pTLV->tu = UVTable[pTex->u1];//*UV_MUL;
			pTLV->tv = UVTable[pTex->v1];//*UV_MUL;
		}

		++pTLV;

		PHD_VBUF_To_TLVERT(pV4, pTLV);

		pTLV->tu = UVTable[pTex->u4];//*UV_MUL;
		pTLV->tv = UVTable[pTex->v4];//*UV_MUL;

		++pTLV;

		CurrentTLVertex = pTLV;
	}
	else
	{
		if (int b = FindBucket(GahTextureHandle[pTex->tpage]); b != -1)
		{
			auto& v = Bucket[b];

			if (!dbl)
			{
				PHD_VBUF_To_TLVERT(pV1, &v.Vertex[v.cnt]);

				v.Vertex[v.cnt].tu = UVTable[pTex->u1]; //*UV_MUL;
				v.Vertex[v.cnt].tv = UVTable[pTex->v1];//*UV_MUL;

				++v.cnt;

				PHD_VBUF_To_TLVERT(pV2, &v.Vertex[v.cnt]);

				v.Vertex[v.cnt].tu = UVTable[pTex->u2]; //*UV_MUL;
				v.Vertex[v.cnt].tv = UVTable[pTex->v2]; //*UV_MUL;

				++v.cnt;

				PHD_VBUF_To_TLVERT(pV3, &v.Vertex[v.cnt]);

				v.Vertex[v.cnt].tu = UVTable[pTex->u3];//*UV_MUL;
				v.Vertex[v.cnt].tv = UVTable[pTex->v3];//*UV_MUL;

				++v.cnt;

				PHD_VBUF_To_TLVERT(pV1, &v.Vertex[v.cnt]);

				v.Vertex[v.cnt].tu = UVTable[pTex->u1];//*UV_MUL;
				v.Vertex[v.cnt].tv = UVTable[pTex->v1];//*UV_MUL;

				++v.cnt;

				PHD_VBUF_To_TLVERT(pV3, &v.Vertex[v.cnt]);

				v.Vertex[v.cnt].tu = UVTable[pTex->u3];//*UV_MUL;
				v.Vertex[v.cnt].tv = UVTable[pTex->v3];//*UV_MUL;

				++v.cnt;

				PHD_VBUF_To_TLVERT(pV4, &v.Vertex[v.cnt]);

				v.Vertex[v.cnt].tu = UVTable[pTex->u4];//*UV_MUL;
				v.Vertex[v.cnt].tv = UVTable[pTex->v4];//*UV_MUL;

				++v.cnt;
			}
			else
			{

				PHD_VBUF_To_TLVERT(pV1, &v.Vertex[v.cnt]);

				v.Vertex[v.cnt].tu = UVTable[pTex->u3];//*UV_MUL;
				v.Vertex[v.cnt].tv = UVTable[pTex->v3];//*UV_MUL;

				++v.cnt;

				PHD_VBUF_To_TLVERT(pV2, &v.Vertex[v.cnt]);

				v.Vertex[v.cnt].tu = UVTable[pTex->u2];//*UV_MUL;
				v.Vertex[v.cnt].tv = UVTable[pTex->v2];//*UV_MUL;

				++v.cnt;

				PHD_VBUF_To_TLVERT(pV3, &v.Vertex[v.cnt]);

				v.Vertex[v.cnt].tu = UVTable[pTex->u1];//*UV_MUL;
				v.Vertex[v.cnt].tv = UVTable[pTex->v1];//*UV_MUL;

				++v.cnt;

				PHD_VBUF_To_TLVERT(pV1, &v.Vertex[v.cnt]);

				v.Vertex[v.cnt].tu = UVTable[pTex->u3];//*UV_MUL;
				v.Vertex[v.cnt].tv = UVTable[pTex->v3];//*UV_MUL;

				++v.cnt;

				PHD_VBUF_To_TLVERT(pV3, &v.Vertex[v.cnt]);

				v.Vertex[v.cnt].tu = UVTable[pTex->u1];//*UV_MUL;
				v.Vertex[v.cnt].tv = UVTable[pTex->v1];//*UV_MUL;

				++v.cnt;

				PHD_VBUF_To_TLVERT(pV4, &v.Vertex[v.cnt]);

				v.Vertex[v.cnt].tu = UVTable[pTex->u4];//*UV_MUL;
				v.Vertex[v.cnt].tv = UVTable[pTex->v4];//*UV_MUL;

				++v.cnt;
			}
		}
	}
}

int16_t* InsertObjectGT3(int16_t* pFaceInfo, int nFaces, enum sort_type nSortType)
{
	while (nFaces)
	{
		PHDTEXTURESTRUCT* pTex = phdtextinfo + (pFaceInfo[3] & 0x7fff);

		InsertGT3(vbuf + pFaceInfo[0],
				  vbuf + pFaceInfo[1],
				  vbuf + pFaceInfo[2],
				  pTex,
				  &pTex->u1,
				  &pTex->u2,
				  &pTex->u3,
				  nSortType,
				  (pFaceInfo[3] >> 15) & 1);

		--nFaces;

		pFaceInfo += 4;
	}

	return pFaceInfo;
}

int16_t* InsertObjectGT4(int16_t* pFaceInfo, int nFaces, sort_type nSortType)
{
	while (nFaces)
	{
		InsertGT4(vbuf + pFaceInfo[0],
				  vbuf + pFaceInfo[1],
				  vbuf + pFaceInfo[2],
				  vbuf + pFaceInfo[3],
				  phdtextinfo + (pFaceInfo[4] & 0x7fff),
				  nSortType,
				  (pFaceInfo[4] >> 15) & 1);

		--nFaces;

		pFaceInfo += 5;
	}

	return pFaceInfo;
}

void HWI_InsertPoly_Gouraud(int nPoints, float zdepth, int nR, int nG, int nB, int nDrawType)
{
	const uint32_t dwAnd = (nDrawType == DRAW_TLV_GA) ? 0x80ffffff : 0xffffffff;

	VERTEX_INFO* pVI = v_buffer;

	if (App.lpZBuffer == 0 || nDrawType == DRAW_TLV_GA)
	{
		int32_t* sort;
		int16_t* info;
		double fconv;

		SetBufferPtrs(sort, info);

		sort[0] = (int32_t)info;
		sort[1] = (int32_t)FTOL(zdepth);
		info[0] = nDrawType;
		info[1] = 0;
		info[2] = nPoints;

		auto pTLV = CurrentTLVertex;

		*((D3DTLVERTEX**)(info + 3)) = pTLV;

		while (nPoints--)
		{
			pTLV->sx = pVI->x;
			pTLV->sy = pVI->y;
			pTLV->sz = f_a - f_boo * pVI->ooz;
			pTLV->rhw = pVI->ooz;

			pVI->vr = (pVI->vr * nR) >> 8;
			pVI->vg = (pVI->vg * nG) >> 8;
			pVI->vb = (pVI->vb * nB) >> 8;

			pTLV->color = RGBA_MAKE(pVI->vr, pVI->vg, pVI->vb, 0xff) & dwAnd;

			++pVI;
			++pTLV;
		}

		CurrentTLVertex = pTLV;
	}
	else
	{
		// put verts into texture bucket
		// fans so do the first 3 verts then add remainder

		if (int b = FindBucket(0); b != -1)
		{
			auto& v = Bucket[b];

			for (int i = 0; i < 3; ++i)
			{
				int cnt = v.cnt;

				pVI->vr = (pVI->vr * nR) >> 8;
				pVI->vg = (pVI->vg * nG) >> 8;
				pVI->vb = (pVI->vb * nB) >> 8;

				v.Vertex[cnt].sx = pVI->x;
				v.Vertex[cnt].sy = pVI->y;
				v.Vertex[cnt].sz = f_a - f_boo * pVI->ooz;
				v.Vertex[cnt].rhw = pVI->ooz;
				v.Vertex[cnt].color = RGBA_MAKE(pVI->vr, pVI->vg, pVI->vb, 0xff) & dwAnd;

				++pVI;
				++v.cnt;
			}

			nPoints -= 3;

			--pVI;

			for (int i = nPoints; i > 0; --i)
			{
				int cnt = v.cnt;

				v.Vertex[cnt + 0].sx = v_buffer->x;
				v.Vertex[cnt + 0].sy = v_buffer->y;
				v.Vertex[cnt + 0].sz = f_a - f_boo * v_buffer->ooz;
				v.Vertex[cnt + 0].rhw = v_buffer->ooz;
				v.Vertex[cnt + 0].color = RGBA_MAKE(v_buffer->vr, v_buffer->vg, v_buffer->vb, 0xff) & dwAnd;

				v.Vertex[cnt + 1].sx = pVI->x;
				v.Vertex[cnt + 1].sy = pVI->y;
				v.Vertex[cnt + 1].sz = f_a - f_boo * pVI->ooz;
				v.Vertex[cnt + 1].rhw = pVI->ooz;
				v.Vertex[cnt + 1].color = RGBA_MAKE(pVI->vr, pVI->vg, pVI->vb, 0xff) & dwAnd;

				++pVI;

				pVI->vr = (pVI->vr * nR) >> 8;
				pVI->vg = (pVI->vg * nG) >> 8;
				pVI->vb = (pVI->vb * nB) >> 8;

				v.Vertex[cnt + 2].sx = pVI->x;
				v.Vertex[cnt + 2].sy = pVI->y;
				v.Vertex[cnt + 2].sz = f_a - f_boo * pVI->ooz;
				v.Vertex[cnt + 2].rhw = pVI->ooz;

				v.Vertex[cnt + 2].color = RGBA_MAKE(pVI->vr, pVI->vg, pVI->vb, 0xff) & dwAnd;

				v.cnt += 3;
			}
		}
	}
}

int16_t* InsertObjectG3(int16_t* pFaceInfo, int nFaces, sort_type nSortType)
{
	int nPoints;

	for (; nFaces; --nFaces, pFaceInfo += 4)
	{
		auto pV1 = vbuf + pFaceInfo[0],
			 pV2 = vbuf + pFaceInfo[1],
			 pV3 = vbuf + pFaceInfo[2];

		if (pV1->clip & pV2->clip & pV3->clip)
			continue;

		if ((pV1->clip | pV2->clip | pV3->clip) < 0)
		{
			if (!visible_zclip(pV1, pV2, pV3))
				continue;

			POINT_INFO aPoint[3];

			PHD_VBUF_To_POINT_INFO(pV1, &aPoint[0]);
			PHD_VBUF_To_POINT_INFO(pV2, &aPoint[1]);
			PHD_VBUF_To_POINT_INFO(pV3, &aPoint[2]);

			nPoints = RoomZedClipper(3, aPoint, v_buffer);

			if (nPoints)
				nPoints = XYGClipper(nPoints, v_buffer);
		}
		else
		{
			IF_NOT_VIS(pV1, pV2, pV3)
				continue;

			PHD_VBUF_To_VERTEX_INFO(pV1, &v_buffer[0]);
			PHD_VBUF_To_VERTEX_INFO(pV2, &v_buffer[1]);
			PHD_VBUF_To_VERTEX_INFO(pV3, &v_buffer[2]);

			nPoints = 3;

			if (pV1->clip | pV2->clip | pV3->clip)
				nPoints = XYGClipper(nPoints, v_buffer);
		}

		if (nPoints)
		{
			auto pPE = G_GouraudPalette + ((pFaceInfo[3] >> 6) & 0x3fc);

			float zdepth;

			switch (nSortType)
			{
			case FAR_SORT: TRIA_DEPTH_FAR(pV1, pV2, pV3); break;
			case MID_SORT: TRIA_DEPTH_MID(pV1, pV2, pV3); break;
			default:	   zdepth = BACK_DEPTH;
			}

			HWI_InsertPoly_Gouraud(nPoints, zdepth, pPE[0], pPE[1], pPE[2], DRAW_TLV_G);
		}
	}

	return pFaceInfo;
}

int16_t* InsertObjectG4(int16_t* pFaceInfo, int nFaces, sort_type nSortType)
{
	int nPoints;

	for (; nFaces; --nFaces, pFaceInfo += 5)
	{
		auto pV1 = vbuf + pFaceInfo[0],
			 pV2 = vbuf + pFaceInfo[1],
			 pV3 = vbuf + pFaceInfo[2],
			 pV4 = vbuf + pFaceInfo[3];

		if (pV1->clip & pV2->clip & pV3->clip & pV4->clip)
			continue;

		if ((pV1->clip | pV2->clip | pV3->clip | pV4->clip) < 0)
		{
			if (!visible_zclip(pV1, pV2, pV3))
				continue;

			POINT_INFO aPoint[4];

			PHD_VBUF_To_POINT_INFO(pV1, &aPoint[0]);
			PHD_VBUF_To_POINT_INFO(pV2, &aPoint[1]);
			PHD_VBUF_To_POINT_INFO(pV3, &aPoint[2]);
			PHD_VBUF_To_POINT_INFO(pV4, &aPoint[3]);

			if (nPoints = RoomZedClipper(4, aPoint, v_buffer); nPoints != 0)
				nPoints = XYGClipper(nPoints, v_buffer);
		}
		else
		{
			IF_NOT_VIS(pV1, pV2, pV3)
				continue;

			PHD_VBUF_To_VERTEX_INFO(pV1, &v_buffer[0]);
			PHD_VBUF_To_VERTEX_INFO(pV2, &v_buffer[1]);
			PHD_VBUF_To_VERTEX_INFO(pV3, &v_buffer[2]);
			PHD_VBUF_To_VERTEX_INFO(pV4, &v_buffer[3]);

			nPoints = 4;

			if (pV1->clip | pV2->clip | pV3->clip | pV4->clip)
				nPoints = XYGClipper(nPoints, v_buffer);
		}

		if (nPoints)
		{
			auto pPE = G_GouraudPalette + ((pFaceInfo[4] >> 6) & 0x3fc);

			float zdepth;

			switch (nSortType)
			{
			case FAR_SORT: QUAD_DEPTH_FAR(pV1, pV2, pV3, pV4); break;
			case MID_SORT: QUAD_DEPTH_MID(pV1, pV2, pV3, pV4); break;
			default:	   zdepth = BACK_DEPTH;
			}

			HWI_InsertPoly_Gouraud(nPoints, zdepth, pPE[0], pPE[1], pPE[2], DRAW_TLV_G);
		}
	}

	return pFaceInfo;
}

void InsertSprite(int nZDepth, int nX1, int nY1, int nX2, int nY2, int16_t* nSprite, int nShade, int nShade1, int drawtype, int offset)
{
	if (nX2 <= nX1 || nY2 <= nY1 || nX2 <= 0 || nY2 <= 0 ||
		nX1 >= phd_winxmax || nY1 >= phd_winymax)
		return;

	nX1 += phd_winxmin;
	nX2 += phd_winxmin;
	nY1 += phd_winymin;
	nY2 += phd_winymin;

	if (nZDepth < phd_znear)
		nZDepth = phd_znear;

	if (nZDepth >= phd_zfar)
		return;

	auto pSprite = (PHDSPRITESTRUCT*)nSprite;

	float fZDepth = (float)nZDepth,
		  ooz = one / fZDepth,
		  fShade = (float)nShade,
		  fU1 = (float)((pSprite->offset << 8) & 0xff00),
		  fV1 = (float)(pSprite->offset & 0xff00),
		  fU2 = fU1 + pSprite->width,
		  fV2 = fV1 + pSprite->height,
		  fUVAdd = (float)App.nUVAdd;

	fU1 += fUVAdd;
	fU2 -= fUVAdd;
	fV1 += fUVAdd;
	fV2 -= fUVAdd;
	fU1 *= ooz;
	fV1 *= ooz;
	fU2 *= ooz;
	fV2 *= ooz;

	v_buffer[0].x = (float)(nX1 + offset);
	v_buffer[0].y = (float)nY1;
	v_buffer[0].ooz = ooz;
	v_buffer[0].vr = GETR(nShade);
	v_buffer[0].vg = GETG(nShade);
	v_buffer[0].vb = GETB(nShade);
	v_buffer[0].u = fU1;
	v_buffer[0].v = fV1;

	v_buffer[1].x = (float)(nX2 + offset);
	v_buffer[1].y = (float)nY1;
	v_buffer[1].ooz = ooz;
	v_buffer[1].u = fU2;
	v_buffer[1].v = fV1;
	v_buffer[1].vr = GETR(nShade);
	v_buffer[1].vg = GETG(nShade);
	v_buffer[1].vb = GETB(nShade);

	if (nShade1 != -1) nShade = nShade1;

	v_buffer[2].x = (float)nX2;
	v_buffer[2].y = (float)nY2;
	v_buffer[2].ooz = ooz;
	v_buffer[2].u = fU2;
	v_buffer[2].v = fV2;
	v_buffer[2].vr = GETR(nShade);
	v_buffer[2].vg = GETG(nShade);
	v_buffer[2].vb = GETB(nShade);

	v_buffer[3].x = (float)nX1;
	v_buffer[3].y = (float)nY2;
	v_buffer[3].ooz = ooz;
	v_buffer[3].u = fU1;
	v_buffer[3].v = fV2;
	v_buffer[3].vr = GETR(nShade);
	v_buffer[3].vg = GETG(nShade);
	v_buffer[3].vb = GETB(nShade);

	int nPoints = 4;

	if (nX1 < phd_winxmin || nX2 > phd_winxmin + phd_winwidth || nY1 < phd_winymin || nY2 > phd_winymin + phd_winheight)
	{
		phd_leftfloat = (float)phd_winxmin;
		phd_rightfloat = (float)(phd_winxmin + phd_winwidth);
		phd_topfloat = (float)phd_winymin;
		phd_bottomfloat = (float)(phd_winymin + phd_winheight);

		nPoints = RoomXYGUVClipper(nPoints, v_buffer);
	}

	if (nPoints)
	{
		temp_var<bool, false> blue_effect(g_blue_effect);
		HWI_InsertClippedPoly_Textured(nPoints, fZDepth, drawtype, pSprite->tpage);
	}
}

void InsertFlatRect(int32_t nX1, int32_t nY1, int32_t nX2, int32_t nY2, int nZDepth, int nColour)
{
	if ((nX2 <= nX1) || (nY2 <= nY1))
		return;

	if (nX1 < phd_winxmin)						nX1 = phd_winxmin;
	if (nY1 < phd_winymin)						nY1 = phd_winymin;
	if (nX2 > (phd_winxmin + phd_winwidth))		nX2 = phd_winxmin + phd_winwidth;
	if (nY2 > (phd_winymin + phd_winheight))	nX2 = phd_winymin + phd_winheight;

	uint8_t* pPE = game_palette + nColour * 3;

	if (App.lpZBuffer == 0)
	{
		int32_t* sort;
		int16_t* info;
		double fconv;

		SetBufferPtrs(sort, info);

		sort[0] = (int32_t)info;
		sort[1] = (int32_t)FTOL(nZDepth);
		info[0] = DRAW_TLV_G;
		info[1] = 0;
		info[2] = 4;

		auto pTLV = CurrentTLVertex;

		*((D3DTLVERTEX**)(info + 3)) = pTLV;

		pTLV[0].sx = pTLV[3].sx = (float)nX1;
		pTLV[0].sy = pTLV[1].sy = (float)nY1;
		pTLV[1].sx = pTLV[2].sx = (float)nX2;
		pTLV[3].sy = pTLV[2].sy = (float)nY2;
		pTLV[0].color = pTLV[1].color = pTLV[2].color = pTLV[3].color = ColorFromRGB(pPE[0], pPE[1], pPE[2]);
		pTLV[0].sz = 0.0f;
		pTLV[1].sz = 0.0f;
		pTLV[2].sz = 0.0f;
		pTLV[3].sz = 0.0f;

		CurrentTLVertex = pTLV + 4;
	}
	else
	{
		if (int b = FindBucket(0); b != -1)
		{
			auto& v = Bucket[b];

			float fooz = one / (float)nZDepth,
				  z = f_a - f_boo * fooz;

			const int& cnt = v.cnt;

			v.Vertex[cnt + 0].sx = (float)nX1;
			v.Vertex[cnt + 0].sy = (float)nY1;
			v.Vertex[cnt + 0].sz = z;
			v.Vertex[cnt + 0].rhw = fooz;
			v.Vertex[cnt + 0].color = ColorFromRGB(pPE[0], pPE[1], pPE[2]);

			v.Vertex[cnt + 1].sx = (float)nX2;
			v.Vertex[cnt + 1].sy = (float)nY1;
			v.Vertex[cnt + 1].sz = z;
			v.Vertex[cnt + 1].rhw = fooz;
			v.Vertex[cnt + 1].color = ColorFromRGB(pPE[0], pPE[1], pPE[2]);

			v.Vertex[cnt + 2].sx = (float)nX2;
			v.Vertex[cnt + 2].sy = (float)nY2;
			v.Vertex[cnt + 2].sz = z;
			v.Vertex[cnt + 2].rhw = fooz;
			v.Vertex[cnt + 2].color = ColorFromRGB(pPE[0], pPE[1], pPE[2]);

			v.Vertex[cnt + 3].sx = (float)nX1;
			v.Vertex[cnt + 3].sy = (float)nY1;
			v.Vertex[cnt + 3].sz = z;
			v.Vertex[cnt + 3].rhw = fooz;
			v.Vertex[cnt + 3].color = ColorFromRGB(pPE[0], pPE[1], pPE[2]);

			v.Vertex[cnt + 4].sx = (float)nX2;
			v.Vertex[cnt + 4].sy = (float)nY2;
			v.Vertex[cnt + 4].sz = z;
			v.Vertex[cnt + 4].rhw = fooz;
			v.Vertex[cnt + 4].color = ColorFromRGB(pPE[0], pPE[1], pPE[2]);

			v.Vertex[cnt + 5].sx = (float)nX1;
			v.Vertex[cnt + 5].sy = (float)nY2;
			v.Vertex[cnt + 5].sz = z;
			v.Vertex[cnt + 5].rhw = fooz;
			v.Vertex[cnt + 5].color = ColorFromRGB(pPE[0], pPE[1], pPE[2]);

			v.cnt += 6;
		}
	}
}

void InsertLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t z, char color)
{
	int32_t* sort;
	int16_t* info;

	SetBufferPtrs(sort, info);

	sort[0] = (int32_t)info;
	sort[1] = (int32_t)z;
	info[0] = DRAW_TLV_L;
	info[1] = 0;
	info[2] = 2;

	auto pTLV = CurrentTLVertex;
	auto pPE = game_palette + ((int)(unsigned char)color) * 3;

	*((D3DTLVERTEX**)(info + 3)) = pTLV;

	pTLV[0].sx = (float)(x1 + phd_winxmin);
	pTLV[0].sy = (float)(y1 + phd_winymin);
	pTLV[1].sx = (float)(x2 + phd_winxmin);
	pTLV[1].sy = (float)(y2 + phd_winymin);
	pTLV[0].color = pTLV[1].color = ColorFromRGB(pPE[0], pPE[1], pPE[2]);

	float fooz = one / (float)z;

	pTLV[0].rhw = pTLV[1].rhw = fooz;
	pTLV[0].sz = pTLV[1].sz = f_a - f_boo * fooz;

	CurrentTLVertex = pTLV + 2;
}

void InsertTransQuad(int32_t sx, int32_t sy, int32_t w, int32_t h, int32_t z)
{
	int32_t* sort;
	int16_t* info;

	SetBufferPtrs(sort, info);

	sort[0] = (int32_t)info;
	sort[1] = (int32_t)z;
	info[0] = DRAW_TLV_GA;
	info[1] = 0;
	info[2] = 4;

	D3DTLVERTEX* pTLV = CurrentTLVertex;

	*((D3DTLVERTEX**)(info + 3)) = pTLV;

	pTLV[0].sx = pTLV[3].sx = (float)sx;
	pTLV[0].sy = pTLV[1].sy = (float)sy;
	pTLV[1].sx = pTLV[2].sx = (float)(sx + w);
	pTLV[3].sy = pTLV[2].sy = (float)(sy + h);
	pTLV[0].color = pTLV[1].color = pTLV[2].color = pTLV[3].color = 0x80000000;

	float fooz = one / (float)z;

	pTLV[0].rhw = pTLV[1].rhw = pTLV[2].rhw = pTLV[3].rhw = fooz;
	pTLV[0].sz = pTLV[1].sz = pTLV[2].sz = pTLV[3].sz = f_a - f_boo * fooz;

	CurrentTLVertex = pTLV + 4;
}

void InsertTrans8(PHD_VBUF* vbuf, int16_t shade)
{
	int8_t bClipOr = vbuf[0].clip | vbuf[1].clip | vbuf[2].clip | vbuf[3].clip | vbuf[4].clip | vbuf[5].clip | vbuf[6].clip | vbuf[7].clip;
	if (bClipOr < 0 || (vbuf[0].clip & vbuf[1].clip & vbuf[2].clip & vbuf[3].clip & vbuf[4].clip & vbuf[5].clip & vbuf[6].clip & vbuf[7].clip))
		return;

	IF_NOT_VIS((vbuf + 0), (vbuf + 1), (vbuf + 2)) return;

	for (int i = 0; i < 8; ++i)
	{
		v_buffer[i].x = (float)vbuf[i].xs;
		v_buffer[i].y = (float)vbuf[i].ys;
		v_buffer[i].ooz = one / (vbuf[i].zv - SHADOW_BIAS);
		v_buffer[i].vr = 0;
		v_buffer[i].vg = 0;
		v_buffer[i].vb = 0;
	}

	int nPoints = 8;

	if (bClipOr)
	{
		phd_leftfloat = (float)phd_winxmin;
		phd_topfloat = (float)phd_winymin;
		phd_rightfloat = (float)(phd_winxmin + phd_winwidth);
		phd_bottomfloat = (float)(phd_winymin + phd_winheight);

		nPoints = XYClipper(nPoints, v_buffer);
	}

	if (nPoints)
		HWI_InsertPoly_Gouraud(nPoints,
			((vbuf[0].zv + vbuf[1].zv + vbuf[2].zv + vbuf[3].zv + vbuf[4].zv + vbuf[5].zv + vbuf[6].zv + vbuf[7].zv) * 0.125f) - SHADOW_BIAS,
			0, 0, 0, DRAW_TLV_GA);
}

/**
* Z-Clipper for X, Y, G, U, V
*/
int RoomZedClipper(int number, POINT_INFO* input, VERTEX_INFO* output)
{
	auto A = input + number - 1,
		 B = input;

	auto pOut = output;
	int nVertices = 0;

	for (; number--; A = B++)
	{
		float fADiff = f_znear - A->zv,		// A out if fADiff >= 0
			  fBDiff = f_znear - B->zv;		// B out if fBDIff >= 0

		if ((*(long*)&fADiff | *(long*)&fBDiff) >= 0)	// if both sign bits clear, both out
			continue;

		if ((*(long*)&fADiff ^ *(long*)&fBDiff) < 0)	// if sign bits are different, either A or B is out
		{
			double fconv;

			float clipper = fBDiff / (A->zv - B->zv);

			pOut->x = (B->xv + (A->xv - B->xv) * clipper) * f_perspoznear + f_centerx;
			pOut->y = (B->yv + (A->yv - B->yv) * clipper) * f_perspoznear + f_centery;

			pOut->ooz = f_oneoznear;

			pOut->u = (B->u + (A->u - B->u) * clipper) * f_oneoznear;
			pOut->v = (B->v + (A->v - B->v) * clipper) * f_oneoznear;

			pOut->vr = FTOL((B->vr + (A->vr - B->vr) * clipper));
			pOut->vg = FTOL((B->vg + (A->vg - B->vg) * clipper));
			pOut->vb = FTOL((B->vb + (A->vb - B->vb) * clipper));

			++pOut;
			++nVertices;
		}

		if (*(long*)&fBDiff < 0)	// if B not out
		{
			pOut->x = B->xs;
			pOut->y = B->ys;
			pOut->ooz = B->ooz;
			pOut->u = B->u * B->ooz;
			pOut->v = B->v * B->ooz;
			pOut->vr = B->vr;
			pOut->vg = B->vg;
			pOut->vb = B->vb;

			++pOut;
			++nVertices;
		}
	}

	return (nVertices < 3 ? 0 : nVertices);
}

int RoomXYGUVClipper(int number, VERTEX_INFO* input)
{
	// Clips to X then to Y and returns new number of points: input must be able to hold extra points!
	// clip against X sides

	auto B = &input[number - 1];

	double fconv;
	float clipper;
	int i, j;

	for (i = j = 0; i < number; ++i)
	{
		auto A = B;

		B = &input[i];

		// clip if point A is off screen
		if (A->x < phd_leftfloat)
		{
			if (B->x < phd_leftfloat)
				continue;

			clipper = (phd_leftfloat - B->x) / (A->x - B->x);

			CLIP_GUV(XYGUV_output[j], B, A);

			XYGUV_output[j].x = phd_leftfloat;
			XYGUV_output[j++].y = B->y + (A->y - B->y) * clipper;
		}
		else if (A->x > phd_rightfloat)
		{
			if (B->x > phd_rightfloat)
				continue;

			clipper = (phd_rightfloat - B->x) / (A->x - B->x);

			CLIP_GUV(XYGUV_output[j], B, A);

			XYGUV_output[j].x = phd_rightfloat;
			XYGUV_output[j++].y = B->y + (A->y - B->y) * clipper;
		}

		// clip if point B is off screen, else add it's point unclipped
		if (B->x < phd_leftfloat)
		{
			clipper = (phd_leftfloat - B->x) / (A->x - B->x);

			CLIP_GUV(XYGUV_output[j], B, A);

			XYGUV_output[j].x = phd_leftfloat;
			XYGUV_output[j++].y = B->y + (A->y - B->y) * clipper;
		}
		else if (B->x > phd_rightfloat)
		{
			clipper = (phd_rightfloat - B->x) / (A->x - B->x);

			CLIP_GUV(XYGUV_output[j], B, A);

			XYGUV_output[j].x = phd_rightfloat;
			XYGUV_output[j++].y = B->y + (A->y - B->y) * clipper;
		}
		else
		{
			// not clipped

			XYGUV_output[j].x = B->x;
			XYGUV_output[j].y = B->y;
			//XYGUV_output[j].g = B->g;
			XYGUV_output[j].ooz = B->ooz;

			XYGUV_output[j].vr = B->vr;
			XYGUV_output[j].vg = B->vg;
			XYGUV_output[j].vb = B->vb;

			XYGUV_output[j].u = B->u;
			XYGUV_output[j++].v = B->v;
		}
	}

	// new number of points in 'XYGUV_output': now clip these against Y
	if (j < 3)
		return 0;

	number = j;

	// clip against Y sides

	B = &XYGUV_output[number - 1];

	for (i = j = 0; i < number; ++i)
	{
		auto A = B;

		B = &XYGUV_output[i];

		// clip if point A is off screen
		if (A->y < phd_topfloat)
		{
			if (B->y < phd_topfloat)
				continue;

			clipper = (phd_topfloat - B->y) / (A->y - B->y);

			CLIP_GUV(input[j], B, A);

			input[j].x = B->x + (A->x - B->x) * clipper;
			input[j++].y = phd_topfloat;
		}
		else if (A->y > phd_bottomfloat)
		{
			if (B->y > phd_bottomfloat)
				continue;

			clipper = (phd_bottomfloat - B->y) / (A->y - B->y);

			CLIP_GUV(input[j], B, A);

			input[j].x = B->x + (A->x - B->x) * clipper;
			input[j++].y = phd_bottomfloat;
		}

		/* Clip if point B is off screen, else add its point unclipped */
		if (B->y < phd_topfloat)
		{
			clipper = (phd_topfloat - B->y) / (A->y - B->y);

			CLIP_GUV(input[j], B, A);

			input[j].x = B->x + (A->x - B->x) * clipper;
			input[j++].y = phd_topfloat;
		}
		else if (B->y > phd_bottomfloat)
		{
			clipper = (phd_bottomfloat - B->y) / (A->y - B->y);

			CLIP_GUV(input[j], B, A);

			input[j].x = B->x + (A->x - B->x) * clipper;
			input[j++].y = phd_bottomfloat;
		}
		else
		{
			// unclipped point

			input[j].x = B->x;
			input[j].y = B->y;
			input[j].ooz = B->ooz;
			input[j].vr = B->vr;
			input[j].vg = B->vg;
			input[j].vb = B->vb;
			input[j].u = B->u;
			input[j++].v = B->v;
		}
	}

	// throw away polygons that have been reduced to lines or less
	return (j < 3 ? 0 : j);
}

void HWI_InsertPoly_GouraudRGB(int nPoints, float zdepth, int nDrawType)
{
	uint32_t dwAnd = (nDrawType == DRAW_TLV_GA) ? 0x80ffffff : 0xffffffff;

	VERTEX_INFO* pVI = v_buffer;

	if (App.lpZBuffer == 0 || nDrawType == DRAW_TLV_GA)
	{
		int32_t* sort;
		int16_t* info;
		double fconv;

		SetBufferPtrs(sort, info);

		sort[0] = (int32_t)info;
		sort[1] = (int32_t)FTOL(zdepth);
		info[0] = nDrawType;
		info[1] = 0;
		info[2] = nPoints;

		auto pTLV = CurrentTLVertex;

		*((D3DTLVERTEX**)(info + 3)) = pTLV;

		while (nPoints--)
		{
			pTLV->sx = pVI->x;
			pTLV->sy = pVI->y;
			pTLV->sz = f_a - f_boo * pVI->ooz;
			pTLV->rhw = pVI->ooz;
			pTLV->color = RGBA_MAKE(pVI->vr, pVI->vg, pVI->vb, 0xff) & dwAnd;

			++pVI;
			++pTLV;
		}

		CurrentTLVertex = pTLV;
	}
	else
	{
		// put verts into texture bucket
		// fans so do the first 3 verts then add remainder

		if (int b = FindBucket(0); b != -1)
		{
			auto& v = Bucket[b];

			for (int i = 0; i < 3; ++i)
			{
				int cnt = v.cnt;

				v.Vertex[cnt].sx = pVI->x;
				v.Vertex[cnt].sy = pVI->y;
				v.Vertex[cnt].sz = f_a - f_boo * pVI->ooz;
				v.Vertex[cnt].rhw = pVI->ooz;
				v.Vertex[cnt].color = RGBA_MAKE(pVI->vr, pVI->vg, pVI->vb, 0xff) & dwAnd;

				++pVI;
				++v.cnt;
			}

			nPoints -= 3;

			--pVI;

			for (int i = nPoints; i > 0; --i)
			{
				int cnt = v.cnt;

				v.Vertex[cnt].sx = v_buffer->x;
				v.Vertex[cnt].sy = v_buffer->y;
				v.Vertex[cnt].sz = f_a - f_boo * v_buffer->ooz;
				v.Vertex[cnt].rhw = v_buffer->ooz;
				v.Vertex[cnt].color = RGBA_MAKE(v_buffer->vr, v_buffer->vg, v_buffer->vb, 0xff) & dwAnd;

				v.Vertex[cnt + 1].sx = pVI->x;
				v.Vertex[cnt + 1].sy = pVI->y;
				v.Vertex[cnt + 1].sz = f_a - f_boo * pVI->ooz;
				v.Vertex[cnt + 1].rhw = pVI->ooz;

				v.Vertex[cnt + 1].color = RGBA_MAKE(pVI->vr, pVI->vg, pVI->vb, 0xff) & dwAnd;

				++pVI;

				v.Vertex[cnt + 2].sx = pVI->x;
				v.Vertex[cnt + 2].sy = pVI->y;
				v.Vertex[cnt + 2].sz = f_a - f_boo * pVI->ooz;
				v.Vertex[cnt + 2].rhw = pVI->ooz;

				v.Vertex[cnt + 2].color = RGBA_MAKE(pVI->vr, pVI->vg, pVI->vb, 0xff) & dwAnd;

				v.cnt += 3;
			}
		}
	}
}

void HWI_InsertAlphaSprite_Sorted(int nX1, int nY1, int nZ1, int nShade1,
								  int nX2, int nY2, int nZ2, int nShade2,
								  int nX3, int nY3, int nZ3, int nShade3,
								  int nX4, int nY4, int nZ4, int nShade4,
								  PHDSPRITESTRUCT* nSprite, int type, int dbl)
{
	temp_var<bool, false> blue_effect(g_blue_effect);

	float u1, v1, u2, v2;

	if (nSprite)
	{
		u1 = (float)((nSprite->offset << 8) & 0xff00);
		v1 = (float)(nSprite->offset & 0xff00);
		u2 = u1 + nSprite->width;
		v2 = v1 + nSprite->height;

		u1 += App.nUVAdd;
		u2 -= App.nUVAdd;
		v1 += App.nUVAdd;
		v2 -= App.nUVAdd;
	}

	v_buffer[0].x = (float)nX1;
	v_buffer[0].y = (float)nY1;
	v_buffer[0].ooz = one / (float)nZ1;
	v_buffer[0].vr = GETR(nShade1);
	v_buffer[0].vg = GETG(nShade1);
	v_buffer[0].vb = GETB(nShade1);

	v_buffer[1].x = (float)nX2;
	v_buffer[1].y = (float)nY2;
	v_buffer[1].ooz = one / (float)nZ2;
	v_buffer[1].vr = GETR(nShade2);
	v_buffer[1].vg = GETG(nShade2);
	v_buffer[1].vb = GETB(nShade2);

	v_buffer[2].x = (float)nX3;
	v_buffer[2].y = (float)nY3;
	v_buffer[2].ooz = one / (float)nZ3;
	v_buffer[2].vr = GETR(nShade3);
	v_buffer[2].vg = GETG(nShade3);
	v_buffer[2].vb = GETB(nShade3);

	v_buffer[3].x = (float)nX4;
	v_buffer[3].y = (float)nY4;
	v_buffer[3].ooz = one / (float)nZ4;
	v_buffer[3].vr = GETR(nShade4);
	v_buffer[3].vg = GETG(nShade4);
	v_buffer[3].vb = GETB(nShade4);

	if (nSprite)
	{
		if (dbl == 0)
		{
			v_buffer[0].u = u1 * v_buffer[0].ooz;
			v_buffer[0].v = v1 * v_buffer[0].ooz;

			v_buffer[1].u = u2 * v_buffer[1].ooz;
			v_buffer[1].v = v1 * v_buffer[1].ooz;

			v_buffer[2].u = u2 * v_buffer[2].ooz;
			v_buffer[2].v = v2 * v_buffer[2].ooz;

			v_buffer[3].u = u1 * v_buffer[3].ooz;
			v_buffer[3].v = v2 * v_buffer[3].ooz;
		}
		else
		{
			v_buffer[0].u = u1 * v_buffer[0].ooz;
			v_buffer[0].v = v1 * v_buffer[0].ooz;

			v_buffer[3].u = u2 * v_buffer[3].ooz;
			v_buffer[3].v = v1 * v_buffer[3].ooz;

			v_buffer[2].u = u2 * v_buffer[2].ooz;
			v_buffer[2].v = v2 * v_buffer[2].ooz;

			v_buffer[1].u = u1 * v_buffer[1].ooz;
			v_buffer[1].v = v2 * v_buffer[1].ooz;
		}
	}

	int nPoints = 4;

	if (nX1 < phd_winxmin || nX2 > phd_winxmin + phd_winwidth || nY1 < phd_winymin || nY2 > phd_winymin + phd_winheight)
	{
		phd_leftfloat = (float)phd_winxmin;
		phd_rightfloat = (float)(phd_winxmin + phd_winwidth);
		phd_topfloat = (float)phd_winymin;
		phd_bottomfloat = (float)(phd_winymin + phd_winheight);

		nPoints = (!nSprite ? XYGClipper(nPoints, v_buffer) : RoomXYGUVClipper(nPoints, v_buffer));
	}

	if (nPoints)
	{
		const float fZDepth = (float)((nZ1 + nZ2 + nZ3 + nZ4) >> 2);

		if (!nSprite)
			HWI_InsertPoly_GouraudRGB(nPoints, fZDepth, type);
		else HWI_InsertClippedPoly_Textured(nPoints, fZDepth, type, nSprite->tpage);
	}
}

int XYGClipper(int number, VERTEX_INFO* input)
{
	// clips to X then to Y and returns new number of points: input must be able to hold extra points!
	// clip against X sides

	auto B = &input[number - 1];

	double fconv;
	float clipper;
	int i, j;

	for (i = j = 0; i < number; ++i)
	{
		auto A = B;

		B = &input[i];

		// clip if point A is off screen
		if (A->x < phd_leftfloat)
		{
			if (B->x < phd_leftfloat)
				continue;

			clipper = (phd_leftfloat - B->x) / (A->x - B->x);

			CLIP_G(XYG_output[j], B, A);

			XYG_output[j].x = phd_leftfloat;
			XYG_output[j++].y = B->y + (A->y - B->y) * clipper;
		}
		else if (A->x > phd_rightfloat)
		{
			if (B->x > phd_rightfloat)
				continue;

			clipper = (phd_rightfloat - B->x) / (A->x - B->x);

			CLIP_G(XYG_output[j], B, A);

			XYG_output[j].x = phd_rightfloat;
			XYG_output[j++].y = B->y + (A->y - B->y) * clipper;
		}

		// clip if point B is off screen, else add it's point unclipped
		if (B->x < phd_leftfloat)
		{
			clipper = (phd_leftfloat - B->x) / (A->x - B->x);

			CLIP_G(XYG_output[j], B, A);

			XYG_output[j].x = phd_leftfloat;
			XYG_output[j++].y = B->y + (A->y - B->y) * clipper;
		}
		else if (B->x > phd_rightfloat)
		{
			clipper = (phd_rightfloat - B->x) / (A->x - B->x);

			CLIP_G(XYG_output[j], B, A);

			XYG_output[j].x = phd_rightfloat;
			XYG_output[j++].y = B->y + (A->y - B->y) * clipper;
		}
		else
		{
			// not clipped

			XYG_output[j].x = B->x;
			XYG_output[j].y = B->y;
			XYG_output[j].ooz = B->ooz;
			XYG_output[j].vr = B->vr;
			XYG_output[j].vg = B->vg;
			XYG_output[j].vb = B->vb;

			++j;
		}
	}

	// new number of points in 'output': now clip these against Y

	if (j < 3)
		return 0;

	number = j;

	/* clip against Y sides */

	B = &XYG_output[number - 1];

	for (i = j = 0; i < number; ++i)
	{
		auto A = B;

		B = &XYG_output[i];

		// clip if point A is off screen
		if (A->y < phd_topfloat)
		{
			if (B->y < phd_topfloat)
				continue;

			clipper = (phd_topfloat - B->y) / (A->y - B->y);

			CLIP_G(input[j], B, A);

			input[j].x = B->x + (A->x - B->x) * clipper;
			input[j++].y = phd_topfloat;
		}
		else if (A->y > phd_bottomfloat)
		{
			if (B->y > phd_bottomfloat)
				continue;

			clipper = (phd_bottomfloat - B->y) / (A->y - B->y);

			CLIP_G(input[j], B, A);

			input[j].x = B->x + (A->x - B->x) * clipper;
			input[j++].y = phd_bottomfloat;
		}

		/* clip if point B is off screen, else add its point unclipped */
		if (B->y < phd_topfloat)
		{
			clipper = (phd_topfloat - B->y) / (A->y - B->y);

			CLIP_G(input[j], B, A);

			input[j].x = B->x + (A->x - B->x) * clipper;
			input[j++].y = phd_topfloat;
		}
		else if (B->y > phd_bottomfloat)
		{
			clipper = (phd_bottomfloat - B->y) / (A->y - B->y);

			CLIP_G(input[j], B, A);

			input[j].x = B->x + (A->x - B->x) * clipper;
			input[j++].y = phd_bottomfloat;
		}
		else
		{
			// unclipped point

			input[j].x = B->x;
			input[j].y = B->y;
			input[j].ooz = B->ooz;
			input[j].ooz = B->ooz;
			input[j].vr = B->vr;
			input[j].vg = B->vg;
			input[j].vb = B->vb;

			++j;
		}
	}

	// Throw away polygons that have been reduced to lines or less
	return (j < 3 ? 0 : j);
}

int XYClipper(int number, VERTEX_INFO* input)
{
	// clips to X then to Y and returns new number of points: input must be able to hold extra points!
	int 			i, j;
	VERTEX_INFO 	output[20];
	float 			clipper;

	// clip against X sides
	auto B = &input[number - 1];

	for (i = j = 0; i < number; ++i)
	{
		auto A = B;

		B = &input[i];

		// clip if point A is off screen
		if (A->x < phd_leftfloat)
		{
			if (B->x < phd_leftfloat)
				continue;

			clipper = (phd_leftfloat - B->x) / (A->x - B->x);

			output[j].x = phd_leftfloat;
			output[j++].y = B->y + (A->y - B->y) * clipper;
		}
		else if (A->x > phd_rightfloat)
		{
			if (B->x > phd_rightfloat)
				continue;

			clipper = (phd_rightfloat - B->x) / (A->x - B->x);

			output[j].x = phd_rightfloat;
			output[j++].y = B->y + (A->y - B->y) * clipper;
		}

		// clip if point B is off screen, else add it's point unclipped
		if (B->x < phd_leftfloat)
		{
			clipper = (phd_leftfloat - B->x) / (A->x - B->x);

			output[j].x = phd_leftfloat;
			output[j++].y = B->y + (A->y - B->y) * clipper;
		}
		else if (B->x > phd_rightfloat)
		{
			clipper = (phd_rightfloat - B->x) / (A->x - B->x);

			output[j].x = phd_rightfloat;
			output[j++].y = B->y + (A->y - B->y) * clipper;
		}
		else
		{
			// not clipped

			output[j].x = B->x;
			output[j++].y = B->y;
		}
	}

	// new number of points in 'output': now clip these against Y

	if (j < 3)
		return 0;

	number = j;

	// clip against Y sides
	B = &output[number - 1];

	for (i = j = 0; i < number; ++i)
	{
		auto A = B;

		B = &output[i];

		// clip if point A is off screen
		if (A->y < phd_topfloat)
		{
			if (B->y < phd_topfloat)
				continue;

			clipper = (phd_topfloat - B->y) / (A->y - B->y);

			input[j].x = B->x + (A->x - B->x) * clipper;
			input[j++].y = phd_topfloat;
		}
		else if (A->y > phd_bottomfloat)
		{
			if (B->y > phd_bottomfloat)
				continue;

			clipper = (phd_bottomfloat - B->y) / (A->y - B->y);

			input[j].x = B->x + (A->x - B->x) * clipper;
			input[j++].y = phd_bottomfloat;
		}

		// clip if point B is off screen, else add its point unclipped
		if (B->y < phd_topfloat)
		{
			clipper = (phd_topfloat - B->y) / (A->y - B->y);

			input[j].x = B->x + (A->x - B->x) * clipper;
			input[j++].y = phd_topfloat;
		}
		else if (B->y > phd_bottomfloat)
		{
			clipper = (phd_bottomfloat - B->y) / (A->y - B->y);

			input[j].x = B->x + (A->x - B->x) * clipper;
			input[j++].y = phd_bottomfloat;
		}
		else
		{
			// unclipped point

			input[j].x = B->x;
			input[j++].y = B->y;
		}
	}

	// throw away polygons that have been reduced to lines or less
	return (j < 3 ? 0 : j);
}
