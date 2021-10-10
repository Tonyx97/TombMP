#include <shared/defs.h>

#include "standard.h"
#include "global.h"
#include "texture.h"
#include "drawprimitive.h"
#include "hwrender.h"
#include "input.h"

#include <3dsystem/hwinsert.h>

#include <main.h>

#include <shared/scripting/resource.h>
#include <shared/scripting/resource_system.h>
#include <shared/timer_system/timer_system.h>

#include <game/laraanim.h>

#include <scripting/events.h>
#include <scripting/debugger.h>

#include <mp/client.h>
#include <mp/chat.h>
#include <ui/ui.h>

D3DTLVERTEX UnRollBuffer[MAX_TLVERTICES];

int LanTextureHandle[MAX_D3D_TEXTURES],
	LnMaxPolys = 0,
	LnMaxVertices = 0;

bool GtZBufferWriteEnabled,
	 GtZBufferCompareEnabled;

void setup_screen_size(int new_sx, int new_sy)
{
	if (!App.DeviceInfoPtr)
		return;

	auto [sw, sh] = g_window->get_resolution();

	if (new_sx == -1 || new_sy == -1)
	{
		new_sx = sw;
		new_sy = sh;
	}

	if (new_sx > sw)
		new_sx = sw;

	if (new_sy > sh)
		new_sy = sh;

	auto xoff = (sw - new_sx) / 2,
		 yoff = (sh - new_sy) / 2;

	phd_InitWindow(xoff, yoff, new_sx, new_sy, 20, DPQ_END, sw, sh);

	game_setup.dump_x = (short)xoff;
	game_setup.dump_y = (short)yoff;
	game_setup.dump_width = (short)new_sx;
	game_setup.dump_height = (short)new_sy;

	fullscreen_clear_needed = true;
}

void HWR_InitVertexList()
{
	CurrentTLVertex = VertexBuffer;
}

void hwr_init()
{
	VertexBuffer = new D3DTLVERTEX[MAX_TLVERTICES]();

	for (int i = 0; i < ArraySize(LanTextureHandle); ++i)
		LanTextureHandle[i] = -1;

	phd_InitBuffers();
}

void hwr_destroy()
{
	phd_DestroyBuffers();

	delete[] VertexBuffer;
}

void hwr_init_state()
{
	App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_FILLMODE, g_window->get_fill_mode());
	App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_SHADEMODE, g_window->get_shade_mode());
	App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_TEXTUREMAG, g_window->get_filter());
	App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, g_window->is_dither_enabled());
	App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, g_window->is_perspective_enabled());
	App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_TEXTUREADDRESS, D3DTADDRESS_CLAMP);
	App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATE);
	App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);
	App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
	App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
	App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, false);

	LnAlphaEnableRenderState = D3DRENDERSTATE_ALPHABLENDENABLE;

	HWR_ResetCurrentTexture();
	HWR_ResetColorKey();
}

void HWR_ResetCurrentTexture()
{
	SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, (GhCurTextureHandle = 0));
}

void HWR_ResetColorKey()
{
	GtColorKeyEnabled = false;

	SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, false);
}

void HWR_ResetZBuffer()
{
	GtZBufferWriteEnabled = GtZBufferCompareEnabled = false;

	if (App.lpZBuffer)
		App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_ALWAYS);
	else
	{
		App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, false);
		App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, false);
	}
}

void HWR_SetCurrentTexture(DWORD dwHandle)
{
	if (dwHandle != GhCurTextureHandle)
	{
		SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, dwHandle);
		GhCurTextureHandle = dwHandle;
	}
}

void HWR_EnableColorKey(bool tEnable)
{
	SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, tEnable);
}

void HWR_EnableZBuffer(bool tWrite, bool tCompare)
{
	if (App.lpZBuffer)
	{
		if (tWrite != GtZBufferWriteEnabled)
		{
			App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, tWrite);
			GtZBufferWriteEnabled = tWrite;
		}

		if (tCompare != GtZBufferCompareEnabled)
		{
			if (App.lpZBuffer)
				App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, tCompare ? D3DCMP_LESSEQUAL : D3DCMP_ALWAYS);
			else App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, tCompare);

			GtZBufferCompareEnabled = tCompare;
		}
	}
}

void HWR_BeginScene()
{
	HWR_GetAllTextureHandles();

	BeginScene();

	// begin UI rendering
	
	g_ui->begin();

	g_resource->trigger_event(events::renderer::ON_SCENE_BEGIN);

	if (App.lpZBuffer != 0)
		InitBuckets();
}

void HWR_EndScene()
{
	// dispatch timers

	g_timer->update();

	// dispatch network packets

	g_client->dispatch_packets();

	// update chat

	g_chat->update();

	// update debugger

	g_debugger->update();

	// add ui from scripts

	g_ui->update();

	g_ui->add_text(std::format("FPS:             {}", int(App.fps)).c_str(), 10, 2 * 16, 18.f, { 0.f, 255.f, 255.f, 1.f }, false);

#ifdef GAMEDEBUG
	g_ui->add_text(std::format("DP Calls:        {}", DrawPrimitiveCnt).c_str(), 10, 3 * 16, 18.f, { 0.f, 255.f, 255.f, 1.f }, false);
	g_ui->add_text(std::format("BF List:         {}", (int)surfacenumbf).c_str(), 10, 4 * 16, 18.f, { 0.f, 255.f, 255.f, 1.f }, false);
	g_ui->add_text(std::format("Polygons:        {}", PolysDrawn).c_str(), 10, 5 * 16, 18.f, { 0.f, 255.f, 255.f, 1.f }, false);
	g_ui->add_text(std::format("Polygons Pushed: {}", PolysPushed).c_str(), 10, 6 * 16, 18.f, { 0.f, 255.f, 255.f, 1.f }, false);
	g_ui->add_text(std::format("PolyTest:        {}", PolyTest).c_str(), 10, 7 * 16, 18.f, { 0.f, 255.f, 255.f, 1.f }, false);
#endif

	// finish UI rendering and render everything

	g_ui->end();

	g_resource->trigger_event(events::renderer::ON_SCENE_END);

	// finish game rendering

	EndScene();
}

void HWR_DrawRoutine(int16_t* pInfo)
{
	auto pV = *((D3DTLVERTEX**)(pInfo + 2));

	int nTPage = pInfo[0],
		nV = pInfo[1];

	int nRoutine = *pInfo++;

	switch (nRoutine)
	{
	case DRAW_TLV_GT:
	case DRAW_TLV_WGT:
	{
		HWR_SetCurrentTexture(GahTextureHandle[nTPage]);
		HWR_EnableColorKey(nRoutine == DRAW_TLV_WGT);

		DrawPrimitive(D3DPT_TRIANGLEFAN, D3DVT_TLVERTEX, pV, nV, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);

		break;
	}
	case DRAW_TLV_G:
	{
		HWR_SetCurrentTexture(0);
		HWR_EnableColorKey(false);

		DrawPrimitive(D3DPT_TRIANGLEFAN, D3DVT_TLVERTEX, pV, nV, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);

		break;
	}
	case DRAW_TLV_L:
	{
		HWR_SetCurrentTexture(0);
		HWR_EnableColorKey(false);

		DrawPrimitive(D3DPT_LINESTRIP, D3DVT_TLVERTEX, pV, nV, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);

		break;
	}
	case DRAW_TLV_GA:
	{
		HWR_SetCurrentTexture(0);

		DWORD dwAlphaEnabled;

		App.lpD3DDevice->GetRenderState(LnAlphaEnableRenderState, &dwAlphaEnabled);
		App.lpD3DDevice->SetRenderState(LnAlphaEnableRenderState, true);

		DrawPrimitive(D3DPT_TRIANGLEFAN, D3DVT_TLVERTEX, pV, nV, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);

		App.lpD3DDevice->SetRenderState(LnAlphaEnableRenderState, dwAlphaEnabled);

		break;
	}
	case DRAW_TLV_GTA:
	{
		HWR_SetCurrentTexture(GahTextureHandle[nTPage]);
		HWR_EnableColorKey(true);

		App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
		App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);

		SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, true);

		DrawPrimitive(D3DPT_TRIANGLEFAN, D3DVT_TLVERTEX, pV, nV, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);

		SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, false);

		App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
		App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
	}
	}
}

void HWR_DrawUnRolledList(int cnt, D3DTLVERTEX* pVB, int nRoutine, int nTPage)
{
	switch (nRoutine)
	{
	case DRAW_TLV_GT:
	case DRAW_TLV_WGT:
		HWR_SetCurrentTexture(GahTextureHandle[nTPage]);

		if (nRoutine == DRAW_TLV_WGT)
		{
			HWR_EnableZBuffer(true, true);
			HWR_EnableColorKey(true);
		}

		DrawPrimitive(D3DPT_TRIANGLELIST, D3DVT_TLVERTEX, pVB, cnt, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);

		if (nRoutine == DRAW_TLV_WGT)
			HWR_EnableZBuffer(false, true);

		break;
	case DRAW_TLV_G:
		HWR_SetCurrentTexture(0);
		HWR_EnableColorKey(false);
		DrawPrimitive(D3DPT_TRIANGLELIST, D3DVT_TLVERTEX, pVB, cnt, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
		break;
	case DRAW_TLV_GA:
		HWR_SetCurrentTexture(0);
		App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, true);
		DrawPrimitive(D3DPT_TRIANGLELIST, D3DVT_TLVERTEX, pVB, cnt, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
		App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, false);
		break;
	case DRAW_TLV_GTA:
		HWR_SetCurrentTexture(GahTextureHandle[nTPage]);
		HWR_EnableColorKey(true);
		App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
		App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
		SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, true);
		DrawPrimitive(D3DPT_TRIANGLELIST, D3DVT_TLVERTEX, pVB, cnt, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
		SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, false);
		App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
		App.lpD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
		break;
	case DRAW_TLV_L:
		HWR_SetCurrentTexture(0);
		HWR_EnableColorKey(false);
		DrawPrimitive(D3DPT_LINELIST, D3DVT_TLVERTEX, pVB, cnt, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
		break;
	}
}

void HWR_DrawPolyListFB(int num, int32_t* sortptr)
{
	auto pSort = (int32_t*)sortptr + ((num << 1) - 2);

	for (int nSurf = 0; nSurf < num; ++nSurf)
	{
		HWR_DrawRoutine((int16_t*)(*pSort));
		pSort -= 2;
	}
}

void HWR_DrawPolyList(int num, int32_t* sortptr)
{
	auto pSort = (int32_t*)sortptr;

	for (int nSurf = 0; nSurf < num; ++nSurf, pSort += 10)
		HWR_DrawRoutine((int16_t*)(*pSort));
}

void TLVERT_To_TLVERT(D3DTLVERTEX* s, D3DTLVERTEX* d)
{
	d->sx = s->sx;
	d->sy = s->sy;
	d->sz = s->sz;
	d->rhw = s->rhw;
	d->tu = s->tu;
	d->tv = s->tv;
	d->color = s->color;
	d->specular = s->specular;
}

void HWR_DrawPolyListBF(int num, int32_t* sortptr)
{
	if (num == 0)
		return;

	auto pSort = (int32_t*)sortptr;
	auto pInfo = (int16_t*)(*pSort);

	int vbcnt = 0,
		nV = pInfo[1],
		Routine1 = *pInfo++,
		Tpage1 = pInfo[0];

	auto pV = *((D3DTLVERTEX**)(pInfo + 2));

	for (int nSurf = 0; nSurf < num; ++nSurf)
	{
		pInfo = (int16_t*)(*pSort);

		int Routine2 = *pInfo++,
			Tpage2 = pInfo[0];

		nV = pInfo[1];
		pV = *((D3DTLVERTEX**)(pInfo + 2));

		if (Routine2 != Routine1 || Tpage1 != Tpage2)
		{
			HWR_DrawUnRolledList(vbcnt, &UnRollBuffer[0], Routine1, Tpage1);

			vbcnt = 0;
			Routine1 = Routine2;
			Tpage1 = Tpage2;
		}

		if (Routine1 != DRAW_TLV_L)
		{
			// Add Fan Into VertexBuffer List

			auto pFanStart = pV;

			TLVERT_To_TLVERT(pV, &UnRollBuffer[vbcnt + 0]);

			++pV;

			TLVERT_To_TLVERT(pV, &UnRollBuffer[vbcnt + 1]);

			++pV;

			TLVERT_To_TLVERT(pV, &UnRollBuffer[vbcnt + 2]);

			vbcnt += 3;
			nV -= 3;

			for (int n = nV; n > 0; --n)
			{
				TLVERT_To_TLVERT(pFanStart, &UnRollBuffer[vbcnt + 0]);
				TLVERT_To_TLVERT(pV, &UnRollBuffer[vbcnt + 1]);

				++pV;

				TLVERT_To_TLVERT(pV, &UnRollBuffer[vbcnt + 2]);

				vbcnt += 3;
			}
		}
		else
		{
			// Add Lines

			TLVERT_To_TLVERT(pV, &UnRollBuffer[vbcnt + 0]);

			++pV;

			TLVERT_To_TLVERT(pV, &UnRollBuffer[vbcnt + 1]);

			vbcnt += 2;
		}

		pSort += 10;
	}

	if (vbcnt > 0)
		HWR_DrawUnRolledList(vbcnt, &UnRollBuffer[0], Routine1, Tpage1);
}

void HWR_LoadTexturePages(int pages, char* img)
{
	HWR_FreeTexturePages();

	for (int page = 0; page < pages; ++page, img += 0x20000)
	{
		int nHandle = DXTextureAdd(256, 256, (uint16_t*)img, DXTextureList, 555);
		
		LanTextureHandle[page] = (nHandle >= 0 ? nHandle : -1);
	}

	HWR_GetAllTextureHandles();
}

void HWR_FreeTexturePages()
{
	for (int nPage = 0; nPage < ArraySize(LanTextureHandle); ++nPage)
	{
		if (int nHandle = LanTextureHandle[nPage]; nHandle >= 0)
		{
			DXTextureFree(nHandle, DXTextureList);
			LanTextureHandle[nPage] = -1;
		}

		GahTextureHandle[nPage] = 0;
	}
}

void HWR_GetAllTextureHandles()
{
	for (int page = 0; page < ArraySize(LanTextureHandle); ++page)
	{
		if (int nHandle = LanTextureHandle[page]; nHandle >= 0)
			GahTextureHandle[page] = DXTextureGetHandle(nHandle, DXTextureList);
		else GahTextureHandle[page] = 0;
	}
}