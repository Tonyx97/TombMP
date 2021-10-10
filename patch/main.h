#include <specific/directx.h>

float WinFrameRate();

struct WINAPP
{
	HWND WindowHandle;
	HINSTANCE hInstance;
		
	DEVICEINFO DeviceInfo;					// Device Information
	DEVICEINFO* DeviceInfoPtr;

	LPDIRECTDRAW2 lpDD;						// DirectDraw2 Interface
	LPDIRECT3D2 lpD3D;						// Direct3D2 Interface
	LPDIRECT3DDEVICE2 lpD3DDevice;			// Direct 3D2 Device

	LPDIRECTDRAWSURFACE3 lpFrontBuffer;		// Primary Surface
	LPDIRECTDRAWSURFACE3 lpBackBuffer;		// Back Buffer
	LPDIRECTDRAWSURFACE3 lpZBuffer;			// Z Buffer
	LPDIRECTDRAWSURFACE3 lpPictureBuffer;	// Picture Buffer
	
	LPDIRECT3DVIEWPORT2 lpViewPort;

	int nUVAdd;
	
	float fps;
};

inline WINAPP App;