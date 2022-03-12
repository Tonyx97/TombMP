#pragma once

namespace events
{
	namespace engine
	{
		static inline std::string ON_TICK = "onTick";
		static inline std::string ON_CONTROL_PHASE = "onControlPhase";
	}

	namespace level
	{
		static inline std::string ON_LEVEL_LOAD = "onLevelLoaded";
		static inline std::string ON_LEVEL_UNLOAD = "onLevelUnloaded";
		static inline std::string ON_LEVEL_FINISH = "onLevelFinish";
	}

	namespace physics
	{
		static inline std::string ON_PLAYER_ITEM_COLLIDE = "onPlayerItemCollide";
	}

	namespace entity
	{
		static inline std::string ON_ENTITY_DAMAGE = "onEntityDamage";
	}

	namespace renderer
	{
		static inline std::string ON_SCENE_BEGIN = "onSceneBegin";
		static inline std::string ON_SCENE_END = "onSceneEnd";
		static inline std::string ON_PLAYER_RENDER = "onPlayerRender";
	}

	namespace ui
	{
		static inline std::string ON_UI = "onUI";
	}

	namespace audio
	{
		static inline std::string ON_AUDIO_PLAY = "onAudioPlay";
	}

	namespace pickup
	{
		static inline std::string ON_SAVECRYSTAL_PICKUP = "onSavecrystalPickup";
		static inline std::string ON_PICKUP_COLLISION = "onPickupCollision";
	}

	namespace vehicle
	{
		static inline std::string ON_VEHICLE_ENTER = "onVehicleEnter";
	}
}