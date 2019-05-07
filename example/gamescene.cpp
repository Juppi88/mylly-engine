#include "gamescene.h"
#include "game.h"
#include "ship.h"
#include "ui.h"
#include "asteroidhandler.h"
#include "projectilehandler.h"
#include "inputhandler.h"

// -------------------------------------------------------------------------------------------------

GameScene::GameScene(void) :
	Scene()
{
}

GameScene::~GameScene(void)
{
	delete m_ship;
}

void GameScene::Create(Game *game)
{
	Scene::Create(game);
}

void GameScene::SetupLevel(Game *game)
{
	// Select the level's background.
	SetBackground(game->GetLevel() - 1);

	// Create the player's ship.
	m_ship = new Ship();
	m_ship->Spawn(game);

	// Spawn some asteroids.
	m_asteroids->SpawnInitialAsteroids(game, ASTEROID_LARGE, 2 + game->GetLevel());

	// Fade in to start the level.
	FadeCamera(true);

	// Display the in-game HUD.
	game->GetUI()->ToggleHUD(true);
	game->GetUI()->ShowLevelLabel(game->GetLevel());
}

void GameScene::Update(Game *game)
{
	m_ship->ProcessInput(game);
	m_ship->Update(game);

	m_asteroids->Update(game);
	m_projectiles->Update(game);

	// Godmode button for testing: destroy all asteroids.
	if (game->GetInputHandler()->IsPressingGodmodeButton()) {
		m_asteroids->DestroyAllAsteroids(game);
	}

	// Enforce ship's boundaries.
	if (!game->IsWithinBoundaries(m_ship->GetPosition())) {
		m_ship->SetPosition(game->WrapBoundaries(m_ship->GetPosition()));
	}

	// Complete the level when all asteroids have been destroyed.
	if (m_asteroids->AllAsteroidsDestroyed() &&
		!game->IsLevelCompleted()) {

		game->OnLevelCompleted();
		game->GetUI()->ShowLevelCompletedLabel();
	}

	Scene::Update(game);
}
