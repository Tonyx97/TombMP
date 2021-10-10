#pragma once

#pragma comment(lib, "dxgi")
#pragma comment(lib, "d2d1")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "dcomp")
#pragma comment(lib, "dwrite")

#include <stdint.h>
#include <tuple>

#include <d3d11_2.h>
#include <d2d1_3helper.h>
#include <dwrite_3.h>
#include <dcomp.h>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

inline float f32_saturate(float f) { return (f < 0.f) ? 0.f : (f > 1.f) ? 1.f : f; }

#define F32_TO_INT8_SAT(x) ((int)(f32_saturate(x) * 255.f + 0.5f))

class d2d1_ui
{
private:

	struct download_progress_bar
	{
		float progress = 0.f,
			  target = 0.f,
			  width = 400.f,
			  height = 20.f;

		bool enabled = false;
	} download_bar {};

	ID3D11Device* device = nullptr;
	IDXGIDevice* dxgi_device = nullptr;
	IDXGIFactory2* dxgi_factory = nullptr;
	IDXGISwapChain1* swapchain = nullptr;
	IDXGISurface2* surface = nullptr;
	ID2D1Bitmap1* bitmap = nullptr;
	ID2D1Factory2* factory = nullptr;
	ID2D1Device1* d2_device = nullptr;
	ID2D1DeviceContext1* d2_device_ctx = nullptr;
	ID2D1SolidColorBrush* d2_solid_brush = nullptr;
	IDWriteFactory* write_factory = nullptr;
	IDWriteTextFormat* write_text_format = nullptr;
	IDCompositionDevice* comp_device = nullptr;
	IDCompositionTarget* comp_target = nullptr;
	IDCompositionVisual* comp_visual = nullptr;

	ImFont* default_font = nullptr;

	HWND target_hwnd = nullptr;

	ImGuiIO* io = nullptr;

	bool initialized = false;

	template <typename T>
	uint32_t to_color(const T& in)
	{
		uint32_t out = ((uint32_t)F32_TO_INT8_SAT(in.x)) << 0;
		out |= ((uint32_t)F32_TO_INT8_SAT(in.y)) << 8;
		out |= ((uint32_t)F32_TO_INT8_SAT(in.z)) << 16;
		out |= ((uint32_t)F32_TO_INT8_SAT(in.w)) << 24;
		return out;
	}

	void destroy();

public:

	d2d1_ui()									{}
	~d2d1_ui()									{ destroy(); }

	bool build_swapchain();
	bool build_composition();

	bool init();

	void begin();
	void update();
	void end();

	void begin_font(ImFont* font);
	void end_font();

	void set_download_bar_progress(float v)		{ download_bar.progress = v; }
	void begin_download_bar(float target);
	void render_download_bar();
	void end_download_bar();

	void begin_window(const char* name, const ImVec2& pos, const ImVec2& size, const ImVec4& color);
	void end_window();

	ImVec2 calc_text_size(const char* text, float size, float wrap);
	ImVec2 get_screen_size()					{ return io->DisplaySize; }

	void draw_rect(float x, float y, float w, float h, float t, const D2D1_COLOR_F& color);
	void draw_filled_rect(float x, float y, float w, float h, const D2D1_COLOR_F& color);
	void draw_line(float x, float y, float x1, float y1, float thickness, const D2D1_COLOR_F& color);
	void draw_circle(float x, float y, float radius, float thickness, const D2D1_COLOR_F& color);
	void draw_filled_circle(float x, float y, float radius, const D2D1_COLOR_F& color);

	float add_text(const char* text, float x, float y, float s, const D2D1_COLOR_F& color, bool center, int shadow = -1, float wrap = 0.f);
	float add_text(const wchar_t* text, float x, float y, float s, const D2D1_COLOR_F& color, bool center, int shadow = -1, float wrap = 0.f);
};

inline std::unique_ptr<d2d1_ui> g_ui;