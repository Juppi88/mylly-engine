#pragma once

#include "gamedefs.h"
#include <mylly/math/vector.h>

// -------------------------------------------------------------------------------------------------

enum AsteroidSize {
	ASTEROID_SMALL,
	ASTEROID_MEDIUM,
	ASTEROID_LARGE,
};

class Asteroid
{
	friend class AsteroidHandler;

	Asteroid(void);
	~Asteroid(void);

public:
	void Spawn(scene_t *scene);

	vec2_t GetPosition(void) const { return m_position; }

	void SetSize(AsteroidSize size);
	void SetPosition(const vec2_t &position);
	void SetDirection(const vec2_t &direction);
	
	void Update(void);

private:
	static constexpr float MOVEMENT_SPEED_MIN = 3.0f;
	static constexpr float MOVEMENT_SPEED_MAX = 6.0f;

	object_t *m_sceneObject = nullptr;

	AsteroidSize m_size = ASTEROID_SMALL;
	vec2_t m_position = vec2_zero();
	vec2_t m_direction = vec2_zero();

	float m_movementSpeed = 0.0f;
	vec3_t m_rotation = vec3_zero();
};