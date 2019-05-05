#pragma once

#include "gamedefs.h"
#include "vector.h"
#include "scene.h"

// -------------------------------------------------------------------------------------------------

class Game
{
public:
	Game(void);
	~Game(void);

	CollisionHandler *GetCollisionHandler(void) const { return m_collisionHandler; }
	InputHandler *GetInputHandler(void) const { return m_input; }
	Scene *GetScene(void) const { return m_scene; }

	void SetupGame(void);
	bool IsSetup(void) const { return (m_scene != nullptr); }

	void LoadLevel(uint32_t level);

	void Update(void);

	object_t *SpawnSceneObject(object_t *parent = nullptr);

	bool IsWithinBoundaries(const Vec2 &position) const;
	Vec2 GetBoundsMin(void) const { return m_boundsMin; }
	Vec2 GetBoundsMax(void) const { return m_boundsMax; }
	Vec2 WrapBoundaries(const Vec2 &position) const;

private:
	void ChangeScene(Scene *nextScene);

private:
	InputHandler *m_input = nullptr;
	CollisionHandler *m_collisionHandler = nullptr;
	Scene *m_scene = nullptr;

	Vec2 m_boundsMin = Vec2();
	Vec2 m_boundsMax = Vec2();

	uint32_t m_currentLevel = 0;
};
