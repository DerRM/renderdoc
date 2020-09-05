#include "../gxm_driver.h"
#include "common/common.h"
#include "strings/string_utils.h"
#include <inttypes.h>

template <typename SerialiserType>
bool WrappedGXM::Serialise_sceGxmDraw(SerialiserType &ser, SceGxmContext *context,
                                      SceGxmPrimitiveType primType, SceGxmIndexFormat indexType,
                                      const void *indexData, unsigned int indexCount)
{
  SERIALISE_ELEMENT_TYPED(uint32_t, primType);
  SERIALISE_ELEMENT_TYPED(uint32_t, indexType);
  SERIALISE_ELEMENT(indexCount);
  RDCLOG("sceGxmDraw, primType: %" PRIu32 ", indexType: %" PRIu32 ", indexCount: %" PRIu32,
         primType, indexType, indexCount);
  return true;
}

INSTANTIATE_FUNCTION_SERIALISED(int, sceGxmDraw, SceGxmContext *context,
                                SceGxmPrimitiveType primType, SceGxmIndexFormat indexType,
                                const void *indexData, unsigned int indexCount);
