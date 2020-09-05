#include "../gxm_driver.h"
#include "common/common.h"
#include "strings/string_utils.h"

template <typename SerialiserType>
bool WrappedGXM::Serialise_sceGxmInitialize(SerialiserType &ser, const SceGxmInitializeParams *params)
{
  //SERIALISE_ELEMENT_LOCAL(InitParams, *params);

  return false;
}

INSTANTIATE_FUNCTION_SERIALISED(int, sceGxmInitialize, const SceGxmInitializeParams *params);
