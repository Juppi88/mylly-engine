#pragma once

#include "gamedefs.h"
#include <mylly/collections/array.h>

// -------------------------------------------------------------------------------------------------

class CollisionHandler
{
public:
	CollisionHandler(void);
	~CollisionHandler(void);

	void RegisterEntity(Entity *entity);
	void UnregisterEntity(Entity *entity);
	void UnregisterAllEntities(void);

	void Update(const Game *game);

private:
	bool Contains(Entity *entity) const;
	bool EntitiesCollide(Entity *entity1, Entity* entity2) const;
	void ApplyCollisionResponse(Entity *entity1, Entity *entity2) const;

private:
	arr_t(Entity*) m_entities = arr_initializer;
};
