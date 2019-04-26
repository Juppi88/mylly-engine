#pragma once

#include "entity.h"

// -------------------------------------------------------------------------------------------------

class Ship : public Entity
{
public:
	Ship(void);
	virtual ~Ship(void) override;

	virtual void Spawn(Game *game) override;
	virtual void Update(void) override;

	// Call this before update.
	void ProcessInput(const InputHandler *input);

private:
	void UpdateControls(const InputHandler *input);

private:
	static constexpr float TURN_SPEED = 180; // degrees/sec
	static constexpr float ACCELERATION = 40; // units/sec^2
	static constexpr float MAX_SPEED = 20; // units/sec

	Vec2 m_velocity = Vec2();
	float m_heading = 0;
};
