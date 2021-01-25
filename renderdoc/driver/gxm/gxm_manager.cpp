#include "gxm_manager.h"

GXMResourceManager::GXMResourceManager(CaptureState &state)
  : ResourceManager(state)
{

}

bool GXMResourceManager::ResourceTypeRelease(GXMResource res)
{
  return false;
}

bool GXMResourceManager::Prepare_InitialState(GXMResource res)
{
  return false;
}

uint64_t GXMResourceManager::GetSize_InitialState(ResourceId resid, const GXMInitialContents &initial)
{
  return uint64_t(0);
}

void GXMResourceManager::Create_InitialState(ResourceId id, GXMResource live, bool hasData)
{

}

void GXMResourceManager::Apply_InitialState(GXMResource live, const GXMInitialContents &initial)
{

}

GXMResource GXMResourceManager::FindInBuffer(uint32_t size, uint32_t addr)
{
  std::map<ResourceId, GXMResource>::const_iterator it;
  for(it = m_LiveResourceMap.cbegin(); it != m_LiveResourceMap.cend(); it++)
  {
    if (it->second.Namespace == GXMNamespace::eResMappedBuffer)
    {
      if (it->second.addr <= addr && (it->second.addr + it->second.size) >= (addr + size))
      {
        return it->second;
      }
    }
  }

  return GXMResource();
}
