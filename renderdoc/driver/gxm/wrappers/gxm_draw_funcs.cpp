#include <inttypes.h>

#include "../gxm_driver.h"
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
  SERIALISE_ELEMENT_TYPED(uint32_t, primType);
  SERIALISE_ELEMENT_TYPED(uint32_t, indexType);
  SERIALISE_ELEMENT_TYPED(uint32_t, indexData);
  SERIALISE_ELEMENT(indexCount);

  SERIALISE_CHECK_READ_ERRORS();

  RDCLOG("sceGxmDraw(context: 0x%x, primType: %" PRIu32 ", indexType: %" PRIu32
      ", indexData: 0x%x, indexCount: %" PRIu32 ")",
      context, primType, indexType, indexData, indexCount);

  if(ser.IsReading() && IsReplayMode(m_State))
  {
    if (IsLoading(m_State))
    {
      uint32_t IdxSize = GetIdxSize(indexType);

      DrawcallDescription draw;
      draw.name = "sceGxmDraw";
      draw.numIndices = indexCount;
      draw.numInstances = 1;
      draw.vertexOffset = 0;
      draw.instanceOffset = 0;

      draw.flags |= DrawFlags::Drawcall;

      draw.topology = MakePrimitiveTopology(primType);
      draw.indexByteWidth = IdxSize;

      draw.eventId = m_CurEventID;

      AddDrawcall(draw);

      m_CurEventID++;
    }
  }

  return true;
}

INSTANTIATE_FUNCTION_SERIALISED(int, sceGxmDraw, SceGxmContext *context,
                                SceGxmPrimitiveType primType, SceGxmIndexFormat indexType,
                                const void *indexData, unsigned int indexCount);
