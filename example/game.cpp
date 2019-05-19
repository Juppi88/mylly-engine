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

void Game::StartNewGame(void)
{
	// Reset score and ships.
	m_score = 0;
	m_scoreSinceLastUFO = 0;
	m_scoreSinceLastPowerUp = 0;
	m_currentPowerUp = POWERUP_NONE;
	m_ships = 3;
	m_currentLevel = 1;
	m_isLevelCompleted = false;
	m_isRespawning = false;

	m_ui->SetScore(0);
	m_ui->SetShipCount(3);

	LoadLevel(1);
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

	if (m_isRespawning &&
		m_input->IsPressingConfirm()) {

		m_isRespawning = false;

		if (m_ships > 0) {

			// The player has ships left, respawn into the middle of the map.
			((GameScene *)m_scene)->RespawnShip(this);
		}
		else {

			// No more ships left, return to main menu.
			m_nextScene = new MenuScene();
			m_scene->FadeCamera(false);
		}
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
	m_scoreSinceLastUFO += amount;
	m_scoreSinceLastPowerUp += amount;

	m_ui->AddScore(amount);
}

float Game::GetDifficultyMultiplier(void) const
{
	// Each level the game gets harder (more asteroids, harder UFO encounters). This multiplier
	// controls the difficulty of the UFO, as in UFO speed and accuracy.

	// This difficulty multiplier starts from 1 and linearly ramps up each level until reaching
	// its maximum value of 2 on level 10.
	float difficulty = 1.0f + (m_currentLevel - 1) / (10.0f - 1);

	if (difficulty > 2) {
		difficulty = 2;
	}

	return difficulty;
}

void Game::OnLevelCompleted(void)
{
	m_isLevelCompleted = true;
}

void Game::OnShipDestroyed(void)
{
	m_isRespawning = true;

	if (m_ships > 0) {
		--m_ships;
	}

	// Reset powerups.
	m_scoreSinceLastPowerUp = 0;
	m_currentPowerUp = POWERUP_NONE;
}

bool Game::ShouldUFOSpawn(void) const
{
	if (m_currentLevel < 3) {
		return (m_scoreSinceLastUFO >= 2500);
	}
	else if (m_currentLevel < 5) {
		return (m_scoreSinceLastUFO >= 3500);
	}
	else if (m_currentLevel < 7) {
		return (m_scoreSinceLastUFO >= 4500);
	}

	return (m_scoreSinceLastUFO >= 5000);
}

bool Game::HasPlayerEarnedPowerUp(void) const
{
	// Player already has all the powerups.
	if (m_currentPowerUp == LAST_POWERUP) {
		return false;
	}

	if (m_currentPowerUp == POWERUP_NONE &&
		m_scoreSinceLastPowerUp >= 3000) {

		return true;
	}

	if (m_currentPowerUp == POWERUP_WEAPON_DOUBLE &&
		m_scoreSinceLastPowerUp >= 5000) {

		return true;
	}

	return false;
}

void Game::OnPowerUpCollected(void)
{
	// Reset powerup counter.
	m_scoreSinceLastPowerUp = 0;

	switch (m_currentPowerUp) {

		case POWERUP_NONE:
			m_currentPowerUp = POWERUP_WEAPON_DOUBLE;
			break;

		case POWERUP_WEAPON_DOUBLE:
			m_currentPowerUp = POWERUP_WEAPON_WIDE;
			break;

		default:
			m_currentPowerUp = LAST_POWERUP; // No more powerups to be earned!
			break;
	}
}
