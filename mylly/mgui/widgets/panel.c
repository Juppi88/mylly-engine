#include "panel.h"
#include "mgui/widget.h"
#include "renderer/mesh.h"

// -------------------------------------------------------------------------------------------------

static void on_panel_refresh_vertices(widget_t *panel);

// -------------------------------------------------------------------------------------------------

static widget_callbacks_t callbacks = {
	NULL,
	NULL,
	NULL,
	on_panel_refresh_vertices,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};

// -------------------------------------------------------------------------------------------------

widget_t *panel_create(widget_t *parent)
{
	// Create the widget.
	widget_t *widget = widget_create(parent);

	// Initialize type specific data.
	widget->type = WIDGET_TYPE_PANEL;
	widget->callbacks = &callbacks;
	widget->state |= WIDGET_STATE_HAS_MESH;

	widget->panel.use_custom_tex_coord = false;

	return widget;
}

void panel_set_custom_texture_coord(widget_t *panel, vec2_t uv1, vec2_t uv2)
{
	if (panel == NULL || panel->type != WIDGET_TYPE_PANEL) {
		return;
	}

	panel->panel.use_custom_tex_coord = true;
	panel->panel.uv1 = uv1;
	panel->panel.uv2 = uv2;
	panel->has_colour_changed = true;
}

static void on_panel_refresh_vertices(widget_t *panel)
{
	if (panel == NULL || !panel->panel.use_custom_tex_coord) {
		return;
	}

	// Override edge and centre quad UV coordinates with custom ones.
	panel->mesh->ui_vertices[0].uv = vec2(panel->panel.uv1.x, panel->panel.uv1.y);
	panel->mesh->ui_vertices[1].uv = vec2(panel->panel.uv1.x, panel->panel.uv2.y);
	panel->mesh->ui_vertices[2].uv = vec2(panel->panel.uv2.x, panel->panel.uv1.y);
	panel->mesh->ui_vertices[3].uv = vec2(panel->panel.uv2.x, panel->panel.uv2.y);

	panel->mesh->ui_vertices[4].uv = panel->mesh->ui_vertices[0].uv;
	panel->mesh->ui_vertices[5].uv = panel->mesh->ui_vertices[1].uv;
	panel->mesh->ui_vertices[6].uv = panel->mesh->ui_vertices[2].uv;
	panel->mesh->ui_vertices[7].uv = panel->mesh->ui_vertices[3].uv;

	// Leave the rest to zero.
	for (int i = 8; i < 16; i++) {
		panel->mesh->ui_vertices[i].uv = vec2_zero();
	}
}
