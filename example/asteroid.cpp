#include "asteroid.h"
#include "utils.h"
#include <mylly/scene/scene.h>
#include <mylly/scene/object.h>
#include <mylly/scene/model.h>
#include <mylly/core/time.h>
#include <mylly/resources/resources.h>
#include <mylly/math/math.h>

// -------------------------------------------------------------------------------------------------

Asteroid::Asteroid(void)
{
}

Asteroid::~Asteroid(void)
{
	if (m_sceneObject != nullptr) {

		obj_destroy(m_sceneObject);
		m_sceneObject = nullptr;
	}
}

void Asteroid::Spawn(scene_t *scene)
{
	// Create an empty parent object for the asteroid which we can rotate around freely.
	m_sceneObject = scene_create_object(scene, nullptr);

	// Create the asteroid object.
	object_t *asteroidObject = scene_create_object(scene, m_sceneObject);

	// Load and set an asteroid model.
	model_t *asteroidModel = res_get_model("rock01");
	obj_set_model(asteroidObject, asteroidModel);

	// Randomize the asteroid's initial position.
	quat_t randomRotation = quat_from_euler_deg(
		Utils::Random(0.0f, 360.0f), Utils::Random(0.0f, 360.0f), Utils::Random(0.0f, 360.0f));

	obj_set_local_rotation(asteroidObject, randomRotation);

	m_movementSpeed = Utils::Random(MOVEMENT_SPEED_MIN, MOVEMENT_SPEED_MAX);
}

void Asteroid::SetSize(AsteroidSize size)
{
	if (m_sceneObject == nullptr) {
		return;
	}

	float scale;

	switch (size) {
		case ASTEROID_LARGE: scale = 3.0f; break;
		case ASTEROID_MEDIUM: scale = 2.0f; break;
		default: scale = 1.0f; break;
	}

	obj_set_local_scale(m_sceneObject, vec3(scale, scale, scale));
	m_size = size;
}

void Asteroid::SetPosition(const vec2_t &position)
{
	m_position = position;

	if (m_sceneObject != nullptr) {
		obj_set_position(m_sceneObject, vec3(m_position.x, 0, m_position.y));
	}
}

void Asteroid::SetDirection(const vec2_t &direction)
{
	m_direction = direction;
	vec2_normalize(&m_direction);
}

void Asteroid::Update(void)
{
	if (m_sceneObject == nullptr) {
		return;
	}

	float dt = get_time().delta_time;

	// Move the asteroid.
	vec2_t movement = vec2_multiply(m_direction, m_movementSpeed * dt);
	SetPosition(vec2_add(m_position, movement));

	// Rotate the asteroid.
	vec3_t euler = vec3(m_direction.x, 0, m_direction.y);
	euler = vec3_multiply(euler, RAD_TO_DEG(dt));

	m_rotation = vec3_add(m_rotation, euler);

	obj_set_local_rotation(m_sceneObject,
		quat_from_euler_deg(-m_rotation.x, m_rotation.y, -m_rotation.z));
}
