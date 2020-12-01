#include "gxm_driver.h"
#include "gxm_manager.h"

template <typename SerialiserType>
bool GXMResourceManager::Serialise_InitialState(SerialiserType &ser, ResourceId id, GXMResourceRecord *record,
                                                const GXMInitialContents *initial)
{
  return false;
}

bool GXMResourceManager::Serialise_InitialState(WriteSerialiser &ser, ResourceId id, GXMResourceRecord *record,
                                                const GXMInitialContents *initial)
{
  return false;
}