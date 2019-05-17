#pragma once

#include "gamedefs.h"
#include "vector.h"
#include <mylly/mgui/widget.h>

// -------------------------------------------------------------------------------------------------

class Utils
{
public:
	static void Initialize(void);

	static float Random(float min, float max);
	static int Random(int min, int max);
	static bool FlipCoin(void);

	static float RotateTowards(float current, float target, float amount);

	static void GetRandomSpawnPosition(const Vec2 &boundsMin, const Vec2 &boundsMax,
	                                   Vec2 &position, Vec2 &direction);

	// UI helpers for quickly creating UI widgets.
	static widget_t *CreateButton(widget_t *parent, const char *text,
	                              anchor_type_t left_type, int16_t left_offset,
	                              anchor_type_t right_type, int16_t right_offset,
	                              anchor_type_t top_type, int16_t top_offset,
	                              anchor_type_t bottom_type, int16_t bottom_offset);

	static widget_t *CreateLabel(widget_t *parent, const char *text, bool isLarge,
	                             anchor_type_t left_type, int16_t left_offset,
	                             anchor_type_t right_type, int16_t right_offset,
	                             anchor_type_t top_type, int16_t top_offset,
	                             anchor_type_t bottom_type, int16_t bottom_offset);

	static constexpr colour_t LABEL_COLOUR = col_a(255, 255, 255, 120);
	static constexpr colour_t HOVERED_LABEL_COLOUR = col_a(255, 255, 255, 255);
};
