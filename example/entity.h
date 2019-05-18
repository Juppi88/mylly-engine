#pragma once

#include "gamedefs.h"
#include "vector.h"

// -------------------------------------------------------------------------------------------------

enum EntityType {
	
	ENTITY_NONE,
	ENTITY_SHIP,
	ENTITY_ASTEROID,
	ENTITY_PROJECTILE,
	ENTITY_UFO,
	ENTITY_POWERUP,
};

// -------------------------------------------------------------------------------------------------

class Entity
{
public:
	virtual ~Entity(void);
	virtual void Spawn(Game *game);
	virtual void Destroy(Game *game);
	virtual void Update(Game *game);

	EntityType GetType(void) const { return m_type; }

	Vec2 GetPosition(void) const { return m_position; }
	void SetPosition(const Vec2 &position);
	Vec3 GetScenePosition(void) const { return Vec3(m_position.x(), 0, m_position.y()); }

	Vec2 GetVelocity(void) const { return m_velocity; }
	void SetVelocity(const Vec2 &velocity) { m_velocity = velocity; }

	float GetDrawDepth(void) const { return m_drawDepth; }
	void SetDrawDepth(float depth) { m_drawDepth = depth; }

	float GetBoundingRadius(void) const { return m_boundingRadius; }
	float GetMass(void) const { return m_mass; }

	bool IsSpawned(void) const { return (m_sceneObject != nullptr); }

	bool IsCollidable(void) const { return m_isCollidable; }
	bool IsColliding(void) const { return (m_collisionEntity != nullptr); }
	bool WasCollidingWith(Entity *other) const { return (m_previousCollisionEntity == other); }
	virtual void OnCollideWith(Entity *other);

	int GetHealth(void) const { return m_health; }
	bool IsDestroyed(void) const { return (m_health == 0); }

protected:
	Entity(EntityType type);

	void SetBoundingRadius(float radius) { m_boundingRadius = radius; }
	void SetMass(float mass) { m_mass = mass; }

	object_t *GetSceneObject(void) const { return m_sceneObject; }
	void SetSceneObject(object_t *obj) { m_sceneObject = obj; }

	void SetCollidable(bool isCollidable) { m_isCollidable = isCollidable; }

	void SetHealth(int health) { m_health = health; }
	void DecreaseHealth(int amount = 1) { if (!IsDestroyed()) { --m_health; } }
	void Kill(void) { m_health = 0; }

private:
	Entity(void);

private:
	object_t *m_sceneObject = nullptr;

	EntityType m_type = ENTITY_NONE;

	Vec2 m_position = Vec2();
	Vec2 m_velocity = Vec2();
	float m_drawDepth = 0.0f;

	float m_boundingRadius = 1.0f;
	float m_mass = 1.0f;

	bool m_isCollidable = true;
	Entity *m_collisionEntity = nullptr;
	Entity *m_previousCollisionEntity = nullptr;

	int m_health = 1;
};
