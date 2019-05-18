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
	virtual void Update(Game *game);

	scene_t *GetSceneRoot(void) const { return m_sceneRoot; }
	AsteroidHandler *GetAsteroidHandler(void) const { return m_asteroids; }
	ProjectileHandler *GetProjectileHandler(void) const { return m_projectiles; }

	void CalculateBoundaries(Vec2 &outMin, Vec2 &outMax);

	void FadeCamera(bool fadeIn);

	virtual void OnEntityDestroyed(Game *game, Entity *entity) {}

protected:
	void CreateCamera(void);

	void SetBackground(uint32_t backgroundIndex);
	object_t *CreateCameraTexture(const char *spriteName, bool isBackground = true);
	void SetupLighting(void);

	bool IsFading(void) const { return (m_fadeEffectEnds != 0); }
	void ProcessFade(Game *game);

protected:
	static constexpr float FADE_DURATION = 0.5f;

	AsteroidHandler *m_asteroids = nullptr;
	ProjectileHandler *m_projectiles = nullptr;

	scene_t *m_sceneRoot = nullptr;
	camera_t *m_camera = nullptr;
	object_t *m_spaceBackground = nullptr;
	light_t *m_directionalLights[2] = { nullptr, nullptr };

	uint32_t m_backgroundIndex = 0;

	object_t *m_fader = nullptr;
	shader_t *m_fadeShader = nullptr;
	float m_fadeEffectEnds = 0;
	bool m_isFadingIn = false;
};
