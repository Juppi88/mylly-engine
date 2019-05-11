#include "ship.h"
#include "game.h"
#include "projectilehandler.h"
#include "inputhandler.h"
#include <mylly/scene/object.h>
#include <mylly/scene/scene.h>
#include <mylly/resources/resources.h>
#include <mylly/core/time.h>
#include <mylly/math/math.h>

// -------------------------------------------------------------------------------------------------

Ship::Ship(void) :
	Entity(ENTITY_SHIP)
{
	SetBoundingRadius(2.0f);
	SetMass(200.0f);
}

Ship::~Ship(void)
{
}

void Ship::Spawn(Game *game)
{
	if (IsSpawned()) {
		return;
	}

	Entity::Spawn(game);

	// Load the spaceship model.
	model_t *shipModel = res_get_model("plane");

	if (shipModel == nullptr) {
		return;
	}

	// Create an empty parent object for the ship. This is because the ship model is rotated
	// in a weird way and we want to be able to set the ship's heading without too complex math.
	SetSceneObject(game->SpawnSceneObject());

	// Create an object under the empty parent and attach the model to it.
	object_t *shipObject = game->SpawnSceneObject(GetSceneObject());
	obj_set_model(shipObject, shipModel);

	// Make the model a bit smaller.
	//obj_set_local_scale(shipObject, vec3(0.01f, 0.01f, 0.01f));
	obj_set_local_scale(shipObject, vec3(0.8f, 0.8f, 0.8f));

	// Rotate the ship model so it's top side up, heading right.
	obj_set_local_rotation(shipObject, quat_from_euler_deg(180, 90, 0));
}

void Ship::Update(Game *game)
{
	// Update the ship's transformation.
	obj_set_position(GetSceneObject(), GetScenePosition().vec());
	obj_set_local_rotation(GetSceneObject(), quat_from_euler_deg(0, m_heading, 0));

	Entity::Update(game);
}

void Ship::ProcessInput(Game *game)
{
	float time = get_time().time;
	float dt = get_time().delta_time;

	// Process ship steering.
	float steering = game->GetInputHandler()->GetSteering();

	if (steering != 0) {

		m_heading += dt * TURN_SPEED * steering;
		m_heading = math_sanitize_angle_deg(m_heading);
	}

	// Process ship movement.
	float accelerationPower = game->GetInputHandler()->GetAcceleration();

	if (accelerationPower != 0) {

		float headingRad = DEG_TO_RAD(m_heading);
		accelerationPower *= ACCELERATION * dt;

		// Calculate acceleration direction.
		Vec2 acceleration = Vec2(cosf(headingRad), -sinf(headingRad));
		acceleration *= accelerationPower;
		
		// Apply acceleration to velocity.
		Vec2 velocity = GetVelocity();
		velocity += acceleration;

		// Limit the ship's speed.
		float speed = velocity.Normalize();
		if (speed > MAX_SPEED) {
			speed = MAX_SPEED;
		}

		velocity *= speed;

		SetVelocity(velocity);
	}

	// Apply movement to ship's position.
	Vec2 movement = GetVelocity() * dt;
	Vec2 target = GetPosition() + movement;

	SetPosition(target);

	// Process weapon fire.
	if (game->GetInputHandler()->IsFiring() && time >= m_nextWeaponFire) {

		float rad = DEG_TO_RAD(m_heading + 90);
		Vec2 direction = Vec2(sinf(rad), cosf(rad));

		game->GetScene()->GetProjectileHandler()->FireProjectile(game, this, direction);

		m_nextWeaponFire = time + 1.0f / WEAPON_FIRE_RATE;
	}
}

void Ship::OnCollideWith(Entity *other)
{
	Entity::OnCollideWith(other);

	// Flag the ship as destroyed. The scene will destroy the ship when convenient and spawn
	// an explosion or some other cool effect.
	if (other->GetType() == ENTITY_ASTEROID) {
		m_isDestroyed = true;
	}
}
