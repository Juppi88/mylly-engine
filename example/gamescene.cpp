#include "gamescene.h"
#include "game.h"
#include "ship.h"
#include "asteroidhandler.h"
#include "projectilehandler.h"

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

	CreateCamera();
	CreateSpaceBackground();
	SetupLighting();
}

void GameScene::SetupLevel(Game *game)
{
	// Create the player's ship.
	m_ship = new Ship();
	m_ship->Spawn(game);

	// Spawn some asteroids.
	m_asteroids->SpawnInitialAsteroids(game, ASTEROID_LARGE, 5);
}

void GameScene::Update(Game *game)
{
	m_ship->ProcessInput(game);
	m_ship->Update(game);

	m_asteroids->Update(game);
	m_projectiles->Update(game);

	// Enforce ship's boundaries.
	if (!game->IsWithinBoundaries(m_ship->GetPosition())) {
		m_ship->SetPosition(game->WrapBoundaries(m_ship->GetPosition()));
	}
}
