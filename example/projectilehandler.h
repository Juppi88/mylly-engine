#pragma once

#include "gamedefs.h"
#include "vector.h"
#include <mylly/collections/array.h>
#include <mylly/math/vector.h>

// -------------------------------------------------------------------------------------------------

class ProjectileHandler
{
public:
	ProjectileHandler(void);
	~ProjectileHandler(void);

	Projectile *FireProjectile(Game *game, Entity *entity,
	                           const Vec2 &spawnPosition, const Vec2 &direction);

	void Update(Game *game);

	void RemoveReference(Projectile *projectile);

private:
	arr_t(Projectile*) m_projectiles = arr_initializer;
};
