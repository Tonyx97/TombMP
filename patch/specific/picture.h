#pragma once

#include "texture.h"

bool LoadPicture(const char* File, LPDIRECTDRAWSURFACE3 lpPictureBuffer, int flag);
void FadePictureUp();
void FadePictureDown();
void ConvertSurfaceToTextures16Bit(LPDIRECTDRAWSURFACE3 lpSurface);
void DrawPicture();
void FreePictureTextures();

inline DXTEXTURE PicTextureList[MAX_D3D_TEXTURES];