#include "behaviour.h"
#include "node.h"
#include "core/memory.h"

// -------------------------------------------------------------------------------------------------

ai_behaviour_t *ai_behaviour_create(ai_t *ai)
{
	NEW(ai_behaviour_t, tree);

	tree->ai = ai;

	// Automatically create a root node for the tree.
	tree->root = ai_node_create_root(tree);

	return tree;
}

void ai_behaviour_destroy(ai_behaviour_t *tree)
{
	if (tree == NULL) {
		return;
	}

	// Recursively destroy all the nodes of the behaviour tree.
	if (tree->root != NULL) {
		ai_node_destroy(tree->root);
	}

	DESTROY(tree);
}

void ai_behaviour_process(ai_behaviour_t *tree)
{
	if (tree == NULL ||
		tree->root == NULL) {
		return;
	}

	// Start processing from the root node. The processing will go on recursively from node to node.
	ai_node_process(tree->root);
}
