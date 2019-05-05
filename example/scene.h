#pragma once

#include "gamedefs.h"
#include "vector.h"
#include <mylly/renderer/colour.h>
#include <mylly/mgui/widget.h>

// -------------------------------------------------------------------------------------------------

class Scene
{
public:
	Scene(void);
	virtual ~Scene(void);

	virtual void Create(Game *game);
	virtual void SetupLevel(Game *game) = 0;
	virtual void Update(Game *game) = 0;

	scene_t *GetSceneRoot(void) const { return m_sceneRoot; }
	AsteroidHandler *GetAsteroidHandler(void) const { return m_asteroids; }
	ProjectileHandler *GetProjectileHandler(void) const { return m_projectiles; }

	void CalculateBoundaries(Vec2 &outMin, Vec2 &outMax);

protected:
	void CreateCamera(void);
	void CreateSpaceBackground(void);
	void SetupLighting(void);

protected:
	static constexpr colour_t AMBIENT_LIGHT_COLOUR = col(50, 70, 120);
	static constexpr colour_t DIRECTIONAL_LIGHT_COLOUR = col(200, 220, 240);

	AsteroidHandler *m_asteroids = nullptr;
	ProjectileHandler *m_projectiles = nullptr;

	scene_t *m_sceneRoot = nullptr;
	camera_t *m_camera = nullptr;
	object_t *m_spaceBackground = nullptr;
	light_t *m_directionalLights[2] = { nullptr, nullptr };
};
