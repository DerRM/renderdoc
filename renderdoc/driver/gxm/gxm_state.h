#pragma once

#include <map>
#include "gxm_common.h"

struct GXMVertexProgram
{
  struct Attribute
  {
    uint32_t location;
    uint32_t binding;
    SceGxmAttributeFormat format;
    uint32_t byteoffset;
  };
  rdcarray<Attribute> vertexAttrs;
};

struct GXMResources
{
  std::map<ResourceId, GXMVertexProgram> m_VertexProgram;
};

struct GXMRenderState
{
  ResourceId vprogram;

  struct IdxBuffer
  {
    ResourceId buf;
  } ibuffer;

  struct VertBuffer
  {
    ResourceId buf;
  };
  rdcarray<VertBuffer> vbuffers;
};
