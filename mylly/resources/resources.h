#pragma once
#ifndef __RESOURCES_H
#define __RESOURCES_H

#include "core/defines.h"
#include "resources/resource.h"

BEGIN_DECLARATIONS;

// -------------------------------------------------------------------------------------------------

// Special resource names.
#define FONT_TEXTURE_NAME "__FONT_TEXTURE"

// -------------------------------------------------------------------------------------------------

void res_initialize(void);
void res_shutdown(void);

texture_t *res_get_texture(const char *name);
sprite_t *res_get_sprite(const char *name);
shader_t *res_get_shader(const char *name);
sprite_anim_t *res_get_sprite_anim(const char *name);
font_t *res_get_font(const char *name, uint32_t size);
model_t *res_get_model(const char *name);
material_t *res_get_material(const char *name);

sprite_t *res_add_empty_sprite(texture_t *texture, const char *name);

END_DECLARATIONS;

#endif
