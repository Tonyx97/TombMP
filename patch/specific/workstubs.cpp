import prof;

#include "standard.h"
#include "global.h"
#include "init.h"
#include "hwrender.h"

#include <main.h>

void S_ExitSystem()
{
	DXSetCooperativeLevel(App.lpDD, App.WindowHandle, DDSCL_NORMAL);
	free_game_memory();
	Sleep(2500);
	exit(1);
}

HRESULT DD_LockSurface(IDirectDrawSurface3* pSurf, DDSURFACEDESC& rDDSD, DWORD dwFlags)
{
	InitDXStruct(rDDSD);
	return pSurf->Lock(NULL, &rDDSD, dwFlags, nullptr);
}

HRESULT DD_UnlockSurface(IDirectDrawSurface3* pSurf, DDSURFACEDESC& rDDSD)
{
	return pSurf->Unlock(rDDSD.lpSurface);
}

HRESULT DD_CreateSurface(DDSURFACEDESC& ddsd, IDirectDrawSurface3*& rpSurf)
{
	IDirectDrawSurface* pSurf;

	HRESULT err = App.lpDD->CreateSurface(&ddsd, &pSurf, nullptr);
	if (DX_TRY(err))
		return err;

	err = pSurf->QueryInterface(IID_IDirectDrawSurface3, (void**)&rpSurf);
	pSurf->Release();
	return err;
}

HRESULT DD_EnsureSurfaceAvailable(IDirectDrawSurface3* pSurf, IDirectDrawSurface3* pTopSurf, bool tClearIfLost)
{
	if (HRESULT err = pSurf->IsLost(); err == DDERR_SURFACELOST)
	{
		prof::print(RED, "Surface lost... trying to restore");

		if (!pTopSurf)
			pTopSurf = pSurf;

		err = pTopSurf->Restore();

		prof::print(YELLOW, "Restore {}", SUCCEEDED(err) ? "succeeded" : "failed");

		if (tClearIfLost && SUCCEEDED(err) && !DD_ClearSurface(pSurf))
			prof::print(RED, "Can't clear restored surface");

		return err;
	}
	else return err;
}

bool DD_ClearSurface(IDirectDrawSurface3* pSurf, RECT* pRect, DWORD dwColour)
{
	DDBLTFX bltfx;
	InitDXStruct(bltfx);
	bltfx.dwFillColor = dwColour;
	return !DX_TRY(pSurf->Blt(pRect, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &bltfx));
}

void DX_DoFlipWait()
{
	while (App.lpFrontBuffer->GetFlipStatus(DDGFS_ISFLIPDONE) == DDERR_WASSTILLDRAWING);
}

void DX_ClearBuffers(DWORD dwClrFlags, DWORD dwColour)
{
	RECT rc;

	auto [sw, sh] = g_window->get_resolution();

	rc.left = rc.top = 0;
	rc.right = sw;
	rc.bottom = sh;

	DWORD dwFlags = 0;

	if (dwClrFlags & DXCB_BACK)		dwFlags = D3DCLEAR_TARGET;
	if (dwClrFlags & DXCB_ZBUFFER)  dwFlags |= D3DCLEAR_ZBUFFER;

	if (dwFlags)
	{
		D3DRECT rc3d;

		rc3d.x1 = rc.left;
		rc3d.y1 = rc.top;
		rc3d.x2 = rc.right;
		rc3d.y2 = rc.bottom;
		
		App.lpViewPort->Clear(1, &rc3d, dwFlags);
	}

	if (dwClrFlags & DXCB_FRONT)
		DD_ClearSurface(App.lpFrontBuffer, &rc, dwColour);

	if (dwClrFlags & DXCB_PICTURE)
	{
		rc.left = rc.top = 0;
		rc.right = 1920; rc.bottom = 1080;

		DD_ClearSurface(App.lpPictureBuffer, &rc, dwColour);
	}
}

bool DX_CheckForLostSurfaces()
{
	if (!App.lpFrontBuffer)
		S_ExitSystem();

	bool tRestart = DX_TRY(DD_EnsureSurfaceAvailable(App.lpFrontBuffer, 0, true));

	tRestart = tRestart || DX_TRY(DD_EnsureSurfaceAvailable(App.lpBackBuffer, App.lpFrontBuffer, true));

	if (App.lpZBuffer != 0)
		tRestart = tRestart || DX_TRY(DD_EnsureSurfaceAvailable(App.lpZBuffer, 0));

	tRestart = tRestart || DX_TRY(DD_EnsureSurfaceAvailable(App.lpPictureBuffer));

	if (tRestart && !game_closedown)
		HWR_GetAllTextureHandles();

	return tRestart;
}

bool DX_UpdateFrame(RECT* prcWindow)
{
	DX_CheckForLostSurfaces();

	auto [sw, sh] = g_window->get_resolution();

	RECT r;

	r.top = 0;
	r.left = 0;
	r.right = sw;
	r.bottom = sh;

	App.lpFrontBuffer->Flip(NULL, DDFLIP_WAIT);
	
	return true;
}