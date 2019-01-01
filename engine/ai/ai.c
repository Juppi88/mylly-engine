#include "ai.h"
#include "behaviour.h"
#include "core/memory.h"

// -------------------------------------------------------------------------------------------------

ai_t *ai_create(object_t *parent)
{
	NEW(ai_t, ai);

	ai->parent = parent;

	return ai;
}

void ai_destroy(ai_t *ai)
{
	if (ai == NULL) {
		return;
	}

	// Destroy assigned behaviour tree.
	if (ai->behaviour != NULL) {
		ai_behaviour_destroy(ai->behaviour);
	}

	DESTROY(ai);
}

void ai_process(ai_t *ai)
{
	if (ai == NULL) {
		return;
	}

	// Process behaviour tree.
	if (ai->behaviour != NULL) {
		ai_behaviour_process(ai->behaviour);
	}
}
