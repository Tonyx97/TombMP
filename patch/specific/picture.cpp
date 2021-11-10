import prof;

#include "standard.h"
#include "global.h"
#include "picture.h"
#include <main.h>
#include "hwrender.h"
#include "drawprimitive.h"
#include "texture.h"

/**
* Load BMP picture into surface
*/
bool LoadPicture(const char* File, LPDIRECTDRAWSURFACE3 lpPictureBuffer, int Flag)
{
	// todo: sort pictures for 8 bit

	auto Bitmap = (HBITMAP)LoadImage(nullptr, File, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION | LR_VGACOLOR | LR_LOADMAP3DCOLORS);
	if (!Bitmap)
		return false;

	auto BitmapDC = CreateCompatibleDC(nullptr);
	BITMAP bm;
	HDC SurfaceDC;

	SelectObject(BitmapDC, Bitmap);
	GetObject(Bitmap, sizeof(bm), &bm);

	lpPictureBuffer->GetDC(&SurfaceDC);
	BitBlt(SurfaceDC, 0, 0, bm.bmWidth, bm.bmHeight, BitmapDC, 0, 0, SRCCOPY);
	lpPictureBuffer->ReleaseDC(SurfaceDC);

	prof::print(YELLOW, "LoadPicture: {}", File);

	if (Flag)
		ConvertSurfaceToTextures16Bit(lpPictureBuffer);

	return true;
}

/**
* Release picture texture handles
*/
void FreePictureTextures()
{
	DXTextureFinish(PicTextureList);
}

/**
* Convert lpSurface into textures
*/
void ConvertSurfaceToTextures16Bit(LPDIRECTDRAWSURFACE3 lpSurface)
{
	if (auto pTPage = new unsigned short[1920 * 1080])
	{
		DDSURFACEDESC ddsd;

		DXInit(ddsd);

		lpSurface->Lock(nullptr, &ddsd, DDLOCK_NOSYSLOCK | DDLOCK_WAIT, nullptr);

		unsigned char rshift, gshift, bshift,
					  rbpp, gbpp, bbpp;

		DXBitMask2ShiftCnt(ddsd.ddpfPixelFormat.dwRBitMask, &rshift, &rbpp);
		DXBitMask2ShiftCnt(ddsd.ddpfPixelFormat.dwGBitMask, &gshift, &gbpp);
		DXBitMask2ShiftCnt(ddsd.ddpfPixelFormat.dwBBitMask, &bshift, &bbpp);

		int BitMask = (rbpp * 100) + (gbpp * 10) + bbpp;

		memcpy(pTPage, ddsd.lpSurface, 1920 * 1080 * sizeof(short));

		lpSurface->Unlock(nullptr);

		DXTextureAdd(1920, 1080, pTPage, PicTextureList, BitMask);

		delete[] pTPage;
	}
	else prof::print(RED, "ConvertPictureToTexture: Failed To Allocate Temp Texture Page");
}

/**
* Display a single tile
*/
void DrawTile(int nDX, int nDY, int nDW, int nDH, int hTPage, int nSX, int nSY, int nSW, int nSH, uint32_t dwV0C, uint32_t dwV1C, uint32_t dwV2C, uint32_t dwV3C)
{
	float fX1 = float(nDX),
		  fY1 = float(nDY),
		  fX2 = float(nDX + nDW),
		  fY2 = float(nDY + nDH),
		  fU1 = float(nSX) * (1.f / 256.f),
		  fV1 = float(nSY) * (1.f / 256.f),
		  fU2 = float(nSX + nSW) * (1.f / 256.f),
		  fV2 = float(nSY + nSH) * (1.f / 256.f),
		  fAdd = float(App.nUVAdd) * (1.f / 65536.f);

	fU1 += fAdd;
	fV1 += fAdd;
	fU2 -= fAdd;
	fV2 -= fAdd;

	D3DTLVERTEX v[4];

	const float fRHW = one / f_zfar;

	v[0].sx = fX1; v[0].sy = fY1; v[0].sz = 0.995f; v[0].rhw = fRHW; v[0].color = dwV0C; v[0].specular = 0; v[0].tu = fU1; v[0].tv = fV1;
	v[1].sx = fX2; v[1].sy = fY1; v[1].sz = 0.995f; v[1].rhw = fRHW; v[1].color = dwV1C; v[1].specular = 0; v[1].tu = fU2; v[1].tv = fV1;
	v[2].sx = fX1; v[2].sy = fY2; v[2].sz = 0.995f; v[2].rhw = fRHW; v[2].color = dwV2C; v[2].specular = 0; v[2].tu = fU1; v[2].tv = fV2;
	v[3].sx = fX2; v[3].sy = fY2; v[3].sz = 0.995f; v[3].rhw = fRHW; v[3].color = dwV3C; v[3].specular = 0; v[3].tu = fU2; v[3].tv = fV2;

	HWR_SetCurrentTexture(hTPage);
	HWR_EnableColorKey(false);

	DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DVT_TLVERTEX, v, 4, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
}

/**
* Display a single tile
*/
void DrawTileEx(int nDX, int nDY, int nDW, int nDH, int hTPage, int nSX, int nSY, int nSW, int nSH, uint32_t dwV0C, uint32_t dwV1C, uint32_t dwV2C, uint32_t dwV3C)
{
	auto [wnd_sx, wnd_sy] = g_window->get_resolution();

	float fX1 = float(nDX),
		  fY1 = float(nDY),
		  fX2 = float(nDX + nDW),
		  fY2 = float(nDY + nDH),
		  fU1 = float(nSX) / float(wnd_sx),
		  fV1 = float(nSY) / float(wnd_sy),
		  fU2 = float(nSX + nSW) / float(wnd_sx),
		  fV2 = float(nSY + nSH) / float(wnd_sy);

	D3DTLVERTEX v[4];

	const float fRHW = one / f_zfar;

	v[0].sx = fX1; v[0].sy = fY1; v[0].sz = 1.f; v[0].rhw = fRHW; v[0].color = dwV0C; v[0].specular = 0; v[0].tu = fU1; v[0].tv = fV1;
	v[1].sx = fX2; v[1].sy = fY1; v[1].sz = 1.f; v[1].rhw = fRHW; v[1].color = dwV1C; v[1].specular = 0; v[1].tu = fU2; v[1].tv = fV1;
	v[2].sx = fX1; v[2].sy = fY2; v[2].sz = 1.f; v[2].rhw = fRHW; v[2].color = dwV2C; v[2].specular = 0; v[2].tu = fU1; v[2].tv = fV2;
	v[3].sx = fX2; v[3].sy = fY2; v[3].sz = 1.f; v[3].rhw = fRHW; v[3].color = dwV3C; v[3].specular = 0; v[3].tu = fU2; v[3].tv = fV2;

	HWR_SetCurrentTexture(hTPage);
	HWR_EnableColorKey(false);

	DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DVT_TLVERTEX, v, 4, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
}

/**
* Draw picture from picture texture files
*/
void DrawPicture(int pic)
{
	// todo: sort pictures for 8 bit

	HWR_EnableZBuffer(false, false);

	auto [wnd_sx, wnd_sy] = g_window->get_resolution();

	DrawTileEx(0, 0, wnd_sx, wnd_sy, DXTextureGetHandle(pic, PicTextureList), 0, 0, wnd_sx, wnd_sy, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff);

	HWR_EnableZBuffer(true, true);
}

void FadePictureUp(int pic)
{
	D3DTLVERTEX v[4];

	auto [_sw, _sh] = g_window->get_resolution();

	float sw = float(_sw),
		  sh = float(_sh);

	v[0].sx = 0;
	v[0].sy = 0;
	v[0].sz = 0;
	v[0].specular = 0;

	v[1].sx = sw;
	v[1].sy = 0;
	v[1].sz = 0;
	v[1].specular = 0;

	v[2].sx = 0;
	v[2].sy = sh;
	v[2].sz = 0;
	v[2].specular = 0;

	v[3].sx = sw;
	v[3].sy = sh;
	v[3].sz = 0;
	v[3].specular = 0;

#if defined(_DEBUG) || defined(LEVEL_EDITOR)
	v[0].color = RGBA_MAKE(0, 0, 0, 0);
	v[1].color = RGBA_MAKE(0, 0, 0, 0);
	v[2].color = RGBA_MAKE(0, 0, 0, 0);
	v[3].color = RGBA_MAKE(0, 0, 0, 0);

	HWR_BeginScene();
	{
		DrawPicture(pic);

		HWR_SetCurrentTexture(0);

		App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, true);
		App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
		App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
		DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DVT_TLVERTEX, &v, 4, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
		App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, false);
	}
	HWR_EndScene();

	App.lpFrontBuffer->Flip(nullptr, DDFLIP_WAIT);
#else
	for (int i = 0; i < 128; ++i)
	{
		int a = 256 - (i * 2);

		v[0].color = RGBA_MAKE(0, 0, 0, a);
		v[1].color = RGBA_MAKE(0, 0, 0, a);
		v[2].color = RGBA_MAKE(0, 0, 0, a);
		v[3].color = RGBA_MAKE(0, 0, 0, a);

		HWR_BeginScene();
		{
			DrawPicture();

			HWR_SetCurrentTexture(0);

			App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, true);
			App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
			App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
			DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DVT_TLVERTEX, &v, 4, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
			App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, false);
		}
		HWR_EndScene();

		App.lpFrontBuffer->Flip(nullptr, DDFLIP_WAIT);
	}
#endif

	DrawPicture(pic);

	App.lpFrontBuffer->Flip(nullptr, DDFLIP_WAIT);
}

void FadePictureDown(int pic)
{
#if defined(_DEBUG) || defined(LEVEL_EDITOR)
	return;
#endif

	D3DTLVERTEX v[4];

	auto [_sw, _sh] = g_window->get_resolution();

	float sw = float(_sw),
		  sh = float(_sh);

	v[0].sx = 0;
	v[0].sy = 0;
	v[0].sz = 0;
	v[0].specular = 0;

	v[1].sx = sw;
	v[1].sy = 0;
	v[1].sz = 0;
	v[1].specular = 0;

	v[2].sx = 0;
	v[2].sy = sh;
	v[2].sz = 0;
	v[2].specular = 0;

	v[3].sx = sw;
	v[3].sy = sh;
	v[3].sz = 0;
	v[3].specular = 0;

	for (int i = 128; i > 0; --i)
	{
		int a = 256 - (i * 2);

		v[0].color = RGBA_MAKE(0, 0, 0, a);
		v[1].color = RGBA_MAKE(0, 0, 0, a);
		v[2].color = RGBA_MAKE(0, 0, 0, a);
		v[3].color = RGBA_MAKE(0, 0, 0, a);

		HWR_BeginScene();
		{
			DrawPicture(pic);

			HWR_SetCurrentTexture(0);

			App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, true);
			App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
			App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
			DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DVT_TLVERTEX, &v, 4, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
			App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, false);
		}
		HWR_EndScene();

		App.lpFrontBuffer->Flip(nullptr, DDFLIP_WAIT);
	}

	v[0].color = RGBA_MAKE(0, 0, 0, 255);
	v[1].color = RGBA_MAKE(0, 0, 0, 255);
	v[2].color = RGBA_MAKE(0, 0, 0, 255);
	v[3].color = RGBA_MAKE(0, 0, 0, 255);

	HWR_BeginScene();
	HWR_SetCurrentTexture(0);
	App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, true);
	App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
	App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
	DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DVT_TLVERTEX, &v, 4, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
	App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, false);
	HWR_EndScene();

	App.lpFrontBuffer->Flip(nullptr, DDFLIP_WAIT);
}