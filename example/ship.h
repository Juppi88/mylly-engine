#pragma once

#include "gamedefs.h"

// -------------------------------------------------------------------------------------------------

class Ship
{
public:
	Ship(void);
	~Ship(void);

	void Spawn(scene_t *scene);

	bool IsSpawned(void) const { return (m_sceneObject != nullptr); }

private:
	object_t *m_sceneObject = nullptr;
};
