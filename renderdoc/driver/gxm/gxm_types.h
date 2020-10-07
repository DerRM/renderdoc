#pragma once

enum class GXMChunk : uint32_t
{
  DeviceInitialisation = 1000,

  MakeContextCurrent,

  vrapi_CreateTextureSwapChain,
  vrapi_CreateTextureSwapChain2,

  sceGxmAddRazorGpuCaptureBuffer,
  sceGxmBeginCommandList,
  sceGxmBeginScene,
  sceGxmBeginSceneEx,
  sceGxmColorSurfaceGetClip,
  sceGxmColorSurfaceGetData,
  sceGxmColorSurfaceGetDitherMode,
  sceGxmColorSurfaceGetFormat,
  sceGxmColorSurfaceGetGammaMode,
  sceGxmColorSurfaceGetScaleMode,
  sceGxmColorSurfaceGetStrideInPixels,
  sceGxmColorSurfaceGetType,
  sceGxmColorSurfaceInit,
  sceGxmColorSurfaceInitDisabled,
  sceGxmColorSurfaceIsEnabled,
  sceGxmColorSurfaceSetClip,
  sceGxmColorSurfaceSetData,
  sceGxmColorSurfaceSetDitherMode,
  sceGxmColorSurfaceSetFormat,
  sceGxmColorSurfaceSetGammaMode,
  sceGxmColorSurfaceSetScaleMode,
  sceGxmCreateContext,
  sceGxmCreateDeferredContext,
  sceGxmCreateRenderTarget,
  sceGxmDepthStencilSurfaceGetBackgroundDepth,
  sceGxmDepthStencilSurfaceGetBackgroundMask,
  sceGxmDepthStencilSurfaceGetBackgroundStencil,
  sceGxmDepthStencilSurfaceGetForceLoadMode,
  sceGxmDepthStencilSurfaceGetForceStoreMode,
  sceGxmDepthStencilSurfaceGetFormat,
  sceGxmDepthStencilSurfaceGetStrideInSamples,
  sceGxmDepthStencilSurfaceInit,
  sceGxmDepthStencilSurfaceInitDisabled,
  sceGxmDepthStencilSurfaceIsEnabled,
  sceGxmDepthStencilSurfaceSetBackgroundDepth,
  sceGxmDepthStencilSurfaceSetBackgroundMask,
  sceGxmDepthStencilSurfaceSetBackgroundStencil,
  sceGxmDepthStencilSurfaceSetForceLoadMode,
  sceGxmDepthStencilSurfaceSetForceStoreMode,
  sceGxmDestroyContext,
  sceGxmDestroyDeferredContext,
  sceGxmDestroyRenderTarget,
  sceGxmDisplayQueueAddEntry,
  sceGxmDisplayQueueFinish,
  sceGxmDraw,
  sceGxmDrawInstanced,
  sceGxmDrawPrecomputed,
  sceGxmEndCommandList,
  sceGxmEndScene,
  sceGxmExecuteCommandList,
  sceGxmFinish,
  sceGxmFragmentProgramGetPassType,
  sceGxmFragmentProgramGetProgram,
  sceGxmFragmentProgramIsEnabled,
  sceGxmGetContextType,
  sceGxmGetDeferredContextFragmentBuffer,
  sceGxmGetDeferredContextVdmBuffer,
  sceGxmGetDeferredContextVertexBuffer,
  sceGxmGetNotificationRegion,
  sceGxmGetParameterBufferThreshold,
  sceGxmGetPrecomputedDrawSize,
  sceGxmGetPrecomputedFragmentStateSize,
  sceGxmGetPrecomputedVertexStateSize,
  sceGxmGetRenderTargetMemSize,
  sceGxmInitialize,
  sceGxmIsDebugVersion,
  sceGxmMapFragmentUsseMemory,
  sceGxmMapMemory,
  sceGxmMapVertexUsseMemory,
  sceGxmMidSceneFlush,
  sceGxmNotificationWait,
  sceGxmPadHeartbeat,
  sceGxmPadTriggerGpuPaTrace,
  sceGxmPopUserMarker,
  sceGxmPrecomputedDrawInit,
  sceGxmPrecomputedDrawSetAllVertexStreams,
  sceGxmPrecomputedDrawSetParams,
  sceGxmPrecomputedDrawSetParamsInstanced,
  sceGxmPrecomputedDrawSetVertexStream,
  sceGxmPrecomputedFragmentStateGetDefaultUniformBuffer,
  sceGxmPrecomputedFragmentStateInit,
  sceGxmPrecomputedFragmentStateSetAllAuxiliarySurfaces,
  sceGxmPrecomputedFragmentStateSetAllTextures,
  sceGxmPrecomputedFragmentStateSetAllUniformBuffers,
  sceGxmPrecomputedFragmentStateSetDefaultUniformBuffer,
  sceGxmPrecomputedFragmentStateSetTexture,
  sceGxmPrecomputedFragmentStateSetUniformBuffer,
  sceGxmPrecomputedVertexStateGetDefaultUniformBuffer,
  sceGxmPrecomputedVertexStateInit,
  sceGxmPrecomputedVertexStateSetAllTextures,
  sceGxmPrecomputedVertexStateSetAllUniformBuffers,
  sceGxmPrecomputedVertexStateSetDefaultUniformBuffer,
  sceGxmPrecomputedVertexStateSetTexture,
  sceGxmPrecomputedVertexStateSetUniformBuffer,
  sceGxmProgramCheck,
  sceGxmProgramFindParameterByName,
  sceGxmProgramFindParameterBySemantic,
  sceGxmProgramGetDefaultUniformBufferSize,
  sceGxmProgramGetFragmentProgramInputs,
  sceGxmProgramGetOutputRegisterFormat,
  sceGxmProgramGetParameter,
  sceGxmProgramGetParameterCount,
  sceGxmProgramGetSize,
  sceGxmProgramGetType,
  sceGxmProgramGetVertexProgramOutputs,
  sceGxmProgramIsDepthReplaceUsed,
  sceGxmProgramIsDiscardUsed,
  sceGxmProgramIsEquivalent,
  sceGxmProgramIsFragColorUsed,
  sceGxmProgramIsNativeColorUsed,
  sceGxmProgramIsSpriteCoordUsed,
  sceGxmProgramParameterGetArraySize,
  sceGxmProgramParameterGetCategory,
  sceGxmProgramParameterGetComponentCount,
  sceGxmProgramParameterGetContainerIndex,
  sceGxmProgramParameterGetIndex,
  sceGxmProgramParameterGetName,
  sceGxmProgramParameterGetResourceIndex,
  sceGxmProgramParameterGetSemantic,
  sceGxmProgramParameterGetSemanticIndex,
  sceGxmProgramParameterGetType,
  sceGxmProgramParameterIsRegFormat,
  sceGxmProgramParameterIsSamplerCube,
  sceGxmPushUserMarker,
  sceGxmRemoveRazorGpuCaptureBuffer,
  sceGxmRenderTargetGetDriverMemBlock,
  sceGxmRenderTargetGetHostMem,
  sceGxmReserveFragmentDefaultUniformBuffer,
  sceGxmReserveVertexDefaultUniformBuffer,
  sceGxmSetAuxiliarySurface,
  sceGxmSetBackDepthBias,
  sceGxmSetBackDepthFunc,
  sceGxmSetBackDepthWriteEnable,
  sceGxmSetBackFragmentProgramEnable,
  sceGxmSetBackLineFillLastPixelEnable,
  sceGxmSetBackPointLineWidth,
  sceGxmSetBackPolygonMode,
  sceGxmSetBackStencilFunc,
  sceGxmSetBackStencilRef,
  sceGxmSetBackVisibilityTestEnable,
  sceGxmSetBackVisibilityTestIndex,
  sceGxmSetBackVisibilityTestOp,
  sceGxmSetCullMode,
  sceGxmSetDefaultRegionClipAndViewport,
  sceGxmSetDeferredContextFragmentBuffer,
  sceGxmSetDeferredContextVdmBuffer,
  sceGxmSetDeferredContextVertexBuffer,
  sceGxmSetFragmentDefaultUniformBuffer,
  sceGxmSetFragmentProgram,
  sceGxmSetFragmentTexture,
  sceGxmSetFragmentUniformBuffer,
  sceGxmSetFrontDepthBias,
  sceGxmSetFrontDepthFunc,
  sceGxmSetFrontDepthWriteEnable,
  sceGxmSetFrontFragmentProgramEnable,
  sceGxmSetFrontLineFillLastPixelEnable,
  sceGxmSetFrontPointLineWidth,
  sceGxmSetFrontPolygonMode,
  sceGxmSetFrontStencilFunc,
  sceGxmSetFrontStencilRef,
  sceGxmSetFrontVisibilityTestEnable,
  sceGxmSetFrontVisibilityTestIndex,
  sceGxmSetFrontVisibilityTestOp,
  sceGxmSetPrecomputedFragmentState,
  sceGxmSetPrecomputedVertexState,
  sceGxmSetRegionClip,
  sceGxmSetTwoSidedEnable,
  sceGxmSetUniformDataF,
  sceGxmSetUserMarker,
  sceGxmSetValidationEnable,
  sceGxmSetVertexDefaultUniformBuffer,
  sceGxmSetVertexProgram,
  sceGxmSetVertexStream,
  sceGxmSetVertexTexture,
  sceGxmSetVertexUniformBuffer,
  sceGxmSetViewport,
  sceGxmSetViewportEnable,
  sceGxmSetVisibilityBuffer,
  sceGxmSetWBufferEnable,
  sceGxmSetWClampEnable,
  sceGxmSetWClampValue,
  sceGxmSetWarningEnabled,
  sceGxmSetYuvProfile,
  sceGxmShaderPatcherAddRefFragmentProgram,
  sceGxmShaderPatcherAddRefVertexProgram,
  sceGxmShaderPatcherCreate,
  sceGxmShaderPatcherCreateFragmentProgram,
  sceGxmShaderPatcherCreateMaskUpdateFragmentProgram,
  sceGxmShaderPatcherCreateVertexProgram,
  sceGxmShaderPatcherDestroy,
  sceGxmShaderPatcherForceUnregisterProgram,
  sceGxmShaderPatcherGetBufferMemAllocated,
  sceGxmShaderPatcherGetFragmentProgramRefCount,
  sceGxmShaderPatcherGetFragmentUsseMemAllocated,
  sceGxmShaderPatcherGetHostMemAllocated,
  sceGxmShaderPatcherGetProgramFromId,
  sceGxmShaderPatcherGetUserData,
  sceGxmShaderPatcherGetVertexProgramRefCount,
  sceGxmShaderPatcherGetVertexUsseMemAllocated,
  sceGxmShaderPatcherRegisterProgram,
  sceGxmShaderPatcherReleaseFragmentProgram,
  sceGxmShaderPatcherReleaseVertexProgram,
  sceGxmShaderPatcherSetAuxiliarySurface,
  sceGxmShaderPatcherSetUserData,
  sceGxmShaderPatcherUnregisterProgram,
  sceGxmSyncObjectCreate,
  sceGxmSyncObjectDestroy,
  sceGxmTerminate,
  sceGxmTextureGetData,
  sceGxmTextureGetFormat,
  sceGxmTextureGetGammaMode,
  sceGxmTextureGetHeight,
  sceGxmTextureGetLodBias,
  sceGxmTextureGetLodMin,
  sceGxmTextureGetMagFilter,
  sceGxmTextureGetMinFilter,
  sceGxmTextureGetMipFilter,
  sceGxmTextureGetMipmapCount,
  sceGxmTextureGetMipmapCountUnsafe,
  sceGxmTextureGetNormalizeMode,
  sceGxmTextureGetPalette,
  sceGxmTextureGetStride,
  sceGxmTextureGetType,
  sceGxmTextureGetUAddrMode,
  sceGxmTextureGetUAddrModeSafe,
  sceGxmTextureGetVAddrMode,
  sceGxmTextureGetVAddrModeSafe,
  sceGxmTextureGetWidth,
  sceGxmTextureInitCube,
  sceGxmTextureInitCubeArbitrary,
  sceGxmTextureInitLinear,
  sceGxmTextureInitLinearStrided,
  sceGxmTextureInitSwizzled,
  sceGxmTextureInitSwizzledArbitrary,
  sceGxmTextureInitTiled,
  sceGxmTextureSetData,
  sceGxmTextureSetFormat,
  sceGxmTextureSetGammaMode,
  sceGxmTextureSetHeight,
  sceGxmTextureSetLodBias,
  sceGxmTextureSetLodMin,
  sceGxmTextureSetMagFilter,
  sceGxmTextureSetMinFilter,
  sceGxmTextureSetMipFilter,
  sceGxmTextureSetMipmapCount,
  sceGxmTextureSetNormalizeMode,
  sceGxmTextureSetPalette,
  sceGxmTextureSetStride,
  sceGxmTextureSetUAddrMode,
  sceGxmTextureSetUAddrModeSafe,
  sceGxmTextureSetVAddrMode,
  sceGxmTextureSetVAddrModeSafe,
  sceGxmTextureSetWidth,
  sceGxmTextureValidate,
  sceGxmTransferCopy,
  sceGxmTransferDownscale,
  sceGxmTransferFill,
  sceGxmTransferFinish,
  sceGxmUnmapFragmentUsseMemory,
  sceGxmUnmapMemory,
  sceGxmUnmapVertexUsseMemory,
  sceGxmVertexFence,
  sceGxmVertexProgramGetProgram,
  sceGxmWaitEvent,
  Max,
};