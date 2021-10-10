#include <shared/defs.h>
#include <shared/scripting/resource.h>
#include <shared/scripting/resource_system.h>

#include <scripting/events.h>

#include <window/window.h>

#include <specific/standard.h>
#include <specific/global.h>
#include <specific/directx.h>

#include <mp/client.h>

#include <main.h>

#include "ui.h"

#include <imgui/imgui_impl_dx11.h>

bool d2d1_ui::build_swapchain()
{
	target_hwnd = g_window->get_win32_handle();

	if (FAILED(D3D11CreateDevice(nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		D3D11_CREATE_DEVICE_BGRA_SUPPORT,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&device,
		nullptr,
		nullptr))) return false;

	device->QueryInterface(IID_PPV_ARGS(&dxgi_device));

	if (FAILED(CreateDXGIFactory2(0, __uuidof(dxgi_factory), reinterpret_cast<void**>(&dxgi_factory))))
		return false;

	DXGI_SWAP_CHAIN_DESC1 description{};
	description.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	description.Stereo = false;
	description.BufferCount = 2;
	description.SampleDesc.Count = 1;
	description.SampleDesc.Quality = 0;
	description.Flags = 0;
	description.Scaling = DXGI_SCALING_STRETCH;
	description.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
	std::tie(description.Width, description.Height) = g_window->get_resolution();

	if (FAILED(dxgi_factory->CreateSwapChainForComposition(device, &description, nullptr, &swapchain)))
		return false;

	return init_imgui_renderer(swapchain, "inconsolata.ttf", float(description.Width), float(description.Height));
}

bool d2d1_ui::build_composition()
{
	if (FAILED(DCompositionCreateDevice(dxgi_device, __uuidof(comp_device), reinterpret_cast<void**>(&comp_device))) ||
		FAILED(comp_device->CreateTargetForHwnd(target_hwnd, true, &comp_target)) ||
		FAILED(comp_device->CreateVisual(&comp_visual)) ||
		FAILED(comp_visual->SetContent(swapchain)) ||
		FAILED(comp_target->SetRoot(comp_visual)) ||
		FAILED(comp_device->Commit()))
		return false;

	return SUCCEEDED(comp_device->WaitForCommitCompletion());
}

bool d2d1_ui::init()
{
	if (!build_swapchain())
		return false;

	if (!build_composition())
		return false;

	io = &ImGui::GetIO();

	return (initialized = true);
}

void d2d1_ui::destroy()
{
	if (!initialized)
		return;

	comp_visual->SetContent(nullptr);
	comp_target->SetRoot(nullptr);
	comp_device->Release();
	comp_visual->Release();
	comp_target->Release();

	imgui_destroy();

	swapchain->Release();
	device->Release();
}

void d2d1_ui::begin()
{
	ImGui::NewFrame();
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.f, 0.f, 0.f, 0.f));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.f, 0.f, 0.f, 0.f));
	ImGui::Begin("##raw_ui", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs);
	ImGui::SetWindowPos({ 0, 0 }, ImGuiCond_Always);
	ImGui::SetWindowSize(io->DisplaySize, ImGuiCond_Always);
}

void d2d1_ui::end()
{
	render_download_bar();

	if (auto netstat_level = g_client->get_net_stat(); netstat_level != -1)
	{
		const auto& netstat = g_client->get_net_stat_str();

		auto [sx, sy] = get_screen_size();

		float height = 65.f;

		switch (netstat_level)
		{
		case 1: height = 160.f; break;
		case 2: height = 350.f; break;
		}

		draw_filled_rect(sx - 360.f, 10.f, 370.f, height, { 0.f, 0.f, 0.f, 0.5f });
		add_text(netstat.c_str(), sx - 350.f, 15.f, 16.f, { 1.f, 1.f, 1.f, 1.f }, false);
	}

	ImGui::GetWindowDrawList()->PushClipRectFullScreen();
	ImGui::End();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::Render();

	imgui_render();

	swapchain->Present(0, 0);
}

void d2d1_ui::begin_font(ImFont* font)
{
	ImGui::PushFont(font);
}

void d2d1_ui::end_font()
{
	ImGui::PopFont();
}

void d2d1_ui::begin_download_bar(float target)
{
	download_bar.enabled = true;
	download_bar.target = target;
	download_bar.progress = 0.f;
}

void d2d1_ui::render_download_bar()
{
	if (!download_bar.enabled)
		return;

	auto [sx, sy] = io->DisplaySize;

	const float progress = download_bar.progress / download_bar.target;

	const auto bar_pos = ImVec2(sx / 2.f - download_bar.width / 2.f, sy - 100.f);

	draw_filled_rect(bar_pos.x, bar_pos.y, download_bar.width, download_bar.height, { 0.f, 0.f, 0.f, 0.5f });
	draw_filled_rect(bar_pos.x + 2.f, bar_pos.y + 2.f, progress * (download_bar.width - 4.f), download_bar.height - 4.f, { 0.f, 0.62f, 1.f, 1.f });

	add_text(std::format("Downloading... {:.0f} % ({:.1f} / {:.1f} MB)",
		progress * 100.f,
		download_bar.progress / (1024.f * 1024.f),
		download_bar.target / (1024.f * 1024.f)).c_str(), sx / 2.f, bar_pos.y + 9.f, 16.f, { 1.f, 1.f, 1.f, 1.f }, true, 0);
}

void d2d1_ui::end_download_bar()
{
	download_bar.enabled = false;
}

void d2d1_ui::update()
{
	g_resource->trigger_event(events::ui::ON_UI);
}

ImVec2 d2d1_ui::calc_text_size(const char* text, float size, float wrap)
{
	return ImGui::GetWindowDrawList()->_Data->Font->CalcTextSizeA(size, FLT_MAX, wrap, text);
}

void d2d1_ui::begin_window(const char* name, const ImVec2& pos, const ImVec2& size, const ImVec4& color)
{
	ImGui::PushStyleColor(ImGuiCol_WindowBg, color);

	ImGui::Begin(name, nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoScrollbar);
	ImGui::SetWindowPos(pos, ImGuiCond_Always);
	ImGui::SetWindowSize(size, ImGuiCond_Always);

	ImGui::PopStyleColor();
}

void d2d1_ui::end_window()
{
	ImGui::End();
}

void d2d1_ui::draw_rect(float x, float y, float w, float h, float t, const D2D1_COLOR_F& color)
{
	ImGui::GetWindowDrawList()->AddQuad({ x, y }, { x, y + h }, { x + w, y + h }, { x + w, y }, to_color(ImVec4(color.r, color.g, color.b, color.a)), t);
}

void d2d1_ui::draw_filled_rect(float x, float y, float w, float h, const D2D1_COLOR_F& color)
{
	ImGui::GetWindowDrawList()->AddQuadFilled({ x, y }, { x, y + h }, { x + w, y + h }, { x + w, y }, to_color(ImVec4(color.r, color.g, color.b, color.a)));
}

void d2d1_ui::draw_line(float x, float y, float x1, float y1, float thickness, const D2D1_COLOR_F& color)
{
	ImGui::GetWindowDrawList()->AddLine({ x, y }, { x1, y1 }, to_color(ImVec4(color.r, color.g, color.b, color.a)), thickness);
}

void d2d1_ui::draw_circle(float x, float y, float radius, float thickness, const D2D1_COLOR_F& color)
{
	ImGui::GetWindowDrawList()->AddCircle({ x, y }, radius, to_color(ImVec4(color.r, color.g, color.b, color.a)), 25, thickness);
}

void d2d1_ui::draw_filled_circle(float x, float y, float radius, const D2D1_COLOR_F& color)
{
	ImGui::GetWindowDrawList()->AddCircleFilled({ x, y }, radius, to_color(ImVec4(color.r, color.g, color.b, color.a)), 25);
}

float d2d1_ui::add_text(const char* text, float x, float y, float s, const D2D1_COLOR_F& color, bool center, int shadow, float wrap)
{
	auto dl = ImGui::GetWindowDrawList();

	const auto size = calc_text_size(text, s, wrap);

	const auto outline_color = to_color(ImVec4(0.f, 0.f, 0.f, 1.f)),
			   text_color = to_color(ImVec4(color.r, color.g, color.b, color.a));

	const auto position = (center ? ImVec2 { x - size.x / 2.f, y - size.y / 2.f } : ImVec2 { x, y });

	if (shadow != -1)
	{
		float shadow_cos = std::cosf(float(shadow)),
			  shadow_sin = std::sinf(float(shadow));

		dl->AddText(nullptr, s, ImVec2(position.x + shadow_cos, position.y + shadow_sin), outline_color, text, 0, wrap);
	}

	dl->AddText(nullptr, s, ImVec2(position.x, position.y), text_color, text, 0, wrap);

	return y + size.y;
}

float d2d1_ui::add_text(const wchar_t* text, float x, float y, float s, const D2D1_COLOR_F& color, bool center, int shadow, float wrap)
{
	auto str_length = wcslen(text) * 2;
	auto utf8_text = std::unique_ptr<char, std::function<void(char*)>>(new char[str_length](), [](char* data) { delete[] data; });

	if (WideCharToMultiByte(CP_UTF8, 0, text, -1, utf8_text.get(), str_length, nullptr, nullptr) == 0)
		return 0.f;

	return add_text(utf8_text.get(), x, y, s, color, center, shadow, wrap);
}