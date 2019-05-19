#pragma once

#include "entity.h"
#include "game.h"

// -------------------------------------------------------------------------------------------------

class Ship : public Entity
{
public:
	Ship(void);
	virtual ~Ship(void) override;

	virtual void Spawn(Game *game) override;
	virtual void Update(Game *game) override;

	// Call this before update.
	void ProcessInput(Game *game);

	virtual void OnCollideWith(Entity *other) override;

private:
	void UpdateControls(const InputHandler *input);
	void FireWeapon(Game *game);

private:
	static constexpr float TURN_SPEED = 180; // degrees/sec
	static constexpr float ACCELERATION = 40; // units/sec^2
	static constexpr float MAX_SPEED = 20; // units/sec
	static constexpr float WEAPON_FIRE_RATE = 6; // shots/sec

	float m_heading = 0;
	float m_nextWeaponFire = 0;
};
