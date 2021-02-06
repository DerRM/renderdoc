#include <inttypes.h>

#include "../gxm_driver.h"
#include "../gxm_replay.h"
#include "common/common.h"
#include "strings/string_utils.h"

static constexpr uint32_t GetIdxSize(SceGxmIndexFormat idxtype)
{
  switch(idxtype)
  {
    default: return 0;
    case SCE_GXM_INDEX_FORMAT_U16: return 2;
    case SCE_GXM_INDEX_FORMAT_U32: return 4;
  }
}

template <typename SerialiserType>
bool WrappedGXM::Serialise_sceGxmDraw(SerialiserType &ser, SceGxmContext *context,
                                      SceGxmPrimitiveType primType, SceGxmIndexFormat indexType,
                                      const void *indexData, unsigned int indexCount)
{
  SERIALISE_ELEMENT_TYPED(uint32_t, context);
  SERIALISE_ELEMENT(primType);
  SERIALISE_ELEMENT(indexType);
  SERIALISE_ELEMENT(indexCount);

  GXMRenderState &state = GetRenderState();

  uint32_t index_type_size = 0;

  if(indexType == SCE_GXM_INDEX_FORMAT_U16)
  {
    index_type_size = 2;
  }
  else
  {
    index_type_size = 4;
  }

  uint32_t indexbufferSize = indexCount * index_type_size;

  SERIALISE_ELEMENT_ARRAY(indexData, indexbufferSize);

  uint32_t max_index = 0;

  switch(indexType)
  {
    case SCE_GXM_INDEX_FORMAT_U16:
    {
      uint16_t *indices = (uint16_t *)indexData;
      for(uint32_t index = 0; index < indexCount; ++index)
      {
        if(max_index < indices[index])
        {
          max_index = indices[index];
        }
      }
    }
    break;
    case SCE_GXM_INDEX_FORMAT_U32:
    {
      uint32_t *indices = (uint32_t *)indexData;
      for(uint32_t index = 0; index < indexCount; ++index)
      {
        if(max_index < indices[index])
        {
          max_index = indices[index];
        }
      }
    }
    break;
    default: break;
  }

  ResourceId indexbuffer;
  SERIALISE_ELEMENT(indexbuffer);

  GXMResource index_res = GetResourceManager()->GetLiveResource(indexbuffer); 

  void *index_data;
  vkMapMemory(m_vulkanState.m_Device, index_res.memory, 0, index_res.size, 0, &index_data);
  memcpy(index_data, indexData, indexbufferSize);
  vkUnmapMemory(m_vulkanState.m_Device, index_res.memory);

  m_RenderState.ibuffer.buf = GetResourceManager()->GetLiveID(indexbuffer);

  uint32_t streamCount;
  SERIALISE_ELEMENT(streamCount);

  state.vbuffers.resize(streamCount);

  for(uint32_t stream_index = 0; stream_index < streamCount; ++stream_index)
  {
    uint32_t vertexBufferSize;
    SERIALISE_ELEMENT(vertexBufferSize);
    const void *vertexData;
    SERIALISE_ELEMENT_ARRAY(vertexData, vertexBufferSize);

    ResourceId vertexBuffer;
    SERIALISE_ELEMENT(vertexBuffer);

    GXMResource vertex_res = GetResourceManager()->GetLiveResource(vertexBuffer);
    
    void *vertex_data;
    vkMapMemory(m_vulkanState.m_Device, vertex_res.memory, 0, vertex_res.size, 0, &vertex_data);
    memcpy(vertex_data, vertexData, vertexBufferSize);
    vkUnmapMemory(m_vulkanState.m_Device, vertex_res.memory);

    state.vbuffers[stream_index].buf = GetResourceManager()->GetLiveID(vertexBuffer);
    state.vbuffers[stream_index].stride = vertexBufferSize / (max_index + 1);
    state.vbuffers[stream_index].offset = 0;
  }

  SERIALISE_CHECK_READ_ERRORS();

  RDCLOG("sceGxmDraw(context: 0x%x, primType: %" PRIu32 ", indexType: %" PRIu32
         ", indexData: 0x%x, indexCount: %" PRIu32 ")",
         context, primType, indexType, indexData, indexCount);

  if(ser.IsReading() && IsReplayMode(m_State))
  {
    if(IsLoading(m_State))
    {
      AddEvent();

      uint32_t IdxSize = GetIdxSize(indexType);

      DrawcallDescription draw;
      draw.name = "sceGxmDraw";
      draw.numIndices = indexCount;
      draw.numInstances = 1;
      draw.vertexOffset = 0;
      draw.instanceOffset = 0;

      draw.flags |= DrawFlags::Drawcall | DrawFlags::Indexed;

      draw.topology = MakePrimitiveTopology(primType);
      draw.indexByteWidth = IdxSize;

      AddDrawcall(draw);
    }
  }

  return true;
}

INSTANTIATE_FUNCTION_SERIALISED(int, sceGxmDraw, SceGxmContext *context,
                                SceGxmPrimitiveType primType, SceGxmIndexFormat indexType,
                                const void *indexData, unsigned int indexCount);
