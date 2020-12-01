#pragma once

#include "core/resource_manager.h"
#include "gxm_initstate.h"
#include "gxm_resources.h"

class WrappedGXM;

struct GXMResourceManagerConfiguration
{
  typedef GXMResource WrappedResourceType;
  typedef GXMResource RealResourceType;
  typedef GXMResourceRecord RecordType;
  typedef GXMInitialContents InitialContentData;
};

class GXMResourceManager : public ResourceManager<GXMResourceManagerConfiguration>
{
public:
  GXMResourceManager(CaptureState &state);
  ~GXMResourceManager() {}

  GXMResourceRecord *GetResourceRecord(GXMResource res)
  {
    auto it = m_GXMResourceRecords.find(res);
    if(it != m_GXMResourceRecords.end())
      return it->second;

    return ResourceManager::GetResourceRecord(GetID(res));
  }

  ResourceId GetID(GXMResource res) 
  {
    auto it = m_CurrentResourceIds.find(res);
    if(it != m_CurrentResourceIds.end())
      return it->second;
    return ResourceId();
  }

  template <typename SerialiserType>
  bool Serialise_InitialState(SerialiserType &ser, ResourceId id, GXMResourceRecord *record,
                              const GXMInitialContents *initial);
  bool Serialise_InitialState(WriteSerialiser &ser, ResourceId id, GXMResourceRecord *record,
                              const GXMInitialContents *initial);

private:
  bool ResourceTypeRelease(GXMResource res);
  bool Prepare_InitialState(GXMResource res);
  uint64_t GetSize_InitialState(ResourceId resid, const GXMInitialContents &initial);

  void Create_InitialState(ResourceId id, GXMResource live, bool hasData);
  void Apply_InitialState(GXMResource live, const GXMInitialContents &initial);

  std::map<GXMResource, GXMResourceRecord *> m_GXMResourceRecords;

  std::map<GXMResource, ResourceId> m_CurrentResourceIds;
};