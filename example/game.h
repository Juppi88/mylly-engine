#pragma once

#include "gamedefs.h"
#include <mylly/math/vector.h>

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

private:
	scene_t *m_gameScene = nullptr;
	camera_t *m_camera = nullptr;

	InputHandler *m_input = nullptr;
	Ship *m_ship = nullptr;

	vec2_t m_boundsMin = vec2_zero();
	vec2_t m_boundsMax = vec2_zero();
};
