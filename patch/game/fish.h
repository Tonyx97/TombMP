#pragma once

#define	MAX_FISH	8

struct FISH_INFO
{
	short x;
	short y;
	short z;
	uint16_t angle;
	short desty;
	signed char	angadd;
	unsigned char speed;
	unsigned char acc;
	unsigned char swim;
};

struct LEADER_INFO
{
	short angle;
	unsigned char speed;
	unsigned char on;
	short angle_time;
	short speed_time;
	short Xrange, Zrange, Yrange;
};

inline FISH_INFO fish[MAX_FISH + (MAX_FISH * 24)];
inline LEADER_INFO lead_info[MAX_FISH];

void S_DrawFish(ITEM_INFO* item);
void SetupShoal(long shoal_number);
void ControlFish(short item_number);