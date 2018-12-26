#pragma once
#ifndef __AI_NODE_H
#define __AI_NODE_H

#include "core/defines.h"
#include "collections/array.h"

BEGIN_DECLARATIONS;

// -------------------------------------------------------------------------------------------------

typedef enum {

	AINODE_ROOT, // The entry point of an AI profile
	AINODE_FLOW, // Control how to execute the child nodes of this node
	AINODE_TASK, // Perform a task
	AINODE_DECORATOR, // Change the result of the immediate child node
	NUM_AINODE_TYPES

} ai_node_type_t;

// -------------------------------------------------------------------------------------------------

typedef enum {

	AISTATE_RUNNING, // The task or branch is running
	AISTATE_FAILURE, // The task or branch has completed and failed
	AISTATE_SUCCESS, // The task or branch has completed successfully

} ai_state_t;

// -------------------------------------------------------------------------------------------------

typedef enum {

	// Process all child nodes in order until the first one fails. When it fails, return failure.
	// If all child nodes succeed, return success.
	AIFLOW_SEQUENCE,

	// Process all nodes in parallel, regardless of whether or not some of them may fail.
	AIFLOW_PARALLEL,
	
	// Process all child nodes in sequence until one of them succeeds. Returns failure if all of the
	// child nodes fail, true when one of them succeeds.
	AIFLOW_SELECTOR,

} ai_flow_type_t;

// -------------------------------------------------------------------------------------------------

typedef struct ai_node_t {

	ai_node_type_t type; // Type of node
	ai_node_t *parent; // The parent of this node, NULL for a root node
	arr_t(ai_node_t*) children; // List of child nodes attached to this node
	bool enabled; // True if this node/branch is enabled
	void *userdata; // Pointer to per-task user data

	// Per-node type data.
	union {
		struct {
			ai_flow_type_t type; // How to execute the child nodes of this flow node
			int last_processed_branch; // Index of the branch which was processed most recently
		} flow;

		struct {
			ai_state_t (*process)(void *userdata); // Task processing method
		} task;

		struct {
			ai_state_t (*decorate)(void *userdata, ai_state_t result);
		} decorator;
	};

} ai_node_t;

// -------------------------------------------------------------------------------------------------

ai_node_t *ai_node_create(ai_node_t *parent);
void ai_node_destroy(ai_node_t *node);

ai_state_t ai_node_process(ai_node_t *node);

uint32_t ai_node_max_children(ai_node_t *node);

END_DECLARATIONS;

#endif
