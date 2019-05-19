#include "floatingobject.h"
#include "game.h"
#include "utils.h"
#include <mylly/scene/object.h>
#include <mylly/core/time.h>
#include <mylly/math/math.h>

// -------------------------------------------------------------------------------------------------

void FloatingObject::Update(Game *game)
{
	if (!IsSpawned()) {
		return;
	}

	float dt = get_time().delta_time;

	// Limit the asteroid's speed.
	Vec2 velocity = GetVelocity();

	float speed = velocity.Normalize();

	if (speed > m_maxSpeed) {
		speed = m_maxSpeed;
	}

	velocity *= speed;
	SetVelocity(velocity);

	// Move the asteroid.
	Vec2 movement = GetVelocity() * dt;
	SetPosition(GetPosition() + movement);

	// Rotate the asteroid.
	Vec2 direction = GetVelocity();
	direction.Normalize();

	Vec3 euler = Vec3(direction.y(), 0, -direction.x());
	euler *= RAD_TO_DEG(dt);

	m_rotation += euler;

	obj_set_local_rotation(GetSceneObject(),
		quat_from_euler_deg(-m_rotation.x(), m_rotation.y(), -m_rotation.z()));

	// Call base update to draw entity debug visualizers.
	Entity::Update(game);
}
