#pragma once

#include "entity.h"

// -------------------------------------------------------------------------------------------------

enum AsteroidSize {
	ASTEROID_SMALL,
	ASTEROID_MEDIUM,
	ASTEROID_LARGE,
};

// -------------------------------------------------------------------------------------------------

class Asteroid : public Entity
{
	friend class AsteroidHandler;

	Asteroid(void);
	virtual ~Asteroid(void) override;

public:
	virtual void Spawn(Game *game) override;
	virtual void Update(Game *game) override;

	void SetSize(AsteroidSize size);
	void SetDirection(const Vec2 &direction);

private:
	static constexpr float MOVEMENT_SPEED_MIN = 3.0f;
	static constexpr float MOVEMENT_SPEED_MAX = 6.0f;

	AsteroidSize m_size = ASTEROID_SMALL;
	Vec3 m_rotation = Vec3();
};
