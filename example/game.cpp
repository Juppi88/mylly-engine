#include "game.h"
#include "inputhandler.h"
#include "collisionhandler.h"
#include "utils.h"
#include "gamescene.h"
#include "menuscene.h"
#include "ui.h"
#include <mylly/core/mylly.h>
#include <mylly/scene/scene.h>

// -------------------------------------------------------------------------------------------------

Game::Game(void)
{
	Utils::Initialize();

	m_collisionHandler = new CollisionHandler();
	m_input = new InputHandler();
	m_ui = new UI();
}

Game::~Game(void)
{
	delete m_input;
	delete m_ui;
	delete m_scene;

	if (m_nextScene != nullptr) {
		delete m_nextScene;
	}
}

void Game::SetupGame(void)
{
	// Setup UI.
	m_ui->Create();

	// Load the main menu scene.
	m_nextScene = new MenuScene();
	ChangeScene();
}

void Game::LoadLevel(uint32_t level)
{
	m_currentLevel = level;

	// Change the scene behind a camera blocking texture. When the fade finishes, it will call
	// ChangeScene in this class.
	m_nextScene = new GameScene();
	m_scene->FadeCamera(false);
}

void Game::Update(void)
{
	m_scene->Update(this);

	if (IsLoadingLevel()) {
		return;
	}

	// Process collisions after moving entities.
	m_collisionHandler->Update(this);

	// Last, update the UI.
	m_ui->Update();

	// Wait for the user to press the confirm key when a level has been completed.
	if (m_isLevelCompleted &&
		m_input->IsPressingConfirm()) {

		m_isLevelCompleted = false;
		LoadLevel(++m_currentLevel);
	}
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

void Game::ChangeScene(void)
{
	if (m_scene != nullptr) {
	
		delete m_scene;
		m_collisionHandler->UnregisterAllEntities();
	}

	// Initialize the next scene.
	m_scene = m_nextScene;
	m_scene->Create(this);
	m_scene->CalculateBoundaries(m_boundsMin, m_boundsMax);

	// Start the game.
	m_scene->SetupLevel(this);
	
	m_ui->SetScore(m_score);
	m_isLevelCompleted = false;

	m_nextScene = nullptr;
}

void Game::AddScore(uint32_t amount)
{
	m_score += amount;
	m_ui->AddScore(amount);
}

void Game::OnLevelCompleted(void)
{
	m_isLevelCompleted = true;
}
