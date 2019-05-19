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

	bool AllAsteroidsDestroyed(void) const { return (m_asteroids.count == 0); }

	void DestroyAllAsteroids(Game *game);

private:
	void OnAsteroidDestroyed(Asteroid *destroyed, Game *game);

private:
	arr_t(Asteroid*) m_asteroids = arr_initializer;
};
