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

#pragma once

#include "api/replay/renderdoc_replay.h"
#include "core/core.h"
#include "replay/replay_driver.h"
#include "gxm_common.h"
#include "gxm_driver.h"

#if ENABLED(RDOC_WIN32)

#include <windows.h>
#define WINDOW_HANDLE_DECL HWND wnd;
#define WINDOW_HANDLE_INIT wnd = NULL;

#endif

class GXMReplay : IReplayDriver
{
public:
  GXMReplay(WrappedGXM *d);
  virtual ~GXMReplay() {}

  void Shutdown();

  APIProperties GetAPIProperties();

  ResourceDescription &GetResourceDesc(ResourceId id);
  rdcarray<ResourceDescription> GetResources();

  rdcarray<BufferDescription> GetBuffers();
  BufferDescription GetBuffer(ResourceId id);

  rdcarray<TextureDescription> GetTextures();
  TextureDescription GetTexture(ResourceId id);

  virtual rdcarray<DebugMessage> GetDebugMessages();

  virtual rdcarray<ShaderEntryPoint> GetShaderEntryPoints(ResourceId shader);
  virtual ShaderReflection *GetShader(ResourceId pipeline, ResourceId shader, ShaderEntryPoint entry);

  virtual rdcarray<rdcstr> GetDisassemblyTargets();
  virtual rdcstr DisassembleShader(ResourceId pipeline, const ShaderReflection *refl,
                                   const rdcstr &target);

  virtual rdcarray<EventUsage> GetUsage(ResourceId id);

  virtual void SavePipelineState(uint32_t eventId);
  virtual const D3D11Pipe::State *GetD3D11PipelineState() { return NULL; }
  virtual const D3D12Pipe::State *GetD3D12PipelineState() { return NULL; }
  virtual const GLPipe::State *GetGLPipelineState() { return NULL; }
  virtual const VKPipe::State *GetVulkanPipelineState() { return NULL; }
  virtual const GXMPipe::State *GetGXMPipelineState() { return &m_PipelineState; }

  virtual FrameRecord GetFrameRecord() { return m_FrameRecord; }
  FrameRecord &WriteFrameRecord() { return m_FrameRecord; }

  virtual ReplayStatus ReadLogInitialisation(RDCFile *rdc, bool storeStructuredBuffers);
  virtual void ReplayLog(uint32_t endEventID, ReplayLogType replayType);
  virtual const SDFile &GetStructuredFile();

  virtual rdcarray<uint32_t> GetPassEvents(uint32_t eventId);

  virtual void InitPostVSBuffers(uint32_t eventId);
  virtual void InitPostVSBuffers(const rdcarray<uint32_t> &passEvents);

  virtual ResourceId GetLiveID(ResourceId id);

  virtual MeshFormat GetPostVSBuffers(uint32_t eventId, uint32_t instID, uint32_t viewID,
                                      MeshDataStage stage);

  virtual void GetBufferData(ResourceId buff, uint64_t offset, uint64_t len, bytebuf &retData);
  virtual void GetTextureData(ResourceId tex, const Subresource &sub,
                              const GetTextureDataParams &params, bytebuf &data);

  virtual void BuildTargetShader(ShaderEncoding sourceEncoding, const bytebuf &source,
                                 const rdcstr &entry, const ShaderCompileFlags &compileFlags,
                                 ShaderStage type, ResourceId &id, rdcstr &errors);
  rdcarray<ShaderEncoding> GetTargetShaderEncodings();
  void ReplaceResource(ResourceId from, ResourceId to);
  void RemoveReplacement(ResourceId id);
  void FreeTargetResource(ResourceId id);

  rdcarray<GPUCounter> EnumerateCounters();
  CounterDescription DescribeCounter(GPUCounter counterID);
  rdcarray<CounterResult> FetchCounters(const rdcarray<GPUCounter> &counterID);

  void FillCBufferVariables(ResourceId pipeline, ResourceId shader, rdcstr entryPoint,
                            uint32_t cbufSlot, rdcarray<ShaderVariable> &outvars,
                            const bytebuf &data);

  rdcarray<PixelModification> PixelHistory(rdcarray<EventUsage> events, ResourceId target, uint32_t x,
                                           uint32_t y, const Subresource &sub, CompType typeCast);
  ShaderDebugTrace *DebugVertex(uint32_t eventId, uint32_t vertid, uint32_t instid, uint32_t idx);
  ShaderDebugTrace *DebugPixel(uint32_t eventId, uint32_t x, uint32_t y, uint32_t sample,
                               uint32_t primitive);
  ShaderDebugTrace *DebugThread(uint32_t eventId, const uint32_t groupid[3],
                                const uint32_t threadid[3]);
  rdcarray<ShaderDebugState> ContinueDebug(ShaderDebugger *debugger);

  ResourceId RenderOverlay(ResourceId texid, CompType typeCast, FloatVector clearCol,
                           DebugOverlay overlay, uint32_t eventId,
                           const rdcarray<uint32_t> &passEvents);

  bool IsRenderOutput(ResourceId id);

  void FileChanged();

  bool NeedRemapForFetch(const ResourceFormat &format);

  DriverInformation GetDriverInfo();

  rdcarray<GPUDevice> GetAvailableGPUs();

  bool IsRemoteProxy();

  rdcarray<WindowingSystem> GetSupportedWindowSystems();

  AMDRGPControl *GetRGPControl() { return NULL; };

  uint64_t MakeOutputWindow(WindowingData window, bool depth);
  void DestroyOutputWindow(uint64_t id);
  bool CheckResizeOutputWindow(uint64_t id);
  void SetOutputWindowDimensions(uint64_t id, int32_t w, int32_t h);
  void GetOutputWindowDimensions(uint64_t id, int32_t &w, int32_t &h);
  void GetOutputWindowData(uint64_t id, bytebuf &retData);
  void ClearOutputWindowColor(uint64_t id, FloatVector col);
  void ClearOutputWindowDepth(uint64_t id, float depth, uint8_t stencil);
  void BindOutputWindow(uint64_t id, bool depth);
  bool IsOutputWindowVisible(uint64_t id);
  void FlipOutputWindow(uint64_t id);

  bool GetMinMax(ResourceId texid, const Subresource &sub, CompType typeCast, float *minval,
                 float *maxval);
  bool GetHistogram(ResourceId texid, const Subresource &sub, CompType typeCast, float minval,
                    float maxval, bool channels[4], rdcarray<uint32_t> &histogram);
  void PickPixel(ResourceId texture, uint32_t x, uint32_t y, const Subresource &sub,
                 CompType typeCast, float pixel[4]);

  ResourceId CreateProxyTexture(const TextureDescription &templateTex);
  void SetProxyTextureData(ResourceId texid, const Subresource &sub, byte *data, size_t dataSize);
  bool IsTextureSupported(const TextureDescription &tex);

  ResourceId CreateProxyBuffer(const BufferDescription &templateBuf);
  void SetProxyBufferData(ResourceId bufid, byte *data, size_t dataSize);

  void RenderMesh(uint32_t eventId, const rdcarray<MeshFormat> &secondaryDraws,
                  const MeshDisplay &cfg);
  bool RenderTexture(TextureDisplay cfg);

  void BuildCustomShader(ShaderEncoding sourceEncoding, const bytebuf &source, const rdcstr &entry,
                         const ShaderCompileFlags &compileFlags, ShaderStage type, ResourceId &id,
                         rdcstr &errors);
  rdcarray<ShaderEncoding> GetCustomShaderEncodings();
  ResourceId ApplyCustomShader(ResourceId shader, ResourceId texid, const Subresource &sub,
                               CompType typeCast);
  void FreeCustomShader(ResourceId id);

  void RenderCheckerboard();

  void RenderHighlightBox(float w, float h, float scale);

  uint32_t PickVertex(uint32_t eventId, int32_t width, int32_t height, const MeshDisplay &cfg,
                      uint32_t x, uint32_t y);

  struct OutputWindow
  {
    OutputWindow();

    void Create(WrappedGXM *driver, bool depth);
    void Destroy(WrappedGXM *driver);

    // implemented in vk_replay_platform.cpp
    void CreateSurface(WrappedGXM *driver);
    void SetWindowHandle(WindowingData window);

    WindowingSystem m_WindowSystem;

    WINDOW_HANDLE_DECL;

    bool fresh = true;
    bool outofdate = false;

    uint32_t width, height;

    bool hasDepth;

    int failures;
    int recreatePause;

    VkSurfaceKHR surface;
    VkSwapchainKHR swap;
    uint32_t numImgs;
    VkImage colimg[8];
    VkImageMemoryBarrier colBarrier[8];

    VkImage bb;
    VkImageView bbview;
    VkDeviceMemory bbmem;
    VkImageMemoryBarrier bbBarrier;
    VkFramebuffer fb, fbdepth;
    VkRenderPass rp, rpdepth;
    uint32_t curidx;

    VkImage resolveimg;
    VkDeviceMemory resolvemem;

    VkImage dsimg;
    VkDeviceMemory dsmem;
    VkImageView dsview;
    VkImageMemoryBarrier depthBarrier;
  };

  struct OverlayRendering
  {
    //void Init(WrappedGXM *driver, VkDescriptorPool descriptorPool);
    //void Destroy(WrappedGXM *driver);

    VkDeviceMemory ImageMem = VK_NULL_HANDLE;
    VkDeviceSize ImageMemSize = 0;
    VkImage Image = VK_NULL_HANDLE;
    VkExtent2D ImageDim = {0, 0};
    VkImageView ImageView = VK_NULL_HANDLE;
    VkFramebuffer NoDepthFB = VK_NULL_HANDLE;
    VkRenderPass NoDepthRP = VK_NULL_HANDLE;

    VkDescriptorSetLayout m_CheckerDescSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout m_CheckerPipeLayout = VK_NULL_HANDLE;
    VkDescriptorSet m_CheckerDescSet = VK_NULL_HANDLE;
    VkPipeline m_CheckerPipeline = VK_NULL_HANDLE;
    VkPipeline m_CheckerMSAAPipeline = VK_NULL_HANDLE;
    VkPipeline m_CheckerF16Pipeline[8] = {VK_NULL_HANDLE};
    GPUBuffer m_CheckerUBO;

    VkDescriptorSetLayout m_QuadDescSetLayout = VK_NULL_HANDLE;
    VkDescriptorSet m_QuadDescSet = VK_NULL_HANDLE;
    VkPipelineLayout m_QuadResolvePipeLayout = VK_NULL_HANDLE;
    VkPipeline m_QuadResolvePipeline[8] = {VK_NULL_HANDLE};

    GPUBuffer m_TriSizeUBO;
    VkDescriptorSetLayout m_TriSizeDescSetLayout = VK_NULL_HANDLE;
    VkDescriptorSet m_TriSizeDescSet = VK_NULL_HANDLE;
    VkPipelineLayout m_TriSizePipeLayout = VK_NULL_HANDLE;
  } m_Overlay;

private:
  WrappedGXM *m_pDriver;

  rdcarray<ResourceDescription> m_Resources;
  std::map<ResourceId, size_t> m_ResourceIdx;

  FrameRecord m_FrameRecord;

  std::map<uint64_t, OutputWindow> m_OutputWindows;
  uint64_t m_OutputWinID;
  uint64_t m_ActiveWinID;
  uint32_t m_DebugWidth, m_DebugHeight;

  GXMPipe::State m_PipelineState;
};
