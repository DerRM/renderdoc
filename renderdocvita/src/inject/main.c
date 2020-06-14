#include <vitasdk.h>
#include <taihen.h>

#include <inttypes.h>

#include "../logging.h"

#define CREATE_PATCHED_CALL(return_type, name, ...)  \
static tai_hook_ref_t name##Ref;                     \
static SceUID name##Hook;                            \
static return_type name##Patched(__VA_ARGS__)

#define IMPORT_HOOK(name, nid) \
    do { \
        name##Hook = taiHookFunctionImport(&name##Ref, TAI_MAIN_MODULE, 0xF76B66BD, nid, name##Patched); \
        if (name##Hook < 0) sceClibPrintf("Could not hook " #name "\n"); \
    } while(0)

CREATE_PATCHED_CALL(int, sceGxmAddRazorGpuCaptureBuffer, void* base, unsigned int size)
{
    sceClibPrintf("sceGxmAddRazorGpuCaptureBuffer(base: %p, size: %" PRIu32 ")\n", base, size);
    return TAI_CONTINUE(int, sceGxmAddRazorGpuCaptureBufferRef, base, size);
}

CREATE_PATCHED_CALL(int, sceGxmBeginCommandList, SceGxmContext *deferredContext)
{
    sceClibPrintf("sceGxmBeginCommandList(deferredContext: %p)\n", deferredContext);
    return TAI_CONTINUE(int, sceGxmBeginCommandListRef, deferredContext);
}

CREATE_PATCHED_CALL(int, sceGxmBeginScene, SceGxmContext *context, unsigned int flags, const SceGxmRenderTarget *renderTarget, const SceGxmValidRegion *validRegion, SceGxmSyncObject *vertexSyncObject, SceGxmSyncObject *fragmentSyncObject, const SceGxmColorSurface *colorSurface, const SceGxmDepthStencilSurface *depthStencil)
{
    sceClibPrintf("sceGxmBeginScene(context: %p, flags: %d, rendertarget: %p, validRegion: %p, vertexSyncObject: %p, fragmentSyncObject: %p, colorSurface: %p, depthStencil: %p)\n", context, flags, renderTarget, validRegion, vertexSyncObject, fragmentSyncObject, colorSurface, depthStencil);
    return TAI_CONTINUE(int, sceGxmBeginSceneRef, context, flags, renderTarget, validRegion, vertexSyncObject, fragmentSyncObject, colorSurface, depthStencil);
}

CREATE_PATCHED_CALL(int, sceGxmBeginSceneEx, SceGxmContext *context, unsigned int flags, const SceGxmRenderTarget *renderTarget, const SceGxmValidRegion *validRegion, SceGxmSyncObject *vertexSyncObject, SceGxmSyncObject *fragmentSyncObject, const SceGxmColorSurface *colorSurface, const SceGxmDepthStencilSurface *loadDepthStencil, const SceGxmDepthStencilSurface *storeDepthStencil)
{
    sceClibPrintf("sceGxmBeginSceneEx(context: %p, flags: %d, rendertarget: %p, validRegion: %p, vertexSyncObject: %p, fragmentSyncObject: %p, colorSurface: %p, loadDepthStencil: %p, storeDepthStencil: %p)\n", context, flags, renderTarget, validRegion, vertexSyncObject, fragmentSyncObject, colorSurface, loadDepthStencil, storeDepthStencil);
    return TAI_CONTINUE(int, sceGxmBeginSceneExRef, context, flags, renderTarget, validRegion, vertexSyncObject, fragmentSyncObject, colorSurface, loadDepthStencil, storeDepthStencil);
}

CREATE_PATCHED_CALL(void, sceGxmColorSurfaceGetClip, const SceGxmColorSurface *surface, unsigned int *xMin, unsigned int *yMin, unsigned int *xMax, unsigned int *yMax)
{
    sceClibPrintf("sceGxmColorSurfaceGetClip(surface: %p, xMin: %p, yMin: %p, xMax: %p, yMax: %p)\n", surface, xMin, yMin, xMax, yMax);
    TAI_CONTINUE(void, sceGxmColorSurfaceGetClipRef, surface, xMin, yMin, xMax, yMax);
}

CREATE_PATCHED_CALL(void *, sceGxmColorSurfaceGetData, const SceGxmColorSurface *surface)
{
    sceClibPrintf("sceGxmColorSurfaceGetData(surface: %p)\n", surface);
    return TAI_CONTINUE(void *, sceGxmColorSurfaceGetDataRef, surface);
}

CREATE_PATCHED_CALL(SceGxmColorSurfaceDitherMode, sceGxmColorSurfaceGetDitherMode, const SceGxmColorSurface *surface)
{
    sceClibPrintf("sceGxmColorSurfaceGetDitherMode(surface: %p)\n", surface);
    return TAI_CONTINUE(SceGxmColorSurfaceDitherMode, sceGxmColorSurfaceGetDitherModeRef, surface);
}

CREATE_PATCHED_CALL(SceGxmColorFormat, sceGxmColorSurfaceGetFormat, const SceGxmColorSurface *surface)
{
    sceClibPrintf("sceGxmColorSurfaceGetFormat(surface: %p)\n", surface);
    return TAI_CONTINUE(SceGxmColorFormat, sceGxmColorSurfaceGetFormatRef, surface);
}

CREATE_PATCHED_CALL(SceGxmColorSurfaceGammaMode, sceGxmColorSurfaceGetGammaMode, const SceGxmColorSurface *surface)
{
    sceClibPrintf("sceGxmColorSurfaceGetGammaMode(surface: %p)\n", surface);
    return TAI_CONTINUE(SceGxmColorSurfaceGammaMode, sceGxmColorSurfaceGetGammaModeRef, surface);
}

CREATE_PATCHED_CALL(SceGxmColorSurfaceScaleMode, sceGxmColorSurfaceGetScaleMode, const SceGxmColorSurface *surface)
{
    sceClibPrintf("sceGxmColorSurfaceGetScaleMode(surface: %p)\n", surface);
    return TAI_CONTINUE(SceGxmColorSurfaceScaleMode, sceGxmColorSurfaceGetScaleModeRef, surface);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmColorSurfaceGetStrideInPixels, const SceGxmColorSurface *surface)
{
    sceClibPrintf("sceGxmColorSurfaceGetStrideInPixels(surface: %p)\n", surface);
    return TAI_CONTINUE(unsigned int, sceGxmColorSurfaceGetStrideInPixelsRef, surface);
}

CREATE_PATCHED_CALL(SceGxmColorSurfaceType, sceGxmColorSurfaceGetType, const SceGxmColorSurface *surface)
{
    sceClibPrintf("sceGxmColorSurfaceGetType(surface: %p)\n", surface);
    return TAI_CONTINUE(SceGxmColorSurfaceType, sceGxmColorSurfaceGetTypeRef, surface);
}

CREATE_PATCHED_CALL(int, sceGxmColorSurfaceInit, SceGxmColorSurface *surface, SceGxmColorFormat colorFormat, SceGxmColorSurfaceType surfaceType, SceGxmColorSurfaceScaleMode scaleMode, SceGxmOutputRegisterSize outputRegisterSize, unsigned int width, unsigned int height, unsigned int strideInPixels, void *data)
{
    sceClibPrintf("sceGxmColorSurfaceInit(surface: %p, colorFormat: %" PRIu32 ", surfaceType: %" PRIu32 ", scaleMode: %" PRIu32", outputRegisterSize: %" PRIu32 ", width: %" PRIu32 ", height: %" PRIu32 ", strideInPixels: %" PRIu32 ", data: %p)\n", surface, colorFormat, surfaceType, scaleMode, outputRegisterSize, width, height, strideInPixels, data);
    return TAI_CONTINUE(int, sceGxmColorSurfaceInitRef, surface, colorFormat, surfaceType, scaleMode, outputRegisterSize, width, height, strideInPixels, data);
}

CREATE_PATCHED_CALL(int, sceGxmColorSurfaceInitDisabled, SceGxmColorSurface *surface)
{
    sceClibPrintf("sceGxmColorSurfaceInitDisabled(surface: %p)\n", surface);
    return TAI_CONTINUE(int, sceGxmColorSurfaceInitDisabledRef, surface);
}

CREATE_PATCHED_CALL(SceBool, sceGxmColorSurfaceIsEnabled, const SceGxmColorSurface *surface)
{
    sceClibPrintf("sceGxmColorSurfaceIsEnabled(surface: %p)\n", surface);
    return TAI_CONTINUE(SceBool, sceGxmColorSurfaceIsEnabledRef, surface);
}

CREATE_PATCHED_CALL(void, sceGxmColorSurfaceSetClip, SceGxmColorSurface *surface, unsigned int xMin, unsigned int yMin, unsigned int xMax, unsigned int yMax)
{
    sceClibPrintf("sceGxmColorSurfaceSetClip(surface: %p, xMin: %" PRIu32 ", yMin: %" PRIu32 ", xMax: %" PRIu32 ", yMax: %" PRIu32 ")\n", surface, xMin, yMin, xMax, yMax);
    return TAI_CONTINUE(void, sceGxmColorSurfaceSetClipRef, surface, xMin, yMin, xMax, yMax);
}

CREATE_PATCHED_CALL(int, sceGxmColorSurfaceSetData, SceGxmColorSurface *surface, void *data)
{
    sceClibPrintf("sceGxmColorSurfaceSetData(surface: %p, data: %p)\n", surface, data);
    return TAI_CONTINUE(int, sceGxmColorSurfaceSetDataRef, surface, data);
}

CREATE_PATCHED_CALL(int, sceGxmColorSurfaceSetDitherMode, SceGxmColorSurface *surface, SceGxmColorSurfaceDitherMode ditherMode)
{
    sceClibPrintf("sceGxmColorSurfaceSetDitherMode(surface: %p, scaleMode: %" PRIu32 ")\n", surface, ditherMode);
    return TAI_CONTINUE(int, sceGxmColorSurfaceSetDitherModeRef, surface, ditherMode);
}

CREATE_PATCHED_CALL(int, sceGxmColorSurfaceSetFormat, SceGxmColorSurface *surface, SceGxmColorFormat format)
{
    sceClibPrintf("sceGxmColorSurfaceSetFormat(surface: %p, scaleMode: %" PRIu32 ")\n", surface, format);
    return TAI_CONTINUE(int, sceGxmColorSurfaceSetFormatRef, surface, format);
}

CREATE_PATCHED_CALL(int, sceGxmColorSurfaceSetGammaMode, SceGxmColorSurface *surface, SceGxmColorSurfaceGammaMode gammaMode)
{
    sceClibPrintf("sceGxmColorSurfaceSetGammaMode(surface: %p, scaleMode: %" PRIu32 ")\n", surface, gammaMode);
    return TAI_CONTINUE(int, sceGxmColorSurfaceSetGammaModeRef, surface, gammaMode);
}

CREATE_PATCHED_CALL(void, sceGxmColorSurfaceSetScaleMode, SceGxmColorSurface *surface, SceGxmColorSurfaceScaleMode scaleMode)
{
    sceClibPrintf("sceGxmColorSurfaceSetScaleMode(surface: %p, scaleMode: %" PRIu32 ")\n", surface, scaleMode);
    TAI_CONTINUE(void, sceGxmColorSurfaceSetScaleModeRef, surface, scaleMode);
}

CREATE_PATCHED_CALL(int, sceGxmCreateContext, const SceGxmContextParams *params, SceGxmContext **context)
{
    sceClibPrintf("sceGxmCreateContext(params: %p, context: %p)\n", params, context);
    return TAI_CONTINUE(int, sceGxmCreateContextRef, params, context);
}

CREATE_PATCHED_CALL(int, sceGxmCreateDeferredContext, const SceGxmContextParams *params, SceGxmContext **deferredContext)
{
    sceClibPrintf("sceGxmCreateDeferredContext(params: %p, deferredContext: %p)\n", params, deferredContext);
    return TAI_CONTINUE(int, sceGxmCreateDeferredContextRef, params, deferredContext);
}

CREATE_PATCHED_CALL(int, sceGxmCreateRenderTarget, const SceGxmRenderTargetParams *params, SceGxmRenderTarget **renderTarget)
{
    sceClibPrintf("sceGxmCreateRenderTarget(params: %p, renderTarget: %p)\n", params, renderTarget);
    return TAI_CONTINUE(int, sceGxmCreateRenderTargetRef, params, renderTarget);
}

CREATE_PATCHED_CALL(float, sceGxmDepthStencilSurfaceGetBackgroundDepth, const SceGxmDepthStencilSurface *surface)
{
    sceClibPrintf("sceGxmDepthStencilSurfaceGetBackgroundDepth(surface: %p)\n", surface);
    return TAI_CONTINUE(float, sceGxmDepthStencilSurfaceGetBackgroundDepthRef, surface);
}

CREATE_PATCHED_CALL(SceBool, sceGxmDepthStencilSurfaceGetBackgroundMask, const SceGxmDepthStencilSurface *surface)
{
    sceClibPrintf("sceGxmDepthStencilSurfaceGetBackgroundMask(surface: %p)\n", surface);
    return TAI_CONTINUE(SceBool, sceGxmDepthStencilSurfaceGetBackgroundMaskRef, surface);
}

CREATE_PATCHED_CALL(unsigned char, sceGxmDepthStencilSurfaceGetBackgroundStencil, const SceGxmDepthStencilSurface *surface)
{
    sceClibPrintf("sceGxmDepthStencilSurfaceGetBackgroundStencil(surface: %p)\n", surface);
    return TAI_CONTINUE(unsigned char, sceGxmDepthStencilSurfaceGetBackgroundStencilRef, surface);
}

CREATE_PATCHED_CALL(SceGxmDepthStencilForceLoadMode, sceGxmDepthStencilSurfaceGetForceLoadMode, const SceGxmDepthStencilSurface *surface)
{
    sceClibPrintf("sceGxmDepthStencilSurfaceGetForceLoadMode(surface: %p)\n", surface);
    return TAI_CONTINUE(SceGxmDepthStencilForceLoadMode, sceGxmDepthStencilSurfaceGetForceLoadModeRef, surface);
}

CREATE_PATCHED_CALL(SceGxmDepthStencilForceStoreMode, sceGxmDepthStencilSurfaceGetForceStoreMode, const SceGxmDepthStencilSurface *surface)
{
    sceClibPrintf("sceGxmDepthStencilSurfaceGetForceStoreMode(surface: %p)\n", surface);
    return TAI_CONTINUE(SceGxmDepthStencilForceStoreMode, sceGxmDepthStencilSurfaceGetForceStoreModeRef, surface);
}

CREATE_PATCHED_CALL(SceGxmDepthStencilFormat, sceGxmDepthStencilSurfaceGetFormat, const SceGxmDepthStencilSurface *surface)
{
    sceClibPrintf("sceGxmDepthStencilSurfaceGetFormat(surface: %p)\n", surface);
    return TAI_CONTINUE(SceGxmDepthStencilFormat, sceGxmDepthStencilSurfaceGetFormatRef, surface);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmDepthStencilSurfaceGetStrideInSamples, const SceGxmDepthStencilSurface *surface)
{
    sceClibPrintf("sceGxmDepthStencilSurfaceGetStrideInSamples(surface: %p)\n", surface);
    return TAI_CONTINUE(unsigned int, sceGxmDepthStencilSurfaceGetStrideInSamplesRef, surface);
}

CREATE_PATCHED_CALL(int, sceGxmDepthStencilSurfaceInit, SceGxmDepthStencilSurface *surface, SceGxmDepthStencilFormat depthStencilFormat, SceGxmDepthStencilSurfaceType surfaceType, unsigned int strideInSamples, void *depthData, void *stencilData)
{
    sceClibPrintf("sceGxmDepthStencilSurfaceInit(surface: %p, depthStencilFormat: %" PRIu32 ", surfaceType: " PRIu32 ", strideInSamples: " PRIu32 ", depthData: %p, stencilData: %p)\n", surface, depthStencilFormat, surfaceType, strideInSamples, depthData, stencilData);
    return TAI_CONTINUE(int, sceGxmDepthStencilSurfaceInitRef, surface, depthStencilFormat, surfaceType, strideInSamples, depthData, stencilData);
}

CREATE_PATCHED_CALL(int, sceGxmDepthStencilSurfaceInitDisabled, SceGxmDepthStencilSurface *surface)
{
    sceClibPrintf("sceGxmDepthStencilSurfaceInitDisabled(surface: %p)\n", surface);
    return TAI_CONTINUE(int, sceGxmDepthStencilSurfaceInitDisabledRef, surface);
};

CREATE_PATCHED_CALL(SceBool, sceGxmDepthStencilSurfaceIsEnabled, const SceGxmDepthStencilSurface *surface)
{
    sceClibPrintf("sceGxmDepthStencilSurfaceIsEnabled(surface: %p)\n", surface);
    return TAI_CONTINUE(SceBool, sceGxmDepthStencilSurfaceIsEnabledRef, surface);
}

CREATE_PATCHED_CALL(void, sceGxmDepthStencilSurfaceSetBackgroundDepth, SceGxmDepthStencilSurface *surface, float backgroundDepth)
{
    sceClibPrintf("sceGxmDepthStencilSurfaceIsEnabled(surface: %p, backgroundDepth: %f)\n", surface, backgroundDepth);
    return TAI_CONTINUE(void, sceGxmDepthStencilSurfaceSetBackgroundDepthRef, surface, backgroundDepth);
}

CREATE_PATCHED_CALL(void, sceGxmDepthStencilSurfaceSetBackgroundMask, SceGxmDepthStencilSurface *surface, SceBool backgroundMask)
{
    sceClibPrintf("sceGxmDepthStencilSurfaceIsEnabled(surface: %p, backgroundMask: %" PRIi32 ")\n", surface, backgroundMask);
    TAI_CONTINUE(void, sceGxmDepthStencilSurfaceSetBackgroundMaskRef, surface, backgroundMask);
}

CREATE_PATCHED_CALL(void, sceGxmDepthStencilSurfaceSetBackgroundStencil, SceGxmDepthStencilSurface *surface, unsigned char backgroundStencil)
{
    sceClibPrintf("sceGxmDepthStencilSurfaceSetBackgroundStencil(surface: %p, backgroundStencil: %" PRIu32 ")\n", surface, backgroundStencil);
    TAI_CONTINUE(void, sceGxmDepthStencilSurfaceSetBackgroundStencilRef, surface, backgroundStencil);
}

CREATE_PATCHED_CALL(void, sceGxmDepthStencilSurfaceSetForceLoadMode, SceGxmDepthStencilSurface *surface, SceGxmDepthStencilForceLoadMode forceLoad)
{
    sceClibPrintf("sceGxmDepthStencilSurfaceSetForceLoadMode(surface: %p, forceLoad: %" PRIu32 ")\n", surface, forceLoad);
    TAI_CONTINUE(void, sceGxmDepthStencilSurfaceSetForceLoadModeRef, surface, forceLoad);
}

CREATE_PATCHED_CALL(void, sceGxmDepthStencilSurfaceSetForceStoreMode, SceGxmDepthStencilSurface *surface, SceGxmDepthStencilForceStoreMode forceStore)
{
    sceClibPrintf("sceGxmDepthStencilSurfaceSetForceStoreMode(surface: %p, forceStore: %" PRIu32 ")\n", surface, forceStore);
    TAI_CONTINUE(void, sceGxmDepthStencilSurfaceSetForceStoreModeRef, surface, forceStore);
}

CREATE_PATCHED_CALL(int, sceGxmInitialize, const SceGxmInitializeParams *params)
{
    sceClibPrintf("sceGxmInitialize(params: %p)\n", params);
    return TAI_CONTINUE(int, sceGxmInitializeRef, params);
}

CREATE_PATCHED_CALL(int, sceGxmTerminate)
{
    sceClibPrintf("sceGxmTerminate()\n");
    return TAI_CONTINUE(int, sceGxmTerminateRef);
}

CREATE_PATCHED_CALL(volatile unsigned int *, sceGxmGetNotificationRegion)
{
    return TAI_CONTINUE(volatile unsigned int *, sceGxmGetNotificationRegionRef);
}

CREATE_PATCHED_CALL(int, sceGxmNotificationWait, const SceGxmNotification *notification)
{
    return TAI_CONTINUE(int, sceGxmNotificationWaitRef, notification);
}

CREATE_PATCHED_CALL(int, sceGxmMapMemory, void *base, SceSize size, SceGxmMemoryAttribFlags attr)
{
    return TAI_CONTINUE(int, sceGxmMapMemoryRef, base, size, attr);
}

CREATE_PATCHED_CALL(int, sceGxmUnmapMemory, void *base)
{
    return TAI_CONTINUE(int, sceGxmUnmapMemoryRef, base);
}

CREATE_PATCHED_CALL(int, sceGxmMapVertexUsseMemory, void *base, SceSize size, unsigned int *offset)
{
    return TAI_CONTINUE(int, sceGxmMapVertexUsseMemoryRef, base, size, offset);
}
CREATE_PATCHED_CALL(int, sceGxmUnmapVertexUsseMemory, void *base)
{
    return TAI_CONTINUE(int, sceGxmUnmapVertexUsseMemoryRef, base);
}

CREATE_PATCHED_CALL(int, sceGxmMapFragmentUsseMemory, void *base, SceSize size, unsigned int *offset)
{
    return TAI_CONTINUE(int, sceGxmMapFragmentUsseMemoryRef, base, size, offset);
}

CREATE_PATCHED_CALL(int, sceGxmUnmapFragmentUsseMemory, void *base)
{
    return TAI_CONTINUE(int, sceGxmUnmapFragmentUsseMemoryRef, base);
}

CREATE_PATCHED_CALL(int, sceGxmDisplayQueueAddEntry, SceGxmSyncObject *oldBuffer, SceGxmSyncObject *newBuffer, const void *callbackData)
{
    return TAI_CONTINUE(int, sceGxmDisplayQueueAddEntryRef, oldBuffer, newBuffer, callbackData);
}

CREATE_PATCHED_CALL(int, sceGxmDisplayQueueFinish)
{
    return TAI_CONTINUE(int, sceGxmDisplayQueueFinishRef);
}

CREATE_PATCHED_CALL(int, sceGxmSyncObjectCreate, SceGxmSyncObject **syncObject)
{
    return TAI_CONTINUE(int, sceGxmSyncObjectCreateRef, syncObject);
}

CREATE_PATCHED_CALL(int, sceGxmSyncObjectDestroy, SceGxmSyncObject *syncObject)
{
    return TAI_CONTINUE(int, sceGxmSyncObjectDestroyRef, syncObject);
}

CREATE_PATCHED_CALL(int, sceGxmDestroyContext, SceGxmContext *context)
{
    return TAI_CONTINUE(int, sceGxmDestroyContextRef, context);
}

CREATE_PATCHED_CALL(void, sceGxmSetValidationEnable, SceGxmContext *context, SceBool enable)
{
    TAI_CONTINUE(void, sceGxmSetValidationEnableRef, context, enable);
}

CREATE_PATCHED_CALL(void, sceGxmSetVertexProgram, SceGxmContext *context, const SceGxmVertexProgram *vertexProgram)
{
    TAI_CONTINUE(void, sceGxmSetVertexProgramRef, context, vertexProgram);
}

CREATE_PATCHED_CALL(void, sceGxmSetFragmentProgram, SceGxmContext *context, const SceGxmFragmentProgram *fragmentProgram)
{
    TAI_CONTINUE(void, sceGxmSetFragmentProgramRef, context, fragmentProgram);
}

CREATE_PATCHED_CALL(int, sceGxmReserveVertexDefaultUniformBuffer, SceGxmContext *context, void **uniformBuffer)
{
    return TAI_CONTINUE(int, sceGxmReserveVertexDefaultUniformBufferRef, context, uniformBuffer);
}

CREATE_PATCHED_CALL(int, sceGxmReserveFragmentDefaultUniformBuffer, SceGxmContext *context, void **uniformBuffer)
{
    return TAI_CONTINUE(int, sceGxmReserveFragmentDefaultUniformBufferRef, context, uniformBuffer);
}

CREATE_PATCHED_CALL(int, sceGxmSetVertexStream, SceGxmContext *context, unsigned int streamIndex, const void *streamData)
{
    return TAI_CONTINUE(int, sceGxmSetVertexStreamRef, context, streamIndex, streamData);
}

CREATE_PATCHED_CALL(int, sceGxmSetVertexTexture, SceGxmContext *context, unsigned int textureIndex, const SceGxmTexture *texture)
{
    return TAI_CONTINUE(int, sceGxmSetVertexTextureRef, context, textureIndex, texture);
}

CREATE_PATCHED_CALL(int, sceGxmSetFragmentTexture, SceGxmContext *context, unsigned int textureIndex, const SceGxmTexture *texture)
{
    return TAI_CONTINUE(int, sceGxmSetFragmentTextureRef, context, textureIndex, texture);
}

CREATE_PATCHED_CALL(int, sceGxmSetVertexUniformBuffer, SceGxmContext *context, unsigned int bufferIndex, const void *bufferData)
{
    return TAI_CONTINUE(int, sceGxmSetVertexUniformBufferRef, context, bufferIndex, bufferData);
}

CREATE_PATCHED_CALL(int, sceGxmSetFragmentUniformBuffer, SceGxmContext *context, unsigned int bufferIndex, const void *bufferData)
{
    return TAI_CONTINUE(int, sceGxmSetFragmentUniformBufferRef, context, bufferIndex, bufferData);
}

CREATE_PATCHED_CALL(int, sceGxmSetAuxiliarySurface, SceGxmContext *context, unsigned int surfaceIndex, const SceGxmAuxiliarySurface *surface)
{
    return TAI_CONTINUE(int, sceGxmSetAuxiliarySurfaceRef, context, surfaceIndex, surface);
}

CREATE_PATCHED_CALL(void, sceGxmSetPrecomputedFragmentState, SceGxmContext *context, const SceGxmPrecomputedFragmentState *precomputedState)
{
    TAI_CONTINUE(void, sceGxmSetPrecomputedFragmentStateRef, context, precomputedState);
}

CREATE_PATCHED_CALL(void, sceGxmSetPrecomputedVertexState, SceGxmContext *context, const SceGxmPrecomputedVertexState *precomputedState)
{
    TAI_CONTINUE(void, sceGxmSetPrecomputedVertexStateRef, context, precomputedState);
}

CREATE_PATCHED_CALL(int, sceGxmDrawPrecomputed, SceGxmContext *context, const SceGxmPrecomputedDraw *precomputedDraw)
{
    return TAI_CONTINUE(int, sceGxmDrawPrecomputedRef, context, precomputedDraw);
}

CREATE_PATCHED_CALL(int, sceGxmDraw, SceGxmContext* context, SceGxmPrimitiveType primType, SceGxmIndexFormat indexType, const void* indexData, unsigned int indexCount)
{
    sceClibPrintf("sceGxmDraw(context: %p, primType: %d, indexType: %d, indexData: %p, indexCount: %d)\n", context, primType, indexType, indexData, indexCount);
    return TAI_CONTINUE(int, sceGxmDrawRef, context, primType, indexType, indexData, indexCount);
}

CREATE_PATCHED_CALL(int, sceGxmDrawInstanced, SceGxmContext *context, SceGxmPrimitiveType primType, SceGxmIndexFormat indexType, const void *indexData, unsigned int indexCount, unsigned int indexWrap)
{
    return TAI_CONTINUE(int, sceGxmDrawInstancedRef, context, primType, indexType, indexData, indexCount, indexWrap);
}

CREATE_PATCHED_CALL(int, sceGxmSetVisibilityBuffer, SceGxmContext *context, void *bufferBase, unsigned int stridePerCore)
{
    return TAI_CONTINUE(int, sceGxmSetVisibilityBufferRef, context, bufferBase, stridePerCore);
}

CREATE_PATCHED_CALL(int, sceGxmMidSceneFlush, SceGxmContext *context, unsigned int flags, SceGxmSyncObject *vertexSyncObject, const SceGxmNotification *vertexNotification)
{
    return TAI_CONTINUE(int, sceGxmMidSceneFlushRef, context, flags, vertexSyncObject, vertexNotification);
}

CREATE_PATCHED_CALL(int, sceGxmEndScene, SceGxmContext *context, const SceGxmNotification *vertexNotification, const SceGxmNotification *fragmentNotification)
{
    return TAI_CONTINUE(int, sceGxmEndSceneRef, context, vertexNotification, fragmentNotification);
}

CREATE_PATCHED_CALL(void, sceGxmSetFrontDepthFunc, SceGxmContext *context, SceGxmDepthFunc depthFunc)
{
    TAI_CONTINUE(void, sceGxmSetFrontDepthFuncRef, context, depthFunc);
}

CREATE_PATCHED_CALL(void, sceGxmSetBackDepthFunc, SceGxmContext *context, SceGxmDepthFunc depthFunc)
{
    TAI_CONTINUE(void, sceGxmSetBackDepthFuncRef, context, depthFunc);
}

CREATE_PATCHED_CALL(void, sceGxmSetFrontFragmentProgramEnable, SceGxmContext *context, SceGxmFragmentProgramMode enable)
{
    TAI_CONTINUE(void, sceGxmSetFrontFragmentProgramEnableRef, context, enable);
}

CREATE_PATCHED_CALL(void, sceGxmSetBackFragmentProgramEnable, SceGxmContext *context, SceGxmFragmentProgramMode enable)
{
    TAI_CONTINUE(void, sceGxmSetBackFragmentProgramEnableRef, context, enable);
}

CREATE_PATCHED_CALL(void, sceGxmSetFrontDepthWriteEnable, SceGxmContext *context, SceGxmDepthWriteMode enable)
{
    TAI_CONTINUE(void, sceGxmSetFrontDepthWriteEnableRef, context, enable);
}

CREATE_PATCHED_CALL(void, sceGxmSetBackDepthWriteEnable, SceGxmContext *context, SceGxmDepthWriteMode enable)
{
    TAI_CONTINUE(void, sceGxmSetBackDepthWriteEnableRef, context, enable);
}

CREATE_PATCHED_CALL(void, sceGxmSetFrontLineFillLastPixelEnable, SceGxmContext *context, SceGxmLineFillLastPixelMode enable)
{
    TAI_CONTINUE(void, sceGxmSetFrontLineFillLastPixelEnableRef, context, enable);
}

CREATE_PATCHED_CALL(void, sceGxmSetBackLineFillLastPixelEnable, SceGxmContext *context, SceGxmLineFillLastPixelMode enable)
{
    TAI_CONTINUE(void, sceGxmSetBackLineFillLastPixelEnableRef, context, enable);
}

CREATE_PATCHED_CALL(void, sceGxmSetFrontStencilRef, SceGxmContext *context, unsigned int sref)
{
    TAI_CONTINUE(void, sceGxmSetFrontStencilRefRef, context, sref);
}

CREATE_PATCHED_CALL(void, sceGxmSetBackStencilRef, SceGxmContext *context, unsigned int sref)
{
    TAI_CONTINUE(void, sceGxmSetBackStencilRefRef, context, sref);
}

CREATE_PATCHED_CALL(void, sceGxmSetFrontPointLineWidth, SceGxmContext *context, unsigned int width)
{
    TAI_CONTINUE(void, sceGxmSetFrontPointLineWidthRef, context, width);
}

CREATE_PATCHED_CALL(void, sceGxmSetBackPointLineWidth, SceGxmContext *context, unsigned int width)
{
    TAI_CONTINUE(void, sceGxmSetBackPointLineWidthRef, context, width);
}

CREATE_PATCHED_CALL(void, sceGxmSetFrontPolygonMode, SceGxmContext *context, SceGxmPolygonMode mode)
{
    TAI_CONTINUE(void, sceGxmSetFrontPolygonModeRef, context, mode);
}

CREATE_PATCHED_CALL(void, sceGxmSetBackPolygonMode, SceGxmContext *context, SceGxmPolygonMode mode)
{
    TAI_CONTINUE(void, sceGxmSetBackPolygonModeRef, context, mode);
}

CREATE_PATCHED_CALL(void, sceGxmSetFrontStencilFunc, SceGxmContext *context, SceGxmStencilFunc func, SceGxmStencilOp stencilFail, SceGxmStencilOp depthFail, SceGxmStencilOp depthPass, unsigned char compareMask, unsigned char writeMask)
{
    TAI_CONTINUE(void, sceGxmSetFrontStencilFuncRef, context, func, stencilFail, depthFail, depthPass, compareMask, writeMask);
}

CREATE_PATCHED_CALL(void, sceGxmSetBackStencilFunc, SceGxmContext *context, SceGxmStencilFunc func, SceGxmStencilOp stencilFail, SceGxmStencilOp depthFail, SceGxmStencilOp depthPass, unsigned char compareMask, unsigned char writeMask)
{
    TAI_CONTINUE(void, sceGxmSetBackStencilFuncRef, context, func, stencilFail, depthFail, depthPass, compareMask, writeMask);
}

CREATE_PATCHED_CALL(void, sceGxmSetFrontDepthBias, SceGxmContext *context, int factor, int units)
{
    TAI_CONTINUE(void, sceGxmSetFrontDepthBiasRef, context, factor, units);
}

CREATE_PATCHED_CALL(void, sceGxmSetBackDepthBias, SceGxmContext *context, int factor, int units)
{
    TAI_CONTINUE(void, sceGxmSetBackDepthBiasRef, context, factor, units);
}

CREATE_PATCHED_CALL(void, sceGxmSetTwoSidedEnable, SceGxmContext *context, SceGxmTwoSidedMode enable)
{
    TAI_CONTINUE(void, sceGxmSetTwoSidedEnableRef, context, enable);
}

CREATE_PATCHED_CALL(void, sceGxmSetViewport, SceGxmContext *context, float xOffset, float xScale, float yOffset, float yScale, float zOffset, float zScale)
{
    TAI_CONTINUE(void, sceGxmSetViewportRef, context, xOffset, xScale, yOffset, yScale, zOffset, zScale);
}

CREATE_PATCHED_CALL(void, sceGxmSetWClampValue, SceGxmContext *context, float clampValue)
{
    TAI_CONTINUE(void, sceGxmSetWClampValueRef, context, clampValue);
}

CREATE_PATCHED_CALL(void, sceGxmSetWClampEnable, SceGxmContext *context, SceGxmWClampMode enable)
{
    TAI_CONTINUE(void, sceGxmSetWClampEnableRef, context, enable);
}

CREATE_PATCHED_CALL(void, sceGxmSetRegionClip, SceGxmContext *context, SceGxmRegionClipMode mode, unsigned int xMin, unsigned int yMin, unsigned int xMax, unsigned int yMax)
{
    TAI_CONTINUE(void, sceGxmSetRegionClipRef, context, mode, xMin, yMin, xMax, yMax);
}

CREATE_PATCHED_CALL(void, sceGxmSetCullMode, SceGxmContext *context, SceGxmCullMode mode)
{
    TAI_CONTINUE(void, sceGxmSetCullModeRef, context, mode);
}

CREATE_PATCHED_CALL(void, sceGxmSetViewportEnable, SceGxmContext *context, SceGxmViewportMode enable)
{
    TAI_CONTINUE(void, sceGxmSetViewportEnableRef, context, enable);
}

CREATE_PATCHED_CALL(void, sceGxmSetWBufferEnable, SceGxmContext *context, SceGxmWBufferMode enable)
{
    TAI_CONTINUE(void, sceGxmSetWBufferEnableRef, context, enable);
}

CREATE_PATCHED_CALL(void, sceGxmSetFrontVisibilityTestIndex, SceGxmContext *context, unsigned int index)
{
    TAI_CONTINUE(void, sceGxmSetFrontVisibilityTestIndexRef, context, index);
}

CREATE_PATCHED_CALL(void, sceGxmSetBackVisibilityTestIndex, SceGxmContext *context, unsigned int index)
{
    TAI_CONTINUE(void, sceGxmSetBackVisibilityTestIndexRef, context, index);
}

CREATE_PATCHED_CALL(void, sceGxmSetFrontVisibilityTestOp, SceGxmContext *context, SceGxmVisibilityTestOp op)
{
    TAI_CONTINUE(void, sceGxmSetFrontVisibilityTestOpRef, context, op);
}

CREATE_PATCHED_CALL(void, sceGxmSetBackVisibilityTestOp, SceGxmContext *context, SceGxmVisibilityTestOp op)
{
    TAI_CONTINUE(void, sceGxmSetBackVisibilityTestOpRef, context, op);
}

CREATE_PATCHED_CALL(void, sceGxmSetFrontVisibilityTestEnable, SceGxmContext *context, SceGxmVisibilityTestMode enable)
{
    TAI_CONTINUE(void, sceGxmSetFrontVisibilityTestEnableRef, context, enable);
}

CREATE_PATCHED_CALL(void, sceGxmSetBackVisibilityTestEnable, SceGxmContext *context, SceGxmVisibilityTestMode enable)
{
    TAI_CONTINUE(void, sceGxmSetBackVisibilityTestEnableRef, context, enable);
}

CREATE_PATCHED_CALL(void, sceGxmFinish, SceGxmContext *context)
{
    TAI_CONTINUE(void, sceGxmFinishRef, context);
}

CREATE_PATCHED_CALL(int, sceGxmPushUserMarker, SceGxmContext *context, const char *tag)
{
    return TAI_CONTINUE(int, sceGxmPushUserMarkerRef, context, tag);
}

CREATE_PATCHED_CALL(int, sceGxmPopUserMarker, SceGxmContext *context)
{
    return TAI_CONTINUE(int, sceGxmPopUserMarkerRef, context);
}

CREATE_PATCHED_CALL(int, sceGxmSetUserMarker, SceGxmContext *context, const char *tag)
{
    return TAI_CONTINUE(int, sceGxmSetUserMarkerPatched, context, tag);
}

CREATE_PATCHED_CALL(int, sceGxmPadHeartbeat, const SceGxmColorSurface *displaySurface, SceGxmSyncObject *displaySyncObject)
{
    return TAI_CONTINUE(int, sceGxmPadHeartbeatRef, displaySurface, displaySyncObject);
}

CREATE_PATCHED_CALL(int, sceGxmPadTriggerGpuPaTrace)
{
    return TAI_CONTINUE(int, sceGxmPadTriggerGpuPaTraceRef);
}

CREATE_PATCHED_CALL(int, sceGxmProgramCheck, const SceGxmProgram *program)
{
    return TAI_CONTINUE(int, sceGxmProgramCheckRef, program);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmProgramGetSize, const SceGxmProgram *program)
{
    return TAI_CONTINUE(unsigned int, sceGxmProgramGetSizeRef, program);
}

CREATE_PATCHED_CALL(SceGxmProgramType, sceGxmProgramGetType, const SceGxmProgram *program)
{
    return TAI_CONTINUE(SceGxmProgramType, sceGxmProgramGetTypeRef, program);
}

CREATE_PATCHED_CALL(SceBool, sceGxmProgramIsDiscardUsed, const SceGxmProgram *program)
{
    return TAI_CONTINUE(SceBool, sceGxmProgramIsDiscardUsedRef, program);
}

CREATE_PATCHED_CALL(SceBool, sceGxmProgramIsDepthReplaceUsed, const SceGxmProgram *program)
{
    return TAI_CONTINUE(SceBool, sceGxmProgramIsDepthReplaceUsedRef, program);
}

CREATE_PATCHED_CALL(SceBool, sceGxmProgramIsSpriteCoordUsed, const SceGxmProgram *program)
{
    return TAI_CONTINUE(SceBool, sceGxmProgramIsSpriteCoordUsedRef, program);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmProgramGetDefaultUniformBufferSize, const SceGxmProgram *program)
{
    return TAI_CONTINUE(unsigned int, sceGxmProgramGetDefaultUniformBufferSizeRef, program);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmProgramGetParameterCount, const SceGxmProgram *program)
{
    return TAI_CONTINUE(unsigned int, sceGxmProgramGetParameterCountRef, program);
}

CREATE_PATCHED_CALL(const SceGxmProgramParameter *, sceGxmProgramGetParameter, const SceGxmProgram *program, unsigned int index)
{
    return TAI_CONTINUE(const SceGxmProgramParameter *, sceGxmProgramGetParameterRef, program, index);
}

CREATE_PATCHED_CALL(const SceGxmProgramParameter *, sceGxmProgramFindParameterByName, const SceGxmProgram *program, const char *name)
{
    return TAI_CONTINUE(const SceGxmProgramParameter *, sceGxmProgramFindParameterByNameRef, program, name);
}

CREATE_PATCHED_CALL(const SceGxmProgramParameter *, sceGxmProgramFindParameterBySemantic, const SceGxmProgram *program, SceGxmParameterSemantic semantic, unsigned int index)
{
    return TAI_CONTINUE(const SceGxmProgramParameter *, sceGxmProgramFindParameterBySemanticRef, program, semantic, index);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmProgramParameterGetIndex, const SceGxmProgram *program, const SceGxmProgramParameter *parameter)
{
    return TAI_CONTINUE(unsigned int, sceGxmProgramParameterGetIndexRef, program, parameter);
}

CREATE_PATCHED_CALL(SceGxmParameterCategory, sceGxmProgramParameterGetCategory, const SceGxmProgramParameter *parameter)
{
    return TAI_CONTINUE(SceGxmParameterCategory, sceGxmProgramParameterGetCategoryRef, parameter);
}

CREATE_PATCHED_CALL(const char *, sceGxmProgramParameterGetName, const SceGxmProgramParameter *parameter)
{
    return TAI_CONTINUE(const char *, sceGxmProgramParameterGetNameRef, parameter);
}

CREATE_PATCHED_CALL(SceGxmParameterSemantic, sceGxmProgramParameterGetSemantic, const SceGxmProgramParameter *parameter)
{
    return TAI_CONTINUE(SceGxmParameterSemantic, sceGxmProgramParameterGetSemanticRef, parameter);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmProgramParameterGetSemanticIndex, const SceGxmProgramParameter *parameter)
{
    return TAI_CONTINUE(unsigned int, sceGxmProgramParameterGetSemanticIndexRef, parameter);
}

CREATE_PATCHED_CALL(SceGxmParameterType, sceGxmProgramParameterGetType, const SceGxmProgramParameter *parameter)
{
    return TAI_CONTINUE(SceGxmParameterType, sceGxmProgramParameterGetTypeRef, parameter);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmProgramParameterGetComponentCount, const SceGxmProgramParameter *parameter)
{
    return TAI_CONTINUE(unsigned int, sceGxmProgramParameterGetComponentCountRef, parameter);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmProgramParameterGetArraySize, const SceGxmProgramParameter *parameter)
{
    return TAI_CONTINUE(unsigned int, sceGxmProgramParameterGetArraySizeRef, parameter);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmProgramParameterGetResourceIndex, const SceGxmProgramParameter *parameter)
{
    return TAI_CONTINUE(unsigned int, sceGxmProgramParameterGetResourceIndexRef, parameter);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmProgramParameterGetContainerIndex, const SceGxmProgramParameter *parameter)
{
    return TAI_CONTINUE(unsigned int, sceGxmProgramParameterGetContainerIndexRef, parameter);
}

CREATE_PATCHED_CALL(SceBool, sceGxmProgramParameterIsSamplerCube, const SceGxmProgramParameter *parameter)
{
    return TAI_CONTINUE(SceBool, sceGxmProgramParameterIsSamplerCubeRef, parameter);
}

CREATE_PATCHED_CALL(const SceGxmProgram *, sceGxmFragmentProgramGetProgram, const SceGxmFragmentProgram *fragmentProgram)
{
    return TAI_CONTINUE(const SceGxmProgram *, sceGxmFragmentProgramGetProgramRef, fragmentProgram);
}

CREATE_PATCHED_CALL(const SceGxmProgram *, sceGxmVertexProgramGetProgram, const SceGxmVertexProgram *vertexProgram)
{
    return TAI_CONTINUE(const SceGxmProgram *, sceGxmVertexProgramGetProgramRef, vertexProgram);
}

CREATE_PATCHED_CALL(int, sceGxmShaderPatcherCreate, const SceGxmShaderPatcherParams *params, SceGxmShaderPatcher **shaderPatcher)
{
    return TAI_CONTINUE(int, sceGxmShaderPatcherCreateRef, params, shaderPatcher);
}

CREATE_PATCHED_CALL(int, sceGxmShaderPatcherSetUserData, SceGxmShaderPatcher *shaderPatcher, void *userData)
{
    return TAI_CONTINUE(int, sceGxmShaderPatcherSetUserDataRef, shaderPatcher, userData);
}

CREATE_PATCHED_CALL(void *, sceGxmShaderPatcherGetUserData, SceGxmShaderPatcher *shaderPatcher)
{
    return TAI_CONTINUE(void *, sceGxmShaderPatcherGetUserDataRef, shaderPatcher);
}

CREATE_PATCHED_CALL(int, sceGxmShaderPatcherDestroy, SceGxmShaderPatcher *shaderPatcher)
{
    return TAI_CONTINUE(int, sceGxmShaderPatcherDestroyRef, shaderPatcher);
}

CREATE_PATCHED_CALL(int, sceGxmShaderPatcherRegisterProgram, SceGxmShaderPatcher *shaderPatcher, const SceGxmProgram *programHeader, SceGxmShaderPatcherId *programId)
{
    return TAI_CONTINUE(int, sceGxmShaderPatcherRegisterProgramRef, shaderPatcher, programHeader, programId);
}

CREATE_PATCHED_CALL(int, sceGxmShaderPatcherUnregisterProgram, SceGxmShaderPatcher *shaderPatcher, SceGxmShaderPatcherId programId)
{
    return TAI_CONTINUE(int, sceGxmShaderPatcherUnregisterProgramRef, shaderPatcher, programId);
}

CREATE_PATCHED_CALL(int, sceGxmShaderPatcherForceUnregisterProgram, SceGxmShaderPatcher *shaderPatcher, SceGxmShaderPatcherId programId)
{
    return TAI_CONTINUE(int, sceGxmShaderPatcherForceUnregisterProgramRef, shaderPatcher, programId);
}

CREATE_PATCHED_CALL(const SceGxmProgram *, sceGxmShaderPatcherGetProgramFromId, SceGxmShaderPatcherId programId)
{
    return TAI_CONTINUE(const SceGxmProgram *, sceGxmShaderPatcherGetProgramFromIdRef, programId);
}

CREATE_PATCHED_CALL(int, sceGxmShaderPatcherSetAuxiliarySurface, SceGxmShaderPatcher *shaderPatcher, unsigned int auxSurfaceIndex, const SceGxmAuxiliarySurface *auxSurface)
{
    return TAI_CONTINUE(int, sceGxmShaderPatcherSetAuxiliarySurfaceRef, shaderPatcher, auxSurfaceIndex, auxSurface);
}

CREATE_PATCHED_CALL(int, sceGxmShaderPatcherCreateVertexProgram, SceGxmShaderPatcher *shaderPatcher, SceGxmShaderPatcherId programId, const SceGxmVertexAttribute *attributes, unsigned int attributeCount, const SceGxmVertexStream *streams, unsigned int streamCount, SceGxmVertexProgram **vertexProgram)
{
    return TAI_CONTINUE(int, sceGxmShaderPatcherCreateVertexProgramRef, shaderPatcher, programId, attributes, attributeCount, streams, streamCount, vertexProgram);
}

CREATE_PATCHED_CALL(int, sceGxmShaderPatcherCreateFragmentProgram, SceGxmShaderPatcher *shaderPatcher, SceGxmShaderPatcherId programId, SceGxmOutputRegisterFormat outputFormat, SceGxmMultisampleMode multisampleMode, const SceGxmBlendInfo *blendInfo, const SceGxmProgram *vertexProgram, SceGxmFragmentProgram **fragmentProgram)
{
    return TAI_CONTINUE(int, sceGxmShaderPatcherCreateFragmentProgramRef, shaderPatcher, programId, outputFormat, multisampleMode, blendInfo, vertexProgram, fragmentProgram);
}

CREATE_PATCHED_CALL(int, sceGxmShaderPatcherCreateMaskUpdateFragmentProgram, SceGxmShaderPatcher *shaderPatcher, SceGxmFragmentProgram **fragmentProgram)
{
    return TAI_CONTINUE(int, sceGxmShaderPatcherCreateMaskUpdateFragmentProgramRef, shaderPatcher, fragmentProgram);
}

CREATE_PATCHED_CALL(int, sceGxmShaderPatcherAddRefVertexProgram, SceGxmShaderPatcher *shaderPatcher, SceGxmVertexProgram *vertexProgram)
{
    return TAI_CONTINUE(int, sceGxmShaderPatcherAddRefVertexProgramRef, shaderPatcher, vertexProgram);
}

CREATE_PATCHED_CALL(int, sceGxmShaderPatcherAddRefFragmentProgram, SceGxmShaderPatcher *shaderPatcher, SceGxmFragmentProgram *fragmentProgram)
{
    return TAI_CONTINUE(int, sceGxmShaderPatcherAddRefFragmentProgramRef, shaderPatcher, fragmentProgram);
}

CREATE_PATCHED_CALL(int, sceGxmShaderPatcherGetVertexProgramRefCount, SceGxmShaderPatcher *shaderPatcher, SceGxmVertexProgram *vertexProgram, unsigned int *count)
{
    return TAI_CONTINUE(int, sceGxmShaderPatcherGetVertexProgramRefCountRef, shaderPatcher, vertexProgram, count);
}

CREATE_PATCHED_CALL(int, sceGxmShaderPatcherGetFragmentProgramRefCount, SceGxmShaderPatcher *shaderPatcher, SceGxmFragmentProgram *fragmentProgram, unsigned int *count)
{
    return TAI_CONTINUE(int, sceGxmShaderPatcherGetFragmentProgramRefCountRef, shaderPatcher, fragmentProgram, count);
}

CREATE_PATCHED_CALL(int, sceGxmShaderPatcherReleaseVertexProgram, SceGxmShaderPatcher *shaderPatcher, SceGxmVertexProgram *vertexProgram)
{
    return TAI_CONTINUE(int, sceGxmShaderPatcherReleaseVertexProgramRef, shaderPatcher, vertexProgram);
}

CREATE_PATCHED_CALL(int, sceGxmShaderPatcherReleaseFragmentProgram, SceGxmShaderPatcher *shaderPatcher, SceGxmFragmentProgram *fragmentProgram)
{
    return TAI_CONTINUE(int, sceGxmShaderPatcherReleaseFragmentProgramRef, shaderPatcher, fragmentProgram);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmShaderPatcherGetHostMemAllocated, const SceGxmShaderPatcher *shaderPatcher)
{
    return TAI_CONTINUE(unsigned int, sceGxmShaderPatcherGetHostMemAllocatedRef, shaderPatcher);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmShaderPatcherGetBufferMemAllocated, const SceGxmShaderPatcher *shaderPatcher)
{
    return TAI_CONTINUE(unsigned int, sceGxmShaderPatcherGetBufferMemAllocatedRef, shaderPatcher);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmShaderPatcherGetVertexUsseMemAllocated, const SceGxmShaderPatcher *shaderPatcher)
{
    return TAI_CONTINUE(unsigned int, sceGxmShaderPatcherGetVertexUsseMemAllocatedRef, shaderPatcher);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmShaderPatcherGetFragmentUsseMemAllocated, const SceGxmShaderPatcher *shaderPatcher)
{
    return TAI_CONTINUE(unsigned int, sceGxmShaderPatcherGetFragmentUsseMemAllocatedRef, shaderPatcher);
}

CREATE_PATCHED_CALL(int, sceGxmTextureInitSwizzled, SceGxmTexture *texture, const void *data, SceGxmTextureFormat texFormat, unsigned int width, unsigned int height, unsigned int mipCount)
{
    return TAI_CONTINUE(int, sceGxmTextureInitSwizzledRef, texture, data, texFormat, width, height, mipCount);
}

CREATE_PATCHED_CALL(int, sceGxmTextureInitLinear, SceGxmTexture *texture, const void *data, SceGxmTextureFormat texFormat, unsigned int width, unsigned int height, unsigned int mipCount)
{
    return TAI_CONTINUE(int, sceGxmTextureInitLinearRef, texture, data, texFormat, width, height, mipCount);
}

CREATE_PATCHED_CALL(int, sceGxmTextureInitLinearStrided, SceGxmTexture *texture, const void *data, SceGxmTextureFormat texFormat, unsigned int width, unsigned int height, unsigned int byteStride)
{
    return TAI_CONTINUE(int, sceGxmTextureInitLinearStridedRef, texture, data, texFormat, width, height, byteStride);
}

CREATE_PATCHED_CALL(int, sceGxmTextureInitTiled, SceGxmTexture *texture, const void *data, SceGxmTextureFormat texFormat, unsigned int width, unsigned int height, unsigned int mipCount)
{
    return TAI_CONTINUE(int, sceGxmTextureInitTiledRef, texture, data, texFormat, width, height, mipCount);
}

CREATE_PATCHED_CALL(int, sceGxmTextureInitCube, SceGxmTexture *texture, const void *data, SceGxmTextureFormat texFormat, unsigned int width, unsigned int height, unsigned int mipCount)
{
    return TAI_CONTINUE(int, sceGxmTextureInitCubeRef, texture, data, texFormat, width, height, mipCount);
}

CREATE_PATCHED_CALL(SceGxmTextureType, sceGxmTextureGetType, const SceGxmTexture *texture)
{
    return TAI_CONTINUE(SceGxmTextureType, sceGxmTextureGetTypeRef, texture);
}

CREATE_PATCHED_CALL(int, sceGxmTextureSetMinFilter, SceGxmTexture *texture, SceGxmTextureFilter minFilter)
{
    return TAI_CONTINUE(int, sceGxmTextureSetMinFilterRef, texture, minFilter);
}

CREATE_PATCHED_CALL(SceGxmTextureFilter, sceGxmTextureGetMinFilter, const SceGxmTexture *texture)
{
    return TAI_CONTINUE(SceGxmTextureFilter, sceGxmTextureGetMinFilterRef, texture);
}

CREATE_PATCHED_CALL(int, sceGxmTextureSetMagFilter, SceGxmTexture *texture, SceGxmTextureFilter magFilter)
{
    return TAI_CONTINUE(int, sceGxmTextureSetMagFilterRef, texture, magFilter);
}

CREATE_PATCHED_CALL(SceGxmTextureFilter, sceGxmTextureGetMagFilter, const SceGxmTexture *texture)
{
    return TAI_CONTINUE(SceGxmTextureFilter, sceGxmTextureGetMagFilterRef, texture);
}

CREATE_PATCHED_CALL(int, sceGxmTextureSetMipFilter, SceGxmTexture *texture, SceGxmTextureMipFilter mipFilter)
{
    return TAI_CONTINUE(int, sceGxmTextureSetMipFilterRef, texture, mipFilter);
}

CREATE_PATCHED_CALL(SceGxmTextureMipFilter, sceGxmTextureGetMipFilter, const SceGxmTexture *texture)
{
    return TAI_CONTINUE(SceGxmTextureMipFilter, sceGxmTextureGetMipFilterRef, texture);
}

CREATE_PATCHED_CALL(int, sceGxmTextureSetUAddrMode, SceGxmTexture *texture, SceGxmTextureAddrMode addrMode)
{
    return TAI_CONTINUE(int, sceGxmTextureSetUAddrModeRef, texture, addrMode);
}

CREATE_PATCHED_CALL(SceGxmTextureAddrMode, sceGxmTextureGetUAddrMode, const SceGxmTexture *texture)
{
    return TAI_CONTINUE(SceGxmTextureAddrMode, sceGxmTextureGetUAddrModeRef, texture);
}

CREATE_PATCHED_CALL(int, sceGxmTextureSetVAddrMode, SceGxmTexture *texture, SceGxmTextureAddrMode addrMode)
{
    return TAI_CONTINUE(int, sceGxmTextureSetVAddrModeRef, texture, addrMode);
}

CREATE_PATCHED_CALL(SceGxmTextureAddrMode, sceGxmTextureGetVAddrMode, const SceGxmTexture *texture)
{
    return TAI_CONTINUE(SceGxmTextureAddrMode, sceGxmTextureGetVAddrModeRef, texture);
}

CREATE_PATCHED_CALL(int, sceGxmTextureSetFormat, SceGxmTexture *texture, SceGxmTextureFormat texFormat)
{
    return TAI_CONTINUE(int, sceGxmTextureSetFormatRef, texture, texFormat);
}

CREATE_PATCHED_CALL(SceGxmTextureFormat, sceGxmTextureGetFormat, const SceGxmTexture *texture)
{
    return TAI_CONTINUE(SceGxmTextureFormat, sceGxmTextureGetFormatRef, texture);
}

CREATE_PATCHED_CALL(int, sceGxmTextureSetLodBias, SceGxmTexture *texture, unsigned int bias)
{
    return TAI_CONTINUE(int, sceGxmTextureSetLodBiasRef, texture, bias);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmTextureGetLodBias, const SceGxmTexture *texture)
{
    return TAI_CONTINUE(unsigned int, sceGxmTextureGetLodBiasRef, texture);
}

CREATE_PATCHED_CALL(int, sceGxmTextureSetStride, SceGxmTexture *texture, unsigned int byteStride)
{
    return TAI_CONTINUE(int, sceGxmTextureSetStrideRef, texture, byteStride);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmTextureGetStride, const SceGxmTexture *texture)
{
    return TAI_CONTINUE(unsigned int, sceGxmTextureGetStrideRef, texture);
}

CREATE_PATCHED_CALL(int, sceGxmTextureSetWidth, SceGxmTexture *texture, unsigned int width)
{
    return TAI_CONTINUE(int, sceGxmTextureSetWidthRef, texture, width);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmTextureGetWidth, const SceGxmTexture *texture)
{
    return TAI_CONTINUE(unsigned int, sceGxmTextureGetWidthRef, texture);
}

CREATE_PATCHED_CALL(int, sceGxmTextureSetHeight, SceGxmTexture *texture, unsigned int height)
{
    return TAI_CONTINUE(int, sceGxmTextureSetHeightRef, texture, height);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmTextureGetHeight, const SceGxmTexture *texture)
{
    return TAI_CONTINUE(unsigned int, sceGxmTextureGetHeightRef, texture);
}

CREATE_PATCHED_CALL(int, sceGxmTextureSetData, SceGxmTexture *texture, const void *data)
{
    return TAI_CONTINUE(int, sceGxmTextureSetDataRef, texture, data);
}

CREATE_PATCHED_CALL(void *, sceGxmTextureGetData, const SceGxmTexture *texture)
{
    return TAI_CONTINUE(void *, sceGxmTextureGetDataRef, texture);
}

CREATE_PATCHED_CALL(int, sceGxmTextureSetMipmapCount, SceGxmTexture *texture, unsigned int mipCount)
{
    return TAI_CONTINUE(int, sceGxmTextureSetMipmapCountRef, texture, mipCount);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmTextureGetMipmapCount, const SceGxmTexture *texture)
{
    return TAI_CONTINUE(unsigned int, sceGxmTextureGetMipmapCountRef, texture);
}

CREATE_PATCHED_CALL(int, sceGxmTextureSetPalette, SceGxmTexture *texture, const void *paletteData)
{
    return TAI_CONTINUE(int, sceGxmTextureSetPaletteRef, texture, paletteData);
}

CREATE_PATCHED_CALL(void *, sceGxmTextureGetPalette, const SceGxmTexture *texture)
{
    return TAI_CONTINUE(void *, sceGxmTextureGetPaletteRef, texture);
}

CREATE_PATCHED_CALL(SceGxmTextureGammaMode, sceGxmTextureGetGammaMode, const SceGxmTexture *texture)
{
    return TAI_CONTINUE(SceGxmTextureGammaMode, sceGxmTextureGetGammaModeRef, texture);
}

CREATE_PATCHED_CALL(int, sceGxmTextureSetGammaMode, SceGxmTexture *texture, SceGxmTextureGammaMode gammaMode)
{
    return TAI_CONTINUE(int, sceGxmTextureSetGammaModeRef, texture, gammaMode);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmGetPrecomputedVertexStateSize, const SceGxmVertexProgram *vertexProgram)
{
    return TAI_CONTINUE(unsigned int, sceGxmGetPrecomputedVertexStateSizeRef, vertexProgram);
}

CREATE_PATCHED_CALL(int, sceGxmPrecomputedVertexStateInit, SceGxmPrecomputedVertexState *precomputedState, const SceGxmVertexProgram *vertexProgram, void *memBlock)
{
    return TAI_CONTINUE(int, sceGxmPrecomputedVertexStateInitRef, precomputedState, vertexProgram, memBlock);
}

CREATE_PATCHED_CALL(void, sceGxmPrecomputedVertexStateSetDefaultUniformBuffer, SceGxmPrecomputedVertexState *precomputedState, void *defaultBuffer)
{
    return TAI_CONTINUE(void, sceGxmPrecomputedVertexStateSetDefaultUniformBufferRef, precomputedState, defaultBuffer);
}

CREATE_PATCHED_CALL(void *, sceGxmPrecomputedVertexStateGetDefaultUniformBuffer, const SceGxmPrecomputedVertexState *precomputedState)
{
    return TAI_CONTINUE(void *, sceGxmPrecomputedVertexStateGetDefaultUniformBufferRef, precomputedState);
}

CREATE_PATCHED_CALL(int, sceGxmPrecomputedVertexStateSetAllTextures, SceGxmPrecomputedVertexState *precomputedState, const SceGxmTexture *textures)
{
    return TAI_CONTINUE(int, sceGxmPrecomputedVertexStateSetAllTexturesRef, precomputedState, textures);
}

CREATE_PATCHED_CALL(int, sceGxmPrecomputedVertexStateSetTexture, SceGxmPrecomputedVertexState *precomputedState, unsigned int textureIndex, const SceGxmTexture *texture)
{
    return TAI_CONTINUE(int, sceGxmPrecomputedVertexStateSetTextureRef, precomputedState, textureIndex, texture);
}

CREATE_PATCHED_CALL(int, sceGxmPrecomputedVertexStateSetAllUniformBuffers, SceGxmPrecomputedVertexState *precomputedState, const void * const *bufferDataArray)
{
    return TAI_CONTINUE(int, sceGxmPrecomputedVertexStateSetAllUniformBuffersRef, precomputedState, bufferDataArray);
}

CREATE_PATCHED_CALL(int, sceGxmPrecomputedVertexStateSetUniformBuffer, SceGxmPrecomputedVertexState *precomputedState, unsigned int bufferIndex, const void *bufferData)
{
    return TAI_CONTINUE(int, sceGxmPrecomputedVertexStateSetUniformBufferRef, precomputedState, bufferIndex, bufferData);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmGetPrecomputedFragmentStateSize, const SceGxmFragmentProgram *fragmentProgram)
{
    return TAI_CONTINUE(unsigned int, sceGxmGetPrecomputedFragmentStateSizeRef, fragmentProgram);
}

CREATE_PATCHED_CALL(int, sceGxmPrecomputedFragmentStateInit, SceGxmPrecomputedFragmentState *precomputedState, const SceGxmFragmentProgram *fragmentProgram, void *memBlock)
{
    return TAI_CONTINUE(int, sceGxmPrecomputedFragmentStateInitRef, precomputedState, fragmentProgram, memBlock);
}

CREATE_PATCHED_CALL(void, sceGxmPrecomputedFragmentStateSetDefaultUniformBuffer, SceGxmPrecomputedFragmentState *precomputedState, void *defaultBuffer)
{
    TAI_CONTINUE(void, sceGxmPrecomputedFragmentStateSetDefaultUniformBufferRef, precomputedState, defaultBuffer);
}

CREATE_PATCHED_CALL(void *, sceGxmPrecomputedFragmentStateGetDefaultUniformBuffer, const SceGxmPrecomputedFragmentState *precomputedState)
{
    return TAI_CONTINUE(void *, sceGxmPrecomputedFragmentStateGetDefaultUniformBufferRef, precomputedState);
}

CREATE_PATCHED_CALL(int, sceGxmPrecomputedFragmentStateSetAllTextures, SceGxmPrecomputedFragmentState *precomputedState, const SceGxmTexture *textureArray)
{
    return TAI_CONTINUE(int, sceGxmPrecomputedFragmentStateSetAllTexturesRef, precomputedState, textureArray);
}

CREATE_PATCHED_CALL(int, sceGxmPrecomputedFragmentStateSetTexture, SceGxmPrecomputedFragmentState *precomputedState, unsigned int textureIndex, const SceGxmTexture *texture)
{
    return TAI_CONTINUE(int, sceGxmPrecomputedFragmentStateSetTextureRef, precomputedState, textureIndex, texture);
}

CREATE_PATCHED_CALL(int, sceGxmPrecomputedFragmentStateSetAllUniformBuffers, SceGxmPrecomputedFragmentState *precomputedState, const void * const *bufferDataArray)
{
    return TAI_CONTINUE(int, sceGxmPrecomputedFragmentStateSetAllUniformBuffersRef, precomputedState, bufferDataArray);
}

CREATE_PATCHED_CALL(int, sceGxmPrecomputedFragmentStateSetUniformBuffer, SceGxmPrecomputedFragmentState *precomputedState, unsigned int bufferIndex, const void *bufferData)
{
    return TAI_CONTINUE(int, sceGxmPrecomputedFragmentStateSetUniformBufferRef, precomputedState, bufferIndex, bufferData);
}

CREATE_PATCHED_CALL(int, sceGxmPrecomputedFragmentStateSetAllAuxiliarySurfaces, SceGxmPrecomputedFragmentState *precomputedState, const SceGxmAuxiliarySurface *auxSurfaceArray)
{
    return TAI_CONTINUE(int, sceGxmPrecomputedFragmentStateSetAllAuxiliarySurfacesRef, precomputedState, auxSurfaceArray);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmGetPrecomputedDrawSize, const SceGxmVertexProgram *vertexProgram)
{
    return TAI_CONTINUE(unsigned int, sceGxmGetPrecomputedDrawSizeRef, vertexProgram);
}

CREATE_PATCHED_CALL(int, sceGxmPrecomputedDrawInit, SceGxmPrecomputedDraw *precomputedDraw, const SceGxmVertexProgram *vertexProgram, void *memBlock)
{
    return TAI_CONTINUE(int, sceGxmPrecomputedDrawInitRef, precomputedDraw, vertexProgram, memBlock);
}

CREATE_PATCHED_CALL(int, sceGxmPrecomputedDrawSetAllVertexStreams, SceGxmPrecomputedDraw *precomputedDraw, const void * const *streamDataArray)
{
    return TAI_CONTINUE(int, sceGxmPrecomputedDrawSetAllVertexStreamsRef, precomputedDraw, streamDataArray);
}

CREATE_PATCHED_CALL(int, sceGxmPrecomputedDrawSetVertexStream, SceGxmPrecomputedDraw *precomputedDraw, unsigned int streamIndex, const void *streamData)
{
    return TAI_CONTINUE(int, sceGxmPrecomputedDrawSetVertexStreamRef, precomputedDraw, streamIndex, streamData);
}

CREATE_PATCHED_CALL(void, sceGxmPrecomputedDrawSetParams, SceGxmPrecomputedDraw *precomputedDraw, SceGxmPrimitiveType primType, SceGxmIndexFormat indexType, const void *indexData, unsigned int indexCount)
{
    TAI_CONTINUE(void, sceGxmPrecomputedDrawSetParamsRef, precomputedDraw, primType, indexType, indexData, indexCount);
}

CREATE_PATCHED_CALL(void, sceGxmPrecomputedDrawSetParamsInstanced, SceGxmPrecomputedDraw *precomputedDraw, SceGxmPrimitiveType primType, SceGxmIndexFormat indexType, const void *indexData, unsigned int indexCount, unsigned int indexWrap)
{
    TAI_CONTINUE(void, sceGxmPrecomputedDrawSetParamsInstancedRef, precomputedDraw, primType, indexType, indexData, indexCount, indexWrap);
}

CREATE_PATCHED_CALL(int, sceGxmGetRenderTargetMemSizes, const SceGxmRenderTargetParams *params, unsigned int *hostMemSize, unsigned int *driverMemSize)
{
    return TAI_CONTINUE(int, sceGxmGetRenderTargetMemSizesRef, params, hostMemSize, driverMemSize);
}

CREATE_PATCHED_CALL(int, sceGxmRenderTargetGetHostMem, const SceGxmRenderTarget *renderTarget, void **hostMem)
{
    return TAI_CONTINUE(int, sceGxmRenderTargetGetHostMemRef, renderTarget, hostMem);
}

CREATE_PATCHED_CALL(int, sceGxmRenderTargetGetDriverMemBlock, const SceGxmRenderTarget *renderTarget, SceUID *driverMemBlock)
{
    return TAI_CONTINUE(int, sceGxmRenderTargetGetDriverMemBlockRef, renderTarget, driverMemBlock);
}

CREATE_PATCHED_CALL(int, sceGxmDestroyRenderTarget, SceGxmRenderTarget *renderTarget)
{
    return TAI_CONTINUE(int, sceGxmDestroyRenderTargetRef, renderTarget);
}

CREATE_PATCHED_CALL(int, sceGxmSetUniformDataF, void *uniformBuffer, const SceGxmProgramParameter *parameter, unsigned int componentOffset, unsigned int componentCount, const float *sourceData)
{
    return TAI_CONTINUE(int, sceGxmSetUniformDataFRef, uniformBuffer, parameter, componentOffset, componentCount, sourceData);
}

CREATE_PATCHED_CALL(int, sceGxmTransferCopy, uint32_t width, uint32_t height, uint32_t colorKeyValue, uint32_t colorKeyMask, SceGxmTransferColorKeyMode colorKeyMode, SceGxmTransferFormat srcFormat, SceGxmTransferType srcType, const void *srcAddress, uint32_t srcX, uint32_t srcY, int32_t srcStride, SceGxmTransferFormat destFormat, SceGxmTransferType destType, void *destAddress, uint32_t destX, uint32_t destY, int32_t destStride, SceGxmSyncObject *syncObject, uint32_t syncFlags, const SceGxmNotification *notification)
{
    return TAI_CONTINUE(int, sceGxmTransferCopyRef, width, height, colorKeyValue, colorKeyMask, colorKeyMode, srcFormat, srcType, srcAddress, srcX, srcY, srcStride, destFormat, destType, destAddress, destX, destY, destStride, syncObject, syncObject, notification);
}

CREATE_PATCHED_CALL(int, sceGxmTransferDownscale, SceGxmTransferFormat srcFormat, const void *srcAddress, unsigned int srcX, unsigned int srcY, unsigned int srcWidth, unsigned int srcHeight, int srcStride, SceGxmTransferFormat destFormat, void *destAddress, unsigned int destX, unsigned int destY, int destStride, SceGxmSyncObject *syncObject, unsigned int syncFlags, const SceGxmNotification* notification)
{
    return TAI_CONTINUE(int, sceGxmTransferDownscaleRef, srcFormat, srcAddress, srcX, srcY, srcWidth, srcHeight, srcStride, destFormat, destAddress, destX, destY, destStride, syncObject, notification);
}

CREATE_PATCHED_CALL(int, sceGxmTransferFinish)
{
    return TAI_CONTINUE(int, sceGxmTransferFinishRef);
}

int _start() __attribute__ ((weak, alias("module_start")));
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
    //IMPORT_HOOK(sceGxmDestroyContext, 0xEDDC5FB2);
    //IMPORT_HOOK(sceGxmDestroyDeferredContext, 0xD6A2FF2F);
    //IMPORT_HOOK(sceGxmDestroyRenderTarget, 0x0B94C50A);
    //IMPORT_HOOK(sceGxmDisplayQueueAddEntry, 0xEC5C26B5);
    //IMPORT_HOOK(sceGxmDisplayQueueFinish, 0xB98C5B0D);
    //IMPORT_HOOK(sceGxmDraw, 0xBC059AFC);
    //IMPORT_HOOK(sceGxmDrawInstanced, 0x14C4E7D3);
    //IMPORT_HOOK(sceGxmDrawPrecomputed, 0xED3F78B8);
    //IMPORT_HOOK(sceGxmEndCommandList, 0x36D85916);
    //IMPORT_HOOK(sceGxmEndScene, 0xFE300E2F);
    //IMPORT_HOOK(sceGxmExecuteCommandList, 0xE8E139DD);
    //IMPORT_HOOK(sceGxmFinish, 0x0733D8AE);
    //IMPORT_HOOK(sceGxmFragmentProgramGetPassType, 0xCE0B0A76);
    //IMPORT_HOOK(sceGxmFragmentProgramGetProgram, 0xE0E3B3F8);
    //IMPORT_HOOK(sceGxmFragmentProgramIsEnabled, 0x5748367E);
    //IMPORT_HOOK(sceGxmGetContextType, 0x2A1BCDDB);
    //IMPORT_HOOK(sceGxmGetDeferredContextFragmentBuffer, 0xFA081D05);
    //IMPORT_HOOK(sceGxmGetDeferredContextVdmBuffer, 0xAD8C2EBA);
    //IMPORT_HOOK(sceGxmGetDeferredContextVertexBuffer, 0x87632B9C);
    //IMPORT_HOOK(sceGxmGetNotificationRegion, 0x8BDE825A);
    //IMPORT_HOOK(sceGxmGetParameterBufferThreshold, 0x6ABF3F76);
    //IMPORT_HOOK(sceGxmGetPrecomputedDrawSize, 0x41BBD792);
    //IMPORT_HOOK(sceGxmGetPrecomputedFragmentStateSize, 0x85DE8506);
    //IMPORT_HOOK(sceGxmGetPrecomputedVertexStateSize, 0x9D83CA3B);
    //IMPORT_HOOK(sceGxmGetRenderTargetMemSizes, 0xB291C959);
    //IMPORT_HOOK(sceGxmInitialize, 0xB0F1E4EC);
    //IMPORT_HOOK(sceGxmIsDebugVersion, 0x873B07C0);
    //IMPORT_HOOK(sceGxmMapFragmentUsseMemory, 0x008402C6);
    //IMPORT_HOOK(sceGxmMapMemory, 0xC61E34FC);
    //IMPORT_HOOK(sceGxmMapVertexUsseMemory, 0xFA437510);
    //IMPORT_HOOK(sceGxmMidSceneFlush, 0x2B5C0444);
    //IMPORT_HOOK(sceGxmNotificationWait, 0x9F448E79);
    //IMPORT_HOOK(sceGxmPadHeartbeat, 0x3D25FCE9);
    //IMPORT_HOOK(sceGxmPadTriggerGpuPaTrace, 0x47E06984);
    //IMPORT_HOOK(sceGxmPopUserMarker, 0x4FA073A6);
    //IMPORT_HOOK(sceGxmPrecomputedDrawInit, 0xA197F096);
    //IMPORT_HOOK(sceGxmPrecomputedDrawSetAllVertexStreams, 0xB6C6F571);
    //IMPORT_HOOK(sceGxmPrecomputedDrawSetParams, 0x884D0D08);
    //IMPORT_HOOK(sceGxmPrecomputedDrawSetParamsInstanced, 0x3A7B1633);
    //IMPORT_HOOK(sceGxmPrecomputedDrawSetVertexStream, 0x6C936214);
    //IMPORT_HOOK(sceGxmPrecomputedFragmentStateGetDefaultUniformBuffer, 0xCECB584A);
    //IMPORT_HOOK(sceGxmPrecomputedFragmentStateInit, 0xE297D7AF);
    //IMPORT_HOOK(sceGxmPrecomputedFragmentStateSetAllAuxiliarySurfaces, 0x9D93B63A);
    //IMPORT_HOOK(sceGxmPrecomputedFragmentStateSetAllTextures, 0xC383DE39);
    //IMPORT_HOOK(sceGxmPrecomputedFragmentStateSetAllUniformBuffers, 0x5A783DC3);
    //IMPORT_HOOK(sceGxmPrecomputedFragmentStateSetDefaultUniformBuffer, 0x91236858);
    //IMPORT_HOOK(sceGxmPrecomputedFragmentStateSetTexture, 0x29118BF1);
    //IMPORT_HOOK(sceGxmPrecomputedFragmentStateSetUniformBuffer, 0xB452F1FB);
    //IMPORT_HOOK(sceGxmPrecomputedVertexStateGetDefaultUniformBuffer, 0xBE5A68EF);
    //IMPORT_HOOK(sceGxmPrecomputedVertexStateInit, 0xBE937F8D);
    //IMPORT_HOOK(sceGxmPrecomputedVertexStateSetAllTextures, 0xC40C9127);
    //IMPORT_HOOK(sceGxmPrecomputedVertexStateSetAllUniformBuffers, 0x0389861D);
    //IMPORT_HOOK(sceGxmPrecomputedVertexStateSetDefaultUniformBuffer, 0x34BF64E3);
    //IMPORT_HOOK(sceGxmPrecomputedVertexStateSetTexture, 0x6A29EB06);
    //IMPORT_HOOK(sceGxmPrecomputedVertexStateSetUniformBuffer, 0xDBF97ED6);
    //IMPORT_HOOK(sceGxmProgramCheck, 0xED8B6C69);
    //IMPORT_HOOK(sceGxmProgramFindParameterByName, 0x277794C4);
    //IMPORT_HOOK(sceGxmProgramFindParameterBySemantic, 0x633CAA54);
    //IMPORT_HOOK(sceGxmProgramGetDefaultUniformBufferSize, 0x8FA3F9C3);
    //IMPORT_HOOK(sceGxmProgramGetFragmentProgramInputs, 0xC6924709);
    //IMPORT_HOOK(sceGxmProgramGetOutputRegisterFormat, 0xE11603B1);
    //IMPORT_HOOK(sceGxmProgramGetParameter, 0x06FF9151);
    //IMPORT_HOOK(sceGxmProgramGetParameterCount, 0xD5D5FCCD);
    //IMPORT_HOOK(sceGxmProgramGetSize, 0xBF5E2090);
    //IMPORT_HOOK(sceGxmProgramGetType, 0x04BB3C59);
    //IMPORT_HOOK(sceGxmProgramGetVertexProgramOutputs, 0xFB01C7E5);
    //IMPORT_HOOK(sceGxmProgramIsDepthReplaceUsed, 0x89613EF2);
    //IMPORT_HOOK(sceGxmProgramIsDiscardUsed, 0x029B4F1C);
    //IMPORT_HOOK(sceGxmProgramIsEquivalent, 0xE23C838C);
    //IMPORT_HOOK(sceGxmProgramIsFragColorUsed, 0x104F23F4);
    //IMPORT_HOOK(sceGxmProgramIsNativeColorUsed, 0xA4297E57);
    //IMPORT_HOOK(sceGxmProgramIsSpriteCoordUsed, 0xE63C53D8);
    //IMPORT_HOOK(sceGxmProgramParameterGetArraySize, 0xDBA8D061);
    //IMPORT_HOOK(sceGxmProgramParameterGetCategory, 0x1997DC17);
    //IMPORT_HOOK(sceGxmProgramParameterGetComponentCount, 0xBD2998D1);
    //IMPORT_HOOK(sceGxmProgramParameterGetContainerIndex, 0xBB58267D);
    //IMPORT_HOOK(sceGxmProgramParameterGetIndex, 0x6E61DDF5);
    //IMPORT_HOOK(sceGxmProgramParameterGetName, 0x6AF88A5D);
    //IMPORT_HOOK(sceGxmProgramParameterGetResourceIndex, 0x5C79D59A);
    //IMPORT_HOOK(sceGxmProgramParameterGetSemantic, 0xE6D9C4CE);
    //IMPORT_HOOK(sceGxmProgramParameterGetSemanticIndex, 0xB85CC13E);
    //IMPORT_HOOK(sceGxmProgramParameterGetType, 0x7B9023C3);
    //IMPORT_HOOK(sceGxmProgramParameterIsRegFormat, 0x871E5009);
    //IMPORT_HOOK(sceGxmProgramParameterIsSamplerCube, 0xF7AA978B);
    //IMPORT_HOOK(sceGxmPushUserMarker, 0x3276C475);
    //IMPORT_HOOK(sceGxmRemoveRazorGpuCaptureBuffer, 0x0468E3F2);
    //IMPORT_HOOK(sceGxmRenderTargetGetDriverMemBlock, 0x49553737);
    //IMPORT_HOOK(sceGxmRenderTargetGetHostMem, 0xD0EDAB4C);
    //IMPORT_HOOK(sceGxmReserveFragmentDefaultUniformBuffer, 0x7B1FABB6);
    //IMPORT_HOOK(sceGxmReserveVertexDefaultUniformBuffer, 0x97118913);
    //IMPORT_HOOK(sceGxmSetAuxiliarySurface, 0x91B4F7F4);
    //IMPORT_HOOK(sceGxmSetBackDepthBias, 0x17B3BF86);
    //IMPORT_HOOK(sceGxmSetBackDepthFunc, 0xB042A4D2);
    //IMPORT_HOOK(sceGxmSetBackDepthWriteEnable, 0xC18B706B);
    //IMPORT_HOOK(sceGxmSetBackFragmentProgramEnable, 0xE26B4834);
    //IMPORT_HOOK(sceGxmSetBackLineFillLastPixelEnable, 0xC88EB702);
    //IMPORT_HOOK(sceGxmSetBackPointLineWidth, 0x8DCB0EDB);
    //IMPORT_HOOK(sceGxmSetBackPolygonMode, 0xF66EC6FE);
    //IMPORT_HOOK(sceGxmSetBackStencilFunc, 0x1A68C8D2);
    //IMPORT_HOOK(sceGxmSetBackStencilRef, 0x866A0517);
    //IMPORT_HOOK(sceGxmSetBackVisibilityTestEnable, 0x17CF46B9);
    //IMPORT_HOOK(sceGxmSetBackVisibilityTestIndex, 0xAE7886FE);
    //IMPORT_HOOK(sceGxmSetBackVisibilityTestOp, 0xC83F0AB3);
    //IMPORT_HOOK(sceGxmSetCullMode, 0xE1CA72AE);
    //IMPORT_HOOK(sceGxmSetDefaultRegionClipAndViewport, 0x60CF708E);
    //IMPORT_HOOK(sceGxmSetDeferredContextFragmentBuffer, 0xC6B3FCD0);
    //IMPORT_HOOK(sceGxmSetDeferredContextVdmBuffer, 0x0B9D13CE);
    //IMPORT_HOOK(sceGxmSetDeferredContextVertexBuffer, 0x27CAD127);
    //IMPORT_HOOK(sceGxmSetFragmentDefaultUniformBuffer, 0xA824EB24);
    //IMPORT_HOOK(sceGxmSetFragmentProgram, 0xAD2F48D9);
    //IMPORT_HOOK(sceGxmSetFragmentTexture, 0x29C34DF5);
    //IMPORT_HOOK(sceGxmSetFragmentUniformBuffer, 0xEA0FC310);
    //IMPORT_HOOK(sceGxmSetFrontDepthBias, 0xAAA97F81);
    //IMPORT_HOOK(sceGxmSetFrontDepthFunc, 0x14BD831F);
    //IMPORT_HOOK(sceGxmSetFrontDepthWriteEnable, 0xF32CBF34);
    //IMPORT_HOOK(sceGxmSetFrontFragmentProgramEnable, 0x575958A8);
    //IMPORT_HOOK(sceGxmSetFrontLineFillLastPixelEnable, 0x5765DE9F);
    //IMPORT_HOOK(sceGxmSetFrontPointLineWidth, 0x06752183);
    //IMPORT_HOOK(sceGxmSetFrontPolygonMode, 0xFD93209D);
    //IMPORT_HOOK(sceGxmSetFrontStencilFunc, 0xB8645A9A);
    //IMPORT_HOOK(sceGxmSetFrontStencilRef, 0x8FA6FE44);
    //IMPORT_HOOK(sceGxmSetFrontVisibilityTestEnable, 0x30459117);
    //IMPORT_HOOK(sceGxmSetFrontVisibilityTestIndex, 0x12625C34);
    //IMPORT_HOOK(sceGxmSetFrontVisibilityTestOp, 0xD0E3CD9A);
    //IMPORT_HOOK(sceGxmSetPrecomputedFragmentState, 0xF8952750);
    //IMPORT_HOOK(sceGxmSetPrecomputedVertexState, 0xB7626A93);
    //IMPORT_HOOK(sceGxmSetRegionClip, 0x70C86868);
    //IMPORT_HOOK(sceGxmSetTwoSidedEnable, 0x0DE9AEB7);
    //IMPORT_HOOK(sceGxmSetUniformDataF, 0x65DD0C84);
    //IMPORT_HOOK(sceGxmSetUserMarker, 0xC7A8CB77);
    //IMPORT_HOOK(sceGxmSetValidationEnable, 0x8C6A24C9);
    //IMPORT_HOOK(sceGxmSetVertexDefaultUniformBuffer, 0xC697CAE5);
    //IMPORT_HOOK(sceGxmSetVertexProgram, 0x31FF8ABD);
    //IMPORT_HOOK(sceGxmSetVertexStream, 0x895DF2E9);
    //IMPORT_HOOK(sceGxmSetVertexTexture, 0x9EB4380F);
    //IMPORT_HOOK(sceGxmSetVertexUniformBuffer, 0xC68015E4);
    //IMPORT_HOOK(sceGxmSetViewport, 0x3EB3380B);
    //IMPORT_HOOK(sceGxmSetViewportEnable, 0x814F61EB);
    //IMPORT_HOOK(sceGxmSetVisibilityBuffer, 0x7767EC49);
    //IMPORT_HOOK(sceGxmSetWBufferEnable, 0xEED86975);
    //IMPORT_HOOK(sceGxmSetWClampEnable, 0x1BF8B853);
    //IMPORT_HOOK(sceGxmSetWClampValue, 0xD096336E);
    //IMPORT_HOOK(sceGxmSetWarningEnabled, 0x12D18B3D);
    //IMPORT_HOOK(sceGxmSetYuvProfile, 0xB32917F0);
    //IMPORT_HOOK(sceGxmShaderPatcherAddRefFragmentProgram, 0x4CD2D19F);
    //IMPORT_HOOK(sceGxmShaderPatcherAddRefVertexProgram, 0x0FD1E589);
    //IMPORT_HOOK(sceGxmShaderPatcherCreate, 0x05032658);
    //IMPORT_HOOK(sceGxmShaderPatcherCreateFragmentProgram, 0x4ED2E49D);
    //IMPORT_HOOK(sceGxmShaderPatcherCreateMaskUpdateFragmentProgram, 0xA4433427);
    //IMPORT_HOOK(sceGxmShaderPatcherCreateVertexProgram, 0xB7BBA6D5);
    //IMPORT_HOOK(sceGxmShaderPatcherDestroy, 0xEAA5B100);
    //IMPORT_HOOK(sceGxmShaderPatcherForceUnregisterProgram, 0x630D4B2E);
    //IMPORT_HOOK(sceGxmShaderPatcherGetBufferMemAllocated, 0xC694D039);
    //IMPORT_HOOK(sceGxmShaderPatcherGetFragmentProgramRefCount, 0x2C5550F0);
    //IMPORT_HOOK(sceGxmShaderPatcherGetFragmentUsseMemAllocated, 0x3C9DDB4A);
    //IMPORT_HOOK(sceGxmShaderPatcherGetHostMemAllocated, 0x9DBBC71C);
    //IMPORT_HOOK(sceGxmShaderPatcherGetProgramFromId, 0xA949A803);
    //IMPORT_HOOK(sceGxmShaderPatcherGetUserData, 0x96A7E6DD);
    //IMPORT_HOOK(sceGxmShaderPatcherGetVertexProgramRefCount, 0xA1A16FF6);
    //IMPORT_HOOK(sceGxmShaderPatcherGetVertexUsseMemAllocated, 0x7D2F83C1);
    //IMPORT_HOOK(sceGxmShaderPatcherRegisterProgram, 0x2B528462);
    //IMPORT_HOOK(sceGxmShaderPatcherReleaseFragmentProgram, 0xBE2743D1);
    //IMPORT_HOOK(sceGxmShaderPatcherReleaseVertexProgram, 0xAC1FF2DA);
    //IMPORT_HOOK(sceGxmShaderPatcherSetAuxiliarySurface, 0x8E5FCC2A);
    //IMPORT_HOOK(sceGxmShaderPatcherSetUserData, 0xF9B8FCFD);
    //IMPORT_HOOK(sceGxmShaderPatcherUnregisterProgram, 0xF103AF8A);
    //IMPORT_HOOK(sceGxmSyncObjectCreate, 0x6A6013E1);
    //IMPORT_HOOK(sceGxmSyncObjectDestroy, 0x889AE88C);
    //IMPORT_HOOK(sceGxmTerminate, 0xB627DE66);
    //IMPORT_HOOK(sceGxmTextureGetData, 0x5341BD46);
    //IMPORT_HOOK(sceGxmTextureGetFormat, 0xE868D2B3);
    //IMPORT_HOOK(sceGxmTextureGetGammaMode, 0xF23FCE81);
    //IMPORT_HOOK(sceGxmTextureGetHeight, 0x5420A086);
    //IMPORT_HOOK(sceGxmTextureGetLodBias, 0x2DE55DA5);
    //IMPORT_HOOK(sceGxmTextureGetLodMin, 0xBE524A2C);
    //IMPORT_HOOK(sceGxmTextureGetMagFilter, 0xAE7FBB51);
    //IMPORT_HOOK(sceGxmTextureGetMinFilter, 0x920666C6);
    //IMPORT_HOOK(sceGxmTextureGetMipFilter, 0xCE94CA15);
    //IMPORT_HOOK(sceGxmTextureGetMipmapCount, 0xF7B7B1E4);
    //IMPORT_HOOK(sceGxmTextureGetMipmapCountUnsafe, 0x4CC42929);
    //IMPORT_HOOK(sceGxmTextureGetNormalizeMode, 0x512BB86C);
    //IMPORT_HOOK(sceGxmTextureGetPalette, 0x0D189C30);
    //IMPORT_HOOK(sceGxmTextureGetStride, 0xB0BD52F3);
    //IMPORT_HOOK(sceGxmTextureGetType, 0xF65D4917);
    //IMPORT_HOOK(sceGxmTextureGetUAddrMode, 0x2AE22788);
    //IMPORT_HOOK(sceGxmTextureGetUAddrModeSafe, 0xC037DA83);
    //IMPORT_HOOK(sceGxmTextureGetVAddrMode, 0x46136CA9);
    //IMPORT_HOOK(sceGxmTextureGetVAddrModeSafe, 0xD2F0D9C1);
    //IMPORT_HOOK(sceGxmTextureGetWidth, 0x126A3EB3);
    //IMPORT_HOOK(sceGxmTextureInitCube, 0x11DC8DC9);
    //IMPORT_HOOK(sceGxmTextureInitCubeArbitrary, 0xE3DF5E3B);
    //IMPORT_HOOK(sceGxmTextureInitLinear, 0x4811AECB);
    //IMPORT_HOOK(sceGxmTextureInitLinearStrided, 0x6679BEF0);
    //IMPORT_HOOK(sceGxmTextureInitSwizzled, 0xD572D547);
    //IMPORT_HOOK(sceGxmTextureInitSwizzledArbitrary, 0x5DBFBA2C);
    //IMPORT_HOOK(sceGxmTextureInitTiled, 0xE6F0DB27);
    //IMPORT_HOOK(sceGxmTextureSetData, 0x855814C4);
    //IMPORT_HOOK(sceGxmTextureSetFormat, 0xFC943596);
    //IMPORT_HOOK(sceGxmTextureSetGammaMode, 0xA6D9F4DA);
    //IMPORT_HOOK(sceGxmTextureSetHeight, 0xAEE7FDD1);
    //IMPORT_HOOK(sceGxmTextureSetLodBias, 0xB65EE6F7);
    //IMPORT_HOOK(sceGxmTextureSetLodMin, 0xB79E43DD);
    //IMPORT_HOOK(sceGxmTextureSetMagFilter, 0xFA695FD7);
    //IMPORT_HOOK(sceGxmTextureSetMinFilter, 0x416764E3);
    //IMPORT_HOOK(sceGxmTextureSetMipFilter, 0x1CA9FE0B);
    //IMPORT_HOOK(sceGxmTextureSetMipmapCount, 0xD2DC4643);
    //IMPORT_HOOK(sceGxmTextureSetNormalizeMode, 0xCE8DDAD0);
    //IMPORT_HOOK(sceGxmTextureSetPalette, 0xDD6AABFA);
    //IMPORT_HOOK(sceGxmTextureSetStride, 0x58D0EB0A);
    //IMPORT_HOOK(sceGxmTextureSetUAddrMode, 0x4281763E);
    //IMPORT_HOOK(sceGxmTextureSetUAddrModeSafe, 0x8699ECF4);
    //IMPORT_HOOK(sceGxmTextureSetVAddrMode, 0x126CDAA3);
    //IMPORT_HOOK(sceGxmTextureSetVAddrModeSafe, 0xFA22F6CC);
    //IMPORT_HOOK(sceGxmTextureSetWidth, 0x2EA178BE);
    //IMPORT_HOOK(sceGxmTextureValidate, 0x5331BED3);
    //IMPORT_HOOK(sceGxmTransferCopy, 0x62312BF8);
    //IMPORT_HOOK(sceGxmTransferDownscale, 0xD10F7EAD);
    //IMPORT_HOOK(sceGxmTransferFill, 0x69DDFF5E);
    //IMPORT_HOOK(sceGxmTransferFinish, 0x45229C39);
    //IMPORT_HOOK(sceGxmUnmapFragmentUsseMemory, 0x80CCEDBB);
    //IMPORT_HOOK(sceGxmUnmapMemory, 0x828C68E8);
    //IMPORT_HOOK(sceGxmUnmapVertexUsseMemory, 0x099134F5);
    //IMPORT_HOOK(sceGxmVertexFence, 0xE05277D6);
    //IMPORT_HOOK(sceGxmVertexProgramGetProgram, 0xBC52320E);
    //IMPORT_HOOK(sceGxmWaitEvent, 0x8BD94593);
    IMPORT_HOOK(sceGxmDraw, 0xBC059AFC);
    IMPORT_HOOK(sceGxmInitialize, 0xB0F1E4EC);
    IMPORT_HOOK(sceGxmTerminate, 0xB627DE66);
    IMPORT_HOOK(sceGxmGetNotificationRegion, 0x8BDE825A);

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize args, void *argp) {
    LOG("end gxm_inject\n");
    return SCE_KERNEL_STOP_SUCCESS;
}