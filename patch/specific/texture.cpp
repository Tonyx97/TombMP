import prof;

#include "standard.h"
#include "global.h"

#include <main.h>
#include "texture.h"
#include "hwrender.h"

bool DXTextureGreyScale = false;

void DXTextureSetGreyScale(bool v)
{
	DXTextureGreyScale = v;
}

/**
* Initialize texture lists
*/
void DXTextureInit(DXTEXTURE TextureList[])
{
	if (TextureList)
		Zero(TextureList);
}

IDirect3DTexture2* DXTextureGetInterface(IDirectDrawSurface3* pSurf)
{
	if (IDirect3DTexture2* pTexture; !DX_TRY(pSurf->QueryInterface(IID_IDirect3DTexture2, (void**)&pTexture)))
		return pTexture;

	return nullptr;
}

int DXTextureFindTextureSlot(DXTEXTURE TextureList[])
{
	for (int i = 0; i < MAX_D3D_TEXTURES; ++i)
		if (!(TextureList[i].dwFlags & TX_FLAG_SLOTUSED))
			return i;

	return -1;
}

bool DXTextureMakeSystemSurface(DXTEXTURE& rTexture)
{
	DDSURFACEDESC ddsd;

	DXInit(ddsd);

	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY | DDSCAPS_TEXTURE;
	ddsd.dwWidth = rTexture.nWidth;
	ddsd.dwHeight = rTexture.nHeight;
	ddsd.ddpfPixelFormat = DXD3DTexture(App.DeviceInfoPtr).ddsd.ddpfPixelFormat;

	if (DX_TRY(DD_CreateSurface(ddsd, rTexture.pSystemSurface)))
	{
		prof::print(RED, "TX_MakeSystemSurface: Can't create surface");
		return false;
	}

	if (rTexture.pPalette)
	{
		if (DX_TRY(rTexture.pSystemSurface->SetPalette(rTexture.pPalette)))
		{
			prof::print(RED, "TX_MakeSystemSurface: Can't attach palette");
			return false;
		}
	}

	return true;
}

bool DXTextureMakeDeviceSurface(DXTEXTURE& rTexture)
{
	DDSURFACEDESC ddsd;

	DXInit(ddsd);

	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_TEXTURE | DDSCAPS_ALLOCONLOAD;
	ddsd.dwWidth = rTexture.nWidth;
	ddsd.dwHeight = rTexture.nHeight;
	ddsd.ddpfPixelFormat = DXD3DTexture(App.DeviceInfoPtr).ddsd.ddpfPixelFormat;

	if (DX_TRY(DD_CreateSurface(ddsd, rTexture.pDeviceSurface)))
	{
		prof::print(RED, "TX_MakeDeviceSurface: Can't create surface");
		return false;
	}

	if (rTexture.pPalette)
	{
		if (DX_TRY(rTexture.pDeviceSurface->SetPalette(rTexture.pPalette)))
		{
			prof::print(RED, "TX_MakeDeviceSurface: Can't attach palette");
			RELEASE_LOGTYPE(rTexture.pDeviceSurface);
			return false;
		}

		DDCOLORKEY ddck { 0 , 0 };

		if (DX_TRY(rTexture.pDeviceSurface->SetColorKey(DDCKEY_SRCBLT, &ddck)))
		{
			prof::print(RED, "Can't set colorkey");
			RELEASE_LOGTYPE(rTexture.pDeviceSurface);
			return false;
		}
	}

	if (!(rTexture.pTexture = DXTextureGetInterface(rTexture.pDeviceSurface)))
	{
		prof::print(RED, "Can't get texture interface");
		RELEASE_LOGTYPE(rTexture.pDeviceSurface);
		return false;
	}

	if (DX_TRY(rTexture.pTexture->GetHandle(App.lpD3DDevice, &rTexture.hTexture)))
	{
		prof::print(RED, "Can't get texture handle");

		RELEASE_LOGTYPE(rTexture.pTexture);
		RELEASE_LOGTYPE(rTexture.pDeviceSurface);

		rTexture.hTexture = 0;

		return false;
	}

	return true;
}

void DXTextureReleaseDeviceSurface(DXTEXTURE& rTexture)
{
	HWR_ResetCurrentTexture();

	RELEASE_LOGTYPE(rTexture.pTexture);
	RELEASE_LOGTYPE(rTexture.pDeviceSurface);

	rTexture.hTexture = 0;
}

int DXTextureNew(int nWidth, int nHeight, DXTEXTURE TextureList[])
{
	if (int nSlot = DXTextureFindTextureSlot(TextureList); nSlot >= 0)
	{
		auto& rTexture = TextureList[nSlot];

		Zero(rTexture);

		rTexture.dwFlags = TX_FLAG_SLOTUSED;
		rTexture.nWidth = nWidth;
		rTexture.nHeight = nHeight;
		rTexture.pPalette = nullptr;

		if (!DXTextureMakeSystemSurface(rTexture))
		{
			prof::print(RED, "Can't make system texture surface");
			return -1;
		}

		if (DXTextureMakeDeviceSurface(rTexture))
			prof::print(GREEN, "Added texture in slot {} | D3D handle={}", nSlot, rTexture.hTexture);
		else prof::print(RED, "Texture {} has no device surface", nSlot);

		return nSlot;
	}

	prof::print(RED, "Out of texture slots");

	return -1;
}

void DXTextureCleanup(int nSlot, DXTEXTURE TextureList[])
{
	auto& rTexture = TextureList[nSlot];

	DXTextureReleaseDeviceSurface(rTexture);

	RELEASE_LOGTYPE(rTexture.pSystemSurface);

	rTexture.dwFlags = 0;
}

void DXTextureFree(int nHandle, DXTEXTURE TextureList[])
{
	if (nHandle >= 0 && (TextureList[nHandle].dwFlags & TX_FLAG_SLOTUSED))
		DXTextureCleanup(nHandle, TextureList);
	else prof::print(RED, "Tried to cleanup invalid texture");
}

void DXTextureFreeAll(DXTEXTURE TextureList[])
{
	for (int i = 0; i < MAX_D3D_TEXTURES; ++i)
		if (TextureList[i].dwFlags & TX_FLAG_SLOTUSED)
			DXTextureCleanup(i, TextureList);
}

bool DXTextureRestore(int nHandle, bool tForce, DXTEXTURE TextureList[])
{
	prof::print(YELLOW, "DXTexture Restore: Entry...");

	if (nHandle < 0)
	{
		prof::print(RED, "Tried to restore invalid texture");
		return false;
	}

	DXTEXTURE& rTexture = TextureList[nHandle];

	if (rTexture.pDeviceSurface == rTexture.pSystemSurface) return true;

	if (tForce || (!rTexture.pDeviceSurface) || (DX_TRY(DD_EnsureSurfaceAvailable(rTexture.pDeviceSurface))))
	{
		DXTextureReleaseDeviceSurface(rTexture);

		if (!DXTextureMakeDeviceSurface(rTexture))
		{
			prof::print(RED, "Texture device surface lost | can't recreate ({})", nHandle);
			return false;
		}

		prof::print(YELLOW, "Re-created device surface for texture {} | D3D Handle = {}", nHandle, rTexture.hTexture);
	}

	DX_TRY(DD_EnsureSurfaceAvailable(rTexture.pSystemSurface));

	auto pSrc = DXTextureGetInterface(rTexture.pSystemSurface);
	if (!pSrc)
	{
		prof::print(RED, "DXTextureRestore({}): can't get src iface", nHandle);
		return false;
	}

	if (rTexture.pTexture->Load(pSrc) != D3D_OK)
	{
		prof::print(RED, "DXTextureRestore({}): can't load texture", nHandle);
		RELEASE_NOLOG(pSrc);
		return false;
	}

	RELEASE_NOLOG(pSrc);

	return true;
}

bool DXTextureRestoreAll(bool tForce, DXTEXTURE TextureList[])
{
	prof::print(YELLOW, "DXTextureRestoreAll");
	
	bool ok = true;

	for (int i = 0; i < MAX_D3D_TEXTURES; ++i)
		if (TextureList[i].dwFlags & TX_FLAG_SLOTUSED)
			ok = ok && DXTextureRestore(i, tForce, TextureList);

	return ok;
}

DWORD DXTextureGetHandle(int nHandle, DXTEXTURE TextureList[])
{
	if (nHandle < 0)
	{
		prof::print(RED, "Tried to get handle of invalid texture");
		return 0;
	}

	auto& rTexture = TextureList[nHandle];

	if (rTexture.pDeviceSurface && (rTexture.pDeviceSurface->IsLost() == DDERR_SURFACELOST))
		DXTextureRestore(nHandle, true, TextureList);

	return (DWORD)rTexture.hTexture;
}

int DXTextureAdd(int nWidth, int nHeight, uint16_t* pTexture, DXTEXTURE TextureList[], int BitFormat)
{
	prof::print(YELLOW, "DXTextureAdd: Create Texture");

	if (int nHandle = DXTextureNew(nWidth, nHeight, TextureList); nHandle >= 0)
	{
		auto& rTexture = TextureList[nHandle];

		DDSURFACEDESC ddsd;

		if (DX_FAILED(DD_LockSurface(rTexture.pSystemSurface, ddsd)))
			return -1;

		auto pDest = (char*)ddsd.lpSurface;

		int nPitch = ddsd.lPitch;

		for (int nY = nHeight; nY--; pDest += nPitch)
		{
			auto pDest2 = (uint8_t*)pDest;

			for (int nX = nWidth; nX--;)
			{
				auto wPixel = *pTexture++;

				int bA, bR, bG, bB;

				switch (BitFormat)
				{
				case 555:
					bA = int16_t(wPixel) >> 15;
					bR = (wPixel >> 7) & 0xf8;
					bG = (wPixel >> 2) & 0xf8;
					bB = (wPixel << 3) & 0xf8;
					break;
				case 888:
				case 565:
					bA = 0;
					bR = (wPixel >> 8) & 0xf8;
					bG = (wPixel >> 3) & 0xf8;
					bB = (wPixel << 3) & 0xf8;
					break;
				}

				if (DXTextureGreyScale)
				{
					bR = bB;
					bG = bB;
				}

				uint32_t dwRGBA = DXTEXTURERGBA(App.DeviceInfoPtr, bR, bG, bB, bA);
				
				for (int nBits = DXD3DTexture(App.DeviceInfoPtr).bpp; nBits > 0; nBits -= 8)
				{
					*pDest2++ = (uint8_t)dwRGBA;
					dwRGBA >>= 8;
				}
			}
		}

		// copy texture to our buffer

		DD_UnlockSurface(rTexture.pSystemSurface, ddsd);

		if (!DXTextureRestore(nHandle, true, TextureList))
			prof::print(RED, "DXTextureAdd: Failed To Restore Texture");

		return nHandle;
	}

	prof::print(RED, "DXTextureAdd: Can't create new texture");

	return -1;
}

void DXTextureFinish(DXTEXTURE TextureList[])
{
	DXTextureFreeAll(TextureList);
}