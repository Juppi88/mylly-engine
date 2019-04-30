#include "asteroid.h"
#include "game.h"
#include "utils.h"
#include <mylly/scene/scene.h>
#include <mylly/scene/object.h>
#include <mylly/scene/model.h>
#include <mylly/core/time.h>
#include <mylly/resources/resources.h>
#include <mylly/math/math.h>
#include <mylly/renderer/debug.h>

// -------------------------------------------------------------------------------------------------

Asteroid::Asteroid(void)
{
}

Asteroid::~Asteroid(void)
{
}

void Asteroid::Spawn(Game *game)
{
	if (IsSpawned()) {
		return;
	}

	Entity::Spawn(game);

	// Create an empty parent object for the asteroid which we can rotate around freely.
	SetSceneObject(scene_create_object(game->GetScene(), nullptr));

	// Create the asteroid object.
	object_t *asteroidObject = scene_create_object(game->GetScene(), GetSceneObject());

	// Load and set an asteroid model.
	model_t *asteroidModel = res_get_model("rock01");
	obj_set_model(asteroidObject, asteroidModel);

	// Randomize the asteroid's initial position.
	quat_t randomRotation = quat_from_euler_deg(
		Utils::Random(0.0f, 360.0f), Utils::Random(0.0f, 360.0f), Utils::Random(0.0f, 360.0f));

	obj_set_local_rotation(asteroidObject, randomRotation);
}

void Asteroid::SetSize(AsteroidSize size)
{
	if (!IsSpawned()) {
		return;
	}

	float scale;

	switch (size) {
		case ASTEROID_LARGE: scale = 3.0f; break;
		case ASTEROID_MEDIUM: scale = 2.0f; break;
		default: scale = 1.0f; break;
	}

	obj_set_local_scale(GetSceneObject(), vec3(scale, scale, scale));
	m_size = size;

	SetBoundingRadius(0.8f * scale);
	SetMass(100.0f * scale);
}

void Asteroid::SetDirection(const Vec2 &direction)
{
	Vec2 velocity = direction;
	velocity.Normalize();

	// Randomize the speed at which the asteroid is moving.
	float speed = Utils::Random(MOVEMENT_SPEED_MIN, MOVEMENT_SPEED_MAX);
	
	SetVelocity(direction * speed);
}

void Asteroid::Update(void)
{
	if (!IsSpawned()) {
		return;
	}

	float dt = get_time().delta_time;

	// Limit the asteroid's speed.
	Vec2 velocity = GetVelocity();

	float speed = velocity.Normalize();
	if (speed > MOVEMENT_SPEED_MAX) {
		speed = MOVEMENT_SPEED_MAX;
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
	Entity::Update();
}
