#include "gxm_replay.h"
#include "3rdparty/glslang/SPIRV/SpvBuilder.h"

#include <driver/shaders/gxp/shader/spirv_recompiler.h>
#include <driver/shaders/gxp/gxm/types.h>

#include <array>
#include <queue>
#include <bitset>

void GXMReplay::InitPostVSBuffers(uint32_t eventId) {

}

void GXMReplay::InitPostVSBuffers(const rdcarray<uint32_t> &passEvents) {

}

MeshFormat GXMReplay::GetPostVSBuffers(uint32_t eventId, uint32_t instID, uint32_t viewID,
                                       MeshDataStage stage)
{

  rdcarray<rdcarray<uint8_t>> shaders = m_pDriver->GetShaders();

  if(shaders.size() >= 1)
  {
    SceGxmProgram *program = (SceGxmProgram*)&shaders[4][0];
    SceGxmProgramType type = program->get_type();
    (void)type;

    FeatureState features;
    shader::usse::SpirvCode spirv = shader::convert_gxp_to_spirv(*program, "shader", features);

   // const SceGxmProgramParameter* parameters = program_parameters(*program);
  //  for(uint32_t param_index = 0; param_index < program->parameter_count; ++param_index)
 //   {
  //    const SceGxmProgramParameter &param = parameters[param_index];

  //    RDCLOG("parameter name: %s", parameter_name_raw(param).c_str());
  //  }
  }

  return MeshFormat();
}