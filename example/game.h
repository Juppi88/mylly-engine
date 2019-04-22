#pragma once

#include "gamedefs.h"
#include <mylly/math/vector.h>
#include <mylly/renderer/colour.h>

// -------------------------------------------------------------------------------------------------

class Game
{
public:
	Game(void);
	~Game(void);

	void SetupGame(void);
	void Shutdown(void);

	bool IsSetup(void) const { return (m_gameScene != nullptr); }

	void Process(void);

private:
	void CreateCamera(void);
	void CreateSpaceBackground(void);

	void SetupLighting(void);

	void EnforceBoundaries(void);
	bool IsWithinBoundaries(const vec2_t &position);
	vec2_t WrapBoundaries(const vec2_t &position);

private:
	static constexpr colour_t AMBIENT_LIGHT_COLOUR = col(80, 120, 175);
	static constexpr colour_t DIRECTIONAL_LIGHT_COLOUR = col(200, 220, 255);

	InputHandler *m_input = nullptr;
	Ship *m_ship = nullptr;

	scene_t *m_gameScene = nullptr;
	camera_t *m_camera = nullptr;
	object_t *m_spaceBackground = nullptr;
	light_t *m_directionalLights[2] = { nullptr, nullptr };

	vec2_t m_boundsMin = vec2_zero();
	vec2_t m_boundsMax = vec2_zero();
};
