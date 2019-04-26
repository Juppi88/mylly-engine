#pragma once

#include "gamedefs.h"
#include "vector.h"

// -------------------------------------------------------------------------------------------------

class Entity
{
public:
	virtual ~Entity(void);
	virtual void Spawn(Game *game);
	virtual void Update(void);

	Vec2 GetPosition(void) const { return m_position; }
	void SetPosition(const Vec2 &position);
	Vec3 GetScenePosition(void) const { return Vec3(m_position.x(), 0, m_position.y()); }

	float GetBoundingRadius(void) const { return m_boundingRadius; }

	bool IsSpawned(void) const { return (m_sceneObject != nullptr); }

	bool IsColliding(void) const { return (m_collisionEntity != nullptr); }
	virtual void OnCollideWith(Entity *other);

protected:
	void SetBoundingRadius(float radius) { m_boundingRadius = radius; }

	object_t *GetSceneObject(void) const { return m_sceneObject; }
	void SetSceneObject(object_t *obj) { m_sceneObject = obj; }

private:
	object_t *m_sceneObject = nullptr;

	Vec2 m_position = Vec2();
	float m_boundingRadius = 1.0f;

	Entity *m_collisionEntity = nullptr;
};
