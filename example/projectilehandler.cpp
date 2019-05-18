#include "projectilehandler.h"
#include "projectile.h"
#include "game.h"

// -------------------------------------------------------------------------------------------------

ProjectileHandler::ProjectileHandler(void)
{

}

ProjectileHandler::~ProjectileHandler(void)
{
	Projectile *projectile;

	arr_foreach(m_projectiles, projectile) {
		delete projectile;
	}

	arr_clear(m_projectiles);
}

Projectile *ProjectileHandler::FireProjectile(Game *game, Entity *entity,
                                              const Vec2 &spawnPosition, const Vec2 &direction)
{
	if (game == nullptr || entity == nullptr) {
		return nullptr;
	}

	Projectile *projectile = new Projectile();
	projectile->SetOwner(entity);
	projectile->Spawn(game);
	projectile->SetPosition(spawnPosition);

	// Calculate an initial velocity for the asteroid.
	Vec2 velocity = direction;
	velocity.Normalize();
	velocity *= projectile->GetSpeed();
	velocity += entity->GetVelocity();

	projectile->SetVelocity(velocity);

	// Store the projectile to a list for processing.
	arr_push(m_projectiles, projectile);

	return nullptr;
}

void ProjectileHandler::Update(Game *game)
{
	Projectile *projectile;

	arr_foreach_reverse(m_projectiles, projectile) {
		projectile->Update(game);
	}
}

void ProjectileHandler::RemoveReference(Projectile *projectile)
{
	arr_remove(m_projectiles, projectile);
}
