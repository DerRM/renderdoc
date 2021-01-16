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

#include "common/common.h"
#include "serialise/serialiser.h"
#include "serialise/rdcfile.h"
#include "maths/formatpacking.h"
#include "strings/string_utils.h"
#include "gxm_driver.h"
#include "gxm_replay.h"

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
  return ResourceId();
}

MeshFormat GXMReplay::GetPostVSBuffers(uint32_t eventId, uint32_t instID, uint32_t viewID,
                                       MeshDataStage stage)
{
  return MeshFormat();
}

void GXMReplay::GetBufferData(ResourceId buff, uint64_t offset, uint64_t len, bytebuf &retData) {}

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
  RDCDEBUG("test");
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

  //VkDevice dev = m_pDriver->m_vulkanState.m_Device;
  VkCommandBuffer cmd = m_pDriver->GetNextCmd();

  VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, NULL,
                                        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};

  VkResult vkr = vkBeginCommandBuffer(cmd, &beginInfo);
  RDCASSERTEQUAL(vkr, VK_SUCCESS);

  //uint32_t uboOffs = 0;

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
                              {
                              (((uint32_t)x + 64) > outw.width ? 64 - (outw.width % 64) : 64),
                              (((uint32_t)y + 64) > outw.height? 64 - (outw.height % 64) : 64) },
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

  //VkDevice dev = m_pDriver->m_vulkanState.m_Device;
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
  if (it == m_ResourceIdx.end())
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

  const GXMVertexProgram& vertexProgram = res.m_VertexProgram[state.vprogram];

  pipe.vertexInput.attributes.resize(vertexProgram.vertexAttrs.size());

  for(size_t i = 0; i < vertexProgram.vertexAttrs.size(); i++)
  {
    pipe.vertexInput.attributes[i].format = ResourceFormat();
  }

  pipe.vertexInput.vertexBuffers.resize(state.vbuffers.size());
  for(size_t i = 0; i < state.vbuffers.size(); i++)
  {
    pipe.vertexInput.vertexBuffers[i].resourceId = rm->GetOriginalID(state.vbuffers[i].buf);
  }
}
