#pragma once

#include "floatingobject.h"

// -------------------------------------------------------------------------------------------------

class PowerUp : public FloatingObject
{
public:
	PowerUp(void);
	virtual ~PowerUp(void) override;

	virtual void Spawn(Game *game) override;

	virtual void OnCollideWith(Entity *other) override;

private:
	static constexpr float MAX_SPEED = 5.0f;
};
