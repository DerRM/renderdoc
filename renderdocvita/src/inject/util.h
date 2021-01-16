#ifndef RENDERDOC_UTIL_H__
#define RENDERDOC_UTIL_H__

#include <vitasdk.h>

#ifdef __cplusplus
extern "C" {
#endif

char const* sysmodule2str(SceUInt16 id);
char const* taihenerr2str(SceUID res);
char const* sysmodres2str(int res);
char const* kernelmemblock2str(SceKernelMemBlockType mem);

#ifdef __cplusplus
}
#endif

#endif
