#include "asteroidhandler.h"
#include "game.h"
#include "utils.h"

// -------------------------------------------------------------------------------------------------

AsteroidHandler::AsteroidHandler(void)
{
}

AsteroidHandler::~AsteroidHandler(void)
{
	Asteroid *asteroid;

	arr_foreach(m_asteroids, asteroid) {
		delete asteroid;
	}

	arr_clear(m_asteroids);
}

void AsteroidHandler::SpawnAsteroids(Game *game, uint32_t count)
{
	for (uint32_t i = 0; i < count; i++) {

		Asteroid *asteroid = new Asteroid();
		asteroid->Spawn(game);

		// Randomize initial values.
		Vec2 spawnPosition, spawnDirection;
		GetRandomSpawnPosition(game, spawnPosition, spawnDirection);
		
		asteroid->SetSize(ASTEROID_LARGE);
		asteroid->SetPosition(spawnPosition);
		asteroid->SetDirection(spawnDirection);

		// Keep track of active asteroids.
		arr_push(m_asteroids, asteroid);
	}
}

void AsteroidHandler::Update(const Game *game)
{
	// Update all active asteroids.
	Asteroid *asteroid;

	arr_foreach(m_asteroids, asteroid) {

		asteroid->Update();

		// Enforce game area boundaries.
		if (!game->IsWithinBoundaries(asteroid->GetPosition())) {
			asteroid->SetPosition(game->WrapBoundaries(asteroid->GetPosition()));
		}
	}
}

void AsteroidHandler::GetRandomSpawnPosition(const Game *game, Vec2 &position, Vec2 &direction) const
{
	Vec2 min = game->GetBoundsMin();
	Vec2 max = game->GetBoundsMax();

	switch (Utils::Random(0, 4)) {

		case 0: // Left
			position = Vec2(min.x(), Utils::Random(min.y(), max.y()));
			direction = vec2(Utils::Random(0.0f, 1.0f), Utils::Random(-1.0f, 1.0f));
			break;

		case 1: // Right
			position = Vec2(max.x(), Utils::Random(min.y(), max.y()));
			direction = Vec2(Utils::Random(0.0f, -1.0f), Utils::Random(-1.0f, 1.0f));
			break;

		case 2: // Top
			position = Vec2(Utils::Random(min.x(), max.x()), max.y());
			direction = Vec2(Utils::Random(-1.0f, 1.0f), Utils::Random(0.0f, -1.0f));
			break;

		default: // Bottom
			position = Vec2(Utils::Random(min.x(), max.x()), min.y());
			direction = Vec2(Utils::Random(-1.0f, 1.0f), Utils::Random(0.0f, 1.0f));
			break;
	}
}
