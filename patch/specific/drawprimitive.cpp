#include "standard.h"
#include "global.h"
#include "drawprimitive.h"

#include <main.h>

LPDIRECT3DDEVICE2 g_d3device;

void InitDrawPrimitive(LPDIRECT3DDEVICE2 lpD3D)
{
	g_d3device = lpD3D;
}

HRESULT BeginScene()
{
#ifdef GAMEDEBUG
	DrawPrimitiveCnt = 0;
	PolysDrawn = 0;
	PolysPushed = 0;
	PolyTest = 0;
#endif

	WinFrameRate();

	return g_d3device->BeginScene();
}

HRESULT EndScene()
{
	return g_d3device->EndScene();
}

HRESULT SetRenderState(D3DRENDERSTATETYPE dwRenderStateType, DWORD dwRenderState)
{
	return g_d3device->SetRenderState(dwRenderStateType, dwRenderState);
}

HRESULT DrawPrimitive(D3DPRIMITIVETYPE dptPrimitiveType, D3DVERTEXTYPE dvtVertexType, LPVOID lpvVertices, DWORD dwVertexCount, DWORD dwFlags)
{
#ifdef GAMEDEBUG
	switch (dptPrimitiveType)
	{
	case D3DPT_TRIANGLELIST: PolysDrawn += dwVertexCount / 3;		break;
	case D3DPT_TRIANGLEFAN:  PolysDrawn += 1 + (dwVertexCount - 3); break;
	}

	++DrawPrimitiveCnt;
#endif

	return g_d3device->DrawPrimitive(dptPrimitiveType, dvtVertexType, lpvVertices, dwVertexCount, dwFlags);
}