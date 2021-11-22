#pragma once

#include <specific/texture.h>

#define VERTSPERBUCKET (1024 + 32)
#define MAXBUCKETS		64

struct TEXTUREBUCKET
{
	DWORD tpage;
	int cnt;
	D3DTLVERTEX Vertex[VERTSPERBUCKET];

	void init()
	{
		tpage = 0xffffffff;
		cnt = 0;
	}
};

int RoomZedClipper(int number, POINT_INFO* input, VERTEX_INFO* output);
int RoomXYGUVClipper(int number, VERTEX_INFO* input);

int16_t* InsertObjectGT3(int16_t* objptr, int number, sort_type sort_routine);
int16_t* InsertObjectGT4(int16_t* objptr, int number, sort_type sort_routine);
int16_t* InsertObjectG3(int16_t* objptr, int number, sort_type sort_routine);
int16_t* InsertObjectG4(int16_t* objptr, int number, sort_type sort_routine);

void InsertGT4(PHD_VBUF* pV1, PHD_VBUF* pV2, PHD_VBUF* pV3, PHD_VBUF* pV4, PHDTEXTURESTRUCT* pTex, sort_type nSortType, unsigned short dbl);
void InsertGT3(PHD_VBUF* pV1, PHD_VBUF* pV2, PHD_VBUF* pV3, PHDTEXTURESTRUCT* pTex, uint16_t* pUV1, uint16_t* pUV2, uint16_t* pUV3, sort_type nSortType, unsigned short dbl);
void InsertFlatRect(int32_t nX1, int32_t nY1, int32_t nX2, int32_t nY2, int nZDepth, int nColour);
void InsertLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t z, char color);
void InsertSprite(int nZDepth, int nX1, int nY1, int nX2, int nY2, int16_t* nSprite, int nShade, int nShade1, int drawtype, int offset);
void InsertTrans8(PHD_VBUF* vbuf, int16_t shade);
void InsertTransQuad(int32_t sx, int32_t sy, int32_t w, int32_t h, int32_t z);

void HWI_InsertAlphaSprite_Sorted(int nX1, int nY1, int nZ1, int nShade1,
								  int nX2, int nY2, int nZ2, int nShade2,
								  int nX3, int nY3, int nZ3, int nShade3,
								  int nX4, int nY4, int nZ4, int nShade4,
								  PHDSPRITESTRUCT* nSprite, int type, int dbl);

void InitBuckets();
void DrawBuckets();

inline TEXTUREBUCKET Bucket[MAXBUCKETS];

inline bool g_lighting_on = true;

inline uint8_t g_forced_red = 0,
			   g_forced_green = 0,
			   g_forced_blue = 0;

inline int LanTextureHandle[MAX_D3D_TEXTURES],
		   texture_pages_count = 0,
		   texture_info_count = 0;