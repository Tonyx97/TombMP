#include "standard.h"
#include "directx.h"

#include <main.h>

#include <window/window.h>

LPDIRECTDRAW2 lpDD2;
LPDIRECT3D2 lpD3D2;
LPDIRECTDRAWSURFACE3 lpDDSurface;
LPDIRECT3DDEVICE2 lpD3DDevice;
HWND WindowHandle;

void WinFreeDX()
{
	DXRelease(App.lpViewPort);
	DXRelease(App.lpD3DDevice);
	DXRelease(App.lpZBuffer);
	DXRelease(App.lpPictureBuffer);
	DXRelease(App.lpBackBuffer);
	DXRelease(App.lpFrontBuffer);
	DXRelease(App.lpD3D);
	DXRelease(App.lpDD);
}

bool WinDXInit(DEVICEINFO* DeviceInfo)
{
	const auto& d3_info = DeviceInfo->DDInfo[0];

	auto [sw, sh] = g_window->get_resolution();

	if (!DXCreateDirectDraw(DeviceInfo, &App.lpDD) ||
		!DXCreateDirect3D(App.lpDD, &App.lpD3D) ||
		!DXSetCooperativeLevel(App.lpDD, App.WindowHandle, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN))
		return false;

	if (!DXSetVideoMode(App.lpDD, sw, sh, 16))
		return false;

	DDSURFACEDESC ddsd;

	DXInit(ddsd);

	ddsd.dwBackBufferCount = 1;
	ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_3DDEVICE | DDSCAPS_COMPLEX;

	if (!DXCreateSurface(App.lpDD, &ddsd, &App.lpFrontBuffer))
		return false;

	DDSCAPS	ddscaps;

	ddscaps.dwCaps = DDSCAPS_BACKBUFFER;

	if (!DXGetAttachedSurface(App.lpFrontBuffer, &ddscaps, &App.lpBackBuffer))
		return false;

	DXInit(ddsd);

	ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_ZBUFFERBITDEPTH;
	ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | DDSCAPS_VIDEOMEMORY;
	ddsd.dwWidth = sw;
	ddsd.dwHeight = sh;
	ddsd.dwZBufferBitDepth = 16;

	if (!DXCreateSurface(App.lpDD, &ddsd, &App.lpZBuffer) ||
		!DXAddAttachedSurface(App.lpBackBuffer, App.lpZBuffer) ||
		!DXCreateDirect3DDevice(App.lpD3D, DXD3DGuid(DeviceInfo), App.lpBackBuffer, &App.lpD3DDevice))
		return false;

	if (!DXCreateViewPort(App.lpD3D, App.lpD3DDevice,
		sw,
		sh,
		&App.lpViewPort))
		return false;

	DXInit(ddsd);

	ddsd.dwWidth = 1920;
	ddsd.dwHeight = 1080;
	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;

	return DXCreateSurface(App.lpDD, &ddsd, &App.lpPictureBuffer);
}

float WinFrameRate()
{
	static auto last_time = clock();
	static int count = 0;

	static float fps = 0.f;

	if (++count == 10)
	{
		auto this_time = clock();
		auto t = (double)((int)this_time - last_time) / (double)CLOCKS_PER_SEC;
		last_time = (int)this_time;
		fps = (float)(count / t);
		count = 0;
	}

	App.fps = fps;

	return fps;
}

/**
* Fill passed DDDevice structure with DD info
*/
void DXGetDeviceInfo(DEVICEINFO* dd, HWND hWnd)
{
	WindowHandle = hWnd;
	memset(dd, 0, sizeof(DEVICEINFO));
	DirectDrawEnumerate(DXEnumDirectDraw, dd);
}

void DXFreeDeviceInfo(DEVICEINFO* dd)
{
	for (int n = 0; n < dd->nDDInfo; ++n)
	{
		for (int i = 0; i < dd->DDInfo[n].nD3DInfo; ++i)
		{
			free(dd->DDInfo[n].D3DInfo[i].DisplayMode);
			free(dd->DDInfo[n].D3DInfo[i].Texture);
		}

		free(dd->DDInfo[n].D3DInfo);
		free(dd->DDInfo[n].DisplayMode);
	}

	free(dd->DDInfo);

	memset(dd, 0, sizeof(DEVICEINFO));
}

/**
* Enumeration of DirectDraw devices
*/
BOOL __stdcall DXEnumDirectDraw(GUID* lpGuid, LPSTR lpDeviceDesc, LPSTR lpDeviceName, LPVOID lpContext)
{
	DEVICEINFO* dd = (DEVICEINFO*)lpContext;

	if (dd->nDDInfo == 0)
		dd->DDInfo = (DIRECTDRAWINFO*)malloc(sizeof(DIRECTDRAWINFO));
	else dd->DDInfo = (DIRECTDRAWINFO*)realloc(dd->DDInfo, sizeof(DIRECTDRAWINFO) * (dd->nDDInfo + 1));

	auto& d3_info = dd->DDInfo[dd->nDDInfo];

	memset(&d3_info, 0, sizeof(DIRECTDRAWINFO));

	// store DirectDraw info	

	if (lpGuid)
	{
		d3_info.lpGuid = &d3_info.Guid;
		memcpy(&d3_info.Guid, lpGuid, sizeof(GUID));
	}
	else d3_info.lpGuid = nullptr;

	lstrcpy(d3_info.About, lpDeviceDesc);
	lstrcpy(d3_info.Name, lpDeviceName);

	// create DD device to get caps

	LPDIRECTDRAW lpDDTemp;

	DirectDrawCreate(lpGuid, &lpDDTemp, nullptr);

	lpDDTemp->QueryInterface(IID_IDirectDraw2, (LPVOID*)&lpDD2);
	DXRelease(lpDDTemp);

	d3_info.DDCaps.dwSize = sizeof(DDCAPS);

	lpDD2->GetCaps(&d3_info.DDCaps, nullptr);
	lpDD2->SetCooperativeLevel(nullptr, DDSCL_NOWINDOWCHANGES | DDSCL_ALLOWMODEX | DDSCL_NORMAL);
	lpDD2->EnumDisplayModes(0, nullptr, (LPVOID)&d3_info, DXEnumDisplayModes);
	lpDD2->QueryInterface(IID_IDirect3D2, (void**)&lpD3D2);
	lpD3D2->EnumDevices(DXEnumDirect3D, (LPVOID)&d3_info);
	lpDD2->SetCooperativeLevel(nullptr, DDSCL_NORMAL);
	lpD3D2->Release();
	lpDD2->Release();

	++dd->nDDInfo;

	return DDENUMRET_OK;
}

/**
* Convert bitmask to shifts and bit cnt
*/
void DXBitMask2ShiftCnt(unsigned long mask, unsigned char* shift, unsigned char* cnt)
{
	unsigned long m;
	unsigned char n;
	for (n = 0, m = mask; !(m & 1); ++n, m >>= 1);
	*shift = n;
	for (n = 0; m & 1; ++n, m >>= 1);
	*cnt = n;
}

HRESULT __stdcall DXEnumDisplayModes(LPDDSURFACEDESC lpddsd, LPVOID lpContext)
{
	DIRECTDRAWINFO* DDInfo = (DIRECTDRAWINFO*)lpContext;

	if (DDInfo->nDisplayMode == 0)
		DDInfo->DisplayMode = (DISPLAYMODE*)malloc(sizeof(DISPLAYMODE));
	else DDInfo->DisplayMode = (DISPLAYMODE*)realloc(DDInfo->DisplayMode, sizeof(DISPLAYMODE) * (DDInfo->nDisplayMode + 1));

	auto& d3_info = DDInfo->DisplayMode[DDInfo->nDisplayMode];

	memset(&d3_info, 0, sizeof(DISPLAYMODE));

	// store details of display mode

	d3_info.w = lpddsd->dwWidth;
	d3_info.h = lpddsd->dwHeight;
	d3_info.bpp = lpddsd->ddpfPixelFormat.dwRGBBitCount;
	d3_info.bPalette = (lpddsd->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8);

	memcpy(&d3_info.ddsd, lpddsd, sizeof(DDSURFACEDESC));

	if (!d3_info.bPalette)
	{
		// get RGBA bit counts and shifts

		DXBitMask2ShiftCnt(lpddsd->ddpfPixelFormat.dwRBitMask, &d3_info.rshift, &d3_info.rbpp);
		DXBitMask2ShiftCnt(lpddsd->ddpfPixelFormat.dwGBitMask, &d3_info.gshift, &d3_info.gbpp);
		DXBitMask2ShiftCnt(lpddsd->ddpfPixelFormat.dwBBitMask, &d3_info.bshift, &d3_info.bbpp);

		if (lpddsd->ddpfPixelFormat.dwRGBAlphaBitMask)
			DXBitMask2ShiftCnt(lpddsd->ddpfPixelFormat.dwRGBAlphaBitMask, &d3_info.ashift, &d3_info.abpp);
	}

	++DDInfo->nDisplayMode;

	return DDENUMRET_OK;
}

HRESULT __stdcall DXEnumDirect3D(LPGUID lpGuid, LPSTR lpDeviceDesc, LPSTR lpDeviceName, LPD3DDEVICEDESC lpHWDesc, LPD3DDEVICEDESC lpHELDesc, LPVOID lpContext)
{
	DIRECTDRAWINFO* DDInfo = (DIRECTDRAWINFO*)lpContext;

	if (lpHWDesc->dwFlags == 0)
		return D3DENUMRET_OK;

	if (DDInfo->nD3DInfo == 0)
		DDInfo->D3DInfo = (DIRECT3DINFO*)malloc(sizeof(DIRECT3DINFO));
	else DDInfo->D3DInfo = (DIRECT3DINFO*)realloc(DDInfo->D3DInfo, sizeof(DIRECT3DINFO) * (DDInfo->nD3DInfo + 1));

	auto& d3_info = DDInfo->D3DInfo[DDInfo->nD3DInfo];

	memset(&d3_info, 0, sizeof(DIRECT3DINFO));

	// store details of driver information

	if (lpGuid)
	{
		d3_info.lpGuid = &d3_info.Guid;
		memcpy(&d3_info.Guid, lpGuid, sizeof(GUID));
	}
	else d3_info.lpGuid = nullptr;

	lstrcpy(d3_info.About, lpDeviceDesc);
	lstrcpy(d3_info.Name, lpDeviceName);

	// store device descriptions

	memcpy(&d3_info.DeviceDesc, lpHWDesc, sizeof(D3DDEVICEDESC));

	d3_info.bAlpha = !!d3_info.DeviceDesc.dpcTriCaps.dwAlphaCmpCaps;

	// check which video modes are compatible with driver and copy details

	for (int i = 0; i < DDInfo->nDisplayMode; ++i)
	{
		if (auto& display_mode_info = DDInfo->DisplayMode[i]; BPPToDDBD(display_mode_info.bpp) & d3_info.DeviceDesc.dwDeviceRenderBitDepth)
		{
			if (display_mode_info.bpp != 8)
			{
				if (d3_info.nDisplayMode == 0)
					d3_info.DisplayMode = (DISPLAYMODE*)malloc(sizeof(DISPLAYMODE));
				else d3_info.DisplayMode = (DISPLAYMODE*)realloc(d3_info.DisplayMode, sizeof(DISPLAYMODE) * (d3_info.nDisplayMode + 1));

				memcpy(&d3_info.DisplayMode[d3_info.nDisplayMode], &display_mode_info, sizeof(DISPLAYMODE));

				++d3_info.nDisplayMode;
			}
		}
	}

	// setup a D3DDEVICE for texture format enum
	// first try using the primary surface

	DDSURFACEDESC ddsd;

	DXInit(ddsd);

	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE;

	lpDDSurface = nullptr;

	DXSetCooperativeLevel(lpDD2, WindowHandle, DDSCL_EXCLUSIVE | DDSCL_NOWINDOWCHANGES);
	DXCreateSurface(lpDD2, &ddsd, &lpDDSurface);
	DXSetCooperativeLevel(lpDD2, WindowHandle, DDSCL_NORMAL);

	if (lpDDSurface)
	{
		// try to create device		

		lpD3DDevice = nullptr;
		DXCreateDirect3DDevice(lpD3D2, d3_info.Guid, lpDDSurface, &lpD3DDevice);

		// check if device is ok

		if (!lpD3DDevice)
		{
			// no device, try another surface format (555 offscreen)

			DXRelease(lpDDSurface);

			ddsd.dwSize = sizeof(ddsd);
			ddsd.dwFlags = DDSD_CAPS | DDSD_PIXELFORMAT | DDSD_HEIGHT | DDSD_WIDTH;
			ddsd.dwWidth = 100;
			ddsd.dwHeight = 100;
			ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY;
			ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
			ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
			ddsd.ddpfPixelFormat.dwFourCC = 0;
			ddsd.ddpfPixelFormat.dwRGBBitCount = 16;
			ddsd.ddpfPixelFormat.dwRBitMask = 0x7C00;
			ddsd.ddpfPixelFormat.dwGBitMask = 0x03E0;
			ddsd.ddpfPixelFormat.dwBBitMask = 0x001F;
			ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0x0000;

			DXCreateSurface(lpDD2, &ddsd, &lpDDSurface);

			// try to create the device again

			if (lpDDSurface)
				DXCreateDirect3DDevice(lpD3D2, d3_info.Guid, lpDDSurface, &lpD3DDevice);
		}

		// now enumerate the texture formats

		if (lpD3DDevice)
		{
			// enumerate texture formats for this d3ddevice

			d3_info.nTexture = 0;

			lpD3DDevice->EnumTextureFormats(DXEnumTextureFormats, (LPVOID)&d3_info);

			DXRelease(lpD3DDevice);
			DXRelease(lpDDSurface);
		}
	}

	++DDInfo->nD3DInfo;

	return D3DENUMRET_OK;
}

HRESULT __stdcall DXEnumTextureFormats(LPDDSURFACEDESC lpddsd, LPVOID lpContext)
{
	DIRECT3DINFO* D3DInfo = (DIRECT3DINFO*)lpContext;

	if (D3DInfo->nTexture == 0)
		D3DInfo->Texture = (D3DTEXTUREINFO*)malloc(sizeof(D3DTEXTUREINFO));
	else D3DInfo->Texture = (D3DTEXTUREINFO*)realloc(D3DInfo->Texture, sizeof(D3DTEXTUREINFO) * (D3DInfo->nTexture + 1));

	auto& d3_info = D3DInfo->Texture[D3DInfo->nTexture];

	memset(&d3_info, 0, sizeof(D3DTEXTUREINFO));
	memcpy(&d3_info.ddsd, lpddsd, sizeof(DDSURFACEDESC));

	// is it palettised

	if (lpddsd->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8)
		d3_info.bpp = 8;
	else if (lpddsd->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED4)
		return D3DENUMRET_OK;
	else
	{
		d3_info.bpp = lpddsd->ddpfPixelFormat.dwRGBBitCount;

		DXBitMask2ShiftCnt(lpddsd->ddpfPixelFormat.dwRBitMask, &d3_info.rshift, &d3_info.rbpp);
		DXBitMask2ShiftCnt(lpddsd->ddpfPixelFormat.dwGBitMask, &d3_info.gshift, &d3_info.gbpp);
		DXBitMask2ShiftCnt(lpddsd->ddpfPixelFormat.dwBBitMask, &d3_info.bshift, &d3_info.bbpp);

		if (lpddsd->ddpfPixelFormat.dwRGBAlphaBitMask != 0)
			DXBitMask2ShiftCnt(lpddsd->ddpfPixelFormat.dwRGBAlphaBitMask, &d3_info.ashift, &d3_info.abpp);
	}

	++D3DInfo->nTexture;

	return D3DENUMRET_OK;
}

DWORD BPPToDDBD(int bpp)
{
	switch (bpp)
	{
	case 1:  return DDBD_1;
	case 2:  return DDBD_2;
	case 4:  return DDBD_4;
	case 8:  return DDBD_8;
	case 16: return DDBD_16;
	case 24: return DDBD_24;
	case 32: return DDBD_32;
	}

	return 0;
}

bool DXCreateDirectDraw(DEVICEINFO* dd, LPDIRECTDRAW2* lpDD2)
{
	LPDIRECTDRAW lpDD;

	if (DirectDrawCreate(dd->DDInfo[0].lpGuid, &lpDD, nullptr) != DD_OK)
		return false;

	int RetVal = lpDD->QueryInterface(IID_IDirectDraw2, (LPVOID*)lpDD2);

	DXRelease(lpDD);

	return (RetVal == DD_OK);
}

bool DXCreateDirect3D(LPDIRECTDRAW2 lpDD2, LPDIRECT3D2* lpD3D2)
{
	return (lpDD2->QueryInterface(IID_IDirect3D2, (LPVOID*)lpD3D2) == DD_OK);
}

bool DXSetCooperativeLevel(LPDIRECTDRAW2 lpDD2, HWND WindowHandle, int Flags)
{
	return (lpDD2->SetCooperativeLevel(WindowHandle, Flags) == DD_OK);
}

bool DXSetVideoMode(LPDIRECTDRAW2 lpDD2, int w, int h, int bpp)
{
	return (lpDD2->SetDisplayMode(w, h, bpp, 0, 0) == DD_OK);
}

bool DXCreateSurface(LPDIRECTDRAW2 lpDD2, DDSURFACEDESC* ddsd, LPDIRECTDRAWSURFACE3* lpSurface)
{
	LPDIRECTDRAWSURFACE lps;

	if (lpDD2->CreateSurface(ddsd, &lps, nullptr) != DD_OK)
		return false;

	int ret_val = lps->QueryInterface(IID_IDirectDrawSurface3, (LPVOID*)lpSurface);

	DXRelease(lps);

	return (ret_val == DD_OK);
}

bool DXGetAttachedSurface(LPDIRECTDRAWSURFACE3 lpPrimary, DDSCAPS* ddscaps, LPDIRECTDRAWSURFACE3* lpAttached)
{
	return (lpPrimary->GetAttachedSurface(ddscaps, lpAttached) == DD_OK);
}

bool DXAddAttachedSurface(LPDIRECTDRAWSURFACE3 lpSurface, LPDIRECTDRAWSURFACE3 lpAddSurface)
{
	return (lpSurface->AddAttachedSurface(lpAddSurface) == DD_OK);
}

bool DXCreateDirect3DDevice(LPDIRECT3D2 lpD3D2, GUID Guid, LPDIRECTDRAWSURFACE3 lpSurface, LPDIRECT3DDEVICE2* lpD3DDevice2)
{
	return (lpD3D2->CreateDevice(Guid, (LPDIRECTDRAWSURFACE)lpSurface, lpD3DDevice2) == DD_OK);
}

bool DXCreateViewPort(LPDIRECT3D2 lpD3D, LPDIRECT3DDEVICE2 lpD3DDevice, int w, int h, IDirect3DViewport2** lpViewport)
{
	if (lpD3D->CreateViewport(lpViewport, nullptr) != DD_OK)
		return false;

	if (lpD3DDevice->AddViewport(*lpViewport) != DD_OK)
		return false;

	float inva = float(h) / float(w);

	D3DVIEWPORT2 viewdata;

	DXInit(viewdata);

	viewdata.dwX = 0;
	viewdata.dwY = 0;
	viewdata.dwWidth = w;
	viewdata.dwHeight = h;
	viewdata.dvClipX = -1.f;
	viewdata.dvClipY = inva;
	viewdata.dvClipWidth = 2.f;
	viewdata.dvClipHeight = 2.f * inva;
	viewdata.dvMinZ = 0.f;
	viewdata.dvMaxZ = 1.f;

	if ((*lpViewport)->SetViewport2(&viewdata) != DD_OK)
		return false;

	return (lpD3DDevice->SetCurrentViewport(*lpViewport) == DD_OK);
}