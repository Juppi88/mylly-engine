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

	void SpawnAsteroids(const Game *game, uint32_t count);

	void Update(const Game *game);

private:
	void GetRandomSpawnPosition(const Game *game, vec2_t &position, vec2_t &direction) const;

private:
	arr_t(Asteroid*) m_asteroids = arr_initializer;
};
