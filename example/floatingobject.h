#pragma once

#include "entity.h"

// -------------------------------------------------------------------------------------------------

class FloatingObject : public Entity
{
public:
	virtual void Update(Game *game) override;

protected:
	FloatingObject(EntityType type) : Entity(type) {}
	void SetMaxSpeed(float maxSpeed) { m_maxSpeed = maxSpeed; }

private:
	Vec3 m_rotation = Vec3();
	float m_maxSpeed = 1.0f;
};
