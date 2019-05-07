#include "utils.h"
#include <time.h>
#include <stdlib.h>
#include <mylly/resources/resources.h>

// -------------------------------------------------------------------------------------------------

const colour_t Utils::LABEL_COLOUR;
const colour_t Utils::HOVERED_LABEL_COLOUR;

// -------------------------------------------------------------------------------------------------

void Utils::Initialize(void)
{
	// Seed random number generator.
	srand(time(NULL));
}

float Utils::Random(float min, float max)
{
	return min + ((float)rand() / RAND_MAX) * (max - min);
}

int Utils::Random(int min, int max)
{
	return min + (rand() % (max - min));
}

bool Utils::FlipCoin(void)
{
	return ((rand() & 1) == 0);
}

widget_t *Utils::CreateButton(widget_t *parent, const char *text,
                              anchor_type_t left_type, int16_t left_offset,
                              anchor_type_t right_type, int16_t right_offset,
                              anchor_type_t top_type, int16_t top_offset,
                              anchor_type_t bottom_type, int16_t bottom_offset)
{
	widget_t *button = button_create(parent);

	widget_set_text_font(button, res_get_font("Oxanium-Medium", 48));
	widget_set_text_colour(button, LABEL_COLOUR);
	widget_set_text_s(button, text);

	widget_set_anchors(button,
	                   left_type, left_offset, right_type, right_offset,
	                   top_type, top_offset, bottom_type, bottom_offset);

	return button;
}

widget_t *Utils::CreateLabel(widget_t *parent, const char *text, bool isLarge,
                             anchor_type_t left_type, int16_t left_offset,
                             anchor_type_t right_type, int16_t right_offset,
                             anchor_type_t top_type, int16_t top_offset,
                             anchor_type_t bottom_type, int16_t bottom_offset)
{
	widget_t *label = label_create(parent);

	if (isLarge) {
		widget_set_text_font(label, res_get_font("Oxanium-ExtraBold", 0));
	}
	else {
		widget_set_text_font(label, res_get_font("Oxanium-Medium", 32));	
	}
	
	widget_set_text_colour(label, LABEL_COLOUR);
	widget_set_text_s(label, text);

	widget_set_anchors(label,
	                   left_type, left_offset, right_type, right_offset,
	                   top_type, top_offset, bottom_type, bottom_offset);

	return label;
}
