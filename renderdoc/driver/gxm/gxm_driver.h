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

#include "common/common.h"
#include "core/core.h"
#include "gxm_common.h"
#include "gxm_manager.h"
#include "gxm_resources.h"
#include "gxm_state.h"

class GXMReplay;

struct GXMInitParams
{
  uint32_t width;
  uint32_t height;
  uint32_t pitch;
  uint32_t size;
  SceDisplayPixelFormat pixelformat;
};

class WrappedGXM : public IFrameCapturer
{
private:
  friend class GXMReplay;
  RDCDriver m_DriverType;

  CaptureState m_State;
  GXMReplay *m_Replay = NULL;
  rdcarray<DrawcallDescription *> m_DrawcallStack;
  uint32_t m_CurEventID, m_CurDrawcallID;
  rdcarray<APIEvent> m_CurEvents, m_Events;
  uint64_t m_CurChunkOffset;
  SDChunkMetaData m_ChunkMetadata;
  SDFile *m_StructuredFile;
  SDFile m_StoredStructuredData;
  bool m_AddedDrawcall;
  DrawcallDescription m_ParentDrawcall;
  GXMResourceManager *m_ResourceManager;
  StreamReader *m_FrameReader = NULL;
public:
  struct VulkanState
  {
    VkInstance m_Instance;
    VkPhysicalDevice m_Gpu;
    VkDevice m_Device;
    uint32_t m_QueueFamilyIndex;
    VkQueue m_Queue;
    VkDebugUtilsMessengerEXT m_DbgUtilsCallback;
  };

  WrappedGXM();
  virtual ~WrappedGXM();

  ReplayStatus Initialise();
  GXMResourceManager *GetResourceManager() { return m_ResourceManager; }
  GXMReplay *GetReplay() { return m_Replay; }

  void ReplayLog(uint32_t startEventID, uint32_t endEventID, ReplayLogType replayType);
  ReplayStatus ContextReplayLog(CaptureState readType, uint32_t startEventID, uint32_t endEventID,
                                bool partial);
  void SetDriverType(RDCDriver type) { m_DriverType = type; }
  RDCDriver GetDriverType() { return m_DriverType; }
  RDCDriver GetFrameCaptureDriver() { return GetDriverType(); }
  void StartFrameCapture(void *dev, void *wnd);
  bool EndFrameCapture(void *dev, void *wnd);
  bool DiscardFrameCapture(void *dev, void *wnd);
  ReplayStatus ReadLogInitialisation(RDCFile *rdc, bool storeStructuredBuffers);
  static rdcstr GetChunkName(uint32_t idx);
  SDFile &GetStructuredFile() { return *m_StructuredFile; }
  const DrawcallDescription &GetRootDraw() { return m_ParentDrawcall; }

  VulkanState &GetVulkanState() { return m_vulkanState; }
  uint32_t WrappedGXM::GetReadbackMemoryIndex(uint32_t resourceRequiredBitmask);
  uint32_t WrappedGXM::GetUploadMemoryIndex(uint32_t resourceRequiredBitmask);
  uint32_t WrappedGXM::GetGPULocalMemoryIndex(uint32_t resourceRequiredBitmask);
  VkCommandBuffer GetNextCmd();
  void WrappedGXM::SubmitCmds();
  void WrappedGXM::FlushQ();
  GXMVkCreationInfo m_CreationInfo;

  GXMRenderState m_RenderState;
  GXMRenderState &GetRenderState() { return m_RenderState; }
  GXMResources m_Resources;

  struct
  {
    void Reset()
    {
      cmdpool = VK_NULL_HANDLE;
      freecmds.clear();
      pendingcmds.clear();
      submittedcmds.clear();

      freesems.clear();
      pendingsems.clear();
      submittedsems.clear();
    }

    VkCommandPool cmdpool;    // the command pool used for allocating our own command buffers

    rdcarray<VkCommandBuffer> freecmds;
    // -> GetNextCmd() ->
    rdcarray<VkCommandBuffer> pendingcmds;
    // -> SubmitCmds() ->
    rdcarray<VkCommandBuffer> submittedcmds;
    // -> FlushQ() ------back to freecmds------^

    rdcarray<VkSemaphore> freesems;
    // -> GetNextSemaphore() ->
    rdcarray<VkSemaphore> pendingsems;
    // -> SubmitSemaphores() ->
    rdcarray<VkSemaphore> submittedsems;
    // -> FlushQ() ----back to freesems-------^
  } m_InternalCmds;

  VkBool32 DebugCallback(MessageSeverity severity, MessageCategory category, int messageCode,
                         const char *pMessageId, const char *pMessage);

  static VkBool32 VKAPI_PTR DebugUtilsCallbackStatic(
      VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
      VkDebugUtilsMessageTypeFlagsEXT messageTypes,
      const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);

private:
  void AddEvent();
  void AddDrawcall(const DrawcallDescription &d);
  bool ProcessChunk(ReadSerialiser &ser, GXMChunk chunk);

  void AddResource(ResourceId id, ResourceType type, const char *defaultNamePrefix);
  void AddResourceCurChunk(ResourceDescription &descr);
  void AddResourceCurChunk(ResourceId id);
  // void AddResourceInitChunk(GXMResource res);

  template <typename SerialiserType>
  bool Serialise_ContextConfiguration(SerialiserType &ser, void *ctx);

  template <typename SerialiserType>
  bool Serialise_InitBufferResource(SerialiserType &ser);

  template <typename SerialiserType>
  bool Serialise_CaptureScope(SerialiserType &ser);

  void AddRequiredExtensions(bool instance, rdcarray<rdcstr> &extensionList,
                             const std::set<rdcstr> &supportedExtensions);

  rdcarray<WindowingSystem> m_SupportedWindowSystems;
  VulkanState m_vulkanState;

  struct PhysicalDeviceData
  {
    uint32_t GetMemoryIndex(uint32_t resourceRequiredBitmask, uint32_t allocRequiredProps,
                            uint32_t allocUndesiredProps);

    uint32_t readbackMemIndex = 0;
    uint32_t uploadMemIndex = 0;
    uint32_t GPULocalMemIndex = 0;

    VkPhysicalDeviceMemoryProperties memProps = {};
  };

  PhysicalDeviceData m_PhysicalDeviceData;

public:
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmInitialize, const SceGxmInitializeParams *params);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmTerminate);
  IMPLEMENT_FUNCTION_SERIALISED(volatile unsigned int *, sceGxmGetNotificationRegion);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmNotificationWait, const SceGxmNotification *notification);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmMapMemory, void *base, SceSize size,
                                SceGxmMemoryAttribFlags attr);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmUnmapMemory, void *base);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmMapVertexUsseMemory, void *base, SceSize size,
                                unsigned int *offset);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmUnmapVertexUsseMemory, void *base);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmMapFragmentUsseMemory, void *base, SceSize size,
                                unsigned int *offset);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmUnmapFragmentUsseMemory, void *base);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmDisplayQueueAddEntry, SceGxmSyncObject *oldBuffer,
                                SceGxmSyncObject *newBuffer, const void *callbackData);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmDisplayQueueFinish);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmSyncObjectCreate, SceGxmSyncObject **syncObject);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmSyncObjectDestroy, SceGxmSyncObject *syncObject);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmCreateContext, const SceGxmContextParams *params,
                                SceGxmContext **context);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmDestroyContext, SceGxmContext *context);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmSetValidationEnable, SceGxmContext *context,
                                SceBool enable);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmSetVertexProgram, SceGxmContext *context,
                                const SceGxmVertexProgram *vertexProgram);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmSetFragmentProgram, SceGxmContext *context,
                                const SceGxmFragmentProgram *fragmentProgram);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmReserveVertexDefaultUniformBuffer,
                                SceGxmContext *context, void **uniformBuffer);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmReserveFragmentDefaultUniformBuffer,
                                SceGxmContext *context, void **uniformBuffer);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmSetVertexStream, SceGxmContext *context,
                                unsigned int streamIndex, const void *streamData);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmSetVertexTexture, SceGxmContext *context,
                                unsigned int textureIndex, const SceGxmTexture *texture);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmSetFragmentTexture, SceGxmContext *context,
                                unsigned int textureIndex, const SceGxmTexture *texture);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmSetVertexUniformBuffer, SceGxmContext *context,
                                unsigned int bufferIndex, const void *bufferData);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmSetFragmentUniformBuffer, SceGxmContext *context,
                                unsigned int bufferIndex, const void *bufferData);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmSetAuxiliarySurface, SceGxmContext *context,
                                unsigned int surfaceIndex, const SceGxmAuxiliarySurface *surface);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmSetPrecomputedFragmentState, SceGxmContext *context,
                                const SceGxmPrecomputedFragmentState *precomputedState);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmSetPrecomputedVertexState, SceGxmContext *context,
                                const SceGxmPrecomputedVertexState *precomputedState);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmDrawPrecomputed, SceGxmContext *context,
                                const SceGxmPrecomputedDraw *precomputedDraw);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmDraw, SceGxmContext *context,
                                SceGxmPrimitiveType primType, SceGxmIndexFormat indexType,
                                const void *indexData, unsigned int indexCount);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmDrawInstanced, SceGxmContext *context,
                                SceGxmPrimitiveType primType, SceGxmIndexFormat indexType,
                                const void *indexData, unsigned int indexCount,
                                unsigned int indexWrap);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmSetVisibilityBuffer, SceGxmContext *context,
                                void *bufferBase, unsigned int stridePerCore);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmBeginScene, SceGxmContext *context, unsigned int flags,
                                const SceGxmRenderTarget *renderTarget,
                                const SceGxmValidRegion *validRegion,
                                SceGxmSyncObject *vertexSyncObject,
                                SceGxmSyncObject *fragmentSyncObject,
                                const SceGxmColorSurface *colorSurface,
                                const SceGxmDepthStencilSurface *depthStencil);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmMidSceneFlush, SceGxmContext *context,
                                unsigned int flags, SceGxmSyncObject *vertexSyncObject,
                                const SceGxmNotification *vertexNotification);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmEndScene, SceGxmContext *context,
                                const SceGxmNotification *vertexNotification,
                                const SceGxmNotification *fragmentNotification);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmSetFrontDepthFunc, SceGxmContext *context,
                                SceGxmDepthFunc depthFunc);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmSetBackDepthFunc, SceGxmContext *context,
                                SceGxmDepthFunc depthFunc);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmSetFrontFragmentProgramEnable, SceGxmContext *context,
                                SceGxmFragmentProgramMode enable);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmSetBackFragmentProgramEnable, SceGxmContext *context,
                                SceGxmFragmentProgramMode enable);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmSetFrontDepthWriteEnable, SceGxmContext *context,
                                SceGxmDepthWriteMode enable);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmSetBackDepthWriteEnable, SceGxmContext *context,
                                SceGxmDepthWriteMode enable);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmSetFrontLineFillLastPixelEnable, SceGxmContext *context,
                                SceGxmLineFillLastPixelMode enable);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmSetBackLineFillLastPixelEnable, SceGxmContext *context,
                                SceGxmLineFillLastPixelMode enable);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmSetFrontStencilRef, SceGxmContext *context,
                                unsigned int sref);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmSetBackStencilRef, SceGxmContext *context,
                                unsigned int sref);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmSetFrontPointLineWidth, SceGxmContext *context,
                                unsigned int width);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmSetBackPointLineWidth, SceGxmContext *context,
                                unsigned int width);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmSetFrontPolygonMode, SceGxmContext *context,
                                SceGxmPolygonMode mode);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmSetBackPolygonMode, SceGxmContext *context,
                                SceGxmPolygonMode mode);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmSetFrontStencilFunc, SceGxmContext *context,
                                SceGxmStencilFunc func, SceGxmStencilOp stencilFail,
                                SceGxmStencilOp depthFail, SceGxmStencilOp depthPass,
                                unsigned char compareMask, unsigned char writeMask);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmSetBackStencilFunc, SceGxmContext *context,
                                SceGxmStencilFunc func, SceGxmStencilOp stencilFail,
                                SceGxmStencilOp depthFail, SceGxmStencilOp depthPass,
                                unsigned char compareMask, unsigned char writeMask);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmSetFrontDepthBias, SceGxmContext *context, int factor,
                                int units);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmSetBackDepthBias, SceGxmContext *context, int factor,
                                int units);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmSetTwoSidedEnable, SceGxmContext *context,
                                SceGxmTwoSidedMode enable);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmSetViewport, SceGxmContext *context, float xOffset,
                                float xScale, float yOffset, float yScale, float zOffset,
                                float zScale);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmSetWClampValue, SceGxmContext *context, float clampValue);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmSetWClampEnable, SceGxmContext *context,
                                SceGxmWClampMode enable);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmSetRegionClip, SceGxmContext *context,
                                SceGxmRegionClipMode mode, unsigned int xMin, unsigned int yMin,
                                unsigned int xMax, unsigned int yMax);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmSetCullMode, SceGxmContext *context, SceGxmCullMode mode);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmSetViewportEnable, SceGxmContext *context,
                                SceGxmViewportMode enable);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmSetWBufferEnable, SceGxmContext *context,
                                SceGxmWBufferMode enable);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmSetFrontVisibilityTestIndex, SceGxmContext *context,
                                unsigned int index);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmSetBackVisibilityTestIndex, SceGxmContext *context,
                                unsigned int index);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmSetFrontVisibilityTestOp, SceGxmContext *context,
                                SceGxmVisibilityTestOp op);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmSetBackVisibilityTestOp, SceGxmContext *context,
                                SceGxmVisibilityTestOp op);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmSetFrontVisibilityTestEnable, SceGxmContext *context,
                                SceGxmVisibilityTestMode enable);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmSetBackVisibilityTestEnable, SceGxmContext *context,
                                SceGxmVisibilityTestMode enable);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmFinish, SceGxmContext *context);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmPushUserMarker, SceGxmContext *context, const char *tag);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmPopUserMarker, SceGxmContext *context);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmSetUserMarker, SceGxmContext *context, const char *tag);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmPadHeartbeat, const SceGxmColorSurface *displaySurface,
                                SceGxmSyncObject *displaySyncObject);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmPadTriggerGpuPaTrace);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmColorSurfaceInit, SceGxmColorSurface *surface,
                                SceGxmColorFormat colorFormat, SceGxmColorSurfaceType surfaceType,
                                SceGxmColorSurfaceScaleMode scaleMode,
                                SceGxmOutputRegisterSize outputRegisterSize, unsigned int width,
                                unsigned int height, unsigned int strideInPixels, void *data);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmColorSurfaceInitDisabled, SceGxmColorSurface *surface);
  IMPLEMENT_FUNCTION_SERIALISED(SceBool, sceGxmColorSurfaceIsEnabled,
                                const SceGxmColorSurface *surface);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmColorSurfaceGetClip, const SceGxmColorSurface *surface,
                                unsigned int *xMin, unsigned int *yMin, unsigned int *xMax,
                                unsigned int *yMax);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmColorSurfaceSetClip, SceGxmColorSurface *surface,
                                unsigned int xMin, unsigned int yMin, unsigned int xMax,
                                unsigned int yMax);
  IMPLEMENT_FUNCTION_SERIALISED(SceGxmColorSurfaceScaleMode, sceGxmColorSurfaceGetScaleMode,
                                const SceGxmColorSurface *surface);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmColorSurfaceSetScaleMode, SceGxmColorSurface *surface,
                                SceGxmColorSurfaceScaleMode scaleMode);
  IMPLEMENT_FUNCTION_SERIALISED(void *, sceGxmColorSurfaceGetData, const SceGxmColorSurface *surface);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmColorSurfaceSetData, SceGxmColorSurface *surface,
                                void *data);
  IMPLEMENT_FUNCTION_SERIALISED(SceGxmColorFormat, sceGxmColorSurfaceGetFormat,
                                const SceGxmColorSurface *surface);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmColorSurfaceSetFormat, SceGxmColorSurface *surface,
                                SceGxmColorFormat format);
  IMPLEMENT_FUNCTION_SERIALISED(SceGxmColorSurfaceType, sceGxmColorSurfaceGetType,
                                const SceGxmColorSurface *surface);
  IMPLEMENT_FUNCTION_SERIALISED(unsigned int, sceGxmColorSurfaceGetStrideInPixels,
                                const SceGxmColorSurface *surface);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmDepthStencilSurfaceInit,
                                SceGxmDepthStencilSurface *surface,
                                SceGxmDepthStencilFormat depthStencilFormat,
                                SceGxmDepthStencilSurfaceType surfaceType,
                                unsigned int strideInSamples, void *depthData, void *stencilData);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmDepthStencilSurfaceInitDisabled,
                                SceGxmDepthStencilSurface *surface);
  IMPLEMENT_FUNCTION_SERIALISED(float, sceGxmDepthStencilSurfaceGetBackgroundDepth,
                                const SceGxmDepthStencilSurface *surface);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmDepthStencilSurfaceSetBackgroundDepth,
                                SceGxmDepthStencilSurface *surface, float backgroundDepth);
  IMPLEMENT_FUNCTION_SERIALISED(unsigned char, sceGxmDepthStencilSurfaceGetBackgroundStencil,
                                const SceGxmDepthStencilSurface *surface);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmDepthStencilSurfaceSetBackgroundStencil,
                                SceGxmDepthStencilSurface *surface, unsigned char backgroundStencil);
  IMPLEMENT_FUNCTION_SERIALISED(SceBool, sceGxmDepthStencilSurfaceIsEnabled,
                                const SceGxmDepthStencilSurface *surface);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmDepthStencilSurfaceSetForceLoadMode,
                                SceGxmDepthStencilSurface *surface,
                                SceGxmDepthStencilForceLoadMode forceLoad);
  IMPLEMENT_FUNCTION_SERIALISED(SceGxmDepthStencilForceLoadMode,
                                sceGxmDepthStencilSurfaceGetForceLoadMode,
                                const SceGxmDepthStencilSurface *surface);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmDepthStencilSurfaceSetForceStoreMode,
                                SceGxmDepthStencilSurface *surface,
                                SceGxmDepthStencilForceStoreMode forceStore);
  IMPLEMENT_FUNCTION_SERIALISED(SceGxmDepthStencilForceStoreMode,
                                sceGxmDepthStencilSurfaceGetForceStoreMode,
                                const SceGxmDepthStencilSurface *surface);
  IMPLEMENT_FUNCTION_SERIALISED(SceGxmColorSurfaceGammaMode, sceGxmColorSurfaceGetGammaMode,
                                const SceGxmColorSurface *surface);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmColorSurfaceSetGammaMode, SceGxmColorSurface *surface,
                                SceGxmColorSurfaceGammaMode gammaMode);
  IMPLEMENT_FUNCTION_SERIALISED(SceGxmColorSurfaceDitherMode, sceGxmColorSurfaceGetDitherMode,
                                const SceGxmColorSurface *surface);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmColorSurfaceSetDitherMode, SceGxmColorSurface *surface,
                                SceGxmColorSurfaceDitherMode ditherMode);
  IMPLEMENT_FUNCTION_SERIALISED(SceGxmDepthStencilFormat, sceGxmDepthStencilSurfaceGetFormat,
                                const SceGxmDepthStencilSurface *surface);
  IMPLEMENT_FUNCTION_SERIALISED(unsigned int, sceGxmDepthStencilSurfaceGetStrideInSamples,
                                const SceGxmDepthStencilSurface *surface);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmProgramCheck, const SceGxmProgram *program);
  IMPLEMENT_FUNCTION_SERIALISED(unsigned int, sceGxmProgramGetSize, const SceGxmProgram *program);
  IMPLEMENT_FUNCTION_SERIALISED(SceGxmProgramType, sceGxmProgramGetType,
                                const SceGxmProgram *program);
  IMPLEMENT_FUNCTION_SERIALISED(SceBool, sceGxmProgramIsDiscardUsed, const SceGxmProgram *program);
  IMPLEMENT_FUNCTION_SERIALISED(SceBool, sceGxmProgramIsDepthReplaceUsed,
                                const SceGxmProgram *program);
  IMPLEMENT_FUNCTION_SERIALISED(SceBool, sceGxmProgramIsSpriteCoordUsed,
                                const SceGxmProgram *program);
  IMPLEMENT_FUNCTION_SERIALISED(unsigned int, sceGxmProgramGetDefaultUniformBufferSize,
                                const SceGxmProgram *program);
  IMPLEMENT_FUNCTION_SERIALISED(unsigned int, sceGxmProgramGetParameterCount,
                                const SceGxmProgram *program);
  IMPLEMENT_FUNCTION_SERIALISED(const SceGxmProgramParameter *, sceGxmProgramGetParameter,
                                const SceGxmProgram *program, unsigned int index);
  IMPLEMENT_FUNCTION_SERIALISED(const SceGxmProgramParameter *, sceGxmProgramFindParameterByName,
                                const SceGxmProgram *program, const char *name);
  IMPLEMENT_FUNCTION_SERIALISED(const SceGxmProgramParameter *,
                                sceGxmProgramFindParameterBySemantic, const SceGxmProgram *program,
                                SceGxmParameterSemantic semantic, unsigned int index);
  IMPLEMENT_FUNCTION_SERIALISED(unsigned int, sceGxmProgramParameterGetIndex,
                                const SceGxmProgram *program,
                                const SceGxmProgramParameter *parameter);
  IMPLEMENT_FUNCTION_SERIALISED(SceGxmParameterCategory, sceGxmProgramParameterGetCategory,
                                const SceGxmProgramParameter *parameter);
  IMPLEMENT_FUNCTION_SERIALISED(const char *, sceGxmProgramParameterGetName,
                                const SceGxmProgramParameter *parameter);
  IMPLEMENT_FUNCTION_SERIALISED(SceGxmParameterSemantic, sceGxmProgramParameterGetSemantic,
                                const SceGxmProgramParameter *parameter);
  IMPLEMENT_FUNCTION_SERIALISED(unsigned int, sceGxmProgramParameterGetSemanticIndex,
                                const SceGxmProgramParameter *parameter);
  IMPLEMENT_FUNCTION_SERIALISED(SceGxmParameterType, sceGxmProgramParameterGetType,
                                const SceGxmProgramParameter *parameter);
  IMPLEMENT_FUNCTION_SERIALISED(unsigned int, sceGxmProgramParameterGetComponentCount,
                                const SceGxmProgramParameter *parameter);
  IMPLEMENT_FUNCTION_SERIALISED(unsigned int, sceGxmProgramParameterGetArraySize,
                                const SceGxmProgramParameter *parameter);
  IMPLEMENT_FUNCTION_SERIALISED(unsigned int, sceGxmProgramParameterGetResourceIndex,
                                const SceGxmProgramParameter *parameter);
  IMPLEMENT_FUNCTION_SERIALISED(unsigned int, sceGxmProgramParameterGetContainerIndex,
                                const SceGxmProgramParameter *parameter);
  IMPLEMENT_FUNCTION_SERIALISED(SceBool, sceGxmProgramParameterIsSamplerCube,
                                const SceGxmProgramParameter *parameter);
  IMPLEMENT_FUNCTION_SERIALISED(const SceGxmProgram *, sceGxmFragmentProgramGetProgram,
                                const SceGxmFragmentProgram *fragmentProgram);
  IMPLEMENT_FUNCTION_SERIALISED(const SceGxmProgram *, sceGxmVertexProgramGetProgram,
                                const SceGxmVertexProgram *vertexProgram);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmShaderPatcherCreate,
                                const SceGxmShaderPatcherParams *params,
                                SceGxmShaderPatcher **shaderPatcher);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmShaderPatcherSetUserData,
                                SceGxmShaderPatcher *shaderPatcher, void *userData);
  IMPLEMENT_FUNCTION_SERIALISED(void *, sceGxmShaderPatcherGetUserData,
                                SceGxmShaderPatcher *shaderPatcher);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmShaderPatcherDestroy, SceGxmShaderPatcher *shaderPatcher);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmShaderPatcherRegisterProgram,
                                SceGxmShaderPatcher *shaderPatcher,
                                const SceGxmProgram *programHeader, SceGxmShaderPatcherId *programId);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmShaderPatcherUnregisterProgram,
                                SceGxmShaderPatcher *shaderPatcher, SceGxmShaderPatcherId programId);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmShaderPatcherForceUnregisterProgram,
                                SceGxmShaderPatcher *shaderPatcher, SceGxmShaderPatcherId programId);
  IMPLEMENT_FUNCTION_SERIALISED(const SceGxmProgram *, sceGxmShaderPatcherGetProgramFromId,
                                SceGxmShaderPatcherId programId);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmShaderPatcherSetAuxiliarySurface,
                                SceGxmShaderPatcher *shaderPatcher, unsigned int auxSurfaceIndex,
                                const SceGxmAuxiliarySurface *auxSurface);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmShaderPatcherCreateVertexProgram,
                                SceGxmShaderPatcher *shaderPatcher, SceGxmShaderPatcherId programId,
                                const SceGxmVertexAttribute *attributes,
                                unsigned int attributeCount, const SceGxmVertexStream *streams,
                                unsigned int streamCount, SceGxmVertexProgram **vertexProgram);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmShaderPatcherCreateFragmentProgram,
                                SceGxmShaderPatcher *shaderPatcher, SceGxmShaderPatcherId programId,
                                SceGxmOutputRegisterFormat outputFormat,
                                SceGxmMultisampleMode multisampleMode,
                                const SceGxmBlendInfo *blendInfo, const SceGxmProgram *vertexProgram,
                                SceGxmFragmentProgram **fragmentProgram);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmShaderPatcherCreateMaskUpdateFragmentProgram,
                                SceGxmShaderPatcher *shaderPatcher,
                                SceGxmFragmentProgram **fragmentProgram);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmShaderPatcherAddRefVertexProgram,
                                SceGxmShaderPatcher *shaderPatcher,
                                SceGxmVertexProgram *vertexProgram);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmShaderPatcherAddRefFragmentProgram,
                                SceGxmShaderPatcher *shaderPatcher,
                                SceGxmFragmentProgram *fragmentProgram);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmShaderPatcherGetVertexProgramRefCount,
                                SceGxmShaderPatcher *shaderPatcher,
                                SceGxmVertexProgram *fragmentProgram, unsigned int *count);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmShaderPatcherGetFragmentProgramRefCount,
                                SceGxmShaderPatcher *shaderPatcher,
                                SceGxmFragmentProgram *fragmentProgram, unsigned int *count);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmShaderPatcherReleaseVertexProgram,
                                SceGxmShaderPatcher *shaderPatcher,
                                SceGxmVertexProgram *vertexProgram);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmShaderPatcherReleaseFragmentProgram,
                                SceGxmShaderPatcher *shaderPatcher,
                                SceGxmFragmentProgram *fragmentProgram);
  IMPLEMENT_FUNCTION_SERIALISED(unsigned int, sceGxmShaderPatcherGetHostMemAllocated,
                                const SceGxmShaderPatcher *shaderPatcher);
  IMPLEMENT_FUNCTION_SERIALISED(unsigned int, sceGxmShaderPatcherGetBufferMemAllocated,
                                const SceGxmShaderPatcher *shaderPatcher);
  IMPLEMENT_FUNCTION_SERIALISED(unsigned int, sceGxmShaderPatcherGetVertexUsseMemAllocated,
                                const SceGxmShaderPatcher *shaderPatcher);
  IMPLEMENT_FUNCTION_SERIALISED(unsigned int, sceGxmShaderPatcherGetFragmentUsseMemAllocated,
                                const SceGxmShaderPatcher *shaderPatcher);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmTextureInitSwizzled, SceGxmTexture *texture,
                                const void *data, SceGxmTextureFormat texFormat, unsigned int width,
                                unsigned int height, unsigned int mipCount);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmTextureInitLinear, SceGxmTexture *texture,
                                const void *data, SceGxmTextureFormat texFormat, unsigned int width,
                                unsigned int height, unsigned int mipCount);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmTextureInitLinearStrided, SceGxmTexture *texture,
                                const void *data, SceGxmTextureFormat texFormat, unsigned int width,
                                unsigned int height, unsigned int byteStride);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmTextureInitTiled, SceGxmTexture *texture,
                                const void *data, SceGxmTextureFormat texFormat, unsigned int width,
                                unsigned int height, unsigned int mipCount);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmTextureInitCube, SceGxmTexture *texture,
                                const void *data, SceGxmTextureFormat texFormat, unsigned int width,
                                unsigned int height, unsigned int mipCount);
  IMPLEMENT_FUNCTION_SERIALISED(SceGxmTextureType, sceGxmTextureGetType,
                                const SceGxmTexture *texture);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmTextureSetMinFilter, SceGxmTexture *texture,
                                SceGxmTextureFilter minFilter);
  IMPLEMENT_FUNCTION_SERIALISED(SceGxmTextureFilter, sceGxmTextureGetMinFilter,
                                const SceGxmTexture *texture);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmTextureSetMagFilter, SceGxmTexture *texture,
                                SceGxmTextureFilter magFilter);
  IMPLEMENT_FUNCTION_SERIALISED(SceGxmTextureFilter, sceGxmTextureGetMagFilter,
                                const SceGxmTexture *texture);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmTextureSetMipFilter, SceGxmTexture *texture,
                                SceGxmTextureMipFilter mipFilter);
  IMPLEMENT_FUNCTION_SERIALISED(SceGxmTextureMipFilter, sceGxmTextureGetMipFilter,
                                const SceGxmTexture *texture);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmTextureSetUAddrMode, SceGxmTexture *texture,
                                SceGxmTextureAddrMode addrMode);
  IMPLEMENT_FUNCTION_SERIALISED(SceGxmTextureAddrMode, sceGxmTextureGetUAddrMode,
                                const SceGxmTexture *texture);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmTextureSetVAddrMode, SceGxmTexture *texture,
                                SceGxmTextureAddrMode addrMode);
  IMPLEMENT_FUNCTION_SERIALISED(SceGxmTextureAddrMode, sceGxmTextureGetVAddrMode,
                                const SceGxmTexture *texture);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmTextureSetFormat, SceGxmTexture *texture,
                                SceGxmTextureFormat texFormat);
  IMPLEMENT_FUNCTION_SERIALISED(SceGxmTextureFormat, sceGxmTextureGetFormat,
                                const SceGxmTexture *texture);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmTextureSetLodBias, SceGxmTexture *texture,
                                unsigned int bias);
  IMPLEMENT_FUNCTION_SERIALISED(unsigned int, sceGxmTextureGetLodBias, const SceGxmTexture *texture);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmTextureSetStride, SceGxmTexture *texture,
                                unsigned int byteStride);
  IMPLEMENT_FUNCTION_SERIALISED(unsigned int, sceGxmTextureGetStride, const SceGxmTexture *texture);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmTextureSetWidth, SceGxmTexture *texture,
                                unsigned int width);
  IMPLEMENT_FUNCTION_SERIALISED(unsigned int, sceGxmTextureGetWidth, const SceGxmTexture *texture);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmTextureSetHeight, SceGxmTexture *texture,
                                unsigned int height);
  IMPLEMENT_FUNCTION_SERIALISED(unsigned int, sceGxmTextureGetHeight, const SceGxmTexture *texture);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmTextureSetData, SceGxmTexture *texture, const void *data);
  IMPLEMENT_FUNCTION_SERIALISED(void *, sceGxmTextureGetData, const SceGxmTexture *texture);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmTextureSetMipmapCount, SceGxmTexture *texture,
                                unsigned int mipCount);
  IMPLEMENT_FUNCTION_SERIALISED(unsigned int, sceGxmTextureGetMipmapCount,
                                const SceGxmTexture *texture);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmTextureSetPalette, SceGxmTexture *texture,
                                const void *paletteData);
  IMPLEMENT_FUNCTION_SERIALISED(void *, sceGxmTextureGetPalette, const SceGxmTexture *texture);
  IMPLEMENT_FUNCTION_SERIALISED(SceGxmTextureGammaMode, sceGxmTextureGetGammaMode,
                                const SceGxmTexture *texture);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmTextureSetGammaMode, SceGxmTexture *texture,
                                SceGxmTextureGammaMode gammaMode);
  IMPLEMENT_FUNCTION_SERIALISED(unsigned int, sceGxmGetPrecomputedVertexStateSize,
                                const SceGxmVertexProgram *vertexProgram);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmPrecomputedVertexStateInit,
                                SceGxmPrecomputedVertexState *precomputedState,
                                const SceGxmVertexProgram *vertexProgram, void *memBlock);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmPrecomputedVertexStateSetDefaultUniformBuffer,
                                SceGxmPrecomputedVertexState *precomputedState, void *defaultBuffer);
  IMPLEMENT_FUNCTION_SERIALISED(void *, sceGxmPrecomputedVertexStateGetDefaultUniformBuffer,
                                const SceGxmPrecomputedVertexState *precomputedState);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmPrecomputedVertexStateSetAllTextures,
                                SceGxmPrecomputedVertexState *precomputedState,
                                const SceGxmTexture *textures);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmPrecomputedVertexStateSetTexture,
                                SceGxmPrecomputedVertexState *precomputedState,
                                unsigned int textureIndex, const SceGxmTexture *texture);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmPrecomputedVertexStateSetAllUniformBuffers,
                                SceGxmPrecomputedVertexState *precomputedState,
                                const void *const *bufferDataArray);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmPrecomputedVertexStateSetUniformBuffer,
                                SceGxmPrecomputedVertexState *precomputedState,
                                unsigned int bufferIndex, const void *bufferData);
  IMPLEMENT_FUNCTION_SERIALISED(unsigned int, sceGxmGetPrecomputedFragmentStateSize,
                                const SceGxmFragmentProgram *fragmentProgram);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmPrecomputedFragmentStateInit,
                                SceGxmPrecomputedFragmentState *precomputedState,
                                const SceGxmFragmentProgram *fragmentProgram, void *memBlock);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmPrecomputedFragmentStateSetDefaultUniformBuffer,
                                SceGxmPrecomputedFragmentState *precomputedState,
                                void *defaultBuffer);
  IMPLEMENT_FUNCTION_SERIALISED(void *, sceGxmPrecomputedFragmentStateGetDefaultUniformBuffer,
                                const SceGxmPrecomputedFragmentState *precomputedState);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmPrecomputedFragmentStateSetAllTextures,
                                SceGxmPrecomputedFragmentState *precomputedState,
                                const SceGxmTexture *textureArray);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmPrecomputedFragmentStateSetTexture,
                                SceGxmPrecomputedFragmentState *precomputedState,
                                unsigned int textureIndex, const SceGxmTexture *texture);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmPrecomputedFragmentStateSetAllUniformBuffers,
                                SceGxmPrecomputedFragmentState *precomputedState,
                                const void *const *bufferDataArray);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmPrecomputedFragmentStateSetUniformBuffer,
                                SceGxmPrecomputedFragmentState *precomputedState,
                                unsigned int bufferIndex, const void *bufferData);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmPrecomputedFragmentStateSetAllAuxiliarySurfaces,
                                SceGxmPrecomputedFragmentState *precomputedState,
                                const SceGxmAuxiliarySurface *auxSurfaceArray);
  IMPLEMENT_FUNCTION_SERIALISED(unsigned int, sceGxmGetPrecomputedDrawSize,
                                const SceGxmVertexProgram *vertexProgram);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmPrecomputedDrawInit,
                                SceGxmPrecomputedDraw *precomputedDraw,
                                const SceGxmVertexProgram *vertexProgram, void *memBlock);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmPrecomputedDrawSetAllVertexStreams,
                                SceGxmPrecomputedDraw *precomputedDraw,
                                const void *const *streamDataArray);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmPrecomputedDrawSetVertexStream,
                                SceGxmPrecomputedDraw *precomputedDraw, unsigned int streamIndex,
                                const void *streamData);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmPrecomputedDrawSetParams,
                                SceGxmPrecomputedDraw *precomputedDraw,
                                SceGxmPrimitiveType primType, SceGxmIndexFormat indexType,
                                const void *indexData, unsigned int indexCount);
  IMPLEMENT_FUNCTION_SERIALISED(void, sceGxmPrecomputedDrawSetParamsInstanced,
                                SceGxmPrecomputedDraw *precomputedDraw, SceGxmPrimitiveType primType,
                                SceGxmIndexFormat indexType, const void *indexData,
                                unsigned int indexCount, unsigned int indexWrap);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmGetRenderTargetMemSizes,
                                const SceGxmRenderTargetParams *params, unsigned int *hostMemSize,
                                unsigned int *driverMemSize);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmCreateRenderTarget, const SceGxmRenderTargetParams *params,
                                SceGxmRenderTarget **renderTarget);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmRenderTargetGetHostMem,
                                const SceGxmRenderTarget *renderTarget, void **hostMem);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmRenderTargetGetDriverMemBlock,
                                const SceGxmRenderTarget *renderTarget, SceUID *driverMemBlock);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmDestroyRenderTarget, SceGxmRenderTarget *renderTarget);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmSetUniformDataF, void *uniformBuffer,
                                const SceGxmProgramParameter *parameter, unsigned int componentOffset,
                                unsigned int componentCount, const float *sourceData);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmTransferCopy, uint32_t width, uint32_t height,
                                uint32_t colorKeyValue, uint32_t colorKeyMask,
                                SceGxmTransferColorKeyMode colorKeyMode,
                                SceGxmTransferFormat srcFormat, SceGxmTransferType srcType,
                                const void *srcAddress, uint32_t srcX, uint32_t srcY,
                                int32_t srcStride, SceGxmTransferFormat destFormat,
                                SceGxmTransferType destType, void *destAddress, uint32_t destX,
                                uint32_t destY, int32_t destStride, SceGxmSyncObject *syncObject,
                                uint32_t syncFlags, const SceGxmNotification *notification);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmTransferDownscale, SceGxmTransferFormat srcFormat,
                                const void *srcAddress, unsigned int srcX, unsigned int srcY,
                                unsigned int srcWidth, unsigned int srcHeight, int srcStride,
                                SceGxmTransferFormat destFormat, void *destAddress,
                                unsigned int destX, unsigned int destY, int destStride,
                                SceGxmSyncObject *syncObject, unsigned int syncFlags,
                                const SceGxmNotification *notification);
  IMPLEMENT_FUNCTION_SERIALISED(int, sceGxmTransferFinish);
};
