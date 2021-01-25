#include "../gxm_driver.h"
#include "common/common.h"
#include "strings/string_utils.h"

#include <inttypes.h>

template <typename SerialiserType>
bool WrappedGXM::Serialise_sceGxmInitialize(SerialiserType &ser, const SceGxmInitializeParams *params)
{
  // SERIALISE_ELEMENT_LOCAL(InitParams, *params);

  return false;
}

template <typename SerialiserType>
bool WrappedGXM::Serialise_sceGxmBeginScene(
    SerialiserType &ser, SceGxmContext *context, unsigned int flags,
    const SceGxmRenderTarget *rendertarget, const SceGxmValidRegion *validRegion,
    SceGxmSyncObject *vertexSyncObject, SceGxmSyncObject *fragmentSyncObject,
    const SceGxmColorSurface *colorSurface, const SceGxmDepthStencilSurface *depthStencil)
{
  SERIALISE_ELEMENT_TYPED(uint32_t, context);
  SERIALISE_ELEMENT(flags);
  SERIALISE_ELEMENT_TYPED(uint32_t, rendertarget);
  SERIALISE_ELEMENT_TYPED(uint32_t, validRegion);
  SERIALISE_ELEMENT_TYPED(uint32_t, vertexSyncObject);
  SERIALISE_ELEMENT_TYPED(uint32_t, fragmentSyncObject);
  SERIALISE_ELEMENT_TYPED(uint32_t, colorSurface);
  SERIALISE_ELEMENT_TYPED(uint32_t, depthStencil);

  RDCLOG(
      "sceGxmBeginScene(context: 0x%x, flags: %d, rendertarget: 0x%x, validRegion: 0x%x, "
      "vertexSyncObject: 0x%x, fragmentSyncObject: 0x%x, colorSurface: 0x%x, depthStencil: 0x%x)",
      context, flags, rendertarget, validRegion, vertexSyncObject, fragmentSyncObject, colorSurface,
      depthStencil);

  return true;
}

template <typename SerialiserType>
bool WrappedGXM::Serialise_sceGxmSetVertexProgram(SerialiserType &ser, SceGxmContext *context,
                                                  const SceGxmVertexProgram *vertexProgram)
{
  SERIALISE_ELEMENT_TYPED(uint32_t, context);
  SERIALISE_ELEMENT_TYPED(uint32_t, vertexProgram);

  uint32_t attributeCount;
  SERIALISE_ELEMENT(attributeCount);

  for (uint32_t attrib_index = 0; attrib_index < attributeCount; ++attrib_index)
  {
    SceGxmAttributeFormat format;
    SERIALISE_ELEMENT(format);

    uint8_t componentCount;
    SERIALISE_ELEMENT(componentCount);

    uint16_t offset;
    SERIALISE_ELEMENT(offset);

    uint16_t streamIndex;
    SERIALISE_ELEMENT(streamIndex);

    uint16_t regIndex;
    SERIALISE_ELEMENT(regIndex);
  }

  RDCLOG("sceGxmSetVertexProgram(context: 0x%x, vertexProgram: 0x%x)", context, vertexProgram);

  return true;
}

template <typename SerialiserType>
bool WrappedGXM::Serialise_sceGxmSetFragmentProgram(SerialiserType &ser, SceGxmContext *context,
                                                    const SceGxmFragmentProgram *fragmentProgram)
{
  SERIALISE_ELEMENT_TYPED(uint32_t, context);
  SERIALISE_ELEMENT_TYPED(uint32_t, fragmentProgram);

  RDCLOG("sceGxmSetFragmentProgram(context: 0x%x, vertexProgram: 0x%x)", context, fragmentProgram);

  return true;
}

template <typename SerialiserType>
bool WrappedGXM::Serialise_sceGxmSetFrontStencilFunc(
    SerialiserType &ser, SceGxmContext *context, SceGxmStencilFunc func,
    SceGxmStencilOp stencilFail, SceGxmStencilOp depthFail, SceGxmStencilOp depthPass,
    unsigned char compareMask, unsigned char writeMask)
{
  SERIALISE_ELEMENT_TYPED(uint32_t, context);
  SERIALISE_ELEMENT(func);
  SERIALISE_ELEMENT(stencilFail);
  SERIALISE_ELEMENT(depthFail);
  SERIALISE_ELEMENT(depthPass);
  SERIALISE_ELEMENT(compareMask);
  SERIALISE_ELEMENT(writeMask);

  RDCLOG("sceGxmSetFrontStencilFunc(context: 0x%x, func: %" PRIu32 ", stencilFail: %" PRIu8
         ", depthFail: %" PRIu8 ", depthPass: %" PRIu8 ", compareMask: %" PRIu8
         ", writeMask: %" PRIu8 ")",
         context, func, stencilFail, depthFail, depthPass, compareMask, writeMask);

  return true;
}

template <typename SerialiserType>
bool WrappedGXM::Serialise_sceGxmSetFrontDepthWriteEnable(SerialiserType &ser, SceGxmContext *context,
                                                          SceGxmDepthWriteMode enable)
{
  SERIALISE_ELEMENT_TYPED(uint32_t, context);
  SERIALISE_ELEMENT(enable);

  RDCLOG("sceGxmSetFrontDepthWriteEnable(context: 0x%x, enable: %" PRIu32 ")", context, enable);

  return true;
}

template <typename SerialiserType>
bool WrappedGXM::Serialise_sceGxmSetFrontStencilRef(SerialiserType &ser, SceGxmContext *context,
                                                    unsigned int sref)
{
  SERIALISE_ELEMENT_TYPED(uint32_t, context);
  SERIALISE_ELEMENT(sref);

  RDCLOG("sceGxmSetFrontStencilRef(context: 0x%x, sref: %" PRIu32 ")", context, sref);

  return true;
}

template <typename SerialiserType>
bool WrappedGXM::Serialise_sceGxmEndScene(SerialiserType &ser, SceGxmContext *context,
                                          const SceGxmNotification *vertexNotification,
                                          const SceGxmNotification *fragmentNotification)
{
  SERIALISE_ELEMENT_TYPED(uint32_t, context);
  SERIALISE_ELEMENT_TYPED(uint32_t, vertexNotification);
  SERIALISE_ELEMENT_TYPED(uint32_t, fragmentNotification);

  RDCLOG("sceGxmEndScene(context: 0x%x, flags: %" PRIu32
         ", vertexNotification: 0x%x, fragmentNotification: 0x%x)",
         context, vertexNotification, fragmentNotification);

  return true;
}

template <typename SerialiserType>
bool WrappedGXM::Serialise_sceGxmPadHeartbeat(SerialiserType &ser,
                                              const SceGxmColorSurface *displaySurface,
                                              SceGxmSyncObject *displaySyncObject)
{
  SERIALISE_ELEMENT_TYPED(uint32_t, displaySurface);
  SERIALISE_ELEMENT_TYPED(uint32_t, displaySyncObject);

  RDCLOG("sceGxmPadHeartbeat(displaySurface: 0x%x, displaySyncObject: 0x%x)", displaySurface,
         displaySyncObject);

  return true;
}

template <typename SerialiserType>
bool WrappedGXM::Serialise_sceGxmDisplayQueueAddEntry(SerialiserType &ser,
                                                      SceGxmSyncObject *oldBuffer,
                                                      SceGxmSyncObject *newBuffer,
                                                      const void *callbackData)
{
  SERIALISE_ELEMENT_TYPED(uint32_t, oldBuffer);
  SERIALISE_ELEMENT_TYPED(uint32_t, newBuffer);
  SERIALISE_ELEMENT_TYPED(uint32_t, callbackData);

  if(IsLoading(m_State))
  {
    AddEvent();

    DrawcallDescription draw;
    draw.name = "End of Capture";
    draw.flags |= DrawFlags::Present;

    AddDrawcall(draw);
  }

  RDCLOG("sceGxmDisplayQueueAddEntry(oldBuffer: 0x%x, newBuffer: 0x%x, callbackData: 0x%x)",
         oldBuffer, newBuffer, callbackData);

  return true;
}

template <typename SerialiserType>
bool WrappedGXM::Serialise_sceGxmSetFrontDepthFunc(SerialiserType &ser, SceGxmContext *context,
                                                   SceGxmDepthFunc depthFunc)
{
  SERIALISE_ELEMENT_TYPED(uint32_t, context);
  SERIALISE_ELEMENT(depthFunc);

  RDCLOG("sceGxmSetFrontDepthFunc(context: 0x%x, depthFunc: %" PRIu32 ")", context, depthFunc);

  return true;
}

INSTANTIATE_FUNCTION_SERIALISED(int, sceGxmInitialize, const SceGxmInitializeParams *params);
INSTANTIATE_FUNCTION_SERIALISED(int, sceGxmBeginScene, SceGxmContext *context, unsigned int flags,
                                const SceGxmRenderTarget *rendertarget,
                                const SceGxmValidRegion *validRegion,
                                SceGxmSyncObject *vertexSyncObject,
                                SceGxmSyncObject *fragmentSyncObject,
                                const SceGxmColorSurface *colorSurface,
                                const SceGxmDepthStencilSurface *depthStencil);
INSTANTIATE_FUNCTION_SERIALISED(void, sceGxmSetVertexProgram, SceGxmContext *context,
                                const SceGxmVertexProgram *vertexProgram);
INSTANTIATE_FUNCTION_SERIALISED(void, sceGxmSetFragmentProgram, SceGxmContext *context,
                                const SceGxmFragmentProgram *fragmentProgram);
INSTANTIATE_FUNCTION_SERIALISED(void, sceGxmSetFrontStencilFunc, SceGxmContext *context,
                                SceGxmStencilFunc func, SceGxmStencilOp stencilFail,
                                SceGxmStencilOp depthFail, SceGxmStencilOp depthPass,
                                unsigned char compareMask, unsigned char writeMask);
INSTANTIATE_FUNCTION_SERIALISED(void, sceGxmSetFrontDepthWriteEnable, SceGxmContext *context,
                                SceGxmDepthWriteMode enable);
INSTANTIATE_FUNCTION_SERIALISED(void, sceGxmSetFrontStencilRef, SceGxmContext *context,
                                unsigned int sref);
INSTANTIATE_FUNCTION_SERIALISED(int, sceGxmEndScene, SceGxmContext *context,
                                const SceGxmNotification *vertexNotification,
                                const SceGxmNotification *fragmentNotification);
INSTANTIATE_FUNCTION_SERIALISED(int, sceGxmPadHeartbeat, const SceGxmColorSurface *displaySurface,
                                SceGxmSyncObject *displaySyncObject);
INSTANTIATE_FUNCTION_SERIALISED(int, sceGxmDisplayQueueAddEntry, SceGxmSyncObject *oldBuffer,
                                SceGxmSyncObject *newBuffer, const void *callbackData);
INSTANTIATE_FUNCTION_SERIALISED(void, sceGxmSetFrontDepthFunc, SceGxmContext *context,
                                SceGxmDepthFunc depthFunc);