#include "game.h"
#include "inputhandler.h"
#include "asteroidhandler.h"
#include "collisionhandler.h"
#include "ship.h"
#include "projectilehandler.h"
#include "utils.h"
#include <mylly/core/mylly.h>
#include <mylly/scene/scene.h>
#include <mylly/scene/object.h>
#include <mylly/scene/camera.h>
#include <mylly/scene/sprite.h>
#include <mylly/scene/light.h>
#include <mylly/resources/resources.h>

// -------------------------------------------------------------------------------------------------

constexpr colour_t Game::AMBIENT_LIGHT_COLOUR;
constexpr colour_t Game::DIRECTIONAL_LIGHT_COLOUR;

// -------------------------------------------------------------------------------------------------

Game::Game(void)
{
	Utils::Initialize();
}

Game::~Game(void)
{
	if (IsSetup()) {
		Shutdown();
	}
}

void Game::SetupGame(void)
{
	if (IsSetup()) {
		return;
	}

	m_collisionHandler = new CollisionHandler();
	m_input = new InputHandler();
	m_projectiles = new ProjectileHandler();

	// Create t main game scene.
	m_gameScene = scene_create();
	mylly_set_scene(m_gameScene);

	// Setup camera.
	CreateCamera();

	// Create a large sprite as the space background.
	CreateSpaceBackground();

	// Setup scene lighting.
	SetupLighting();

	// Create the player's ship.
	m_ship = new Ship();
	m_ship->Spawn(this);

	// Spawn some asteroids.
	m_asteroids = new AsteroidHandler();
	m_asteroids->SpawnInitialAsteroids(this, 5);
}

void Game::Shutdown(void)
{
	if (!IsSetup()) {
		return;
	}

	delete m_asteroids;
	m_asteroids = nullptr;

	delete m_ship;
	m_ship = nullptr;

	delete m_input;
	m_input = nullptr;

	delete m_projectiles;
	m_projectiles = nullptr;

	obj_destroy(m_camera->parent);
	obj_destroy(m_spaceBackground);
	obj_destroy(m_directionalLights[0]->parent);
	obj_destroy(m_directionalLights[1]->parent);

	scene_destroy(m_gameScene);
	m_gameScene = nullptr;
}

void Game::Update(void)
{
	m_ship->ProcessInput(this, m_input);
	m_ship->Update(this);

	m_projectiles->Update(this);
	m_asteroids->Update(this);

	// Process collisions after moving entities.
	m_collisionHandler->Update(this);

	EnforceBoundaries();
}

bool Game::IsWithinBoundaries(const Vec2 &position) const
{
	return (
		position.x() > m_boundsMin.x() &&
		position.x() < m_boundsMax.x() &&
		position.y() < m_boundsMin.y() &&
		position.y() > m_boundsMin.y()
	);
}

Vec2 Game::WrapBoundaries(const Vec2 &position) const
{
	Vec2 wrapped = position;

	if (position.x() < m_boundsMin.x()) wrapped.x(m_boundsMax.x());
	if (position.x() > m_boundsMax.x()) wrapped.x(m_boundsMin.x());
	if (position.y() < m_boundsMin.y()) wrapped.y(m_boundsMax.y());
	if (position.y() > m_boundsMax.y()) wrapped.y(m_boundsMin.y());

	return wrapped;
}

void Game::CreateCamera(void)
{
	// Create a camera object and add it to the scene.
	object_t *cameraObject = scene_create_object(m_gameScene, nullptr);
	m_camera = obj_add_camera(cameraObject);

	// Setup the camera's view.
	obj_set_position(cameraObject, vector3(0.0f, -50.0f, 0.0f));
	obj_look_at(cameraObject, vec3_zero(), vec3_forward());

	camera_set_orthographic_projection(m_camera, 45, ORTOGRAPHIC_NEAR, 100);

	// Calculate game area boundaries with the current camera. OpenGL vertical coordinates
	// get smaller from top to botton, hence the reversed Y coordinate.
	uint16_t screenWidth, screenHeight;
	mylly_get_resolution(&screenWidth, &screenHeight);

	vec3_t min = camera_screen_to_world(m_camera, vec3(0, (float)screenHeight, 0.1f));
	vec3_t max = camera_screen_to_world(m_camera, vec3((float)screenWidth, 0, 0.1f));

	float padding = 2.0f;

	m_boundsMin = vec2(min.x - padding, min.z - padding);
	m_boundsMax = vec2(max.x + padding, max.z + padding);

	// Apply anti-aliasing to the rendering result.
	camera_add_post_processing_effect(m_camera, res_get_shader("effect-fxaa"));
}

void Game::CreateSpaceBackground(void)
{
	// Create an object for the background element and attach a space sprite to it.
	m_spaceBackground = scene_create_object(m_gameScene, nullptr);
	obj_set_position(m_spaceBackground, vec3(0, 20, 0));

	sprite_t *sprite = res_get_sprite("space");
	obj_set_sprite(m_spaceBackground, sprite);

	// Get the size of the sprite in world units and scale it to cover the entire view of the camera.
	float spriteWidth = sprite->size.x / sprite->pixels_per_unit;
	float viewWidth = m_boundsMax.x() - m_boundsMin.x();
	float scale = viewWidth / spriteWidth;

	obj_set_local_scale(m_spaceBackground, vec3(scale, scale, 1));

	// Rotate the sprite to face the camera.
	obj_set_local_rotation(m_spaceBackground, quat_from_euler_deg(90, 0, 0));
}

void Game::SetupLighting(void)
{
	// Adjust ambient lighting.
	scene_set_ambient_light(m_gameScene, AMBIENT_LIGHT_COLOUR);

	// Create two directional lights to light the ship and other 3D objects.
	for (int i = 0; i < 2; i++) {
		
		object_t *lightObject = scene_create_object(m_gameScene, nullptr);
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

void Game::EnforceBoundaries(void)
{
	if (!IsWithinBoundaries(m_ship->GetPosition())) {
		m_ship->SetPosition(WrapBoundaries(m_ship->GetPosition()));
	}
}
