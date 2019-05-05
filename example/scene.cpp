#include "scene.h"
#include "game.h"
#include "asteroidhandler.h"
#include "projectilehandler.h"
#include <mylly/core/mylly.h>
#include <mylly/scene/scene.h>
#include <mylly/scene/object.h>
#include <mylly/scene/camera.h>
#include <mylly/scene/sprite.h>
#include <mylly/scene/light.h>
#include <mylly/resources/resources.h>

// -------------------------------------------------------------------------------------------------

constexpr colour_t Scene::AMBIENT_LIGHT_COLOUR;
constexpr colour_t Scene::DIRECTIONAL_LIGHT_COLOUR;

// -------------------------------------------------------------------------------------------------

Scene::Scene(void)
{
}

Scene::~Scene(void)
{
	delete m_asteroids;
	delete m_projectiles;

	obj_destroy(m_camera->parent);
	obj_destroy(m_spaceBackground);

	obj_destroy(m_directionalLights[0]->parent);
	obj_destroy(m_directionalLights[1]->parent);

	if (m_sceneRoot != nullptr) {

		mylly_set_scene(nullptr);
		scene_destroy(m_sceneRoot);
	}
}

void Scene::Create(Game *game)
{
	// Create a scene root.
	m_sceneRoot = scene_create();
	mylly_set_scene(m_sceneRoot);

	// Create a handler for the asteroids in the game.
	m_asteroids = new AsteroidHandler();
	m_projectiles = new ProjectileHandler();

}

void Scene::CalculateBoundaries(Vec2 &outMin, Vec2 &outMax)
{
	if (m_camera == nullptr) {

		outMin = Vec2(0, 0);
		outMax = Vec2(100, 100);
		return;
	}

	// Calculate game area boundaries with the current camera. OpenGL vertical coordinates
	// get smaller from top to botton, hence the reversed Y coordinate.
	uint16_t screenWidth, screenHeight;
	mylly_get_resolution(&screenWidth, &screenHeight);

	vec3_t min = camera_screen_to_world(m_camera, vec3(0, (float)screenHeight, 0.1f));
	vec3_t max = camera_screen_to_world(m_camera, vec3((float)screenWidth, 0, 0.1f));

	float padding = 2.0f;

	outMin = vec2(min.x - padding, min.z - padding);
	outMax = vec2(max.x + padding, max.z + padding);
}

void Scene::CreateCamera(void)
{
	// Create a camera object and add it to the scene.
	object_t *cameraObject = scene_create_object(m_sceneRoot, nullptr);
	m_camera = obj_add_camera(cameraObject);

	// Setup the camera's view.
	obj_set_position(cameraObject, vector3(0.0f, -50.0f, 0.0f));
	obj_look_at(cameraObject, vec3_zero(), vec3_forward());

	camera_set_orthographic_projection(m_camera, 45, ORTOGRAPHIC_NEAR, 100);

	// Apply anti-aliasing to the rendering result.
	camera_add_post_processing_effect(m_camera, res_get_shader("effect-fxaa"));
}

void Scene::CreateSpaceBackground(void)
{
	// Create an object for the background element and attach a space sprite to it.
	m_spaceBackground = scene_create_object(m_sceneRoot, nullptr);
	obj_set_position(m_spaceBackground, vec3(0, 20, 0));

	sprite_t *sprite = res_get_sprite("space");
	obj_set_sprite(m_spaceBackground, sprite);

	// Get the size of the sprite in world units and scale it to cover the entire view of the camera.
	Vec2 min, max;
	CalculateBoundaries(min, max);

	float spriteWidth = sprite->size.x / sprite->pixels_per_unit;
	float viewWidth = max.x() - min.x();
	float scale = viewWidth / spriteWidth;

	obj_set_local_scale(m_spaceBackground, vec3(scale, scale, 1));

	// Rotate the sprite to face the camera.
	obj_set_local_rotation(m_spaceBackground, quat_from_euler_deg(90, 0, 0));
}

void Scene::SetupLighting(void)
{
	// Adjust ambient lighting.
	scene_set_ambient_light(m_sceneRoot, AMBIENT_LIGHT_COLOUR);

	// Create two directional lights to light the ship and other 3D objects.
	for (int i = 0; i < 2; i++) {
		
		object_t *lightObject = scene_create_object(m_sceneRoot, nullptr);
		light_t *light = obj_add_light(lightObject);

		m_directionalLights[i] = light;

		light_set_type(light, LIGHT_DIRECTIONAL);
		light_set_colour(light, DIRECTIONAL_LIGHT_COLOUR);
		light_set_intensity(light, 1.5f);

		if (i == 0) {
			light_set_direction(light, vec3(10, -3, 5));
		}
		else {
			light_set_direction(light, vec3(-10, -3, -5));
		}
	}
}
