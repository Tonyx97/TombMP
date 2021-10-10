#pragma once

bool init_imgui_renderer(struct IDXGISwapChain* sc, const char* font, float sx = 0.f, float sy = 0.f);
void imgui_render();
void imgui_destroy();