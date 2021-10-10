#pragma once

void move_cb(int x, int y);
void resize_cb(int width, int height);
void framebuffer_cb(int width, int height);
void kb_cb(int scancode, int action, int mods);
void chars_cb(unsigned int codepoint, int mods);
void cursor_pos_cb(double xpos, double ypos);
void focus_cb(int focus);
void mouse_btn_cb(int button, int action, int mods);
void scroll_cb(double yoffset);