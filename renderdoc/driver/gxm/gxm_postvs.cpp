#include "gxm_replay.h"
#include "3rdparty/glslang/SPIRV/SpvBuilder.h"

void GXMReplay::InitPostVSBuffers(uint32_t eventId) {

}

void GXMReplay::InitPostVSBuffers(const rdcarray<uint32_t> &passEvents) {

}

MeshFormat GXMReplay::GetPostVSBuffers(uint32_t eventId, uint32_t instID, uint32_t viewID,
                                       MeshDataStage stage)
{

  spv::SpvBuildLogger spv_logger;
  spv::Builder builder(SPV_VERSION, 0x1337 << 12, &spv_logger);
  (void)builder;

  return MeshFormat();
}