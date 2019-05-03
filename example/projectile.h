#pragma once

#include "entity.h"

// -------------------------------------------------------------------------------------------------

class Projectile : public Entity
{
	friend class ProjectileHandler;

private:
	Projectile(void);
	virtual ~Projectile(void) override;

	void SetOwner(Entity *owner) { m_owner = owner; }

public:
	virtual void Spawn(Game *game) override;
	virtual void Destroy(Game *game) override;
	virtual void Update(Game *game) override;

	Entity *GetOwner(void) const { return m_owner; }
	float GetSpeed(void) const { return SPEED; }

private:
	static constexpr float SPEED = 25.0f; // Units/Sec
	static constexpr float LIFETIME = 1.0f; // Seconds

	Entity *m_owner = nullptr;
	float m_expiresTime = 0;
};
