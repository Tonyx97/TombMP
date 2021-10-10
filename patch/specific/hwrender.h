#pragma once

#include "texture.h"

#define MAX_TLVERTICES (1 << 15)

inline DWORD GahTextureHandle[MAX_D3D_TEXTURES];
inline D3DTLVERTEX* CurrentTLVertex;
inline DWORD GhCurTextureHandle;
inline bool GtColorKeyEnabled;

void hwr_init();
void hwr_destroy();
void HWR_EnableZBuffer(bool tWrite, bool tCompare);
void HWR_InitVertexList();
void HWR_SetCurrentTexture(DWORD dwHandle);
void HWR_EnableColorKey(bool tEnable);
void HWR_BeginScene();
void hwr_init_state();
void HWR_DrawPolyList(int num, int32_t* sortptr);
void HWR_EndScene();
void HWR_GetAllTextureHandles();
void HWR_ResetCurrentTexture();
void HWR_LoadTexturePages(int pages, char* img);
void HWR_FreeTexturePages();
void HWR_ResetColorKey();
void HWR_DrawPolyListFB(int num, int32_t* sortptr);
void HWR_DrawPolyListBF(int num, int32_t* sortptr);

inline D3DTLVERTEX* VertexBuffer = nullptr;