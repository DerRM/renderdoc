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
#include <algorithm>
#include "common/common.h"
#include "serialise/rdcfile.h"

#include "gxm_replay.h"
#include "gxm_vk_shader_cache.h"

void WrappedGXM::StartFrameCapture(void *dev, void *wnd) {}

bool WrappedGXM::EndFrameCapture(void *dev, void *wnd)
{
  return false;
}

bool WrappedGXM::DiscardFrameCapture(void *dev, void *wnd)
{
  return false;
}

rdcstr WrappedGXM::GetChunkName(uint32_t idx)
{
  if((SystemChunk)idx < SystemChunk::FirstDriverChunk)
    return ToStr((SystemChunk)idx);

  return ToStr((GXMChunk)idx);
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

  ser.ConfigureStructuredExport(&GetChunkName, storeStructuredBuffers);

  m_StructuredFile = &ser.GetStructuredFile();

  int chunkIdx = 0;

  struct chunkinfo
  {
    chunkinfo() : count(0), totalsize(0), total(0.0) {}
    int count;
    uint64_t totalsize;
    double total;
  };

  std::map<GXMChunk, chunkinfo> chunkInfos;

  SCOPED_TIMER("chunk initialisation");

  uint64_t frameDataSize = 0;

  for(;;)
  {
    uint64_t offsetStart = reader->GetOffset();

    GXMChunk context = ser.ReadChunk<GXMChunk>();

    chunkIdx++;

    if(ser.GetReader()->IsErrored())
      return ReplayStatus::APIDataCorrupted;

    m_ChunkMetadata = ser.ChunkMetadata();

    bool success = ProcessChunk(ser, context);

    ser.EndChunk();

    if(ser.GetReader()->IsErrored())
      return ReplayStatus::APIDataCorrupted;

    if(!success)
      return ReplayStatus::APIReplayFailed;

    uint64_t offsetEnd = reader->GetOffset();

    if((SystemChunk)context == SystemChunk::CaptureScope)
    {
      GetReplay()->WriteFrameRecord().frameInfo.fileOffset = offsetStart;

      frameDataSize = reader->GetSize() - reader->GetOffset();

      m_FrameReader = new StreamReader(reader, frameDataSize);

      ReplayStatus status = ContextReplayLog(m_State, 0, 0, false);

      if(status != ReplayStatus::Succeeded)
        return status;
    }

    chunkInfos[context].totalsize += offsetEnd - offsetStart;
    chunkInfos[context].count++;

    if((SystemChunk)context == SystemChunk::CaptureScope || reader->IsErrored() || reader->AtEnd())
      break;
  }

  m_StructuredFile->Swap(m_StoredStructuredData);

  m_StructuredFile = &m_StoredStructuredData;

  m_Replay->WriteFrameRecord().drawcallList = m_ParentDrawcall.children;

  return ReplayStatus::Succeeded;
}

void WrappedGXM::AddDrawcall(const DrawcallDescription &d)
{
  m_AddedDrawcall = true;

  DrawcallDescription draw = d;
  draw.eventId = m_CurEventID;
  draw.drawcallId = m_CurDrawcallID;

  {
    RDCEraseEl(draw.outputs);

    draw.outputs[0] =
        GetResourceManager()->GetOriginalID(GetResourceManager()->GetID(FramebufferRes()));
  }

  m_CurDrawcallID++;

  draw.events = m_CurEvents;
  m_CurEvents.clear();

  m_DrawcallStack.back()->children.push_back(draw);
}

void WrappedGXM::AddEvent()
{
  APIEvent apievent;

  apievent.fileOffset = m_CurChunkOffset;
  apievent.eventId = m_CurEventID;

  apievent.chunkIndex = uint32_t(m_StructuredFile->chunks.size() - 1);
  apievent.callstack = m_ChunkMetadata.callstack;

  m_CurEvents.push_back(apievent);

  if(IsLoading(m_State))
  {
    m_Events.resize(apievent.eventId + 1);
    m_Events[apievent.eventId] = apievent;
  }
}

void WrappedGXM::AddResource(ResourceId id, ResourceType type, const char *defaultNamePrefix)
{
  ResourceDescription &descr = GetReplay()->GetResourceDesc(id);
  descr.resourceId = id;

  uint64_t num;
  memcpy(&num, &id, sizeof(uint64_t));
  descr.name = defaultNamePrefix + (" " + ToStr(num));
  descr.autogeneratedName = true;
  descr.type = type;
  AddResourceCurChunk(descr);
}

void WrappedGXM::AddResourceCurChunk(ResourceDescription &descr)
{
  descr.initialisationChunks.push_back((uint32_t)m_StructuredFile->chunks.size() - 1);
}

void WrappedGXM::AddResourceCurChunk(ResourceId id)
{
  AddResourceCurChunk(GetReplay()->GetResourceDesc(id));
}

template <typename SerialiserType>
bool WrappedGXM::Serialise_ContextConfiguration(SerialiserType &ser, void *ctx)
{
  uint32_t base;
  SERIALISE_ELEMENT(base);

  GXMInitParams params = {};
  SERIALISE_ELEMENT(params.width);
  SERIALISE_ELEMENT(params.height);
  SERIALISE_ELEMENT(params.pitch);
  SERIALISE_ELEMENT(params.pixelformat);
  ResourceId framebuffer;
  SERIALISE_ELEMENT(framebuffer);

  if(!GetResourceManager()->HasLiveResource(framebuffer))
  {
    GXMResource res = FramebufferRes();
    m_ResourceManager->RegisterResource(res);
    GetResourceManager()->AddLiveResource(framebuffer, res);

    AddResource(framebuffer, ResourceType::SwapchainImage, "Backbuffer Color");
  }

  (void)params;

  return true;
}

uint32_t WrappedGXM::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(m_vulkanState.m_Gpu, &memProperties);
  for(uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
  {
    if((typeFilter & (1 << i)) &&
       (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
    {
      return i;
    }
  }

  return 0;
}

template <typename SerialiserType>
bool WrappedGXM::Serialise_InitBufferResource(SerialiserType &ser)
{
  GXMBufferType type;
  SERIALISE_ELEMENT(type);
  uint32_t addr;
  SERIALISE_ELEMENT(addr);
  uint32_t size;
  SERIALISE_ELEMENT(size);
  ResourceId buffer;
  SERIALISE_ELEMENT(buffer);

  SERIALISE_CHECK_READ_ERRORS();

  if(IsReplayingAndReading())
  {
    if(type == GXMBufferType::SceGxmMappedBuffer)
    {
      AddResource(buffer, ResourceType::Buffer, "Mapped Buffer");
    }
    else
    {
      GXMResource res = GetResourceManager()->FindInBuffer(size, addr);
      ResourceId liveid = GetResourceManager()->GetID(res);
      ResourceId origId = GetResourceManager()->GetOriginalID(liveid);

      GetReplay()->GetResourceDesc(origId).derivedResources.push_back(buffer);
      GetReplay()->GetResourceDesc(buffer).parentResources.push_back(origId);

      VkBufferCreateInfo bufferInfo = {};
      bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
      bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
      bufferInfo.size = size;
      if(type == GXMBufferType::SceGxmIndexBuffer)
        bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
      else if(type == GXMBufferType::SceGxmVertexBuffer)
        bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

      VkBuffer vkbuffer;
      VkResult result = vkCreateBuffer(m_vulkanState.m_Device, &bufferInfo, NULL, &vkbuffer);
      if(result != VK_SUCCESS)
      {
        RDCERR("vkCreateBuffer failed");
      }

      VkMemoryRequirements memReqs;
      vkGetBufferMemoryRequirements(m_vulkanState.m_Device, vkbuffer, &memReqs);

      VkMemoryAllocateInfo allocateInfo = {};
      allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
      allocateInfo.allocationSize = memReqs.size;
      allocateInfo.memoryTypeIndex =
          findMemoryType(memReqs.memoryTypeBits,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

      VkDeviceMemory bufferMemory;
      result = vkAllocateMemory(m_vulkanState.m_Device, &allocateInfo, NULL, &bufferMemory);
      if(result != VK_SUCCESS)
      {
        RDCERR("vkAllocateMemory failed");
      }

      result = vkBindBufferMemory(m_vulkanState.m_Device, vkbuffer, bufferMemory, 0);
      if(result != VK_SUCCESS)
      {
        RDCERR("vkBindBufferMemory failed");
      }

      BufferDescription desc = {};

      if(type == GXMBufferType::SceGxmIndexBuffer)
      {
        GXMResource vertex_res = IndexBufferRes(addr, size, vkbuffer, bufferMemory);
        ResourceId live = m_ResourceManager->RegisterResource(vertex_res);
        GetResourceManager()->AddLiveResource(buffer, vertex_res);
        AddResource(buffer, ResourceType::Buffer, "Index Buffer");
        desc.creationFlags = BufferCategory::Index;
      }
      else if(type == GXMBufferType::SceGxmVertexBuffer)
      {
        GXMResource index_res = VertexBufferRes(addr, size, vkbuffer, bufferMemory);
        ResourceId live = m_ResourceManager->RegisterResource(index_res);
        GetResourceManager()->AddLiveResource(buffer, index_res);
        AddResource(buffer, ResourceType::Buffer, "Vertex Buffer");
        desc.creationFlags = BufferCategory::Vertex;
      }

      desc.resourceId = buffer;
      desc.length = size;
      m_buffers.push_back(desc);
    }
  }

  return true;
}

template <typename SerialiserType>
bool WrappedGXM::Serialise_InitShaderResources(SerialiserType &ser)
{
  ResourceId shader;
  SERIALISE_ELEMENT(shader);

  uint32_t program_size;
  SERIALISE_ELEMENT(program_size);

  const void *programData;
  SERIALISE_ELEMENT_ARRAY(programData, program_size + (4 - (program_size % 4)));
  rdcarray<uint8_t> data;
  data.assign((const uint8_t*)programData, program_size + (4 - (program_size % 4)));
  m_shaders.push_back(data);

  return true;
}

template <typename SerialiserType>
bool WrappedGXM::Serialise_CaptureScope(SerialiserType &ser)
{
  uint32_t framecount;
  SERIALISE_ELEMENT(framecount);
  return true;
}

bool WrappedGXM::ProcessChunk(ReadSerialiser &ser, GXMChunk chunk)
{
  m_AddedDrawcall = false;

  SystemChunk system = (SystemChunk)chunk;
  if(system == SystemChunk::CaptureScope)
  {
    return Serialise_CaptureScope(ser);
  }
  else if(system == SystemChunk::CaptureEnd)
  {
    return true;
  }

  switch(chunk)
  {
    case GXMChunk::DeviceInitialisation: break;
    case GXMChunk::MakeContextCurrent: break;
    case GXMChunk::vrapi_CreateTextureSwapChain: break;
    case GXMChunk::vrapi_CreateTextureSwapChain2: break;
    case GXMChunk::sceGxmAddRazorGpuCaptureBuffer: break;
    case GXMChunk::sceGxmBeginCommandList: break;
    case GXMChunk::sceGxmBeginScene: return Serialise_sceGxmBeginScene(ser, 0, 0, 0, 0, 0, 0, 0, 0);
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
    case GXMChunk::sceGxmEndScene: return Serialise_sceGxmEndScene(ser, 0, 0, 0);
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
    case GXMChunk::sceGxmInitialize: return Serialise_sceGxmInitialize(ser, 0);
    case GXMChunk::sceGxmIsDebugVersion: break;
    case GXMChunk::sceGxmMapFragmentUsseMemory: break;
    case GXMChunk::sceGxmMapMemory: break;
    case GXMChunk::sceGxmMapVertexUsseMemory: break;
    case GXMChunk::sceGxmMidSceneFlush: break;
    case GXMChunk::sceGxmNotificationWait: break;
    case GXMChunk::sceGxmPadHeartbeat: return Serialise_sceGxmPadHeartbeat(ser, 0, 0);
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
    case GXMChunk::sceGxmSetFragmentProgram: return Serialise_sceGxmSetFragmentProgram(ser, 0, 0);
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
    case GXMChunk::sceGxmSetFrontStencilRef: return Serialise_sceGxmSetFrontStencilRef(ser, 0, 0);
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
    case GXMChunk::sceGxmSetVertexProgram: return Serialise_sceGxmSetVertexProgram(ser, 0, 0);
    case GXMChunk::sceGxmSetVertexStream: return Serialise_sceGxmSetVertexStream(ser, 0, 0, 0);
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
    case GXMChunk::ContextConfiguration: return Serialise_ContextConfiguration(ser, NULL);
    case GXMChunk::InitBufferResources: return Serialise_InitBufferResource(ser);
    case GXMChunk::InitShaderResources: return Serialise_InitShaderResources(ser);
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
  m_CurDrawcallID = 0;
  m_CurChunkOffset = 0;

  m_StructuredFile = &m_StoredStructuredData;
  m_AddedDrawcall = false;

  m_DrawcallStack.push_back(&m_ParentDrawcall);

  CaptureState state = CaptureState::LoadingReplaying;

  m_ResourceManager = new GXMResourceManager(state);

  m_vulkanState.m_Instance = VK_NULL_HANDLE;
}

WrappedGXM::~WrappedGXM()
{
  SAFE_DELETE(m_FrameReader);
}

VkBool32 WrappedGXM::DebugCallback(MessageSeverity severity, MessageCategory category,
                                   int messageCode, const char *pMessageId, const char *pMessage)
{
  {
    // ignore perf warnings
    if(category == MessageCategory::Performance)
      return false;

    // "Non-linear image is aliased with linear buffer"
    // Not an error, the validation layers complain at our whole-mem bufs
    if(strstr(pMessageId, "InvalidAliasing") || strstr(pMessage, "InvalidAliasing"))
      return false;

    // "vkCreateSwapchainKHR() called with imageExtent, which is outside the bounds returned by
    // vkGetPhysicalDeviceSurfaceCapabilitiesKHR(): currentExtent"
    // This is quite racey, the currentExtent can change in between us checking it and the valiation
    // layers checking it. We handle out of date, so this is likely fine.
    if(strstr(pMessageId, "VUID-VkSwapchainCreateInfoKHR-imageExtent"))
      return false;

    RDCWARN("[%s] %s", pMessageId, pMessage);
  }

  return false;
}

VkBool32 VKAPI_PTR WrappedGXM::DebugUtilsCallbackStatic(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
{
  MessageSeverity severity = MessageSeverity::Low;

  if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    severity = MessageSeverity::High;
  else if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    severity = MessageSeverity::Medium;
  else if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
    severity = MessageSeverity::Low;
  else if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
    severity = MessageSeverity::Info;

  MessageCategory category = MessageCategory::Miscellaneous;

  if(messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
    category = MessageCategory::Performance;

  rdcstr msgid;

  const char *pMessageId = pCallbackData->pMessageIdName;
  int messageCode = pCallbackData->messageIdNumber;

  if(messageCode == 0 && pMessageId && !strncmp(pMessageId, "VUID", 4))
  {
    const char *c = pMessageId + strlen(pMessageId) - 1;
    int mult = 1;

    while(c > pMessageId && *c >= '0' && *c <= '9')
    {
      messageCode += mult * int(*c - '0');
      mult *= 10;
      c--;
    }
  }

  if(!pMessageId)
  {
    msgid = StringFormat::Fmt("%d", pCallbackData->messageIdNumber);
    pMessageId = msgid.c_str();
  }

  return ((WrappedGXM *)pUserData)
      ->DebugCallback(severity, category, messageCode, pMessageId, pCallbackData->pMessage);
}

rdcarray<BufferDescription> WrappedGXM::GetBuffers()
{
  return m_buffers;
}

rdcarray<rdcarray<uint8_t>> WrappedGXM::GetShaders()
{
  return m_shaders;
}

ReplayStatus WrappedGXM::Initialise()
{
  uint32_t count = 0;

  vkEnumerateInstanceExtensionProperties(NULL, &count, NULL);

  rdcarray<VkExtensionProperties> instExtensions(count);

  vkEnumerateInstanceExtensionProperties(NULL, &count, &instExtensions[0]);

  std::set<rdcstr> supportedExtensions;

  for(uint32_t extIndex = 0; extIndex < count; ++extIndex)
  {
    supportedExtensions.insert(instExtensions[extIndex].extensionName);
  }

  rdcarray<rdcstr> enabledExtensions;
  AddRequiredExtensions(true, enabledExtensions, supportedExtensions);

  if(supportedExtensions.find(VK_EXT_DEBUG_UTILS_EXTENSION_NAME) != supportedExtensions.end() &&
     !enabledExtensions.contains(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
  {
    if(!m_Replay->IsRemoteProxy())
      RDCLOG("Enabling VK_EXT_debug_utils");
    enabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }
  else if(supportedExtensions.find(VK_EXT_DEBUG_REPORT_EXTENSION_NAME) != supportedExtensions.end() &&
          !enabledExtensions.contains(VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
  {
    if(!m_Replay->IsRemoteProxy())
      RDCLOG("Enabling VK_EXT_debug_report");
    enabledExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
  }

  rdcarray<const char *> strExtensions(enabledExtensions.size());
  for(uint32_t strExtIndex = 0; strExtIndex < enabledExtensions.size(); ++strExtIndex)
  {
    strExtensions[strExtIndex] = enabledExtensions[strExtIndex].c_str();
  }

  std::set<rdcstr> supportedLayers;

  vkEnumerateInstanceLayerProperties(&count, NULL);

  rdcarray<VkLayerProperties> layerprops(count);
  vkEnumerateInstanceLayerProperties(&count, &layerprops[0]);

  for(uint32_t e = 0; e < count; e++)
    supportedLayers.insert(layerprops[e].layerName);

  const char KhronosValidation[] = "VK_LAYER_KHRONOS_validation";
  const char LunarGValidation[] = "VK_LAYER_LUNARG_standard_validation";

  rdcarray<rdcstr> enabledLayers;

  if(supportedLayers.find(KhronosValidation) != supportedLayers.end())
  {
    RDCLOG("Enabling %s layer for API validation", KhronosValidation);
    // params.Layers.push_back(KhronosValidation);
    // m_LayersEnabled[VkCheckLayer_unique_objects] = true;
    enabledLayers.push_back(KhronosValidation);
  }
  else if(supportedLayers.find(LunarGValidation) != supportedLayers.end())
  {
    RDCLOG("Enabling %s layer for API validation", LunarGValidation);
    // params.Layers.push_back(LunarGValidation);
    // m_LayersEnabled[VkCheckLayer_unique_objects] = true;
    enabledLayers.push_back(KhronosValidation);
  }
  else
  {
    RDCLOG("API validation layers are not available, check you have the Vulkan SDK installed");
  }

  rdcarray<const char *> strLayers(enabledLayers.size());

  for(uint32_t strLayerIndex = 0; strLayerIndex < enabledLayers.size(); ++strLayerIndex)
  {
    strLayers[strLayerIndex] = enabledLayers[strLayerIndex].c_str();
  }

  VkApplicationInfo appinfo = {
      VK_STRUCTURE_TYPE_APPLICATION_INFO, NULL, "GXM Driver", 1, "GXM", 1, VK_MAKE_VERSION(1, 0, 0),
  };

  VkInstanceCreateInfo instinfo = {
      VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      NULL,
      0,
      &appinfo,
      (uint32_t)enabledLayers.size(),
      &strLayers[0],
      (uint32_t)enabledExtensions.size(),
      &strExtensions[0],
  };

  vkCreateInstance(&instinfo, NULL, &m_vulkanState.m_Instance);

  InitialiseVulkanInstanceCalls(m_vulkanState.m_Instance);

  VkDebugUtilsMessengerCreateInfoEXT debugInfo = {};
  debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  debugInfo.pfnUserCallback = &DebugUtilsCallbackStatic;
  debugInfo.pUserData = this;
  debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                          VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                          VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

  vkCreateDebugUtilsMessengerEXT(m_vulkanState.m_Instance, &debugInfo, NULL,
                                 &m_vulkanState.m_DbgUtilsCallback);

  vkEnumeratePhysicalDevices(m_vulkanState.m_Instance, &count, NULL);

  rdcarray<VkPhysicalDevice> gpus(count);

  vkEnumeratePhysicalDevices(m_vulkanState.m_Instance, &count, &gpus[0]);

  m_vulkanState.m_Gpu = gpus[0];

  vkGetPhysicalDeviceFeatures(m_vulkanState.m_Gpu, &m_vulkanState.m_DeviceFeatures);
  vkGetPhysicalDeviceProperties(m_vulkanState.m_Gpu, &m_vulkanState.m_DeviceProperties);

  vkGetPhysicalDeviceQueueFamilyProperties(m_vulkanState.m_Gpu, &count, NULL);

  rdcarray<VkQueueFamilyProperties> queues(count);

  vkGetPhysicalDeviceQueueFamilyProperties(m_vulkanState.m_Gpu, &count, &queues[0]);

  for(uint32_t queueIndex = 0; queueIndex < queues.size(); ++queueIndex)
  {
    if(queues[queueIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT)
    {
      m_vulkanState.m_QueueFamilyIndex = queueIndex;
      break;
    }
  }

  float prio = 1.0f;

  VkDeviceQueueCreateInfo queueinfo = {
      VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, NULL, 0,
      m_vulkanState.m_QueueFamilyIndex,           1,    &prio,
  };

  vkEnumerateDeviceExtensionProperties(m_vulkanState.m_Gpu, NULL, &count, NULL);

  rdcarray<VkExtensionProperties> deviceExtensions(count);

  vkEnumerateDeviceExtensionProperties(m_vulkanState.m_Gpu, NULL, &count, &deviceExtensions[0]);

  std::set<rdcstr> supportedDeviceExtensions;

  for(uint32_t extIndex = 0; extIndex < count; ++extIndex)
  {
    supportedDeviceExtensions.insert(deviceExtensions[extIndex].extensionName);
  }

  rdcarray<rdcstr> enabledDeviceExtensions;
  AddRequiredExtensions(false, enabledDeviceExtensions, supportedDeviceExtensions);

  rdcarray<const char *> strDeviceExtensions(enabledDeviceExtensions.size());
  for(uint32_t strExtIndex = 0; strExtIndex < enabledDeviceExtensions.size(); ++strExtIndex)
  {
    strDeviceExtensions[strExtIndex] = enabledDeviceExtensions[strExtIndex].c_str();
  }

  VkPhysicalDeviceFeatures enabledFeatures = {};

  if(m_vulkanState.m_DeviceFeatures.depthClamp)
    enabledFeatures.depthClamp = true;
  if(m_vulkanState.m_DeviceFeatures.fillModeNonSolid)
    enabledFeatures.fillModeNonSolid = true;
  if(m_vulkanState.m_DeviceFeatures.geometryShader)
    enabledFeatures.geometryShader = true;

  VkDeviceCreateInfo deviceinfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                                   NULL,
                                   0,
                                   1,
                                   &queueinfo,
                                   0,
                                   NULL,
                                   (uint32_t)enabledDeviceExtensions.size(),
                                   &strDeviceExtensions[0],
                                   &enabledFeatures};

  m_vulkanState.m_DeviceFeatures = enabledFeatures;

  vkCreateDevice(m_vulkanState.m_Gpu, &deviceinfo, NULL, &m_vulkanState.m_Device);

  InitialiseVulkanDeviceCalls(m_vulkanState.m_Device);

  vkGetDeviceQueue(m_vulkanState.m_Device, m_vulkanState.m_QueueFamilyIndex, 0,
                   &m_vulkanState.m_Queue);

  vkGetPhysicalDeviceMemoryProperties(m_vulkanState.m_Gpu, &m_PhysicalDeviceData.memProps);

  m_InternalCmds.Reset();

  if(m_InternalCmds.cmdpool == VK_NULL_HANDLE)
  {
    VkCommandPoolCreateInfo poolInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, NULL,
                                        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                                        m_vulkanState.m_QueueFamilyIndex};
    VkResult vkr =
        vkCreateCommandPool(m_vulkanState.m_Device, &poolInfo, NULL, &m_InternalCmds.cmdpool);
    RDCASSERTEQUAL(vkr, VK_SUCCESS);
  }

  m_PhysicalDeviceData.readbackMemIndex =
      m_PhysicalDeviceData.GetMemoryIndex(~0U, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 0);
  m_PhysicalDeviceData.uploadMemIndex =
      m_PhysicalDeviceData.GetMemoryIndex(~0U, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 0);
  m_PhysicalDeviceData.GPULocalMemIndex = m_PhysicalDeviceData.GetMemoryIndex(
      ~0U, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

  m_DebugManager = new GXMDebugManager(this);
  m_ShaderCache = new GXMVkShaderCache(this);

  GetReplay()->m_General.Init(this, VK_NULL_HANDLE);
  GetReplay()->m_MeshRender.Init(this, GetReplay()->m_General.DescriptorPool);
  GetReplay()->m_Overlay.Init(this, GetReplay()->m_General.DescriptorPool);

  return ReplayStatus::Succeeded;
}

uint32_t WrappedGXM::GetReadbackMemoryIndex(uint32_t resourceRequiredBitmask)
{
  if(resourceRequiredBitmask & (1 << m_PhysicalDeviceData.readbackMemIndex))
    return m_PhysicalDeviceData.readbackMemIndex;

  return m_PhysicalDeviceData.GetMemoryIndex(resourceRequiredBitmask,
                                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 0);
}

uint32_t WrappedGXM::GetUploadMemoryIndex(uint32_t resourceRequiredBitmask)
{
  if(resourceRequiredBitmask & (1 << m_PhysicalDeviceData.uploadMemIndex))
    return m_PhysicalDeviceData.uploadMemIndex;

  return m_PhysicalDeviceData.GetMemoryIndex(resourceRequiredBitmask,
                                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 0);
}

uint32_t WrappedGXM::GetGPULocalMemoryIndex(uint32_t resourceRequiredBitmask)
{
  if(resourceRequiredBitmask & (1 << m_PhysicalDeviceData.GPULocalMemIndex))
    return m_PhysicalDeviceData.GPULocalMemIndex;

  return m_PhysicalDeviceData.GetMemoryIndex(resourceRequiredBitmask,
                                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
}

uint32_t WrappedGXM::PhysicalDeviceData::GetMemoryIndex(uint32_t resourceRequiredBitmask,
                                                        uint32_t allocRequiredProps,
                                                        uint32_t allocUndesiredProps)
{
  uint32_t best = memProps.memoryTypeCount;

  for(uint32_t memIndex = 0; memIndex < memProps.memoryTypeCount; memIndex++)
  {
    if(resourceRequiredBitmask & (1 << memIndex))
    {
      uint32_t memTypeFlags = memProps.memoryTypes[memIndex].propertyFlags;

      if((memTypeFlags & allocRequiredProps) == allocRequiredProps)
      {
        if(memTypeFlags & allocUndesiredProps)
          best = memIndex;
        else
          return memIndex;
      }
    }
  }

  if(best == memProps.memoryTypeCount)
  {
    RDCERR("Couldn't find any matching heap! requirements %x / %x too strict",
           resourceRequiredBitmask, allocRequiredProps);
    return 0;
  }
  return best;
}

VkCommandBuffer WrappedGXM::GetNextCmd()
{
  VkCommandBuffer ret;

  if(!m_InternalCmds.freecmds.empty())
  {
    ret = m_InternalCmds.freecmds.back();
    m_InternalCmds.freecmds.pop_back();

    vkResetCommandBuffer(ret, 0);
  }
  else
  {
    VkCommandBufferAllocateInfo cmdInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        NULL,
        m_InternalCmds.cmdpool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        1,
    };
    VkResult vkr = vkAllocateCommandBuffers(m_vulkanState.m_Device, &cmdInfo, &ret);
    RDCASSERTEQUAL(vkr, VK_SUCCESS);
  }

  m_InternalCmds.pendingcmds.push_back(ret);

  return ret;
}

void WrappedGXM::SubmitCmds()
{
  // nothing to do
  if(m_InternalCmds.pendingcmds.empty())
    return;

  rdcarray<VkCommandBuffer> cmds = m_InternalCmds.pendingcmds;

  VkSubmitInfo submitInfo = {
      VK_STRUCTURE_TYPE_SUBMIT_INFO,
      NULL,
      0,
      NULL,
      NULL,
      (uint32_t)cmds.size(),
      &cmds[0],    // command buffers
      0,
      NULL,    // signal semaphores
  };

  // we might have work to do (e.g. debug manager creation command buffer) but
  // no queue, if the device is destroyed immediately. In this case we can just
  // skip the submit
  if(m_vulkanState.m_Queue != VK_NULL_HANDLE)
  {
    VkResult vkr = vkQueueSubmit(m_vulkanState.m_Queue, 1, &submitInfo, VK_NULL_HANDLE);
    RDCASSERTEQUAL(vkr, VK_SUCCESS);
  }

#if ENABLED(SINGLE_FLUSH_VALIDATE)
  FlushQ();
#endif

  m_InternalCmds.submittedcmds.append(m_InternalCmds.pendingcmds);
  m_InternalCmds.pendingcmds.clear();
}

void WrappedGXM::FlushQ()
{
  // VKTODOLOW could do away with the need for this function by keeping
  // commands until N presents later, or something, or checking on fences.
  // If we do so, then check each use for FlushQ to see if it needs a
  // CPU-GPU sync or whether it is just looking to recycle command buffers
  // (Particularly the one in vkQueuePresentKHR drawing the overlay)

  // see comment in SubmitQ()
  if(m_vulkanState.m_Queue != VK_NULL_HANDLE)
  {
    VkResult vkr = vkQueueWaitIdle(m_vulkanState.m_Queue);
    RDCASSERTEQUAL(vkr, VK_SUCCESS);
  }

#if ENABLED(SINGLE_FLUSH_VALIDATE)
  if(m_Device != VK_NULL_HANDLE)
  {
    ObjDisp(m_Device)->DeviceWaitIdle(Unwrap(m_Device));
    VkResult vkr = ObjDisp(m_Device)->DeviceWaitIdle(Unwrap(m_Device));
    RDCASSERTEQUAL(vkr, VK_SUCCESS);
  }
#endif

  if(!m_InternalCmds.submittedcmds.empty())
  {
    m_InternalCmds.freecmds.append(m_InternalCmds.submittedcmds);
    m_InternalCmds.submittedcmds.clear();
  }

  if(!m_InternalCmds.submittedsems.empty())
  {
    m_InternalCmds.freesems.append(m_InternalCmds.submittedsems);
    m_InternalCmds.submittedsems.clear();
  }
}

void WrappedGXM::ReplayLog(uint32_t startEventID, uint32_t endEventID, ReplayLogType replayType)
{
  bool partial = true;

  if(startEventID == 0 && (replayType == eReplay_WithoutDraw || replayType == eReplay_Full))
  {
    startEventID = 1;
    partial = false;
  }

  m_State = CaptureState::ActiveReplaying;

  ReplayStatus status = ReplayStatus::Succeeded;

  if(replayType == eReplay_Full)
    status = ContextReplayLog(m_State, startEventID, endEventID, partial);
  else if(replayType == eReplay_WithoutDraw)
    status = ContextReplayLog(m_State, startEventID, RDCMAX(1U, endEventID) - 1, partial);
  else if(replayType == eReplay_OnlyDraw)
    status = ContextReplayLog(m_State, endEventID, endEventID, partial);
  else
    RDCFATAL("Unexpected replay type");

  RDCASSERTEQUAL(status, ReplayStatus::Succeeded);
}

ReplayStatus WrappedGXM::ContextReplayLog(CaptureState readType, uint32_t startEventID,
                                          uint32_t endEventID, bool partial)
{
  m_FrameReader->SetOffset(0);

  ReadSerialiser ser(m_FrameReader, Ownership::Nothing);

  SDFile *prevFile = m_StructuredFile;

  if(IsLoading(m_State) || IsStructuredExporting(m_State))
  {
    ser.ConfigureStructuredExport(&GetChunkName, IsStructuredExporting(m_State));

    ser.GetStructuredFile().Swap(*m_StructuredFile);

    m_StructuredFile = &ser.GetStructuredFile();
  }

  SystemChunk header = ser.ReadChunk<SystemChunk>();
  RDCASSERTEQUAL(header, SystemChunk::CaptureBegin);

  if(partial)
    ser.SkipCurrentChunk();

  ser.EndChunk();

  m_CurEvents.clear();

  if(IsActiveReplaying(m_State))
  {
    size_t idx = startEventID;
    while(idx < m_Events.size() - 1 && m_Events[idx].eventId == 0)
      idx++;

    APIEvent &ev = m_Events[RDCMIN(idx, m_Events.size() - 1)];

    m_CurEventID = ev.eventId;
    if(partial)
      ser.GetReader()->SetOffset(ev.fileOffset);
  }
  else
  {
    m_CurEventID = 1;
    m_CurDrawcallID = 1;
  }

  for(;;)
  {
    if(IsActiveReplaying(m_State) && m_CurEventID > endEventID)
    {
      // we can just break out if we've done all the events desired.
      break;
    }

    m_CurChunkOffset = ser.GetReader()->GetOffset();

    GXMChunk chunktype = ser.ReadChunk<GXMChunk>();

    if(ser.GetReader()->IsErrored())
      return ReplayStatus::APIDataCorrupted;

    bool success = ProcessChunk(ser, chunktype);

    ser.EndChunk();

    if(ser.GetReader()->IsErrored())
      return ReplayStatus::APIDataCorrupted;

    if(!success)
      return ReplayStatus::APIReplayFailed;    // TODO use variable

    if((SystemChunk)chunktype == SystemChunk::CaptureEnd)
      break;

    if(IsLoading(m_State))
    {
      if(!m_AddedDrawcall)
        AddEvent();
    }

    m_CurEventID++;
  }

  // swap the structure back now that we've accumulated the frame as well.
  if(IsLoading(m_State) || IsStructuredExporting(m_State))
    ser.GetStructuredFile().Swap(*prevFile);

  m_StructuredFile = prevFile;

  if(IsLoading(m_State))
  {
    // GetReplay()->WriteFrameRecord().drawcallList = m_ParentDrawcall.children;

    // SetupDrawcallPointers(m_DrawcallStack, GetReplay()->WriteFrameRecord().drawcallList);
  }

  return ReplayStatus::Succeeded;
}
