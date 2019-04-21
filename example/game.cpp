#include "game.h"
#include "inputhandler.h"
#include "ship.h"
#include <mylly/core/mylly.h>
#include <mylly/scene/scene.h>
#include <mylly/scene/object.h>
#include <mylly/scene/camera.h>

// -------------------------------------------------------------------------------------------------

Game::Game(void)
{
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

	m_input = new InputHandler();

	// Create t main game scene.
	m_gameScene = scene_create();
	mylly_set_scene(m_gameScene);

	// Adjust scene lighting.
	scene_set_ambient_light(m_gameScene, col(150, 150, 150));

	// Setup camera.
	CreateCamera();

	// Create the player's ship.
	m_ship = new Ship();
	m_ship->Spawn(m_gameScene);
}

void Game::Shutdown(void)
{
	if (!IsSetup()) {
		return;
	}

	delete m_ship;
	m_ship = nullptr;

	delete m_input;
	m_input = nullptr;

	obj_destroy(m_camera->parent);

	scene_destroy(m_gameScene);
	m_gameScene = nullptr;
}

void Game::Process(void)
{

}

void Game::CreateCamera(void)
{
	// Create a camera object and add it to the scene.
	object_t *cameraObject = scene_create_object(m_gameScene, nullptr);
	m_camera = obj_add_camera(cameraObject);

	// Setup the camera's view.
	obj_set_position(cameraObject, vector3(0.0f, 5.0f, 0.0f));
	obj_look_at(cameraObject, vec3_zero(), vec3_right());

	camera_set_perspective_projection(m_camera, 60, PERSPECTIVE_NEAR, PERSPECTIVE_FAR);
}
