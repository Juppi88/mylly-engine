#include "texture.h"
#include "core/memory.h"
#include "core/string.h"
#include "renderer/renderer.h"
#include "resources/resource.h"
#include "io/log.h"
#include <png.h>
#include <jpeglib.h>
#include <jerror.h>
#include <stdio.h>

// -------------------------------------------------------------------------------------------------

// PNG loader helpers.
static void texture_read_png_data(png_structp ptr, png_bytep data, png_size_t length);

// JPEG loader helpers.
static boolean texture_jpeg_loader_fill_input_buffer(j_decompress_ptr cinfo);
static void texture_jpeg_loader_skip_input_data(j_decompress_ptr cinfo, long num_bytes);
static void texture_jpeg_loader_no_op(j_decompress_ptr cinfo);

// -------------------------------------------------------------------------------------------------

// A temporary file-like struct for reading PNG data from an in-memory block.
typedef struct {

	void *buffer;
	size_t length;
	size_t offset;

} png_file_t;

// -------------------------------------------------------------------------------------------------

texture_t *texture_create(const char *name, const char *path)
{
	NEW(texture_t, texture);

	texture->resource.res_name = string_duplicate(name);
	texture->resource.name = texture->resource.res_name;

	if (path != NULL) {
		texture->resource.path = string_duplicate(path);
	}

	texture->gpu_texture = -1;

	arr_init(texture->sprites);

	return texture;
}

void texture_destroy(texture_t *texture)
{
	if (texture == NULL) {
		return;
	}

	// Release GPU resources.
	rend_delete_texture(texture->gpu_texture);

	arr_clear(texture->sprites);

	DESTROY(texture->resource.res_name);
	DESTROY(texture->resource.path);
	DESTROY(texture->data);
	DESTROY(texture);
}

bool texture_load_png(texture_t *texture, void *data, size_t data_length, TEX_FILTER filter)
{
	if (texture == NULL || data == NULL) {
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

	if (colour_type != PNG_COLOR_TYPE_RGB &&
		colour_type != PNG_COLOR_TYPE_RGB_ALPHA) {

		log_warning(
			"Resources",
			"Unsupported texture type %d (currently only RGB with or without alpha is supported)",
			colour_type
		);

		png_destroy_read_struct(&png, &info, &end);
		return false;
	}

	bool has_alpha = (colour_type == PNG_COLOR_TYPE_RGB_ALPHA);

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

	// Load the texture onto the GPU.
	texture->gpu_texture = rend_generate_texture(texture->data, texture->width, texture->height,
                                            has_alpha ? TEX_FORMAT_RGBA : TEX_FORMAT_RGB, filter);

	return true;
}

static void texture_read_png_data(png_structp ptr, png_bytep data, png_size_t length)
{
	png_file_t *file = (png_file_t *)png_get_io_ptr(ptr);

	memcpy(data, (const void *)((const char *)file->buffer + file->offset), length);

	file->offset += length;
}

bool texture_load_jpeg(texture_t *texture, void *data, size_t data_length, TEX_FILTER filter)
{
	// Create a jpeg decompressor struct with the default error handler.
	// TODO: The default handler uses exit() on certain issues, so this may have to be replaced
	// with a custom error handler in the future.
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jpeg_err;
	
	cinfo.err = jpeg_std_error(&jpeg_err);
	jpeg_create_decompress(&cinfo);

	// libjpeg62 doesn't offer a way to decompress jpeg data from data stored into memory, so we'll
	// have to explicitly define a source and a few handler methods for the decompressor.
	cinfo.src = (*cinfo.mem->alloc_small)(
		(j_common_ptr)&cinfo,
		JPOOL_PERMANENT,
		sizeof(struct jpeg_source_mgr)
	);

	cinfo.src->init_source = texture_jpeg_loader_no_op;
	cinfo.src->fill_input_buffer = texture_jpeg_loader_fill_input_buffer;
	cinfo.src->skip_input_data = texture_jpeg_loader_skip_input_data;
	cinfo.src->resync_to_restart = jpeg_resync_to_restart;
	cinfo.src->term_source = texture_jpeg_loader_no_op;
	cinfo.src->bytes_in_buffer = data_length;
	cinfo.src->next_input_byte = (JOCTET *)data;

	// Read the jpeg header to ensure the data in the memory is from a valid jpeg file.
	if (jpeg_read_header(&cinfo, true) != 1) {
		
		log_warning("Resources", "File %s is not a normal JPEG.", texture->resource.path);
		return false;
	}

	// Decompress the jpeg data and allocate buffers for the decompressed bitmap.
	jpeg_start_decompress(&cinfo);

	texture->width = cinfo.output_width;
	texture->height = cinfo.output_height;

	int bytes_per_pixel = cinfo.output_components;
	size_t texture_size = texture->width * texture->height * bytes_per_pixel;

	texture->data = mem_alloc(texture_size);

	// Read the scanlines of the jpeg into the bitmap.
	uint8_t *bitmap_buffer = texture->data;
	int row_stride = texture->width * bytes_per_pixel;

	while (cinfo.output_scanline < cinfo.output_height) {

		uint8_t *buffer_array[1];

		buffer_array[0] = bitmap_buffer + (cinfo.output_scanline) * row_stride;
		jpeg_read_scanlines(&cinfo, buffer_array, 1);
	}

	// Done reading the bitmap data, time for memory cleanup.
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	// Figure out texture format from the pixel size of the bitmap.
	TEX_FORMAT format;

	switch (bytes_per_pixel) {
		case 3:
			format = TEX_FORMAT_RGB;
			break;

		case 4:
			format = TEX_FORMAT_RGBA;
			break;

		default:
			log_warning("Resources", "Unsupported pixel size %d in JPEG file %s.",
			             bytes_per_pixel, texture->resource.path);
			return false;
	}

	// Load the texture onto the GPU.
	texture->gpu_texture = rend_generate_texture(texture->data, texture->width, texture->height,
                                                 format, filter);

	return true;
}

static boolean texture_jpeg_loader_fill_input_buffer(j_decompress_ptr cinfo)
{
	// The loader should never reach here as the buffer should already be loaded into memory.
    ERREXIT(cinfo, JERR_INPUT_EMPTY);
	return true;
}

static void texture_jpeg_loader_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
    if (num_bytes > 0) {
        cinfo->src->next_input_byte += (size_t)num_bytes;
        cinfo->src->bytes_in_buffer -= (size_t)num_bytes;
    }
}

static void texture_jpeg_loader_no_op(j_decompress_ptr cinfo)
{
	// Since we're using a memory block as the data source, we can skip some operations such as
	// initializing and terminating the data source.
	UNUSED(cinfo);
}

bool texture_load_glyph_bitmap(texture_t *texture, uint8_t *data, uint16_t width, uint16_t height)
{
	if (texture == NULL || data == NULL) {
		return false;
	}

	// Remove old texture data.
	DESTROY(texture->data);

	texture->width = width;
	texture->height = height;
	texture->data = data;

	// Generate a GPU object for this texture.
	texture->gpu_texture = rend_generate_texture(texture->data, texture->width, texture->height,
                                                 TEX_FORMAT_GRAYSCALE, TEX_FILTER_POINT);

	return true;
}
