#pragma once

#include "texture.h"

bool LoadPicture(const char* File, LPDIRECTDRAWSURFACE3 lpPictureBuffer, int flag);
void FadePictureUp(int pic);
void FadePictureDown(int pic);
void ConvertSurfaceToTextures16Bit(LPDIRECTDRAWSURFACE3 lpSurface);
void DrawPicture(int pic);
void FreePictureTextures();

inline DXTEXTURE PicTextureList[MAX_D3D_TEXTURES];