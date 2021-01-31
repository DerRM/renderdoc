#pragma once

struct MeshDisplayPipelines
{
  enum
  {
    ePipe_Wire = 0,
    ePipe_WireDepth,
    ePipe_Solid,
    ePipe_SolidDepth,
    ePipe_Lit,
    ePipe_Secondary,
    ePipe_Count,
  };

  VkPipeline pipes[ePipe_Count] = {};
};

class GXMDebugManager
{
public:
  GXMDebugManager(WrappedGXM *driver);
  ~GXMDebugManager();

  MeshDisplayPipelines CacheMeshDisplayPipelines(VkPipelineLayout pipeLayout,
                                                 const MeshFormat &primary,
                                                 const MeshFormat &secondary);
};
