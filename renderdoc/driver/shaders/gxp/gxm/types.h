#pragma once

#include <array>

#define SCE_GXM_REAL_MAX_UNIFORM_BUFFER (SCE_GXM_MAX_UNIFORM_BUFFERS + 1)    // Include default

namespace gxp
{
// Used to map GXM program parameters to GLSL data types
enum class GenericParameterType
{
  Scalar,
  Vector,
  Matrix,
  Array
};
}    // namespace gxp

enum SceGxmVertexProgramOutputs : int
{
  _SCE_GXM_VERTEX_PROGRAM_OUTPUT_INVALID = 0,

  SCE_GXM_VERTEX_PROGRAM_OUTPUT_POSITION = 1 << 0,
  SCE_GXM_VERTEX_PROGRAM_OUTPUT_FOG = 1 << 1,
  SCE_GXM_VERTEX_PROGRAM_OUTPUT_COLOR0 = 1 << 2,
  SCE_GXM_VERTEX_PROGRAM_OUTPUT_COLOR1 = 1 << 3,
  SCE_GXM_VERTEX_PROGRAM_OUTPUT_TEXCOORD0 = 1 << 4,
  SCE_GXM_VERTEX_PROGRAM_OUTPUT_TEXCOORD1 = 1 << 5,
  SCE_GXM_VERTEX_PROGRAM_OUTPUT_TEXCOORD2 = 1 << 6,
  SCE_GXM_VERTEX_PROGRAM_OUTPUT_TEXCOORD3 = 1 << 7,
  SCE_GXM_VERTEX_PROGRAM_OUTPUT_TEXCOORD4 = 1 << 8,
  SCE_GXM_VERTEX_PROGRAM_OUTPUT_TEXCOORD5 = 1 << 9,
  SCE_GXM_VERTEX_PROGRAM_OUTPUT_TEXCOORD6 = 1 << 10,
  SCE_GXM_VERTEX_PROGRAM_OUTPUT_TEXCOORD7 = 1 << 11,
  SCE_GXM_VERTEX_PROGRAM_OUTPUT_TEXCOORD8 = 1 << 12,
  SCE_GXM_VERTEX_PROGRAM_OUTPUT_TEXCOORD9 = 1 << 13,
  SCE_GXM_VERTEX_PROGRAM_OUTPUT_PSIZE = 1 << 14,
  SCE_GXM_VERTEX_PROGRAM_OUTPUT_CLIP0 = 1 << 15,
  SCE_GXM_VERTEX_PROGRAM_OUTPUT_CLIP1 = 1 << 16,
  SCE_GXM_VERTEX_PROGRAM_OUTPUT_CLIP2 = 1 << 17,
  SCE_GXM_VERTEX_PROGRAM_OUTPUT_CLIP3 = 1 << 18,
  SCE_GXM_VERTEX_PROGRAM_OUTPUT_CLIP4 = 1 << 19,
  SCE_GXM_VERTEX_PROGRAM_OUTPUT_CLIP5 = 1 << 20,
  SCE_GXM_VERTEX_PROGRAM_OUTPUT_CLIP6 = 1 << 21,
  SCE_GXM_VERTEX_PROGRAM_OUTPUT_CLIP7 = 1 << 22,

  _SCE_GXM_VERTEX_PROGRAM_OUTPUT_LAST = 1 << 23
};

enum SceGxmFragmentProgramInputs : int
{
  _SCE_GXM_FRAGMENT_PROGRAM_INPUT_NONE = 0,

  SCE_GXM_FRAGMENT_PROGRAM_INPUT_POSITION = 1 << 0,
  SCE_GXM_FRAGMENT_PROGRAM_INPUT_FOG = 1 << 1,
  SCE_GXM_FRAGMENT_PROGRAM_INPUT_COLOR0 = 1 << 2,
  SCE_GXM_FRAGMENT_PROGRAM_INPUT_COLOR1 = 1 << 3,
  SCE_GXM_FRAGMENT_PROGRAM_INPUT_TEXCOORD0 = 1 << 4,
  SCE_GXM_FRAGMENT_PROGRAM_INPUT_TEXCOORD1 = 1 << 5,
  SCE_GXM_FRAGMENT_PROGRAM_INPUT_TEXCOORD2 = 1 << 6,
  SCE_GXM_FRAGMENT_PROGRAM_INPUT_TEXCOORD3 = 1 << 7,
  SCE_GXM_FRAGMENT_PROGRAM_INPUT_TEXCOORD4 = 1 << 8,
  SCE_GXM_FRAGMENT_PROGRAM_INPUT_TEXCOORD5 = 1 << 9,
  SCE_GXM_FRAGMENT_PROGRAM_INPUT_TEXCOORD6 = 1 << 10,
  SCE_GXM_FRAGMENT_PROGRAM_INPUT_TEXCOORD7 = 1 << 11,
  SCE_GXM_FRAGMENT_PROGRAM_INPUT_TEXCOORD8 = 1 << 12,
  SCE_GXM_FRAGMENT_PROGRAM_INPUT_TEXCOORD9 = 1 << 13,
  SCE_GXM_FRAGMENT_PROGRAM_INPUT_SPRITECOORD = 1 << 14,

  _SCE_GXM_FRAGMENT_PROGRAM_INPUT_LAST = 1 << 15
};

struct SceGxmProgramParameterContainer
{
  uint16_t container_index;
  uint16_t unk02;
  uint16_t base_sa_offset;
  uint16_t size_in_f32;
};

using SceGxmVertexOutputTexCoordInfos = std::array<uint8_t, 10>;

#pragma pack(push, 1)
struct SceGxmProgramVertexVaryings
{
  std::uint8_t unk0[8];
  std::uint8_t fragment_output_start;    // this might be wrong
  std::uint8_t unk1;
  std::uint8_t output_param_type;
  std::uint8_t output_comp_count;

  std::uint16_t varyings_count;
  std::uint16_t pad0;               // padding maybe
  std::uint32_t vertex_outputs1;    // includes everything except texcoord outputs
  std::uint32_t vertex_outputs2;    // includes texcoord outputs
  std::uint32_t unk18;
  std::uint16_t semantic_index_offset;
  std::uint16_t semantic_instance_offset;
};
#pragma pack(pop)

static_assert(sizeof(SceGxmProgramVertexVaryings) == 32);

struct SceGxmProgram
{
  std::uint32_t magic;    // should be "GXP\0"

  std::uint8_t major_version;    // min 1
  std::uint8_t minor_version;    // min 4
  std::uint16_t unk6;            // maybe padding

  std::uint32_t
      size;    // size of file - ignoring padding bytes at the end after SceGxmProgramParameter table
  std::uint32_t unkC;

  std::uint16_t unk10;
  std::uint8_t unk12;
  std::uint8_t unk13;

  std::uint8_t type{
      0};    // shader profile, seems to contain more info in bits after the first(bitfield?)
  std::uint8_t unk15;
  std::uint8_t special_flags;
  std::uint8_t unk17;

  std::uint32_t unk18;
  std::uint32_t unk1C;

  std::uint32_t unk20;    // bit 6 denotes whether a frag shader writes directly to output (usees
                          // __nativecolor modifier) or not
  std::uint32_t parameter_count;
  std::uint32_t parameters_offset;    // Number of bytes from the start of this field to the first parameter.
  std::uint32_t varyings_offset;    // offset to vertex outputs / fragment inputs, relative to this field

  std::uint16_t primary_reg_count;      // (PAs)
  std::uint16_t secondary_reg_count;    // (SAs)
  std::uint16_t temp_reg_count1;        // not sure // - verify this
  std::uint16_t unk36;
  std::uint16_t temp_reg_count2;    // not sure // - verify this
  std::uint16_t unk3A;              // some item count?

  std::uint32_t primary_program_instr_count;
  std::uint32_t primary_program_offset;

  std::uint32_t unk44;

  std::uint32_t secondary_program_offset;        // relative to the beginning of this field
  std::uint32_t secondary_program_offset_end;    // relative to the beginning of this field

  std::uint32_t unk_50;    // usually zero?
  std::uint32_t unk_54;    // usually zero?
  std::uint32_t unk_58;    // usually zero?
  std::uint32_t unk_5C;    // usually zero?

  std::uint32_t unk_60;
  std::uint32_t default_uniform_buffer_count;
  std::uint32_t unk_68;
  std::uint32_t unk_6C;

  std::uint32_t literals_count;
  std::uint32_t literals_offset;
  std::uint32_t uniform_buffer_count;
  std::uint32_t uniform_buffer_offset;

  std::uint32_t dependent_sampler_count;
  std::uint32_t dependent_sampler_offset;
  std::uint32_t unk_88;
  std::uint32_t unk_8C;
  std::uint32_t container_count;
  std::uint32_t container_offset;

  SceGxmProgramType get_type() const { return static_cast<SceGxmProgramType>(type & 1); }
  bool is_vertex() const { return get_type() == SceGxmProgramType::SCE_GXM_VERTEX_PROGRAM; }
  bool is_fragment() const { return get_type() == SceGxmProgramType::SCE_GXM_FRAGMENT_PROGRAM; }
  uint64_t *primary_program_start() const
  {
    return (uint64_t *)((uint8_t *)&primary_program_offset + primary_program_offset);
  }
  uint64_t *secondary_program_start() const
  {
    return (uint64_t *)((uint8_t *)&secondary_program_offset + secondary_program_offset);
  }
  uint64_t *secondary_program_end() const
  {
    return (uint64_t *)((uint8_t *)&secondary_program_offset_end + secondary_program_offset_end);
  }
  bool is_discard_used() const { return ((type >> 3) & 1); }
  bool is_depth_replace_used() const { return ((type >> 4) & 1); }
  bool is_sprite_coord_used() const { return ((type >> 5) & 1); }
  bool is_native_color() const { return ((type >> 6) & 1); }
  bool is_frag_color_used() const { return ((type >> 7) & 1); }
  SceGxmParameterType get_fragment_output_type() const
  {
    return static_cast<const SceGxmParameterType>(
        reinterpret_cast<const SceGxmProgramVertexVaryings *>(
            reinterpret_cast<const std::uint8_t *>(&varyings_offset) + varyings_offset)
            ->output_param_type);
  }
  std::uint8_t get_fragment_output_component_count() const
  {
    return reinterpret_cast<const SceGxmProgramVertexVaryings *>(
               reinterpret_cast<const std::uint8_t *>(&varyings_offset) + varyings_offset)
        ->output_comp_count;
  }
  bool is_secondary_program_available() const
  {
    return secondary_program_offset < secondary_program_offset_end + 4;
  }
};

struct SceGxmProgramParameter
{
  int32_t name_offset;    // Number of bytes from the start of this structure to the name string.
  
  uint16_t category : 4;    // SceGxmParameterCategory
  uint16_t type : 4;    // SceGxmParameterType - applicable for constants, not applicable
                                  // for samplers (select type like float, half, fixed ...)
  uint16_t component_count : 4;    // applicable for constants, not applicable for samplers
                                             // (select size like float2, float3, float3 ...)
  uint16_t container_index : 4;    // applicable for constants, not applicable for
                                              // samplers (buffer, default, texture)
  uint8_t semantic;    // applicable only for for vertex attributes, for everything else it's 0
  uint8_t semantic_index;
  uint32_t array_size;
  int32_t resource_index;

  bool is_sampler_cube() const { return (semantic_index >> 4) & 1; }
};

struct SceGxmUniformBufferInfo
{
  std::uint16_t reside_buffer;    ///< If reside buffer = 0, this is a in memory buffer. Likely SSBO
  std::uint16_t ldst_base_offset;    ///< Point to the register number starting from container's
                                     ///< base SA offset, storing base value.
  std::int32_t ldst_base_value;    ///< Value representing the starting offset in bytes to load/store buffer data.
};

struct SceGxmDependentSampler
{
  std::uint16_t resource_index_layout_offset;    ///< The resource index of the sampler, in range of [index * 4, (index + 1) * 4)
  std::uint16_t sa_offset;    ///< The SA offset correspond to the sampler
};

enum SceGxmSpecialFlag
{
  SCE_GXM_SPECIAL_HAS_INDEX_SEMANTIC = 1 << 1,
  SCE_GXM_SPECIAL_HAS_INSTANCE_SEMANTIC = 1 << 2
};

struct SceGxmProgramAttributeDescriptor
{
  std::uint32_t attribute_info;
  std::uint32_t resource_index;
  std::uint32_t size;
  std::uint32_t component_info;    ///< Total components and type
};
