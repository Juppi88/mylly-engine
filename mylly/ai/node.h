#pragma once
#ifndef __AI_NODE_H
#define __AI_NODE_H

#include "core/defines.h"
#include "collections/array.h"

BEGIN_DECLARATIONS;

// -------------------------------------------------------------------------------------------------

typedef enum {

	AI_NODE_ROOT, // The entry point of an AI profile
	AI_NODE_FLOW, // Control how to execute the child nodes of this node
	AI_NODE_TASK, // Perform a task
	AI_NODE_DECORATOR, // Change the result of the immediate child node
	NUM_AI_NODE_TYPES

} ai_node_type_t;

// -------------------------------------------------------------------------------------------------

typedef enum {

	AI_STATE_RUNNING, // The task or branch is running
	AI_STATE_FAILURE, // The task or branch has completed and failed
	AI_STATE_SUCCESS, // The task or branch has completed successfully

} ai_state_t;

// -------------------------------------------------------------------------------------------------

typedef enum {

	// Process all child nodes in order until the first one fails. When it fails, return failure.
	// If all child nodes succeed, return success.
	AI_FLOW_SEQUENCE,

	// Process all nodes in parallel, regardless of whether or not some of them may fail.
	AI_FLOW_PARALLEL,
	
	// Process all child nodes in sequence until one of them succeeds. Returns failure if all of the
	// child nodes fail, true when one of them succeeds.
	AI_FLOW_SELECTOR,

} ai_flow_type_t;

// -------------------------------------------------------------------------------------------------

typedef struct {

	ai_flow_type_t type; // How to execute the child nodes of this flow node
	int last_processed_branch; // Index of the branch which was processed most recently

} ai_flow_node_t;

// -------------------------------------------------------------------------------------------------

typedef struct {

	void *userdata; // Pointer to per-task user data
	ai_state_t (*process)(void *userdata); // Task processing method

} ai_task_node_t;

// -------------------------------------------------------------------------------------------------

typedef struct {

	void *userdata; // Pointer to per-task user data
	ai_state_t (*decorate)(void *userdata, ai_state_t result); // Decorator method

} ai_decorator_node_t;

// -------------------------------------------------------------------------------------------------

typedef struct ai_node_t {

	ai_node_type_t type; // Type of node
	ai_behaviour_t *behaviour; // Behaviour tree the node is a part of
	ai_node_t *parent; // The parent of this node, NULL for a root node
	arr_t(ai_node_t*) children; // List of child nodes attached to this node
	bool enabled; // True if this node/branch is enabled

	// Per-node type data.
	union {
		ai_flow_node_t flow;
		ai_task_node_t task;
		ai_decorator_node_t decorator;
	};

} ai_node_t;

// -------------------------------------------------------------------------------------------------

// Constructor macros for AI nodes defined above.

#if !defined(__cplusplus) || !defined(_MSC_VER)
	#define ai_task(data, process) (ai_task_node_t){ (void *)data, process }
	#define ai_decorator(data, decorate) (ai_decorator_node_t){ (void *)data, decorate }
#else
	#define ai_task(data, process) { (void *)data, process }
	#define ai_decorator(data, decorate) { (void *)data, decorate }
#endif

// -------------------------------------------------------------------------------------------------

// Create a root AI behaviour tree node. NOTE: Should only be called by the behaviour tree. To add
// child nodes to a behaviour tree, call ai_node_add_XXX methods below.
ai_node_t *ai_node_create_root(ai_behaviour_t *behaviour);

ai_node_t *ai_node_add_flow(ai_node_t *parent, ai_flow_type_t type);
ai_node_t *ai_node_add_task(ai_node_t *parent, const ai_task_node_t task_data);
ai_node_t *ai_node_add_decorator(ai_node_t *parent, const ai_decorator_node_t decorator_data);

// Destroy a behaviour tree and all its child nodes.
void ai_node_destroy(ai_node_t *node);

ai_state_t ai_node_process(ai_node_t *node);

uint32_t ai_node_max_children(ai_node_t *node);

END_DECLARATIONS;

#endif
