#include "node.h"
#include "ai.h"
#include "core/memory.h"
#include "io/log.h"

// -------------------------------------------------------------------------------------------------

static ai_node_t *ai_node_create(ai_node_t *parent, ai_node_type_t type);
static ai_state_t ai_node_process_flow(ai_node_t *node);
static ai_state_t ai_node_process_task(ai_node_t *node);
static ai_state_t ai_node_process_decorator(ai_node_t *node);

// -------------------------------------------------------------------------------------------------

static ai_node_t *ai_node_create(ai_node_t *parent, ai_node_type_t type)
{
	NEW(ai_node_t, node);

	node->parent = parent;
	node->type = type;
	node->enabled = true;

	// Copy behaviour tree reference from the parent node.
	if (parent != NULL) {
		node->behaviour = parent->behaviour;
	}

	return node;
}

ai_node_t *ai_node_create_root(ai_behaviour_t *behaviour)
{
	ai_node_t *node = ai_node_create(NULL, AI_NODE_ROOT);
	node->behaviour = behaviour;

	return node;
}

ai_node_t *ai_node_add_flow(ai_node_t *parent, ai_flow_type_t type)
{
	if (parent == NULL) {
		return NULL;
	}

	// Make sure the parent node is allowed to have more children.
	if (parent->children.count >= ai_node_max_children(parent)) {

		log_warning("AI", "Attempting to add too many child nodes!");
		return NULL;
	}

	// Create the new flow node.
	ai_node_t *node = ai_node_create(parent, AI_NODE_FLOW);
	node->flow.type = type;

	// Attach the node to the parent node.
	arr_push(parent->children, node);

	return node;
}

ai_node_t *ai_node_add_task(ai_node_t *parent, const ai_task_node_t task_data)
{
	if (parent == NULL) {
		return NULL;
	}

	// Make sure the parent node is allowed to have more children.
	if (parent->children.count >= ai_node_max_children(parent)) {

		log_warning("AI", "Attempting to add too many child nodes!");
		return NULL;
	}

	// Create the new task node.
	ai_node_t *node = ai_node_create(parent, AI_NODE_TASK);
	node->task = task_data;

	// Attach the node to the parent node.
	arr_push(parent->children, node);

	return node;
}

ai_node_t *ai_node_add_decorator(ai_node_t *parent, const ai_decorator_node_t decorator_data)
{
	if (parent == NULL) {
		return NULL;
	}

	// Make sure the parent node is allowed to have more children.
	if (parent->children.count >= ai_node_max_children(parent)) {

		log_warning("AI", "Attempting to add too many child nodes!");
		return NULL;
	}

	// Create the new task node.
	ai_node_t *node = ai_node_create(parent, AI_NODE_DECORATOR);
	node->decorator = decorator_data;

	// Attach the node to the parent node.
	arr_push(parent->children, node);

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

		// Set parent reference to NULL to avoid unnecessary array operations (the whole array
		// will be destroyed anyway).
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
		return AI_STATE_SUCCESS;
	}

	switch (node->type) {

		case AI_NODE_ROOT:
		
			// Process the only child node of a root, then return.
			if (node->children.count != 0) {
				return ai_node_process(node->children.items[0]);
			}
			break;

		case AI_NODE_FLOW:
			return ai_node_process_flow(node);

		case AI_NODE_TASK:
			return ai_node_process_task(node);

		case AI_NODE_DECORATOR:
			return ai_node_process_decorator(node);

		default:
			break;
	}

	return AI_STATE_SUCCESS;
}

uint32_t ai_node_max_children(ai_node_t *node)
{
	if (node == NULL) {
		return 0;
	}

	switch (node->type) {
		case AI_NODE_ROOT: return 1;
		case AI_NODE_FLOW: return 255;
		case AI_NODE_TASK: return 0;
		case AI_NODE_DECORATOR: return 1;

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
		case AI_FLOW_SEQUENCE:
		default:

			arr_foreach(node->children, child) {

				result = ai_node_process(child);

				// Halt processing to the first child which does not report success.
				if (result != AI_STATE_SUCCESS) {

					node->flow.last_processed_branch = (int)arr_foreach_index();
					return result;
				}
			}
			break;

		// Process all nodes in parallel, regardless of whether or not some of them may fail.
		case AI_FLOW_PARALLEL:

			arr_foreach(node->children, child) {
				ai_node_process(child);
			}
			break;

		// Process all child nodes in sequence until one of them succeeds. Returns failure if
		// all of the child nodes fail, true when one of them succeeds.
		case AI_FLOW_SELECTOR:

			arr_foreach(node->children, child) {

				result = ai_node_process(child);

				// Halt processing to the first child which succeeds.
				if (result == AI_STATE_SUCCESS) {

					node->flow.last_processed_branch = (int)arr_foreach_index();
					return result;
				}
			}

			return AI_STATE_RUNNING;
	}

	return AI_STATE_SUCCESS;
}

static ai_state_t ai_node_process_task(ai_node_t *node)
{
	// Run the task processing method.
	if (node->task.process != NULL) {
		return node->task.process(node->task.userdata);
	}

	return AI_STATE_SUCCESS;
}

static ai_state_t ai_node_process_decorator(ai_node_t *node)
{
	ai_state_t result = AI_STATE_SUCCESS;

	// Run the only child of the decorator node.
	if (node->children.count != 0) {
		result = ai_node_process(node->children.items[0]);
	}

	// Apply the decorator method to the result.
	if (node->decorator.decorate != NULL) {
		result = node->decorator.decorate(node->decorator.userdata, result);
	}

	return result;
}
