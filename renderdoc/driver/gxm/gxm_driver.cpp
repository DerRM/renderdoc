/******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2019-2020 Baldur Karlsson
 * Copyright (c) 2014 Crytek
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ******************************************************************************/

#include "gxm_driver.h"
#include "serialise/rdcfile.h"

#include "gxm_replay.h"

void WrappedGXM::StartFrameCapture(void *dev, void *wnd) {}

bool WrappedGXM::EndFrameCapture(void *dev, void *wnd)
{
  return false;
}

bool WrappedGXM::DiscardFrameCapture(void *dev, void *wnd)
{
  return false;
}

ReplayStatus WrappedGXM::ReadLogInitialisation(RDCFile *rdc, bool storeStructuredBuffers)
{
  int sectionIdx = rdc->SectionIndex(SectionType::FrameCapture);

  if(sectionIdx < 0)
    return ReplayStatus::FileCorrupted;

  StreamReader *reader = rdc->ReadSection(sectionIdx);

  if(reader->IsErrored())
  {
    delete reader;
    return ReplayStatus::FileIOFailed;
  }

  ReadSerialiser ser(reader, Ownership::Stream);

  for(;;)
  {
    GXMChunk context = ser.ReadChunk<GXMChunk>();

    bool success = ProcessChunk(ser, context);

    if(!success)
      return ReplayStatus::APIReplayFailed;

    if((SystemChunk)context == SystemChunk::CaptureScope || reader->IsErrored() || reader->AtEnd())
      break;
  }

  m_Replay->WriteFrameRecord().drawcallList = m_Drawcalls;

  return ReplayStatus::Succeeded;
}

void WrappedGXM::AddDrawcall(const DrawcallDescription &d) 
{
  m_Drawcalls.push_back(d);
}

bool WrappedGXM::ProcessChunk(ReadSerialiser &ser, GXMChunk chunk)
{
  switch(chunk)
  {
    case GXMChunk::DeviceInitialisation: break;
    case GXMChunk::MakeContextCurrent: break;
    case GXMChunk::vrapi_CreateTextureSwapChain: break;
    case GXMChunk::vrapi_CreateTextureSwapChain2: break;
    case GXMChunk::sceGxmAddRazorGpuCaptureBuffer: break;
    case GXMChunk::sceGxmBeginCommandList: break;
    case GXMChunk::sceGxmBeginScene: 
      return Serialise_sceGxmBeginScene(ser, 0, 0, 0, 0, 0, 0, 0, 0);
    case GXMChunk::sceGxmBeginSceneEx: break;
    case GXMChunk::sceGxmColorSurfaceGetClip: break;
    case GXMChunk::sceGxmColorSurfaceGetData: break;
    case GXMChunk::sceGxmColorSurfaceGetDitherMode: break;
    case GXMChunk::sceGxmColorSurfaceGetFormat: break;
    case GXMChunk::sceGxmColorSurfaceGetGammaMode: break;
    case GXMChunk::sceGxmColorSurfaceGetScaleMode: break;
    case GXMChunk::sceGxmColorSurfaceGetStrideInPixels: break;
    case GXMChunk::sceGxmColorSurfaceGetType: break;
    case GXMChunk::sceGxmColorSurfaceInit: break;
    case GXMChunk::sceGxmColorSurfaceInitDisabled: break;
    case GXMChunk::sceGxmColorSurfaceIsEnabled: break;
    case GXMChunk::sceGxmColorSurfaceSetClip: break;
    case GXMChunk::sceGxmColorSurfaceSetData: break;
    case GXMChunk::sceGxmColorSurfaceSetDitherMode: break;
    case GXMChunk::sceGxmColorSurfaceSetFormat: break;
    case GXMChunk::sceGxmColorSurfaceSetGammaMode: break;
    case GXMChunk::sceGxmColorSurfaceSetScaleMode: break;
    case GXMChunk::sceGxmCreateContext: break;
    case GXMChunk::sceGxmCreateDeferredContext: break;
    case GXMChunk::sceGxmCreateRenderTarget: break;
    case GXMChunk::sceGxmDepthStencilSurfaceGetBackgroundDepth: break;
    case GXMChunk::sceGxmDepthStencilSurfaceGetBackgroundMask: break;
    case GXMChunk::sceGxmDepthStencilSurfaceGetBackgroundStencil: break;
    case GXMChunk::sceGxmDepthStencilSurfaceGetForceLoadMode: break;
    case GXMChunk::sceGxmDepthStencilSurfaceGetForceStoreMode: break;
    case GXMChunk::sceGxmDepthStencilSurfaceGetFormat: break;
    case GXMChunk::sceGxmDepthStencilSurfaceGetStrideInSamples: break;
    case GXMChunk::sceGxmDepthStencilSurfaceInit: break;
    case GXMChunk::sceGxmDepthStencilSurfaceInitDisabled: break;
    case GXMChunk::sceGxmDepthStencilSurfaceIsEnabled: break;
    case GXMChunk::sceGxmDepthStencilSurfaceSetBackgroundDepth: break;
    case GXMChunk::sceGxmDepthStencilSurfaceSetBackgroundMask: break;
    case GXMChunk::sceGxmDepthStencilSurfaceSetBackgroundStencil: break;
    case GXMChunk::sceGxmDepthStencilSurfaceSetForceLoadMode: break;
    case GXMChunk::sceGxmDepthStencilSurfaceSetForceStoreMode: break;
    case GXMChunk::sceGxmDestroyContext: break;
    case GXMChunk::sceGxmDestroyDeferredContext: break;
    case GXMChunk::sceGxmDestroyRenderTarget: break;
    case GXMChunk::sceGxmDisplayQueueAddEntry: 
      return Serialise_sceGxmDisplayQueueAddEntry(ser, 0, 0, 0);
    case GXMChunk::sceGxmDisplayQueueFinish: break;
    case GXMChunk::sceGxmDraw:
      return Serialise_sceGxmDraw(ser, 0, (SceGxmPrimitiveType)0, (SceGxmIndexFormat)0, 0, 0);
    case GXMChunk::sceGxmDrawInstanced: break;
    case GXMChunk::sceGxmDrawPrecomputed: break;
    case GXMChunk::sceGxmEndCommandList: break;
    case GXMChunk::sceGxmEndScene: 
      return Serialise_sceGxmEndScene(ser, 0, 0, 0);
    case GXMChunk::sceGxmExecuteCommandList: break;
    case GXMChunk::sceGxmFinish: break;
    case GXMChunk::sceGxmFragmentProgramGetPassType: break;
    case GXMChunk::sceGxmFragmentProgramGetProgram: break;
    case GXMChunk::sceGxmFragmentProgramIsEnabled: break;
    case GXMChunk::sceGxmGetContextType: break;
    case GXMChunk::sceGxmGetDeferredContextFragmentBuffer: break;
    case GXMChunk::sceGxmGetDeferredContextVdmBuffer: break;
    case GXMChunk::sceGxmGetDeferredContextVertexBuffer: break;
    case GXMChunk::sceGxmGetNotificationRegion: break;
    case GXMChunk::sceGxmGetParameterBufferThreshold: break;
    case GXMChunk::sceGxmGetPrecomputedDrawSize: break;
    case GXMChunk::sceGxmGetPrecomputedFragmentStateSize: break;
    case GXMChunk::sceGxmGetPrecomputedVertexStateSize: break;
    case GXMChunk::sceGxmGetRenderTargetMemSize: break;
    case GXMChunk::sceGxmInitialize: 
      return Serialise_sceGxmInitialize(ser, 0);
    case GXMChunk::sceGxmIsDebugVersion: break;
    case GXMChunk::sceGxmMapFragmentUsseMemory: break;
    case GXMChunk::sceGxmMapMemory: break;
    case GXMChunk::sceGxmMapVertexUsseMemory: break;
    case GXMChunk::sceGxmMidSceneFlush: break;
    case GXMChunk::sceGxmNotificationWait: break;
    case GXMChunk::sceGxmPadHeartbeat: 
      return Serialise_sceGxmPadHeartbeat(ser, 0, 0);
    case GXMChunk::sceGxmPadTriggerGpuPaTrace: break;
    case GXMChunk::sceGxmPopUserMarker: break;
    case GXMChunk::sceGxmPrecomputedDrawInit: break;
    case GXMChunk::sceGxmPrecomputedDrawSetAllVertexStreams: break;
    case GXMChunk::sceGxmPrecomputedDrawSetParams: break;
    case GXMChunk::sceGxmPrecomputedDrawSetParamsInstanced: break;
    case GXMChunk::sceGxmPrecomputedDrawSetVertexStream: break;
    case GXMChunk::sceGxmPrecomputedFragmentStateGetDefaultUniformBuffer: break;
    case GXMChunk::sceGxmPrecomputedFragmentStateInit: break;
    case GXMChunk::sceGxmPrecomputedFragmentStateSetAllAuxiliarySurfaces: break;
    case GXMChunk::sceGxmPrecomputedFragmentStateSetAllTextures: break;
    case GXMChunk::sceGxmPrecomputedFragmentStateSetAllUniformBuffers: break;
    case GXMChunk::sceGxmPrecomputedFragmentStateSetDefaultUniformBuffer: break;
    case GXMChunk::sceGxmPrecomputedFragmentStateSetTexture: break;
    case GXMChunk::sceGxmPrecomputedFragmentStateSetUniformBuffer: break;
    case GXMChunk::sceGxmPrecomputedVertexStateGetDefaultUniformBuffer: break;
    case GXMChunk::sceGxmPrecomputedVertexStateInit: break;
    case GXMChunk::sceGxmPrecomputedVertexStateSetAllTextures: break;
    case GXMChunk::sceGxmPrecomputedVertexStateSetAllUniformBuffers: break;
    case GXMChunk::sceGxmPrecomputedVertexStateSetDefaultUniformBuffer: break;
    case GXMChunk::sceGxmPrecomputedVertexStateSetTexture: break;
    case GXMChunk::sceGxmPrecomputedVertexStateSetUniformBuffer: break;
    case GXMChunk::sceGxmProgramCheck: break;
    case GXMChunk::sceGxmProgramFindParameterByName: break;
    case GXMChunk::sceGxmProgramFindParameterBySemantic: break;
    case GXMChunk::sceGxmProgramGetDefaultUniformBufferSize: break;
    case GXMChunk::sceGxmProgramGetFragmentProgramInputs: break;
    case GXMChunk::sceGxmProgramGetOutputRegisterFormat: break;
    case GXMChunk::sceGxmProgramGetParameter: break;
    case GXMChunk::sceGxmProgramGetParameterCount: break;
    case GXMChunk::sceGxmProgramGetSize: break;
    case GXMChunk::sceGxmProgramGetType: break;
    case GXMChunk::sceGxmProgramGetVertexProgramOutputs: break;
    case GXMChunk::sceGxmProgramIsDepthReplaceUsed: break;
    case GXMChunk::sceGxmProgramIsDiscardUsed: break;
    case GXMChunk::sceGxmProgramIsEquivalent: break;
    case GXMChunk::sceGxmProgramIsFragColorUsed: break;
    case GXMChunk::sceGxmProgramIsNativeColorUsed: break;
    case GXMChunk::sceGxmProgramIsSpriteCoordUsed: break;
    case GXMChunk::sceGxmProgramParameterGetArraySize: break;
    case GXMChunk::sceGxmProgramParameterGetCategory: break;
    case GXMChunk::sceGxmProgramParameterGetComponentCount: break;
    case GXMChunk::sceGxmProgramParameterGetContainerIndex: break;
    case GXMChunk::sceGxmProgramParameterGetIndex: break;
    case GXMChunk::sceGxmProgramParameterGetName: break;
    case GXMChunk::sceGxmProgramParameterGetResourceIndex: break;
    case GXMChunk::sceGxmProgramParameterGetSemantic: break;
    case GXMChunk::sceGxmProgramParameterGetSemanticIndex: break;
    case GXMChunk::sceGxmProgramParameterGetType: break;
    case GXMChunk::sceGxmProgramParameterIsRegFormat: break;
    case GXMChunk::sceGxmProgramParameterIsSamplerCube: break;
    case GXMChunk::sceGxmPushUserMarker: break;
    case GXMChunk::sceGxmRemoveRazorGpuCaptureBuffer: break;
    case GXMChunk::sceGxmRenderTargetGetDriverMemBlock: break;
    case GXMChunk::sceGxmRenderTargetGetHostMem: break;
    case GXMChunk::sceGxmReserveFragmentDefaultUniformBuffer: 
      return Serialise_sceGxmReserveFragmentDefaultUniformBuffer(ser, 0, 0);
    case GXMChunk::sceGxmReserveVertexDefaultUniformBuffer: 
      return Serialise_sceGxmReserveVertexDefaultUniformBuffer(ser, 0, 0);
    case GXMChunk::sceGxmSetAuxiliarySurface: break;
    case GXMChunk::sceGxmSetBackDepthBias: break;
    case GXMChunk::sceGxmSetBackDepthFunc: break;
    case GXMChunk::sceGxmSetBackDepthWriteEnable: break;
    case GXMChunk::sceGxmSetBackFragmentProgramEnable: break;
    case GXMChunk::sceGxmSetBackLineFillLastPixelEnable: break;
    case GXMChunk::sceGxmSetBackPointLineWidth: break;
    case GXMChunk::sceGxmSetBackPolygonMode: break;
    case GXMChunk::sceGxmSetBackStencilFunc: break;
    case GXMChunk::sceGxmSetBackStencilRef: break;
    case GXMChunk::sceGxmSetBackVisibilityTestEnable: break;
    case GXMChunk::sceGxmSetBackVisibilityTestIndex: break;
    case GXMChunk::sceGxmSetBackVisibilityTestOp: break;
    case GXMChunk::sceGxmSetCullMode: break;
    case GXMChunk::sceGxmSetDefaultRegionClipAndViewport: break;
    case GXMChunk::sceGxmSetDeferredContextFragmentBuffer: break;
    case GXMChunk::sceGxmSetDeferredContextVdmBuffer: break;
    case GXMChunk::sceGxmSetDeferredContextVertexBuffer: break;
    case GXMChunk::sceGxmSetFragmentDefaultUniformBuffer: break;
    case GXMChunk::sceGxmSetFragmentProgram: 
      return Serialise_sceGxmSetFragmentProgram(ser, 0, 0);
    case GXMChunk::sceGxmSetFragmentTexture: break;
    case GXMChunk::sceGxmSetFragmentUniformBuffer: break;
    case GXMChunk::sceGxmSetFrontDepthBias: break;
    case GXMChunk::sceGxmSetFrontDepthFunc: 
      return Serialise_sceGxmSetFrontDepthFunc(ser, 0, (SceGxmDepthFunc)0);
    case GXMChunk::sceGxmSetFrontDepthWriteEnable: 
      return Serialise_sceGxmSetFrontDepthWriteEnable(ser, 0, (SceGxmDepthWriteMode)0);
    case GXMChunk::sceGxmSetFrontFragmentProgramEnable: break;
    case GXMChunk::sceGxmSetFrontLineFillLastPixelEnable: break;
    case GXMChunk::sceGxmSetFrontPointLineWidth: break;
    case GXMChunk::sceGxmSetFrontPolygonMode: break;
    case GXMChunk::sceGxmSetFrontStencilFunc: 
      return Serialise_sceGxmSetFrontStencilFunc(ser, 0, (SceGxmStencilFunc)0, (SceGxmStencilOp)0,
                                                 (SceGxmStencilOp)0, (SceGxmStencilOp)0, 0, 0);
    case GXMChunk::sceGxmSetFrontStencilRef: 
      return Serialise_sceGxmSetFrontStencilRef(ser, 0, 0);
    case GXMChunk::sceGxmSetFrontVisibilityTestEnable: break;
    case GXMChunk::sceGxmSetFrontVisibilityTestIndex: break;
    case GXMChunk::sceGxmSetFrontVisibilityTestOp: break;
    case GXMChunk::sceGxmSetPrecomputedFragmentState: break;
    case GXMChunk::sceGxmSetPrecomputedVertexState: break;
    case GXMChunk::sceGxmSetRegionClip: break;
    case GXMChunk::sceGxmSetTwoSidedEnable: break;
    case GXMChunk::sceGxmSetUniformDataF: 
      return Serialise_sceGxmSetUniformDataF(ser, 0, 0, 0, 0, 0);
    case GXMChunk::sceGxmSetUserMarker: break;
    case GXMChunk::sceGxmSetValidationEnable: break;
    case GXMChunk::sceGxmSetVertexDefaultUniformBuffer: break;
    case GXMChunk::sceGxmSetVertexProgram: 
      return Serialise_sceGxmSetVertexProgram(ser, 0, 0);
    case GXMChunk::sceGxmSetVertexStream: 
      return Serialise_sceGxmSetVertexStream(ser, 0, 0, 0);
    case GXMChunk::sceGxmSetVertexTexture: break;
    case GXMChunk::sceGxmSetVertexUniformBuffer: break;
    case GXMChunk::sceGxmSetViewport: break;
    case GXMChunk::sceGxmSetViewportEnable: break;
    case GXMChunk::sceGxmSetVisibilityBuffer: break;
    case GXMChunk::sceGxmSetWBufferEnable: break;
    case GXMChunk::sceGxmSetWClampEnable: break;
    case GXMChunk::sceGxmSetWClampValue: break;
    case GXMChunk::sceGxmSetWarningEnabled: break;
    case GXMChunk::sceGxmSetYuvProfile: break;
    case GXMChunk::sceGxmShaderPatcherAddRefFragmentProgram: break;
    case GXMChunk::sceGxmShaderPatcherAddRefVertexProgram: break;
    case GXMChunk::sceGxmShaderPatcherCreate: break;
    case GXMChunk::sceGxmShaderPatcherCreateFragmentProgram: break;
    case GXMChunk::sceGxmShaderPatcherCreateMaskUpdateFragmentProgram: break;
    case GXMChunk::sceGxmShaderPatcherCreateVertexProgram: break;
    case GXMChunk::sceGxmShaderPatcherDestroy: break;
    case GXMChunk::sceGxmShaderPatcherForceUnregisterProgram: break;
    case GXMChunk::sceGxmShaderPatcherGetBufferMemAllocated: break;
    case GXMChunk::sceGxmShaderPatcherGetFragmentProgramRefCount: break;
    case GXMChunk::sceGxmShaderPatcherGetFragmentUsseMemAllocated: break;
    case GXMChunk::sceGxmShaderPatcherGetHostMemAllocated: break;
    case GXMChunk::sceGxmShaderPatcherGetProgramFromId: break;
    case GXMChunk::sceGxmShaderPatcherGetUserData: break;
    case GXMChunk::sceGxmShaderPatcherGetVertexProgramRefCount: break;
    case GXMChunk::sceGxmShaderPatcherGetVertexUsseMemAllocated: break;
    case GXMChunk::sceGxmShaderPatcherRegisterProgram: break;
    case GXMChunk::sceGxmShaderPatcherReleaseFragmentProgram: break;
    case GXMChunk::sceGxmShaderPatcherReleaseVertexProgram: break;
    case GXMChunk::sceGxmShaderPatcherSetAuxiliarySurface: break;
    case GXMChunk::sceGxmShaderPatcherSetUserData: break;
    case GXMChunk::sceGxmShaderPatcherUnregisterProgram: break;
    case GXMChunk::sceGxmSyncObjectCreate: break;
    case GXMChunk::sceGxmSyncObjectDestroy: break;
    case GXMChunk::sceGxmTerminate: break;
    case GXMChunk::sceGxmTextureGetData: break;
    case GXMChunk::sceGxmTextureGetFormat: break;
    case GXMChunk::sceGxmTextureGetGammaMode: break;
    case GXMChunk::sceGxmTextureGetHeight: break;
    case GXMChunk::sceGxmTextureGetLodBias: break;
    case GXMChunk::sceGxmTextureGetLodMin: break;
    case GXMChunk::sceGxmTextureGetMagFilter: break;
    case GXMChunk::sceGxmTextureGetMinFilter: break;
    case GXMChunk::sceGxmTextureGetMipFilter: break;
    case GXMChunk::sceGxmTextureGetMipmapCount: break;
    case GXMChunk::sceGxmTextureGetMipmapCountUnsafe: break;
    case GXMChunk::sceGxmTextureGetNormalizeMode: break;
    case GXMChunk::sceGxmTextureGetPalette: break;
    case GXMChunk::sceGxmTextureGetStride: break;
    case GXMChunk::sceGxmTextureGetType: break;
    case GXMChunk::sceGxmTextureGetUAddrMode: break;
    case GXMChunk::sceGxmTextureGetUAddrModeSafe: break;
    case GXMChunk::sceGxmTextureGetVAddrMode: break;
    case GXMChunk::sceGxmTextureGetVAddrModeSafe: break;
    case GXMChunk::sceGxmTextureGetWidth: break;
    case GXMChunk::sceGxmTextureInitCube: break;
    case GXMChunk::sceGxmTextureInitCubeArbitrary: break;
    case GXMChunk::sceGxmTextureInitLinear: break;
    case GXMChunk::sceGxmTextureInitLinearStrided: break;
    case GXMChunk::sceGxmTextureInitSwizzled: break;
    case GXMChunk::sceGxmTextureInitSwizzledArbitrary: break;
    case GXMChunk::sceGxmTextureInitTiled: break;
    case GXMChunk::sceGxmTextureSetData: break;
    case GXMChunk::sceGxmTextureSetFormat: break;
    case GXMChunk::sceGxmTextureSetGammaMode: break;
    case GXMChunk::sceGxmTextureSetHeight: break;
    case GXMChunk::sceGxmTextureSetLodBias: break;
    case GXMChunk::sceGxmTextureSetLodMin: break;
    case GXMChunk::sceGxmTextureSetMagFilter: break;
    case GXMChunk::sceGxmTextureSetMinFilter: break;
    case GXMChunk::sceGxmTextureSetMipFilter: break;
    case GXMChunk::sceGxmTextureSetMipmapCount: break;
    case GXMChunk::sceGxmTextureSetNormalizeMode: break;
    case GXMChunk::sceGxmTextureSetPalette: break;
    case GXMChunk::sceGxmTextureSetStride: break;
    case GXMChunk::sceGxmTextureSetUAddrMode: break;
    case GXMChunk::sceGxmTextureSetUAddrModeSafe: break;
    case GXMChunk::sceGxmTextureSetVAddrMode: break;
    case GXMChunk::sceGxmTextureSetVAddrModeSafe: break;
    case GXMChunk::sceGxmTextureSetWidth: break;
    case GXMChunk::sceGxmTextureValidate: break;
    case GXMChunk::sceGxmTransferCopy: break;
    case GXMChunk::sceGxmTransferDownscale: break;
    case GXMChunk::sceGxmTransferFill: break;
    case GXMChunk::sceGxmTransferFinish: break;
    case GXMChunk::sceGxmUnmapFragmentUsseMemory: break;
    case GXMChunk::sceGxmUnmapMemory: break;
    case GXMChunk::sceGxmUnmapVertexUsseMemory: break;
    case GXMChunk::sceGxmVertexFence: break;
    case GXMChunk::sceGxmVertexProgramGetProgram: break;
    case GXMChunk::sceGxmWaitEvent: break;
    case GXMChunk::Max: break;
    default: break;
  }
  return false;
}

WrappedGXM::WrappedGXM() 
{
  m_Replay = new GXMReplay(this);

  if(RenderDoc::Inst().IsReplayApp())
  {
    m_State = CaptureState::LoadingReplaying;
  }
  else
  {
    m_State = CaptureState::BackgroundCapturing;
  }

  m_CurEventID = 0;
}

WrappedGXM::~WrappedGXM() 
{
}
