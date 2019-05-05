#pragma once

#include "gamedefs.h"
#include "asteroid.h"
#include <mylly/collections/array.h>
#include <mylly/math/vector.h>

// -------------------------------------------------------------------------------------------------

class AsteroidHandler
{
public:
	AsteroidHandler(void);
	~AsteroidHandler(void);

	void SpawnInitialAsteroids(Game *game, AsteroidSize size, uint32_t count);

	void Update(Game *game);

	void RemoveReference(Asteroid *asteroid);
	void RemoveAllAsteroids(void);

private:
	void GetRandomSpawnPosition(const Game *game, Vec2 &position, Vec2 &direction) const;
	void OnAsteroidDestroyed(Asteroid *destroyed, Game *game);

private:
	arr_t(Asteroid*) m_asteroids = arr_initializer;
};
