
#include "file_manager.h"
#include "resource_manager.h"

ResourceManager g_resource_manager;

#include <atomic>

#ifdef __cplusplus
extern "C" {
#endif

#include <vitasdk.h>
#include <taihen.h>

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <new>

#include "util.h"
#include "../logging.h"
#include "../network.h"
#include "../serializer.h"
#include "../kernel/api.h"

#include <driver/gxm/gxm_types.h>

struct ProgramHeader {
    uint8_t magic[4];
    uint32_t version;
    uint32_t length;
};

struct FileHeader {
    uint64_t magic;
    uint32_t version;
    uint32_t headerLength;
    char progVersion[16];
};

struct BinaryThumbnail
{
  uint16_t width;
  uint16_t height;
  uint32_t length;
  uint8_t data[1];
};

struct CaptureMetaData
{
  uint64_t machineIdent;
  uint32_t driverID;
  uint8_t driverNameLength;
  char driverName[1];
} captureMetaData = 
{
    0,
    0,
    1,
    {0}
};

struct BinarySectionHeader
{
  uint8_t isASCII;
  uint8_t zero[3];
  uint32_t sectionType;
  uint64_t sectionCompressedLength;
  uint64_t sectionUncompressedLength;
  uint64_t sectionVersion;
  uint32_t sectionFlags;
  uint32_t sectionNameLength;
  char name[1];
};

static char g_titleid[16];
int g_capture = 0;
int g_log = 0;
int g_log_display = 0;
File g_file;
std::atomic<int64_t> g_resourceid;

uint64_t g_fileoffset = 0;
void* g_framebuffers[64] = {};
uint32_t g_framebufferCount = 0;

void* g_baseFrame;

uint32_t g_frameCount = 0;
uint32_t g_logFrameCount = 0;
uint32_t g_displayWaitCount = 0;
void* g_baseAddr = NULL;

SceDisplayFrameBuf g_framebuffer;

struct MappedMemory
{
    GXMBufferType bufferType;
    const void* addr;
    uint32_t size;
    uint32_t referenced;
    int64_t resId;
};

#define MAX_MAPPED_MEMORY_COUNT 1024
MappedMemory* g_memoryArray = NULL;
uint32_t g_mappedMemoryCount = 0;

const SceGxmVertexProgram * g_activeVertexProgram = NULL;
struct VertexStream
{
    const void* dataPtr;
    uint32_t stride;
    uint32_t indexSource;
};

VertexStream g_activeVertexStreams[SCE_GXM_MAX_VERTEX_STREAMS] = { 0 };
uint32_t g_activeStreamCount = 0;

const uint32_t ALIGNMENT = 64;
const uint8_t g_alignmentBytes[ALIGNMENT] = { 0 };

#define ALIGN_TO_64(bytes) \
    g_file.write(g_alignmentBytes, (ALIGNMENT - ((bytes) % ALIGNMENT)) % ALIGNMENT);

#define PRINT_LOG

#undef LOGD
#ifdef PRINT_LOG
#define LOGD(...) \
if (g_log) \
{ \
    LOG(__VA_ARGS__); \
}
#else
#define LOGD(...)
#endif

typedef struct sce_module_exports {
  uint16_t size;           // size of this structure; 0x20 for Vita 1.x
  uint8_t  lib_version[2]; //
  uint16_t attribute;      // ?
  uint16_t num_functions;  // number of exported functions
  uint16_t num_vars;       // number of exported variables
  uint16_t unk;
  uint32_t num_tls_vars;   // number of exported TLS variables?  <-- pretty sure wrong // yifanlu
  uint32_t lib_nid;        // NID of this specific export list; one PRX can export several names
  char     *lib_name;      // name of the export module
  uint32_t *nid_table;     // array of 32-bit NIDs for the exports, first functions then vars
  void     **entry_table;  // array of pointers to exported functions and then variables
} sce_module_exports_t;

struct sce_module_imports_1 {
  uint16_t size;               // size of this structure; 0x34
  uint16_t version;            //
  uint16_t flags;              //
  uint16_t num_functions;      // number of imported functions
  uint16_t num_vars;           // number of imported variables
  uint16_t num_tls_vars;       // number of imported TLS variables
  uint32_t reserved1;          // ?
  uint32_t lib_nid;            // NID of the module to link to
  char     *lib_name;          // name of module
  uint32_t reserved2;          // ?
  uint32_t *func_nid_table;    // array of function NIDs (numFuncs)
  void     **func_entry_table; // parallel array of pointers to stubs; they're patched by the loader to jump to the final code
  uint32_t *var_nid_table;     // NIDs of the imported variables (numVars)
  void     **var_entry_table;  // array of pointers to "ref tables" for each variable
  uint32_t *tls_nid_table;     // NIDs of the imported TLS variables (numTlsVars)
  void     **tls_entry_table;  // array of pointers to ???
};

struct sce_module_imports_2 {
  uint16_t size; // 0x24
  uint16_t version;
  uint16_t flags;
  uint16_t num_functions;
  uint32_t reserved1;
  uint32_t lib_nid;
  char     *lib_name;
  uint32_t *func_nid_table;
  void     **func_entry_table;
  uint32_t unk1;
  uint32_t unk2;
};

typedef union sce_module_imports {
  uint16_t size;
  struct sce_module_imports_1 type1;
  struct sce_module_imports_2 type2;
} sce_module_imports_t;

typedef struct SceGxmCommandList {
    uint32_t data[8];
} SceGxmCommandList;

typedef enum SceGxmContextType {
    SCE_GXM_CONTEXT_TYPE_IMMEDIATE,
    SCE_GXM_CONTEXT_TYPE_DEFERRED
} SceGxmContextType;

typedef enum SceGxmWarning {
    SCE_GXM_WARNING_SCENE_SPLIT,
    SCE_GXM_WARNING_VERTEX_DEFAULT_UNIFORM_BUFFER_RECYCLED,
    SCE_GXM_WARNING_FRAGMENT_DEFAULT_UNIFORM_BUFFER_RECYCLED,
    SCE_GXM_WARNING_STREAMS_PROVIDED_WITH_ZERO_COUNT,
    SCE_GXM_WARNING_ATTRIBUTES_PROVIDED_WITH_ZERO_COUNT,
    SCE_GXM_WARNING_PROGRAM_REGISTERED_WITH_SHADER_PATCHER,
    SCE_GXM_WARNING_BLEND_INFO_IGNORED_FOR_NATIVECOLOR,
    SCE_GXM_WARNING_USING_INTERPOLANT_NOT_WRITTEN_BY_VERTEX_PROGRAM,
    SCE_GXM_WARNING_DEPTH_STENCIL_SURFACE_SETTING_IGNORED,
    SCE_GXM_WARNING_DEFERRED_CONTEXT_MISSING_VIEWPORT,
    SCE_GXM_WARNING_DEFERRED_CONTEXT_MISSING_REGION_CLIP,
    SCE_GXM_WARNING_USING_INAPPROPRIATE_MEMORY_CACHE_CONFIGURATION,
    SCE_GXM_WARNING_PRECOMPUTING_DISABLED_FRAGMENT_STATE
} SceGxmWarning;

typedef void (*disp_callback_t)(const void* test);
static disp_callback_t g_displaycallback;
void display_queue_callback_patched(const void* callbackdata) {
    if (g_displaycallback) g_displaycallback(callbackdata);
}

#define CREATE_PATCHED_CALL(return_type, name, ...)  \
static tai_hook_ref_t name##Ref;                     \
static SceUID name##Hook;                            \
static return_type name##Patched(__VA_ARGS__)

#define CHUNK_SIZE 2048 // Screenshot buffer size in bytes
static uint8_t sshot_buffer[CHUNK_SIZE];

static tai_hook_ref_t sceDisplaySetFrameBufRef;
static SceUID sceDisplaySetFrameBufHook;
static int sceDisplaySetFrameBufPatched(SceDisplayFrameBuf* frameBuf, SceDisplaySetBufSync bufSync) {

    if (g_log_display) {
        // Opening screenshot output file
		SceDateTime time;
		sceRtcGetCurrentClockLocalTime(&time);
        char fname[256];
		sceClibSnprintf(fname, 256, "ux0:/data/renderdoc/%s-%d-%d-%d-%d-%d-%d.bmp", "renderdoc", time.year, time.month, time.day, time.hour, time.minute, time.second);
		SceUID fd = sceIoOpen(fname, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 0777);
		
		// Writing Bitmap Header
		memset(sshot_buffer, 0, 0x36);
		*((uint16_t*)(&sshot_buffer[0x00])) = 0x4D42;
		*((uint32_t*)(&sshot_buffer[0x02])) = ((frameBuf->width * frameBuf->height)<<2) + 0x36;
		*((uint32_t*)(&sshot_buffer[0x0A])) = 0x36;
		*((uint32_t*)(&sshot_buffer[0x0E])) = 0x28;
		*((uint32_t*)(&sshot_buffer[0x12])) = frameBuf->width;
		*((uint32_t*)(&sshot_buffer[0x16])) = frameBuf->height;
		*((uint32_t*)(&sshot_buffer[0x1A])) = 0x00200001;
		*((uint32_t*)(&sshot_buffer[0x22])) = ((frameBuf->width * frameBuf->height) << 2);
		sceIoWrite(fd, sshot_buffer, 0x36);
		
		// Writing Bitmap Table
		uint32_t x, y, i;
		i = 0;
		uint32_t* buffer = (uint32_t*)sshot_buffer;
		uint32_t* framedata = (uint32_t*)frameBuf->base;
		for (y = 1; y <= frameBuf->height; y++){
			for (x = 0; x < frameBuf->width; x++){
				buffer[i] = framedata[x + (frameBuf->height - y) * frameBuf->pitch];
				uint8_t* clr = (uint8_t*)&buffer[i];
				uint8_t r = clr[0];
				uint8_t g = clr[1];
				uint8_t b = clr[2];
				uint8_t a = clr[3];
				buffer[i] = (a << 24) | (r << 16) | (g << 8) | b;
				i++;
				if (i == (CHUNK_SIZE >> 2)){
					i = 0;
					sceIoWrite(fd, sshot_buffer, CHUNK_SIZE);
				}
			}
		}
		if (i != 0) sceIoWrite(fd, sshot_buffer, i << 2);
		
		// Saving file
		sceIoClose(fd);

        g_log_display = 0;
    }

    if (frameBuf) {
        g_framebuffer = *frameBuf;
    }
    else {
        memset(&g_framebuffer, 0, sizeof(SceDisplayFrameBuf));
    }
    

    return TAI_NEXT(sceDisplaySetFrameBuf, sceDisplaySetFrameBufRef, frameBuf, bufSync);
}

//static tai_hook_ref_t sceDisplayWaitVblankStartRef;
//static SceUID sceDisplayWaitVblankStartHook;
//static int sceDisplayWaitVblankStartPatched() {

//    int ret = TAI_NEXT(int, sceDisplayWaitVblankStartRef);

    //LOGD("sceDisplayWaitVblankStart() called\n");
//    return ret;
//}

typedef int(*threadfunc_t)(SceSize args, void* init);
static SceUID serverThreadId;
static int sRemoteControlThreadInit(SceSize args, void *init);
static SceUID createThread(const char *thread_name, threadfunc_t thread_func, size_t user_data_size, void* user_data);

static tai_hook_ref_t sceNetInitRef;
static SceUID sceNetInitHook;
static int sceNetInitPatched(SceNetInitParam* param) {

    int ret = 0;//TAI_NEXT(int, sceNetInitRef, param);
    LOG("sceNetInit(param: %p) called\n", param);
   
    return ret;
}

#define MY_TAI_MAIN_MODULE ((const char*)0)

static uint8_t networkbuffer[16 * 1024];

static tai_hook_ref_t sceSysmoduleLoadModuleRef;
static SceUID sceSysmoduleLoadModuleHook;
static int sceSysmoduleLoadModulePatched(SceSysmoduleModuleId id) {

    int ret = TAI_NEXT(sceSysmoduleLoadModule, sceSysmoduleLoadModuleRef, id);

    LOG("sceSysmoduleLoadModule(id: %s) called with ret: %" PRIi32 "\n", sysmodule2str(id), ret);

    if (ret >= 0 && SCE_SYSMODULE_NET == id) {

        sceNetInitHook = taiHookFunctionImport(&sceNetInitRef, MY_TAI_MAIN_MODULE, 0x6BF8B2A2, 0xEB03E265, (const void*)&sceNetInitPatched);
        if (sceNetInitHook < 0) {
            LOG("Could not hook sceNetInit, reason: %s\n", taihenerr2str(sceNetInitHook)); 
        }
        else {
            LOG("hooked sceNetInit\n");
        }
/*
        SceUID modlist[256];
        int numMods = 256;
        int res = sceKernelGetModuleList(0x7FFFFFFF, modlist, &numMods);
        if (res < 0) LOG("Could not get module list\n"); 
        else {
            LOG("Got module list with %" PRIi32 " modules\n", numMods);
            for (int index = 0; index < numMods; ++index) {
                SceKernelModuleInfo modInfo = {};
                res = sceKernelGetModuleInfo(modlist[index], &modInfo);
                if (res >= 0) {
                    LOG("modid: %" PRIi32", mod name: %s, mod version: %" PRIu8 ".%" PRIu8 "\n", modlist[index], modInfo.module_name, ((uint8_t*)(&modInfo.flags))[3], ((uint8_t*)(&modInfo.flags))[2]);
                    
                    tai_module_info_t taiModInfo = {};
                    taiModInfo.size = sizeof(tai_module_info_t);
                    res = taiGetModuleInfo(modInfo.module_name, &taiModInfo);
                    if (res < 0) LOG("Could not get module info, reason: %s\n", taihenerr2str(res)); else LOG("got module info\n");

                    for (uintptr_t cur = taiModInfo.imports_start; cur < taiModInfo.imports_end; ) {
                        sce_module_imports_t *import = (sce_module_imports_t*)cur;
                        if (import->size == sizeof(struct sce_module_imports_1)) {
                            LOG("import Lib: %s, Lib Nid: 0x%08X\n", import->type1.lib_name, import->type1.lib_nid);
                            for (uint16_t i = 0; i < import->type1.num_functions; ++i) {
                                LOG("\timport Func Nid: 0x%08X\n", import->type1.func_nid_table[i]);
                            }
                        }
                        else if (import->size == sizeof(struct sce_module_imports_2)) {
                            LOG("import Lib: %s, Lib Nid: 0x%08X\n", import->type2.lib_name, import->type2.lib_nid);
                            for (uint16_t i = 0; i < import->type2.num_functions; ++i) {
                                LOG("\timport Func Nid: 0x%08X\n", import->type2.func_nid_table[i]);
                            }
                        }
                        cur += import->size;
                    }

                    for (uintptr_t cur = taiModInfo.exports_start; cur < taiModInfo.exports_start; ) {
                        sce_module_exports_t *export = (sce_module_exports_t*)cur;
                        LOG("export Lib: %s, Lib Nid: 0x%08X\n", export->lib_name, export->lib_nid);
                        for (uint16_t i = 0; i < export->num_functions; ++i) {
                            LOG("\texport Func Nid: 0x%08X\n", export->nid_table[i]);
                        }
                        
                        cur += export->size;
                    }
                }
            }
        }
                    */
    }

    return ret;
}

static tai_hook_ref_t sceSysmoduleUnloadModuleRef;
static SceUID sceSysmoduleUnloadModuleHook;
static int sceSysmoduleUnloadModulePatched(SceSysmoduleModuleId id) {

    int ret = TAI_NEXT(sceSysmoduleUnloadModule, sceSysmoduleUnloadModuleRef, id);
    LOGD("sceSysmoduleUnloadModule(id: %s) called with ret: %" PRIi32 "\n", sysmodule2str(id), ret);


   // if (ret >= 0 && SCE_SYSMODULE_NET == id) {
    //    taiHookRelease(sceNetInitHook, sceNetInitRef);
    //}

    return ret;
}

static tai_hook_ref_t scePowerSetUsingWirelessRef;
static SceUID scePowerSetUsingWirelessHook;
static int scePowerSetUsingWirelessPatched(int enable) {
    enable = 1;
    int ret = TAI_NEXT(scePowerSetUsingWireless, scePowerSetUsingWirelessRef, enable);

    LOGD("scePowerSetUsingWireless(enable: %" PRIi32 ") called with ret: %" PRIi32 "\n", enable, ret);

    return ret;
}

static tai_hook_ref_t scePowerSetConfigurationModeRef;
static SceUID scePowerSetConfigurationModeHook;
int scePowerSetConfigurationModePatched(int mode) {
    int ret = 0;//TAI_NEXT(int, scePowerSetConfigurationModeRef, mode);

    LOGD("scePowerSetConfigurationMode(mode: %" PRIi32 ") called with ret: %" PRIi32 "\n", mode, ret);

    return ret;
}

static tai_hook_ref_t sceKernelAllocMemBlockRef;
static SceUID sceKernelAllocMemBlockHook;
SceUID sceKernelAllocMemBlockPatched(const char *name, SceKernelMemBlockType type, int size, SceKernelAllocMemBlockOpt *optp) {
    SceUID res = TAI_NEXT(sceKernelAllocMemBlock, sceKernelAllocMemBlockRef, name, type, size, optp);
    LOG("sceKernelAllocMemBlock(ret SceUID: %" PRIi32 ", name: %s, type: %s, size: %" PRIi32 ")\n", res, name, kernelmemblock2str(type), size);
    return res;
}

static tai_hook_ref_t sceKernelGetMemBlockBaseRef;
static SceUID sceKernelGetMemBlockBaseHook;
int sceKernelGetMemBlockBasePatched(SceUID uid, void **basep) {
    int res = TAI_NEXT(sceKernelGetMemBlockBase, sceKernelGetMemBlockBaseRef, uid, basep);
    //LOG("sceKernelGetMemBlockBase(uid: %" PRIi32 ", base pointer: %p)\n", uid, *basep);
    return res;
}

#define IMPORT_HOOK(name, nid) \
    do { \
        name##Hook = taiHookFunctionImport(&name##Ref, MY_TAI_MAIN_MODULE, 0xF76B66BD, nid, (const void*)&name##Patched); \
        if (name##Hook < 0) { LOGD("Could not hook " #name "\n"); } \
    } while(0)

CREATE_PATCHED_CALL(void *, sceGxmTextureGetData, const SceGxmTexture *texture)
{
    
    LOGD("sceGxmTextureGetData(texture: %p)\n", texture);
    return TAI_NEXT(sceGxmTextureGetData, sceGxmTextureGetDataRef, texture);
}

CREATE_PATCHED_CALL(SceGxmTextureFormat, sceGxmTextureGetFormat, const SceGxmTexture *texture)
{
    LOGD("sceGxmTextureGetFormat(texture: %p)\n", texture);
    return TAI_NEXT(sceGxmTextureGetFormat, sceGxmTextureGetFormatRef, texture);
}

CREATE_PATCHED_CALL(SceGxmTextureGammaMode, sceGxmTextureGetGammaMode, const SceGxmTexture *texture)
{
    LOGD("sceGxmTextureGetGammaMode(texture: %p)\n", texture);
    return TAI_NEXT(sceGxmTextureGetGammaMode, sceGxmTextureGetGammaModeRef, texture);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmTextureGetHeight, const SceGxmTexture *texture)
{
    LOGD("sceGxmTextureGetHeight(texture: %p)\n", texture);
    return TAI_NEXT(sceGxmTextureGetHeight, sceGxmTextureGetHeightRef, texture);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmTextureGetLodBias, const SceGxmTexture *texture)
{
    LOGD("sceGxmTextureGetLodBias(texture: %p)\n", texture);
    return TAI_NEXT(sceGxmTextureGetLodBias, sceGxmTextureGetLodBiasRef, texture);
}

unsigned int sceGxmTextureGetLodMin(const SceGxmTexture *texture);
CREATE_PATCHED_CALL(unsigned int, sceGxmTextureGetLodMin, const SceGxmTexture *texture)
{
    LOGD("sceGxmTextureGetLodMin(texture: %p)\n", texture);
    return TAI_NEXT(sceGxmTextureGetLodMin, sceGxmTextureGetLodMinRef, texture);
}

CREATE_PATCHED_CALL(SceGxmTextureFilter, sceGxmTextureGetMagFilter, const SceGxmTexture *texture)
{
    LOGD("sceGxmTextureGetMagFilter(texture: %p)\n", texture);
    return TAI_NEXT(sceGxmTextureGetMagFilter, sceGxmTextureGetMagFilterRef, texture);
}

CREATE_PATCHED_CALL(SceGxmTextureFilter, sceGxmTextureGetMinFilter, const SceGxmTexture *texture)
{
    LOGD("sceGxmTextureGetMinFilter(texture: %p)\n", texture);
    return TAI_NEXT(sceGxmTextureGetMinFilter, sceGxmTextureGetMinFilterRef, texture);
}

CREATE_PATCHED_CALL(SceGxmTextureMipFilter, sceGxmTextureGetMipFilter, const SceGxmTexture *texture)
{
    LOGD("sceGxmTextureGetMipFilter(texture: %p)\n", texture);
    return TAI_NEXT(sceGxmTextureGetMipFilter, sceGxmTextureGetMipFilterRef, texture);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmTextureGetMipmapCount, const SceGxmTexture *texture)
{
    LOGD("sceGxmTextureGetMipmapCount(texture: %p)\n", texture);
    return TAI_NEXT(sceGxmTextureGetMipmapCount, sceGxmTextureGetMipmapCountRef, texture);
}

unsigned int sceGxmTextureGetMipmapCountUnsafe(const SceGxmTexture *texture);
CREATE_PATCHED_CALL(unsigned int, sceGxmTextureGetMipmapCountUnsafe, const SceGxmTexture *texture)
{
    LOGD("sceGxmTextureGetMipmapCountUnsafe(texture: %p)\n", texture);
    return TAI_NEXT(sceGxmTextureGetMipmapCountUnsafe, sceGxmTextureGetMipmapCountUnsafeRef, texture);
}

SceGxmTextureNormalizeMode sceGxmTextureGetNormalizeMode(const SceGxmTexture *texture);
CREATE_PATCHED_CALL(SceGxmTextureNormalizeMode, sceGxmTextureGetNormalizeMode, const SceGxmTexture *texture)
{
    LOGD("sceGxmTextureGetNormalizeMode(texture: %p)\n", texture);
    return TAI_NEXT(sceGxmTextureGetNormalizeMode, sceGxmTextureGetNormalizeModeRef, texture);
}

CREATE_PATCHED_CALL(void *, sceGxmTextureGetPalette, const SceGxmTexture *texture)
{
    LOGD("sceGxmTextureGetPalette(texture: %p)\n", texture);
    return TAI_NEXT(sceGxmTextureGetPalette, sceGxmTextureGetPaletteRef, texture);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmTextureGetStride, const SceGxmTexture *texture)
{
    LOGD("sceGxmTextureGetStride(texture: %p)\n", texture);
    return TAI_NEXT(sceGxmTextureGetStride, sceGxmTextureGetStrideRef, texture);
}

CREATE_PATCHED_CALL(SceGxmTextureType, sceGxmTextureGetType, const SceGxmTexture *texture)
{
    LOGD("sceGxmTextureGetType(texture: %p)\n", texture);
    return TAI_NEXT(sceGxmTextureGetType, sceGxmTextureGetTypeRef, texture);
}

CREATE_PATCHED_CALL(SceGxmTextureAddrMode, sceGxmTextureGetUAddrMode, const SceGxmTexture *texture)
{
    LOGD("sceGxmTextureGetUAddrMode(texture: %p)\n", texture);
    return TAI_NEXT(sceGxmTextureGetUAddrMode, sceGxmTextureGetUAddrModeRef, texture);
}

SceGxmTextureAddrMode sceGxmTextureGetUAddrModeSafe(const SceGxmTexture *texture);
CREATE_PATCHED_CALL(SceGxmTextureAddrMode, sceGxmTextureGetUAddrModeSafe, const SceGxmTexture *texture)
{
    LOGD("sceGxmTextureGetUAddrModeSafe(texture: %p)\n", texture);
    return TAI_NEXT(sceGxmTextureGetUAddrModeSafe, sceGxmTextureGetUAddrModeSafeRef, texture);
}

CREATE_PATCHED_CALL(SceGxmTextureAddrMode, sceGxmTextureGetVAddrMode, const SceGxmTexture *texture)
{
    LOGD("sceGxmTextureGetVAddrMode(texture: %p)\n", texture);
    return TAI_NEXT(sceGxmTextureGetVAddrMode, sceGxmTextureGetVAddrModeRef, texture);
}

SceGxmTextureAddrMode sceGxmTextureGetVAddrModeSafe(const SceGxmTexture *texture);
CREATE_PATCHED_CALL(SceGxmTextureAddrMode, sceGxmTextureGetVAddrModeSafe, const SceGxmTexture *texture)
{
    LOGD("sceGxmTextureGetVAddrModeSafe(texture: %p)\n", texture);
    return TAI_NEXT(sceGxmTextureGetVAddrModeSafe, sceGxmTextureGetVAddrModeSafeRef, texture);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmTextureGetWidth, const SceGxmTexture *texture)
{
    LOGD("sceGxmTextureGetWidth(texture: %p)\n", texture);
    return TAI_NEXT(sceGxmTextureGetWidth, sceGxmTextureGetWidthRef, texture);
}

CREATE_PATCHED_CALL(int, sceGxmProgramCheck, const SceGxmProgram *program)
{
    LOGD("sceGxmProgramCheck(program: %p)\n", program);
    return TAI_NEXT(sceGxmProgramCheck, sceGxmProgramCheckRef, program);
}

CREATE_PATCHED_CALL(const SceGxmProgramParameter *, sceGxmProgramFindParameterByName, const SceGxmProgram *program, const char *name)
{
    LOGD("sceGxmProgramFindParameterByName(program: %p, name: %s)\n", program, name);
    return TAI_NEXT(sceGxmProgramFindParameterByName, sceGxmProgramFindParameterByNameRef, program, name);
}

CREATE_PATCHED_CALL(const SceGxmProgramParameter *, sceGxmProgramFindParameterBySemantic, const SceGxmProgram *program, SceGxmParameterSemantic semantic, unsigned int index)
{
    LOGD("sceGxmProgramFindParameterBySemantic(program: %p, semantic: %" PRIu32 ", index: %" PRIu32 ")\n", program, semantic, index);
    return TAI_NEXT(sceGxmProgramFindParameterBySemantic, sceGxmProgramFindParameterBySemanticRef, program, semantic, index);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmProgramGetDefaultUniformBufferSize, const SceGxmProgram *program)
{
    LOGD("sceGxmProgramGetDefaultUniformBufferSize(program: %p)\n", program);
    return TAI_NEXT(sceGxmProgramGetDefaultUniformBufferSize, sceGxmProgramGetDefaultUniformBufferSizeRef, program);
}

unsigned int sceGxmProgramGetFragmentProgramInputs(const SceGxmProgram *program);
CREATE_PATCHED_CALL(unsigned int, sceGxmProgramGetFragmentProgramInputs, const SceGxmProgram *program)
{
    LOGD("sceGxmProgramGetFragmentProgramInputs(program: %p)\n", program);
    return TAI_NEXT(sceGxmProgramGetFragmentProgramInputs, sceGxmProgramGetFragmentProgramInputsRef, program);
}

unsigned int sceGxmProgramGetOutputRegisterFormat(const SceGxmProgram *program, SceGxmParameterType *type, unsigned int *componentCount);
CREATE_PATCHED_CALL(unsigned int, sceGxmProgramGetOutputRegisterFormat, const SceGxmProgram *program, SceGxmParameterType *type, unsigned int *componentCount)
{
    LOGD("sceGxmProgramGetOutputRegisterFormat(program: %p, type: %p, componentCount: %p)\n", program, type, componentCount);
    return TAI_NEXT(sceGxmProgramGetOutputRegisterFormat, sceGxmProgramGetOutputRegisterFormatRef, program, type, componentCount);
}

CREATE_PATCHED_CALL(const SceGxmProgramParameter *, sceGxmProgramGetParameter, const SceGxmProgram *program, unsigned int index)
{
    LOGD("sceGxmProgramGetParameter(program: %p, index: %" PRIu32 ")\n", program, index);
    return TAI_NEXT(sceGxmProgramGetParameter, sceGxmProgramGetParameterRef, program, index);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmProgramGetParameterCount, const SceGxmProgram *program)
{
    LOGD("sceGxmProgramGetParameterCount(program: %p)\n", program);
    return TAI_NEXT(sceGxmProgramGetParameterCount, sceGxmProgramGetParameterCountRef, program);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmProgramGetSize, const SceGxmProgram *program)
{
    LOGD("sceGxmProgramGetSize(program: %p)\n", program);
    return TAI_NEXT(sceGxmProgramGetSize, sceGxmProgramGetSizeRef, program);
}

CREATE_PATCHED_CALL(SceGxmProgramType, sceGxmProgramGetType, const SceGxmProgram *program)
{
    LOGD("sceGxmProgramGetType(program: %p)\n", program);
    return TAI_NEXT(sceGxmProgramGetType, sceGxmProgramGetTypeRef, program);
}

unsigned int sceGxmProgramGetVertexProgramOutputs(const SceGxmProgram *program);
CREATE_PATCHED_CALL(unsigned int, sceGxmProgramGetVertexProgramOutputs, const SceGxmProgram *program)
{
    LOGD("sceGxmProgramGetVertexProgramOutputs(program: %p)\n", program);
    return TAI_NEXT(sceGxmProgramGetVertexProgramOutputs, sceGxmProgramGetVertexProgramOutputsRef, program);
}

CREATE_PATCHED_CALL(SceBool, sceGxmProgramIsDepthReplaceUsed, const SceGxmProgram *program)
{
    LOGD("sceGxmProgramIsDepthReplaceUsed(program: %p)\n", program);
    return TAI_NEXT(sceGxmProgramIsDepthReplaceUsed, sceGxmProgramIsDepthReplaceUsedRef, program);
}

CREATE_PATCHED_CALL(SceBool, sceGxmProgramIsDiscardUsed, const SceGxmProgram *program)
{
    LOGD("sceGxmProgramIsDiscardUsed(program: %p)\n", program);
    return TAI_NEXT(sceGxmProgramIsDiscardUsed, sceGxmProgramIsDiscardUsedRef, program);
}

SceBool sceGxmProgramIsEquivalent(const SceGxmProgram *programA, const SceGxmProgram *programB);
CREATE_PATCHED_CALL(SceBool, sceGxmProgramIsEquivalent, const SceGxmProgram *programA, const SceGxmProgram *programB)
{
    LOGD("sceGxmProgramIsEquivalent(programA: %p, programB: %p)\n", programA, programB);
    return TAI_NEXT(sceGxmProgramIsEquivalent, sceGxmProgramIsEquivalentRef, programA, programB);
}

SceBool sceGxmProgramIsFragColorUsed(const SceGxmProgram *program);
CREATE_PATCHED_CALL(SceBool, sceGxmProgramIsFragColorUsed, const SceGxmProgram *program)
{
    LOGD("sceGxmProgramIsFragColorUsed(program: %p)\n", program);
    return TAI_NEXT(sceGxmProgramIsFragColorUsed, sceGxmProgramIsFragColorUsedRef, program);
}

SceBool sceGxmProgramIsNativeColorUsed(const SceGxmProgram *program);
CREATE_PATCHED_CALL(SceBool, sceGxmProgramIsNativeColorUsed, const SceGxmProgram *program)
{
    LOGD("sceGxmProgramIsNativeColorUsed(program: %p)\n", program);
    return TAI_NEXT(sceGxmProgramIsNativeColorUsed, sceGxmProgramIsNativeColorUsedRef, program);
}

CREATE_PATCHED_CALL(SceBool, sceGxmProgramIsSpriteCoordUsed, const SceGxmProgram *program)
{
    LOGD("sceGxmProgramIsSpriteCoordUsed(program: %p)\n", program);
    return TAI_NEXT(sceGxmProgramIsSpriteCoordUsed, sceGxmProgramIsSpriteCoordUsedRef, program);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmProgramParameterGetArraySize, const SceGxmProgramParameter *parameter)
{
    LOGD("sceGxmProgramParameterGetArraySize(parameter: %p)\n", parameter);
    return TAI_NEXT(sceGxmProgramParameterGetArraySize, sceGxmProgramParameterGetArraySizeRef, parameter);
}

CREATE_PATCHED_CALL(SceGxmParameterCategory, sceGxmProgramParameterGetCategory, const SceGxmProgramParameter *parameter)
{
    LOGD("sceGxmProgramParameterGetCategory(parameter: %p)\n", parameter);
    return TAI_NEXT(sceGxmProgramParameterGetCategory, sceGxmProgramParameterGetCategoryRef, parameter);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmProgramParameterGetComponentCount, const SceGxmProgramParameter *parameter)
{
    LOGD("sceGxmProgramParameterGetComponentCount(parameter: %p)\n", parameter);
    return TAI_NEXT(sceGxmProgramParameterGetComponentCount, sceGxmProgramParameterGetComponentCountRef, parameter);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmProgramParameterGetContainerIndex, const SceGxmProgramParameter *parameter)
{
    LOGD("sceGxmProgramParameterGetContainerIndex(parameter: %p)\n", parameter);
    return TAI_NEXT(sceGxmProgramParameterGetContainerIndex, sceGxmProgramParameterGetContainerIndexRef, parameter);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmProgramParameterGetIndex, const SceGxmProgram *program, const SceGxmProgramParameter *parameter)
{
    LOGD("sceGxmProgramParameterGetIndex(program: %p, parameter: %p)\n", program, parameter);
    return TAI_NEXT(sceGxmProgramParameterGetIndex, sceGxmProgramParameterGetIndexRef, program, parameter);
}

CREATE_PATCHED_CALL(const char *, sceGxmProgramParameterGetName, const SceGxmProgramParameter *parameter)
{
    LOGD("sceGxmProgramParameterGetName(parameter: %p)\n", parameter);
    return TAI_NEXT(sceGxmProgramParameterGetName, sceGxmProgramParameterGetNameRef, parameter);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmProgramParameterGetResourceIndex, const SceGxmProgramParameter *parameter)
{
    LOGD("sceGxmProgramParameterGetResourceIndex(parameter: %p)\n", parameter);
    return TAI_NEXT(sceGxmProgramParameterGetResourceIndex, sceGxmProgramParameterGetResourceIndexRef, parameter);
}

CREATE_PATCHED_CALL(SceGxmParameterSemantic, sceGxmProgramParameterGetSemantic, const SceGxmProgramParameter *parameter)
{
    LOGD("sceGxmProgramParameterGetSemantic(parameter: %p)\n", parameter);
    return TAI_NEXT(sceGxmProgramParameterGetSemantic, sceGxmProgramParameterGetSemanticRef, parameter);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmProgramParameterGetSemanticIndex, const SceGxmProgramParameter *parameter)
{
    LOGD("sceGxmProgramParameterGetSemanticIndex(parameter: %p)\n", parameter);
    return TAI_NEXT(sceGxmProgramParameterGetSemanticIndex, sceGxmProgramParameterGetSemanticIndexRef, parameter);
}

CREATE_PATCHED_CALL(SceGxmParameterType, sceGxmProgramParameterGetType, const SceGxmProgramParameter *parameter)
{
    LOGD("sceGxmProgramParameterGetType(parameter: %p)\n", parameter);
    return TAI_NEXT(sceGxmProgramParameterGetType, sceGxmProgramParameterGetTypeRef, parameter);
}

SceBool sceGxmProgramParameterIsRegFormat(const SceGxmProgram *program, const SceGxmProgramParameter *parameter);
CREATE_PATCHED_CALL(SceBool, sceGxmProgramParameterIsRegFormat, const SceGxmProgram *program, const SceGxmProgramParameter *parameter)
{
    LOGD("sceGxmProgramParameterIsRegFormat(program: %p, parameter: %p)\n", program, parameter);
    return TAI_NEXT(sceGxmProgramParameterIsRegFormat, sceGxmProgramParameterIsRegFormatRef, program, parameter);
}

CREATE_PATCHED_CALL(SceBool, sceGxmProgramParameterIsSamplerCube, const SceGxmProgramParameter *parameter)
{
    LOGD("sceGxmProgramParameterIsSamplerCube(parameter: %p)\n", parameter);
    return TAI_NEXT(sceGxmProgramParameterIsSamplerCube, sceGxmProgramParameterIsSamplerCubeRef, parameter);
}

int sceGxmAddRazorGpuCaptureBuffer(void* base, unsigned int size);
CREATE_PATCHED_CALL(int, sceGxmAddRazorGpuCaptureBuffer, void* base, unsigned int size)
{
    LOGD("sceGxmAddRazorGpuCaptureBuffer(base: %p, size: %" PRIu32 ")\n", base, size);
    return TAI_NEXT(sceGxmAddRazorGpuCaptureBuffer, sceGxmAddRazorGpuCaptureBufferRef, base, size);
}

int sceGxmBeginCommandList(SceGxmContext *deferredContext);
CREATE_PATCHED_CALL(int, sceGxmBeginCommandList, SceGxmContext *deferredContext)
{
    LOGD("sceGxmBeginCommandList(deferredContext: %p)\n", deferredContext);
    return TAI_NEXT(sceGxmBeginCommandList, sceGxmBeginCommandListRef, deferredContext);
}

CREATE_PATCHED_CALL(int, sceGxmBeginScene, SceGxmContext *context, unsigned int flags, const SceGxmRenderTarget *renderTarget, const SceGxmValidRegion *validRegion, SceGxmSyncObject *vertexSyncObject, SceGxmSyncObject *fragmentSyncObject, const SceGxmColorSurface *colorSurface, const SceGxmDepthStencilSurface *depthStencil)
{
    if (g_log) {

        uint32_t chunkSize = 0;

        GXMChunk type = GXMChunk::sceGxmBeginScene;
        g_fileoffset += g_file.write(type);
        g_fileoffset += g_file.write(chunkSize);
        g_fileoffset += g_file.write(context);
        g_fileoffset += g_file.write(flags);
        g_fileoffset += g_file.write(renderTarget);
        g_fileoffset += g_file.write(validRegion);
        g_fileoffset += g_file.write(vertexSyncObject);
        g_fileoffset += g_file.write(fragmentSyncObject);
        g_fileoffset += g_file.write(colorSurface);
        g_fileoffset += g_file.write(depthStencil);

        g_fileoffset += ALIGN_TO_64(g_fileoffset);
    }

    LOGD("sceGxmBeginScene(context: %p, flags: %d, rendertarget: %p, validRegion: %p, vertexSyncObject: %p, fragmentSyncObject: %p, colorSurface: %p, depthStencil: %p)\n", context, flags, renderTarget, validRegion, vertexSyncObject, fragmentSyncObject, colorSurface, depthStencil);
    return TAI_NEXT(sceGxmBeginScene, sceGxmBeginSceneRef, context, flags, renderTarget, validRegion, vertexSyncObject, fragmentSyncObject, colorSurface, depthStencil);
}

int sceGxmBeginSceneEx(SceGxmContext *context, unsigned int flags, const SceGxmRenderTarget *renderTarget, const SceGxmValidRegion *validRegion, SceGxmSyncObject *vertexSyncObject, SceGxmSyncObject *fragmentSyncObject, const SceGxmColorSurface *colorSurface, const SceGxmDepthStencilSurface *loadDepthStencil, const SceGxmDepthStencilSurface *storeDepthStencil);
CREATE_PATCHED_CALL(int, sceGxmBeginSceneEx, SceGxmContext *context, unsigned int flags, const SceGxmRenderTarget *renderTarget, const SceGxmValidRegion *validRegion, SceGxmSyncObject *vertexSyncObject, SceGxmSyncObject *fragmentSyncObject, const SceGxmColorSurface *colorSurface, const SceGxmDepthStencilSurface *loadDepthStencil, const SceGxmDepthStencilSurface *storeDepthStencil)
{
    LOGD("sceGxmBeginSceneEx(context: %p, flags: %d, rendertarget: %p, validRegion: %p, vertexSyncObject: %p, fragmentSyncObject: %p, colorSurface: %p, loadDepthStencil: %p, storeDepthStencil: %p)\n", context, flags, renderTarget, validRegion, vertexSyncObject, fragmentSyncObject, colorSurface, loadDepthStencil, storeDepthStencil);
    return TAI_NEXT(sceGxmBeginSceneEx, sceGxmBeginSceneExRef, context, flags, renderTarget, validRegion, vertexSyncObject, fragmentSyncObject, colorSurface, loadDepthStencil, storeDepthStencil);
}

CREATE_PATCHED_CALL(void, sceGxmColorSurfaceGetClip, const SceGxmColorSurface *surface, unsigned int *xMin, unsigned int *yMin, unsigned int *xMax, unsigned int *yMax)
{
    LOGD("sceGxmColorSurfaceGetClip(surface: %p, xMin: %p, yMin: %p, xMax: %p, yMax: %p)\n", surface, xMin, yMin, xMax, yMax);
    TAI_NEXT(sceGxmColorSurfaceGetClip, sceGxmColorSurfaceGetClipRef, surface, xMin, yMin, xMax, yMax);
}

CREATE_PATCHED_CALL(void *, sceGxmColorSurfaceGetData, const SceGxmColorSurface *surface)
{
    LOGD("sceGxmColorSurfaceGetData(surface: %p)\n", surface);
    return TAI_NEXT(sceGxmColorSurfaceGetData, sceGxmColorSurfaceGetDataRef, surface);
}

CREATE_PATCHED_CALL(SceGxmColorSurfaceDitherMode, sceGxmColorSurfaceGetDitherMode, const SceGxmColorSurface *surface)
{
    LOGD("sceGxmColorSurfaceGetDitherMode(surface: %p)\n", surface);
    return TAI_NEXT(sceGxmColorSurfaceGetDitherMode, sceGxmColorSurfaceGetDitherModeRef, surface);
}

CREATE_PATCHED_CALL(SceGxmColorFormat, sceGxmColorSurfaceGetFormat, const SceGxmColorSurface *surface)
{
    LOGD("sceGxmColorSurfaceGetFormat(surface: %p)\n", surface);
    return TAI_NEXT(sceGxmColorSurfaceGetFormat, sceGxmColorSurfaceGetFormatRef, surface);
}

CREATE_PATCHED_CALL(SceGxmColorSurfaceGammaMode, sceGxmColorSurfaceGetGammaMode, const SceGxmColorSurface *surface)
{
    LOGD("sceGxmColorSurfaceGetGammaMode(surface: %p)\n", surface);
    return TAI_NEXT(sceGxmColorSurfaceGetGammaMode, sceGxmColorSurfaceGetGammaModeRef, surface);
}

CREATE_PATCHED_CALL(SceGxmColorSurfaceScaleMode, sceGxmColorSurfaceGetScaleMode, const SceGxmColorSurface *surface)
{
    LOGD("sceGxmColorSurfaceGetScaleMode(surface: %p)\n", surface);
    return TAI_NEXT(sceGxmColorSurfaceGetScaleMode, sceGxmColorSurfaceGetScaleModeRef, surface);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmColorSurfaceGetStrideInPixels, const SceGxmColorSurface *surface)
{
    LOGD("sceGxmColorSurfaceGetStrideInPixels(surface: %p)\n", surface);
    return TAI_NEXT(sceGxmColorSurfaceGetStrideInPixels, sceGxmColorSurfaceGetStrideInPixelsRef, surface);
}

CREATE_PATCHED_CALL(SceGxmColorSurfaceType, sceGxmColorSurfaceGetType, const SceGxmColorSurface *surface)
{
    LOGD("sceGxmColorSurfaceGetType(surface: %p)\n", surface);
    return TAI_NEXT(sceGxmColorSurfaceGetType, sceGxmColorSurfaceGetTypeRef, surface);
}

CREATE_PATCHED_CALL(int, sceGxmColorSurfaceInit, SceGxmColorSurface *surface, SceGxmColorFormat colorFormat, SceGxmColorSurfaceType surfaceType, SceGxmColorSurfaceScaleMode scaleMode, SceGxmOutputRegisterSize outputRegisterSize, unsigned int width, unsigned int height, unsigned int strideInPixels, void *data)
{
    LOGD("sceGxmColorSurfaceInit(surface: %p, colorFormat: %" PRIu32 ", surfaceType: %" PRIu32 ", scaleMode: %" PRIu32", outputRegisterSize: %" PRIu32 ", width: %" PRIu32 ", height: %" PRIu32 ", strideInPixels: %" PRIu32 ", data: %p)\n", surface, colorFormat, surfaceType, scaleMode, outputRegisterSize, width, height, strideInPixels, data);
    return TAI_NEXT(sceGxmColorSurfaceInit, sceGxmColorSurfaceInitRef, surface, colorFormat, surfaceType, scaleMode, outputRegisterSize, width, height, strideInPixels, data);
}

CREATE_PATCHED_CALL(int, sceGxmColorSurfaceInitDisabled, SceGxmColorSurface *surface)
{
    LOGD("sceGxmColorSurfaceInitDisabled(surface: %p)\n", surface);
    return TAI_NEXT(sceGxmColorSurfaceInitDisabled, sceGxmColorSurfaceInitDisabledRef, surface);
}

CREATE_PATCHED_CALL(SceBool, sceGxmColorSurfaceIsEnabled, const SceGxmColorSurface *surface)
{
    LOGD("sceGxmColorSurfaceIsEnabled(surface: %p)\n", surface);
    return TAI_NEXT(sceGxmColorSurfaceIsEnabled, sceGxmColorSurfaceIsEnabledRef, surface);
}

CREATE_PATCHED_CALL(void, sceGxmColorSurfaceSetClip, SceGxmColorSurface *surface, unsigned int xMin, unsigned int yMin, unsigned int xMax, unsigned int yMax)
{
    LOGD("sceGxmColorSurfaceSetClip(surface: %p, xMin: %" PRIu32 ", yMin: %" PRIu32 ", xMax: %" PRIu32 ", yMax: %" PRIu32 ")\n", surface, xMin, yMin, xMax, yMax);
    return TAI_NEXT(sceGxmColorSurfaceSetClip, sceGxmColorSurfaceSetClipRef, surface, xMin, yMin, xMax, yMax);
}

CREATE_PATCHED_CALL(int, sceGxmColorSurfaceSetData, SceGxmColorSurface *surface, void *data)
{
    LOGD("sceGxmColorSurfaceSetData(surface: %p, data: %p)\n", surface, data);
    return TAI_NEXT(sceGxmColorSurfaceSetData, sceGxmColorSurfaceSetDataRef, surface, data);
}

CREATE_PATCHED_CALL(int, sceGxmColorSurfaceSetDitherMode, SceGxmColorSurface *surface, SceGxmColorSurfaceDitherMode ditherMode)
{
    LOGD("sceGxmColorSurfaceSetDitherMode(surface: %p, scaleMode: %" PRIu32 ")\n", surface, ditherMode);
    return TAI_NEXT(sceGxmColorSurfaceSetDitherMode, sceGxmColorSurfaceSetDitherModeRef, surface, ditherMode);
}

CREATE_PATCHED_CALL(int, sceGxmColorSurfaceSetFormat, SceGxmColorSurface *surface, SceGxmColorFormat format)
{
    LOGD("sceGxmColorSurfaceSetFormat(surface: %p, scaleMode: %" PRIu32 ")\n", surface, format);
    return TAI_NEXT(sceGxmColorSurfaceSetFormat, sceGxmColorSurfaceSetFormatRef, surface, format);
}

CREATE_PATCHED_CALL(int, sceGxmColorSurfaceSetGammaMode, SceGxmColorSurface *surface, SceGxmColorSurfaceGammaMode gammaMode)
{
    LOGD("sceGxmColorSurfaceSetGammaMode(surface: %p, scaleMode: %" PRIu32 ")\n", surface, gammaMode);
    return TAI_NEXT(sceGxmColorSurfaceSetGammaMode, sceGxmColorSurfaceSetGammaModeRef, surface, gammaMode);
}

CREATE_PATCHED_CALL(void, sceGxmColorSurfaceSetScaleMode, SceGxmColorSurface *surface, SceGxmColorSurfaceScaleMode scaleMode)
{
    LOGD("sceGxmColorSurfaceSetScaleMode(surface: %p, scaleMode: %" PRIu32 ")\n", surface, scaleMode);
    TAI_NEXT(sceGxmColorSurfaceSetScaleMode, sceGxmColorSurfaceSetScaleModeRef, surface, scaleMode);
}

CREATE_PATCHED_CALL(int, sceGxmCreateContext, const SceGxmContextParams *params, SceGxmContext **context)
{
    LOGD("sceGxmCreateContext(params: %p, context: %p)\n", params, context);
    return TAI_NEXT(sceGxmCreateContext, sceGxmCreateContextRef, params, context);
}

int sceGxmCreateDeferredContext(const SceGxmContextParams *params, SceGxmContext **deferredContext);
CREATE_PATCHED_CALL(int, sceGxmCreateDeferredContext, const SceGxmContextParams *params, SceGxmContext **deferredContext)
{
    LOGD("sceGxmCreateDeferredContext(params: %p, deferredContext: %p)\n", params, deferredContext);
    return TAI_NEXT(sceGxmCreateDeferredContext, sceGxmCreateDeferredContextRef, params, deferredContext);
}

CREATE_PATCHED_CALL(int, sceGxmCreateRenderTarget, const SceGxmRenderTargetParams *params, SceGxmRenderTarget **renderTarget)
{
    LOGD("sceGxmCreateRenderTarget(params: %p, renderTarget: %p)\n", params, renderTarget);
    return TAI_NEXT(sceGxmCreateRenderTarget, sceGxmCreateRenderTargetRef, params, renderTarget);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmDepthStencilSurfaceGetBackgroundDepth, const SceGxmDepthStencilSurface *surface)
{
    LOGD("sceGxmDepthStencilSurfaceGetBackgroundDepth(surface: %p)\n", surface);
    return TAI_NEXT(sceGxmDepthStencilSurfaceGetBackgroundDepth, sceGxmDepthStencilSurfaceGetBackgroundDepthRef, surface);
}

SceBool sceGxmDepthStencilSurfaceGetBackgroundMask(const SceGxmDepthStencilSurface *surface);
CREATE_PATCHED_CALL(SceBool, sceGxmDepthStencilSurfaceGetBackgroundMask, const SceGxmDepthStencilSurface *surface)
{
    LOGD("sceGxmDepthStencilSurfaceGetBackgroundMask(surface: %p)\n", surface);
    return TAI_NEXT(sceGxmDepthStencilSurfaceGetBackgroundMask, sceGxmDepthStencilSurfaceGetBackgroundMaskRef, surface);
}

CREATE_PATCHED_CALL(unsigned char, sceGxmDepthStencilSurfaceGetBackgroundStencil, const SceGxmDepthStencilSurface *surface)
{
    LOGD("sceGxmDepthStencilSurfaceGetBackgroundStencil(surface: %p)\n", surface);
    return TAI_NEXT(sceGxmDepthStencilSurfaceGetBackgroundStencil, sceGxmDepthStencilSurfaceGetBackgroundStencilRef, surface);
}

CREATE_PATCHED_CALL(SceGxmDepthStencilForceLoadMode, sceGxmDepthStencilSurfaceGetForceLoadMode, const SceGxmDepthStencilSurface *surface)
{
    LOGD("sceGxmDepthStencilSurfaceGetForceLoadMode(surface: %p)\n", surface);
    return TAI_NEXT(sceGxmDepthStencilSurfaceGetForceLoadMode, sceGxmDepthStencilSurfaceGetForceLoadModeRef, surface);
}

CREATE_PATCHED_CALL(SceGxmDepthStencilForceStoreMode, sceGxmDepthStencilSurfaceGetForceStoreMode, const SceGxmDepthStencilSurface *surface)
{
    LOGD("sceGxmDepthStencilSurfaceGetForceStoreMode(surface: %p)\n", surface);
    return TAI_NEXT(sceGxmDepthStencilSurfaceGetForceStoreMode, sceGxmDepthStencilSurfaceGetForceStoreModeRef, surface);
}

CREATE_PATCHED_CALL(SceGxmDepthStencilFormat, sceGxmDepthStencilSurfaceGetFormat, const SceGxmDepthStencilSurface *surface)
{
    LOGD("sceGxmDepthStencilSurfaceGetFormat(surface: %p)\n", surface);
    return TAI_NEXT(sceGxmDepthStencilSurfaceGetFormat, sceGxmDepthStencilSurfaceGetFormatRef, surface);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmDepthStencilSurfaceGetStrideInSamples, const SceGxmDepthStencilSurface *surface)
{
    LOGD("sceGxmDepthStencilSurfaceGetStrideInSamples(surface: %p)\n", surface);
    return TAI_NEXT(sceGxmDepthStencilSurfaceGetStrideInSamples, sceGxmDepthStencilSurfaceGetStrideInSamplesRef, surface);
}

CREATE_PATCHED_CALL(int, sceGxmDepthStencilSurfaceInit, SceGxmDepthStencilSurface *surface, SceGxmDepthStencilFormat depthStencilFormat, SceGxmDepthStencilSurfaceType surfaceType, unsigned int strideInSamples, void *depthData, void *stencilData)
{
    LOGD("sceGxmDepthStencilSurfaceInit(surface: %p, depthStencilFormat: %" PRIu32 ", surfaceType: %" PRIu32 ", strideInSamples: %" PRIu32 ", depthData: %p, stencilData: %p)\n", surface, depthStencilFormat, surfaceType, strideInSamples, depthData, stencilData);
    return TAI_NEXT(sceGxmDepthStencilSurfaceInit, sceGxmDepthStencilSurfaceInitRef, surface, depthStencilFormat, surfaceType, strideInSamples, depthData, stencilData);
}

CREATE_PATCHED_CALL(int, sceGxmDepthStencilSurfaceInitDisabled, SceGxmDepthStencilSurface *surface)
{
    LOGD("sceGxmDepthStencilSurfaceInitDisabled(surface: %p)\n", surface);
    return TAI_NEXT(sceGxmDepthStencilSurfaceInitDisabled, sceGxmDepthStencilSurfaceInitDisabledRef, surface);
};

CREATE_PATCHED_CALL(SceBool, sceGxmDepthStencilSurfaceIsEnabled, const SceGxmDepthStencilSurface *surface)
{
    LOGD("sceGxmDepthStencilSurfaceIsEnabled(surface: %p)\n", surface);
    return TAI_NEXT(sceGxmDepthStencilSurfaceIsEnabled, sceGxmDepthStencilSurfaceIsEnabledRef, surface);
}

CREATE_PATCHED_CALL(void, sceGxmDepthStencilSurfaceSetBackgroundDepth, SceGxmDepthStencilSurface *surface, float backgroundDepth)
{
    LOGD("sceGxmDepthStencilSurfaceSetBackgroundDepth(surface: %p, backgroundDepth: %f)\n", surface, backgroundDepth);
    return TAI_NEXT(sceGxmDepthStencilSurfaceSetBackgroundDepth, sceGxmDepthStencilSurfaceSetBackgroundDepthRef, surface, backgroundDepth);
}

void sceGxmDepthStencilSurfaceSetBackgroundMask(SceGxmDepthStencilSurface *surface, SceBool backgroundMask);
CREATE_PATCHED_CALL(void, sceGxmDepthStencilSurfaceSetBackgroundMask, SceGxmDepthStencilSurface *surface, SceBool backgroundMask)
{
    LOGD("sceGxmDepthStencilSurfaceSetBackgroundMask(surface: %p, backgroundMask: %" PRIi32 ")\n", surface, backgroundMask);
    TAI_NEXT(sceGxmDepthStencilSurfaceSetBackgroundMask, sceGxmDepthStencilSurfaceSetBackgroundMaskRef, surface, backgroundMask);
}

CREATE_PATCHED_CALL(void, sceGxmDepthStencilSurfaceSetBackgroundStencil, SceGxmDepthStencilSurface *surface, unsigned char backgroundStencil)
{
    LOGD("sceGxmDepthStencilSurfaceSetBackgroundStencil(surface: %p, backgroundStencil: %" PRIu32 ")\n", surface, backgroundStencil);
    TAI_NEXT(sceGxmDepthStencilSurfaceSetBackgroundStencil, sceGxmDepthStencilSurfaceSetBackgroundStencilRef, surface, backgroundStencil);
}

CREATE_PATCHED_CALL(void, sceGxmDepthStencilSurfaceSetForceLoadMode, SceGxmDepthStencilSurface *surface, SceGxmDepthStencilForceLoadMode forceLoad)
{
    LOGD("sceGxmDepthStencilSurfaceSetForceLoadMode(surface: %p, forceLoad: %" PRIu32 ")\n", surface, forceLoad);
    TAI_NEXT(sceGxmDepthStencilSurfaceSetForceLoadMode, sceGxmDepthStencilSurfaceSetForceLoadModeRef, surface, forceLoad);
}

CREATE_PATCHED_CALL(void, sceGxmDepthStencilSurfaceSetForceStoreMode, SceGxmDepthStencilSurface *surface, SceGxmDepthStencilForceStoreMode forceStore)
{
    LOGD("sceGxmDepthStencilSurfaceSetForceStoreMode(surface: %p, forceStore: %" PRIu32 ")\n", surface, forceStore);
    TAI_NEXT(sceGxmDepthStencilSurfaceSetForceStoreMode, sceGxmDepthStencilSurfaceSetForceStoreModeRef, surface, forceStore);
}

CREATE_PATCHED_CALL(int, sceGxmDestroyContext, SceGxmContext *context)
{
    LOGD("sceGxmDestroyContext(context: %p)\n", context);
    return TAI_NEXT(sceGxmDestroyContext, sceGxmDestroyContextRef, context);
}

int sceGxmDestroyDeferredContext(SceGxmContext *context);
CREATE_PATCHED_CALL(int, sceGxmDestroyDeferredContext, SceGxmContext *context)
{
    LOGD("sceGxmDestroyDeferredContext(context: %p)\n", context);
    return TAI_NEXT(sceGxmDestroyDeferredContext, sceGxmDestroyDeferredContextRef, context);
}

CREATE_PATCHED_CALL(int, sceGxmDestroyRenderTarget, SceGxmRenderTarget *renderTarget)
{
    LOGD("sceGxmDestroyRenderTarget(renderTarget, %p)\n", renderTarget);
    return TAI_NEXT(sceGxmDestroyRenderTarget, sceGxmDestroyRenderTargetRef, renderTarget);
}

static void serializeBuffers()
{
    uint32_t chunkSize = 0;
    
    LOG("mapped memory count: %" PRIu32 "\n", g_mappedMemoryCount);

    for (uint32_t buffer_index = 0; buffer_index < g_mappedMemoryCount; ++buffer_index) {
        if (g_memoryArray[buffer_index].referenced) {
            MappedMemory &buffer = g_memoryArray[buffer_index];
            GXMChunk type = GXMChunk::InitBufferResources;
            g_fileoffset += g_file.write(type);
            g_fileoffset += g_file.write(chunkSize);
            g_fileoffset += g_file.write(buffer.bufferType);
            g_fileoffset += g_file.write(buffer.addr);
            g_fileoffset += g_file.write(buffer.size);
            g_fileoffset += g_file.write(buffer.resId);

            g_fileoffset += ALIGN_TO_64(g_fileoffset);

            LOG("referenced buffer: %" PRIi32 "\n", buffer_index);
        }
    }
}

char fname[256];

CREATE_PATCHED_CALL(int, sceGxmDisplayQueueAddEntry, SceGxmSyncObject *oldBuffer, SceGxmSyncObject *newBuffer, const void *callbackData)
{
    if (g_log) {
        uint32_t chunkSize = 0;

        GXMChunk type = GXMChunk::sceGxmDisplayQueueAddEntry;
        g_fileoffset += g_file.write(type);
        g_fileoffset += g_file.write(chunkSize);
        g_fileoffset += g_file.write(oldBuffer);
        g_fileoffset += g_file.write(newBuffer);
        g_fileoffset += g_file.write(callbackData);

        g_fileoffset += ALIGN_TO_64(g_fileoffset);
    }
    LOGD("sceGxmDisplayQueueAddEntry(oldBuffer: %p, newBuffer: %p, callbackData: %p)\n", oldBuffer, newBuffer, callbackData);

    int ret = TAI_NEXT(sceGxmDisplayQueueAddEntry, sceGxmDisplayQueueAddEntryRef, oldBuffer, newBuffer, callbackData);
    
    if (g_log) {
        g_log = 0;
        g_log_display = 1;
        g_logFrameCount = g_frameCount;
        
        uint32_t chunkSize = 0;

        GXMChunk type = (GXMChunk)6; // SystemChunk::CaptureEnd
        g_fileoffset += g_file.write(type);
        g_fileoffset += g_file.write(chunkSize);
        g_fileoffset += ALIGN_TO_64(g_fileoffset);

        g_file.close();

        SceDateTime time;
		sceRtcGetCurrentClockLocalTime(&time);
		sceClibSnprintf(fname, 256, "ux0:/data/renderdoc/%s_%04d.%02d.%02d_%02d.%02d.rdc", g_titleid, time.year, time.month, time.day, time.hour, time.minute);

        g_file.open(fname);

        struct FileHeader header = {};
        header.magic = 1129268306U;
        header.version = 257U;
        memset(header.progVersion, 0, 16);
        memcpy(header.progVersion, "1.7 09dce6", 10);

        struct BinaryThumbnail thumbHeader = {};
        thumbHeader.width = 0;
        thumbHeader.height = 0;
        thumbHeader.length = 0;

        struct CaptureMetaData meta;
        meta.driverID = 11;
        meta.machineIdent = 0;
        meta.driverNameLength = 4;

        header.headerLength = sizeof(struct FileHeader) + offsetof(struct BinaryThumbnail, data) + thumbHeader.length +
                              offsetof(struct CaptureMetaData, driverName) + meta.driverNameLength;
        g_file.write(header);
        g_file.write(&thumbHeader, offsetof(struct BinaryThumbnail, data));
        g_file.write(&meta, offsetof(struct CaptureMetaData, driverName));
        g_file.write("GXM", meta.driverNameLength);

        struct BinarySectionHeader section;
        section.isASCII = '\0';
        memset(section.zero, 0, 3);
        section.sectionType = 1U;
        section.sectionCompressedLength = 0;
        section.sectionUncompressedLength = 0;
        section.sectionVersion = 0;
        section.sectionFlags = 0;
        section.sectionNameLength = 13;

        g_file.write(&section, offsetof(struct BinarySectionHeader, name));
        g_file.write("FrameCapture", section.sectionNameLength);

        uint64_t drawcallfilesize = g_fileoffset;
        LOG("drawcallfilesize: %" PRIu64 "\n", drawcallfilesize);

        g_fileoffset = 0;

        // TODO: serialize used resources here
        serializeBuffers();

        uint64_t bufferfilesize = g_fileoffset;
        LOG("bufferfilesize: %" PRIu64 "\n", bufferfilesize);

        File tmp_file;
        tmp_file.open("ux0:/data/renderdoc/drawcalls.bin");

        uint8_t buffer[CHUNK_SIZE];

        uint32_t offset = 0;
        uint32_t buffer_size = 0;
        while(offset < drawcallfilesize) {

            if (drawcallfilesize - offset > CHUNK_SIZE) 
                buffer_size = CHUNK_SIZE;
            else
                buffer_size = drawcallfilesize - offset;
        
            tmp_file.read(buffer, buffer_size);
            g_file.write(buffer, buffer_size);

            offset += buffer_size;
        }

        //g_file.close();
        tmp_file.close();

        struct test_t {
            uint64_t file_size;
            uint64_t file_size_compressed;
        };

        struct test_t test;
        test.file_size = drawcallfilesize + bufferfilesize;
        test.file_size_compressed = drawcallfilesize + bufferfilesize;

        offset = sizeof(struct FileHeader) + offsetof(struct BinaryThumbnail, data) + offsetof(struct CaptureMetaData, driverName) + 4 + offsetof(struct BinarySectionHeader, sectionCompressedLength);
        g_file.replaceData(offset, sizeof(test_t), &test, sizeof(test_t));
        g_file.close();

        LOG("file size without header: %" PRIu64 "\n", drawcallfilesize + bufferfilesize);
        g_fileoffset = 0;
        //g_fd = -1;
    }

    ++g_frameCount;

    if (g_capture) {
        // clear all display queue calls to make sure the current 
        // framebuffer is really the one we currently have rendered to
        sceGxmDisplayQueueFinish();

        g_capture = 0;

        g_log = 1;

        g_file.open("ux0:/data/renderdoc/drawcalls.bin");

        uint32_t chunkSize = 0;

        GXMChunk type = (GXMChunk)5; // SystemChunk::CaptureScope
        g_fileoffset += g_file.write(type);
        g_fileoffset += g_file.write(chunkSize);
        g_fileoffset += g_file.write(g_frameCount);
        g_fileoffset += ALIGN_TO_64(g_fileoffset);

        type = (GXMChunk)4; // SystemChunk::CaptureBegin
        g_fileoffset += g_file.write(type);
        g_fileoffset += g_file.write(chunkSize);
        g_fileoffset += ALIGN_TO_64(g_fileoffset);

        type = GXMChunk::ContextConfiguration;
        g_fileoffset += g_file.write(type);
        g_fileoffset += g_file.write(chunkSize);
        g_fileoffset += g_file.write(g_framebuffer.base);
        g_fileoffset += g_file.write(g_framebuffer.width);
        g_fileoffset += g_file.write(g_framebuffer.height);
        g_fileoffset += g_file.write(g_framebuffer.pitch);
        g_fileoffset += g_file.write(g_framebuffer.pixelformat);
        g_fileoffset += g_file.write(++g_resourceid);

        g_fileoffset += ALIGN_TO_64(g_fileoffset);
    }

    return ret;
}

CREATE_PATCHED_CALL(int, sceGxmDisplayQueueFinish)
{
    LOGD("sceGxmDisplayQueueFinish()\n");
    return TAI_NEXT(sceGxmDisplayQueueFinish, sceGxmDisplayQueueFinishRef);
}

CREATE_PATCHED_CALL(int, sceGxmDraw, SceGxmContext* context, SceGxmPrimitiveType primType, SceGxmIndexFormat indexType, const void* indexData, unsigned int indexCount)
{
    LOGD("sceGxmDraw(context: %p, primType: %" PRIu32 ", indexType: %" PRIu32 ", indexData: %p, indexCount: %" PRIu32 ")\n", context, primType, indexType, indexData, indexCount);
    
    int res = TAI_NEXT(sceGxmDraw, sceGxmDrawRef, context, primType, indexType, indexData, indexCount);

    if (g_log) {
        uint32_t chunkSize = 0;

        GXMChunk type = GXMChunk::sceGxmDraw;
        g_fileoffset += g_file.write(type);
        g_fileoffset += g_file.write(chunkSize);
        g_fileoffset += g_file.write(context);
        g_fileoffset += g_file.write(primType);
        g_fileoffset += g_file.write(indexType);
        g_fileoffset += g_file.write(indexCount);

        uint32_t max_index = 0;
        uint32_t index_type_size = 0;

        switch(indexType) {
            case SCE_GXM_INDEX_FORMAT_U16: {
                index_type_size = 2;
                uint16_t* indices = (uint16_t*)indexData;
                for (uint32_t index = 0; index < indexCount; ++index) {
                    if (max_index < indices[index]) {
                        max_index = indices[index];
                    }
                }
            } break;
            case SCE_GXM_INDEX_FORMAT_U32: {
                index_type_size = 4;
                uint32_t* indices = (uint32_t*)indexData;
                for (uint32_t index = 0; index < indexCount; ++index) {
                    if (max_index < indices[index]) {
                        max_index = indices[index];
                    }
                }
            } break;
            default: break;
        }

        LOGD("sceGxmDraw, max_index: %" PRIu32 "\n", max_index);

        uint64_t indexbufferSize = indexCount * index_type_size;
        g_fileoffset += g_file.write(indexbufferSize);

        g_fileoffset += ALIGN_TO_64(g_fileoffset);
        g_fileoffset += g_file.write(indexData, indexbufferSize);
        
        for (uint32_t memory_index = 0; memory_index < g_mappedMemoryCount; ++memory_index) {
            if (indexData >= g_memoryArray[memory_index].addr && ((uintptr_t)indexData + indexbufferSize) < ((uintptr_t)g_memoryArray[memory_index].addr + g_memoryArray[memory_index].size)) {
                g_memoryArray[memory_index].referenced = 1;
                break;
            }
        }

        uint64_t resId = 0;
        for (uint32_t memory_index = 0; memory_index < g_mappedMemoryCount; ++memory_index) {
            if (indexData == g_memoryArray[memory_index].addr && indexbufferSize == g_memoryArray[memory_index].size)
            {
                resId = g_memoryArray[memory_index].resId;
                break;
            }
        }

        if (resId != 0)
        {
            g_fileoffset += g_file.write(resId);
        }
        else
        {
            resId = ++g_resourceid;
            g_fileoffset += g_file.write(resId);

            if (g_mappedMemoryCount + 1 < MAX_MAPPED_MEMORY_COUNT) {
                g_memoryArray[g_mappedMemoryCount].bufferType = GXMBufferType::SceGxmIndexBuffer;
                g_memoryArray[g_mappedMemoryCount].resId = resId;
                g_memoryArray[g_mappedMemoryCount].addr = indexData;
                g_memoryArray[g_mappedMemoryCount].size = indexbufferSize;
                g_memoryArray[g_mappedMemoryCount].referenced = 1;
                ++g_mappedMemoryCount;
            }
            else {
                LOG("Warn: too much mapped memory blocks\n");
            }
        }
   
        //ProgramResource programRes;
        //g_resource_manager.find(GXMType::SceGxmProgram, (uint32_t)vertexRes.programId, &programRes);

        //uint32_t parameter_count = sceGxmProgramGetParameterCount(programRes.program);

        //LOGD("sceGxmDraw: program length: %" PRIu32 ", parameter count: %" PRIu32 "\n", programRes.programLength, parameter_count);

        g_fileoffset += g_file.write(g_activeStreamCount);

        for (uint32_t stream_index = 0; stream_index < g_activeStreamCount; ++stream_index) {
            VertexStream const& current_stream = g_activeVertexStreams[g_activeVertexStreams[stream_index].indexSource];
            uint32_t vertexBufferSize = (max_index + 1) * current_stream.stride;
            g_fileoffset += g_file.write(vertexBufferSize);
            g_fileoffset += g_file.write((uint64_t)vertexBufferSize);

            g_fileoffset += ALIGN_TO_64(g_fileoffset);
            g_fileoffset += g_file.write(current_stream.dataPtr, vertexBufferSize);

            for (uint32_t memory_index = 0; memory_index < g_mappedMemoryCount; ++memory_index) {
                if (current_stream.dataPtr >= g_memoryArray[memory_index].addr && ((uintptr_t)current_stream.dataPtr + vertexBufferSize) < ((uintptr_t)g_memoryArray[memory_index].addr + g_memoryArray[memory_index].size)) {
                    g_memoryArray[memory_index].referenced = 1;
                    break;
                }
            }


            resId = 0;
            for (uint32_t memory_index = 0; memory_index < g_mappedMemoryCount; ++memory_index) {
                if (current_stream.dataPtr == g_memoryArray[memory_index].addr && vertexBufferSize == g_memoryArray[memory_index].size)
                {
                    resId = g_memoryArray[memory_index].resId;
                    break;
                }
            }
            
            if (resId != 0)
            {
                g_fileoffset += g_file.write(resId);
            }
            else
            {
                resId = ++g_resourceid;
                g_fileoffset += g_file.write(resId);

                if (g_mappedMemoryCount + 1 < MAX_MAPPED_MEMORY_COUNT) {
                    g_memoryArray[g_mappedMemoryCount].bufferType = GXMBufferType::SceGxmVertexBuffer;
                    g_memoryArray[g_mappedMemoryCount].resId = resId;
                    g_memoryArray[g_mappedMemoryCount].addr = current_stream.dataPtr;
                    g_memoryArray[g_mappedMemoryCount].size = vertexBufferSize;
                    g_memoryArray[g_mappedMemoryCount].referenced = 1;
                    ++g_mappedMemoryCount;
                }
                else {
                    LOG("Warn: too much mapped memory blocks\n");
                }
            }
        }

        g_fileoffset += ALIGN_TO_64(g_fileoffset);
    }

    return res;
}

CREATE_PATCHED_CALL(int, sceGxmDrawInstanced, SceGxmContext *context, SceGxmPrimitiveType primType, SceGxmIndexFormat indexType, const void *indexData, unsigned int indexCount, unsigned int indexWrap)
{
    LOGD("sceGxmDrawInstanced(context: %p, primType: %" PRIu32 ", indexType: %" PRIu32 ", indexData: %p, indexCount: %" PRIu32 ", indexWrap: %" PRIu32 ")\n", context, primType, indexType, indexData, indexCount);
    return TAI_NEXT(sceGxmDrawInstanced, sceGxmDrawInstancedRef, context, primType, indexType, indexData, indexCount, indexWrap);
}

CREATE_PATCHED_CALL(int, sceGxmDrawPrecomputed, SceGxmContext *context, const SceGxmPrecomputedDraw *precomputedDraw)
{
    LOGD("sceGxmDrawPrecomputed(context: %p, precomputedState: %p)\n", context, precomputedDraw);
    return TAI_NEXT(sceGxmDrawPrecomputed, sceGxmDrawPrecomputedRef, context, precomputedDraw);
}

int sceGxmEndCommandList(SceGxmContext *deferredContext, SceGxmCommandList *commandList);
CREATE_PATCHED_CALL(int, sceGxmEndCommandList, SceGxmContext *deferredContext, SceGxmCommandList *commandList)
{
    LOGD("sceGxmEndCommandList(deferredContext: %p, commandList: %p)\n", deferredContext, commandList);
    return TAI_NEXT(sceGxmEndCommandList, sceGxmEndCommandListRef, deferredContext, commandList);
}

CREATE_PATCHED_CALL(int, sceGxmEndScene, SceGxmContext *context, const SceGxmNotification *vertexNotification, const SceGxmNotification *fragmentNotification)
{
    if (g_log) {
        uint32_t chunkSize = 0;

        GXMChunk type = GXMChunk::sceGxmEndScene;
        g_fileoffset += g_file.write(type);
        g_fileoffset += g_file.write(chunkSize);
        g_fileoffset += g_file.write(context);
        g_fileoffset += g_file.write(vertexNotification);
        g_fileoffset += g_file.write(fragmentNotification);

        g_fileoffset += ALIGN_TO_64(g_fileoffset);
    }

    LOGD("sceGxmEndScene(context: %p, flags: %" PRIu32 ", vertexNotification: %p, fragmentNotification: %p)\n", context, vertexNotification, fragmentNotification);
    return TAI_NEXT(sceGxmEndScene, sceGxmEndSceneRef, context, vertexNotification, fragmentNotification);
}

int sceGxmExecuteCommandList(SceGxmContext *immediateContext, SceGxmCommandList *commandList);
CREATE_PATCHED_CALL(int, sceGxmExecuteCommandList, SceGxmContext *immediateContext, SceGxmCommandList *commandList)
{
    LOGD("sceGxmExecuteCommandList(immediateContext: %p, commandList: %p)\n", immediateContext, commandList);
    return TAI_NEXT(sceGxmExecuteCommandList, sceGxmExecuteCommandListRef, immediateContext, commandList);
}

CREATE_PATCHED_CALL(void, sceGxmFinish, SceGxmContext *context)
{
    LOGD("sceGxmFinish(context: %p)\n", context);
    TAI_NEXT(sceGxmFinish, sceGxmFinishRef, context);
}

SceGxmPassType sceGxmFragmentProgramGetPassType(SceGxmFragmentProgram *fragmentProgram);
CREATE_PATCHED_CALL(SceGxmPassType, sceGxmFragmentProgramGetPassType, SceGxmFragmentProgram *fragmentProgram)
{
    LOGD("sceGxmFragmentProgramGetPassType(fragmentProgram: %p)\n", fragmentProgram);
    return TAI_NEXT(sceGxmFragmentProgramGetPassType, sceGxmFragmentProgramGetPassTypeRef, fragmentProgram);
}

CREATE_PATCHED_CALL(const SceGxmProgram *, sceGxmFragmentProgramGetProgram, const SceGxmFragmentProgram *fragmentProgram)
{
    LOGD("sceGxmFragmentProgramGetProgram(fragmentProgram: %p)\n", fragmentProgram);
    return TAI_NEXT(sceGxmFragmentProgramGetProgram, sceGxmFragmentProgramGetProgramRef, fragmentProgram);
}

SceBool sceGxmFragmentProgramIsEnabled(const SceGxmFragmentProgram *fragmentProgram);
CREATE_PATCHED_CALL(SceBool, sceGxmFragmentProgramIsEnabled, const SceGxmFragmentProgram *fragmentProgram)
{
    LOGD("sceGxmFragmentProgramIsEnabled(fragmentProgram: %p)\n", fragmentProgram);
    return TAI_NEXT(sceGxmFragmentProgramIsEnabled, sceGxmFragmentProgramIsEnabledRef, fragmentProgram);
}

int sceGxmGetContextType(const SceGxmContext *context, SceGxmContextType *type);
CREATE_PATCHED_CALL(int, sceGxmGetContextType, const SceGxmContext *context, SceGxmContextType *type)
{
    LOGD("sceGxmGetContextType(context: %p, type: %p)\n", context, type);
    return TAI_NEXT(sceGxmGetContextType, sceGxmGetContextTypeRef, context, type);
}

int sceGxmGetDeferredContextFragmentBuffer(const SceGxmContext *deferredContext, void **mem);
CREATE_PATCHED_CALL(int, sceGxmGetDeferredContextFragmentBuffer, const SceGxmContext *deferredContext, void **mem)
{
    LOGD("sceGxmGetDeferredContextFragmentBuffer(deferredContext: %p, mem: %p)\n", deferredContext, mem);
    return TAI_NEXT(sceGxmGetDeferredContextFragmentBuffer, sceGxmGetDeferredContextFragmentBufferRef, deferredContext, mem);
}

int sceGxmGetDeferredContextVdmBuffer(const SceGxmContext *deferredContext, void **mem);
CREATE_PATCHED_CALL(int, sceGxmGetDeferredContextVdmBuffer, const SceGxmContext *deferredContext, void **mem)
{
    LOGD("sceGxmGetDeferredContextVdmBuffer(deferredContext: %p, mem: %p)\n", deferredContext, mem);
    return TAI_NEXT(sceGxmGetDeferredContextVdmBuffer, sceGxmGetDeferredContextVdmBufferRef, deferredContext, mem);
}

int sceGxmGetDeferredContextVertexBuffer(const SceGxmContext *deferredContext, void **mem);
CREATE_PATCHED_CALL(int, sceGxmGetDeferredContextVertexBuffer, const SceGxmContext *deferredContext, void **mem)
{
    LOGD("sceGxmGetDeferredContextVertexBuffer(deferredContext: %p, mem: %p)\n", deferredContext, mem);
    return TAI_NEXT(sceGxmGetDeferredContextVertexBuffer, sceGxmGetDeferredContextVertexBufferRef, deferredContext, mem);
}

CREATE_PATCHED_CALL(volatile unsigned int *, sceGxmGetNotificationRegion)
{
    LOGD("sceGxmGetNotificationRegion()\n");
    return TAI_NEXT(sceGxmGetNotificationRegion, sceGxmGetNotificationRegionRef);
}

int sceGxmGetParameterBufferThreshold(unsigned int *parameterBufferSize);
CREATE_PATCHED_CALL(int, sceGxmGetParameterBufferThreshold, unsigned int *parameterBufferSize)
{
    LOGD("sceGxmGetParameterBufferThreshold(parameterBufferSize: %p)\n", parameterBufferSize);
    return TAI_NEXT(sceGxmGetParameterBufferThreshold, sceGxmGetParameterBufferThresholdRef, parameterBufferSize);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmGetPrecomputedDrawSize, const SceGxmVertexProgram *vertexProgram)
{
    LOGD("sceGxmGetPrecomputedDrawSize(vertexProgram, %p)\n", vertexProgram);
    return TAI_NEXT(sceGxmGetPrecomputedDrawSize, sceGxmGetPrecomputedDrawSizeRef, vertexProgram);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmGetPrecomputedFragmentStateSize, const SceGxmFragmentProgram *fragmentProgram)
{
    LOGD("sceGxmGetPrecomputedFragmentStateSize(fragmentProgram, %p)\n", fragmentProgram);
    return TAI_NEXT(sceGxmGetPrecomputedFragmentStateSize, sceGxmGetPrecomputedFragmentStateSizeRef, fragmentProgram);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmGetPrecomputedVertexStateSize, const SceGxmVertexProgram *vertexProgram)
{
    LOGD("sceGxmGetPrecomputedVertexStateSize(vertexProgram, %p)\n", vertexProgram);
    return TAI_NEXT(sceGxmGetPrecomputedVertexStateSize, sceGxmGetPrecomputedVertexStateSizeRef, vertexProgram);
}

CREATE_PATCHED_CALL(int, sceGxmGetRenderTargetMemSizes, const SceGxmRenderTargetParams *params, unsigned int *hostMemSize, unsigned int *driverMemSize)
{
    LOGD("sceGxmGetRenderTargetMemSizes(params, %p, hostMemSize: %p, driverMemSize: %p)\n", params, hostMemSize, driverMemSize);
    return TAI_NEXT(sceGxmGetRenderTargetMemSizes, sceGxmGetRenderTargetMemSizesRef, params, hostMemSize, driverMemSize);
}

CREATE_PATCHED_CALL(int, sceGxmInitialize, SceGxmInitializeParams *params)
{
    int ret = sceNetShowNetstat();
    if (SCE_NET_ERROR_ENOTINIT == static_cast<uint32_t>(ret)) {
        LOG("Net not initialized so far\n");

        if (sceSysmoduleIsLoaded(SCE_SYSMODULE_NET) != SCE_SYSMODULE_LOADED)
            sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
        SceNetInitParam initparam;
        initparam.memory = networkbuffer;
        initparam.size = 16*1024;
        initparam.flags = 0;
        ret = sceNetInit(&initparam);
        if (ret < 0) {
            LOG("sceNetInit failed: %08X\n", ret);
        }

        serverThreadId = createThread("TargetControlServerThread", &sRemoteControlThreadInit, 0, NULL);
    }
    else if (ret < 0) {
        LOG("Net error\n");
    }
    else {
        LOG("Net seems to be initialized\n");
        serverThreadId = createThread("TargetControlServerThread", &sRemoteControlThreadInit, 0, NULL);
    }

    LOG("sceGxmInitialize(params: %p)\n", params);
    LOG("sceGxmInitialize(params.displayQueueCallback: %p)\n", params->displayQueueCallback);
    LOG("sceGxmInitialize(params.displayQueueCallbackDataSize: %" PRIu32 ")\n", params->displayQueueCallbackDataSize);

    g_displaycallback = params->displayQueueCallback;
    params->displayQueueCallback = &display_queue_callback_patched;

    return TAI_NEXT(sceGxmInitialize, sceGxmInitializeRef, params);
}

SceBool sceGxmIsDebugVersion();
CREATE_PATCHED_CALL(SceBool, sceGxmIsDebugVersion)
{
    LOGD("sceGxmIsDebugVersion()\n");
    return TAI_NEXT(sceGxmIsDebugVersion, sceGxmIsDebugVersionRef);
}

CREATE_PATCHED_CALL(int, sceGxmMapFragmentUsseMemory, void *base, SceSize size, unsigned int *offset)
{
    LOGD("sceGxmMapFragmentUsseMemory(base: %p, size: %" PRIu32 ", offset: %p)\n", base, size, offset);
    return TAI_NEXT(sceGxmMapFragmentUsseMemory, sceGxmMapFragmentUsseMemoryRef, base, size, offset);
}

CREATE_PATCHED_CALL(int, sceGxmMapMemory, void *base, SceSize size, SceGxmMemoryAttribFlags attr)
{
    uint64_t resid = ++g_resourceid;

    if (g_log) {
        uint32_t chunkSize = 0;

        GXMChunk type = GXMChunk::sceGxmMapMemory;
        g_fileoffset += g_file.write(type);
        g_fileoffset += g_file.write(chunkSize);
        g_fileoffset += g_file.write(base);
        g_fileoffset += g_file.write(size);
        g_fileoffset += g_file.write(attr);
        g_fileoffset += g_file.write(resid);

        g_fileoffset += ALIGN_TO_64(g_fileoffset);
    }

    if (g_mappedMemoryCount + 1 < MAX_MAPPED_MEMORY_COUNT) {
        g_memoryArray[g_mappedMemoryCount].bufferType = GXMBufferType::SceGxmMappedBuffer;
        g_memoryArray[g_mappedMemoryCount].resId = resid;
        g_memoryArray[g_mappedMemoryCount].addr = base;
        g_memoryArray[g_mappedMemoryCount].size = size;
        ++g_mappedMemoryCount;
    }
    else {
        LOG("Warn: too much mapped memory blocks\n");
    }

    LOG("sceGxmMapMemory(base: %p, size: %" PRIu32 ", attr: %" PRIu32 ")\n", base, size, attr);
    return TAI_NEXT(sceGxmMapMemory, sceGxmMapMemoryRef, base, size, attr);
}

CREATE_PATCHED_CALL(int, sceGxmMapVertexUsseMemory, void *base, SceSize size, unsigned int *offset)
{
    LOGD("sceGxmMapVertexUsseMemory(base: %p, size: %" PRIu32 ", offset: %p)\n", base, size, offset);
    return TAI_NEXT(sceGxmMapVertexUsseMemory, sceGxmMapVertexUsseMemoryRef, base, size, offset);
}

CREATE_PATCHED_CALL(int, sceGxmMidSceneFlush, SceGxmContext *context, unsigned int flags, SceGxmSyncObject *vertexSyncObject, const SceGxmNotification *vertexNotification)
{
    LOGD("sceGxmMidSceneFlush(context: %p, flags: %" PRIu32 ", vertexSyncObject: %p, vertexNotification: %p)\n", context, flags, vertexSyncObject, vertexNotification);
    return TAI_NEXT(sceGxmMidSceneFlush, sceGxmMidSceneFlushRef, context, flags, vertexSyncObject, vertexNotification);
}

CREATE_PATCHED_CALL(int, sceGxmNotificationWait, const SceGxmNotification *notification)
{
    LOGD("sceGxmNotificationWait(notification: %p)\n", notification);
    return TAI_NEXT(sceGxmNotificationWait, sceGxmNotificationWaitRef, notification);
}

CREATE_PATCHED_CALL(int, sceGxmPadHeartbeat, const SceGxmColorSurface *displaySurface, SceGxmSyncObject *displaySyncObject)
{
    if (g_log) {
        uint32_t chunkSize = 0;

        GXMChunk type = GXMChunk::sceGxmPadHeartbeat;
        g_fileoffset += g_file.write(type);
        g_fileoffset += g_file.write(chunkSize);
        g_fileoffset += g_file.write(displaySurface);
        g_fileoffset += g_file.write(displaySyncObject);

        g_fileoffset += ALIGN_TO_64(g_fileoffset);
    }

    LOGD("sceGxmPadHeartbeat(displaySurface: %p, displaySyncObject: %p)\n", displaySurface, displaySyncObject);
    return TAI_NEXT(sceGxmPadHeartbeat, sceGxmPadHeartbeatRef, displaySurface, displaySyncObject);
}

CREATE_PATCHED_CALL(int, sceGxmPadTriggerGpuPaTrace)
{
    LOGD("sceGxmPadTriggerGpuPaTrace()\n");
    return TAI_NEXT(sceGxmPadTriggerGpuPaTrace, sceGxmPadTriggerGpuPaTraceRef);
}

CREATE_PATCHED_CALL(int, sceGxmPopUserMarker, SceGxmContext *context)
{
    LOGD("sceGxmPopUserMarker(context: %p)\n", context);
    return TAI_NEXT(sceGxmPopUserMarker, sceGxmPopUserMarkerRef, context);
}

CREATE_PATCHED_CALL(int, sceGxmPrecomputedDrawInit, SceGxmPrecomputedDraw *precomputedDraw, const SceGxmVertexProgram *vertexProgram, void *memBlock)
{
    LOGD("sceGxmPrecomputedDrawInit(precomputedDraw, %p, vertexProgram: %p, memBlock: %p)\n", precomputedDraw, vertexProgram, memBlock);
    return TAI_NEXT(sceGxmPrecomputedDrawInit, sceGxmPrecomputedDrawInitRef, precomputedDraw, vertexProgram, memBlock);
}

CREATE_PATCHED_CALL(int, sceGxmPrecomputedDrawSetAllVertexStreams, SceGxmPrecomputedDraw *precomputedDraw, const void * const *streamDataArray)
{
    LOGD("sceGxmPrecomputedDrawSetAllVertexStreams(precomputedDraw, %p, streamDataArray: %p)\n", precomputedDraw, streamDataArray);
    return TAI_NEXT(sceGxmPrecomputedDrawSetAllVertexStreams, sceGxmPrecomputedDrawSetAllVertexStreamsRef, precomputedDraw, streamDataArray);
}

CREATE_PATCHED_CALL(void, sceGxmPrecomputedDrawSetParams, SceGxmPrecomputedDraw *precomputedDraw, SceGxmPrimitiveType primType, SceGxmIndexFormat indexType, const void *indexData, unsigned int indexCount)
{
    LOGD("sceGxmPrecomputedDrawSetParams(precomputedDraw, %p, primType: %" PRIu32 ", indexType: %" PRIu32 ", indexData: %p, indexCount: %" PRIu32 ")\n", precomputedDraw, primType, indexType, indexData, indexCount);
    TAI_NEXT(sceGxmPrecomputedDrawSetParams, sceGxmPrecomputedDrawSetParamsRef, precomputedDraw, primType, indexType, indexData, indexCount);
}

CREATE_PATCHED_CALL(void, sceGxmPrecomputedDrawSetParamsInstanced, SceGxmPrecomputedDraw *precomputedDraw, SceGxmPrimitiveType primType, SceGxmIndexFormat indexType, const void *indexData, unsigned int indexCount, unsigned int indexWrap)
{
    LOGD("sceGxmPrecomputedDrawSetParamsInstanced(precomputedDraw, %p, primType: %" PRIu32 ", indexType: %" PRIu32 ", indexData: %p, indexCount: %" PRIu32 ", indexWrap: %" PRIu32 ")\n", precomputedDraw, primType, indexType, indexData, indexCount, indexWrap);
    TAI_NEXT(sceGxmPrecomputedDrawSetParamsInstanced, sceGxmPrecomputedDrawSetParamsInstancedRef, precomputedDraw, primType, indexType, indexData, indexCount, indexWrap);
}

CREATE_PATCHED_CALL(int, sceGxmPrecomputedDrawSetVertexStream, SceGxmPrecomputedDraw *precomputedDraw, unsigned int streamIndex, const void *streamData)
{
    LOGD("sceGxmPrecomputedDrawSetVertexStream(precomputedDraw, %p, streamIndex: %" PRIu32 ", streamData: %p)\n", precomputedDraw, streamIndex, streamData);
    return TAI_NEXT(sceGxmPrecomputedDrawSetVertexStream, sceGxmPrecomputedDrawSetVertexStreamRef, precomputedDraw, streamIndex, streamData);
}

CREATE_PATCHED_CALL(void *, sceGxmPrecomputedFragmentStateGetDefaultUniformBuffer, const SceGxmPrecomputedFragmentState *precomputedState)
{
    LOGD("sceGxmPrecomputedFragmentStateGetDefaultUniformBuffer(precomputedState, %p)\n", precomputedState);
    return TAI_NEXT(sceGxmPrecomputedFragmentStateGetDefaultUniformBuffer, sceGxmPrecomputedFragmentStateGetDefaultUniformBufferRef, precomputedState);
}

CREATE_PATCHED_CALL(int, sceGxmPrecomputedFragmentStateInit, SceGxmPrecomputedFragmentState *precomputedState, const SceGxmFragmentProgram *fragmentProgram, void *memBlock)
{
    LOGD("sceGxmPrecomputedFragmentStateInit(precomputedState, %p, fragmentProgram: %p, memBlock: %p)\n", precomputedState, fragmentProgram, memBlock);
    return TAI_NEXT(sceGxmPrecomputedFragmentStateInit, sceGxmPrecomputedFragmentStateInitRef, precomputedState, fragmentProgram, memBlock);
}

CREATE_PATCHED_CALL(int, sceGxmPrecomputedFragmentStateSetAllAuxiliarySurfaces, SceGxmPrecomputedFragmentState *precomputedState, const SceGxmAuxiliarySurface *auxSurfaceArray)
{
    LOGD("sceGxmPrecomputedFragmentStateSetAllAuxiliarySurfaces(precomputedState, %p, auxSurfaceArray: %p)\n", precomputedState, auxSurfaceArray);
    return TAI_NEXT(sceGxmPrecomputedFragmentStateSetAllAuxiliarySurfaces, sceGxmPrecomputedFragmentStateSetAllAuxiliarySurfacesRef, precomputedState, auxSurfaceArray);
}

CREATE_PATCHED_CALL(int, sceGxmPrecomputedFragmentStateSetAllTextures, SceGxmPrecomputedFragmentState *precomputedState, const SceGxmTexture *textureArray)
{
    LOGD("sceGxmPrecomputedFragmentStateSetAllTextures(precomputedState, %p, textureArray: %p)\n", precomputedState, textureArray);
    return TAI_NEXT(sceGxmPrecomputedFragmentStateSetAllTextures, sceGxmPrecomputedFragmentStateSetAllTexturesRef, precomputedState, textureArray);
}

CREATE_PATCHED_CALL(int, sceGxmPrecomputedFragmentStateSetAllUniformBuffers, SceGxmPrecomputedFragmentState *precomputedState, const void * const *bufferDataArray)
{
    LOGD("sceGxmPrecomputedFragmentStateSetAllUniformBuffers(precomputedState, %p, bufferDataArray: %p)\n", precomputedState, bufferDataArray);
    return TAI_NEXT(sceGxmPrecomputedFragmentStateSetAllUniformBuffers, sceGxmPrecomputedFragmentStateSetAllUniformBuffersRef, precomputedState, bufferDataArray);
}

CREATE_PATCHED_CALL(void, sceGxmPrecomputedFragmentStateSetDefaultUniformBuffer, SceGxmPrecomputedFragmentState *precomputedState, void *defaultBuffer)
{
    LOGD("sceGxmPrecomputedFragmentStateSetDefaultUniformBuffer(precomputedState, %p, defaultBuffer: %p)\n", precomputedState, defaultBuffer);
    TAI_NEXT(sceGxmPrecomputedFragmentStateSetDefaultUniformBuffer, sceGxmPrecomputedFragmentStateSetDefaultUniformBufferRef, precomputedState, defaultBuffer);
}

CREATE_PATCHED_CALL(int, sceGxmPrecomputedFragmentStateSetTexture, SceGxmPrecomputedFragmentState *precomputedState, unsigned int textureIndex, const SceGxmTexture *texture)
{
    LOGD("sceGxmPrecomputedFragmentStateSetTexture(precomputedState, %p, textureIndex: %" PRIu32 ", texture: %p)\n", precomputedState, textureIndex, texture);
    return TAI_NEXT(sceGxmPrecomputedFragmentStateSetTexture, sceGxmPrecomputedFragmentStateSetTextureRef, precomputedState, textureIndex, texture);
}

CREATE_PATCHED_CALL(int, sceGxmPrecomputedFragmentStateSetUniformBuffer, SceGxmPrecomputedFragmentState *precomputedState, unsigned int bufferIndex, const void *bufferData)
{
    LOGD("sceGxmPrecomputedFragmentStateSetUniformBuffer(precomputedState, %p, bufferIndex: %" PRIu32 ", bufferData: %p)\n", precomputedState, bufferIndex, bufferData);
    return TAI_NEXT(sceGxmPrecomputedFragmentStateSetUniformBuffer, sceGxmPrecomputedFragmentStateSetUniformBufferRef, precomputedState, bufferIndex, bufferData);
}

CREATE_PATCHED_CALL(void *, sceGxmPrecomputedVertexStateGetDefaultUniformBuffer, const SceGxmPrecomputedVertexState *precomputedState)
{
    LOGD("sceGxmPrecomputedVertexStateGetDefaultUniformBuffer(precomputedState, %p)\n", precomputedState);
    return TAI_NEXT(sceGxmPrecomputedVertexStateGetDefaultUniformBuffer, sceGxmPrecomputedVertexStateGetDefaultUniformBufferRef, precomputedState);
}

CREATE_PATCHED_CALL(int, sceGxmPrecomputedVertexStateInit, SceGxmPrecomputedVertexState *precomputedState, const SceGxmVertexProgram *vertexProgram, void *memBlock)
{
    LOGD("sceGxmPrecomputedVertexStateInit(precomputedState, %p, vertexProgram: %p, memBlock: %p)\n", precomputedState, vertexProgram, memBlock);
    return TAI_NEXT(sceGxmPrecomputedVertexStateInit, sceGxmPrecomputedVertexStateInitRef, precomputedState, vertexProgram, memBlock);
}

CREATE_PATCHED_CALL(int, sceGxmPrecomputedVertexStateSetAllTextures, SceGxmPrecomputedVertexState *precomputedState, const SceGxmTexture *textures)
{
    LOGD("sceGxmPrecomputedVertexStateSetAllTextures(precomputedState, %p, textures: %p)\n", precomputedState, textures);
    return TAI_NEXT(sceGxmPrecomputedVertexStateSetAllTextures, sceGxmPrecomputedVertexStateSetAllTexturesRef, precomputedState, textures);
}

CREATE_PATCHED_CALL(int, sceGxmPrecomputedVertexStateSetAllUniformBuffers, SceGxmPrecomputedVertexState *precomputedState, const void * const *bufferDataArray)
{
    LOGD("sceGxmPrecomputedVertexStateSetAllUniformBuffers(precomputedState, %p, bufferDataArray: %p)\n", precomputedState, bufferDataArray);
    return TAI_NEXT(sceGxmPrecomputedVertexStateSetAllUniformBuffers, sceGxmPrecomputedVertexStateSetAllUniformBuffersRef, precomputedState, bufferDataArray);
}

CREATE_PATCHED_CALL(void, sceGxmPrecomputedVertexStateSetDefaultUniformBuffer, SceGxmPrecomputedVertexState *precomputedState, void *defaultBuffer)
{
    LOGD("sceGxmPrecomputedVertexStateSetDefaultUniformBuffer(precomputedState, %p, defaultBuffer: %p)\n", precomputedState, defaultBuffer);
    return TAI_NEXT(sceGxmPrecomputedVertexStateSetDefaultUniformBuffer, sceGxmPrecomputedVertexStateSetDefaultUniformBufferRef, precomputedState, defaultBuffer);
}

CREATE_PATCHED_CALL(int, sceGxmPrecomputedVertexStateSetTexture, SceGxmPrecomputedVertexState *precomputedState, unsigned int textureIndex, const SceGxmTexture *texture)
{
    LOGD("sceGxmPrecomputedVertexStateSetTexture(precomputedState, %p, textureIndex: %" PRIu32 ", texture: %p)\n", precomputedState, textureIndex, texture);
    return TAI_NEXT(sceGxmPrecomputedVertexStateSetTexture, sceGxmPrecomputedVertexStateSetTextureRef, precomputedState, textureIndex, texture);
}

CREATE_PATCHED_CALL(int, sceGxmPrecomputedVertexStateSetUniformBuffer, SceGxmPrecomputedVertexState *precomputedState, unsigned int bufferIndex, const void *bufferData)
{
    LOGD("sceGxmPrecomputedVertexStateSetUniformBuffer(precomputedState, %p, bufferIndex: %" PRIu32 ", bufferData: %p)\n", precomputedState, bufferIndex, bufferData);
    return TAI_NEXT(sceGxmPrecomputedVertexStateSetUniformBuffer, sceGxmPrecomputedVertexStateSetUniformBufferRef, precomputedState, bufferIndex, bufferData);
}

CREATE_PATCHED_CALL(int, sceGxmPushUserMarker, SceGxmContext *context, const char *tag)
{
    LOGD("sceGxmPushUserMarker(context: %p, tag: %s)\n", context, tag);
    return TAI_NEXT(sceGxmPushUserMarker, sceGxmPushUserMarkerRef, context, tag);
}

int sceGxmRemoveRazorGpuCaptureBuffer(void *base);
CREATE_PATCHED_CALL(int, sceGxmRemoveRazorGpuCaptureBuffer, void *base)
{
    LOGD("sceGxmRemoveRazorGpuCaptureBuffer(base: %p)\n", base);
    return TAI_NEXT(sceGxmRemoveRazorGpuCaptureBuffer, sceGxmRemoveRazorGpuCaptureBufferRef, base);
}

CREATE_PATCHED_CALL(int, sceGxmRenderTargetGetDriverMemBlock, const SceGxmRenderTarget *renderTarget, SceUID *driverMemBlock)
{
    LOGD("sceGxmRenderTargetGetDriverMemBlock(renderTarget, %p, driverMemBlock: %p)\n", renderTarget, driverMemBlock);
    return TAI_NEXT(sceGxmRenderTargetGetDriverMemBlock, sceGxmRenderTargetGetDriverMemBlockRef, renderTarget, driverMemBlock);
}

CREATE_PATCHED_CALL(int, sceGxmRenderTargetGetHostMem, const SceGxmRenderTarget *renderTarget, void **hostMem)
{
    LOGD("sceGxmRenderTargetGetHostMem(renderTarget, %p, hostMem: %p)\n", renderTarget, hostMem);
    return TAI_NEXT(sceGxmRenderTargetGetHostMem, sceGxmRenderTargetGetHostMemRef, renderTarget, hostMem);
}

CREATE_PATCHED_CALL(int, sceGxmReserveFragmentDefaultUniformBuffer, SceGxmContext *context, void **uniformBuffer)
{
    int res = TAI_NEXT(sceGxmReserveFragmentDefaultUniformBuffer, sceGxmReserveFragmentDefaultUniformBufferRef, context, uniformBuffer);
    if (g_log) {
        uint32_t chunkSize = 0;

        GXMChunk type = GXMChunk::sceGxmReserveFragmentDefaultUniformBuffer;
        g_fileoffset += g_file.write(type);
        g_fileoffset += g_file.write(chunkSize);
        g_fileoffset += g_file.write(context);
        g_fileoffset += g_file.write(uniformBuffer);

        g_fileoffset += ALIGN_TO_64(g_fileoffset);
    }

    LOGD("sceGxmReserveFragmentDefaultUniformBuffer(context: %p, uniformBuffer: %p)\n", context, uniformBuffer);
    return res;
}

CREATE_PATCHED_CALL(int, sceGxmReserveVertexDefaultUniformBuffer, SceGxmContext *context, void **uniformBuffer)
{
    if (g_log) {
        uint32_t chunkSize = 0;

        GXMChunk type = GXMChunk::sceGxmReserveVertexDefaultUniformBuffer;
        g_fileoffset += g_file.write(type);
        g_fileoffset += g_file.write(chunkSize);
        g_fileoffset += g_file.write(context);
        g_fileoffset += g_file.write(uniformBuffer);

        g_fileoffset += ALIGN_TO_64(g_fileoffset);
    }

    LOGD("sceGxmReserveVertexDefaultUniformBuffer(context: %p, uniformBuffer: %p)\n", context, uniformBuffer);
    return TAI_NEXT(sceGxmReserveVertexDefaultUniformBuffer, sceGxmReserveVertexDefaultUniformBufferRef, context, uniformBuffer);
}

CREATE_PATCHED_CALL(int, sceGxmSetAuxiliarySurface, SceGxmContext *context, unsigned int surfaceIndex, const SceGxmAuxiliarySurface *surface)
{
    LOGD("sceGxmSetAuxiliarySurface(context: %p, surfaceIndex: %" PRIu32 ", surface: %p)\n", context, surfaceIndex, surface);
    return TAI_NEXT(sceGxmSetAuxiliarySurface, sceGxmSetAuxiliarySurfaceRef, context, surfaceIndex, surface);
}

CREATE_PATCHED_CALL(void, sceGxmSetBackDepthBias, SceGxmContext *context, int factor, int units)
{
    LOGD("sceGxmSetBackDepthBias(context: %p, factor: %" PRIi32 ", units: %" PRIi32 ")\n", context, factor, units);
    TAI_NEXT(sceGxmSetBackDepthBias, sceGxmSetBackDepthBiasRef, context, factor, units);
}

CREATE_PATCHED_CALL(void, sceGxmSetBackDepthFunc, SceGxmContext *context, SceGxmDepthFunc depthFunc)
{
    LOGD("sceGxmSetBackDepthFunc(context: %p, depthFunc: %" PRIu32 ")\n", context, depthFunc);
    TAI_NEXT(sceGxmSetBackDepthFunc, sceGxmSetBackDepthFuncRef, context, depthFunc);
}

CREATE_PATCHED_CALL(void, sceGxmSetBackDepthWriteEnable, SceGxmContext *context, SceGxmDepthWriteMode enable)
{
    LOGD("sceGxmSetBackDepthWriteEnable(context: %p, enable: %" PRIu32 ")\n", context, enable);
    TAI_NEXT(sceGxmSetBackDepthWriteEnable, sceGxmSetBackDepthWriteEnableRef, context, enable);
}

CREATE_PATCHED_CALL(void, sceGxmSetBackFragmentProgramEnable, SceGxmContext *context, SceGxmFragmentProgramMode enable)
{
    LOGD("sceGxmSetBackFragmentProgramEnable(context: %p, enable: %" PRIu32 ")\n", context, enable);
    TAI_NEXT(sceGxmSetBackFragmentProgramEnable, sceGxmSetBackFragmentProgramEnableRef, context, enable);
}

CREATE_PATCHED_CALL(void, sceGxmSetBackLineFillLastPixelEnable, SceGxmContext *context, SceGxmLineFillLastPixelMode enable)
{
    LOGD("sceGxmSetBackLineFillLastPixelEnable(context: %p, enable: %" PRIu32 ")\n", context, enable);
    TAI_NEXT(sceGxmSetBackLineFillLastPixelEnable, sceGxmSetBackLineFillLastPixelEnableRef, context, enable);
}

CREATE_PATCHED_CALL(void, sceGxmSetBackPointLineWidth, SceGxmContext *context, unsigned int width)
{
    LOGD("sceGxmSetBackPointLineWidth(context: %p, width: %" PRIu32 ")\n", context, width);
    TAI_NEXT(sceGxmSetBackPointLineWidth, sceGxmSetBackPointLineWidthRef, context, width);
}

CREATE_PATCHED_CALL(void, sceGxmSetBackPolygonMode, SceGxmContext *context, SceGxmPolygonMode mode)
{
    LOGD("sceGxmSetBackPolygonMode(context: %p, mode: %" PRIu32 ")\n", context, mode);
    TAI_NEXT(sceGxmSetBackPolygonMode, sceGxmSetBackPolygonModeRef, context, mode);
}

CREATE_PATCHED_CALL(void, sceGxmSetBackStencilFunc, SceGxmContext *context, SceGxmStencilFunc func, SceGxmStencilOp stencilFail, SceGxmStencilOp depthFail, SceGxmStencilOp depthPass, unsigned char compareMask, unsigned char writeMask)
{
    LOGD("sceGxmSetBackStencilFunc(context: %p, func: %" PRIu32 ", func: %" PRIu32 ", stencilFail: %" PRIu32 ", depthFail: %" PRIu32 ", depthPass: %" PRIu32 ", compareMask: %" PRIu32 ", writeMask: %" PRIu32 ")\n", context, func, stencilFail, depthFail, depthPass, compareMask, writeMask);
    TAI_NEXT(sceGxmSetBackStencilFunc, sceGxmSetBackStencilFuncRef, context, func, stencilFail, depthFail, depthPass, compareMask, writeMask);
}

CREATE_PATCHED_CALL(void, sceGxmSetBackStencilRef, SceGxmContext *context, unsigned int sref)
{
    LOGD("sceGxmSetBackStencilRef(context: %p, sref: %" PRIu32 ")\n", context, sref);
    TAI_NEXT(sceGxmSetBackStencilRef, sceGxmSetBackStencilRefRef, context, sref);
}

CREATE_PATCHED_CALL(void, sceGxmSetBackVisibilityTestEnable, SceGxmContext *context, SceGxmVisibilityTestMode enable)
{
    LOGD("sceGxmSetBackVisibilityTestEnable(context: %p, enable: %" PRIu32 ")\n", context, enable);
    TAI_NEXT(sceGxmSetBackVisibilityTestEnable, sceGxmSetBackVisibilityTestEnableRef, context, enable);
}

CREATE_PATCHED_CALL(void, sceGxmSetBackVisibilityTestIndex, SceGxmContext *context, unsigned int index)
{
    LOGD("sceGxmSetBackVisibilityTestIndex(context: %p, index: %" PRIu32 ")\n", context, index);
    TAI_NEXT(sceGxmSetBackVisibilityTestIndex, sceGxmSetBackVisibilityTestIndexRef, context, index);
}

CREATE_PATCHED_CALL(void, sceGxmSetBackVisibilityTestOp, SceGxmContext *context, SceGxmVisibilityTestOp op)
{
    LOGD("sceGxmSetBackVisibilityTestOp(context: %p, op: %" PRIu32 ")\n", context, op);
    TAI_NEXT(sceGxmSetBackVisibilityTestOp, sceGxmSetBackVisibilityTestOpRef, context, op);
}

CREATE_PATCHED_CALL(void, sceGxmSetCullMode, SceGxmContext *context, SceGxmCullMode mode)
{
    LOGD("sceGxmSetCullMode(context: %p, mode: %" PRIu32 ")\n", context, mode);
    TAI_NEXT(sceGxmSetCullMode, sceGxmSetCullModeRef, context, mode);
}

void sceGxmSetDefaultRegionClipAndViewport(SceGxmContext *context, unsigned int xMax, unsigned int yMax);
CREATE_PATCHED_CALL(void, sceGxmSetDefaultRegionClipAndViewport, SceGxmContext *context, unsigned int xMax, unsigned int yMax)
{
    LOGD("sceGxmSetDefaultRegionClipAndViewport(context: %p, xMax: %" PRIu32 ", yMax: %" PRIu32 ")\n", context, xMax, yMax);
    TAI_NEXT(sceGxmSetDefaultRegionClipAndViewport, sceGxmSetDefaultRegionClipAndViewportRef, context, xMax, yMax);
}

int sceGxmSetDeferredContextFragmentBuffer(SceGxmContext *deferredContext, void *mem, unsigned int size);
CREATE_PATCHED_CALL(int, sceGxmSetDeferredContextFragmentBuffer, SceGxmContext *deferredContext, void *mem, unsigned int size)
{
    LOGD("sceGxmSetDeferredContextFragmentBuffer(deferredContext: %p, mem: %p, size: %" PRIu32 ")\n", deferredContext, mem, size);
    return TAI_NEXT(sceGxmSetDeferredContextFragmentBuffer, sceGxmSetDeferredContextFragmentBufferRef, deferredContext, mem, size);
}

int sceGxmSetDeferredContextVdmBuffer(SceGxmContext *deferredContext, void *mem, unsigned int size);
CREATE_PATCHED_CALL(int, sceGxmSetDeferredContextVdmBuffer, SceGxmContext *deferredContext, void *mem, unsigned int size)
{
    LOGD("sceGxmSetDeferredContextVdmBuffer(deferredContext: %p, mem: %p, size: %" PRIu32 ")\n", deferredContext, mem, size);
    return TAI_NEXT(sceGxmSetDeferredContextVdmBuffer, sceGxmSetDeferredContextVdmBufferRef, deferredContext, mem, size);
}

int sceGxmSetDeferredContextVertexBuffer(SceGxmContext *deferredContext, void *mem, unsigned int size);
CREATE_PATCHED_CALL(int, sceGxmSetDeferredContextVertexBuffer, SceGxmContext *deferredContext, void *mem, unsigned int size)
{
    LOGD("sceGxmSetDeferredContextVertexBuffer(deferredContext: %p, mem: %p, size: %" PRIu32 ")\n", deferredContext, mem, size);
    return TAI_NEXT(sceGxmSetDeferredContextVertexBuffer, sceGxmSetDeferredContextVertexBufferRef, deferredContext, mem, size);
}

int sceGxmSetFragmentDefaultUniformBuffer(SceGxmContext *context, const void *bufferData);
CREATE_PATCHED_CALL(int, sceGxmSetFragmentDefaultUniformBuffer, SceGxmContext *context, const void *bufferData)
{
    LOGD("sceGxmSetFragmentDefaultUniformBuffer(context: %p, bufferData: %p)\n", context, bufferData);
    return TAI_NEXT(sceGxmSetFragmentDefaultUniformBuffer, sceGxmSetFragmentDefaultUniformBufferRef, context, bufferData);
}

CREATE_PATCHED_CALL(void, sceGxmSetFragmentProgram, SceGxmContext *context, const SceGxmFragmentProgram *fragmentProgram)
{
    if (g_log) {
        uint32_t chunkSize = 0;

        GXMChunk type = GXMChunk::sceGxmSetFragmentProgram;
        g_fileoffset += g_file.write(type);
        g_fileoffset += g_file.write(chunkSize);
        g_fileoffset += g_file.write(context);
        g_fileoffset += g_file.write(fragmentProgram);

        g_fileoffset += ALIGN_TO_64(g_fileoffset);
    }

    LOGD("sceGxmSetFragmentProgram(context: %p, fragmentProgram: %p)\n", context, fragmentProgram);
    TAI_NEXT(sceGxmSetFragmentProgram, sceGxmSetFragmentProgramRef, context, fragmentProgram);
}

CREATE_PATCHED_CALL(int, sceGxmSetFragmentTexture, SceGxmContext *context, unsigned int textureIndex, const SceGxmTexture *texture)
{
    LOGD("sceGxmSetFragmentTexture(context: %p, textureIndex: %" PRIu32 ", texture: %p)\n", context, textureIndex, texture);
    void* data = sceGxmTextureGetData(texture);
    SceGxmTextureFormat format = sceGxmTextureGetFormat(texture);
    uint32_t width = sceGxmTextureGetWidth(texture);
    uint32_t height = sceGxmTextureGetHeight(texture);

    LOGD("\tuse texture: data: %p, textureBaseFormat: %s, textureSwizzleMode: %s, width: %" PRIu32 ", height: %" PRIu32 "\n", data, textureBaseFormat2str(format), textureSwizzleMode2str(format), width, height);
    return TAI_NEXT(sceGxmSetFragmentTexture, sceGxmSetFragmentTextureRef, context, textureIndex, texture);
}

CREATE_PATCHED_CALL(int, sceGxmSetFragmentUniformBuffer, SceGxmContext *context, unsigned int bufferIndex, const void *bufferData)
{
    LOGD("sceGxmSetFragmentUniformBuffer(context: %p, bufferIndex: %" PRIu32 ", bufferData: %p)\n", context, bufferIndex, bufferData);
    return TAI_NEXT(sceGxmSetFragmentUniformBuffer, sceGxmSetFragmentUniformBufferRef, context, bufferIndex, bufferData);
}

CREATE_PATCHED_CALL(void, sceGxmSetFrontDepthBias, SceGxmContext *context, int factor, int units)
{
    LOGD("sceGxmSetFrontDepthBias(context: %p, factor: %" PRIi32 ", units: %" PRIi32 ")\n", context, factor, units);
    TAI_NEXT(sceGxmSetFrontDepthBias, sceGxmSetFrontDepthBiasRef, context, factor, units);
}

CREATE_PATCHED_CALL(void, sceGxmSetFrontDepthFunc, SceGxmContext *context, SceGxmDepthFunc depthFunc)
{
    if (g_log) {
        uint32_t chunkSize = 0;

        GXMChunk type = GXMChunk::sceGxmSetFrontDepthFunc;
        g_fileoffset += g_file.write(type);
        g_fileoffset += g_file.write(chunkSize);
        g_fileoffset += g_file.write(context);
        g_fileoffset += g_file.write(depthFunc);

        g_fileoffset += ALIGN_TO_64(g_fileoffset);
    }

    LOGD("sceGxmSetFrontDepthFunc(context: %p, depthFunc: %" PRIu32 ")\n", context, depthFunc);
    TAI_NEXT(sceGxmSetFrontDepthFunc, sceGxmSetFrontDepthFuncRef, context, depthFunc);
}

CREATE_PATCHED_CALL(void, sceGxmSetFrontDepthWriteEnable, SceGxmContext *context, SceGxmDepthWriteMode enable)
{
    if (g_log) {
        uint32_t chunkSize = 0;

        GXMChunk type = GXMChunk::sceGxmSetFrontDepthWriteEnable;
        g_fileoffset += g_file.write(type);
        g_fileoffset += g_file.write(chunkSize);
        g_fileoffset += g_file.write(context);
        g_fileoffset += g_file.write(enable);

        g_fileoffset += ALIGN_TO_64(g_fileoffset);
    }

    LOGD("sceGxmSetFrontDepthWriteEnable(context: %p, enable: %" PRIu32 ")\n", context, enable);
    TAI_NEXT(sceGxmSetFrontDepthWriteEnable, sceGxmSetFrontDepthWriteEnableRef, context, enable);
}

CREATE_PATCHED_CALL(void, sceGxmSetFrontFragmentProgramEnable, SceGxmContext *context, SceGxmFragmentProgramMode enable)
{
    LOGD("sceGxmSetFrontFragmentProgramEnable(context: %p, enable: %" PRIu32 ")\n", context, enable);
    TAI_NEXT(sceGxmSetFrontFragmentProgramEnable, sceGxmSetFrontFragmentProgramEnableRef, context, enable);
}

CREATE_PATCHED_CALL(void, sceGxmSetFrontLineFillLastPixelEnable, SceGxmContext *context, SceGxmLineFillLastPixelMode enable)
{
    LOGD("sceGxmSetFrontLineFillLastPixelEnable(context: %p, enable: %" PRIu32 ")\n", context, enable);
    TAI_NEXT(sceGxmSetFrontLineFillLastPixelEnable, sceGxmSetFrontLineFillLastPixelEnableRef, context, enable);
}

CREATE_PATCHED_CALL(void, sceGxmSetFrontPointLineWidth, SceGxmContext *context, unsigned int width)
{
    LOGD("sceGxmSetFrontPointLineWidth(context: %p, width: %" PRIu32 ")\n", context, width);
    TAI_NEXT(sceGxmSetFrontPointLineWidth, sceGxmSetFrontPointLineWidthRef, context, width);
}

CREATE_PATCHED_CALL(void, sceGxmSetFrontPolygonMode, SceGxmContext *context, SceGxmPolygonMode mode)
{
    LOGD("sceGxmSetFrontPolygonMode(context: %p, mode: %" PRIu32 ")\n", context, mode);
    TAI_NEXT(sceGxmSetFrontPolygonMode, sceGxmSetFrontPolygonModeRef, context, mode);
}

CREATE_PATCHED_CALL(void, sceGxmSetFrontStencilFunc, SceGxmContext *context, SceGxmStencilFunc func, SceGxmStencilOp stencilFail, SceGxmStencilOp depthFail, SceGxmStencilOp depthPass, unsigned char compareMask, unsigned char writeMask)
{
    if (g_log) {
        uint32_t chunkSize = 0;

        GXMChunk type = GXMChunk::sceGxmSetFrontStencilFunc;
        g_fileoffset += g_file.write(type);
        g_fileoffset += g_file.write(chunkSize);
        g_fileoffset += g_file.write(context);
        g_fileoffset += g_file.write((uint32_t)func);
        g_fileoffset += g_file.write((uint32_t)stencilFail);
        g_fileoffset += g_file.write((uint32_t)depthFail);
        g_fileoffset += g_file.write((uint32_t)depthPass);
        g_fileoffset += g_file.write(compareMask);
        g_fileoffset += g_file.write(writeMask);

        g_fileoffset += ALIGN_TO_64(g_fileoffset);
    }

    LOGD("sceGxmSetFrontStencilFunc(context: %p, func: %" PRIu32 ", stencilFail: %" PRIu32 ", depthFail: %" PRIu32 ", depthPass: %" PRIu32 ", compareMask: %" PRIu8 ", writeMask: %" PRIu8 ")\n", context, func, stencilFail, depthFail, depthPass, compareMask, writeMask);
    TAI_NEXT(sceGxmSetFrontStencilFunc, sceGxmSetFrontStencilFuncRef, context, func, stencilFail, depthFail, depthPass, compareMask, writeMask);
}

CREATE_PATCHED_CALL(void, sceGxmSetFrontStencilRef, SceGxmContext *context, unsigned int sref)
{
    if (g_log) {
        uint32_t chunkSize = 0;

        GXMChunk type = GXMChunk::sceGxmSetFrontStencilRef;
        g_fileoffset += g_file.write(type);
        g_fileoffset += g_file.write(chunkSize);
        g_fileoffset += g_file.write(context);
        g_fileoffset += g_file.write(sref);

        g_fileoffset += ALIGN_TO_64(g_fileoffset);
    }

    LOGD("sceGxmSetFrontStencilRef(context: %p, sref: %" PRIu32 ")\n", context, sref);
    TAI_NEXT(sceGxmSetFrontStencilRef, sceGxmSetFrontStencilRefRef, context, sref);
}

CREATE_PATCHED_CALL(void, sceGxmSetFrontVisibilityTestEnable, SceGxmContext *context, SceGxmVisibilityTestMode enable)
{
    LOGD("sceGxmSetFrontVisibilityTestEnable(context: %p, enable: %" PRIu32 ")\n", context, enable);
    TAI_NEXT(sceGxmSetFrontVisibilityTestEnable, sceGxmSetFrontVisibilityTestEnableRef, context, enable);
}

CREATE_PATCHED_CALL(void, sceGxmSetFrontVisibilityTestIndex, SceGxmContext *context, unsigned int index)
{
    LOGD("sceGxmSetFrontVisibilityTestIndex(context: %p, index: %" PRIu32 ")\n", context, index);
    TAI_NEXT(sceGxmSetFrontVisibilityTestIndex, sceGxmSetFrontVisibilityTestIndexRef, context, index);
}

CREATE_PATCHED_CALL(void, sceGxmSetFrontVisibilityTestOp, SceGxmContext *context, SceGxmVisibilityTestOp op)
{
    LOGD("sceGxmSetFrontVisibilityTestOp(context: %p, op: %" PRIu32 ")\n", context, op);
    TAI_NEXT(sceGxmSetFrontVisibilityTestOp, sceGxmSetFrontVisibilityTestOpRef, context, op);
}

CREATE_PATCHED_CALL(void, sceGxmSetPrecomputedFragmentState, SceGxmContext *context, const SceGxmPrecomputedFragmentState *precomputedState)
{
    LOGD("sceGxmSetPrecomputedFragmentState(context: %p, precomputedState: %p)\n", context, precomputedState);
    TAI_NEXT(sceGxmSetPrecomputedFragmentState, sceGxmSetPrecomputedFragmentStateRef, context, precomputedState);
}

CREATE_PATCHED_CALL(void, sceGxmSetPrecomputedVertexState, SceGxmContext *context, const SceGxmPrecomputedVertexState *precomputedState)
{
    LOGD("sceGxmSetPrecomputedVertexState(context: %p, precomputedState: %p)\n", context, precomputedState);
    TAI_NEXT(sceGxmSetPrecomputedVertexState, sceGxmSetPrecomputedVertexStateRef, context, precomputedState);
}

CREATE_PATCHED_CALL(void, sceGxmSetRegionClip, SceGxmContext *context, SceGxmRegionClipMode mode, unsigned int xMin, unsigned int yMin, unsigned int xMax, unsigned int yMax)
{
    LOGD("sceGxmSetRegionClip(context: %p, mode: %" PRIu32 ", xMin: %" PRIu32 ", yMin: %" PRIu32 ", xMax: %" PRIu32 ", yMax: %" PRIu32 ")\n", context, mode, xMin, yMin, xMax, yMax);
    TAI_NEXT(sceGxmSetRegionClip, sceGxmSetRegionClipRef, context, mode, xMin, yMin, xMax, yMax);
}

CREATE_PATCHED_CALL(void, sceGxmSetTwoSidedEnable, SceGxmContext *context, SceGxmTwoSidedMode enable)
{
    LOGD("sceGxmSetTwoSidedEnable(context: %p, enable: %" PRIu32 ")\n", context, enable);
    TAI_NEXT(sceGxmSetTwoSidedEnable, sceGxmSetTwoSidedEnableRef, context, enable);
}

CREATE_PATCHED_CALL(int, sceGxmSetUniformDataF, void *uniformBuffer, const SceGxmProgramParameter *parameter, unsigned int componentOffset, unsigned int componentCount, const float *sourceData)
{
    if (g_log) {
        uint32_t chunkSize = 0;

        GXMChunk type = GXMChunk::sceGxmSetUniformDataF;
        g_fileoffset += g_file.write(type);
        g_fileoffset += g_file.write(chunkSize);
        g_fileoffset += g_file.write(uniformBuffer);
        g_fileoffset += g_file.write(parameter);
        g_fileoffset += g_file.write(componentOffset);
        g_fileoffset += g_file.write(componentCount);
        g_fileoffset += g_file.write(sourceData);

        g_fileoffset += ALIGN_TO_64(g_fileoffset);
    }

    LOGD("sceGxmSetUniformDataF(uniformBuffer, %p, parameter: %p, componentOffset: %" PRIu32 ", componentCount: %" PRIu32 ", sourceData: %p)\n", uniformBuffer, parameter, componentOffset, componentCount, sourceData);
    return TAI_NEXT(sceGxmSetUniformDataF, sceGxmSetUniformDataFRef, uniformBuffer, parameter, componentOffset, componentCount, sourceData);
}

CREATE_PATCHED_CALL(int, sceGxmSetUserMarker, SceGxmContext *context, const char *tag)
{
    LOGD("sceGxmSetUserMarker(context: %p, tag: %s)\n", context, tag);
    return TAI_NEXT(sceGxmSetUserMarker, sceGxmSetUserMarkerPatched, context, tag);
}

CREATE_PATCHED_CALL(void, sceGxmSetValidationEnable, SceGxmContext *context, SceBool enable)
{
    LOGD("sceGxmSetValidationEnable(context: %p, enable: %s)\n", context, enable ? "true" : "false");
    TAI_NEXT(sceGxmSetValidationEnable, sceGxmSetValidationEnableRef, context, enable);
}

int sceGxmSetVertexDefaultUniformBuffer(SceGxmContext *context, const void *bufferData);
CREATE_PATCHED_CALL(int, sceGxmSetVertexDefaultUniformBuffer, SceGxmContext *context, const void *bufferData)
{
    LOGD("sceGxmSetVertexDefaultUniformBuffer(context: %p, bufferData: %p)\n", context, bufferData);
    return TAI_NEXT(sceGxmSetVertexDefaultUniformBuffer, sceGxmSetVertexDefaultUniformBufferRef, context, bufferData);
}

CREATE_PATCHED_CALL(void, sceGxmSetVertexProgram, SceGxmContext *context, const SceGxmVertexProgram *vertexProgram)
{
    if (g_log) {
        uint32_t chunkSize = 0;

        GXMChunk type = GXMChunk::sceGxmSetVertexProgram;
        g_fileoffset += g_file.write(type);
        g_fileoffset += g_file.write(chunkSize);
        g_fileoffset += g_file.write(context);
        g_fileoffset += g_file.write(vertexProgram);

        VertexProgramResource vertexRes;
        g_resource_manager.find(GXMType::SceGxmVertexProgram, (uint32_t)vertexProgram, &vertexRes);

        g_fileoffset += g_file.write(vertexRes.attributeCount);

        LOGD("sceGxmSetVertexProgram, active vertex program: attribute count %" PRIu32 "\n", vertexRes.attributeCount);
        for (uint32_t attribute_index = 0; attribute_index < vertexRes.attributeCount; ++attribute_index) {
            SceGxmVertexAttribute& attribute = vertexRes.attributes[attribute_index];
            LOGD("\tattribute: stream index: %" PRIu16 ", offset: %" PRIu16 ", format %s, componentCount: %" PRIu8 ", regIndex: %" PRIu16 "\n", attribute.streamIndex, attribute.offset, attributeFormat2str(attribute.format), attribute.componentCount, attribute.regIndex);
            g_fileoffset += g_file.write((uint32_t)attribute.format);
            g_fileoffset += g_file.write(attribute.componentCount);
            g_fileoffset += g_file.write(attribute.offset);
            g_fileoffset += g_file.write(attribute.streamIndex);
            g_fileoffset += g_file.write(attribute.regIndex);
        }

        LOGD("sceGxmSetVertexProgram, streamCount: %" PRIu32 "\n", vertexRes.streamCount);

        for (uint32_t stream_index = 0; stream_index < vertexRes.streamCount; ++stream_index) {
            SceGxmVertexStream& stream = vertexRes.streams[stream_index];
            LOGD("\tstream: stride %" PRIu16 ", indexSource: %" PRIu16 "\n", stream.stride, stream.indexSource);

            g_activeVertexStreams[stream_index].indexSource = stream.indexSource;
            g_activeVertexStreams[stream_index].stride = stream.stride;
        }

        g_fileoffset += ALIGN_TO_64(g_fileoffset);

        g_activeStreamCount = vertexRes.streamCount;
    }

    LOGD("sceGxmSetVertexProgram(context: %p, vertexProgram: %p)\n", context, vertexProgram);
    g_activeVertexProgram = vertexProgram;

    TAI_NEXT(sceGxmSetVertexProgram, sceGxmSetVertexProgramRef, context, vertexProgram);
}

CREATE_PATCHED_CALL(int, sceGxmSetVertexStream, SceGxmContext *context, unsigned int streamIndex, const void *streamData)
{
    if (g_log) {
        uint32_t chunkSize = 0;

        GXMChunk type = GXMChunk::sceGxmSetVertexStream;
        g_fileoffset += g_file.write(type);
        g_fileoffset += g_file.write(chunkSize);
        g_fileoffset += g_file.write(context);
        g_fileoffset += g_file.write(streamIndex);
        g_fileoffset += g_file.write(streamData);

        g_fileoffset += ALIGN_TO_64(g_fileoffset);
    }

    LOGD("sceGxmSetVertexStream(context: %p, streamIndex: %" PRIu32 ", streamData: %p)\n", context, streamIndex, streamData);
    g_activeVertexStreams[streamIndex].dataPtr = streamData;

    return TAI_NEXT(sceGxmSetVertexStream, sceGxmSetVertexStreamRef, context, streamIndex, streamData);
}

CREATE_PATCHED_CALL(int, sceGxmSetVertexTexture, SceGxmContext *context, unsigned int textureIndex, const SceGxmTexture *texture)
{
    LOGD("sceGxmSetVertexTexture(context: %p, textureIndex: %" PRIu32 ", texture: %p)\n", context, textureIndex, texture);
    return TAI_NEXT(sceGxmSetVertexTexture, sceGxmSetVertexTextureRef, context, textureIndex, texture);
}

CREATE_PATCHED_CALL(int, sceGxmSetVertexUniformBuffer, SceGxmContext *context, unsigned int bufferIndex, const void *bufferData)
{
    LOGD("sceGxmSetVertexUniformBuffer(context: %p, bufferIndex: %" PRIu32 ", bufferData: %p)\n", context, bufferIndex, bufferData);
    return TAI_NEXT(sceGxmSetVertexUniformBuffer, sceGxmSetVertexUniformBufferRef, context, bufferIndex, bufferData);
}

CREATE_PATCHED_CALL(void, sceGxmSetViewport, SceGxmContext *context, float xOffset, float xScale, float yOffset, float yScale, float zOffset, float zScale)
{
    LOGD("sceGxmSetViewport(context: %p, xOffset: %f, xScale: %f, yOffset: %f, yScale: %f, zOffset: %f, zScale: %f)\n", context, xOffset, xScale, yOffset, yScale, zOffset, zScale);
    TAI_NEXT(sceGxmSetViewport, sceGxmSetViewportRef, context, xOffset, xScale, yOffset, yScale, zOffset, zScale);
}

CREATE_PATCHED_CALL(void, sceGxmSetViewportEnable, SceGxmContext *context, SceGxmViewportMode enable)
{
    LOGD("sceGxmSetViewportEnable(context: %p, enable: %" PRIu32 ")\n", context, enable);
    TAI_NEXT(sceGxmSetViewportEnable, sceGxmSetViewportEnableRef, context, enable);
}

CREATE_PATCHED_CALL(int, sceGxmSetVisibilityBuffer, SceGxmContext *context, void *bufferBase, unsigned int stridePerCore)
{
    LOGD("sceGxmSetVisibilityBuffer(context: %p, bufferBase: %p, stridePerCore: %" PRIu32 ")\n", context, bufferBase, stridePerCore);
    return TAI_NEXT(sceGxmSetVisibilityBuffer, sceGxmSetVisibilityBufferRef, context, bufferBase, stridePerCore);
}

CREATE_PATCHED_CALL(void, sceGxmSetWBufferEnable, SceGxmContext *context, SceGxmWBufferMode enable)
{
    LOGD("sceGxmSetWBufferEnable(context: %p, enable: %" PRIu32 ")\n", context, enable);
    TAI_NEXT(sceGxmSetWBufferEnable, sceGxmSetWBufferEnableRef, context, enable);
}

CREATE_PATCHED_CALL(void, sceGxmSetWClampEnable, SceGxmContext *context, SceGxmWClampMode enable)
{
    LOGD("sceGxmSetWClampEnable(context: %p, enable: %" PRIu32 ")\n", context, enable);
    TAI_NEXT(sceGxmSetWClampEnable, sceGxmSetWClampEnableRef, context, enable);
}

CREATE_PATCHED_CALL(void, sceGxmSetWClampValue, SceGxmContext *context, float clampValue)
{
    LOGD("sceGxmSetWClampValue(context: %p, clampValue: %f)\n", context, clampValue);
    TAI_NEXT(sceGxmSetWClampValue, sceGxmSetWClampValueRef, context, clampValue);
}

int sceGxmSetWarningEnabled(SceGxmWarning warning, SceBool enable);
CREATE_PATCHED_CALL(int, sceGxmSetWarningEnabled, SceGxmWarning warning, SceBool enable)
{
    LOGD("sceGxmSetWarningEnabled(warning: %" PRIu32 ", enable: %s)\n", warning, enable ? "true" : "false");
    return TAI_NEXT(sceGxmSetWarningEnabled, sceGxmSetWarningEnabledRef, warning, enable);
}

int sceGxmSetYuvProfile(SceGxmContext *immediateContext, unsigned int cscIndex, SceGxmYuvProfile profile);
CREATE_PATCHED_CALL(int, sceGxmSetYuvProfile, SceGxmContext *immediateContext, unsigned int cscIndex, SceGxmYuvProfile profile)
{
    LOGD("sceGxmSetYuvProfile(immediateContext: %p, cscIndex: %" PRIu32 ", profile: %" PRIu32 ")\n", immediateContext, cscIndex, profile);
    return TAI_NEXT(sceGxmSetYuvProfile, sceGxmSetYuvProfileRef, immediateContext, cscIndex, profile);
}

CREATE_PATCHED_CALL(int, sceGxmShaderPatcherAddRefFragmentProgram, SceGxmShaderPatcher *shaderPatcher, SceGxmFragmentProgram *fragmentProgram)
{
    LOGD("sceGxmShaderPatcherAddRefFragmentProgram(shaderPatcher: %p, fragmentProgram: %p)\n", shaderPatcher, fragmentProgram);
    return TAI_NEXT(sceGxmShaderPatcherAddRefFragmentProgram, sceGxmShaderPatcherAddRefFragmentProgramRef, shaderPatcher, fragmentProgram);
}

CREATE_PATCHED_CALL(int, sceGxmShaderPatcherAddRefVertexProgram, SceGxmShaderPatcher *shaderPatcher, SceGxmVertexProgram *vertexProgram)
{
    LOGD("sceGxmShaderPatcherAddRefVertexProgram(shaderPatcher: %p, vertexProgram: %p)\n", shaderPatcher, vertexProgram);
    return TAI_NEXT(sceGxmShaderPatcherAddRefVertexProgram, sceGxmShaderPatcherAddRefVertexProgramRef, shaderPatcher, vertexProgram);
}

CREATE_PATCHED_CALL(int, sceGxmShaderPatcherCreate, const SceGxmShaderPatcherParams *params, SceGxmShaderPatcher **shaderPatcher)
{
    LOGD("sceGxmShaderPatcherCreate(params: %p, shaderPatcher: %p)\n", params, shaderPatcher);
    return TAI_NEXT(sceGxmShaderPatcherCreate, sceGxmShaderPatcherCreateRef, params, shaderPatcher);
}

CREATE_PATCHED_CALL(int, sceGxmShaderPatcherCreateFragmentProgram, SceGxmShaderPatcher *shaderPatcher, SceGxmShaderPatcherId programId, SceGxmOutputRegisterFormat outputFormat, SceGxmMultisampleMode multisampleMode, const SceGxmBlendInfo *blendInfo, const SceGxmProgram *vertexProgram, SceGxmFragmentProgram **fragmentProgram)
{
    int res = TAI_NEXT(sceGxmShaderPatcherCreateFragmentProgram, sceGxmShaderPatcherCreateFragmentProgramRef, shaderPatcher, programId, outputFormat, multisampleMode, blendInfo, vertexProgram, fragmentProgram);
    
    if (fragmentProgram && !g_resource_manager.contains((uint32_t)*fragmentProgram)) {
        GXMType type = GXMType::SceGxmFragmentProgram;

        FragmentProgramResource fragmentProg;
        fragmentProg.type = type;
        fragmentProg.id = (uint32_t)*fragmentProgram;
        
        fragmentProg.fragmentProgram = *fragmentProgram;
        fragmentProg.size += sizeof(*fragmentProgram);
        fragmentProg.programId = programId;
        fragmentProg.size += sizeof(programId);
        fragmentProg.outputFormat = outputFormat;
        fragmentProg.size += sizeof(outputFormat);
        fragmentProg.multisampleMode = multisampleMode;
        fragmentProg.size += sizeof(multisampleMode);
        fragmentProg.vertexProgram = vertexProgram;
        fragmentProg.size += sizeof(vertexProgram);
        if (blendInfo) {
            fragmentProg.hasBlendInfo = 1;
            fragmentProg.blendInfo = *blendInfo;
            fragmentProg.size += sizeof(*blendInfo);
        }
        fragmentProg.size += sizeof(uint32_t); // for hasBlendInfo

        g_resource_manager.add(&fragmentProg);
    }
    LOGD("sceGxmShaderPatcherCreateFragmentProgram(shaderPatcher: %p, programId: %p, outputFormat: %" PRIu32 ", multisampleMode: %" PRIu32 ", blendInfo: %p, vertexProgram: %p, fragmentProgram: %p)\n", shaderPatcher, programId, outputFormat, multisampleMode, blendInfo, vertexProgram, *fragmentProgram);
    return res;
}

CREATE_PATCHED_CALL(int, sceGxmShaderPatcherCreateMaskUpdateFragmentProgram, SceGxmShaderPatcher *shaderPatcher, SceGxmFragmentProgram **fragmentProgram)
{
    LOGD("sceGxmShaderPatcherCreateMaskUpdateFragmentProgram(shaderPatcher: %p, fragmentProgram: %p)\n", shaderPatcher, fragmentProgram);
    return TAI_NEXT(sceGxmShaderPatcherCreateMaskUpdateFragmentProgram, sceGxmShaderPatcherCreateMaskUpdateFragmentProgramRef, shaderPatcher, fragmentProgram);
}

CREATE_PATCHED_CALL(int, sceGxmShaderPatcherCreateVertexProgram, SceGxmShaderPatcher *shaderPatcher, SceGxmShaderPatcherId programId, const SceGxmVertexAttribute *attributes, unsigned int attributeCount, const SceGxmVertexStream *streams, unsigned int streamCount, SceGxmVertexProgram **vertexProgram)
{
    int res = TAI_NEXT(sceGxmShaderPatcherCreateVertexProgram, sceGxmShaderPatcherCreateVertexProgramRef, shaderPatcher, programId, attributes, attributeCount, streams, streamCount, vertexProgram);

    if (vertexProgram && !g_resource_manager.contains((uint32_t)*vertexProgram)) {
        GXMType type = GXMType::SceGxmVertexProgram;
        
        VertexProgramResource vertexProg;
        vertexProg.type = type;
        vertexProg.id = (uint32_t)*vertexProgram;
        
        vertexProg.vertexProgram = *vertexProgram;
        vertexProg.size += sizeof(*vertexProgram);
        vertexProg.programId = programId;
        vertexProg.size += sizeof(programId);
        vertexProg.attributeCount = attributeCount;
        vertexProg.size += sizeof(attributeCount);
        memcpy(vertexProg.attributes, attributes, sizeof(SceGxmVertexAttribute) * attributeCount);
        vertexProg.size += sizeof(SceGxmVertexAttribute) * attributeCount;
        vertexProg.streamCount = streamCount;
        vertexProg.size += sizeof(streamCount);
        memcpy(vertexProg.streams, streams, sizeof(SceGxmVertexStream) * streamCount);
        vertexProg.size += sizeof(SceGxmVertexStream) * streamCount;
        
        g_resource_manager.add(&vertexProg);
    }

    LOGD("sceGxmShaderPatcherCreateVertexProgram(shaderPatcher: %p, programId: %p, attributes: %p, attributeCount: %" PRIu32 ", streams: %p, streamCount: %" PRIu32 ", vertexProgram: %p)\n", shaderPatcher, programId, attributes, attributeCount, streams, streamCount, *vertexProgram);
    return res;
}

CREATE_PATCHED_CALL(int, sceGxmShaderPatcherDestroy, SceGxmShaderPatcher *shaderPatcher)
{
    LOGD("sceGxmShaderPatcherDestroy(shaderPatcher: %p)\n", shaderPatcher);
    return TAI_NEXT(sceGxmShaderPatcherDestroy, sceGxmShaderPatcherDestroyRef, shaderPatcher);
}

CREATE_PATCHED_CALL(int, sceGxmShaderPatcherForceUnregisterProgram, SceGxmShaderPatcher *shaderPatcher, SceGxmShaderPatcherId programId)
{
    LOGD("sceGxmShaderPatcherForceUnregisterProgram(shaderPatcher: %p, programId: %p)\n", shaderPatcher, programId);
    return TAI_NEXT(sceGxmShaderPatcherForceUnregisterProgram, sceGxmShaderPatcherForceUnregisterProgramRef, shaderPatcher, programId);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmShaderPatcherGetBufferMemAllocated, const SceGxmShaderPatcher *shaderPatcher)
{
    LOGD("sceGxmShaderPatcherGetBufferMemAllocated(shaderPatcher: %p)\n", shaderPatcher);
    return TAI_NEXT(sceGxmShaderPatcherGetBufferMemAllocated, sceGxmShaderPatcherGetBufferMemAllocatedRef, shaderPatcher);
}

CREATE_PATCHED_CALL(int, sceGxmShaderPatcherGetFragmentProgramRefCount, SceGxmShaderPatcher *shaderPatcher, SceGxmFragmentProgram *fragmentProgram, unsigned int *count)
{
    LOGD("sceGxmShaderPatcherGetFragmentProgramRefCount(shaderPatcher: %p, fragmentProgram: %p, count: %p)\n", shaderPatcher, fragmentProgram, count);
    return TAI_NEXT(sceGxmShaderPatcherGetFragmentProgramRefCount, sceGxmShaderPatcherGetFragmentProgramRefCountRef, shaderPatcher, fragmentProgram, count);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmShaderPatcherGetFragmentUsseMemAllocated, const SceGxmShaderPatcher *shaderPatcher)
{
    LOGD("sceGxmShaderPatcherGetFragmentUsseMemAllocated(shaderPatcher: %p)\n", shaderPatcher);
    return TAI_NEXT(sceGxmShaderPatcherGetFragmentUsseMemAllocated, sceGxmShaderPatcherGetFragmentUsseMemAllocatedRef, shaderPatcher);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmShaderPatcherGetHostMemAllocated, const SceGxmShaderPatcher *shaderPatcher)
{
    LOGD("sceGxmShaderPatcherGetHostMemAllocated(shaderPatcher: %p)\n", shaderPatcher);
    return TAI_NEXT(sceGxmShaderPatcherGetHostMemAllocated, sceGxmShaderPatcherGetHostMemAllocatedRef, shaderPatcher);
}

CREATE_PATCHED_CALL(const SceGxmProgram *, sceGxmShaderPatcherGetProgramFromId, SceGxmShaderPatcherId programId)
{
    LOGD("sceGxmShaderPatcherGetProgramFromId(programId: %p)\n", programId);
    return TAI_NEXT(sceGxmShaderPatcherGetProgramFromId, sceGxmShaderPatcherGetProgramFromIdRef, programId);
}

CREATE_PATCHED_CALL(void *, sceGxmShaderPatcherGetUserData, SceGxmShaderPatcher *shaderPatcher)
{
    LOGD("sceGxmShaderPatcherGetUserData(shaderPatcher: %p)\n", shaderPatcher);
    return TAI_NEXT(sceGxmShaderPatcherGetUserData, sceGxmShaderPatcherGetUserDataRef, shaderPatcher);
}

CREATE_PATCHED_CALL(int, sceGxmShaderPatcherGetVertexProgramRefCount, SceGxmShaderPatcher *shaderPatcher, SceGxmVertexProgram *vertexProgram, unsigned int *count)
{
    LOGD("sceGxmShaderPatcherGetVertexProgramRefCount(shaderPatcher: %p, fragmentProgram: %p, count: %p)\n", shaderPatcher, vertexProgram, count);
    return TAI_NEXT(sceGxmShaderPatcherGetVertexProgramRefCount, sceGxmShaderPatcherGetVertexProgramRefCountRef, shaderPatcher, vertexProgram, count);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmShaderPatcherGetVertexUsseMemAllocated, const SceGxmShaderPatcher *shaderPatcher)
{
    LOGD("sceGxmShaderPatcherGetVertexUsseMemAllocated(shaderPatcher: %p)\n", shaderPatcher);
    return TAI_NEXT(sceGxmShaderPatcherGetVertexUsseMemAllocated, sceGxmShaderPatcherGetVertexUsseMemAllocatedRef, shaderPatcher);
}

static uint32_t shadercount = 0;

CREATE_PATCHED_CALL(int, sceGxmShaderPatcherRegisterProgram, SceGxmShaderPatcher *shaderPatcher, const SceGxmProgram *programHeader, SceGxmShaderPatcherId *programId)
{
    int res = TAI_NEXT(sceGxmShaderPatcherRegisterProgram, sceGxmShaderPatcherRegisterProgramRef, shaderPatcher, programHeader, programId);

    if (g_log && programHeader) {
        ++shadercount;
        ProgramHeader* header = (ProgramHeader*)programHeader;
        GXMType type = GXMType::SceGxmProgram;

        LOGD("program #%" PRIu32" header: magic: %s, version: %" PRIu32 ", byte length: %" PRIu32 ", padded length: %" PRIu32 ", id_size: %" PRIu32 ", id: %" PRIu32 "\n", shadercount, header->magic, header->version, header->length, header->length + (4 - (header->length % 4)), sizeof(SceGxmShaderPatcherId), *programId);

        ProgramResource programRes;
        programRes.type = type;
        programRes.id = (uint32_t)*programId;

        programRes.programId = *programId;
        programRes.size += sizeof(*programId);
        programRes.programLength = header->length + (4 - (header->length % 4));
        programRes.size += sizeof(uint32_t);
        programRes.program = programHeader;
        programRes.size += programRes.programLength;

        uint32_t parameter_count = sceGxmProgramGetParameterCount(programHeader);
        LOGD("Register program: parameter count %" PRIu32 "\n", parameter_count);

        for (uint32_t parameter_index = 0; parameter_index < parameter_count; ++parameter_index) {
            SceGxmProgramParameter const * parameter = sceGxmProgramGetParameter(programHeader, parameter_index);
            char const* name = sceGxmProgramParameterGetName(parameter);
            uint32_t resIndex = sceGxmProgramParameterGetResourceIndex(parameter);
            LOGD("Parameter[%" PRIu32 "] name: %s, resource index: %" PRIu32 "\n", parameter_index, name, resIndex);
        }

        g_resource_manager.add(&programRes);
    }

    LOGD("sceGxmShaderPatcherRegisterProgram(shaderPatcher: %p, programHeader: %p, programId: %p)\n", shaderPatcher, programHeader, programId);
    return res;
}

CREATE_PATCHED_CALL(int, sceGxmShaderPatcherReleaseFragmentProgram, SceGxmShaderPatcher *shaderPatcher, SceGxmFragmentProgram *fragmentProgram)
{
    LOGD("sceGxmShaderPatcherReleaseFragmentProgram(shaderPatcher: %p, fragmentProgram: %p)\n", shaderPatcher, fragmentProgram);
    return TAI_NEXT(sceGxmShaderPatcherReleaseFragmentProgram, sceGxmShaderPatcherReleaseFragmentProgramRef, shaderPatcher, fragmentProgram);
}

CREATE_PATCHED_CALL(int, sceGxmShaderPatcherReleaseVertexProgram, SceGxmShaderPatcher *shaderPatcher, SceGxmVertexProgram *vertexProgram)
{
    LOGD("sceGxmShaderPatcherReleaseVertexProgram(shaderPatcher: %p, fragmentProgram: %p)\n", shaderPatcher, vertexProgram);
    return TAI_NEXT(sceGxmShaderPatcherReleaseVertexProgram, sceGxmShaderPatcherReleaseVertexProgramRef, shaderPatcher, vertexProgram);
}

CREATE_PATCHED_CALL(int, sceGxmShaderPatcherSetAuxiliarySurface, SceGxmShaderPatcher *shaderPatcher, unsigned int auxSurfaceIndex, const SceGxmAuxiliarySurface *auxSurface)
{
    LOGD("sceGxmShaderPatcherSetAuxiliarySurface(shaderPatcher: %p, auxSurfaceIndex: %" PRIu32 ", auxSurface: %p)\n", shaderPatcher, auxSurfaceIndex, auxSurfaceIndex);
    return TAI_NEXT(sceGxmShaderPatcherSetAuxiliarySurface, sceGxmShaderPatcherSetAuxiliarySurfaceRef, shaderPatcher, auxSurfaceIndex, auxSurface);
}

CREATE_PATCHED_CALL(int, sceGxmShaderPatcherSetUserData, SceGxmShaderPatcher *shaderPatcher, void *userData)
{
    LOGD("sceGxmShaderPatcherSetUserData(shaderPatcher: %p, userData: %p)\n", shaderPatcher, userData);
    return TAI_NEXT(sceGxmShaderPatcherSetUserData, sceGxmShaderPatcherSetUserDataRef, shaderPatcher, userData);
}

CREATE_PATCHED_CALL(int, sceGxmShaderPatcherUnregisterProgram, SceGxmShaderPatcher *shaderPatcher, SceGxmShaderPatcherId programId)
{
    LOGD("sceGxmShaderPatcherUnregisterProgram(shaderPatcher: %p, programId: %p)\n", shaderPatcher, programId);
    return TAI_NEXT(sceGxmShaderPatcherUnregisterProgram, sceGxmShaderPatcherUnregisterProgramRef, shaderPatcher, programId);
}

CREATE_PATCHED_CALL(int, sceGxmSyncObjectCreate, SceGxmSyncObject **syncObject)
{
    LOGD("sceGxmSyncObjectCreate(syncObject: %p)\n", syncObject);
    return TAI_NEXT(sceGxmSyncObjectCreate, sceGxmSyncObjectCreateRef, syncObject);
}

CREATE_PATCHED_CALL(int, sceGxmSyncObjectDestroy, SceGxmSyncObject *syncObject)
{
    LOGD("sceGxmSyncObjectDestroy(syncObject: %p)\n", syncObject);
    return TAI_NEXT(sceGxmSyncObjectDestroy, sceGxmSyncObjectDestroyRef, syncObject);
}

CREATE_PATCHED_CALL(int, sceGxmTerminate)
{
    LOGD("sceGxmTerminate()\n");
    return TAI_NEXT(sceGxmTerminate, sceGxmTerminateRef);
}

CREATE_PATCHED_CALL(int, sceGxmTextureInitCube, SceGxmTexture *texture, const void *data, SceGxmTextureFormat texFormat, unsigned int width, unsigned int height, unsigned int mipCount)
{
    LOGD("sceGxmTextureInitCube(texture: %p, data: %p, texBaseFormat: %s, texSwizzleMode: %s, width: %" PRIu32 ", height: %" PRIu32 ", mipCount: %" PRIu32 ")\n", texture, data, textureBaseFormat2str(texFormat), textureSwizzleMode2str(texFormat), width, height, mipCount);
    return TAI_NEXT(sceGxmTextureInitCube, sceGxmTextureInitCubeRef, texture, data, texFormat, width, height, mipCount);
}

int sceGxmTextureInitCubeArbitrary(SceGxmTexture *texture, const void *data, SceGxmTextureFormat texFormat, unsigned int width, unsigned int height, unsigned int mipCount);
CREATE_PATCHED_CALL(int, sceGxmTextureInitCubeArbitrary, SceGxmTexture *texture, const void *data, SceGxmTextureFormat texFormat, unsigned int width, unsigned int height, unsigned int mipCount)
{
    LOGD("sceGxmTextureInitCubeArbitrary(texture: %p, data: %p, texBaseFormat: %s, texSwizzleMode: %s, width: %" PRIu32 ", height: %" PRIu32 ", mipCount: %" PRIu32 ")\n", texture, data, textureBaseFormat2str(texFormat), textureSwizzleMode2str(texFormat), width, height, mipCount);
    return TAI_NEXT(sceGxmTextureInitCubeArbitrary, sceGxmTextureInitCubeArbitraryRef, texture, data, texFormat, width, height, mipCount);
}

CREATE_PATCHED_CALL(int, sceGxmTextureInitLinear, SceGxmTexture *texture, const void *data, SceGxmTextureFormat texFormat, unsigned int width, unsigned int height, unsigned int mipCount)
{
    LOGD("sceGxmTextureInitLinear(texture: %p, data: %p, texBaseFormat: %s, texSwizzleMode: %s, width: %" PRIu32 ", height: %" PRIu32 ", mipCount: %" PRIu32 ")\n", texture, data, textureBaseFormat2str(texFormat), textureSwizzleMode2str(texFormat), width, height, mipCount);
    return TAI_NEXT(sceGxmTextureInitLinear, sceGxmTextureInitLinearRef, texture, data, texFormat, width, height, mipCount);
}

CREATE_PATCHED_CALL(int, sceGxmTextureInitLinearStrided, SceGxmTexture *texture, const void *data, SceGxmTextureFormat texFormat, unsigned int width, unsigned int height, unsigned int byteStride)
{
    LOGD("sceGxmTextureInitLinearStrided(texture: %p, data: %p, texBaseFormat: %s, texSwizzleMode: %s, width: %" PRIu32 ", height: %" PRIu32 ", byteStride: %" PRIu32 ")\n", texture, data, textureBaseFormat2str(texFormat), textureSwizzleMode2str(texFormat), width, height, byteStride);
    return TAI_NEXT(sceGxmTextureInitLinearStrided, sceGxmTextureInitLinearStridedRef, texture, data, texFormat, width, height, byteStride);
}

CREATE_PATCHED_CALL(int, sceGxmTextureInitSwizzled, SceGxmTexture *texture, const void *data, SceGxmTextureFormat texFormat, unsigned int width, unsigned int height, unsigned int mipCount)
{
    LOGD("sceGxmTextureInitSwizzled(texture: %p, data: %p, texBaseFormat: %s, texSwizzleMode: %s, width: %" PRIu32 ", height: %" PRIu32 ", mipCount: %" PRIu32 ")\n", texture, data, textureBaseFormat2str(texFormat), textureSwizzleMode2str(texFormat), width, height, mipCount);
    return TAI_NEXT(sceGxmTextureInitSwizzled, sceGxmTextureInitSwizzledRef, texture, data, texFormat, width, height, mipCount);
}

int sceGxmTextureInitSwizzledArbitrary(SceGxmTexture *texture, const void *data, SceGxmTextureFormat texFormat, unsigned int width, unsigned int height, unsigned int mipCount);
CREATE_PATCHED_CALL(int, sceGxmTextureInitSwizzledArbitrary, SceGxmTexture *texture, const void *data, SceGxmTextureFormat texFormat, unsigned int width, unsigned int height, unsigned int mipCount)
{
    LOGD("sceGxmTextureInitSwizzledArbitrary(texture: %p, data: %p, texBaseFormat: %s, texSwizzleMode: %s, width: %" PRIu32 ", height: %" PRIu32 ", mipCount: %" PRIu32 ")\n", texture, data, textureBaseFormat2str(texFormat), textureSwizzleMode2str(texFormat), width, height, mipCount);
    return TAI_NEXT(sceGxmTextureInitSwizzledArbitrary, sceGxmTextureInitSwizzledArbitraryRef, texture, data, texFormat, width, height, mipCount);
}

CREATE_PATCHED_CALL(int, sceGxmTextureInitTiled, SceGxmTexture *texture, const void *data, SceGxmTextureFormat texFormat, unsigned int width, unsigned int height, unsigned int mipCount)
{
    LOGD("sceGxmTextureInitTiled(texture: %p, data: %p, texBaseFormat: %s, texSwizzleMode: %s, width: %" PRIu32 ", height: %" PRIu32 ", mipCount: %" PRIu32 ")\n", texture, data, textureBaseFormat2str(texFormat), textureSwizzleMode2str(texFormat), width, height, mipCount);
    return TAI_NEXT(sceGxmTextureInitTiled, sceGxmTextureInitTiledRef, texture, data, texFormat, width, height, mipCount);
}

CREATE_PATCHED_CALL(int, sceGxmTextureSetData, SceGxmTexture *texture, const void *data)
{
    LOGD("sceGxmTextureSetData(texture: %p, data: %p)\n", texture, data);
    return TAI_NEXT(sceGxmTextureSetData, sceGxmTextureSetDataRef, texture, data);
}

CREATE_PATCHED_CALL(int, sceGxmTextureSetFormat, SceGxmTexture *texture, SceGxmTextureFormat texFormat)
{
    LOGD("sceGxmTextureSetFormat(texture: %p, texFormat: %" PRIu32 ")\n", texture, texFormat);
    return TAI_NEXT(sceGxmTextureSetFormat, sceGxmTextureSetFormatRef, texture, texFormat);
}

CREATE_PATCHED_CALL(int, sceGxmTextureSetGammaMode, SceGxmTexture *texture, SceGxmTextureGammaMode gammaMode)
{
    LOGD("sceGxmTextureSetGammaMode(texture: %p, gammaMode: %" PRIu32 ")\n", texture, gammaMode);
    return TAI_NEXT(sceGxmTextureSetGammaMode, sceGxmTextureSetGammaModeRef, texture, gammaMode);
}

CREATE_PATCHED_CALL(int, sceGxmTextureSetHeight, SceGxmTexture *texture, unsigned int height)
{
    LOGD("sceGxmTextureSetHeight(texture: %p, height: %" PRIu32 ")\n", texture, height);
    return TAI_NEXT(sceGxmTextureSetHeight, sceGxmTextureSetHeightRef, texture, height);
}

CREATE_PATCHED_CALL(int, sceGxmTextureSetLodBias, SceGxmTexture *texture, unsigned int bias)
{
    LOGD("sceGxmTextureSetLodBias(texture: %p, bias: %" PRIu32 ")\n", texture, bias);
    return TAI_NEXT(sceGxmTextureSetLodBias, sceGxmTextureSetLodBiasRef, texture, bias);
}

int sceGxmTextureSetLodMin(SceGxmTexture *texture, unsigned int lodMin);
CREATE_PATCHED_CALL(int, sceGxmTextureSetLodMin, SceGxmTexture *texture, unsigned int lodMin)
{
    LOGD("sceGxmTextureSetLodMin(texture: %p, lodMin: %" PRIu32 ")\n", texture, lodMin);
    return TAI_NEXT(sceGxmTextureSetLodMin, sceGxmTextureSetLodMinRef, texture, lodMin);
}

CREATE_PATCHED_CALL(int, sceGxmTextureSetMagFilter, SceGxmTexture *texture, SceGxmTextureFilter magFilter)
{
    LOGD("sceGxmTextureSetMagFilter(texture: %p, minFilter: %" PRIu32 ")\n", texture, magFilter);
    return TAI_NEXT(sceGxmTextureSetMagFilter, sceGxmTextureSetMagFilterRef, texture, magFilter);
}

CREATE_PATCHED_CALL(int, sceGxmTextureSetMinFilter, SceGxmTexture *texture, SceGxmTextureFilter minFilter)
{
    LOGD("sceGxmTextureSetMinFilter(texture: %p, minFilter: %" PRIu32 ")\n", texture, minFilter);
    return TAI_NEXT(sceGxmTextureSetMinFilter, sceGxmTextureSetMinFilterRef, texture, minFilter);
}

CREATE_PATCHED_CALL(int, sceGxmTextureSetMipFilter, SceGxmTexture *texture, SceGxmTextureMipFilter mipFilter)
{
    LOGD("sceGxmTextureSetMipFilter(texture: %p, mipFilter: %" PRIu32 ")\n", texture, mipFilter);
    return TAI_NEXT(sceGxmTextureSetMipFilter, sceGxmTextureSetMipFilterRef, texture, mipFilter);
}

CREATE_PATCHED_CALL(int, sceGxmTextureSetMipmapCount, SceGxmTexture *texture, unsigned int mipCount)
{
    LOGD("sceGxmTextureSetMipmapCount(texture: %p, mipCount: %" PRIu32 ")\n", texture, mipCount);
    return TAI_NEXT(sceGxmTextureSetMipmapCount, sceGxmTextureSetMipmapCountRef, texture, mipCount);
}

int sceGxmTextureSetNormalizeMode(SceGxmTexture *texture, SceGxmTextureNormalizeMode normalizeMode);
CREATE_PATCHED_CALL(int, sceGxmTextureSetNormalizeMode, SceGxmTexture *texture, SceGxmTextureNormalizeMode normalizeMode)
{
    LOGD("sceGxmTextureSetNormalizeMode(texture: %p, mipCount: %" PRIu32 ")\n", texture, normalizeMode);
    return TAI_NEXT(sceGxmTextureSetNormalizeMode, sceGxmTextureSetNormalizeModeRef, texture, normalizeMode);
}

CREATE_PATCHED_CALL(int, sceGxmTextureSetPalette, SceGxmTexture *texture, const void *paletteData)
{
    LOGD("sceGxmTextureSetPalette(texture: %p, paletteData: %p)\n", texture, paletteData);
    return TAI_NEXT(sceGxmTextureSetPalette, sceGxmTextureSetPaletteRef, texture, paletteData);
}

CREATE_PATCHED_CALL(int, sceGxmTextureSetStride, SceGxmTexture *texture, unsigned int byteStride)
{
    LOGD("sceGxmTextureSetStride(texture: %p, byteStride: %" PRIu32 ")\n", texture, byteStride);
    return TAI_NEXT(sceGxmTextureSetStride, sceGxmTextureSetStrideRef, texture, byteStride);
}

CREATE_PATCHED_CALL(int, sceGxmTextureSetUAddrMode, SceGxmTexture *texture, SceGxmTextureAddrMode addrMode)
{
    LOGD("sceGxmTextureSetUAddrMode(texture: %p, addrMode: %" PRIu32 ")\n", texture, addrMode);
    return TAI_NEXT(sceGxmTextureSetUAddrMode, sceGxmTextureSetUAddrModeRef, texture, addrMode);
}

int sceGxmTextureSetUAddrModeSafe(SceGxmTexture *texture, SceGxmTextureAddrMode addrMode);
CREATE_PATCHED_CALL(int, sceGxmTextureSetUAddrModeSafe, SceGxmTexture *texture, SceGxmTextureAddrMode addrMode)
{
    LOGD("sceGxmTextureSetUAddrModeSafe(texture: %p, addrMode: %" PRIu32 ")\n", texture, addrMode);
    return TAI_NEXT(sceGxmTextureSetUAddrModeSafe, sceGxmTextureSetUAddrModeSafeRef, texture, addrMode);
}

CREATE_PATCHED_CALL(int, sceGxmTextureSetVAddrMode, SceGxmTexture *texture, SceGxmTextureAddrMode addrMode)
{
    LOGD("sceGxmTextureSetVAddrMode(texture: %p, addrMode: %" PRIu32 ")\n", texture, addrMode);
    return TAI_NEXT(sceGxmTextureSetVAddrMode, sceGxmTextureSetVAddrModeRef, texture, addrMode);
}

int sceGxmTextureSetVAddrModeSafe(SceGxmTexture *texture, SceGxmTextureAddrMode addrMode);
CREATE_PATCHED_CALL(int, sceGxmTextureSetVAddrModeSafe, SceGxmTexture *texture, SceGxmTextureAddrMode addrMode)
{
    LOGD("sceGxmTextureSetVAddrModeSafe(texture: %p, addrMode: %" PRIu32 ")\n", texture, addrMode);
    return TAI_NEXT(sceGxmTextureSetVAddrModeSafe, sceGxmTextureSetVAddrModeSafeRef, texture, addrMode);
}

CREATE_PATCHED_CALL(int, sceGxmTextureSetWidth, SceGxmTexture *texture, unsigned int width)
{
    LOGD("sceGxmTextureSetWidth(texture: %p, width: %" PRIu32 ")\n", texture, width);
    return TAI_NEXT(sceGxmTextureSetWidth, sceGxmTextureSetWidthRef, texture, width);
}

int sceGxmTextureValidate(SceGxmTexture *texture);
CREATE_PATCHED_CALL(int, sceGxmTextureValidate, SceGxmTexture *texture)
{
    LOGD("sceGxmTextureValidate(texture: %p)\n", texture);
    return TAI_NEXT(sceGxmTextureValidate, sceGxmTextureValidateRef, texture);
}

CREATE_PATCHED_CALL(int, sceGxmTransferCopy, uint32_t width, uint32_t height, uint32_t colorKeyValue, uint32_t colorKeyMask, SceGxmTransferColorKeyMode colorKeyMode, SceGxmTransferFormat srcFormat, SceGxmTransferType srcType, const void *srcAddress, uint32_t srcX, uint32_t srcY, int32_t srcStride, SceGxmTransferFormat destFormat, SceGxmTransferType destType, void *destAddress, uint32_t destX, uint32_t destY, int32_t destStride, SceGxmSyncObject *syncObject, uint32_t syncFlags, const SceGxmNotification *notification)
{
    LOGD("sceGxmTransferCopy(width: %" PRIu32 ", height: %" PRIu32 ", colorKeyValue: %" PRIu32 ", colorKeyMask: %" PRIu32 ", colorKeyMode: %" PRIu32 ", srcFormat: %" PRIu32 ", srcType: %" PRIu32 ", srcAddress: %p, srcX: %" PRIu32 ", srcY: " PRIu32 ", srcStride: %" PRIu32 ", destFormat: %" PRIu32 ", destType: %" PRIu32 ", destAddress: %p, destX: %" PRIu32 ", destY: " PRIu32 ", destStride: %" PRIu32 ", syncObject: %p, syncFlags: %" PRIu32 ", notification: %p)\n", width, height, colorKeyValue, colorKeyMask, colorKeyMode, srcFormat, srcType, srcAddress, srcX, srcY, srcStride, destFormat, destType, destAddress, destX, destY, destStride, syncObject, syncFlags, notification);
    return TAI_NEXT(sceGxmTransferCopy, sceGxmTransferCopyRef, width, height, colorKeyValue, colorKeyMask, colorKeyMode, srcFormat, srcType, srcAddress, srcX, srcY, srcStride, destFormat, destType, destAddress, destX, destY, destStride, syncObject, syncFlags, notification);
}

CREATE_PATCHED_CALL(int, sceGxmTransferDownscale, SceGxmTransferFormat srcFormat, const void *srcAddress, unsigned int srcX, unsigned int srcY, unsigned int srcWidth, unsigned int srcHeight, int srcStride, SceGxmTransferFormat destFormat, void *destAddress, unsigned int destX, unsigned int destY, int destStride, SceGxmSyncObject *syncObject, unsigned int syncFlags, const SceGxmNotification* notification)
{
    LOGD("sceGxmTransferDownscale(srcFormat: %" PRIu32 ", srcAddress: %p, srcX: %" PRIu32 ", srcY: " PRIu32 ", srcStride: %" PRIu32 ", destFormat: %" PRIu32 ", destAddress: %p, destX: %" PRIu32 ", destY: " PRIu32 ", destStride: %" PRIu32 ", syncObject: %p, syncFlags: %" PRIu32 ", notification: %p)\n", srcFormat, srcAddress, srcX, srcY, srcStride, destFormat, destAddress, destX, destY, destStride, syncObject, syncFlags, notification);
    return TAI_NEXT(sceGxmTransferDownscale, sceGxmTransferDownscaleRef, srcFormat, srcAddress, srcX, srcY, srcWidth, srcHeight, srcStride, destFormat, destAddress, destX, destY, destStride, syncObject, syncFlags, notification);
}

int sceGxmTransferFill(unsigned int fillColor, SceGxmTransferFormat destFormat, void *destAddress, unsigned int destX, unsigned int destY, int destStride, SceGxmSyncObject *syncObject, unsigned int syncFlags, const SceGxmNotification* notification);
CREATE_PATCHED_CALL(int, sceGxmTransferFill, unsigned int fillColor, SceGxmTransferFormat destFormat, void *destAddress, unsigned int destX, unsigned int destY, int destStride, SceGxmSyncObject *syncObject, unsigned int syncFlags, const SceGxmNotification* notification)
{
    LOGD("sceGxmTransferFill(fillColor: %" PRIu32 ", destFormat: %" PRIu32 ", destAddress: %p, destX: %" PRIu32 ", destY: " PRIu32 ", destStride: %" PRIu32 ", syncObject: %p, syncFlags: %" PRIu32 ", notification: %p)\n", fillColor, destFormat, destAddress, destX, destY, destStride, syncObject, syncFlags, notification);
    return TAI_NEXT(sceGxmTransferFill, sceGxmTransferFillRef, fillColor, destFormat, destAddress, destX, destY, destStride, syncObject, syncFlags, notification);
}

CREATE_PATCHED_CALL(int, sceGxmTransferFinish)
{
    LOGD("sceGxmTransferFinish()\n");
    return TAI_NEXT(sceGxmTransferFinish, sceGxmTransferFinishRef);
}

CREATE_PATCHED_CALL(int, sceGxmUnmapFragmentUsseMemory, void *base)
{
    LOGD("sceGxmUnmapFragmentUsseMemory(base: %p)\n", base);
    return TAI_NEXT(sceGxmUnmapFragmentUsseMemory, sceGxmUnmapFragmentUsseMemoryRef, base);
}

CREATE_PATCHED_CALL(int, sceGxmUnmapMemory, void *base)
{
    if (g_log) {
        uint32_t chunkSize = 0;

        GXMChunk type = GXMChunk::sceGxmUnmapMemory;
        g_fileoffset += g_file.write(type);
        g_fileoffset += g_file.write(chunkSize);
        g_fileoffset += g_file.write(base);
        g_fileoffset += ALIGN_TO_64(g_fileoffset);
    }

    for (uint32_t index = 0; index < g_mappedMemoryCount; ++index) {
        if (g_memoryArray[index].addr == base) {
            if (index == g_mappedMemoryCount - 1) {
                g_memoryArray[index].resId = 0;
                g_memoryArray[index].addr = NULL;
                g_memoryArray[index].size = 0;
            }
            else {
                MappedMemory tmp = g_memoryArray[g_mappedMemoryCount - 1];
                g_memoryArray[index] = tmp;
                --g_mappedMemoryCount;
            }
        }
    }

    LOG("sceGxmuUnmapMemory(base: %p)\n", base);
    return TAI_NEXT(sceGxmUnmapMemory, sceGxmUnmapMemoryRef, base);
}

CREATE_PATCHED_CALL(int, sceGxmUnmapVertexUsseMemory, void *base)
{
    LOGD("sceGxmUnmapVertexUsseMemory(base: %p)\n", base);
    return TAI_NEXT(sceGxmUnmapVertexUsseMemory, sceGxmUnmapVertexUsseMemoryRef, base);
}

int sceGxmVertexFence();
CREATE_PATCHED_CALL(int, sceGxmVertexFence)
{
    LOGD("sceGxmVertexFence()\n");
    return TAI_NEXT(sceGxmVertexFence, sceGxmVertexFenceRef);
}

CREATE_PATCHED_CALL(const SceGxmProgram *, sceGxmVertexProgramGetProgram, const SceGxmVertexProgram *vertexProgram)
{
    LOGD("sceGxmVertexProgramGetProgram(fragmentProgram: %p)\n", vertexProgram);
    return TAI_NEXT(sceGxmVertexProgramGetProgram, sceGxmVertexProgramGetProgramRef, vertexProgram);
}

int sceGxmWaitEvent();
CREATE_PATCHED_CALL(int, sceGxmWaitEvent)
{
    LOGD("sceGxmWaitEvent()\n");
    return TAI_NEXT(sceGxmWaitEvent, sceGxmWaitEventRef);
}

#define RENDERDOC_FIRST_TARGET_CONTROL_PORT 38920
#define RENDERDOC_LAST_TARGET_CONTROL_PORT 38927

static int running = 0;

enum PacketType
{
  ePacket_Noop = 1,
  ePacket_Handshake,
  ePacket_Busy,
  ePacket_NewCapture,
  ePacket_APIUse,
  ePacket_TriggerCapture,
  ePacket_CopyCapture,
  ePacket_DeleteCapture,
  ePacket_QueueCapture,
  ePacket_NewChild,
  ePacket_CaptureProgress,
  ePacket_CycleActiveWindow,
  ePacket_CapturableWindowCount
};

enum RemoteServerPacket
{
  eRemoteServer_Noop = 1,
  eRemoteServer_Handshake,
  eRemoteServer_VersionMismatch,
  eRemoteServer_Busy,

  eRemoteServer_Ping,
  eRemoteServer_RemoteDriverList,
  eRemoteServer_TakeOwnershipCapture,
  eRemoteServer_CopyCaptureToRemote,
  eRemoteServer_CopyCaptureFromRemote,
  eRemoteServer_OpenLog,
  eRemoteServer_LogOpenProgress,
  eRemoteServer_LogOpened,
  eRemoteServer_HasCallstacks,
  eRemoteServer_InitResolver,
  eRemoteServer_ResolverProgress,
  eRemoteServer_GetResolve,
  eRemoteServer_CloseLog,
  eRemoteServer_HomeDir,
  eRemoteServer_ListDir,
  eRemoteServer_ExecuteAndInject,
  eRemoteServer_ShutdownServer,
  eRemoteServer_GetDriverName,
  eRemoteServer_GetSectionCount,
  eRemoteServer_FindSectionByName,
  eRemoteServer_FindSectionByType,
  eRemoteServer_GetSectionProperties,
  eRemoteServer_GetSectionContents,
  eRemoteServer_WriteSection,
  eRemoteServer_GetAvailableGPUs,
  eRemoteServer_RemoteServerCount,
};

struct Client
{
    int client_socket;
};

SceUID createThread(const char *thread_name, threadfunc_t thread_func, size_t user_data_size, void* user_data) 
{
    SceUID thid = sceKernelCreateThread(thread_name, thread_func, 0x40, 0x4000, 0, 0, NULL);
    if (thid < 0)
    {
        return -1;
    }

    int res = sceKernelStartThread(thid, user_data_size, user_data);
    if (res < 0)
    {
        sceKernelDeleteThread(thid);
        return -1;
    }

    return thid;
}

static void RespondToClientControl(int* socket, enum PacketType type)
{
    switch (type)
    {
    case ePacket_Handshake:
    {
        LOG("Respond to Handshake\n");

        uint32_t handshake = ePacket_Handshake;
        WriteUint32(socket, handshake);

        uint32_t chunkSize = 0;
        WriteUint32(socket, chunkSize);

        uint32_t version = 6;
        WriteUint32(socket, version);

        uint32_t len = 6;
        uint8_t ps_str[] = "PSVita";
        WriteString(socket, ps_str, len);
        
        SceUID pid = sceKernelGetProcessId();

        WriteUint32(socket, (uint32_t)pid);

        uint8_t alignment[38] = {};
        Write(socket, alignment, 38);
    } break;
    case ePacket_TriggerCapture:
    {
        float captureProgress = 0.0f;
        LOG("Respond to trigger capture\n");
        while (captureProgress < 1.0f) {
            uint32_t trigger = ePacket_CaptureProgress;
            WriteUint32(socket, trigger);

            uint32_t chunkSize = 0;
            WriteUint32(socket, chunkSize);

            WriteFloat(socket, captureProgress);

            uint8_t alignment[52] = {};
            Write(socket, alignment, 52);

            captureProgress += 0.001f;
            sceKernelDelayThread(5 * 1000);
        }
    } break;
    default:
    {
        LOG("Respond Noop\n");
        uint32_t type = (uint32_t)ePacket_Noop;
        WriteUint32(socket, type);
    } break;
    }
}

static void ReceiveFromClientControl(int* socket, enum PacketType type)
{
    switch (type)
    {
    case ePacket_Handshake:
    {
        LOG("Received Handshake\n");
        
        uint32_t chunkSize = 0;
        ReadUint32(socket, &chunkSize);

        LOG("Received chunkSize: %" PRIu32 "\n", chunkSize);

        uint32_t version;
        ReadUint32(socket, &version);

        LOG("Received version: %" PRIu32 "\n", version);

        uint8_t padding[52];

        Read(socket, padding, 52);
    } break;
    case ePacket_TriggerCapture:
    {
        LOG("Received trigger Capture\n");
        uint32_t chunkSize = 0;
        ReadUint32(socket, &chunkSize);

        LOG("Received chunkSize: %" PRIu32 "\n", chunkSize);

        uint32_t numFrames;
        ReadUint32(socket, &numFrames);

        LOG("Received numFrames: %" PRIu32 "\n", numFrames);

        uint8_t padding[52];

        Read(socket, padding, 52);
        g_capture = 1;

    } break;
    default:
    {
        uint8_t bytes[60];
        LOG("Received unknown Client Control Packet: %" PRIu32 "\n", (uint32_t)type);
        Read(socket, bytes, 60);
    } break;
    }

    RespondToClientControl(socket, type);
}

static int sClientControlThreadInit(SceSize args, void *init)
{
    if (args < 1)
    {
        return 0;
    }

    struct Client* client_sock = (struct Client*)init;
    int client = client_sock->client_socket;

    if (client != -1) {
        uint32_t type = 0;
        ReadUint32(&client, &type);

        if (type == 0)
        {
            if (client != -1){
                uint8_t bytes[64];
                RecvDataNonBlocking(&client, bytes, 64);
                sceKernelDelayThread(5 * 1000);
            }
            return 0;
        }

        ReceiveFromClientControl(&client, (PacketType)type);
        sceKernelDelayThread(5 * 1000);
    }

    if (client != -1) 
    {
        uint32_t type = (uint32_t)ePacket_APIUse;
        WriteUint32(&client, type);

        uint32_t chunkSize = 0;
        WriteUint32(&client, chunkSize);

        uint32_t driver = 11;
        WriteUint32(&client, driver);

        uint8_t presenting = 1;
        WriteBool(&client, presenting);

        uint8_t supported = 1;
        WriteBool(&client, supported);

        uint8_t alignment[50] = {};
        Write(&client, alignment, 50);
    }

    while(client != -1)
    {
        if (IsRecvDataWaiting(&client)) {
            uint32_t type = 0;
            ReadUint32(&client, &type);

            /*if (type == 0)
            {
                if (client != -1){
                    uint8_t bytes[64];
                    RecvDataBlocking(&client, bytes, 64);
                    sceKernelDelayThread(5 * 1000);
                }
                return 0;
            }*/

            ReceiveFromClientControl(&client, (PacketType)type);
        }
        sceKernelDelayThread(5 * 1000);

//        uint32_t type = (uint32_t)ePacket_Noop;
//        WriteUint32(&client, type);

        /*if (type == 0)
        {
            if (client != -1){
                uint8_t bytes[64];
                RecvDataNonBlocking(&client, bytes, 64);
                sceKernelDelayThread(5 * 1000);
            }
            continue;
        }

        ReceiveFromClientControl(&client, (enum PacketType)type);*/
        //sceKernelDelayThread(500 * 1000);
    }

    return 0;
}

static int sRemoteControlThreadInit(SceSize args, void *init)
{
    LOG("Remote Server started\n");
    uint32_t port = RENDERDOC_FIRST_TARGET_CONTROL_PORT;
    int socket = CreateSocket("0.0.0.0", port & 0xffff, 4);
    LOG("Socket created: %" PRIi32 "\n", socket);

    while (socket < 0 && port <= RENDERDOC_LAST_TARGET_CONTROL_PORT) 
    {
        ++port;
        socket = CreateSocket("0.0.0.0", port & 0xffff, 4);
        LOG("Socket created: %" PRIi32 "\n", socket);
    }

    if (socket >= 0) 
    {
        LOG("Server with port: %" PRIu32 "\n", port);
        running = 1;
    }

    while(running)
    {
        int client = AcceptClient(&socket, 0);

        if (client < 0) 
        {
            if (!(socket >= 0))
            {
                LOG("Error in accept - shutting down server\n");
                sceNetSocketClose(socket);
                return 0;
            }
            sceKernelDelayThread(5 * 1000);
            continue;
        }
        else {
            LOG("accepted connection to client for remote server\n");
        }

        struct Client client_sock = {};
        client_sock.client_socket = client;

        SceUID clientThreadId = createThread("RemoteServerClientControlThread", &sClientControlThreadInit, sizeof(client_sock), &client_sock);

        int threadStatus;
        SceUInt timeout = (SceUInt)-1;
        sceKernelWaitThreadEnd(clientThreadId, &threadStatus, &timeout);

        sceKernelDeleteThread(clientThreadId);
    }

    return 0;
}

int _start(SceSize args, void *argp) __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp) {
    LOG("gxm_inject\n");

    IMPORT_HOOK(sceGxmAddRazorGpuCaptureBuffer, 0xE9E81073);
    IMPORT_HOOK(sceGxmBeginCommandList, 0x944D3F83);
    IMPORT_HOOK(sceGxmBeginScene, 0x8734FF4E);
    IMPORT_HOOK(sceGxmBeginSceneEx, 0x4709CF5A);
    IMPORT_HOOK(sceGxmColorSurfaceGetClip, 0x07DFEE4B);
    IMPORT_HOOK(sceGxmColorSurfaceGetData, 0x2DB6026C);
    IMPORT_HOOK(sceGxmColorSurfaceGetDitherMode, 0x200A96E1);
    IMPORT_HOOK(sceGxmColorSurfaceGetFormat, 0xF3C1C6C6);
    IMPORT_HOOK(sceGxmColorSurfaceGetGammaMode, 0xEE0B4DF0);
    IMPORT_HOOK(sceGxmColorSurfaceGetScaleMode, 0x6E3FA74D);
    IMPORT_HOOK(sceGxmColorSurfaceGetStrideInPixels, 0xF33D9980);
    IMPORT_HOOK(sceGxmColorSurfaceGetType, 0x52FDE962);
    IMPORT_HOOK(sceGxmColorSurfaceInit, 0xED0F6E25);
    IMPORT_HOOK(sceGxmColorSurfaceInitDisabled, 0x613639FA);
    IMPORT_HOOK(sceGxmColorSurfaceIsEnabled, 0x0E0EBB57);
    IMPORT_HOOK(sceGxmColorSurfaceSetClip, 0x86456F7B);
    IMPORT_HOOK(sceGxmColorSurfaceSetData, 0x537CA400);
    IMPORT_HOOK(sceGxmColorSurfaceSetDitherMode, 0x45027BAB);
    IMPORT_HOOK(sceGxmColorSurfaceSetFormat, 0x5F9A3A16);
    IMPORT_HOOK(sceGxmColorSurfaceSetGammaMode, 0xF5C89643);
    IMPORT_HOOK(sceGxmColorSurfaceSetScaleMode, 0x6B96EDF7);
    IMPORT_HOOK(sceGxmCreateContext, 0xE84CE5B4);
    IMPORT_HOOK(sceGxmCreateDeferredContext, 0x249D5B00);
    IMPORT_HOOK(sceGxmCreateRenderTarget, 0x207AF96B);
    IMPORT_HOOK(sceGxmDepthStencilSurfaceGetBackgroundDepth, 0x269B56BE);
    IMPORT_HOOK(sceGxmDepthStencilSurfaceGetBackgroundMask, 0x85D4DEFE);
    IMPORT_HOOK(sceGxmDepthStencilSurfaceGetBackgroundStencil, 0xAAFC062B);
    IMPORT_HOOK(sceGxmDepthStencilSurfaceGetForceLoadMode, 0x2F5CC20C);
    IMPORT_HOOK(sceGxmDepthStencilSurfaceGetForceStoreMode, 0x544AA05A);
    IMPORT_HOOK(sceGxmDepthStencilSurfaceGetFormat, 0x8504038D);
    IMPORT_HOOK(sceGxmDepthStencilSurfaceGetStrideInSamples, 0x11628789);
    IMPORT_HOOK(sceGxmDepthStencilSurfaceInit, 0xCA9D41D1);
    IMPORT_HOOK(sceGxmDepthStencilSurfaceInitDisabled, 0xA41DB0D6);
    IMPORT_HOOK(sceGxmDepthStencilSurfaceIsEnabled, 0x082200E1);
    IMPORT_HOOK(sceGxmDepthStencilSurfaceSetBackgroundDepth, 0x32F280F0);
    IMPORT_HOOK(sceGxmDepthStencilSurfaceSetBackgroundMask, 0xF28A688A);
    IMPORT_HOOK(sceGxmDepthStencilSurfaceSetBackgroundStencil, 0xF5D3F3E8);
    IMPORT_HOOK(sceGxmDepthStencilSurfaceSetForceLoadMode, 0x0C44ACD7);
    IMPORT_HOOK(sceGxmDepthStencilSurfaceSetForceStoreMode, 0x12AAA7AF);
    IMPORT_HOOK(sceGxmDestroyContext, 0xEDDC5FB2);
    IMPORT_HOOK(sceGxmDestroyDeferredContext, 0xD6A2FF2F);
    IMPORT_HOOK(sceGxmDestroyRenderTarget, 0x0B94C50A);
    IMPORT_HOOK(sceGxmDisplayQueueAddEntry, 0xEC5C26B5);
    IMPORT_HOOK(sceGxmDisplayQueueFinish, 0xB98C5B0D);
    IMPORT_HOOK(sceGxmDraw, 0xBC059AFC);
    IMPORT_HOOK(sceGxmDrawInstanced, 0x14C4E7D3);
    IMPORT_HOOK(sceGxmDrawPrecomputed, 0xED3F78B8);
    IMPORT_HOOK(sceGxmEndCommandList, 0x36D85916);
    IMPORT_HOOK(sceGxmEndScene, 0xFE300E2F);
    IMPORT_HOOK(sceGxmExecuteCommandList, 0xE8E139DD);
    IMPORT_HOOK(sceGxmFinish, 0x0733D8AE);
    IMPORT_HOOK(sceGxmFragmentProgramGetPassType, 0xCE0B0A76);
    IMPORT_HOOK(sceGxmFragmentProgramGetProgram, 0xE0E3B3F8);
    IMPORT_HOOK(sceGxmFragmentProgramIsEnabled, 0x5748367E);
    IMPORT_HOOK(sceGxmGetContextType, 0x2A1BCDDB);
    IMPORT_HOOK(sceGxmGetDeferredContextFragmentBuffer, 0xFA081D05);
    IMPORT_HOOK(sceGxmGetDeferredContextVdmBuffer, 0xAD8C2EBA);
    IMPORT_HOOK(sceGxmGetDeferredContextVertexBuffer, 0x87632B9C);
    IMPORT_HOOK(sceGxmGetNotificationRegion, 0x8BDE825A);
    IMPORT_HOOK(sceGxmGetParameterBufferThreshold, 0x6ABF3F76);
    IMPORT_HOOK(sceGxmGetPrecomputedDrawSize, 0x41BBD792);
    IMPORT_HOOK(sceGxmGetPrecomputedFragmentStateSize, 0x85DE8506);
    IMPORT_HOOK(sceGxmGetPrecomputedVertexStateSize, 0x9D83CA3B);
    IMPORT_HOOK(sceGxmGetRenderTargetMemSizes, 0xB291C959);
    IMPORT_HOOK(sceGxmInitialize, 0xB0F1E4EC);
    IMPORT_HOOK(sceGxmIsDebugVersion, 0x873B07C0);
    IMPORT_HOOK(sceGxmMapFragmentUsseMemory, 0x008402C6);
    IMPORT_HOOK(sceGxmMapMemory, 0xC61E34FC);
    IMPORT_HOOK(sceGxmMapVertexUsseMemory, 0xFA437510);
    IMPORT_HOOK(sceGxmMidSceneFlush, 0x2B5C0444);
    IMPORT_HOOK(sceGxmNotificationWait, 0x9F448E79);
    IMPORT_HOOK(sceGxmPadHeartbeat, 0x3D25FCE9);
    IMPORT_HOOK(sceGxmPadTriggerGpuPaTrace, 0x47E06984);
    IMPORT_HOOK(sceGxmPopUserMarker, 0x4FA073A6);
    IMPORT_HOOK(sceGxmPrecomputedDrawInit, 0xA197F096);
    IMPORT_HOOK(sceGxmPrecomputedDrawSetAllVertexStreams, 0xB6C6F571);
    IMPORT_HOOK(sceGxmPrecomputedDrawSetParams, 0x884D0D08);
    IMPORT_HOOK(sceGxmPrecomputedDrawSetParamsInstanced, 0x3A7B1633);
    IMPORT_HOOK(sceGxmPrecomputedDrawSetVertexStream, 0x6C936214);
    IMPORT_HOOK(sceGxmPrecomputedFragmentStateGetDefaultUniformBuffer, 0xCECB584A);
    IMPORT_HOOK(sceGxmPrecomputedFragmentStateInit, 0xE297D7AF);
    IMPORT_HOOK(sceGxmPrecomputedFragmentStateSetAllAuxiliarySurfaces, 0x9D93B63A);
    IMPORT_HOOK(sceGxmPrecomputedFragmentStateSetAllTextures, 0xC383DE39);
    IMPORT_HOOK(sceGxmPrecomputedFragmentStateSetAllUniformBuffers, 0x5A783DC3);
    IMPORT_HOOK(sceGxmPrecomputedFragmentStateSetDefaultUniformBuffer, 0x91236858);
    IMPORT_HOOK(sceGxmPrecomputedFragmentStateSetTexture, 0x29118BF1);
    IMPORT_HOOK(sceGxmPrecomputedFragmentStateSetUniformBuffer, 0xB452F1FB);
    IMPORT_HOOK(sceGxmPrecomputedVertexStateGetDefaultUniformBuffer, 0xBE5A68EF);
    IMPORT_HOOK(sceGxmPrecomputedVertexStateInit, 0xBE937F8D);
    IMPORT_HOOK(sceGxmPrecomputedVertexStateSetAllTextures, 0xC40C9127);
    IMPORT_HOOK(sceGxmPrecomputedVertexStateSetAllUniformBuffers, 0x0389861D);
    IMPORT_HOOK(sceGxmPrecomputedVertexStateSetDefaultUniformBuffer, 0x34BF64E3);
    IMPORT_HOOK(sceGxmPrecomputedVertexStateSetTexture, 0x6A29EB06);
    IMPORT_HOOK(sceGxmPrecomputedVertexStateSetUniformBuffer, 0xDBF97ED6);
    IMPORT_HOOK(sceGxmProgramCheck, 0xED8B6C69);
    IMPORT_HOOK(sceGxmProgramFindParameterByName, 0x277794C4);
    IMPORT_HOOK(sceGxmProgramFindParameterBySemantic, 0x633CAA54);
    IMPORT_HOOK(sceGxmProgramGetDefaultUniformBufferSize, 0x8FA3F9C3);
    IMPORT_HOOK(sceGxmProgramGetFragmentProgramInputs, 0xC6924709);
    IMPORT_HOOK(sceGxmProgramGetOutputRegisterFormat, 0xE11603B1);
    IMPORT_HOOK(sceGxmProgramGetParameter, 0x06FF9151);
    IMPORT_HOOK(sceGxmProgramGetParameterCount, 0xD5D5FCCD);
    IMPORT_HOOK(sceGxmProgramGetSize, 0xBF5E2090);
    IMPORT_HOOK(sceGxmProgramGetType, 0x04BB3C59);
    IMPORT_HOOK(sceGxmProgramGetVertexProgramOutputs, 0xFB01C7E5);
    IMPORT_HOOK(sceGxmProgramIsDepthReplaceUsed, 0x89613EF2);
    IMPORT_HOOK(sceGxmProgramIsDiscardUsed, 0x029B4F1C);
    IMPORT_HOOK(sceGxmProgramIsEquivalent, 0xE23C838C);
    IMPORT_HOOK(sceGxmProgramIsFragColorUsed, 0x104F23F4);
    IMPORT_HOOK(sceGxmProgramIsNativeColorUsed, 0xA4297E57);
    IMPORT_HOOK(sceGxmProgramIsSpriteCoordUsed, 0xE63C53D8);
    IMPORT_HOOK(sceGxmProgramParameterGetArraySize, 0xDBA8D061);
    IMPORT_HOOK(sceGxmProgramParameterGetCategory, 0x1997DC17);
    IMPORT_HOOK(sceGxmProgramParameterGetComponentCount, 0xBD2998D1);
    IMPORT_HOOK(sceGxmProgramParameterGetContainerIndex, 0xBB58267D);
    IMPORT_HOOK(sceGxmProgramParameterGetIndex, 0x6E61DDF5);
    IMPORT_HOOK(sceGxmProgramParameterGetName, 0x6AF88A5D);
    IMPORT_HOOK(sceGxmProgramParameterGetResourceIndex, 0x5C79D59A);
    IMPORT_HOOK(sceGxmProgramParameterGetSemantic, 0xE6D9C4CE);
    IMPORT_HOOK(sceGxmProgramParameterGetSemanticIndex, 0xB85CC13E);
    IMPORT_HOOK(sceGxmProgramParameterGetType, 0x7B9023C3);
    IMPORT_HOOK(sceGxmProgramParameterIsRegFormat, 0x871E5009);
    IMPORT_HOOK(sceGxmProgramParameterIsSamplerCube, 0xF7AA978B);
    IMPORT_HOOK(sceGxmPushUserMarker, 0x3276C475);
    IMPORT_HOOK(sceGxmRemoveRazorGpuCaptureBuffer, 0x0468E3F2);
    IMPORT_HOOK(sceGxmRenderTargetGetDriverMemBlock, 0x49553737);
    IMPORT_HOOK(sceGxmRenderTargetGetHostMem, 0xD0EDAB4C);
    IMPORT_HOOK(sceGxmReserveFragmentDefaultUniformBuffer, 0x7B1FABB6);
    IMPORT_HOOK(sceGxmReserveVertexDefaultUniformBuffer, 0x97118913);
    IMPORT_HOOK(sceGxmSetAuxiliarySurface, 0x91B4F7F4);
    IMPORT_HOOK(sceGxmSetBackDepthBias, 0x17B3BF86);
    IMPORT_HOOK(sceGxmSetBackDepthFunc, 0xB042A4D2);
    IMPORT_HOOK(sceGxmSetBackDepthWriteEnable, 0xC18B706B);
    IMPORT_HOOK(sceGxmSetBackFragmentProgramEnable, 0xE26B4834);
    IMPORT_HOOK(sceGxmSetBackLineFillLastPixelEnable, 0xC88EB702);
    IMPORT_HOOK(sceGxmSetBackPointLineWidth, 0x8DCB0EDB);
    IMPORT_HOOK(sceGxmSetBackPolygonMode, 0xF66EC6FE);
    IMPORT_HOOK(sceGxmSetBackStencilFunc, 0x1A68C8D2);
    IMPORT_HOOK(sceGxmSetBackStencilRef, 0x866A0517);
    IMPORT_HOOK(sceGxmSetBackVisibilityTestEnable, 0x17CF46B9);
    IMPORT_HOOK(sceGxmSetBackVisibilityTestIndex, 0xAE7886FE);
    IMPORT_HOOK(sceGxmSetBackVisibilityTestOp, 0xC83F0AB3);
    IMPORT_HOOK(sceGxmSetCullMode, 0xE1CA72AE);
    IMPORT_HOOK(sceGxmSetDefaultRegionClipAndViewport, 0x60CF708E);
    IMPORT_HOOK(sceGxmSetDeferredContextFragmentBuffer, 0xC6B3FCD0);
    IMPORT_HOOK(sceGxmSetDeferredContextVdmBuffer, 0x0B9D13CE);
    IMPORT_HOOK(sceGxmSetDeferredContextVertexBuffer, 0x27CAD127);
    IMPORT_HOOK(sceGxmSetFragmentDefaultUniformBuffer, 0xA824EB24);
    IMPORT_HOOK(sceGxmSetFragmentProgram, 0xAD2F48D9);
    IMPORT_HOOK(sceGxmSetFragmentTexture, 0x29C34DF5);
    IMPORT_HOOK(sceGxmSetFragmentUniformBuffer, 0xEA0FC310);
    IMPORT_HOOK(sceGxmSetFrontDepthBias, 0xAAA97F81);
    IMPORT_HOOK(sceGxmSetFrontDepthFunc, 0x14BD831F);
    IMPORT_HOOK(sceGxmSetFrontDepthWriteEnable, 0xF32CBF34);
    IMPORT_HOOK(sceGxmSetFrontFragmentProgramEnable, 0x575958A8);
    IMPORT_HOOK(sceGxmSetFrontLineFillLastPixelEnable, 0x5765DE9F);
    IMPORT_HOOK(sceGxmSetFrontPointLineWidth, 0x06752183);
    IMPORT_HOOK(sceGxmSetFrontPolygonMode, 0xFD93209D);
    IMPORT_HOOK(sceGxmSetFrontStencilFunc, 0xB8645A9A);
    IMPORT_HOOK(sceGxmSetFrontStencilRef, 0x8FA6FE44);
    IMPORT_HOOK(sceGxmSetFrontVisibilityTestEnable, 0x30459117);
    IMPORT_HOOK(sceGxmSetFrontVisibilityTestIndex, 0x12625C34);
    IMPORT_HOOK(sceGxmSetFrontVisibilityTestOp, 0xD0E3CD9A);
    IMPORT_HOOK(sceGxmSetPrecomputedFragmentState, 0xF8952750);
    IMPORT_HOOK(sceGxmSetPrecomputedVertexState, 0xB7626A93);
    IMPORT_HOOK(sceGxmSetRegionClip, 0x70C86868);
    IMPORT_HOOK(sceGxmSetTwoSidedEnable, 0x0DE9AEB7);
    IMPORT_HOOK(sceGxmSetUniformDataF, 0x65DD0C84);
    IMPORT_HOOK(sceGxmSetUserMarker, 0xC7A8CB77);
    IMPORT_HOOK(sceGxmSetValidationEnable, 0x8C6A24C9);
    IMPORT_HOOK(sceGxmSetVertexDefaultUniformBuffer, 0xC697CAE5);
    IMPORT_HOOK(sceGxmSetVertexProgram, 0x31FF8ABD);
    IMPORT_HOOK(sceGxmSetVertexStream, 0x895DF2E9);
    IMPORT_HOOK(sceGxmSetVertexTexture, 0x9EB4380F);
    IMPORT_HOOK(sceGxmSetVertexUniformBuffer, 0xC68015E4);
    IMPORT_HOOK(sceGxmSetViewport, 0x3EB3380B);
    IMPORT_HOOK(sceGxmSetViewportEnable, 0x814F61EB);
    IMPORT_HOOK(sceGxmSetVisibilityBuffer, 0x7767EC49);
    IMPORT_HOOK(sceGxmSetWBufferEnable, 0xEED86975);
    IMPORT_HOOK(sceGxmSetWClampEnable, 0x1BF8B853);
    IMPORT_HOOK(sceGxmSetWClampValue, 0xD096336E);
    IMPORT_HOOK(sceGxmSetWarningEnabled, 0x12D18B3D);
    IMPORT_HOOK(sceGxmSetYuvProfile, 0xB32917F0);
    IMPORT_HOOK(sceGxmShaderPatcherAddRefFragmentProgram, 0x4CD2D19F);
    IMPORT_HOOK(sceGxmShaderPatcherAddRefVertexProgram, 0x0FD1E589);
    IMPORT_HOOK(sceGxmShaderPatcherCreate, 0x05032658);
    IMPORT_HOOK(sceGxmShaderPatcherCreateFragmentProgram, 0x4ED2E49D);
    IMPORT_HOOK(sceGxmShaderPatcherCreateMaskUpdateFragmentProgram, 0xA4433427);
    IMPORT_HOOK(sceGxmShaderPatcherCreateVertexProgram, 0xB7BBA6D5);
    IMPORT_HOOK(sceGxmShaderPatcherDestroy, 0xEAA5B100);
    IMPORT_HOOK(sceGxmShaderPatcherForceUnregisterProgram, 0x630D4B2E);
    IMPORT_HOOK(sceGxmShaderPatcherGetBufferMemAllocated, 0xC694D039);
    IMPORT_HOOK(sceGxmShaderPatcherGetFragmentProgramRefCount, 0x2C5550F0);
    IMPORT_HOOK(sceGxmShaderPatcherGetFragmentUsseMemAllocated, 0x3C9DDB4A);
    IMPORT_HOOK(sceGxmShaderPatcherGetHostMemAllocated, 0x9DBBC71C);
    IMPORT_HOOK(sceGxmShaderPatcherGetProgramFromId, 0xA949A803);
    IMPORT_HOOK(sceGxmShaderPatcherGetUserData, 0x96A7E6DD);
    IMPORT_HOOK(sceGxmShaderPatcherGetVertexProgramRefCount, 0xA1A16FF6);
    IMPORT_HOOK(sceGxmShaderPatcherGetVertexUsseMemAllocated, 0x7D2F83C1);
    IMPORT_HOOK(sceGxmShaderPatcherRegisterProgram, 0x2B528462);
    IMPORT_HOOK(sceGxmShaderPatcherReleaseFragmentProgram, 0xBE2743D1);
    IMPORT_HOOK(sceGxmShaderPatcherReleaseVertexProgram, 0xAC1FF2DA);
    IMPORT_HOOK(sceGxmShaderPatcherSetAuxiliarySurface, 0x8E5FCC2A);
    IMPORT_HOOK(sceGxmShaderPatcherSetUserData, 0xF9B8FCFD);
    IMPORT_HOOK(sceGxmShaderPatcherUnregisterProgram, 0xF103AF8A);
    IMPORT_HOOK(sceGxmSyncObjectCreate, 0x6A6013E1);
    IMPORT_HOOK(sceGxmSyncObjectDestroy, 0x889AE88C);
    IMPORT_HOOK(sceGxmTerminate, 0xB627DE66);
    IMPORT_HOOK(sceGxmTextureGetData, 0x5341BD46);
    IMPORT_HOOK(sceGxmTextureGetFormat, 0xE868D2B3);
    IMPORT_HOOK(sceGxmTextureGetGammaMode, 0xF23FCE81);
    IMPORT_HOOK(sceGxmTextureGetHeight, 0x5420A086);
    IMPORT_HOOK(sceGxmTextureGetLodBias, 0x2DE55DA5);
    IMPORT_HOOK(sceGxmTextureGetLodMin, 0xBE524A2C);
    IMPORT_HOOK(sceGxmTextureGetMagFilter, 0xAE7FBB51);
    IMPORT_HOOK(sceGxmTextureGetMinFilter, 0x920666C6);
    IMPORT_HOOK(sceGxmTextureGetMipFilter, 0xCE94CA15);
    IMPORT_HOOK(sceGxmTextureGetMipmapCount, 0xF7B7B1E4);
    IMPORT_HOOK(sceGxmTextureGetMipmapCountUnsafe, 0x4CC42929);
    IMPORT_HOOK(sceGxmTextureGetNormalizeMode, 0x512BB86C);
    IMPORT_HOOK(sceGxmTextureGetPalette, 0x0D189C30);
    IMPORT_HOOK(sceGxmTextureGetStride, 0xB0BD52F3);
    IMPORT_HOOK(sceGxmTextureGetType, 0xF65D4917);
    IMPORT_HOOK(sceGxmTextureGetUAddrMode, 0x2AE22788);
    IMPORT_HOOK(sceGxmTextureGetUAddrModeSafe, 0xC037DA83);
    IMPORT_HOOK(sceGxmTextureGetVAddrMode, 0x46136CA9);
    IMPORT_HOOK(sceGxmTextureGetVAddrModeSafe, 0xD2F0D9C1);
    IMPORT_HOOK(sceGxmTextureGetWidth, 0x126A3EB3);
    IMPORT_HOOK(sceGxmTextureInitCube, 0x11DC8DC9);
    IMPORT_HOOK(sceGxmTextureInitCubeArbitrary, 0xE3DF5E3B);
    IMPORT_HOOK(sceGxmTextureInitLinear, 0x4811AECB);
    IMPORT_HOOK(sceGxmTextureInitLinearStrided, 0x6679BEF0);
    IMPORT_HOOK(sceGxmTextureInitSwizzled, 0xD572D547);
    IMPORT_HOOK(sceGxmTextureInitSwizzledArbitrary, 0x5DBFBA2C);
    IMPORT_HOOK(sceGxmTextureInitTiled, 0xE6F0DB27);
    IMPORT_HOOK(sceGxmTextureSetData, 0x855814C4);
    IMPORT_HOOK(sceGxmTextureSetFormat, 0xFC943596);
    IMPORT_HOOK(sceGxmTextureSetGammaMode, 0xA6D9F4DA);
    IMPORT_HOOK(sceGxmTextureSetHeight, 0xAEE7FDD1);
    IMPORT_HOOK(sceGxmTextureSetLodBias, 0xB65EE6F7);
    IMPORT_HOOK(sceGxmTextureSetLodMin, 0xB79E43DD);
    IMPORT_HOOK(sceGxmTextureSetMagFilter, 0xFA695FD7);
    IMPORT_HOOK(sceGxmTextureSetMinFilter, 0x416764E3);
    IMPORT_HOOK(sceGxmTextureSetMipFilter, 0x1CA9FE0B);
    IMPORT_HOOK(sceGxmTextureSetMipmapCount, 0xD2DC4643);
    IMPORT_HOOK(sceGxmTextureSetNormalizeMode, 0xCE8DDAD0);
    IMPORT_HOOK(sceGxmTextureSetPalette, 0xDD6AABFA);
    IMPORT_HOOK(sceGxmTextureSetStride, 0x58D0EB0A);
    IMPORT_HOOK(sceGxmTextureSetUAddrMode, 0x4281763E);
    IMPORT_HOOK(sceGxmTextureSetUAddrModeSafe, 0x8699ECF4);
    IMPORT_HOOK(sceGxmTextureSetVAddrMode, 0x126CDAA3);
    IMPORT_HOOK(sceGxmTextureSetVAddrModeSafe, 0xFA22F6CC);
    IMPORT_HOOK(sceGxmTextureSetWidth, 0x2EA178BE);
    IMPORT_HOOK(sceGxmTextureValidate, 0x5331BED3);
    IMPORT_HOOK(sceGxmTransferCopy, 0x62312BF8);
    IMPORT_HOOK(sceGxmTransferDownscale, 0xD10F7EAD);
    IMPORT_HOOK(sceGxmTransferFill, 0x69DDFF5E);
    IMPORT_HOOK(sceGxmTransferFinish, 0x45229C39);
    IMPORT_HOOK(sceGxmUnmapFragmentUsseMemory, 0x80CCEDBB);
    IMPORT_HOOK(sceGxmUnmapMemory, 0x828C68E8);
    IMPORT_HOOK(sceGxmUnmapVertexUsseMemory, 0x099134F5);
    IMPORT_HOOK(sceGxmVertexFence, 0xE05277D6);
    IMPORT_HOOK(sceGxmVertexProgramGetProgram, 0xBC52320E);
    IMPORT_HOOK(sceGxmWaitEvent, 0x8BD94593);

    sceSysmoduleLoadModuleHook = taiHookFunctionImport(&sceSysmoduleLoadModuleRef, MY_TAI_MAIN_MODULE, 0x03FCF19D, 0x79A0160A, (const void*)&sceSysmoduleLoadModulePatched);
    if (sceSysmoduleLoadModuleHook < 0) LOG("Could not hook sceSysmoduleLoadModule\n"); else LOG("hooked sceSysmoduleLoadModule\n");
    
    sceSysmoduleUnloadModuleHook = taiHookFunctionImport(&sceSysmoduleUnloadModuleRef, MY_TAI_MAIN_MODULE, 0x03FCF19D, 0x31D87805, (const void*)&sceSysmoduleUnloadModulePatched);
    if (sceSysmoduleUnloadModuleHook < 0) LOG("Could not hook sceSysmoduleUnloadModule\n"); else LOG("hooked sceSysmoduleUnloadModule\n");

    scePowerSetUsingWirelessHook = taiHookFunctionImport(&scePowerSetUsingWirelessRef, MY_TAI_MAIN_MODULE, TAI_ANY_LIBRARY, 0x4D695C1F, (const void*)&scePowerSetUsingWirelessPatched);
    if (scePowerSetUsingWirelessHook < 0) LOG("Could not hook scePowerSetUsingWireless\n"); else LOG("hooked scePowerSetUsingWireless\n");
    
    scePowerSetConfigurationModeHook = taiHookFunctionImport(&scePowerSetConfigurationModeRef, MY_TAI_MAIN_MODULE, TAI_ANY_LIBRARY, 0x3CE187B6, (const void*)&scePowerSetConfigurationModePatched);
    if (scePowerSetConfigurationModeHook < 0) LOG("Could not hook scePowerSetConfigurationMode\n"); else LOG("hooked scePowerSetConfigurationMode\n");

    sceKernelAllocMemBlockHook = taiHookFunctionImport(&sceKernelAllocMemBlockRef, MY_TAI_MAIN_MODULE, 0x37FE725A, 0xB9D5EBDE, (const void*)sceKernelAllocMemBlockPatched);
    if (sceKernelAllocMemBlockHook < 0) LOG("Could not hook sceKernelAllocMemBlock\n"); else LOG("hooked sceKernelAllocMemBlockHook\n");

    sceKernelGetMemBlockBaseHook = taiHookFunctionImport(&sceKernelGetMemBlockBaseRef, MY_TAI_MAIN_MODULE, 0x37FE725A, 0xB8EF5818, (const void*)&sceKernelGetMemBlockBasePatched);
    if (sceKernelGetMemBlockBaseHook < 0) LOG("Could not hook sceKernelGetMemBlockBase\n"); else LOG("hooked sceKernelGetMemBlockBase\n");

    sceSysmoduleLoadModule(SCE_SYSMODULE_NET);

    //sceNetInitHook = taiHookFunctionImport(&sceNetInitRef, TAI_MAIN_MODULE, TAI_ANY_LIBRARY, 0xEB03E265, sceNetInitPatched);
    //if (sceNetInitHook < 0) LOG("Could not hook sceNetInit\n"); else LOG("hooked sceNetInit\n");

    sceDisplaySetFrameBufHook = taiHookFunctionImport(&sceDisplaySetFrameBufRef, MY_TAI_MAIN_MODULE, 0x4FAACD11, 0x7A410B64, (const void*)&sceDisplaySetFrameBufPatched);
    if (sceDisplaySetFrameBufHook < 0) LOG("Could not hook sceDisplaySetFrameBuf\n"); else LOG("hooked sceDisplaySetFrameBuf\n");

	sceAppMgrAppParamGetString(0, 12, g_titleid , 256);


    sceIoMkdir("ux0:/data/renderdoc", 0777);
    sceIoRemove("ux0:/data/renderdoc/resources.bin");
    SceUID resource_fd = sceIoOpen("ux0:/data/renderdoc/resources.bin", SCE_O_RDWR | SCE_O_CREAT, 0777);
    sceIoWrite(resource_fd, 0, 0);
    sceIoClose(resource_fd);
    resource_fd = -1;

    sceIoRemove("ux0:/data/renderdoc/drawcalls.bin");
    resource_fd = sceIoOpen("ux0:/data/renderdoc/drawcalls.bin", SCE_O_RDWR | SCE_O_CREAT, 0777);
    sceIoWrite(resource_fd, 0, 0);
    sceIoClose(resource_fd);
    resource_fd = -1;

    File file;
    file.open("ux0:/data/renderdoc/resources.bin");
    g_resource_manager.init(file);

    g_resourceid = 1;

    /*
    kuIoRemove("ux0:/data/renderdoc/test");
    SceUID test_fd;
    kuIoOpen("ux0:/data/renderdoc/test", SCE_O_RDWR | SCE_O_CREAT, &test_fd);
    kuIoWrite(test_fd, 0, 0);
    kuIoClose(test_fd);
    test_fd = -1;

    File file;
    file.open("ux0:/data/renderdoc/test");

    ProgramHeader header;
    header.length = 1234;
    header.magic[0] = 't';
    header.magic[1] = 'e';
    header.magic[2] = 's';
    header.magic[3] = 't';
    header.version = 4321;

    file.write(header);
    header.magic[1] = 'a';
    file.write(header);
    header.magic[1] = 'i';
    file.write(header);
    file.close();

    header.magic[1] = 'o';
    file.replaceData(sizeof(ProgramHeader), sizeof(ProgramHeader), &header, sizeof(ProgramHeader));
    file.close();

    uint32_t test = 0;
    file.write(test);
    ++test;
    file.write(test);
    ++test;
    file.write(test);
    ++test;
    file.write(test);
    file.close();

    file.removeData(sizeof(uint32_t) * 2, sizeof(uint32_t));
    file.close();
    */

    //sceDisplayWaitVblankStartHook = taiHookFunctionImport(&sceDisplayWaitVblankStartRef, TAI_MAIN_MODULE, 0x5ED8F994, 0x05F27764, sceDisplayWaitVblankStartPatched);
    //if (sceDisplayWaitVblankStartHook < 0) LOG("Could not hook sceDisplayWaitVblankStart\n"); else LOG("hooked sceDisplayWaitVblankStart\n");

   // serverThreadId = createThread("TargetControlServerThread", &sRemoteControlThreadInit, 0, NULL);
    //clientControlThreadId = createThread("TargetClientControlThread", &sClientControlStartThreadInit, 0, NULL);

    SceUID memuid = sceKernelAllocMemBlock("test",
		SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE, 2 * 1024 * 1024, NULL);
    if (sceKernelGetMemBlockBase(memuid, &g_baseAddr) < 0) {
        LOG("could not get base addr\n");
    }
    LOG("CDialog allocation: memuid: %" PRIi32 ", base addr: %p\n", memuid, g_baseAddr);

    g_memoryArray = new(g_baseAddr)MappedMemory[MAX_MAPPED_MEMORY_COUNT];

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize args, void *argp) {
    LOG("end gxm_inject\n");
    return SCE_KERNEL_STOP_SUCCESS;
}

#ifdef __cplusplus
}
#endif
