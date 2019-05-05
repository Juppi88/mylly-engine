#include "asteroid.h"
#include "asteroidhandler.h"
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

Asteroid::Asteroid(void) :
	Entity(ENTITY_ASTEROID)
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
	SetSceneObject(game->SpawnSceneObject());

	// Create the asteroid object.
	object_t *asteroidObject = game->SpawnSceneObject(GetSceneObject());

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

	// Set the asteroid's health and model's scale based on the size of the asteroid.
	float scale, mass;

	switch (size) {

		case ASTEROID_LARGE:
			scale = 3.0f;
			mass = 300.0f;
			m_health = 3;
			break;

		case ASTEROID_MEDIUM:
			scale = 2.0f;
			mass = 150.0f;
			m_health = 2;
			break;

		default:
			mass = 50.0f;
			scale = 1.0f;
			m_health = 1;
			break;
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
	float speed = GetSpeedMultiplier() * Utils::Random(MOVEMENT_SPEED_MIN, MOVEMENT_SPEED_MAX);
	
	SetVelocity(direction * speed);
}

void Asteroid::Destroy(Game *game)
{
	if (!IsSpawned()) {
		return;
	}

	game->GetScene()->GetAsteroidHandler()->RemoveReference(this);

	// Do final cleanup.
	Entity::Destroy(game);
}

void Asteroid::Update(Game *game)
{
	if (!IsSpawned()) {
		return;
	}

	float dt = get_time().delta_time;

	// Limit the asteroid's speed.
	Vec2 velocity = GetVelocity();

	float speed = velocity.Normalize();

	if (speed > GetSpeedMultiplier() * MOVEMENT_SPEED_MAX) {
		speed = GetSpeedMultiplier() * MOVEMENT_SPEED_MAX;
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

void Asteroid::OnCollideWith(Entity *other)
{
	Entity::OnCollideWith(other);

	if (other->GetType() == ENTITY_PROJECTILE) {

		// Calculate damage to the asteroid.
		if (!IsDestroyed()) {
			--m_health;
		}
	}
}
