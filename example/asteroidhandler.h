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

	void SpawnAsteroids(Game *game, uint32_t count);

	void Update(Game *game);

private:
	void GetRandomSpawnPosition(const Game *game, Vec2 &position, Vec2 &direction) const;

private:
	arr_t(Asteroid*) m_asteroids = arr_initializer;
};
