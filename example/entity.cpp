#include "entity.h"
#include "game.h"
#include "collisionhandler.h"
#include <mylly/scene/object.h>
#include <mylly/renderer/debug.h>

// -------------------------------------------------------------------------------------------------

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

void Entity::Update(void)
{
	// Draw the boundaries of the entity.
	colour_t circleColour = COL_GREEN;

	if (IsColliding()) {
		circleColour = COL_RED;
	}

	debug_draw_circle(GetScenePosition().vec(), m_boundingRadius, circleColour, false);

	// Reset collision entity on each frame before recalculating collisions.
	m_collisionEntity = nullptr;
}

void Entity::SetPosition(const Vec2 &position)
{
	m_position = position;

	if (m_sceneObject != nullptr) {
		obj_set_position(m_sceneObject, vec3(m_position.x(), 0, m_position.y()));
	}
}

void Entity::OnCollideWith(Entity *other)
{
	m_collisionEntity = other;

	// TODO: Handle collisions differently when it is with the ship.
}
