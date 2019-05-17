#pragma once

#include "entity.h"
#include <mylly/ai/node.h>

// -------------------------------------------------------------------------------------------------

class Ufo : public Entity
{
public:
	Ufo(void);
	virtual ~Ufo(void) override;

	virtual void Spawn(Game *game) override;
	virtual void Update(Game *game) override;

	virtual void OnCollideWith(Entity *other) override;

private:
	void SetupAI(Game *game);

	// Ideally AI tasks would exist in their own classes but since this is a simple demo we don't
	// want to clutter things too much with small classes.
	static ai_state_t AI_ProcessMovement(void *context);
	static ai_state_t AI_FireWeaponsWhenCloseEnough(void *context);

	ai_state_t ProcessMovement(void);
	Vec2 CalculateTargetPosition(Entity *targetEntity);
	float CalculateLocalAvoidance(const Vec2 &direction);

	ai_state_t FireWeaponsWhenCloseEnough(void);

private:
	static constexpr float TURN_SPEED = 90; // degrees/sec
	static constexpr float ACCELERATION = 20; // units/sec^2
	static constexpr float MAX_SPEED = 5; // units/sec
	static constexpr float WEAPON_FIRE_RATE = 0.5f; // shots/sec

	float m_heading = 0;

	float m_nextWeaponFire = 0;

	// TODO: Try to figure out a better way than keeping a reference to the game in the entity!
	Game *m_game = nullptr;
};
