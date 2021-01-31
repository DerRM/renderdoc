#pragma once

#include <map>
#include "gxm_common.h"

struct GXMVertexProgram
{
  struct Attribute
  {
    SceGxmAttributeFormat format;
    uint8_t componentCount;
    uint16_t offset;
    uint16_t streamIndex;
    uint16_t regIndex;
  };
  rdcarray<Attribute> vertexAttrs;
};

struct GXMResources
{
  std::map<SceGxmVertexProgram const*, GXMVertexProgram> m_VertexProgram;
};

struct GXMRenderState
{
  SceGxmVertexProgram const* vprogram;

  struct IdxBuffer
  {
    ResourceId buf;
  } ibuffer;

  struct VertBuffer
  {
    ResourceId buf;
    uint32_t stride;
    uint32_t offset;
  };
  rdcarray<VertBuffer> vbuffers;
};
