#pragma once

struct DISPLAYMODE
{
	DDSURFACEDESC ddsd;		// Surface Description

	int w,					// Width
		h;					// Height

	unsigned long bpp;		// Bits Per Pixel

	unsigned char rbpp,		// Red Bits Per Pixel
				  gbpp,		// Green Bits Per Pixel
				  bbpp,		// Blue Bits Per Pixel
				  abpp,		// Alpha Bits Per Pixel
				  rshift,	// Shifts For Red 
				  gshift,	// Shifts For Green
				  bshift,	// Shifts For Blue
				  ashift;	// Shifts For Alpha

	bool bPalette;			// Is It Palettised
};

struct D3DTEXTUREINFO
{
	DDSURFACEDESC ddsd;		// Texture Format Description

	DDPIXELFORMAT ddpf;		// Texture Pixel Format

	unsigned long bpp;		// Bits Per Pixel (if Palettised)

	unsigned char rbpp,		// Red Bits Per Pixel 
				  gbpp,		// Green Bits Per Pixel
				  bbpp,		// Blue Bits Per Pixel
				  abpp,		// Alpha Bits Per Pixel
				  rshift,	// Shifts For Red
				  gshift,	// Shifts For Green
				  bshift,	// Shifts For Blue
				  ashift;	// Shifts For Alpha
};

struct DIRECT3DINFO
{
	D3DDEVICEDESC DeviceDesc;	// Device Description

	char Name[30],				// Name String
		 About[80];				// Description String

	LPGUID lpGuid;				// GUID

	GUID Guid;		

	DISPLAYMODE* DisplayMode;	// Ptr To Compatible Display Modes

	D3DTEXTUREINFO*	Texture;	// Texture Info

	int nDisplayMode,			// Number Of Display Modes
		nTexture;				// Number Of Texture Formats

	bool bAlpha;				// Does It Support Alpha
};

struct DIRECTDRAWINFO
{
	DDCAPS DDCaps;				// Device Capabilites

	char Name[30],				// Name String   
		 About[80];				// Description String

	LPGUID lpGuid;				// GUID

	GUID Guid;

	DISPLAYMODE* DisplayMode;	// Ptr To Display Mode Structure

	DIRECT3DINFO* D3DInfo;		// Ptr To D3D Info Structure

	int nDisplayMode,			// Number Of Display Modes	
		nD3DInfo;				// Number Of D3D Drivers
};

struct DEVICEINFO
{	
	DIRECTDRAWINFO*	DDInfo;		// Ptr To Info Structure
	int nDDInfo;				// Entries
};

#define DXRelease(ptr)			{ if (ptr) { ptr->Release(); ptr = nullptr; } }
#define DXInit(dstruct)			{ memset(&dstruct, 0, sizeof(dstruct)); dstruct.dwSize = sizeof(dstruct); }
#define DXD3DGuid(a)			a->DDInfo[0].D3DInfo[0].Guid
#define DXD3D(a)				a->DDInfo[0].D3DInfo[0]
#define DXD3DTexture(a)			a->DDInfo[0].D3DInfo[0].Texture[6]

#define DXTEXTURERGBA(i, r, g, b, a)	r >> (8 - DXD3DTexture(i).rbpp)<<(DXD3DTexture(i).rshift)|\
										g >> (8 - DXD3DTexture(i).gbpp)<<(DXD3DTexture(i).gshift)|\
										b >> (8 - DXD3DTexture(i).bbpp)<<(DXD3DTexture(i).bshift)|\
										a >> (8 - DXD3DTexture(i).abpp)<<(DXD3DTexture(i).ashift)	

void WinFreeDX();
bool WinDXInit(DEVICEINFO* DeviceInfo);
float WinFrameRate();
void DXGetDeviceInfo(DEVICEINFO* dd, HWND);
void DXFreeDeviceInfo(DEVICEINFO* dd);
BOOL __stdcall DXEnumDirectDraw(GUID* lpGuid, LPSTR lpDeviceDesc, LPSTR lpDeviceName, LPVOID lpContext);
HRESULT	__stdcall DXEnumDisplayModes(LPDDSURFACEDESC lpddsd, LPVOID lpContext);
HRESULT	__stdcall DXEnumDirect3D(LPGUID lpGuid, LPSTR lpDeviceDesc, LPSTR lpDeviceName, LPD3DDEVICEDESC lpHWDesc, LPD3DDEVICEDESC lpHELDesc, LPVOID lpContext);
HRESULT __stdcall DXEnumTextureFormats(LPDDSURFACEDESC lpDDSD, LPVOID lpContext);
void DXBitMask2ShiftCnt(unsigned long mask, unsigned char* shift, unsigned char* cnt);
DWORD BPPToDDBD(int bpp);

bool DXCreateDirectDraw(DEVICEINFO* dd, LPDIRECTDRAW2* lpDD2);
bool DXCreateDirect3D(LPDIRECTDRAW2 lpDD2, LPDIRECT3D2* lpD3D2);
bool DXSetCooperativeLevel(LPDIRECTDRAW2 lpDD2, HWND WindowHandle, int Flags);
bool DXSetVideoMode(LPDIRECTDRAW2 lpDD2, int w, int h, int bpp);
bool DXCreateSurface(LPDIRECTDRAW2 lpDD2, DDSURFACEDESC* ddsd, LPDIRECTDRAWSURFACE3* lpSurface);
bool DXGetAttachedSurface(LPDIRECTDRAWSURFACE3 lpPrimary, DDSCAPS* ddscaps, LPDIRECTDRAWSURFACE3* lpAttached);
bool DXAddAttachedSurface(LPDIRECTDRAWSURFACE3 lpSurface, LPDIRECTDRAWSURFACE3 lpAddSurface);
bool DXCreateDirect3DDevice(LPDIRECT3D2 lpD3D2, GUID Guid, LPDIRECTDRAWSURFACE3 lpSurface, LPDIRECT3DDEVICE2* lpD3DDevice2);
bool DXCreateViewPort(LPDIRECT3D2 lpD3D, LPDIRECT3DDEVICE2 lpD3DDevice, int w, int h, IDirect3DViewport2** lpViewport);