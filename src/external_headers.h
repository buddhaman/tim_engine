#include <SDL2/SDL.h>
#include "GL/gl3w.h"

#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_FIXED_TYPES
#include "nuklear.h"
#include "nuklear_sdl_gl3.h"

#include "stb_ds.h"
#include "stb_image.h"
#include "stb_image_write.h"
#include "stb_truetype.h"

#include "chipmunk/chipmunk_structs.h"
#include "chipmunk/chipmunk.h"

#include "tinydir.h"

