#include "node.h"
#include "ai.h"
#include "core/memory.h"

// -------------------------------------------------------------------------------------------------

static ai_state_t ai_node_process_flow(ai_node_t *node);
static ai_state_t ai_node_process_task(ai_node_t *node);
static ai_state_t ai_node_process_decorator(ai_node_t *node);

// -------------------------------------------------------------------------------------------------

ai_node_t *ai_node_create(ai_node_t *parent)
{
	NEW(ai_node_t, node);

	node->parent = parent;
	node->enabled = true;

	return node;
}

void ai_node_destroy(ai_node_t *node)
{
	if (node == NULL) {
		return;
	}

	// Recursively destroy all child nodes.
	ai_node_t *child;
	arr_foreach(node->children, child) {

		child->parent = NULL;
		ai_node_destroy(child->parent);
	}

	// Remove reference from the parent.
	if (node->parent != NULL) {
		arr_remove(node->parent->children, node);
	}

	DELETE(node);
}

ai_state_t ai_node_process(ai_node_t *node)
{
	if (node == NULL ||
		!node->enabled) {

		// Complete disabled branches successfully.	
		return AISTATE_SUCCESS;
	}

	switch (node->type) {

		case AINODE_ROOT:
		
			// Process the only child node of a root, then return.
			if (node->children.count != 0) {
				return ai_node_process(node->children.items[0]);
			}
			break;

		case AINODE_FLOW:
			return ai_node_process_flow(node);

		case AINODE_TASK:
			return ai_node_process_task(node);

		case AINODE_DECORATOR:
			return ai_node_process_decorator(node);

		default:
			break;
	}

	return AISTATE_SUCCESS;
}

uint32_t ai_node_max_children(ai_node_t *node)
{
	if (node == NULL) {
		return 0;
	}

	switch (node->type) {
		case AINODE_ROOT: return 1;
		case AINODE_FLOW: return 255;
		case AINODE_TASK: return 0;
		case AINODE_DECORATOR: return 1;

		default: return 0;
	}
}

static ai_state_t ai_node_process_flow(ai_node_t *node)
{
	ai_node_t *child;
	ai_state_t result;

	switch (node->flow.type) {

		// Process all child nodes in order until the first one fails or is still running.
		// Returns success if all of the child nodes succeed.
		case AIFLOW_SEQUENCE:
		default:

			arr_foreach(node->children, child) {

				result = ai_node_process(child);

				// Halt processing to the first child which does not report success.
				if (result != AISTATE_SUCCESS) {

					node->flow.last_processed_branch = (int)arr_foreach_index();
					return result;
				}
			}
			break;

		// Process all nodes in parallel, regardless of whether or not some of them may fail.
		case AIFLOW_PARALLEL:

			arr_foreach(node->children, child) {
				ai_node_process(child);
			}
			break;

		// Process all child nodes in sequence until one of them succeeds. Returns failure if
		// all of the child nodes fail, true when one of them succeeds.
		case AIFLOW_SELECTOR:

			arr_foreach(node->children, child) {

				result = ai_node_process(child);

				// Halt processing to the first child which succeeds.
				if (result == AISTATE_SUCCESS) {

					node->flow.last_processed_branch = (int)arr_foreach_index();
					return result;
				}
			}

			return AISTATE_RUNNING;
	}

	return AISTATE_SUCCESS;
}

static ai_state_t ai_node_process_task(ai_node_t *node)
{
	// Run the task processing method.
	if (node->task.process != NULL) {
		return node->task.process(node->userdata);
	}

	return AISTATE_SUCCESS;
}

static ai_state_t ai_node_process_decorator(ai_node_t *node)
{
	ai_state_t result = AISTATE_SUCCESS;

	// Run the only child of the decorator node.
	if (node->children.count != 0) {
		result = ai_node_process(node->children.items[0]);
	}

	// Apply the decorator method to the result.
	if (node->decorator.decorate != NULL) {
		result = node->decorator.decorate(node->userdata, result);
	}

	return result;
}
