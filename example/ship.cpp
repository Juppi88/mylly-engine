#include "ship.h"
#include "inputhandler.h"
#include <mylly/scene/object.h>
#include <mylly/scene/scene.h>
#include <mylly/resources/resources.h>
#include <mylly/core/time.h>
#include <mylly/math/math.h>

// -------------------------------------------------------------------------------------------------

Ship::Ship(void)
{
}

Ship::~Ship(void)
{
	if (m_sceneObject != nullptr) {

		obj_destroy(m_sceneObject);
		m_sceneObject = nullptr;
	}
}

void Ship::Spawn(scene_t *scene)
{
	if (IsSpawned()) {
		return;
	}

	// Load the spaceship model.
	model_t *shipModel = res_get_model("fighterjet");

	if (shipModel == nullptr) {
		return;
	}

	// Create an empty parent object for the ship. This is because the ship model is rotated
	// in a weird way and we want to be able to set the ship's heading without too complex math.
	m_sceneObject = scene_create_object(scene, nullptr);

	// Create an object under the empty parent and attach the model to it.
	object_t *shipObject = scene_create_object(scene, m_sceneObject);
	obj_set_model(shipObject, shipModel);

	// Make the model a bit smaller.
	obj_set_local_scale(shipObject, vec3(0.01f, 0.01f, 0.01f));

	// Rotate the ship model so it's top side up, heading right.
	obj_set_local_rotation(shipObject, quat_from_euler_deg(180, 90, 0));
}

void Ship::Update(const InputHandler *input)
{
	// Process player input.
	UpdateControls(input);

	// Update the ship's transformation.
	obj_set_position(m_sceneObject, vec3(m_position.x, 0, m_position.y));
	obj_set_local_rotation(m_sceneObject, quat_from_euler_deg(0, m_heading, 0));
}

void Ship::UpdateControls(const InputHandler *input)
{
	float dt = get_time().delta_time;

	// Process ship steering.
	float steering = input->GetSteering();

	if (steering != 0) {

		m_heading += dt * TURN_SPEED * steering;
		m_heading = math_sanitize_angle_deg(m_heading);
	}

	// Process ship movement.
	float accelerationPower = input->GetAcceleration();

	if (accelerationPower != 0) {

		float headingRad = DEG_TO_RAD(m_heading);
		accelerationPower *= ACCELERATION * dt;

		// Calculate acceleration direction.
		vec2_t acceleration = vec2(cosf(headingRad), -sinf(headingRad));
		acceleration = vec2_multiply(acceleration, accelerationPower);
		
		// Apply acceleration to velocity.
		m_velocity = vec2_add(m_velocity, acceleration);

		// Limit the ship's speed.
		float speed = vec2_normalize(&m_velocity);
		if (speed > MAX_SPEED) {
			speed = MAX_SPEED;
		}

		m_velocity = vec2_multiply(m_velocity, speed);
	}

	// Apply movement to ship's position.
	vec2_t movement = vec2_multiply(m_velocity, dt);
	m_position = vec2_add(m_position, movement);
}
