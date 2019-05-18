#pragma once

#include "floatingobject.h"

// -------------------------------------------------------------------------------------------------

enum AsteroidSize {
	ASTEROID_SMALL,
	ASTEROID_MEDIUM,
	ASTEROID_LARGE,
};

// -------------------------------------------------------------------------------------------------

class Asteroid : public FloatingObject
{
	friend class AsteroidHandler;

	Asteroid(void);
	virtual ~Asteroid(void) override;

public:
	virtual void Spawn(Game *game) override;
	virtual void Destroy(Game *game) override;

	AsteroidSize GetSize(void) const { return m_size; }
	void SetSize(AsteroidSize size);

	void SetDirection(const Vec2 &direction);

	virtual void OnCollideWith(Entity *other) override;

private:
	float GetSpeedMultiplier(void) const { return 3.0f / (m_size + 1); }

private:
	static constexpr float MOVEMENT_SPEED_MIN = 3.0f;
	static constexpr float MOVEMENT_SPEED_MAX = 6.0f;

	AsteroidSize m_size = ASTEROID_SMALL;
};
