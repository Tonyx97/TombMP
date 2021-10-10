#pragma once

#define MAX_FLARE_AGE		(30 * 30)
#define FLARE_YOUNG			(16)
#define FLARE_OLD			(MAX_FLARE_AGE - (3 * 30))
#define FLARE_DEAD			(MAX_FLARE_AGE - 24)
#define	FLARE_BUBBLESIZE	8
#define	FLARE_BUBBLERANGE	8

#define FL_HOLD_FT		1
#define FL_THROW_FT		32
#define FL_DRAW_FT		39
#define FL_IGNITE_FT	23
#define FL_2HOLD_FT		15

#define FL_HOLD_F		0
#define FL_THROW_F		(FL_HOLD_F+FL_HOLD_FT)
#define FL_DRAW_F		(FL_THROW_F+FL_THROW_FT)
#define FL_IGNITE_F		(FL_DRAW_F+FL_DRAW_FT)
#define FL_2HOLD_F		(FL_IGNITE_F+FL_IGNITE_FT)
#define FL_END_F		(FL_2HOLD_F+FL_2HOLD_FT)

#define FL_DRAWGOTIT_F		(13 + FL_DRAW_F)
#define FL_PICKUP_F			(58 + FL_PICKINGUP_F)
#define FL_THROWRELEASE_F (20 + FL_THROW_F)

#define FL_THROW_FRAMES	32

int do_flare_light(PHD_VECTOR* pos, int flare_age);
void do_flare_in_hand(int flare_age);
void DrawFlareInAir(ITEM_INFO* item);
void create_flare(int thrown);
void set_flare_arm(int frame);
void draw_flare();
void undraw_flare();
void draw_flare_meshes();
void undraw_flare_meshes();
void ready_flare();
void FlareControl(int16_t item_number);