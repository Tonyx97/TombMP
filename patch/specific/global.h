#pragma once

#include <game/anim.h>

#define _X	0
#define _Y  1
#define _Z  2
#define XYZ	3

#define DPQ_S (DPQ_START << W2V_SHIFT)
#define DPQ_E (DPQ_END << W2V_SHIFT)

#define S_GOURAUD_COL(a,b,c,S) ((512 - S) << 4)

#define ArraySize(a)	(sizeof(a) / sizeof(*a))
#define Zero(thing)		memset(&thing, 0, sizeof(thing))

#define RELEASE_NOLOG(x)	if (x) { (x)->Release(); (x) = 0; }
#define RELEASE_LOGTYPE(x)	if (x) { int nRef = (x)->Release(); prof::print(YELLOW, "Releasing " #x " -> {}", nRef); (x) = 0; }
#define RELEASE(x)			RELEASE_LOGTYPE(x)

#define DXCB_FRONT		1
#define DXCB_BACK		2
#define DXCB_THIRD		4
#define DXCB_ZBUFFER	8
#define DXCB_RENDER		16
#define DXCB_PICTURE	32
#define DXCB_CLIENT		64
#define DXCB_CLRWINDOW	256

#define EndOfArray(a) (a + ArraySize(a))

#define RELEASEARRAY(x) for (int i = 0; i < ArraySize(x); ++i) if (x[i]) { x[i]->Release(); x[i] = 0;}

#define InitDXStruct(s) memset(&s, 0, sizeof(s)), s.dwSize = sizeof(s)
#define ZeroArray(a)	memset(a, 0, sizeof(a))

enum game_malloc_types
{
	TEMP_ALLOC,
	TEXTURE_PAGES,
	MESH_POINTERS,
	MESHES,
	ANIMS,
	STRUCTS,
	RANGES,
	COMMANDS,
	BONES,
	FRAMES,
	ROOM_TEXTURES,
	ROOM_INFOS,
	ROOM_MESH,
	ROOM_DOOR,
	ROOM_FLOOR,
	ROOM_LIGHTS,
	ROOM_STATIC_MESH_INFOS,
	OBJECTS,
	FLOOR_DATA,
	ITEMS,
	CAMERAS,
	SOUND_FX,
	BOXES,
	OVERLAPS,
	GROUNDZONE,
	FLYZONE,
	ANIMATING_TEXTURE_RANGES,
	CINEMATIC_FRAMES,
	LOADDEMO_BUFFER,
	SAVEDEMO_BUFFER,
	CINEMATIC_EFFECTS,
	MUMMY_HEAD_TURN,
	EXTRA_DOOR_STUFF,
	EFFECTS_ARRAY,
	CREATURE_DATA,
	CREATURE_LOT,
	SAMPLEINFOS,
	SAMPLES,
	SAMPLEOFFSETS,
	ROLLINGBALL_STUFF,
	SKIDOO_STUFF,
	LOAD_PICCY_BUFFER,
	FMV_BUFFERS,
	POLYGON_BUFFERS,
	ORDER_TABLES,
	CLUTS,
	TEXTURE_INFOS,
	SPRITE_INFOS,
	NUM_MALLOC_TYPES
};

enum InitResult
{
	INIT_OK,

	INIT_ERR_PreferredAdapterNotFound,
	INIT_ERR_CantCreateWindow,
	INIT_ERR_CantCreateDirectDraw,
	INIT_ERR_CantInitRenderer,
	INIT_ERR_CantCreateDirectInput,
	INIT_ERR_CantCreateKeyboardDevice,
	INIT_ERR_CantSetKBCooperativeLevel,
	INIT_ERR_CantSetKBDataFormat,
	INIT_ERR_CantAcquireKeyboard,
	INIT_ERR_CantSetDSCooperativeLevel,

	INIT_ERR_DD_SetExclusiveMode,
	INIT_ERR_DD_ClearExclusiveMode,
	INIT_ERR_SetDisplayMode,
	INIT_ERR_CreateScreenBuffers,
	INIT_ERR_GetBackBuffer,
	INIT_ERR_CreatePalette,
	INIT_ERR_SetPalette,
	INIT_ERR_CreatePrimarySurface,
	INIT_ERR_CreateBackBuffer,
	INIT_ERR_CreateClipper,
	INIT_ERR_SetClipperHWnd,
	INIT_ERR_SetClipper,
	INIT_ERR_CreateZBuffer,
	INIT_ERR_AttachZBuffer,
	INIT_ERR_CreateRenderBuffer,
	INIT_ERR_CreatePictureBuffer,
	INIT_ERR_D3D_Create,
	INIT_ERR_CreateDevice,
	INIT_ERR_CreateViewport,
	INIT_ERR_AddViewport,
	INIT_ERR_SetViewport2,
	INIT_ERR_SetCurrentViewport,

	INIT_ERR_ClearRenderBuffer,
	INIT_ERR_UpdateRenderInfo,
	INIT_ERR_GetThirdBuffer,

	INIT_ERR_GoFullScreen,
	INIT_ERR_GoWindowed,

	INIT_ERR_WrongBitDepth,
	INIT_ERR_GetPixelFormat,
	INIT_ERR_GetDisplayMode
};

inline RECT grc_dump_window;
inline bool game_closedown = false;
inline SETUP game_setup = { 320, 200, 25, 25, 50, 0, 0 };

inline uint8_t game_palette[256 * 3],
			 water_palette[256 * 3];

inline double screen_sizer = 1.0,
			  game_sizer = 1.0;

inline int16_t* anim_tex_ranges = nullptr;

inline D3DRENDERSTATETYPE LnAlphaEnableRenderState;

inline MESH_INFO* CurrentMesh;

inline PHD_VECTOR CamRot,
				  CamPos;

inline bool fullscreen_clear_needed = false;

bool DX_CheckForLostSurfaces();
void DX_DoFlipWait();
void DX_ClearBuffers(DWORD dwClrFlags, DWORD dwColour = 0);

HRESULT DD_LockSurface(IDirectDrawSurface3* pSurf, DDSURFACEDESC& rDDSD, DWORD dwFlags = DDLOCK_WAIT | DDLOCK_WRITEONLY);
HRESULT DD_UnlockSurface(IDirectDrawSurface3* pSurf, DDSURFACEDESC& rDDSD);
bool DX_UpdateFrame(RECT* prcWindow = 0);
bool DD_ClearSurface(IDirectDrawSurface3* pSurf, RECT* pRect = 0, DWORD dwColour = 0);

HRESULT DD_EnsureSurfaceAvailable(IDirectDrawSurface3* pSurf, IDirectDrawSurface3* pTopSurf = 0, bool tClearIfLost = false);
HRESULT DD_CreateSurface(DDSURFACEDESC& ddsd, IDirectDrawSurface3*& rpSurf);

void setup_screen_size(int new_sx = -1, int new_sy = -1);

int32_t GetRandomDraw();
void SeedRandomDraw(int32_t seed);
int32_t GetRandomControl();
void SeedRandomControl(int32_t seed);

void S_SeedRandom();

void S_ExitSystem();

inline bool DX_TRY(HRESULT hr)		{ return !SUCCEEDED(hr); }
inline bool DX_FAILED(HRESULT hr)	{ return FAILED(hr); }