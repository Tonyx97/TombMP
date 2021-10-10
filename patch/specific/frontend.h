#pragma once

void S_DrawScreenLine(int32_t sx, int32_t sy, int32_t z, int32_t w, int32_t h, int32_t col, uint16_t* grdptr, uint16_t flags);
void S_DrawLine(int32_t x1, int32_t y1, int32_t z, int32_t x2, int32_t y2, int32_t col, uint16_t* grdptr, uint16_t flags);
void S_DrawScreenBox(int32_t sx, int32_t sy, int32_t z, int32_t w, int32_t h, int32_t col, uint16_t* grdptr, uint16_t flags);
void S_DrawScreenFBox(int32_t sx, int32_t sy, int32_t z, int32_t w, int32_t h, int32_t col, uint16_t* grdptr, uint16_t flags);
void S_FadeToBlack();
SG_COL S_COLOUR(int R, int G, int B);