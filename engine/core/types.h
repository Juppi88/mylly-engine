#pragma once
#ifndef __TYPES_H
#define __TYPES_H

/*
====================================================================================================

	Types

	Contains forward declarations of most common engine types.

====================================================================================================
*/

// Rendering and models
typedef struct font_t font_t;
typedef struct mesh_t mesh_t;
typedef struct model_t model_t;
typedef struct shader_t shader_t;
typedef struct texture_t texture_t;

// Scene and scene objects
typedef struct animator_t animator_t;
typedef struct camera_t camera_t;
typedef struct emitter_t emitter_t;
typedef struct object_t object_t;
typedef struct scene_t scene_t;
typedef struct sprite_t sprite_t;
typedef struct sprite_anim_t sprite_anim_t;

// AI types
typedef struct ai_t ai_t;
typedef struct ai_behaviour_t ai_behaviour_t;
typedef struct ai_node_t ai_node_t;

#endif
