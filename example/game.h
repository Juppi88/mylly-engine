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
	UI *GetUI(void) const { return m_ui; }
	Scene *GetScene(void) const { return m_scene; }

	void SetupGame(void);
	bool IsSetup(void) const { return (m_scene != nullptr); }

	void LoadLevel(uint32_t level);
	void ChangeScene(void);

	void Update(void);

	object_t *SpawnSceneObject(object_t *parent = nullptr);

	bool IsWithinBoundaries(const Vec2 &position) const;
	Vec2 GetBoundsMin(void) const { return m_boundsMin; }
	Vec2 GetBoundsMax(void) const { return m_boundsMax; }
	Vec2 WrapBoundaries(const Vec2 &position) const;

	uint32_t GetLevel(void) const { return m_currentLevel; }
	uint32_t GetScore(void) const { return m_score; }
	void AddScore(uint32_t amount);

	bool IsLevelCompleted(void) const { return m_isLevelCompleted; }
	void OnLevelCompleted(void);

private:
	bool IsLoadingLevel(void) const { return (m_nextScene != nullptr); }

private:
	InputHandler *m_input = nullptr;
	CollisionHandler *m_collisionHandler = nullptr;
	UI *m_ui = nullptr;
	Scene *m_scene = nullptr;
	Scene *m_nextScene = nullptr;

	Vec2 m_boundsMin = Vec2();
	Vec2 m_boundsMax = Vec2();

	uint32_t m_currentLevel = 1;
	uint32_t m_score = 0;
	bool m_isLevelCompleted = false;
};
