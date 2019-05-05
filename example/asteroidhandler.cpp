#include "asteroidhandler.h"
#include "game.h"
#include "utils.h"
#include <mylly/math/math.h>

// -------------------------------------------------------------------------------------------------

AsteroidHandler::AsteroidHandler(void)
{
}

AsteroidHandler::~AsteroidHandler(void)
{
	RemoveAllAsteroids();
}

void AsteroidHandler::SpawnInitialAsteroids(Game *game, AsteroidSize size, uint32_t count)
{
	for (uint32_t i = 0; i < count; i++) {

		Asteroid *asteroid = new Asteroid();
		asteroid->Spawn(game);

		// Randomize initial values.
		Vec2 spawnPosition, spawnDirection;
		GetRandomSpawnPosition(game, spawnPosition, spawnDirection);
		
		asteroid->SetSize(size);
		asteroid->SetPosition(spawnPosition);
		asteroid->SetDirection(spawnDirection);

		// Keep track of active asteroids.
		arr_push(m_asteroids, asteroid);
	}
}

void AsteroidHandler::Update(Game *game)
{
	Asteroid *asteroid;

	// Remove all destroyed asteroids and spawn smaller fragments in their place.
	arr_foreach_reverse(m_asteroids, asteroid) {

		if (asteroid->IsDestroyed()) {

			OnAsteroidDestroyed(asteroid, game);

			// Remove the destroyed asteroid from the game.
			asteroid->Destroy(game);
		}
	}

	// Update all active asteroids.
	arr_foreach(m_asteroids, asteroid) {

		asteroid->Update(game);

		// Enforce game area boundaries.
		if (!game->IsWithinBoundaries(asteroid->GetPosition())) {
			asteroid->SetPosition(game->WrapBoundaries(asteroid->GetPosition()));
		}
	}
}

void AsteroidHandler::RemoveReference(Asteroid *asteroid)
{
	arr_remove(m_asteroids, asteroid);
}

void AsteroidHandler::RemoveAllAsteroids(void)
{
	Asteroid *asteroid;

	arr_foreach(m_asteroids, asteroid) {
		delete asteroid;
	}

	arr_clear(m_asteroids);
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

void AsteroidHandler::OnAsteroidDestroyed(Asteroid *destroyed, Game *game)
{
	// TODO: Spawn some sort of an effect.

	// Smallest asteroids do not split into smaller fragments.
	if (destroyed->GetSize() == ASTEROID_SMALL) {
		return;
	}

	// Small fragments.
	AsteroidSize size = (destroyed->GetSize() == ASTEROID_LARGE ? ASTEROID_MEDIUM : ASTEROID_SMALL);

	for (uint32_t i = 0; i < 2; i++) {

		Asteroid *asteroid = new Asteroid();
		asteroid->Spawn(game);

		// Spawn near the original asteroid.
		Vec2 direction = destroyed->GetVelocity().Normalized();
		direction.Rotate(i == 0 ? -PI / 4 : PI / 4);

		Vec2 position = destroyed->GetPosition() + direction;

		asteroid->SetSize(size);
		asteroid->SetPosition(position);
		asteroid->SetDirection(direction);

		// Keep track of active asteroids.
		arr_push(m_asteroids, asteroid);
	}

}
