#pragma once

#include "gamedefs.h"
#include "vector.h"
#include <mylly/renderer/colour.h>

// -------------------------------------------------------------------------------------------------

class Game
{
public:
	Game(void);
	~Game(void);

	CollisionHandler *GetCollisionHandler(void) const { return m_collisionHandler; }
	ProjectileHandler *GetProjectileHandler(void) const { return m_projectiles; }

	scene_t *GetScene(void) const { return m_gameScene; }

	void SetupGame(void);
	void Shutdown(void);

	bool IsSetup(void) const { return (m_gameScene != nullptr); }

	void Update(void);

	bool IsWithinBoundaries(const Vec2 &position) const;
	Vec2 GetBoundsMin(void) const { return m_boundsMin; }
	Vec2 GetBoundsMax(void) const { return m_boundsMax; }
	Vec2 WrapBoundaries(const Vec2 &position) const;

private:
	void CreateCamera(void);
	void CreateSpaceBackground(void);

	void SetupLighting(void);

	void EnforceBoundaries(void);

private:
	static constexpr colour_t AMBIENT_LIGHT_COLOUR = col(50, 70, 120);
	static constexpr colour_t DIRECTIONAL_LIGHT_COLOUR = col(200, 220, 240);

	InputHandler *m_input = nullptr;
	CollisionHandler *m_collisionHandler = nullptr;
	AsteroidHandler *m_asteroids = nullptr;
	ProjectileHandler *m_projectiles = nullptr;
	Ship *m_ship = nullptr;

	scene_t *m_gameScene = nullptr;
	camera_t *m_camera = nullptr;
	object_t *m_spaceBackground = nullptr;
	light_t *m_directionalLights[2] = { nullptr, nullptr };

	Vec2 m_boundsMin = Vec2();
	Vec2 m_boundsMax = Vec2();
};
