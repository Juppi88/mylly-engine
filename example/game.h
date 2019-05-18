#pragma once

#include "gamedefs.h"
#include "vector.h"
#include "scene.h"

// -------------------------------------------------------------------------------------------------

enum PowerUpType {

	POWERUP_NONE, // Regular poor weapon
	POWERUP_WEAPON_DOUBLE, // Double shooter
	POWERUP_WEAPON_WIDE, // Scattergun

	LAST_POWERUP = POWERUP_WEAPON_WIDE
};

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

	void StartNewGame(void);
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
	uint32_t GetShips(void) const { return m_ships; }
	void AddScore(uint32_t amount);

	float GetDifficultyMultiplier(void) const;

	bool IsLevelCompleted(void) const { return m_isLevelCompleted; }
	void OnLevelCompleted(void);
	void OnShipDestroyed(void);

	bool ShouldUFOSpawn(void) const;
	void ResetUFOCounter(void) { m_scoreSinceLastUFO = 0; }

	bool HasPlayerEarnedPowerUp(void) const;
	PowerUpType GetCurrentPowerUp(void) const { return m_currentPowerUp; }
	void OnPowerUpCollected(void);

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
	uint32_t m_ships = 3;
	bool m_isLevelCompleted = false;
	bool m_isRespawning = false;

	uint32_t m_scoreSinceLastUFO = 0;

	uint32_t m_scoreSinceLastPowerUp = 0;
	PowerUpType m_currentPowerUp = POWERUP_NONE;
};
