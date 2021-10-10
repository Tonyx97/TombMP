#pragma once

#define BLADE_DAMAGE			100
#define STATUE_DAMAGE			20
#define SPIKE_DAMAGE			15
#define TEETHTRAP_DAMAGE 		400
#define FALLING_CEILING_DAMAGE 	300
#define ROLLINGBALL_DAMAGE_AIR	100
#define ICICLE_DAMAGE			200
#define SPINBLADE_DAMAGE		100
#define HOOK_DAMAGE				50
#define PROP_DAMAGE				200
#define SPIKEWALL_DAMAGE		20
#define DART_SPEED				256
#define FLAME_TOONEAR_DAMAGE	5
#define FLAME_ONFIRE_DAMAGE		7
#define LAVA_GLOB_DAMAGE		10
#define WEDGE_SPEED				25
#define STATUE_TOUCH			0x80
#define SPIKE_SPEED				1
#define BOARD_OFF				0
#define BOARD_ON				1
#define ROLL_SHAKE_RANGE		(WALL_L * 10)
#define	FIRE_WIDTH				256

enum
{
	BIG_FIRE = 0,
	SMALL_FIRE,
	JET_FIRE,
	SIDE_FIRE
};

enum statue_anims
{
	STATUE_EMPTY,
	STATUE_STOP,
	STATUE_CUT
};

enum spin_anims
{
	SPIN_EMPTY,
	SPIN_STOP,
	SPIN_ROLL
};

enum prop_anims
{
	PROP_ON,
	PROP_OFF
};

enum icicle_anims
{
	ICICLE_EMPTY,
	ICICLE_BREAK,
	ICICLE_FALL,
	ICICLE_LAND
};

enum tt_states
{
	TT_NICE,
	TT_NASTY
};

enum trap_anims
{
	TRAP_SET,
	TRAP_ACTIVATE,
	TRAP_WORKING,
	TRAP_FINISHED
};

enum door_anims
{
	DOOR_CLOSED,
	DOOR_OPEN
};

void EarthQuake(int16_t item_number);
void ControlCutShotgun(int16_t item_number);

void MiniCopterControl(int16_t item_number);
void CopterControl(int16_t item_number);

void ControlLaraAlarm(int16_t item_number);
void ControlDingDong(int16_t item_number);
void ControlClockChimes(int16_t item_number);
void ControlBirdTweeter(int16_t item_number);

void DeathSlideCollision(int16_t item_number, ITEM_INFO* litem, COLL_INFO* coll);
void ControlDeathSlide(int16_t item_number);

void SpikeControl(int16_t item_number);
void ControlSpikeWall(int16_t item_number);
void ControlCeilingSpikes(int16_t item_number);

void HookControl(int16_t item_number);
void PropellerControl(int16_t item_number);

void SpinningBlade(int16_t item_number);

void IcicleControl(int16_t item_number);

void InitialiseBlade(int16_t item_number);
void BladeControl(int16_t item_number);
void InitialiseKillerStatue(int16_t item_number);
void SpringBoardControl(int16_t item_number);

void InitialiseWindow(int16_t item_number);
void SmashWindow(int16_t item_number);
void WindowControl(int16_t item_number);

void InitialiseRollingBall(int16_t item_number);
void RollingBallControl(int16_t item_number);
void RollingBallCollision(int16_t item_num, ITEM_INFO* litem, COLL_INFO* coll);

void SpikeCollision(int16_t item_num, ITEM_INFO* litem, COLL_INFO* coll);

void InitialiseDoor(int16_t item_number);
void DoorControl(int16_t item_number);

void TrapDoorControl(int16_t item_number);
void TrapDoorFloor(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height);
void TrapDoorCeiling(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height);
int OnTrapDoor(ITEM_INFO* item, int32_t x, int32_t y);

void FallingBlock(int16_t item_number);
void FallingBlockFloor(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height);
void FallingBlockCeiling(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height);

void DrawBridgeFloor(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height);
void DrawBridgeCeiling(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height);
void DrawBridgeCollision(int16_t item_num, ITEM_INFO* laraitem, COLL_INFO* coll);

void Pendulum(int16_t item_number);

void InitialiseLift(int16_t item_number);
void LiftControl(int16_t item_number);
void LiftFloor(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height);
void LiftCeiling(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height);

void BridgeFlatFloor(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height);
void BridgeFlatCeiling(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height);
void BridgeTilt1Floor(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height);
void BridgeTilt1Ceiling(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height);
void BridgeTilt2Floor(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height);
void BridgeTilt2Ceiling(ITEM_INFO* item, int32_t x, int32_t y, int32_t z, int32_t* height);

void GeneralControl(int16_t item_number);
void DetonatorControl(int16_t item_number);
void TeethTrap(int16_t item_number);
void FallingCeiling(int16_t item_number);

void DartEmitterControl(int16_t item_num);
void DartsControl(int16_t item_num);

void SideFlameEmitterControl(int16_t item_number);
void FlameEmitterControl(int16_t item_number);
void FlameEmitter2Control(int16_t item_number);
void FlameEmitter3Control(int16_t item_number);
void FlameControl(int16_t fx_number);
void LaraBurn();
void draw_lara_fire(ITEM_INFO* item, int r, int g, int b);

void LavaBurn(ITEM_INFO* item);

void WaterFall(int16_t item_number);

int ItemNearLara(ITEM_INFO* item, PHD_3DPOS* pos, int32_t distance);

void FlareControl(int16_t item_number);

void PoleCollision(int16_t item_num, ITEM_INFO* l, COLL_INFO* coll);