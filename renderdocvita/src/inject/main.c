#include <vitasdk.h>
#include <taihen.h>

#include "../logging.h"

#define CREATE_PATCHED_CALL(return_type, name, ...)  \
static tai_hook_ref_t name##Ref;                     \
static SceUID name##Hook;                            \
static return_type name##Patched(__VA_ARGS__)

#define IMPORT_HOOK(name, nid) \
    name##Hook = taiHookFunctionImport(&name##Ref, TAI_MAIN_MODULE, 0xF76B66BD, nid, name##Patched)

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

CREATE_PATCHED_CALL(int, sceGxmCreateContext, const SceGxmContextParams *params, SceGxmContext **context)
{
    return TAI_CONTINUE(int, sceGxmCreateContextRef, params, context);
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

CREATE_PATCHED_CALL(int, sceGxmBeginScene, SceGxmContext *context, unsigned int flags, const SceGxmRenderTarget *renderTarget, const SceGxmValidRegion *validRegion, SceGxmSyncObject *vertexSyncObject, SceGxmSyncObject *fragmentSyncObject, const SceGxmColorSurface *colorSurface, const SceGxmDepthStencilSurface *depthStencil)
{
    sceClibPrintf("sceGxmBeginScene(context: %p, flags: %d, rendertarget: %p, validRegion: %p, vertexSyncObject: %p, fragmentSyncObject: %p, colorSurface: %p, depthStencil: %p)\n", context, flags, renderTarget, validRegion, vertexSyncObject, fragmentSyncObject, colorSurface, depthStencil);
    return TAI_CONTINUE(int, sceGxmBeginSceneRef, context, flags, renderTarget, validRegion, vertexSyncObject, fragmentSyncObject, colorSurface, depthStencil);
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

CREATE_PATCHED_CALL(int, sceGxmColorSurfaceInit, SceGxmColorSurface *surface, SceGxmColorFormat colorFormat, SceGxmColorSurfaceType surfaceType, SceGxmColorSurfaceScaleMode scaleMode, SceGxmOutputRegisterSize outputRegisterSize, unsigned int width, unsigned int height, unsigned int strideInPixels, void *data)
{
    return TAI_CONTINUE(int, sceGxmColorSurfaceInitRef, surface, colorFormat, surfaceType, scaleMode, outputRegisterSize, width, height, strideInPixels, data);
}

CREATE_PATCHED_CALL(int, sceGxmColorSurfaceInitDisabled, SceGxmColorSurface *surface)
{
    return TAI_CONTINUE(int, sceGxmColorSurfaceInitDisabledRef, surface);
}

CREATE_PATCHED_CALL(SceBool, sceGxmColorSurfaceIsEnabled, const SceGxmColorSurface *surface)
{
    return TAI_CONTINUE(SceBool, sceGxmColorSurfaceIsEnabledRef, surface);
}

CREATE_PATCHED_CALL(void, sceGxmColorSurfaceGetClip, const SceGxmColorSurface *surface, unsigned int *xMin, unsigned int *yMin, unsigned int *xMax, unsigned int *yMax)
{
    TAI_CONTINUE(void, sceGxmColorSurfaceGetClipRef, surface, xMin, yMin, xMax, yMax);
}

CREATE_PATCHED_CALL(void, sceGxmColorSurfaceSetClip, SceGxmColorSurface *surface, unsigned int xMin, unsigned int yMin, unsigned int xMax, unsigned int yMax)
{
    return TAI_CONTINUE(void, sceGxmColorSurfaceSetClipRef, surface, xMin, yMin, xMax, yMax);
}

CREATE_PATCHED_CALL(SceGxmColorSurfaceScaleMode, sceGxmColorSurfaceGetScaleMode, const SceGxmColorSurface *surface)
{
    return TAI_CONTINUE(SceGxmColorSurfaceScaleMode, sceGxmColorSurfaceGetScaleModeRef, surface);
}

CREATE_PATCHED_CALL(void, sceGxmColorSurfaceSetScaleMode, SceGxmColorSurface *surface, SceGxmColorSurfaceScaleMode scaleMode)
{
    TAI_CONTINUE(void, sceGxmColorSurfaceSetScaleModeRef, surface, scaleMode);
}

CREATE_PATCHED_CALL(void *, sceGxmColorSurfaceGetData, const SceGxmColorSurface *surface)
{
    return TAI_CONTINUE(void *, sceGxmColorSurfaceGetDataRef, surface);
}

CREATE_PATCHED_CALL(int, sceGxmColorSurfaceSetData, SceGxmColorSurface *surface, void *data)
{
    return TAI_CONTINUE(int, sceGxmColorSurfaceSetDataRef, surface, data);
}

CREATE_PATCHED_CALL(SceGxmColorFormat, sceGxmColorSurfaceGetFormat, const SceGxmColorSurface *surface)
{
    return TAI_CONTINUE(SceGxmColorFormat, sceGxmColorSurfaceGetFormatRef, surface);
}

CREATE_PATCHED_CALL(int, sceGxmColorSurfaceSetFormat, SceGxmColorSurface *surface, SceGxmColorFormat format)
{
    return TAI_CONTINUE(int, sceGxmColorSurfaceSetFormatRef, surface, format);
}

CREATE_PATCHED_CALL(SceGxmColorSurfaceType, sceGxmColorSurfaceGetType, const SceGxmColorSurface *surface)
{
    return TAI_CONTINUE(SceGxmColorSurfaceType, sceGxmColorSurfaceGetTypeRef, surface);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmColorSurfaceGetStrideInPixels, const SceGxmColorSurface *surface)
{
    return TAI_CONTINUE(unsigned int, sceGxmColorSurfaceGetStrideInPixelsRef, surface);
}

CREATE_PATCHED_CALL(int, sceGxmDepthStencilSurfaceInit, SceGxmDepthStencilSurface *surface, SceGxmDepthStencilFormat depthStencilFormat, SceGxmDepthStencilSurfaceType surfaceType, unsigned int strideInSamples, void *depthData, void *stencilData)
{
    return TAI_CONTINUE(int, sceGxmDepthStencilSurfaceInitRef, surface, depthStencilFormat, surfaceType, strideInSamples, depthData, stencilData);
}

CREATE_PATCHED_CALL(int, sceGxmDepthStencilSurfaceInitDisabled, SceGxmDepthStencilSurface *surface)
{
    return TAI_CONTINUE(int, sceGxmDepthStencilSurfaceInitDisabledRef, surface);
};

CREATE_PATCHED_CALL(float, sceGxmDepthStencilSurfaceGetBackgroundDepth, const SceGxmDepthStencilSurface *surface)
{
    return TAI_CONTINUE(float, sceGxmDepthStencilSurfaceGetBackgroundDepthRef, surface);
}

CREATE_PATCHED_CALL(void, sceGxmDepthStencilSurfaceSetBackgroundDepth, SceGxmDepthStencilSurface *surface, float backgroundDepth)
{
    return TAI_CONTINUE(void, sceGxmDepthStencilSurfaceSetBackgroundDepthRef, surface, backgroundDepth);
}

CREATE_PATCHED_CALL(unsigned char, sceGxmDepthStencilSurfaceGetBackgroundStencil, const SceGxmDepthStencilSurface *surface)
{
    return TAI_CONTINUE(unsigned char, sceGxmDepthStencilSurfaceGetBackgroundStencilRef, surface);
}

CREATE_PATCHED_CALL(void, sceGxmDepthStencilSurfaceSetBackgroundStencil, SceGxmDepthStencilSurface *surface, unsigned char backgroundStencil)
{
    TAI_CONTINUE(void, sceGxmDepthStencilSurfaceSetBackgroundStencilRef, surface, backgroundStencil);
}

CREATE_PATCHED_CALL(SceBool, sceGxmDepthStencilSurfaceIsEnabled, const SceGxmDepthStencilSurface *surface)
{
    return TAI_CONTINUE(SceBool, sceGxmDepthStencilSurfaceIsEnabledRef, surface);
}

CREATE_PATCHED_CALL(void, sceGxmDepthStencilSurfaceSetForceLoadMode, SceGxmDepthStencilSurface *surface, SceGxmDepthStencilForceLoadMode forceLoad)
{
    TAI_CONTINUE(void, sceGxmDepthStencilSurfaceSetForceLoadModeRef, surface, forceLoad);
}

CREATE_PATCHED_CALL(SceGxmDepthStencilForceLoadMode, sceGxmDepthStencilSurfaceGetForceLoadMode, const SceGxmDepthStencilSurface *surface)
{
    return TAI_CONTINUE(SceGxmDepthStencilForceLoadMode, sceGxmDepthStencilSurfaceGetForceLoadModeRef, surface);
}

CREATE_PATCHED_CALL(void, sceGxmDepthStencilSurfaceSetForceStoreMode, SceGxmDepthStencilSurface *surface, SceGxmDepthStencilForceStoreMode forceStore)
{
    TAI_CONTINUE(void, sceGxmDepthStencilSurfaceSetForceStoreModeRef, surface, forceStore);
}

CREATE_PATCHED_CALL(SceGxmDepthStencilForceStoreMode, sceGxmDepthStencilSurfaceGetForceStoreMode, const SceGxmDepthStencilSurface *surface)
{
    return TAI_CONTINUE(SceGxmDepthStencilForceStoreMode, sceGxmDepthStencilSurfaceGetForceStoreModeRef, surface);
}

CREATE_PATCHED_CALL(SceGxmColorSurfaceGammaMode, sceGxmColorSurfaceGetGammaMode, const SceGxmColorSurface *surface)
{
    return TAI_CONTINUE(SceGxmColorSurfaceGammaMode, sceGxmColorSurfaceGetGammaModeRef, surface);
}

CREATE_PATCHED_CALL(int, sceGxmColorSurfaceSetGammaMode, SceGxmColorSurface *surface, SceGxmColorSurfaceGammaMode gammaMode)
{
    return TAI_CONTINUE(int, sceGxmColorSurfaceSetGammaModeRef, surface, gammaMode);
}

CREATE_PATCHED_CALL(SceGxmColorSurfaceDitherMode, sceGxmColorSurfaceGetDitherMode, const SceGxmColorSurface *surface)
{
    return TAI_CONTINUE(SceGxmColorSurfaceDitherMode, sceGxmColorSurfaceGetDitherModeRef, surface);
}

CREATE_PATCHED_CALL(int, sceGxmColorSurfaceSetDitherMode, SceGxmColorSurface *surface, SceGxmColorSurfaceDitherMode ditherMode)
{
    return TAI_CONTINUE(int, sceGxmColorSurfaceSetDitherModeRef, surface, ditherMode);
}

CREATE_PATCHED_CALL(SceGxmDepthStencilFormat, sceGxmDepthStencilSurfaceGetFormat, const SceGxmDepthStencilSurface *surface)
{
    return TAI_CONTINUE(SceGxmDepthStencilFormat, sceGxmDepthStencilSurfaceGetFormatRef, surface);
}

CREATE_PATCHED_CALL(unsigned int, sceGxmDepthStencilSurfaceGetStrideInSamples, const SceGxmDepthStencilSurface *surface)
{
    return TAI_CONTINUE(unsigned int, sceGxmDepthStencilSurfaceGetStrideInSamplesRef, surface);
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

CREATE_PATCHED_CALL(int, sceGxmCreateRenderTarget, const SceGxmRenderTargetParams *params, SceGxmRenderTarget **renderTarget)
{
    return TAI_CONTINUE(int, sceGxmCreateRenderTargetRef, params, renderTarget);
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

    IMPORT_HOOK(sceGxmDraw, 0xBC059AFC);
    IMPORT_HOOK(sceGxmBeginScene, 0x8734FF4E);
    IMPORT_HOOK(sceGxmInitialize, 0xB0F1E4EC);
    IMPORT_HOOK(sceGxmTerminate, 0xB627DE66);
    IMPORT_HOOK(sceGxmGetNotificationRegion, 0x8BDE825A);

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize args, void *argp) {
    LOG("end gxm_inject\n");
    return SCE_KERNEL_STOP_SUCCESS;
}