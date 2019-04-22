#pragma once

#include "gamedefs.h"
#include <mylly/math/vector.h>

// -------------------------------------------------------------------------------------------------

class Ship
{
public:
	Ship(void);
	~Ship(void);

	void Spawn(scene_t *scene);

	void Update(const InputHandler *input);

	bool IsSpawned(void) const { return (m_sceneObject != nullptr); }

	vec2_t GetPosition(void) const { return m_position; }
	void SetPosition(const vec2_t &position) { m_position = position; }

private:
	void UpdateControls(const InputHandler *input);

private:
	static constexpr float TURN_SPEED = 180; // degrees/sec
	static constexpr float ACCELERATION = 40; // units/sec^2
	static constexpr float MAX_SPEED = 20; // units/sec

	object_t *m_sceneObject = nullptr;

	vec2_t m_position = vec2_zero();
	vec2_t m_velocity = vec2_zero();
	float m_heading = 0;
};
