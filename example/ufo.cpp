#include "ufo.h"
#include "game.h"
#include "ship.h"
#include "utils.h"
#include "projectilehandler.h"
#include "projectile.h"
#include "gamescene.h"
#include <mylly/scene/object.h>
#include <mylly/scene/scene.h>
#include <mylly/resources/resources.h>
#include <mylly/core/time.h>
#include <mylly/math/math.h>
#include <mylly/ai/ai.h>
#include <mylly/ai/behaviour.h>
#include <mylly/renderer/debug.h>

// -------------------------------------------------------------------------------------------------

Ufo::Ufo(void) :
	Entity(ENTITY_UFO)
{
	SetBoundingRadius(1.3f);
	SetMass(150.0f);
	SetHealth(2);

	// Set initial velocity.
	SetVelocity(Vec2(1, 0));

	// Make sure the UFO can't fire during the first 3 seconds.
	m_nextWeaponFire = get_time().time + 3.0f;
}

Ufo::~Ufo(void)
{
}

void Ufo::Spawn(Game *game)
{
	if (IsSpawned()) {
		return;
	}

	Entity::Spawn(game);

	// Load the UFO model.
	model_t *shipModel = res_get_model("ufo");

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
	obj_set_local_scale(shipObject, vec3(0.3f, 0.3f, 0.3f));

	// Rotate the UFO model so it's top side up, heading right.
	obj_set_local_rotation(shipObject, quat_from_euler_deg(180, 90, 0));

	m_game = game;

	// Create an AI profile.
	SetupAI(game);
}

void Ufo::Update(Game *game)
{
	float dt = get_time().delta_time;

	// Apply movement.
	Vec2 movement = GetVelocity() * dt;
	Vec2 target = GetPosition() + movement;

	SetPosition(target);

	// Update the UFO's transformation.
	obj_set_position(GetSceneObject(), GetScenePosition().vec());
	obj_set_local_rotation(GetSceneObject(), quat_from_euler_deg(0, m_heading, 0));

	Entity::Update(game);
}

void Ufo::OnCollideWith(Entity *other)
{
	Entity::OnCollideWith(other);

	// The UFO isn't affected by asteroid hits, however if the player hits the UFO with their
	// weapons, the UFO's health will be decreased.
	if (other->GetType() == ENTITY_PROJECTILE &&
		((Projectile *)other)->IsOwnedByPlayer()) {

		Kill();
	}
}

void Ufo::SetupAI(Game *game)
{
	ai_t *ai = obj_add_ai(GetSceneObject());
	ai_behaviour_t *behaviour = ai_behaviour_create(ai);

	ai_set_behaviour(ai, behaviour);

	/*
	The AI of the UFO would look something like this as a graph (flow from top->bottom, left->right)

	ROOT
	|- FLOW: parallel
	   |- TASK: move ufo
	   |- TASK: fire if close to ship
	*/

	ai_node_t *sequence = ai_node_add_flow(behaviour->root, AI_FLOW_PARALLEL);
	/*ai_node_t *moveTask =*/ ai_node_add_task(sequence, ai_task(this, AI_ProcessMovement));
	/*ai_node_t *fireTask =*/ ai_node_add_task(sequence, ai_task(this, AI_FireWeaponsWhenCloseEnough));
}

ai_state_t Ufo::AI_ProcessMovement(void *context)
{
	Ufo *self = (Ufo *)context;
	return self->ProcessMovement();
}

ai_state_t Ufo::AI_FireWeaponsWhenCloseEnough(void *context)
{
	Ufo *self = (Ufo *)context;
	return self->FireWeaponsWhenCloseEnough();
}

ai_state_t Ufo::ProcessMovement(void)
{
	// TODO: This is horrible, figure out how to un-horrify it.
	Ship *playerShip = ((GameScene *)m_game->GetScene())->GetPlayerShip();

	// Check whether the player ship is destroyed and if so, continue on the current course.
	if (playerShip == nullptr) {
		return AI_STATE_FAILURE;
	}
	
	// Calculate target position and direction. The UFO tries to get close to the player ship.
	Vec2 target = CalculateTargetPosition(playerShip);
	Vec2 position = GetPosition();
	Vec2 direction = (target - position).Normalized();

	// From the direction, calculate target heading and apply local avoidance to it.
	float targetHeading = CalculateLocalAvoidance(direction);

	// Lerp the UFO's heading towards the target heading.
	float dt = get_time().delta_time;
	m_heading = Utils::RotateTowards(m_heading, targetHeading, dt * TURN_SPEED);

	// Apply direction to velocity.
	float headingRad = DEG_TO_RAD(m_heading);
	float acceleration = ACCELERATION * dt;

	Vec2 addVelocity = Vec2(cosf(headingRad), -sinf(headingRad));
	addVelocity *= acceleration;
	
	// Apply acceleration to velocity.
	Vec2 velocity = GetVelocity();
	velocity += addVelocity;

	// Limit the ship's speed.
	float speed = velocity.Normalize();
	float difficulty = m_game->GetDifficultyMultiplier();

	if (speed > difficulty * MAX_SPEED) {
		speed = difficulty * MAX_SPEED;
	}

	velocity *= speed;

	SetVelocity(velocity);

	// DEBUGGING STUFF!
	/*Vec3 sceneDirection = Vec3(direction.x(), 0, direction.y());
	sceneDirection += GetScenePosition();

	debug_draw_line(GetScenePosition().vec(), sceneDirection.vec(), COL_RED, false);*/

	return AI_STATE_RUNNING;
}

Vec2 Ufo::CalculateTargetPosition(Entity *targetEntity)
{
	// Calculate a position a few units off the actual target (i.e. the player ship).
	Vec2 target = targetEntity->GetPosition();
	Vec2 direction = (GetPosition() - target).Normalized();

	target += direction * 10.0f;

	return target;
}

float Ufo::CalculateLocalAvoidance(const Vec2 &direction)
{
	float heading = RAD_TO_DEG(atan2f(direction.x(), direction.y()));
	heading = math_sanitize_angle_deg(heading - 90);

	// TODO: Calculate local avoidance here!

	return heading;
}

ai_state_t Ufo::FireWeaponsWhenCloseEnough(void)
{
	// TODO: This is horrible, figure out how to un-horrify it.
	Ship *playerShip = ((GameScene *)m_game->GetScene())->GetPlayerShip();

	// Check whether the player ship is destroyed and if so, continue on the current course.
	if (playerShip == nullptr) {
		return AI_STATE_FAILURE;
	}

	// Can the UFO fire again just yet?
	float time = get_time().time;

	if (time < m_nextWeaponFire) {
		return AI_STATE_FAILURE;
	}

	// Calculate target direction for the weapon.
	Vec2 target = CalculateTargetPosition(playerShip);
	Vec2 position = GetPosition();
	Vec2 direction = (target - position).Normalized();

	// We don't want the UFO to be too difficult, so we're adding a bit of random offset to the fire.
	float difficulty = m_game->GetDifficultyMultiplier();

	float heading = RAD_TO_DEG(atan2f(direction.x(), direction.y()));
	heading = math_sanitize_angle_deg(heading - 90);
	heading += (Utils::FlipCoin() ? -1 : 1) * Utils::Random(5.0f, 30.0f) / (1 + 10 * difficulty);
	heading = DEG_TO_RAD(heading);

	direction = Vec2(cosf(heading), -sinf(heading));

	// Fire!
	m_game->GetScene()->GetProjectileHandler()->FireProjectile(
		m_game, this, GetPosition(), direction
	);

	// Set weapon on cooldown.
	m_nextWeaponFire = time + 1.0f / WEAPON_FIRE_RATE;
	
	return AI_STATE_RUNNING;
}
