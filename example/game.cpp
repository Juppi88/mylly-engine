#include "game.h"
#include "inputhandler.h"
#include "collisionhandler.h"
#include "utils.h"
#include "gamescene.h"
#include "menuscene.h"
#include <mylly/core/mylly.h>
#include <mylly/scene/scene.h>

// -------------------------------------------------------------------------------------------------

Game::Game(void)
{
	Utils::Initialize();

	m_collisionHandler = new CollisionHandler();
	m_input = new InputHandler();
}

Game::~Game(void)
{
	delete m_input;
	m_input = nullptr;

	delete m_scene;
	m_scene = nullptr;
}

void Game::SetupGame(void)
{
	// Load the main menu scene.
	ChangeScene(new MenuScene());
}

void Game::LoadLevel(uint32_t level)
{
	m_currentLevel = level;
	ChangeScene(new GameScene());
}

void Game::Update(void)
{
	m_scene->Update(this);

	// Process collisions after moving entities.
	m_collisionHandler->Update(this);
}

object_t *Game::SpawnSceneObject(object_t *parent)
{
	if (m_scene == nullptr ||
		m_scene->GetSceneRoot() == nullptr) {

		return nullptr;
	}

	return scene_create_object(m_scene->GetSceneRoot(), parent);
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

void Game::ChangeScene(Scene *nextScene)
{
	// Unload the current scene.
	if (m_scene != nullptr) {

		delete m_scene;
		m_collisionHandler->UnregisterAllEntities();
	}

	// Initialize the next scene.
	m_scene = nextScene;
	m_scene->Create(this);
	m_scene->CalculateBoundaries(m_boundsMin, m_boundsMax);

	// Start the game.
	m_scene->SetupLevel(this);
}
