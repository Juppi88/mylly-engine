#include "texture.h"
#include "core/memory.h"
#include "core/string.h"
#include "renderer/renderer.h"
#include "io/log.h"
#include <png.h>
#include <stdio.h>

// --------------------------------------------------------------------------------

static void texture_read_png_data(png_structp ptr, png_bytep data, png_size_t length);

// --------------------------------------------------------------------------------

// A temporary file-like struct for reading PNG data from an in-memory block.
typedef struct {
	void *buffer;
	size_t length;
	size_t offset;
} png_file_t;

// --------------------------------------------------------------------------------

texture_t *texture_create(const char *name, const char *path)
{
	NEW(texture_t, texture);

	texture->resource.name = string_duplicate(name);

	if (path != NULL) {
		texture->resource.path = string_duplicate(path);
	}

	texture->gpu_texture = -1;

	return texture;
}

void texture_destroy(texture_t *texture)
{
	if (texture == NULL) {
		return;
	}

	// Release GPU resources.
	rend_delete_texture(texture->gpu_texture);

	DESTROY(texture->resource.name);
	DESTROY(texture->resource.path);
	DESTROY(texture->data);
	DESTROY(texture);
}

bool texture_load_png(texture_t *texture, void *data, size_t data_length)
{
	if (texture == NULL) {
		return false;
	}

	// Read the PNG file header.
	png_byte header[8];
	memcpy(header, data, sizeof(header));

	// Make sure the file is a PNG.
	if (png_sig_cmp(header, 0, 8)) {
		return false;
	}

	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (png == NULL) {
		return false;
	}

	png_infop info = png_create_info_struct(png);

	if (info == NULL) {

		png_destroy_read_struct(&png, (png_infopp)NULL, (png_infopp)NULL);
		return false;
	}

	png_infop end = png_create_info_struct(png);

	if (end == NULL) {

		png_destroy_read_struct(&png, (png_infopp)info, (png_infopp)NULL);
		return false;
	}
	// Set jump point for handling future errors.
    if (setjmp(png_jmpbuf(png))) {

		png_destroy_read_struct(&png, &info, &end);
		return false;
	}

	// Remove old texture data.
	DESTROY(texture->data);

	// Create a temporary read struct for reading the PNG from an in-memory block instead of a file.
	png_file_t file = {
		.buffer = data,
		.length = data_length,
		.offset = sizeof(header) // Skip the header bytes
	};

	// Initialize reading.
	png_set_read_fn(png, (png_voidp)&file, texture_read_png_data);

	// Make the library ignore the header bytes because the header was already read.
	png_set_sig_bytes(png, sizeof(header));

	// Read all the info up until the image data.
	png_read_info(png, info);

	// Get info about the PNG.
	int bit_depth, colour_type;
	png_uint_32 width, height;

	png_get_IHDR(png, info, &width, &height, &bit_depth, &colour_type, NULL, NULL, NULL);

	if (colour_type != PNG_COLOR_TYPE_RGB_ALPHA) {

		log_warning("Renderer",
			"Unsupported texture type (currently only PNG_COLOR_TYPE_RGB_ALPHA is supported)");

		png_destroy_read_struct(&png, &info, &end);
		return false;
	}

	texture->width = (uint16_t)width;
	texture->height = (uint16_t)height;
	
	// Update the PNG info.
	png_read_update_info(png, info);

	// Get texture row size (in bytes).
	int row_bytes = png_get_rowbytes(png, info);

	// Adjust the row size to OpenGL format (4 byte alignment).
	row_bytes += 3 - ((row_bytes - 1) % 4);

	// Allocate memory for the texture data and texture row pointers.
	png_byte *tex_data = mem_alloc_fast(row_bytes * height * sizeof(png_byte) + 15);
	png_bytep *row_pointers = mem_alloc_fast(height * sizeof(png_bytep));

    // Store pointers to each texture row (this is because of the 4-byte alignment).
    for (uint32_t i = 0; i < height; i++) {
		row_pointers[height - 1 - i] = tex_data + i * row_bytes;
	}

    // Finally read the PNG data.
    png_read_image(png, row_pointers);
    png_read_end(png, NULL);

    texture->data = tex_data;

	// Do cleanup.
	png_destroy_read_struct(&png, &info, &end);
	mem_free(row_pointers);

	// Generate a GPU object for this texture.
	texture->gpu_texture = rend_generate_texture(texture->data, texture->width, texture->height);

	return true;
}

static void texture_read_png_data(png_structp ptr, png_bytep data, png_size_t length)
{
	png_file_t *file = (png_file_t *)png_get_io_ptr(ptr);

	memcpy(data, (const void *)((const char *)file->buffer + file->offset), length);

	file->offset += length;
}
