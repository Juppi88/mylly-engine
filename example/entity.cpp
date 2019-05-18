#include "entity.h"
#include "game.h"
#include "collisionhandler.h"
#include <mylly/scene/object.h>
#include <mylly/renderer/debug.h>

// -------------------------------------------------------------------------------------------------

Entity::Entity(EntityType type)
{
	m_type = type;
}

Entity::~Entity(void)
{
	if (m_sceneObject != nullptr) {

		obj_destroy(m_sceneObject);
		m_sceneObject = nullptr;
	}
}

void Entity::Spawn(Game *game)
{
	if (IsSpawned()) {
		return;
	}

	game->GetCollisionHandler()->RegisterEntity(this);
}

void Entity::Destroy(Game *game)
{
	if (!IsSpawned()) {
		return;
	}

	// Inform the current scene that this entity was destroyed.
	game->GetScene()->OnEntityDestroyed(game, this);

	game->GetCollisionHandler()->UnregisterEntity(this);
	delete this;
}

void Entity::Update(Game *game)
{
	// Draw the collider boundaries.
	colour_t circleColour = COL_GREEN;

	if (IsColliding()) {
		circleColour = COL_RED;
	}

	//debug_draw_circle(GetScenePosition().vec(), m_boundingRadius, circleColour, false);

	// Draw a line to indicate the entity's direction.
	Vec2 direction = GetVelocity();
	float speed = direction.Normalize();

	Vec3 sceneDirection = Vec3(direction.x(), 0, direction.y());
	sceneDirection *= 0.5f * speed;
	sceneDirection += GetScenePosition();

	//debug_draw_line(GetScenePosition().vec(), sceneDirection.vec(), COL_GREEN, false);

	// Reset collision entity on each frame before recalculating collisions.
	m_previousCollisionEntity = m_collisionEntity;
	m_collisionEntity = nullptr;

	// Ensure the entity stays within the game area.
	if (!game->IsWithinBoundaries(GetPosition())) {
		SetPosition(game->WrapBoundaries(GetPosition()));
	}
}

void Entity::SetPosition(const Vec2 &position)
{
	m_position = position;

	if (m_sceneObject != nullptr) {
		obj_set_position(m_sceneObject, vec3(m_position.x(), m_drawDepth, m_position.y()));
	}
}

void Entity::OnCollideWith(Entity *other)
{
	m_collisionEntity = other;
}
