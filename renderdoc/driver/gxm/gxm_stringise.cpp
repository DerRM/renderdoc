#include "gxm_common.h"
#include "gxm_driver.h"

#define ForEachSupported(FUNC) \
  FUNC(sceGxmAddRazorGpuCaptureBuffer, sceGxmAddRazorGpuCaptureBuffer) \
  FUNC(sceGxmBeginCommandList, sceGxmBeginCommandList) \
  FUNC(sceGxmBeginScene, sceGxmBeginScene) \
  FUNC(sceGxmBeginSceneEx, sceGxmBeginSceneEx) \
  FUNC(sceGxmColorSurfaceGetClip, sceGxmColorSurfaceGetClip) \
  FUNC(sceGxmColorSurfaceGetData, sceGxmColorSurfaceGetData) \
  FUNC(sceGxmColorSurfaceGetDitherMode, sceGxmColorSurfaceGetDitherMode) \
  FUNC(sceGxmColorSurfaceGetFormat, sceGxmColorSurfaceGetFormat) \
  FUNC(sceGxmColorSurfaceGetGammaMode, sceGxmColorSurfaceGetGammaMode) \
  FUNC(sceGxmColorSurfaceGetScaleMode, sceGxmColorSurfaceGetScaleMode) \
  FUNC(sceGxmColorSurfaceGetStrideInPixels, sceGxmColorSurfaceGetStrideInPixels) \
  FUNC(sceGxmColorSurfaceGetType, sceGxmColorSurfaceGetType) \
  FUNC(sceGxmColorSurfaceInit, sceGxmColorSurfaceInit) \
  FUNC(sceGxmColorSurfaceInitDisabled, sceGxmColorSurfaceInitDisabled) \
  FUNC(sceGxmColorSurfaceIsEnabled, sceGxmColorSurfaceIsEnabled) \
  FUNC(sceGxmColorSurfaceSetClip, sceGxmColorSurfaceSetClip) \
  FUNC(sceGxmColorSurfaceSetData, sceGxmColorSurfaceSetData) \
  FUNC(sceGxmColorSurfaceSetDitherMode, sceGxmColorSurfaceSetDitherMode) \
  FUNC(sceGxmColorSurfaceSetFormat, sceGxmColorSurfaceSetFormat) \
  FUNC(sceGxmColorSurfaceSetGammaMode, sceGxmColorSurfaceSetGammaMode) \
  FUNC(sceGxmColorSurfaceSetScaleMode, sceGxmColorSurfaceSetScaleMode) \
  FUNC(sceGxmCreateContext, sceGxmCreateContext) \
  FUNC(sceGxmCreateDeferredContext, sceGxmCreateDeferredContext) \
  FUNC(sceGxmCreateRenderTarget, sceGxmCreateRenderTarget) \
  FUNC(sceGxmDepthStencilSurfaceGetBackgroundDepth, sceGxmDepthStencilSurfaceGetBackgroundDepth) \
  FUNC(sceGxmDepthStencilSurfaceGetBackgroundMask, sceGxmDepthStencilSurfaceGetBackgroundMask) \
  FUNC(sceGxmDepthStencilSurfaceGetBackgroundStencil, sceGxmDepthStencilSurfaceGetBackgroundStencil) \
  FUNC(sceGxmDepthStencilSurfaceGetForceLoadMode, sceGxmDepthStencilSurfaceGetForceLoadMode) \
  FUNC(sceGxmDepthStencilSurfaceGetForceStoreMode, sceGxmDepthStencilSurfaceGetForceStoreMode) \
  FUNC(sceGxmDepthStencilSurfaceGetFormat, sceGxmDepthStencilSurfaceGetFormat) \
  FUNC(sceGxmDepthStencilSurfaceGetStrideInSamples, sceGxmDepthStencilSurfaceGetStrideInSamples) \
  FUNC(sceGxmDepthStencilSurfaceInit, sceGxmDepthStencilSurfaceInit) \
  FUNC(sceGxmDepthStencilSurfaceInitDisabled, sceGxmDepthStencilSurfaceInitDisabled) \
  FUNC(sceGxmDepthStencilSurfaceIsEnabled, sceGxmDepthStencilSurfaceIsEnabled) \
  FUNC(sceGxmDepthStencilSurfaceSetBackgroundDepth, sceGxmDepthStencilSurfaceSetBackgroundDepth) \
  FUNC(sceGxmDepthStencilSurfaceSetBackgroundMask, sceGxmDepthStencilSurfaceSetBackgroundMask) \
  FUNC(sceGxmDepthStencilSurfaceSetBackgroundStencil, sceGxmDepthStencilSurfaceSetBackgroundStencil) \
  FUNC(sceGxmDepthStencilSurfaceSetForceLoadMode, sceGxmDepthStencilSurfaceSetForceLoadMode) \
  FUNC(sceGxmDepthStencilSurfaceSetForceStoreMode, sceGxmDepthStencilSurfaceSetForceStoreMode) \
  FUNC(sceGxmDestroyContext, sceGxmDestroyContext) \
  FUNC(sceGxmDestroyDeferredContext, sceGxmDestroyDeferredContext) \
  FUNC(sceGxmDestroyRenderTarget, sceGxmDestroyRenderTarget) \
  FUNC(sceGxmDisplayQueueAddEntry, sceGxmDisplayQueueAddEntry) \
  FUNC(sceGxmDisplayQueueFinish, sceGxmDisplayQueueFinish) \
  FUNC(sceGxmDraw, sceGxmDraw) \
  FUNC(sceGxmDrawInstanced, sceGxmDrawInstanced) \
  FUNC(sceGxmDrawPrecomputed, sceGxmDrawPrecomputed) \
  FUNC(sceGxmEndCommandList, sceGxmEndCommandList) \
  FUNC(sceGxmEndScene, sceGxmEndScene) \
  FUNC(sceGxmExecuteCommandList, sceGxmExecuteCommandList) \
  FUNC(sceGxmFinish, sceGxmFinish) \
  FUNC(sceGxmFragmentProgramGetPassType, sceGxmFragmentProgramGetPassType) \
  FUNC(sceGxmFragmentProgramGetProgram, sceGxmFragmentProgramGetProgram) \
  FUNC(sceGxmFragmentProgramIsEnabled, sceGxmFragmentProgramIsEnabled) \
  FUNC(sceGxmGetContextType, sceGxmGetContextType) \
  FUNC(sceGxmGetDeferredContextFragmentBuffer, sceGxmGetDeferredContextFragmentBuffer) \
  FUNC(sceGxmGetDeferredContextVdmBuffer, sceGxmGetDeferredContextVdmBuffer) \
  FUNC(sceGxmGetDeferredContextVertexBuffer, sceGxmGetDeferredContextVertexBuffer) \
  FUNC(sceGxmGetNotificationRegion, sceGxmGetNotificationRegion) \
  FUNC(sceGxmGetParameterBufferThreshold, sceGxmGetParameterBufferThreshold) \
  FUNC(sceGxmGetPrecomputedDrawSize, sceGxmGetPrecomputedDrawSize) \
  FUNC(sceGxmGetPrecomputedFragmentStateSize, sceGxmGetPrecomputedFragmentStateSize) \
  FUNC(sceGxmGetPrecomputedVertexStateSize, sceGxmGetPrecomputedVertexStateSize) \
  FUNC(sceGxmGetRenderTargetMemSize, sceGxmGetRenderTargetMemSize) \
  FUNC(sceGxmInitialize, sceGxmInitialize) \
  FUNC(sceGxmIsDebugVersion, sceGxmIsDebugVersion) \
  FUNC(sceGxmMapFragmentUsseMemory, sceGxmMapFragmentUsseMemory) \
  FUNC(sceGxmMapMemory, sceGxmMapMemory) \
  FUNC(sceGxmMapVertexUsseMemory, sceGxmMapVertexUsseMemory) \
  FUNC(sceGxmMidSceneFlush, sceGxmMidSceneFlush) \
  FUNC(sceGxmNotificationWait, sceGxmNotificationWait) \
  FUNC(sceGxmPadHeartbeat, sceGxmPadHeartbeat) \
  FUNC(sceGxmPadTriggerGpuPaTrace, sceGxmPadTriggerGpuPaTrace) \
  FUNC(sceGxmPopUserMarker, sceGxmPopUserMarker) \
  FUNC(sceGxmPrecomputedDrawInit, sceGxmPrecomputedDrawInit) \
  FUNC(sceGxmPrecomputedDrawSetAllVertexStreams, sceGxmPrecomputedDrawSetAllVertexStreams) \
  FUNC(sceGxmPrecomputedDrawSetParams, sceGxmPrecomputedDrawSetParams) \
  FUNC(sceGxmPrecomputedDrawSetParamsInstanced, sceGxmPrecomputedDrawSetParamsInstanced) \
  FUNC(sceGxmPrecomputedDrawSetVertexStream, sceGxmPrecomputedDrawSetVertexStream) \
  FUNC(sceGxmPrecomputedFragmentStateGetDefaultUniformBuffer, sceGxmPrecomputedFragmentStateGetDefaultUniformBuffer) \
  FUNC(sceGxmPrecomputedFragmentStateInit, sceGxmPrecomputedFragmentStateInit) \
  FUNC(sceGxmPrecomputedFragmentStateSetAllAuxiliarySurfaces, sceGxmPrecomputedFragmentStateSetAllAuxiliarySurfaces) \
  FUNC(sceGxmPrecomputedFragmentStateSetAllTextures, sceGxmPrecomputedFragmentStateSetAllTextures) \
  FUNC(sceGxmPrecomputedFragmentStateSetAllUniformBuffers, sceGxmPrecomputedFragmentStateSetAllUniformBuffers) \
  FUNC(sceGxmPrecomputedFragmentStateSetDefaultUniformBuffer, sceGxmPrecomputedFragmentStateSetDefaultUniformBuffer) \
  FUNC(sceGxmPrecomputedFragmentStateSetTexture, sceGxmPrecomputedFragmentStateSetTexture) \
  FUNC(sceGxmPrecomputedFragmentStateSetUniformBuffer, sceGxmPrecomputedFragmentStateSetUniformBuffer) \
  FUNC(sceGxmPrecomputedVertexStateGetDefaultUniformBuffer, sceGxmPrecomputedVertexStateGetDefaultUniformBuffer) \
  FUNC(sceGxmPrecomputedVertexStateInit, sceGxmPrecomputedVertexStateInit) \
  FUNC(sceGxmPrecomputedVertexStateSetAllTextures, sceGxmPrecomputedVertexStateSetAllTextures) \
  FUNC(sceGxmPrecomputedVertexStateSetAllUniformBuffers, sceGxmPrecomputedVertexStateSetAllUniformBuffers) \
  FUNC(sceGxmPrecomputedVertexStateSetDefaultUniformBuffer, sceGxmPrecomputedVertexStateSetDefaultUniformBuffer) \
  FUNC(sceGxmPrecomputedVertexStateSetTexture, sceGxmPrecomputedVertexStateSetTexture) \
  FUNC(sceGxmPrecomputedVertexStateSetUniformBuffer, sceGxmPrecomputedVertexStateSetUniformBuffer) \
  FUNC(sceGxmProgramCheck, sceGxmProgramCheck) \
  FUNC(sceGxmProgramFindParameterByName, sceGxmProgramFindParameterByName) \
  FUNC(sceGxmProgramFindParameterBySemantic, sceGxmProgramFindParameterBySemantic) \
  FUNC(sceGxmProgramGetDefaultUniformBufferSize, sceGxmProgramGetDefaultUniformBufferSize) \
  FUNC(sceGxmProgramGetFragmentProgramInputs, sceGxmProgramGetFragmentProgramInputs) \
  FUNC(sceGxmProgramGetOutputRegisterFormat, sceGxmProgramGetOutputRegisterFormat) \
  FUNC(sceGxmProgramGetParameter, sceGxmProgramGetParameter) \
  FUNC(sceGxmProgramGetParameterCount, sceGxmProgramGetParameterCount) \
  FUNC(sceGxmProgramGetSize, sceGxmProgramGetSize) \
  FUNC(sceGxmProgramGetType, sceGxmProgramGetType) \
  FUNC(sceGxmProgramGetVertexProgramOutputs, sceGxmProgramGetVertexProgramOutputs) \
  FUNC(sceGxmProgramIsDepthReplaceUsed, sceGxmProgramIsDepthReplaceUsed) \
  FUNC(sceGxmProgramIsDiscardUsed, sceGxmProgramIsDiscardUsed) \
  FUNC(sceGxmProgramIsEquivalent, sceGxmProgramIsEquivalent) \
  FUNC(sceGxmProgramIsFragColorUsed, sceGxmProgramIsFragColorUsed) \
  FUNC(sceGxmProgramIsNativeColorUsed, sceGxmProgramIsNativeColorUsed) \
  FUNC(sceGxmProgramIsSpriteCoordUsed, sceGxmProgramIsSpriteCoordUsed) \
  FUNC(sceGxmProgramParameterGetArraySize, sceGxmProgramParameterGetArraySize) \
  FUNC(sceGxmProgramParameterGetCategory, sceGxmProgramParameterGetCategory) \
  FUNC(sceGxmProgramParameterGetComponentCount, sceGxmProgramParameterGetComponentCount) \
  FUNC(sceGxmProgramParameterGetContainerIndex, sceGxmProgramParameterGetContainerIndex) \
  FUNC(sceGxmProgramParameterGetIndex, sceGxmProgramParameterGetIndex) \
  FUNC(sceGxmProgramParameterGetName, sceGxmProgramParameterGetName) \
  FUNC(sceGxmProgramParameterGetResourceIndex, sceGxmProgramParameterGetResourceIndex) \
  FUNC(sceGxmProgramParameterGetSemantic, sceGxmProgramParameterGetSemantic) \
  FUNC(sceGxmProgramParameterGetSemanticIndex, sceGxmProgramParameterGetSemanticIndex) \
  FUNC(sceGxmProgramParameterGetType, sceGxmProgramParameterGetType) \
  FUNC(sceGxmProgramParameterIsRegFormat, sceGxmProgramParameterIsRegFormat) \
  FUNC(sceGxmProgramParameterIsSamplerCube, sceGxmProgramParameterIsSamplerCube) \
  FUNC(sceGxmPushUserMarker, sceGxmPushUserMarker) \
  FUNC(sceGxmRemoveRazorGpuCaptureBuffer, sceGxmRemoveRazorGpuCaptureBuffer) \
  FUNC(sceGxmRenderTargetGetDriverMemBlock, sceGxmRenderTargetGetDriverMemBlock) \
  FUNC(sceGxmRenderTargetGetHostMem, sceGxmRenderTargetGetHostMem) \
  FUNC(sceGxmReserveFragmentDefaultUniformBuffer, sceGxmReserveFragmentDefaultUniformBuffer) \
  FUNC(sceGxmReserveVertexDefaultUniformBuffer, sceGxmReserveVertexDefaultUniformBuffer) \
  FUNC(sceGxmSetAuxiliarySurface, sceGxmSetAuxiliarySurface) \
  FUNC(sceGxmSetBackDepthBias, sceGxmSetBackDepthBias) \
  FUNC(sceGxmSetBackDepthFunc, sceGxmSetBackDepthFunc) \
  FUNC(sceGxmSetBackDepthWriteEnable, sceGxmSetBackDepthWriteEnable) \
  FUNC(sceGxmSetBackFragmentProgramEnable, sceGxmSetBackFragmentProgramEnable) \
  FUNC(sceGxmSetBackLineFillLastPixelEnable, sceGxmSetBackLineFillLastPixelEnable) \
  FUNC(sceGxmSetBackPointLineWidth, sceGxmSetBackPointLineWidth) \
  FUNC(sceGxmSetBackPolygonMode, sceGxmSetBackPolygonMode) \
  FUNC(sceGxmSetBackStencilFunc, sceGxmSetBackStencilFunc) \
  FUNC(sceGxmSetBackStencilRef, sceGxmSetBackStencilRef) \
  FUNC(sceGxmSetBackVisibilityTestEnable, sceGxmSetBackVisibilityTestEnable) \
  FUNC(sceGxmSetBackVisibilityTestIndex, sceGxmSetBackVisibilityTestIndex) \
  FUNC(sceGxmSetBackVisibilityTestOp, sceGxmSetBackVisibilityTestOp) \
  FUNC(sceGxmSetCullMode, sceGxmSetCullMode) \
  FUNC(sceGxmSetDefaultRegionClipAndViewport, sceGxmSetDefaultRegionClipAndViewport) \
  FUNC(sceGxmSetDeferredContextFragmentBuffer, sceGxmSetDeferredContextFragmentBuffer) \
  FUNC(sceGxmSetDeferredContextVdmBuffer, sceGxmSetDeferredContextVdmBuffer) \
  FUNC(sceGxmSetDeferredContextVertexBuffer, sceGxmSetDeferredContextVertexBuffer) \
  FUNC(sceGxmSetFragmentDefaultUniformBuffer, sceGxmSetFragmentDefaultUniformBuffer) \
  FUNC(sceGxmSetFragmentProgram, sceGxmSetFragmentProgram) \
  FUNC(sceGxmSetFragmentTexture, sceGxmSetFragmentTexture) \
  FUNC(sceGxmSetFragmentUniformBuffer, sceGxmSetFragmentUniformBuffer) \
  FUNC(sceGxmSetFrontDepthBias, sceGxmSetFrontDepthBias) \
  FUNC(sceGxmSetFrontDepthFunc, sceGxmSetFrontDepthFunc) \
  FUNC(sceGxmSetFrontDepthWriteEnable, sceGxmSetFrontDepthWriteEnable) \
  FUNC(sceGxmSetFrontFragmentProgramEnable, sceGxmSetFrontFragmentProgramEnable) \
  FUNC(sceGxmSetFrontLineFillLastPixelEnable, sceGxmSetFrontLineFillLastPixelEnable) \
  FUNC(sceGxmSetFrontPointLineWidth, sceGxmSetFrontPointLineWidth) \
  FUNC(sceGxmSetFrontPolygonMode, sceGxmSetFrontPolygonMode) \
  FUNC(sceGxmSetFrontStencilFunc, sceGxmSetFrontStencilFunc) \
  FUNC(sceGxmSetFrontStencilRef, sceGxmSetFrontStencilRef) \
  FUNC(sceGxmSetFrontVisibilityTestEnable, sceGxmSetFrontVisibilityTestEnable) \
  FUNC(sceGxmSetFrontVisibilityTestIndex, sceGxmSetFrontVisibilityTestIndex) \
  FUNC(sceGxmSetFrontVisibilityTestOp, sceGxmSetFrontVisibilityTestOp) \
  FUNC(sceGxmSetPrecomputedFragmentState, sceGxmSetPrecomputedFragmentState) \
  FUNC(sceGxmSetPrecomputedVertexState, sceGxmSetPrecomputedVertexState) \
  FUNC(sceGxmSetRegionClip, sceGxmSetRegionClip) \
  FUNC(sceGxmSetTwoSidedEnable, sceGxmSetTwoSidedEnable) \
  FUNC(sceGxmSetUniformDataF, sceGxmSetUniformDataF) \
  FUNC(sceGxmSetUserMarker, sceGxmSetUserMarker) \
  FUNC(sceGxmSetValidationEnable, sceGxmSetValidationEnable) \
  FUNC(sceGxmSetVertexDefaultUniformBuffer, sceGxmSetVertexDefaultUniformBuffer) \
  FUNC(sceGxmSetVertexProgram, sceGxmSetVertexProgram) \
  FUNC(sceGxmSetVertexStream, sceGxmSetVertexStream) \
  FUNC(sceGxmSetVertexTexture, sceGxmSetVertexTexture) \
  FUNC(sceGxmSetVertexUniformBuffer, sceGxmSetVertexUniformBuffer) \
  FUNC(sceGxmSetViewport, sceGxmSetViewport) \
  FUNC(sceGxmSetViewportEnable, sceGxmSetViewportEnable) \
  FUNC(sceGxmSetVisibilityBuffer, sceGxmSetVisibilityBuffer) \
  FUNC(sceGxmSetWBufferEnable, sceGxmSetWBufferEnable) \
  FUNC(sceGxmSetWClampEnable, sceGxmSetWClampEnable) \
  FUNC(sceGxmSetWClampValue, sceGxmSetWClampValue) \
  FUNC(sceGxmSetWarningEnabled, sceGxmSetWarningEnabled) \
  FUNC(sceGxmSetYuvProfile, sceGxmSetYuvProfile) \
  FUNC(sceGxmShaderPatcherAddRefFragmentProgram, sceGxmShaderPatcherAddRefFragmentProgram) \
  FUNC(sceGxmShaderPatcherAddRefVertexProgram, sceGxmShaderPatcherAddRefVertexProgram) \
  FUNC(sceGxmShaderPatcherCreate, sceGxmShaderPatcherCreate) \
  FUNC(sceGxmShaderPatcherCreateFragmentProgram, sceGxmShaderPatcherCreateFragmentProgram) \
  FUNC(sceGxmShaderPatcherCreateMaskUpdateFragmentProgram, sceGxmShaderPatcherCreateMaskUpdateFragmentProgram) \
  FUNC(sceGxmShaderPatcherCreateVertexProgram, sceGxmShaderPatcherCreateVertexProgram) \
  FUNC(sceGxmShaderPatcherDestroy, sceGxmShaderPatcherDestroy) \
  FUNC(sceGxmShaderPatcherForceUnregisterProgram, sceGxmShaderPatcherForceUnregisterProgram) \
  FUNC(sceGxmShaderPatcherGetBufferMemAllocated, sceGxmShaderPatcherGetBufferMemAllocated) \
  FUNC(sceGxmShaderPatcherGetFragmentProgramRefCount, sceGxmShaderPatcherGetFragmentProgramRefCount) \
  FUNC(sceGxmShaderPatcherGetFragmentUsseMemAllocated, sceGxmShaderPatcherGetFragmentUsseMemAllocated) \
  FUNC(sceGxmShaderPatcherGetHostMemAllocated, sceGxmShaderPatcherGetHostMemAllocated) \
  FUNC(sceGxmShaderPatcherGetProgramFromId, sceGxmShaderPatcherGetProgramFromId) \
  FUNC(sceGxmShaderPatcherGetUserData, sceGxmShaderPatcherGetUserData) \
  FUNC(sceGxmShaderPatcherGetVertexProgramRefCount, sceGxmShaderPatcherGetVertexProgramRefCount) \
  FUNC(sceGxmShaderPatcherGetVertexUsseMemAllocated, sceGxmShaderPatcherGetVertexUsseMemAllocated) \
  FUNC(sceGxmShaderPatcherRegisterProgram, sceGxmShaderPatcherRegisterProgram) \
  FUNC(sceGxmShaderPatcherReleaseFragmentProgram, sceGxmShaderPatcherReleaseFragmentProgram) \
  FUNC(sceGxmShaderPatcherReleaseVertexProgram, sceGxmShaderPatcherReleaseVertexProgram) \
  FUNC(sceGxmShaderPatcherSetAuxiliarySurface, sceGxmShaderPatcherSetAuxiliarySurface) \
  FUNC(sceGxmShaderPatcherSetUserData, sceGxmShaderPatcherSetUserData) \
  FUNC(sceGxmShaderPatcherUnregisterProgram, sceGxmShaderPatcherUnregisterProgram) \
  FUNC(sceGxmSyncObjectCreate, sceGxmSyncObjectCreate) \
  FUNC(sceGxmSyncObjectDestroy, sceGxmSyncObjectDestroy) \
  FUNC(sceGxmTerminate, sceGxmTerminate) \
  FUNC(sceGxmTextureGetData, sceGxmTextureGetData) \
  FUNC(sceGxmTextureGetFormat, sceGxmTextureGetFormat) \
  FUNC(sceGxmTextureGetGammaMode, sceGxmTextureGetGammaMode) \
  FUNC(sceGxmTextureGetHeight, sceGxmTextureGetHeight) \
  FUNC(sceGxmTextureGetLodBias, sceGxmTextureGetLodBias) \
  FUNC(sceGxmTextureGetLodMin, sceGxmTextureGetLodMin) \
  FUNC(sceGxmTextureGetMagFilter, sceGxmTextureGetMagFilter) \
  FUNC(sceGxmTextureGetMinFilter, sceGxmTextureGetMinFilter) \
  FUNC(sceGxmTextureGetMipFilter, sceGxmTextureGetMipFilter) \
  FUNC(sceGxmTextureGetMipmapCount, sceGxmTextureGetMipmapCount) \
  FUNC(sceGxmTextureGetMipmapCountUnsafe, sceGxmTextureGetMipmapCountUnsafe) \
  FUNC(sceGxmTextureGetNormalizeMode, sceGxmTextureGetNormalizeMode) \
  FUNC(sceGxmTextureGetPalette, sceGxmTextureGetPalette) \
  FUNC(sceGxmTextureGetStride, sceGxmTextureGetStride) \
  FUNC(sceGxmTextureGetType, sceGxmTextureGetType) \
  FUNC(sceGxmTextureGetUAddrMode, sceGxmTextureGetUAddrMode) \
  FUNC(sceGxmTextureGetUAddrModeSafe, sceGxmTextureGetUAddrModeSafe) \
  FUNC(sceGxmTextureGetVAddrMode, sceGxmTextureGetVAddrMode) \
  FUNC(sceGxmTextureGetVAddrModeSafe, sceGxmTextureGetVAddrModeSafe) \
  FUNC(sceGxmTextureGetWidth, sceGxmTextureGetWidth) \
  FUNC(sceGxmTextureInitCube, sceGxmTextureInitCube) \
  FUNC(sceGxmTextureInitCubeArbitrary, sceGxmTextureInitCubeArbitrary) \
  FUNC(sceGxmTextureInitLinear, sceGxmTextureInitLinear) \
  FUNC(sceGxmTextureInitLinearStrided, sceGxmTextureInitLinearStrided) \
  FUNC(sceGxmTextureInitSwizzled, sceGxmTextureInitSwizzled) \
  FUNC(sceGxmTextureInitSwizzledArbitrary, sceGxmTextureInitSwizzledArbitrary) \
  FUNC(sceGxmTextureInitTiled, sceGxmTextureInitTiled) \
  FUNC(sceGxmTextureSetData, sceGxmTextureSetData) \
  FUNC(sceGxmTextureSetFormat, sceGxmTextureSetFormat) \
  FUNC(sceGxmTextureSetGammaMode, sceGxmTextureSetGammaMode) \
  FUNC(sceGxmTextureSetHeight, sceGxmTextureSetHeight) \
  FUNC(sceGxmTextureSetLodBias, sceGxmTextureSetLodBias) \
  FUNC(sceGxmTextureSetLodMin, sceGxmTextureSetLodMin) \
  FUNC(sceGxmTextureSetMagFilter, sceGxmTextureSetMagFilter) \
  FUNC(sceGxmTextureSetMinFilter, sceGxmTextureSetMinFilter) \
  FUNC(sceGxmTextureSetMipFilter, sceGxmTextureSetMipFilter) \
  FUNC(sceGxmTextureSetMipmapCount, sceGxmTextureSetMipmapCount) \
  FUNC(sceGxmTextureSetNormalizeMode, sceGxmTextureSetNormalizeMode) \
  FUNC(sceGxmTextureSetPalette, sceGxmTextureSetPalette) \
  FUNC(sceGxmTextureSetStride, sceGxmTextureSetStride) \
  FUNC(sceGxmTextureSetUAddrMode, sceGxmTextureSetUAddrMode) \
  FUNC(sceGxmTextureSetUAddrModeSafe, sceGxmTextureSetUAddrModeSafe) \
  FUNC(sceGxmTextureSetVAddrMode, sceGxmTextureSetVAddrMode) \
  FUNC(sceGxmTextureSetVAddrModeSafe, sceGxmTextureSetVAddrModeSafe) \
  FUNC(sceGxmTextureSetWidth, sceGxmTextureSetWidth) \
  FUNC(sceGxmTextureValidate, sceGxmTextureValidate) \
  FUNC(sceGxmTransferCopy, sceGxmTransferCopy) \
  FUNC(sceGxmTransferDownscale, sceGxmTransferDownscale) \
  FUNC(sceGxmTransferFill, sceGxmTransferFill) \
  FUNC(sceGxmTransferFinish, sceGxmTransferFinish) \
  FUNC(sceGxmUnmapFragmentUsseMemory, sceGxmUnmapFragmentUsseMemory) \
  FUNC(sceGxmUnmapMemory, sceGxmUnmapMemory) \
  FUNC(sceGxmUnmapVertexUsseMemory, sceGxmUnmapVertexUsseMemory) \
  FUNC(sceGxmVertexFence, sceGxmVertexFence) \
  FUNC(sceGxmVertexProgramGetProgram, sceGxmVertexProgramGetProgram) \
  FUNC(sceGxmWaitEvent, sceGxmWaitEvent) \
  FUNC(ContextConfiguration, ContextConfiguration) \


template <>
rdcstr DoStringise(const GXMChunk & el)
{
  BEGIN_ENUM_STRINGISE(GXMChunk)
  {
    STRINGISE_ENUM_CLASS(DeviceInitialisation);

    STRINGISE_ENUM_CLASS_NAMED(MakeContextCurrent, "MakeContextCurrent");

    STRINGISE_ENUM_CLASS(vrapi_CreateTextureSwapChain);
    STRINGISE_ENUM_CLASS(vrapi_CreateTextureSwapChain2);

// re-use list of GL functions as chunks. Many of these will be aliased. This may not appear in the
// same order as the definition, but that's OK.
#define StringiseFunction(function, alias) STRINGISE_ENUM_CLASS_NAMED(alias, STRINGIZE(alias));

    ForEachSupported(StringiseFunction)
  }
  END_ENUM_STRINGISE()
}

template <>
rdcstr DoStringise(const SceGxmPrimitiveType &el)
{
  BEGIN_ENUM_STRINGISE(SceGxmPrimitiveType);
  {
    STRINGISE_ENUM(SCE_GXM_PRIMITIVE_TRIANGLES)
    STRINGISE_ENUM(SCE_GXM_PRIMITIVE_LINES)
    STRINGISE_ENUM(SCE_GXM_PRIMITIVE_POINTS)
    STRINGISE_ENUM(SCE_GXM_PRIMITIVE_TRIANGLE_STRIP)
    STRINGISE_ENUM(SCE_GXM_PRIMITIVE_TRIANGLE_FAN)
    STRINGISE_ENUM(SCE_GXM_PRIMITIVE_TRIANGLE_EDGES)
  }
  END_ENUM_STRINGISE();
}

template <>
rdcstr DoStringise(const SceGxmIndexFormat &el)
{
  BEGIN_ENUM_STRINGISE(SceGxmIndexFormat);
  {
    STRINGISE_ENUM(SCE_GXM_INDEX_FORMAT_U16)
    STRINGISE_ENUM(SCE_GXM_INDEX_FORMAT_U32)
  }
  END_ENUM_STRINGISE();
}

template <>
rdcstr DoStringise(const SceGxmDepthFunc &el)
{
  BEGIN_ENUM_STRINGISE(SceGxmDepthFunc);
  {
    STRINGISE_ENUM(SCE_GXM_DEPTH_FUNC_NEVER)
    STRINGISE_ENUM(SCE_GXM_DEPTH_FUNC_LESS)
    STRINGISE_ENUM(SCE_GXM_DEPTH_FUNC_EQUAL)
    STRINGISE_ENUM(SCE_GXM_DEPTH_FUNC_LESS_EQUAL)
    STRINGISE_ENUM(SCE_GXM_DEPTH_FUNC_GREATER)
    STRINGISE_ENUM(SCE_GXM_DEPTH_FUNC_NOT_EQUAL)
    STRINGISE_ENUM(SCE_GXM_DEPTH_FUNC_GREATER_EQUAL)
    STRINGISE_ENUM(SCE_GXM_DEPTH_FUNC_ALWAYS)
  }
  END_ENUM_STRINGISE();
}

template <>
rdcstr DoStringise(const SceGxmStencilFunc &el)
{
  BEGIN_ENUM_STRINGISE(SceGxmStencilFunc);
  {
    STRINGISE_ENUM(SCE_GXM_STENCIL_FUNC_NEVER)
    STRINGISE_ENUM(SCE_GXM_STENCIL_FUNC_LESS)
    STRINGISE_ENUM(SCE_GXM_STENCIL_FUNC_EQUAL)
    STRINGISE_ENUM(SCE_GXM_STENCIL_FUNC_LESS_EQUAL)
    STRINGISE_ENUM(SCE_GXM_STENCIL_FUNC_GREATER)
    STRINGISE_ENUM(SCE_GXM_STENCIL_FUNC_NOT_EQUAL)
    STRINGISE_ENUM(SCE_GXM_STENCIL_FUNC_GREATER_EQUAL)
    STRINGISE_ENUM(SCE_GXM_STENCIL_FUNC_ALWAYS)
  }
  END_ENUM_STRINGISE();
}

template <>
rdcstr DoStringise(const SceGxmStencilOp &el)
{
  BEGIN_ENUM_STRINGISE(SceGxmStencilOp);
  {
    STRINGISE_ENUM(SCE_GXM_STENCIL_OP_KEEP)
    STRINGISE_ENUM(SCE_GXM_STENCIL_OP_ZERO)
    STRINGISE_ENUM(SCE_GXM_STENCIL_OP_REPLACE)
    STRINGISE_ENUM(SCE_GXM_STENCIL_OP_INCR)
    STRINGISE_ENUM(SCE_GXM_STENCIL_OP_DECR)
    STRINGISE_ENUM(SCE_GXM_STENCIL_OP_INVERT)
    STRINGISE_ENUM(SCE_GXM_STENCIL_OP_INCR_WRAP)
    STRINGISE_ENUM(SCE_GXM_STENCIL_OP_DECR_WRAP)
  }
  END_ENUM_STRINGISE();
}

template <>
rdcstr DoStringise(const SceGxmDepthWriteMode &el)
{
  BEGIN_ENUM_STRINGISE(SceGxmDepthWriteMode);
  {
    STRINGISE_ENUM(SCE_GXM_DEPTH_WRITE_DISABLED)
    STRINGISE_ENUM(SCE_GXM_DEPTH_WRITE_ENABLED)
  }
  END_ENUM_STRINGISE();
}

template <>
rdcstr DoStringise(const SceDisplayPixelFormat &el)
{
  BEGIN_ENUM_STRINGISE(SceDisplayPixelFormat);
  {
    STRINGISE_ENUM(SCE_DISPLAY_PIXELFORMAT_A8B8G8R8)
  }
  END_ENUM_STRINGISE();
}

template <>
rdcstr DoStringise(const SceGxmMemoryAttribFlags &el)
{
  BEGIN_ENUM_STRINGISE(SceGxmMemoryAttribFlags);
  {
    STRINGISE_ENUM(SCE_GXM_MEMORY_ATTRIB_READ)
    STRINGISE_ENUM(SCE_GXM_MEMORY_ATTRIB_WRITE)
    STRINGISE_ENUM(SCE_GXM_MEMORY_ATTRIB_RW)
  }
  END_ENUM_STRINGISE();
}
