#pragma once

#define MAX_D3D_TEXTURES 128
#define TX_FLAG_SLOTUSED 1

struct DXTEXTURE
{
	IDirectDrawSurface3* pSystemSurface;
	IDirectDrawSurface3* pDeviceSurface;
	IDirectDrawPalette* pPalette;
	IDirect3DTexture2* pTexture;

	D3DTEXTUREHANDLE hTexture;

	int nWidth,
		nHeight;

	DWORD dwFlags;

	unsigned short* pSoftwareSurface;
};

inline DXTEXTURE DXTextureList[MAX_D3D_TEXTURES];

void DXTextureInit(DXTEXTURE[]);
int DXTextureAdd(int nWidth, int nHeight, uint16_t* pTexture, DXTEXTURE TextureList[], int BitFormat);
IDirect3DTexture2* DXTextureGetInterface(IDirectDrawSurface3* pSurf);
int DXTextureFindTextureSlot(DXTEXTURE[]);
bool DXTextureMakeSystemSurface(DXTEXTURE& rTexture);
bool DXTextureMakeDeviceSurface(DXTEXTURE& rTexture);
void DXTextureReleaseDeviceSurface(DXTEXTURE& rTexture);
int DXTextureNew(int nWidth, int nHeight, DXTEXTURE[]);
void DXTextureCleanup(int nSlot, DXTEXTURE[]);
void DXTextureFreeAll(DXTEXTURE[]);
bool DXTextureRestore(int nHandle, bool tForce, DXTEXTURE[]);
bool DXTextureRestoreAll(bool tForce, DXTEXTURE[]);
DWORD DXTextureGetHandle(int nHandle, DXTEXTURE[]);
void DXTextureFinish(DXTEXTURE[]);
void DXTextureFree(int nHandle, DXTEXTURE[]);
void DXTextureSetGreyScale(bool);