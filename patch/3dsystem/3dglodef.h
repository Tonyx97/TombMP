#pragma once

#include <stdint.h>

#define BETTER_ROOM_SORT

#define WIBBLE_SIZE 32
#define MAX_WIBBLE  2
#define MAX_SHADE   0x300

#define DPQ_END		40480 * 3
#define DPQ_START	32288 * 3

#define	MAX_VERTICES	65536 * 12				// maximum vertices at a go
#define MAX_POLYGONS	4000 * 12       		// maximum number of polygons
#define MAX_MATRICES	128 * 12				// maximum matrices in stack
#define W2V_SHIFT 		14						// shift scale of view.frame to world.frame
#define	W2V_SCALE 		(1 << W2V_SHIFT)		// scale of view frame to world frame
#define	MAX_TEXTURES	8192			
#define MAX_SPRITES		512

#define SPRITE_REL				0x00000000
#define SPRITE_ABS				0x01000000
#define SPRITE_SEMITRANS		0x02000000
#define SPRITE_SCALE			0x04000000
#define SPRITE_SHADE			0x08000000
#define SPRITE_TINT				0x10000000
#define SPRITE_TRANS_HALF		0x00000000
#define SPRITE_TRANS_ADD		0x20000000
#define SPRITE_TRANS_SUB		0x40000000
#define SPRITE_TRANS_QUARTER	0x60000000
#define SPRITE_COLOUR(r,g,b)   ((r) | ((g) << 8) | ((b) << 16))

#define phd_PopMatrix()		phd_mxptr -= 12

#define IF_NOT_VIS(vn1, vn2, vn3) if ((int32_t)(((vn3->xs - vn2->xs) * (vn1->ys - vn2->ys)) - \
		  							  	       ((vn1->xs - vn2->xs) * (vn3->ys - vn2->ys))) < 0 )

#define BACK_DEPTH 1000000000.f

#define TRIA_DEPTH_MID(vn1, vn2, vn3)		zdepth = (vn1->zv + vn2->zv + vn3->zv) * 0.333333333333333333f;
#define QUAD_DEPTH_MID(vn1, vn2, vn3, vn4)	zdepth = (vn1->zv + vn2->zv + vn3->zv + vn4->zv) * 0.25f;

#define TRIA_DEPTH_FAR(vn1, vn2, vn3)		zdepth = vn1->zv;\
											if (vn2->zv > zdepth) zdepth = vn2->zv;\
											if (vn3->zv > zdepth) zdepth = vn3->zv;
#define QUAD_DEPTH_FAR(vn1, vn2, vn3, vn4)	zdepth = vn1->zv;\
											if (vn2->zv > zdepth) zdepth = vn2->zv;\
											if (vn3->zv > zdepth) zdepth = vn3->zv;\
											if (vn4->zv > zdepth) zdepth = vn4->zv;

#define TRIA_DEPTH_NEAR(vn1, vn2, vn3)		zdepth = vn1->zv;\
											if (vn2->zv < zdepth) zdepth = vn2->zv;\
											if (vn3->zv < zdepth) zdepth = vn3->zv;
#define QUAD_DEPTH_NEAR(vn1, vn2, vn3, vn4)	zdepth = (int32_t)vn1->zv;\
											if (vn2->zv < zdepth) zdepth = (int32_t)vn2->zv;\
											if (vn3->zv < zdepth) zdepth = (int32_t)vn3->zv;\
											if (vn4->zv < zdepth) zdepth = (int32_t)vn4->zv;

#define ABS(x)	 (((x) < 0) ? (-(x)) : (x))
#define MIN(x,y) ((x)<=(y) ? (x):(y))
#define MAX(x,y) ((x)>=(y) ? (x):(y))

#define	TRIGMULT2(A, B)		(((A) * (B)) >> W2V_SHIFT)
#define	TRIGMULT3(A, B, C)	(TRIGMULT2(TRIGMULT2(A, B), C))

// enums

enum sort_type { MID_SORT, FAR_SORT, BACK_SORT };

enum msoff
{
	M00, M01, M02, M03,
	M10, M11, M12, M13,
	M20, M21, M22, M23
};

enum
{
	DRAW_POLY_GTMAP,
	DRAW_POLY_WGTMAP,
	DRAW_POLY_GTMAP_PERSPECTIVE,
	DRAW_POLY_WGTMAP_PERSPECTIVE,
	DRAW_POLY_LINE,
	DRAW_POLY_FLAT,
	DRAW_POLY_GOURAUD,
	DRAW_POLY_TRANS,
	DRAW_SCALED_SPRITE,

	DRAW_TLV_GT,
	DRAW_TLV_WGT,
	DRAW_TLV_G,
	DRAW_TLV_L,
	DRAW_TLV_GA,
	DRAW_TLV_GTA,
	DRAW_TLV_GTT,
};

/* main structures */

using PHD_ANGLE = int16_t;

struct PHD_VECTOR
{
	int32_t x = 0,
		    y = 0,
		    z = 0;
};

struct PHD_ANGLE_VEC
{
	PHD_ANGLE x = 0,
			  y = 0,
			  z = 0;
};

struct PHD_3DPOS
{
	int32_t x_pos, y_pos, z_pos;
	PHD_ANGLE x_rot, y_rot, z_rot;
};

struct PHDTEXTURESTRUCT
{
	uint16_t drawtype,
		     tpage,
		     u1, v1,
		     u2, v2,
		     u3, v3,
		     u4, v4;
};

struct PHDSPRITESTRUCT
{
	uint16_t tpage,
		   offset,
		   width,
		   height,
		   x1, y1,
		   x2, y2;
};

struct PHD_VBUF
{
	float xv, yv, zv,
		  ooz,
		  xs, ys;
	
	int32_t z;

	int8_t clip, fog;

	int16_t g;
	
	uint16_t u, v;

	int dynamic;
};

struct XBUF_X
{
	int32_t Xleft, Xright;
};

struct XBUF_XG
{
	int32_t Xleft, Gleft,
		   Xright, Gright;
};

struct XBUF_XGUV
{
	int32_t Xleft, Gleft, Uleft, Vleft,
		   Xright, Gright, Uright, Vright;
};

struct XBUF_PERSP
{
	int32_t Xleft, Gleft;
	int	UOZleft, VOZleft, OOZleft;
	int32_t Xright, Gright;
	int	UOZright, VOZright, OOZright;
};

struct XBUF_PERSP_FP
{
	int32_t Xleft, Gleft;
	float UOZleft, VOZleft, OOZleft;
	int32_t Xright, Gright;
	float UOZright, VOZright, OOZright;
};

struct VERTEX_INFO
{
	float x, y,
		  ooz,
		  u, v, g;

	int vr, vg, vb;
};

struct POINT_INFO
{
	float xv, yv, zv,
		  ooz,
		  xs, ys,
		  u, v,
		  g;

	int vr, vg, vb;
};

inline VERTEX_INFO v_buffer[40];

/* general variables */

inline float wibble_table[WIBBLE_SIZE];
inline int32_t rand_table[WIBBLE_SIZE];
inline int16_t shade_table[WIBBLE_SIZE];

// water variables
inline int32_t wibble_offset = 0,
			   water_effect = 0,
			   mid_sort;

inline float phd_leftfloat;  	// floating point copies of phd_left etc
inline float phd_rightfloat;
inline float phd_topfloat;
inline float phd_bottomfloat;
inline float g_no_depth;

inline int32_t phd_scrwidth;						// width of screen in pixels
inline char* phd_winptr;						// pointer to 3dwindow
inline int16_t phd_winxmax;						// max x coord of window
inline int32_t phd_winwidth;						// width of window
inline int32_t phd_winheight;					// height of window
inline PHDSPRITESTRUCT phdsprinfo[MAX_SPRITES];
inline char gouraud_table[256][32];
inline char depthq_table[33][256];

inline int32_t phd_left;				// left clipping value
inline int32_t phd_right;			// right clipping value
inline int32_t phd_top;				// top clipping value
inline int32_t phd_bottom;			// bottom clipping value
inline int16_t phd_winxmin;
inline int16_t phd_winymin;
inline int16_t phd_winymax;			// max y coord of window
inline int32_t phd_centerx;			// xcenter of window
inline int32_t phd_centery;			// ycenter of window
inline int32_t phd_znear;			// minimum z coord in view frame
inline int32_t phd_zfar;				// maximum z coord in view frame
inline int32_t phd_viewdist;			// maximum View distance in world frame
inline int32_t phd_scrheight;		// height of screen in pixels
inline int32_t phd_persp;			// perspective scale factor
inline int32_t phd_oopersp;

inline float one = (256.f * 8.f * 16384.f);		// in 3dinsert.cpp
inline float f_znear;							// phd_znear
inline float f_zfar;							// phd_zfar
inline float f_oneoznear;						// one / phd_znear
inline float f_persp;							// phd_persp
inline float f_oneopersp;						// one / phd_persp
inline float f_perspoznear;						// phd_persp / phd_znear
inline float f_centerx;							// phd_centrex
inline float f_centery;							// phd_centrey
inline float f_oneozfar;						// one / phd_zfar
inline float f_a, f_b, f_boo;					// a / b factors for z buffering (f_boo is f_b / one)
inline float f_gamma = 4.f;

inline int32_t w2v_matrix[12];					// world to view matrix
inline int32_t* matrix_stack = nullptr;			// matrix stack for animations etc

inline PHD_VECTOR ls_vector_view;			// light vector in view frame

inline int32_t surfacenumbf;
inline int32_t* sort3dptrbf;		// back to front sort ptr
inline int16_t* info3dptrbf;

//inline int32_t** sort3d_bufferbf = nullptr;
inline int32_t sort3d_bufferbf[MAX_POLYGONS][10];
inline int16_t* info3d_bufferbf = nullptr;

inline PHD_VBUF* vbuf = nullptr;

inline PHDTEXTURESTRUCT* phdtextinfo = nullptr;

inline int32_t* phd_mxptr = nullptr;		// pointer to current matrix

/*** 3d_gen.cpp ***/
void phd_InitBuffers();
void phd_DestroyBuffers();
void phd_GenerateW2V(const PHD_3DPOS& viewpos);
void phd_LookAt(const PHD_VECTOR& src, const PHD_VECTOR& tar, PHD_ANGLE roll);
PHD_ANGLE_VEC phd_GetVectorAngles(const PHD_VECTOR& src);
void phd_NormaliseVector(int32_t x, int32_t y, int32_t z, int32_t* dest);
void phd_NormaliseMatrix();
PHD_ANGLE_VEC phd_GetMatrixAngles();
PHD_VECTOR phd_GetMatrixPosition();
void phd_RotX(PHD_ANGLE rx);
void phd_RotY(PHD_ANGLE ry);
void phd_RotZ(PHD_ANGLE rz);
void phd_RotYXZ(PHD_ANGLE ry, PHD_ANGLE rx, PHD_ANGLE rz);
void phd_RotYXZpack(int32_t rots);
bool phd_TranslateRel(int32_t x, int32_t y, int32_t z);
void phd_TranslateAbs(int32_t x, int32_t y, int32_t z);
void phd_PutPolygons(int16_t* objptr, int clipstatus);
int16_t* calc_vertice_light(int16_t* objptr, int16_t*);
int16_t* calc_object_vertices(int16_t* objptr);
int16_t* calc_roomvert(int16_t* objptr, int far_clip);
void phd_RotateLight(PHD_ANGLE pitch, PHD_ANGLE yaw);
void phd_PointLight(PHD_3DPOS* destpos, PHD_3DPOS* lightpos);
void phd_InitPolyList();
//void phd_SortPolyList(int, int32_t** buffer);
void phd_SortPolyList(int, int32_t buffer[][10]);
//void do_quickysorty(int left, int right, int32_t** buffer);
void do_quickysorty(int left, int right, int32_t buffer[][10]);
void phd_InitWindow(int x, int y, int width, int height, int nearz, int farz, int scrwidth, int scrheight);
void AlterFOV(PHD_ANGLE	fov);

/*** math_funcs.cpp ***/
void phd_PushMatrix();
void phd_PushUnitMatrix();
void phd_UnitMatrix();
int32_t phd_cos(int32_t angle);
int32_t phd_sin(int32_t angle);
uint32_t phd_sqrt(uint32_t n);
int16_t phd_atan(int32_t x, int32_t y);

/*** 3dinsert.cpp ***/
bool visible_zclip(PHD_VBUF* vn1, PHD_VBUF* vn2, PHD_VBUF* vn3);
int XYGClipper(int number, VERTEX_INFO* input);
int XYClipper(int number, VERTEX_INFO* input);

/*** hwinsert.cpp ***/
inline bool g_blue_effect = false;
inline uint8_t G_GouraudPalette[256 * 4];

/*** scalespr.cpp ***/
void S_DrawSprite(uint32_t dwFlags, int32_t nX, int32_t nY, int32_t nZ, int16_t* nSprite, int16_t nShade, int16_t nScale);
void S_DrawScreenSprite2d(int32_t sx, int32_t sy, int32_t z, int32_t scaleH, int32_t scaleV, int16_t* sprnum, int16_t shade, uint16_t flags);
void S_DrawScreenSprite(int32_t sx, int32_t sy, int32_t z, int32_t scaleH, int32_t scaleV, int16_t sprnum, int16_t shade, uint16_t flags);
int16_t* ins_room_sprite(int16_t* objptr, int num);