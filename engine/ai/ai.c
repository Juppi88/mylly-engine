#include "ai.h"
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

	DELETE(ai);
}

void ai_process(ai_t *ai)
{
	if (ai == NULL) {
		return;
	}

	// TODO: Process behaviour tree
}
