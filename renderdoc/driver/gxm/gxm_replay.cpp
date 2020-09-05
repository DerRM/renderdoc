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
#include "gxm_driver.h"
#include "gxm_replay.h"

ReplayStatus GXMReplay::ReadLogInitialisation(RDCFile *rdc, bool storeStructuredBuffers)
{
  return m_pDriver->ReadLogInitialisation(rdc, storeStructuredBuffers);
}

void GXMReplay::ReplayLog(uint32_t endEventID, ReplayLogType replayType) {}

const SDFile &GXMReplay::GetStructuredFile()
{
  return m_file;
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
  return uint64_t();
}

void GXMReplay::DestroyOutputWindow(uint64_t id) {}

bool GXMReplay::CheckResizeOutputWindow(uint64_t id)
{
  return false;
}

void GXMReplay::SetOutputWindowDimensions(uint64_t id, int32_t w, int32_t h) {}

void GXMReplay::GetOutputWindowDimensions(uint64_t id, int32_t &w, int32_t &h) {}

void GXMReplay::GetOutputWindowData(uint64_t id, bytebuf &retData) {}

void GXMReplay::ClearOutputWindowColor(uint64_t id, FloatVector col) {}

void GXMReplay::ClearOutputWindowDepth(uint64_t id, float depth, uint8_t stencil) {}

void GXMReplay::BindOutputWindow(uint64_t id, bool depth) {}

bool GXMReplay::IsOutputWindowVisible(uint64_t id)
{
  return false;
}

void GXMReplay::FlipOutputWindow(uint64_t id) {}

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

void GXMReplay::RenderCheckerboard() {}

void GXMReplay::RenderHighlightBox(float w, float h, float scale) {}

uint32_t GXMReplay::PickVertex(uint32_t eventId, int32_t width, int32_t height,
                               const MeshDisplay &cfg, uint32_t x, uint32_t y)
{
  return uint32_t();
}

static GXMReplay *s_gxmreplay = NULL;

ReplayStatus GXM_CreateReplayDevice(RDCFile *rdc, const ReplayOptions &opts, IReplayDriver **driver)
{
  RDCLOG("Creating an GXM replay device");

  WrappedGXM *gxmdriver = new WrappedGXM();

  *driver = (IReplayDriver *)gxmdriver->GetReplay();

  return ReplayStatus::Succeeded;
}

static DriverRegistration GXMDriverRegistration(RDCDriver::GXM, &GXM_CreateReplayDevice);

GXMReplay::GXMReplay(WrappedGXM *d)
{
  m_pDriver = d;
}

void GXMReplay::Shutdown() {}

APIProperties GXMReplay::GetAPIProperties()
{
  return APIProperties();
}

rdcarray<ResourceDescription> GXMReplay::GetResources()
{
  return rdcarray<ResourceDescription>();
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

void GXMReplay::SavePipelineState(uint32_t eventId) {}

const D3D11Pipe::State *GXMReplay::GetD3D11PipelineState()
{
  return nullptr;
}

const D3D12Pipe::State *GXMReplay::GetD3D12PipelineState()
{
  return nullptr;
}

const GLPipe::State *GXMReplay::GetGLPipelineState()
{
  return nullptr;
}

const VKPipe::State *GXMReplay::GetVulkanPipelineState()
{
  return nullptr;
}

FrameRecord GXMReplay::GetFrameRecord()
{
  return FrameRecord();
}
