#include "ship.h"
#include <mylly/scene/object.h>
#include <mylly/scene/scene.h>
#include <mylly/resources/resources.h>

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

	// Create an empty object into the game scene and attach the model to it.
	m_sceneObject = scene_create_object(scene, nullptr);
	obj_set_model(m_sceneObject, shipModel);

	// Make the model a bit smaller.
	obj_set_local_rotation(m_sceneObject, quat_from_euler_deg(0, 0, 0));
	obj_set_local_scale(m_sceneObject, vec3(0.01f, 0.01f, 0.01f));
}
