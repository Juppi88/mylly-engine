#include "collisionhandler.h"
#include "entity.h"

// -------------------------------------------------------------------------------------------------

CollisionHandler::CollisionHandler(void)
{

}

CollisionHandler::~CollisionHandler(void)
{
	arr_clear(m_entities);
}

void CollisionHandler::RegisterEntity(Entity *entity)
{
	if (Contains(entity)) {
		return;
	}

	arr_push(m_entities, entity);
}

void CollisionHandler::UnregisterEntity(Entity *entity)
{
	if (!Contains(entity)) {
		return;
	}

	arr_remove(m_entities, entity);
}

void CollisionHandler::UnregisterAllEntities(void)
{
	arr_clear(m_entities);
}

void CollisionHandler::Update(const Game *game)
{
	// This could be optimized a lot by i.e. keeping track of the entities we've checked, but since
	// the object count in the game is so low I'm not going to bother.
	for (uint32_t i = 0; i < m_entities.count; i++) {

		Entity *entity = m_entities.items[i];

		for (uint32_t j = 0; j < m_entities.count; j++) {

			// Skip the entity itself.
			if (i == j) {
				continue;
			}

			if (entity->IsColliding()) {
				continue;
			}

			Entity *other = m_entities.items[j];

			if (EntitiesCollide(entity, other)) {

				if (/*entity->WasCollidingWith(other) ||*/ other->IsColliding()) {
					continue;
				}

				// Detected a collision! Apply collision response and notify both entities.
				if (entity->IsCollidable() && other->IsCollidable()) {
					ApplyCollisionResponse(entity, other);
				}

				entity->OnCollideWith(other);
				other->OnCollideWith(entity);
			}
		}
	}
}

bool CollisionHandler::Contains(Entity *entity) const
{
	int index;
	arr_find(m_entities, entity, index);

	return (index >= 0);
}

bool CollisionHandler::EntitiesCollide(Entity *entity1, Entity* entity2) const
{
	// We're detecting simple sphere-shpere collisions.
	// First, calculate distance between the two entities.
	Vec2 direction = entity2->GetPosition() - entity1->GetPosition();
	float distance = direction.Normalize();

	// If the distance between the two objects is less than their radii combined.
	return (distance < entity1->GetBoundingRadius() + entity2->GetBoundingRadius());
}

void CollisionHandler::ApplyCollisionResponse(Entity *entity1, Entity *entity2) const
{
	// Calculate a direction vector between the two objects.
	Vec2 direction = entity1->GetPosition() - entity2->GetPosition();
	direction.Normalize();

	// Calculate a velocity vector in relation to the direction for object 1.
	Vec2 v1 = entity1->GetVelocity();
	float x1 = direction.Dot(v1);
	Vec2 v1x = direction * x1;
	Vec2 v1y = v1 - v1x;
	float m1 = entity1->GetMass();
	
	// Do the same for the other object.
	direction *= -1;
	Vec2 v2 = entity2->GetVelocity();
	float x2 = direction.Dot(v2);
	Vec2 v2x = direction * x2;
	Vec2 v2y = v2 - v2x;
	float m2 = entity2->GetMass();

	Vec2 velocity1 = v1x * (m1 - m2) / (m1 + m2) + v2x * (2 * m2) / (m1 + m2) + v1y;
	entity1->SetVelocity(velocity1);

	Vec2 velocity2 = v1x * (2 * m1) / (m1 + m2) + v2x * (m2 - m1) / (m1 + m2) + v2y;
	entity2->SetVelocity(velocity2);

	// Repel the other entity so the objects don't get inside of each other.
	float minDistance = (entity1->GetBoundingRadius() + entity2->GetBoundingRadius()) + 0.01f;
	Vec2 position = entity1->GetPosition() + direction * minDistance;

	entity2->SetPosition(position);
}
