#include <graphics/image_data.h>
#include <cstdlib>
#include <cstring>
#include <png.h>
#include <system/debug_log.h>

namespace gef
{
	ImageData::ImageData() :
		image_(NULL),
		clut_(NULL),
        width_(0),
        height_(0)
	{
	}

	ImageData::~ImageData()
	{
		delete image_;
		delete clut_;
	}
	ImageData::ImageData(const char* filename) : ImageData()
	{
        // Setup png_image
        png_image image{};
        image.version = PNG_IMAGE_VERSION;

        // Process png from file
        if (png_image_begin_read_from_file(&image, filename) != 0) {
            image.format = PNG_FORMAT_RGBA;
            png_bytep buffer = (png_bytep)malloc(PNG_IMAGE_SIZE(image));
            if(buffer == NULL){
                gef::DebugOut("PNGLoader: \"%s\": %s\n", filename, "Could not allocate image buffer");
                png_image_free(&image);
                return;
            }
            if (png_image_finish_read(&image, NULL/*background*/, buffer, 0/*row_stride*/, NULL/*colormap*/) != 0) {
                set_image(buffer);
                set_width(image.width);
                set_height(image.height);
                return;
            }
            free(buffer);
        }

        // Error and cleanup
        gef::DebugOut("PNGLoader: \"%s\": %s\n", filename, image.message);
        png_image_free(&image);
        
	}
}