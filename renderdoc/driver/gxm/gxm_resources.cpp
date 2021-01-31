#include "gxm_resources.h"

ResourceFormat MakeAttributeFormat(SceGxmAttributeFormat fmt, uint8_t componentCount)
{
  ResourceFormat ret;

  ret.type = ResourceFormatType::Regular;
  ret.compByteWidth = 0;
  ret.compCount = componentCount;
  ret.compType = CompType::Typeless;

  if (fmt == SCE_GXM_ATTRIBUTE_FORMAT_UNTYPED)
  {
    return ret;
  }

  switch(fmt)
  {
    case SCE_GXM_ATTRIBUTE_FORMAT_U8:
    case SCE_GXM_ATTRIBUTE_FORMAT_S8:
    case SCE_GXM_ATTRIBUTE_FORMAT_U16:
    case SCE_GXM_ATTRIBUTE_FORMAT_S16:
    case SCE_GXM_ATTRIBUTE_FORMAT_U8N:
    case SCE_GXM_ATTRIBUTE_FORMAT_S8N:
    case SCE_GXM_ATTRIBUTE_FORMAT_U16N:
    case SCE_GXM_ATTRIBUTE_FORMAT_S16N:
    case SCE_GXM_ATTRIBUTE_FORMAT_F16:
    case SCE_GXM_ATTRIBUTE_FORMAT_F32:
    default: break;
  }

  switch(fmt)
  {
    case SCE_GXM_ATTRIBUTE_FORMAT_U8:
    case SCE_GXM_ATTRIBUTE_FORMAT_U16: ret.compType = CompType::UInt; break;
    case SCE_GXM_ATTRIBUTE_FORMAT_S8:
    case SCE_GXM_ATTRIBUTE_FORMAT_S16: ret.compType = CompType::SInt; break;
    case SCE_GXM_ATTRIBUTE_FORMAT_U8N:
    case SCE_GXM_ATTRIBUTE_FORMAT_U16N: ret.compType = CompType::UNorm; break;
    case SCE_GXM_ATTRIBUTE_FORMAT_S8N:
    case SCE_GXM_ATTRIBUTE_FORMAT_S16N: ret.compType = CompType::SNorm; break;
    case SCE_GXM_ATTRIBUTE_FORMAT_F16:
    case SCE_GXM_ATTRIBUTE_FORMAT_F32: ret.compType = CompType::Float; break;
    default: break;
  }

  switch(fmt)
  {
    case SCE_GXM_ATTRIBUTE_FORMAT_U8:
    case SCE_GXM_ATTRIBUTE_FORMAT_S8:
    case SCE_GXM_ATTRIBUTE_FORMAT_U8N:
    case SCE_GXM_ATTRIBUTE_FORMAT_S8N: ret.compByteWidth = 1; break;
    case SCE_GXM_ATTRIBUTE_FORMAT_U16:
    case SCE_GXM_ATTRIBUTE_FORMAT_S16:
    case SCE_GXM_ATTRIBUTE_FORMAT_U16N:
    case SCE_GXM_ATTRIBUTE_FORMAT_S16N:
    case SCE_GXM_ATTRIBUTE_FORMAT_F16: ret.compByteWidth = 2; break;
    case SCE_GXM_ATTRIBUTE_FORMAT_F32: ret.compByteWidth = 4; break;
    default: break;
  }

  return ret;
}