#pragma once

#define MAGICNUM	(double)((1 << 26) * ((1 << 26) * 1.5))
#define FTOL(x)		((fconv = (x) + (MAGICNUM)), *(int*)(&fconv))

#ifdef GAMEDEBUG
inline int DrawPrimitiveCnt;
inline int PolysDrawn;
inline int PolysPushed;
inline int PolyTest;
#endif

void InitDrawPrimitive(LPDIRECT3DDEVICE2 lpD3D);

HRESULT DrawPrimitive(D3DPRIMITIVETYPE dptPrimitiveType, D3DVERTEXTYPE dvtVertexType, LPVOID lpvVertices, DWORD dwVertexCount, DWORD dwFlags);
HRESULT BeginScene();
HRESULT EndScene();
HRESULT SetRenderState(D3DRENDERSTATETYPE dwRenderStateType, DWORD dwRenderState);