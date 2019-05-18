#include "powerup.h"
#include "game.h"
#include "utils.h"
#include <mylly/scene/object.h>
#include <mylly/resources/resources.h>
#include <mylly/math/math.h>

// -------------------------------------------------------------------------------------------------

PowerUp::PowerUp(void) :
	FloatingObject(ENTITY_POWERUP)
{
	SetMaxSpeed(MAX_SPEED);
}

PowerUp::~PowerUp(void)
{
}

void PowerUp::Spawn(Game *game)
{
	if (IsSpawned()) {
		return;
	}

	Entity::Spawn(game);

	SetSceneObject(game->SpawnSceneObject());

	model_t *crateModel = res_get_model("crate");
	obj_set_model(GetSceneObject(), crateModel);

	obj_set_local_scale(GetSceneObject(), vec3(1.5f, 1.5f, 1.5f));

	// Make the powerup to move to a random direction.
	Vec2 direction = Vec2(Utils::Random(-1.0f, 1.0f), Utils::Random(-1.0f, 1.0f)).Normalized();
	float speed = Utils::Random(1.0f, MAX_SPEED);
	
	SetVelocity(direction * speed);
}

void PowerUp::OnCollideWith(Entity *other)
{
	Entity::OnCollideWith(other);

	// If the powerup collided with the player ship, it was collected. Destroy the powerup which
	// will inform the game that it was collected.
	if (other->GetType() == ENTITY_SHIP) {
		Kill();
	}
}
