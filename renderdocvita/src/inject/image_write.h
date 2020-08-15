#ifndef RENDERDOC_VITA_IMAGE_WRITE_H_
#define RENDERDOC_VITA_IMAGE_WRITE_H_

#include <vitasdk.h>
#include "../logging.h"

#define STBI_WRITE_NO_STDIO

void* custom_malloc(size_t size);
void* custom_realloc(void* block, size_t size);
void custom_free(void* block);

#define STBIW_MALLOC(sz) custom_malloc(sz)
#define STBIW_REALLOC(p,newsz) custom_realloc(p,newsz)
#define STBIW_FREE(p) custom_free(p)

#define STBIW_ASSERT(x) do {} while(0)

#include "../stb_image_write.h"

#endif
