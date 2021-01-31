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

#include "gxm_replay.h"
#include "common/common.h"
#include "maths/camera.h"
#include "maths/formatpacking.h"
#include "maths/matrix.h"
#include "serialise/rdcfile.h"
#include "serialise/serialiser.h"
#include "strings/string_utils.h"
#include "gxm_debug.h"
#include "gxm_driver.h"

#define VULKAN 1
#include "data/glsl/glsl_ubos_cpp.h"

ReplayStatus GXMReplay::ReadLogInitialisation(RDCFile *rdc, bool storeStructuredBuffers)
{
  return m_pDriver->ReadLogInitialisation(rdc, storeStructuredBuffers);
}

void GXMReplay::ReplayLog(uint32_t endEventID, ReplayLogType replayType)
{
  m_pDriver->ReplayLog(0, endEventID, replayType);
}

const SDFile &GXMReplay::GetStructuredFile()
{
  return m_pDriver->GetStructuredFile();
}

rdcarray<uint32_t> GXMReplay::GetPassEvents(uint32_t eventId)
{
  return rdcarray<uint32_t>();
}

void GXMReplay::InitPostVSBuffers(uint32_t eventId) {}

void GXMReplay::InitPostVSBuffers(const rdcarray<uint32_t> &passEvents) {}

ResourceId GXMReplay::GetLiveID(ResourceId id)
{
  if(!m_pDriver->GetResourceManager()->HasLiveResource(id))
    return ResourceId();
  return m_pDriver->GetResourceManager()->GetLiveID(id);
}

MeshFormat GXMReplay::GetPostVSBuffers(uint32_t eventId, uint32_t instID, uint32_t viewID,
                                       MeshDataStage stage)
{
  return MeshFormat();
}

void GXMReplay::GetBufferData(ResourceId buff, uint64_t offset, uint64_t len, bytebuf &retData)
{
  GXMResource res = m_pDriver->GetResourceManager()->GetCurrentResource(buff);

  if(res.buffer == VK_NULL_HANDLE)
  {
    RDCERR("Getting buffer data for unknown buffer/memory %s!", ToStr(buff).c_str());
    return;
  }

  if(offset >= res.size)
  {
    return;
  }

  if(len == 0)
  {
    len = res.size - offset;
  }

  if(VkDeviceSize(offset + len) > res.size)
  {
    RDCWARN("Attempting to read off the end of the buffer (%llu %llu). Will be clamped (%u)",
            offset, len, res.size);
    len = RDCMIN(len, res.size - offset);
  }

  retData.resize((size_t)len);

  ResourceId resId = GetResourceDesc(m_pDriver->GetResourceManager()->GetOriginalID(buff)).parentResources[0];

  GXMResource mapped_res =
      m_pDriver->GetResourceManager()->GetLiveResource(resId);

  if(res.memory == VK_NULL_HANDLE)
  {
    RDCERR("Getting memory for buffer failed");
    return;
  }

  byte *pData = NULL;
  vkMapMemory(m_pDriver->m_vulkanState.m_Device, mapped_res.memory, res.addr - mapped_res.addr + offset, len, 0, (void **)&pData);
  memcpy(&retData[0], pData, len);
  vkUnmapMemory(m_pDriver->m_vulkanState.m_Device, mapped_res.memory);
}

void GXMReplay::GetTextureData(ResourceId tex, const Subresource &sub,
                               const GetTextureDataParams &params, bytebuf &data)
{
}

void GXMReplay::BuildTargetShader(ShaderEncoding sourceEncoding, const bytebuf &source,
                                  const rdcstr &entry, const ShaderCompileFlags &compileFlags,
                                  ShaderStage type, ResourceId &id, rdcstr &errors)
{
}

rdcarray<ShaderEncoding> GXMReplay::GetTargetShaderEncodings()
{
  return rdcarray<ShaderEncoding>();
}

void GXMReplay::ReplaceResource(ResourceId from, ResourceId to) {}

void GXMReplay::RemoveReplacement(ResourceId id) {}

void GXMReplay::FreeTargetResource(ResourceId id) {}

rdcarray<GPUCounter> GXMReplay::EnumerateCounters()
{
  return rdcarray<GPUCounter>();
}

CounterDescription GXMReplay::DescribeCounter(GPUCounter counterID)
{
  return CounterDescription();
}

rdcarray<CounterResult> GXMReplay::FetchCounters(const rdcarray<GPUCounter> &counterID)
{
  return rdcarray<CounterResult>();
}

void GXMReplay::FillCBufferVariables(ResourceId pipeline, ResourceId shader, rdcstr entryPoint,
                                     uint32_t cbufSlot, rdcarray<ShaderVariable> &outvars,
                                     const bytebuf &data)
{
}

rdcarray<PixelModification> GXMReplay::PixelHistory(rdcarray<EventUsage> events, ResourceId target,
                                                    uint32_t x, uint32_t y, const Subresource &sub,
                                                    CompType typeCast)
{
  return rdcarray<PixelModification>();
}

ShaderDebugTrace *GXMReplay::DebugVertex(uint32_t eventId, uint32_t vertid, uint32_t instid,
                                         uint32_t idx)
{
  return nullptr;
}

ShaderDebugTrace *GXMReplay::DebugPixel(uint32_t eventId, uint32_t x, uint32_t y, uint32_t sample,
                                        uint32_t primitive)
{
  return nullptr;
}

ShaderDebugTrace *GXMReplay::DebugThread(uint32_t eventId, const uint32_t groupid[3],
                                         const uint32_t threadid[3])
{
  return nullptr;
}

rdcarray<ShaderDebugState> GXMReplay::ContinueDebug(ShaderDebugger *debugger)
{
  return rdcarray<ShaderDebugState>();
}

ResourceId GXMReplay::RenderOverlay(ResourceId texid, CompType typeCast, FloatVector clearCol,
                                    DebugOverlay overlay, uint32_t eventId,
                                    const rdcarray<uint32_t> &passEvents)
{
  return ResourceId();
}

bool GXMReplay::IsRenderOutput(ResourceId id)
{
  return false;
}

void GXMReplay::FileChanged() {}

bool GXMReplay::NeedRemapForFetch(const ResourceFormat &format)
{
  return false;
}

DriverInformation GXMReplay::GetDriverInfo()
{
  return DriverInformation();
}

rdcarray<GPUDevice> GXMReplay::GetAvailableGPUs()
{
  return rdcarray<GPUDevice>();
}

bool GXMReplay::IsRemoteProxy()
{
  return false;
}

rdcarray<WindowingSystem> GXMReplay::GetSupportedWindowSystems()
{
  return rdcarray<WindowingSystem>();
}

uint64_t GXMReplay::MakeOutputWindow(WindowingData window, bool depth)
{
  uint64_t id = m_OutputWinID;
  m_OutputWinID++;

  m_OutputWindows[id].m_WindowSystem = window.system;

  if(window.system != WindowingSystem::Unknown && window.system != WindowingSystem::Headless)
    m_OutputWindows[id].SetWindowHandle(window);

  if(window.system != WindowingSystem::Unknown)
  {
    int32_t w, h;

    if(window.system == WindowingSystem::Headless)
    {
      w = window.headless.width;
      h = window.headless.height;
    }
    else
    {
      GetOutputWindowDimensions(id, w, h);
    }

    m_OutputWindows[id].width = w;
    m_OutputWindows[id].height = h;

    m_OutputWindows[id].Create(m_pDriver, depth);
  }

  return id;
}

void GXMReplay::DestroyOutputWindow(uint64_t id) {}

void GXMReplay::SetOutputWindowDimensions(uint64_t id, int32_t w, int32_t h) {}

void GXMReplay::GetOutputWindowData(uint64_t id, bytebuf &retData) {}

void GXMReplay::ClearOutputWindowColor(uint64_t id, FloatVector col) {}

void GXMReplay::ClearOutputWindowDepth(uint64_t id, float depth, uint8_t stencil) {}

bool GXMReplay::GetMinMax(ResourceId texid, const Subresource &sub, CompType typeCast,
                          float *minval, float *maxval)
{
  return false;
}

bool GXMReplay::GetHistogram(ResourceId texid, const Subresource &sub, CompType typeCast,
                             float minval, float maxval, bool channels[4],
                             rdcarray<uint32_t> &histogram)
{
  return false;
}

void GXMReplay::PickPixel(ResourceId texture, uint32_t x, uint32_t y, const Subresource &sub,
                          CompType typeCast, float pixel[4])
{
}

ResourceId GXMReplay::CreateProxyTexture(const TextureDescription &templateTex)
{
  return ResourceId();
}

void GXMReplay::SetProxyTextureData(ResourceId texid, const Subresource &sub, byte *data,
                                    size_t dataSize)
{
}

bool GXMReplay::IsTextureSupported(const TextureDescription &tex)
{
  return false;
}

ResourceId GXMReplay::CreateProxyBuffer(const BufferDescription &templateBuf)
{
  return ResourceId();
}

void GXMReplay::SetProxyBufferData(ResourceId bufid, byte *data, size_t dataSize) {}

void GXMReplay::RenderMesh(uint32_t eventId, const rdcarray<MeshFormat> &secondaryDraws,
                           const MeshDisplay &cfg)
{
  if(cfg.position.vertexResourceId == ResourceId() || cfg.position.numIndices == 0)
    return;

  // TODO: REMOVE!!!!!
  if(cfg.position.vertexResourceId != ResourceId())
    return;

  auto it = m_OutputWindows.find(m_ActiveWinID);
  if(m_ActiveWinID == 0 || it == m_OutputWindows.end())
    return;

  OutputWindow &outw = it->second;

  // if the swapchain failed to create, do nothing. We will try to recreate it
  // again in CheckResizeOutputWindow (once per render 'frame')
  if(outw.m_WindowSystem != WindowingSystem::Headless && outw.swap == VK_NULL_HANDLE)
    return;

  // VkDevice dev = m_pDriver->m_vulkanState.m_Device;
  VkCommandBuffer cmd = m_pDriver->GetNextCmd();

  VkResult vkr = VK_SUCCESS;

  VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, NULL,
                                        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};

  vkr = vkBeginCommandBuffer(cmd, &beginInfo);
  RDCASSERTEQUAL(vkr, VK_SUCCESS);

  // VkMarkerRegion::Begin(
  //    StringFormat::Fmt("RenderMesh with %zu secondary draws", secondaryDraws.size()), cmd);

  VkRenderPassBeginInfo rpbegin = {
      VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      NULL,
      outw.rpdepth,
      outw.fbdepth,
      {{
           0,
           0,
       },
       {m_DebugWidth, m_DebugHeight}},
      0,
      NULL,
  };
  vkCmdBeginRenderPass(cmd, &rpbegin, VK_SUBPASS_CONTENTS_INLINE);

  VkViewport viewport = {0.0f, 0.0f, (float)m_DebugWidth, (float)m_DebugHeight, 0.0f, 1.0f};
  vkCmdSetViewport(cmd, 0, 1, &viewport);

  Matrix4f projMat =
      Matrix4f::Perspective(90.0f, 0.1f, 100000.0f, float(m_DebugWidth) / float(m_DebugHeight));
  Matrix4f InvProj = projMat.Inverse();

  Matrix4f camMat = cfg.cam ? ((Camera *)cfg.cam)->GetMatrix() : Matrix4f::Identity();

  Matrix4f ModelViewProj = projMat.Mul(camMat);
  Matrix4f guessProjInv;

  if(cfg.position.unproject)
  {
    // the derivation of the projection matrix might not be right (hell, it could be an
    // orthographic projection). But it'll be close enough likely.
    Matrix4f guessProj =
        cfg.position.farPlane != FLT_MAX
            ? Matrix4f::Perspective(cfg.fov, cfg.position.nearPlane, cfg.position.farPlane, cfg.aspect)
            : Matrix4f::ReversePerspective(cfg.fov, cfg.position.nearPlane, cfg.aspect);

    if(cfg.ortho)
    {
      guessProj = Matrix4f::Orthographic(cfg.position.nearPlane, cfg.position.farPlane);
    }

    guessProjInv = guessProj.Inverse();

    ModelViewProj = projMat.Mul(camMat.Mul(guessProjInv));
  }

  if(!secondaryDraws.empty())
  {
    size_t mapsUsed = 0;

    for(size_t i = 0; i < secondaryDraws.size(); i++)
    {
      const MeshFormat &fmt = secondaryDraws[i];

      if(fmt.vertexResourceId != ResourceId())
      {
        // TODO should move the color to a push constant so we don't have to map all the time
        uint32_t uboOffs = 0;
        MeshUBOData *data = (MeshUBOData *)m_MeshRender.UBO.Map(&uboOffs);

        data->mvp = ModelViewProj;
        data->color = Vec4f(fmt.meshColor.x, fmt.meshColor.y, fmt.meshColor.z, fmt.meshColor.w);
        data->homogenousInput = cfg.position.unproject;
        data->pointSpriteSize = Vec2f(0.0f, 0.0f);
        data->displayFormat = MESHDISPLAY_SOLID;
        data->rawoutput = 0;

        m_MeshRender.UBO.Unmap();

        mapsUsed++;

        if(mapsUsed + 1 >= m_MeshRender.UBO.GetRingCount())
        {
          // flush and sync so we can use more maps
          vkCmdEndRenderPass(cmd);

          vkr = vkEndCommandBuffer(cmd);
          RDCASSERTEQUAL(vkr, VK_SUCCESS);

          m_pDriver->SubmitCmds();
          m_pDriver->FlushQ();

          mapsUsed = 0;

          cmd = m_pDriver->GetNextCmd();

          vkr = vkBeginCommandBuffer(cmd, &beginInfo);
          RDCASSERTEQUAL(vkr, VK_SUCCESS);
          vkCmdBeginRenderPass(cmd, &rpbegin, VK_SUBPASS_CONTENTS_INLINE);

          vkCmdSetViewport(cmd, 0, 1, &viewport);
        }

        MeshDisplayPipelines secondaryCache = GetDebugManager()->CacheMeshDisplayPipelines(
            m_MeshRender.PipeLayout, secondaryDraws[i], secondaryDraws[i]);

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_MeshRender.PipeLayout, 0, 1,
                                &m_MeshRender.DescSet, 1, &uboOffs);

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          secondaryCache.pipes[MeshDisplayPipelines::ePipe_WireDepth]);

        VkBuffer vb = m_pDriver->GetResourceManager()->GetLiveResource(fmt.vertexResourceId).buffer;

        VkDeviceSize offs = fmt.vertexByteOffset;
        vkCmdBindVertexBuffers(cmd, 0, 1, &vb, &offs);

        if(fmt.indexByteStride)
        {
          VkIndexType idxtype = VK_INDEX_TYPE_UINT16;
          if(fmt.indexByteStride == 4)
            idxtype = VK_INDEX_TYPE_UINT32;
          else if(fmt.indexByteStride == 1)
            idxtype = VK_INDEX_TYPE_UINT8_EXT;

          if(fmt.indexResourceId != ResourceId())
          {
            VkBuffer ib =
                m_pDriver->GetResourceManager()->GetLiveResource(fmt.indexResourceId).buffer;

            vkCmdBindIndexBuffer(cmd, ib, fmt.indexByteOffset, idxtype);
          }
          vkCmdDrawIndexed(cmd, fmt.numIndices, 1, 0, fmt.baseVertex, 0);
        }
        else
        {
          vkCmdDraw(cmd, fmt.numIndices, 1, 0, 0);
        }
      }
    }

    {
      // flush and sync so we can use more maps
      vkCmdEndRenderPass(cmd);

      vkr = vkEndCommandBuffer(cmd);
      RDCASSERTEQUAL(vkr, VK_SUCCESS);

      m_pDriver->SubmitCmds();
      m_pDriver->FlushQ();

      cmd = m_pDriver->GetNextCmd();

      vkr = vkBeginCommandBuffer(cmd, &beginInfo);
      RDCASSERTEQUAL(vkr, VK_SUCCESS);
      vkCmdBeginRenderPass(cmd, &rpbegin, VK_SUBPASS_CONTENTS_INLINE);

      vkCmdSetViewport(cmd, 0, 1, &viewport);
    }
  }

  MeshDisplayPipelines cache = GetDebugManager()->CacheMeshDisplayPipelines(
      m_MeshRender.PipeLayout, cfg.position, cfg.second);

  if(cfg.position.vertexResourceId != ResourceId())
  {
    VkBuffer vb =
        m_pDriver->GetResourceManager()->GetLiveResource(cfg.position.vertexResourceId).buffer;

    VkDeviceSize offs = cfg.position.vertexByteOffset;

    // we source all data from the first instanced value in the instanced case, so make sure we
    // offset correctly here.
    if(cfg.position.instanced && cfg.position.instStepRate)
      offs += cfg.position.vertexByteStride * (cfg.curInstance / cfg.position.instStepRate);

    vkCmdBindVertexBuffers(cmd, 0, 1, &vb, &offs);
  }

  SolidShade solidShadeMode = cfg.solidShadeMode;

  // can't support secondary shading without a buffer - no pipeline will have been created
  if(solidShadeMode == SolidShade::Secondary && cfg.second.vertexResourceId == ResourceId())
    solidShadeMode = SolidShade::NoSolid;

  if(solidShadeMode == SolidShade::Secondary)
  {
    VkBuffer vb =
        m_pDriver->GetResourceManager()->GetLiveResource(cfg.second.vertexResourceId).buffer;

    VkDeviceSize offs = cfg.second.vertexByteOffset;

    // we source all data from the first instanced value in the instanced case, so make sure we
    // offset correctly here.
    if(cfg.second.instanced && cfg.second.instStepRate)
      offs += cfg.second.vertexByteStride * (cfg.curInstance / cfg.second.instStepRate);

    vkCmdBindVertexBuffers(cmd, 1, 1, &vb, &offs);
  }

  // solid render
  if(solidShadeMode != SolidShade::NoSolid && cfg.position.topology < Topology::PatchList)
  {
    VkPipeline pipe = VK_NULL_HANDLE;
    switch(solidShadeMode)
    {
      default:
      case SolidShade::Solid: pipe = cache.pipes[MeshDisplayPipelines::ePipe_SolidDepth]; break;
      case SolidShade::Lit:
        pipe = cache.pipes[MeshDisplayPipelines::ePipe_Lit];
        // point list topologies don't have lighting obvious, just render them as solid
        if(pipe == VK_NULL_HANDLE)
          pipe = cache.pipes[MeshDisplayPipelines::ePipe_SolidDepth];
        break;
      case SolidShade::Secondary: pipe = cache.pipes[MeshDisplayPipelines::ePipe_Secondary]; break;
    }

    // can't support lit rendering without the pipeline - maybe geometry shader wasn't supported.
    if(solidShadeMode == SolidShade::Lit && pipe == VK_NULL_HANDLE)
      pipe = cache.pipes[MeshDisplayPipelines::ePipe_SolidDepth];

    uint32_t uboOffs = 0;
    MeshUBOData *data = (MeshUBOData *)m_MeshRender.UBO.Map(&uboOffs);

    if(solidShadeMode == SolidShade::Lit)
      data->invProj = projMat.Inverse();

    data->mvp = ModelViewProj;
    data->color = Vec4f(0.8f, 0.8f, 0.0f, 1.0f);
    data->homogenousInput = cfg.position.unproject;
    data->pointSpriteSize = Vec2f(0.0f, 0.0f);
    data->displayFormat = (uint32_t)solidShadeMode;
    data->rawoutput = 0;

    if(solidShadeMode == SolidShade::Secondary && cfg.second.showAlpha)
      data->displayFormat = MESHDISPLAY_SECONDARY_ALPHA;

    m_MeshRender.UBO.Unmap();

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_MeshRender.PipeLayout, 0, 1,
                            &m_MeshRender.DescSet, 1, &uboOffs);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);

    if(cfg.position.indexByteStride)
    {
      VkIndexType idxtype = VK_INDEX_TYPE_UINT16;
      if(cfg.position.indexByteStride == 4)
        idxtype = VK_INDEX_TYPE_UINT32;
      else if(cfg.position.indexByteStride == 1)
        idxtype = VK_INDEX_TYPE_UINT8_EXT;

      if(cfg.position.indexResourceId != ResourceId())
      {
        VkBuffer ib =
            m_pDriver->GetResourceManager()->GetLiveResource(cfg.position.indexResourceId).buffer;

        vkCmdBindIndexBuffer(cmd, ib, cfg.position.indexByteOffset, idxtype);
      }
      vkCmdDrawIndexed(cmd, cfg.position.numIndices, 1, 0, cfg.position.baseVertex, 0);
    }
    else
    {
      vkCmdDraw(cmd, cfg.position.numIndices, 1, 0, 0);
    }
  }

  // wireframe render
  if(solidShadeMode == SolidShade::NoSolid || cfg.wireframeDraw ||
     cfg.position.topology >= Topology::PatchList)
  {
    Vec4f wireCol =
        Vec4f(cfg.position.meshColor.x, cfg.position.meshColor.y, cfg.position.meshColor.z, 1.0f);

    uint32_t uboOffs = 0;
    MeshUBOData *data = (MeshUBOData *)m_MeshRender.UBO.Map(&uboOffs);

    data->mvp = ModelViewProj;
    data->color = wireCol;
    data->displayFormat = (uint32_t)SolidShade::Solid;
    data->homogenousInput = cfg.position.unproject;
    data->pointSpriteSize = Vec2f(0.0f, 0.0f);
    data->rawoutput = 0;

    m_MeshRender.UBO.Unmap();

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_MeshRender.PipeLayout, 0, 1,
                            &m_MeshRender.DescSet, 1, &uboOffs);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      cache.pipes[MeshDisplayPipelines::ePipe_WireDepth]);

    if(cfg.position.indexByteStride)
    {
      VkIndexType idxtype = VK_INDEX_TYPE_UINT16;
      if(cfg.position.indexByteStride == 4)
        idxtype = VK_INDEX_TYPE_UINT32;
      else if(cfg.position.indexByteStride == 1)
        idxtype = VK_INDEX_TYPE_UINT8_EXT;

      if(cfg.position.indexResourceId != ResourceId())
      {
        VkBuffer ib =
            m_pDriver->GetResourceManager()->GetLiveResource(cfg.position.indexResourceId).buffer;

        vkCmdBindIndexBuffer(cmd, ib, cfg.position.indexByteOffset, idxtype);
      }
      vkCmdDrawIndexed(cmd, cfg.position.numIndices, 1, 0, cfg.position.baseVertex, 0);
    }
    else
    {
      vkCmdDraw(cmd, cfg.position.numIndices, 1, 0, 0);
    }
  }

  MeshFormat helper;
  helper.indexByteStride = 2;
  helper.topology = Topology::LineList;

  helper.format.type = ResourceFormatType::Regular;
  helper.format.compByteWidth = 4;
  helper.format.compCount = 4;
  helper.format.compType = CompType::Float;

  helper.vertexByteStride = sizeof(Vec4f);

  // cache pipelines for use in drawing wireframe helpers
  cache = GetDebugManager()->CacheMeshDisplayPipelines(m_MeshRender.PipeLayout, helper, helper);

  if(cfg.showBBox)
  {
    Vec4f a = Vec4f(cfg.minBounds.x, cfg.minBounds.y, cfg.minBounds.z, cfg.minBounds.w);
    Vec4f b = Vec4f(cfg.maxBounds.x, cfg.maxBounds.y, cfg.maxBounds.z, cfg.maxBounds.w);

    Vec4f TLN = Vec4f(a.x, b.y, a.z, 1.0f);    // TopLeftNear, etc...
    Vec4f TRN = Vec4f(b.x, b.y, a.z, 1.0f);
    Vec4f BLN = Vec4f(a.x, a.y, a.z, 1.0f);
    Vec4f BRN = Vec4f(b.x, a.y, a.z, 1.0f);

    Vec4f TLF = Vec4f(a.x, b.y, b.z, 1.0f);
    Vec4f TRF = Vec4f(b.x, b.y, b.z, 1.0f);
    Vec4f BLF = Vec4f(a.x, a.y, b.z, 1.0f);
    Vec4f BRF = Vec4f(b.x, a.y, b.z, 1.0f);

    // 12 frustum lines => 24 verts
    Vec4f bbox[24] = {
        TLN, TRN, TRN, BRN, BRN, BLN, BLN, TLN,

        TLN, TLF, TRN, TRF, BLN, BLF, BRN, BRF,

        TLF, TRF, TRF, BRF, BRF, BLF, BLF, TLF,
    };

    VkDeviceSize vboffs = 0;
    Vec4f *ptr = (Vec4f *)m_MeshRender.BBoxVB.Map(vboffs);

    memcpy(ptr, bbox, sizeof(bbox));

    m_MeshRender.BBoxVB.Unmap();

    vkCmdBindVertexBuffers(cmd, 0, 1, &m_MeshRender.BBoxVB.buf, &vboffs);

    uint32_t uboOffs = 0;
    MeshUBOData *data = (MeshUBOData *)m_MeshRender.UBO.Map(&uboOffs);

    data->mvp = ModelViewProj;
    data->color = Vec4f(0.2f, 0.2f, 1.0f, 1.0f);
    data->displayFormat = (uint32_t)SolidShade::Solid;
    data->homogenousInput = 0;
    data->pointSpriteSize = Vec2f(0.0f, 0.0f);
    data->rawoutput = 0;

    m_MeshRender.UBO.Unmap();

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_MeshRender.PipeLayout, 0, 1,
                            &m_MeshRender.DescSet, 1, &uboOffs);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      cache.pipes[MeshDisplayPipelines::ePipe_WireDepth]);

    vkCmdDraw(cmd, 24, 1, 0, 0);
  }

  // draw axis helpers
  if(!cfg.position.unproject)
  {
    VkDeviceSize vboffs = 0;
    vkCmdBindVertexBuffers(cmd, 0, 1, &m_MeshRender.AxisFrustumVB.buf, &vboffs);

    uint32_t uboOffs = 0;
    MeshUBOData *data = (MeshUBOData *)m_MeshRender.UBO.Map(&uboOffs);

    data->mvp = ModelViewProj;
    data->color = Vec4f(1.0f, 0.0f, 0.0f, 1.0f);
    data->displayFormat = (uint32_t)SolidShade::Solid;
    data->homogenousInput = 0;
    data->pointSpriteSize = Vec2f(0.0f, 0.0f);
    data->rawoutput = 0;

    m_MeshRender.UBO.Unmap();

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_MeshRender.PipeLayout, 0, 1,
                            &m_MeshRender.DescSet, 1, &uboOffs);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      cache.pipes[MeshDisplayPipelines::ePipe_Wire]);

    vkCmdDraw(cmd, 2, 1, 0, 0);

    // poke the color (this would be a good candidate for a push constant)
    data = (MeshUBOData *)m_MeshRender.UBO.Map(&uboOffs);

    data->mvp = ModelViewProj;
    data->color = Vec4f(0.0f, 1.0f, 0.0f, 1.0f);
    data->displayFormat = (uint32_t)SolidShade::Solid;
    data->homogenousInput = 0;
    data->pointSpriteSize = Vec2f(0.0f, 0.0f);
    data->rawoutput = 0;

    m_MeshRender.UBO.Unmap();

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_MeshRender.PipeLayout, 0, 1,
                            &m_MeshRender.DescSet, 1, &uboOffs);
    vkCmdDraw(cmd, 2, 1, 2, 0);

    data = (MeshUBOData *)m_MeshRender.UBO.Map(&uboOffs);

    data->mvp = ModelViewProj;
    data->color = Vec4f(0.0f, 0.0f, 1.0f, 1.0f);
    data->displayFormat = (uint32_t)SolidShade::Solid;
    data->homogenousInput = 0;
    data->pointSpriteSize = Vec2f(0.0f, 0.0f);
    data->rawoutput = 0;

    m_MeshRender.UBO.Unmap();

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_MeshRender.PipeLayout, 0, 1,
                            &m_MeshRender.DescSet, 1, &uboOffs);
    vkCmdDraw(cmd, 2, 1, 4, 0);
  }

  // 'fake' helper frustum
  if(cfg.position.unproject)
  {
    VkDeviceSize vboffs = sizeof(Vec4f) * 6;    // skim the axis helpers
    vkCmdBindVertexBuffers(cmd, 0, 1, &m_MeshRender.AxisFrustumVB.buf, &vboffs);

    uint32_t uboOffs = 0;
    MeshUBOData *data = (MeshUBOData *)m_MeshRender.UBO.Map(&uboOffs);

    data->mvp = ModelViewProj;
    data->color = Vec4f(1.0f, 1.0f, 1.0f, 1.0f);
    data->displayFormat = (uint32_t)SolidShade::Solid;
    data->homogenousInput = 0;
    data->pointSpriteSize = Vec2f(0.0f, 0.0f);
    data->rawoutput = 0;

    m_MeshRender.UBO.Unmap();

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_MeshRender.PipeLayout, 0, 1,
                            &m_MeshRender.DescSet, 1, &uboOffs);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      cache.pipes[MeshDisplayPipelines::ePipe_Wire]);

    vkCmdDraw(cmd, 24, 1, 0, 0);
  }

  // show highlighted vertex
  /*if(cfg.highlightVert != ~0U)
  {
    {
      // need to end our cmd buffer, it might be submitted in GetBufferData when caching highlight
      // data
      vkCmdEndRenderPass(cmd);

      vkr = vkEndCommandBuffer(cmd);
      RDCASSERTEQUAL(vkr, VK_SUCCESS);

#if ENABLED(SINGLE_FLUSH_VALIDATE)
      m_pDriver->SubmitCmds();
#endif
    }

    m_HighlightCache.CacheHighlightingData(eventId, cfg);

    {
      // get a new cmdbuffer and begin it
      cmd = m_pDriver->GetNextCmd();

      vkr = vkBeginCommandBuffer(cmd, &beginInfo);
      RDCASSERTEQUAL(vkr, VK_SUCCESS);
      vkCmdBeginRenderPass(cmd, &rpbegin, VK_SUBPASS_CONTENTS_INLINE);

      vkCmdSetViewport(cmd, 0, 1, &viewport);
    }

    Topology meshtopo = cfg.position.topology;

    ///////////////////////////////////////////////////////////////
    // vectors to be set from buffers, depending on topology

    // this vert (blue dot, required)
    FloatVector activeVertex;

    // primitive this vert is a part of (red prim, optional)
    rdcarray<FloatVector> activePrim;

    // for patch lists, to show other verts in patch (green dots, optional)
    // for non-patch lists, we use the activePrim and adjacentPrimVertices
    // to show what other verts are related
    rdcarray<FloatVector> inactiveVertices;

    // adjacency (line or tri, strips or lists) (green prims, optional)
    // will be N*M long, N adjacent prims of M verts each. M = primSize below
    rdcarray<FloatVector> adjacentPrimVertices;

    helper.topology = Topology::TriangleList;
    uint32_t primSize = 3;    // number of verts per primitive

    if(meshtopo == Topology::LineList || meshtopo == Topology::LineStrip ||
       meshtopo == Topology::LineList_Adj || meshtopo == Topology::LineStrip_Adj)
    {
      primSize = 2;
      helper.topology = Topology::LineList;
    }
    else
    {
      // update the cache, as it's currently linelist
      helper.topology = Topology::TriangleList;
      cache = GetDebugManager()->CacheMeshDisplayPipelines(m_MeshRender.PipeLayout, helper, helper);
    }

    bool valid = m_HighlightCache.FetchHighlightPositions(cfg, activeVertex, activePrim,
                                                          adjacentPrimVertices, inactiveVertices);

    if(valid)
    {
      ////////////////////////////////////////////////////////////////
      // prepare rendering (for both vertices & primitives)

      // if data is from post transform, it will be in clipspace
      if(cfg.position.unproject)
        ModelViewProj = projMat.Mul(camMat.Mul(guessProjInv));
      else
        ModelViewProj = projMat.Mul(camMat);

      MeshUBOData uniforms = {};
      uniforms.mvp = ModelViewProj;
      uniforms.color = Vec4f(1.0f, 1.0f, 1.0f, 1.0f);
      uniforms.displayFormat = (uint32_t)SolidShade::Solid;
      uniforms.homogenousInput = cfg.position.unproject;
      uniforms.pointSpriteSize = Vec2f(0.0f, 0.0f);

      uint32_t uboOffs = 0;
      MeshUBOData *ubodata = (MeshUBOData *)m_MeshRender.UBO.Map(&uboOffs);
      *ubodata = uniforms;
      m_MeshRender.UBO.Unmap();

      vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_MeshRender.PipeLayout, 0, 1,
                                &m_MeshRender.DescSet, 1, &uboOffs);

      vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          cache.pipes[MeshDisplayPipelines::ePipe_Solid]);

      ////////////////////////////////////////////////////////////////
      // render primitives

      // Draw active primitive (red)
      uniforms.color = Vec4f(1.0f, 0.0f, 0.0f, 1.0f);
      // poke the color (this would be a good candidate for a push constant)
      ubodata = (MeshUBOData *)m_MeshRender.UBO.Map(&uboOffs);
      *ubodata = uniforms;
      m_MeshRender.UBO.Unmap();
      vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_MeshRender.PipeLayout, 0, 1,
                                &m_MeshRender.DescSet, 1, &uboOffs);

      if(activePrim.size() >= primSize)
      {
        VkDeviceSize vboffs = 0;
        Vec4f *ptr = (Vec4f *)m_MeshRender.BBoxVB.Map(vboffs, sizeof(Vec4f) * primSize);

        memcpy(ptr, &activePrim[0], sizeof(Vec4f) * primSize);

        m_MeshRender.BBoxVB.Unmap();

        vkCmdBindVertexBuffers(cmd, 0, 1, &m_MeshRender.BBoxVB.buf, &vboffs);

        vkCmdDraw(cmd, primSize, 1, 0, 0);
      }

      // Draw adjacent primitives (green)
      uniforms.color = Vec4f(0.0f, 1.0f, 0.0f, 1.0f);
      // poke the color (this would be a good candidate for a push constant)
      ubodata = (MeshUBOData *)m_MeshRender.UBO.Map(&uboOffs);
      *ubodata = uniforms;
      m_MeshRender.UBO.Unmap();
      vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_MeshRender.PipeLayout, 0, 1,
                                &m_MeshRender.DescSet, 1, &uboOffs);

      if(adjacentPrimVertices.size() >= primSize && (adjacentPrimVertices.size() % primSize) == 0)
      {
        VkDeviceSize vboffs = 0;
        Vec4f *ptr =
            (Vec4f *)m_MeshRender.BBoxVB.Map(vboffs, sizeof(Vec4f) * adjacentPrimVertices.size());

        memcpy(ptr, &adjacentPrimVertices[0], sizeof(Vec4f) * adjacentPrimVertices.size());

        m_MeshRender.BBoxVB.Unmap();

        vkCmdBindVertexBuffers(cmd, 0, 1, &m_MeshRender.BBoxVB.buf, &vboffs);

        vkCmdDraw(cmd, (uint32_t)adjacentPrimVertices.size(), 1, 0, 0);
      }

      ////////////////////////////////////////////////////////////////
      // prepare to render dots
      float scale = 800.0f / float(m_DebugHeight);
      float asp = float(m_DebugWidth) / float(m_DebugHeight);

      uniforms.pointSpriteSize = Vec2f(scale / asp, scale);

      // Draw active vertex (blue)
      uniforms.color = Vec4f(0.0f, 0.0f, 1.0f, 1.0f);
      // poke the color (this would be a good candidate for a push constant)
      ubodata = (MeshUBOData *)m_MeshRender.UBO.Map(&uboOffs);
      *ubodata = uniforms;
      m_MeshRender.UBO.Unmap();
      vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_MeshRender.PipeLayout, 0, 1,
                                &m_MeshRender.DescSet, 1, &uboOffs);

      // vertices are drawn with tri strips
      helper.topology = Topology::TriangleStrip;
      cache = GetDebugManager()->CacheMeshDisplayPipelines(m_MeshRender.PipeLayout, helper, helper);

      FloatVector vertSprite[4] = {
          activeVertex,
          activeVertex,
          activeVertex,
          activeVertex,
      };

      vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_MeshRender.PipeLayout, 0, 1,
                                &m_MeshRender.DescSet, 1, &uboOffs);

      vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          cache.pipes[MeshDisplayPipelines::ePipe_Solid]);

      {
        VkDeviceSize vboffs = 0;
        Vec4f *ptr = (Vec4f *)m_MeshRender.BBoxVB.Map(vboffs, sizeof(vertSprite));

        memcpy(ptr, &vertSprite[0], sizeof(vertSprite));

        m_MeshRender.BBoxVB.Unmap();

        vkCmdBindVertexBuffers(cmd, 0, 1, &m_MeshRender.BBoxVB.buf, &vboffs);

        vkCmdDraw(cmd, 4, 1, 0, 0);
      }

      // Draw inactive vertices (green)
      uniforms.color = Vec4f(0.0f, 1.0f, 0.0f, 1.0f);
      // poke the color (this would be a good candidate for a push constant)
      ubodata = (MeshUBOData *)m_MeshRender.UBO.Map(&uboOffs);
      *ubodata = uniforms;
      m_MeshRender.UBO.Unmap();
      vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_MeshRender.PipeLayout, 0, 1,
                                &m_MeshRender.DescSet, 1, &uboOffs);

      if(!inactiveVertices.empty())
      {
        VkDeviceSize vboffs = 0;
        FloatVector *ptr = (FloatVector *)m_MeshRender.BBoxVB.Map(vboffs, sizeof(vertSprite));

        for(size_t i = 0; i < inactiveVertices.size(); i++)
        {
          *ptr++ = inactiveVertices[i];
          *ptr++ = inactiveVertices[i];
          *ptr++ = inactiveVertices[i];
          *ptr++ = inactiveVertices[i];
        }

        m_MeshRender.BBoxVB.Unmap();

        for(size_t i = 0; i < inactiveVertices.size(); i++)
        {
          vkCmdBindVertexBuffers(cmd, 0, 1, &m_MeshRender.BBoxVB.buf, &vboffs);

          vkCmdDraw(cmd, 4, 1, 0, 0);

          vboffs += sizeof(FloatVector) * 4;
        }
      }
    }
  }*/

  vkCmdEndRenderPass(cmd);

  // VkMarkerRegion::End(cmd);

  vkr = vkEndCommandBuffer(cmd);
  RDCASSERTEQUAL(vkr, VK_SUCCESS);

#if ENABLED(SINGLE_FLUSH_VALIDATE)
  m_pDriver->SubmitCmds();
#endif
}

bool GXMReplay::RenderTexture(TextureDisplay cfg)
{
  return false;
}

void GXMReplay::BuildCustomShader(ShaderEncoding sourceEncoding, const bytebuf &source,
                                  const rdcstr &entry, const ShaderCompileFlags &compileFlags,
                                  ShaderStage type, ResourceId &id, rdcstr &errors)
{
}

rdcarray<ShaderEncoding> GXMReplay::GetCustomShaderEncodings()
{
  return rdcarray<ShaderEncoding>();
}

ResourceId GXMReplay::ApplyCustomShader(ResourceId shader, ResourceId texid, const Subresource &sub,
                                        CompType typeCast)
{
  return ResourceId();
}

void GXMReplay::FreeCustomShader(ResourceId id) {}

void GXMReplay::RenderCheckerboard()
{
  auto it = m_OutputWindows.find(m_ActiveWinID);
  if(m_ActiveWinID == 0 || it == m_OutputWindows.end())
    return;

  OutputWindow &outw = it->second;

  // if the swapchain failed to create, do nothing. We will try to recreate it
  // again in CheckResizeOutputWindow (once per render 'frame')
  if(outw.m_WindowSystem != WindowingSystem::Headless && outw.swap == VK_NULL_HANDLE)
    return;

  // VkDevice dev = m_pDriver->m_vulkanState.m_Device;
  VkCommandBuffer cmd = m_pDriver->GetNextCmd();

  VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, NULL,
                                        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};

  VkResult vkr = vkBeginCommandBuffer(cmd, &beginInfo);
  RDCASSERTEQUAL(vkr, VK_SUCCESS);

  // uint32_t uboOffs = 0;

  VkRenderPassBeginInfo rpbegin = {
      VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      NULL,
      outw.rp,
      outw.fb,
      {{
           0,
           0,
       },
       {m_DebugWidth, m_DebugHeight}},
      0,
      NULL,
  };
  vkCmdBeginRenderPass(cmd, &rpbegin, VK_SUBPASS_CONTENTS_INLINE);

  /*if(m_Overlay.m_CheckerPipeline != VK_NULL_HANDLE)
  {
    CheckerboardUBOData *data = (CheckerboardUBOData *)m_Overlay.m_CheckerUBO.Map(&uboOffs);
    data->BorderWidth = 0.0f;
    data->RectPosition = Vec2f();
    data->RectSize = Vec2f();
    data->CheckerSquareDimension = 64.0f;
    data->InnerColor = Vec4f();

    data->PrimaryColor = ConvertSRGBToLinear(RenderDoc::Inst().DarkCheckerboardColor());
    data->SecondaryColor = ConvertSRGBToLinear(RenderDoc::Inst().LightCheckerboardColor());
    m_Overlay.m_CheckerUBO.Unmap();

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                        outw.dsimg == VK_NULL_HANDLE ? Unwrap(m_Overlay.m_CheckerPipeline)
                                                     : Unwrap(m_Overlay.m_CheckerMSAAPipeline));
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                              Unwrap(m_Overlay.m_CheckerPipeLayout), 0, 1,
                              UnwrapPtr(m_Overlay.m_CheckerDescSet), 1, &uboOffs);

    VkViewport viewport = {0.0f, 0.0f, (float)m_DebugWidth, (float)m_DebugHeight, 0.0f, 1.0f};
    vkCmdSetViewport(cmd, 0, 1, &viewport);

    vkCmdDraw(cmd, 4, 1, 0, 0);

    if(m_pDriver->GetDriverInfo().QualcommLeakingUBOOffsets())
    {
      uboOffs = 0;
      vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                Unwrap(m_Overlay.m_CheckerPipeLayout), 0, 1,
                                UnwrapPtr(m_Overlay.m_CheckerDescSet), 1, &uboOffs);
    }
  }
  else*/
  {
    // some mobile chips fail to create the checkerboard pipeline. Use an alternate approach with
    // CmdClearAttachment and many rects.

    Vec4f lightCol = RenderDoc::Inst().LightCheckerboardColor();
    Vec4f darkCol = RenderDoc::Inst().DarkCheckerboardColor();

    VkClearAttachment light = {
        VK_IMAGE_ASPECT_COLOR_BIT, 0, {{{lightCol.x, lightCol.y, lightCol.z, lightCol.w}}}};
    VkClearAttachment dark = {
        VK_IMAGE_ASPECT_COLOR_BIT, 0, {{{darkCol.x, darkCol.y, darkCol.z, darkCol.w}}}};

    VkClearRect fullRect = {{
                                {0, 0},
                                {outw.width, outw.height},
                            },
                            0,
                            1};

    vkCmdClearAttachments(cmd, 1, &light, 1, &fullRect);

    rdcarray<VkClearRect> squares;

    for(int32_t y = 0; y < (int32_t)outw.height; y += 128)
    {
      for(int32_t x = 0; x < (int32_t)outw.width; x += 128)
      {
        VkClearRect square = {{
                                  {x, y},
                                  {(((uint32_t)x + 64) > outw.width ? 64 - (outw.width % 64) : 64),
                                   (((uint32_t)y + 64) > outw.height ? 64 - (outw.height % 64) : 64)},
                              },
                              0,
                              1};

        squares.push_back(square);

        square.rect.offset.x +=
            ((uint32_t)square.rect.offset.x + 64) > outw.width ? (outw.width % 64) - 1 : 64;
        square.rect.extent.width =
            ((uint32_t)square.rect.offset.x + 64) > outw.width ? (outw.width % 64) - 1 : 64;

        if(square.rect.offset.x == (int32_t)(outw.width - 1))
        {
          square.rect.extent.width = 0;
        }

        square.rect.offset.y +=
            ((uint32_t)square.rect.offset.y + 64) > outw.height ? (outw.height % 64) - 1 : 64;
        square.rect.extent.height =
            ((uint32_t)square.rect.offset.y + 64) > outw.height ? (outw.height % 64) - 1 : 64;

        if(square.rect.offset.y == (int32_t)(outw.height - 1))
        {
          square.rect.extent.height = 0;
        }

        if(square.rect.extent.width > 0 && square.rect.extent.height > 0)
        {
          squares.push_back(square);
        }
      }
    }

    vkCmdClearAttachments(cmd, 1, &dark, (uint32_t)squares.size(), squares.data());
  }

  vkCmdEndRenderPass(cmd);

  vkr = vkEndCommandBuffer(cmd);
  RDCASSERTEQUAL(vkr, VK_SUCCESS);

#if ENABLED(SINGLE_FLUSH_VALIDATE)
  m_pDriver->SubmitCmds();
#endif
}

void GXMReplay::RenderHighlightBox(float w, float h, float scale)
{
  auto it = m_OutputWindows.find(m_ActiveWinID);
  if(m_ActiveWinID == 0 || it == m_OutputWindows.end())
    return;

  OutputWindow &outw = it->second;

  // if the swapchain failed to create, do nothing. We will try to recreate it
  // again in CheckResizeOutputWindow (once per render 'frame')
  if(outw.m_WindowSystem != WindowingSystem::Headless && outw.swap == VK_NULL_HANDLE)
    return;

  // VkDevice dev = m_pDriver->m_vulkanState.m_Device;
  VkCommandBuffer cmd = m_pDriver->GetNextCmd();

  VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, NULL,
                                        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};

  VkResult vkr = vkBeginCommandBuffer(cmd, &beginInfo);
  RDCASSERTEQUAL(vkr, VK_SUCCESS);

  {
    VkRenderPassBeginInfo rpbegin = {
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        NULL,
        outw.rp,
        outw.fb,
        {{
             0,
             0,
         },
         {m_DebugWidth, m_DebugHeight}},
        0,
        NULL,
    };
    vkCmdBeginRenderPass(cmd, &rpbegin, VK_SUBPASS_CONTENTS_INLINE);

    VkClearAttachment black = {VK_IMAGE_ASPECT_COLOR_BIT, 0, {{{0.0f, 0.0f, 0.0f, 1.0f}}}};
    VkClearAttachment white = {VK_IMAGE_ASPECT_COLOR_BIT, 0, {{{1.0f, 1.0f, 1.0f, 1.0f}}}};

    uint32_t sz = uint32_t(scale);

    VkOffset2D tl = {int32_t(w / 2.0f + 0.5f), int32_t(h / 2.0f + 0.5f)};

    VkClearRect rect[4] = {
        {{
             {tl.x, tl.y},
             {1, sz},
         },
         0,
         1},
        {{
             {tl.x + (int32_t)sz, tl.y},
             {1, sz + 1},
         },
         0,
         1},
        {{
             {tl.x, tl.y},
             {sz, 1},
         },
         0,
         1},
        {{
             {tl.x, tl.y + (int32_t)sz},
             {sz, 1},
         },
         0,
         1},
    };

    // inner
    vkCmdClearAttachments(cmd, 1, &white, 4, rect);

    rect[0].rect.offset.x--;
    rect[1].rect.offset.x++;
    rect[2].rect.offset.x--;
    rect[3].rect.offset.x--;

    rect[0].rect.offset.y--;
    rect[1].rect.offset.y--;
    rect[2].rect.offset.y--;
    rect[3].rect.offset.y++;

    rect[0].rect.extent.height += 2;
    rect[1].rect.extent.height += 2;
    rect[2].rect.extent.width += 2;
    rect[3].rect.extent.width += 2;

    // outer
    vkCmdClearAttachments(cmd, 1, &black, 4, rect);

    vkCmdEndRenderPass(cmd);
  }

  vkr = vkEndCommandBuffer(cmd);
  RDCASSERTEQUAL(vkr, VK_SUCCESS);

#if ENABLED(SINGLE_FLUSH_VALIDATE)
  m_pDriver->SubmitCmds();
#endif
}

uint32_t GXMReplay::PickVertex(uint32_t eventId, int32_t width, int32_t height,
                               const MeshDisplay &cfg, uint32_t x, uint32_t y)
{
  return uint32_t();
}

static GXMReplay *s_gxmreplay = NULL;

ReplayStatus GXM_CreateReplayDevice(RDCFile *rdc, const ReplayOptions &opts, IReplayDriver **driver)
{
  RDCLOG("Creating an GXM replay device");

  void *module = LoadVulkanLibrary();

  if(module == NULL)
  {
    RDCERR("Failed to load vulkan library");
    return ReplayStatus::APIInitFailed;
  }

  InitialiseVulkanCalls(module);

  WrappedGXM *gxmdriver = new WrappedGXM();

  gxmdriver->Initialise();

  *driver = (IReplayDriver *)gxmdriver->GetReplay();

  return ReplayStatus::Succeeded;
}

static DriverRegistration GXMDriverRegistration(RDCDriver::GXM, &GXM_CreateReplayDevice);

GXMReplay::GXMReplay(WrappedGXM *d)
{
  m_pDriver = d;
  m_OutputWinID = 1;
  m_ActiveWinID = 0;
}

void GXMReplay::Shutdown() {}

APIProperties GXMReplay::GetAPIProperties()
{
  APIProperties ret;

  ret.pipelineType = GraphicsAPI::GXM;
  ret.localRenderer = GraphicsAPI::GXM;
  ret.degraded = false;
  ret.vendor = GPUVendor::Imagination;
  ret.shadersMutable = false;

  return ret;
}

ResourceDescription &GXMReplay::GetResourceDesc(ResourceId id)
{
  auto it = m_ResourceIdx.find(id);
  if(it == m_ResourceIdx.end())
  {
    m_ResourceIdx[id] = m_Resources.size();
    m_Resources.push_back(ResourceDescription());
    m_Resources.back().resourceId = id;
    return m_Resources.back();
  }

  return m_Resources[it->second];
}

rdcarray<ResourceDescription> GXMReplay::GetResources()
{
  return m_Resources;
}

rdcarray<BufferDescription> GXMReplay::GetBuffers()
{
  return rdcarray<BufferDescription>();
}

BufferDescription GXMReplay::GetBuffer(ResourceId id)
{
  return BufferDescription();
}

rdcarray<TextureDescription> GXMReplay::GetTextures()
{
  return rdcarray<TextureDescription>();
}

TextureDescription GXMReplay::GetTexture(ResourceId id)
{
  return TextureDescription();
}

rdcarray<DebugMessage> GXMReplay::GetDebugMessages()
{
  return rdcarray<DebugMessage>();
}

rdcarray<ShaderEntryPoint> GXMReplay::GetShaderEntryPoints(ResourceId shader)
{
  return rdcarray<ShaderEntryPoint>();
}

ShaderReflection *GXMReplay::GetShader(ResourceId pipeline, ResourceId shader, ShaderEntryPoint entry)
{
  return nullptr;
}

rdcarray<rdcstr> GXMReplay::GetDisassemblyTargets()
{
  return rdcarray<rdcstr>();
}

rdcstr GXMReplay::DisassembleShader(ResourceId pipeline, const ShaderReflection *refl,
                                    const rdcstr &target)
{
  return rdcstr();
}

rdcarray<EventUsage> GXMReplay::GetUsage(ResourceId id)
{
  return rdcarray<EventUsage>();
}

void GXMReplay::SavePipelineState(uint32_t eventId)
{
  const GXMRenderState &state = m_pDriver->m_RenderState;
  GXMResources &res = m_pDriver->m_Resources;
  GXMPipe::State &pipe = m_PipelineState;

  GXMResourceManager *rm = m_pDriver->GetResourceManager();

  pipe.vertexInput.indexBuffer = rm->GetOriginalID(state.ibuffer.buf);

  const GXMVertexProgram &vertexProgram = res.m_VertexProgram[state.vprogram];

  pipe.vertexInput.attributes.resize(vertexProgram.vertexAttrs.size());

  for(size_t i = 0; i < vertexProgram.vertexAttrs.size(); i++)
  {
    pipe.vertexInput.attributes[i].format = MakeAttributeFormat(
        vertexProgram.vertexAttrs[i].format, vertexProgram.vertexAttrs[i].componentCount);
    pipe.vertexInput.attributes[i].vertexBufferSlot = vertexProgram.vertexAttrs[i].streamIndex;
    pipe.vertexInput.attributes[i].byteOffset = vertexProgram.vertexAttrs[i].offset;
  }

  pipe.vertexInput.vertexBuffers.resize(state.vbuffers.size());
  for(size_t i = 0; i < state.vbuffers.size(); i++)
  {
    pipe.vertexInput.vertexBuffers[i].resourceId = rm->GetOriginalID(state.vbuffers[i].buf);
    pipe.vertexInput.vertexBuffers[i].byteStride = state.vbuffers[i].stride;
    pipe.vertexInput.vertexBuffers[i].byteOffset = state.vbuffers[i].offset;
  }
}

GXMDebugManager *GXMReplay::GetDebugManager()
{
  return m_pDriver->GetDebugManager();
}
