#include "gxm_resources.h"
#include "gxm_driver.h"

ResourceFormat MakeAttributeFormat(SceGxmAttributeFormat fmt, uint8_t componentCount)
{
  ResourceFormat ret;

  ret.type = ResourceFormatType::Regular;
  ret.compByteWidth = 0;
  ret.compCount = componentCount;
  ret.compType = CompType::Typeless;

  if (fmt == SCE_GXM_ATTRIBUTE_FORMAT_UNTYPED)
  {
    ret.compType = CompType::Float;
    ret.compCount = 4;
    ret.compByteWidth = 2;
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

VkFormat MakeGXMVkFormat(ResourceFormat fmt)
{
  VkFormat ret = VK_FORMAT_UNDEFINED;

  if(fmt.Special())
  {
    switch(fmt.type)
    {
      case ResourceFormatType::Undefined: return ret;
      case ResourceFormatType::BC1:
      {
        if(fmt.compCount == 3)
          ret = fmt.SRGBCorrected() ? VK_FORMAT_BC1_RGB_SRGB_BLOCK : VK_FORMAT_BC1_RGB_UNORM_BLOCK;
        else
          ret = fmt.SRGBCorrected() ? VK_FORMAT_BC1_RGBA_SRGB_BLOCK : VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        break;
      }
      case ResourceFormatType::BC2:
        ret = fmt.SRGBCorrected() ? VK_FORMAT_BC2_SRGB_BLOCK : VK_FORMAT_BC2_UNORM_BLOCK;
        break;
      case ResourceFormatType::BC3:
        ret = fmt.SRGBCorrected() ? VK_FORMAT_BC3_SRGB_BLOCK : VK_FORMAT_BC3_UNORM_BLOCK;
        break;
      case ResourceFormatType::BC4:
        ret = fmt.compType == CompType::SNorm ? VK_FORMAT_BC4_SNORM_BLOCK : VK_FORMAT_BC4_UNORM_BLOCK;
        break;
      case ResourceFormatType::BC5:
        ret = fmt.compType == CompType::SNorm ? VK_FORMAT_BC5_SNORM_BLOCK : VK_FORMAT_BC5_UNORM_BLOCK;
        break;
      case ResourceFormatType::BC6:
        ret = fmt.compType == CompType::SNorm ? VK_FORMAT_BC6H_SFLOAT_BLOCK
                                              : VK_FORMAT_BC6H_UFLOAT_BLOCK;
        break;
      case ResourceFormatType::BC7:
        ret = fmt.SRGBCorrected() ? VK_FORMAT_BC7_SRGB_BLOCK : VK_FORMAT_BC7_UNORM_BLOCK;
        break;
      case ResourceFormatType::ETC2:
      {
        if(fmt.compCount == 3)
          ret = fmt.SRGBCorrected() ? VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK
                                    : VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
        else
          ret = fmt.SRGBCorrected() ? VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK
                                    : VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
        break;
      }
      case ResourceFormatType::EAC:
      {
        if(fmt.compCount == 1)
          ret = fmt.compType == CompType::SNorm ? VK_FORMAT_EAC_R11_SNORM_BLOCK
                                                : VK_FORMAT_EAC_R11_UNORM_BLOCK;
        else if(fmt.compCount == 2)
          ret = fmt.compType == CompType::SNorm ? VK_FORMAT_EAC_R11G11_SNORM_BLOCK
                                                : VK_FORMAT_EAC_R11G11_UNORM_BLOCK;
        else
          ret = fmt.SRGBCorrected() ? VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK
                                    : VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
        break;
      }
      case ResourceFormatType::R10G10B10A2:
        if(fmt.compType == CompType::UNorm)
          ret = fmt.BGRAOrder() ? VK_FORMAT_A2R10G10B10_UNORM_PACK32
                                : VK_FORMAT_A2B10G10R10_UNORM_PACK32;
        else if(fmt.compType == CompType::UInt)
          ret = fmt.BGRAOrder() ? VK_FORMAT_A2R10G10B10_UINT_PACK32
                                : VK_FORMAT_A2B10G10R10_UINT_PACK32;
        else if(fmt.compType == CompType::UScaled)
          ret = fmt.BGRAOrder() ? VK_FORMAT_A2R10G10B10_USCALED_PACK32
                                : VK_FORMAT_A2B10G10R10_USCALED_PACK32;
        else if(fmt.compType == CompType::SNorm)
          ret = fmt.BGRAOrder() ? VK_FORMAT_A2R10G10B10_SNORM_PACK32
                                : VK_FORMAT_A2B10G10R10_SNORM_PACK32;
        else if(fmt.compType == CompType::SInt)
          ret = fmt.BGRAOrder() ? VK_FORMAT_A2R10G10B10_SINT_PACK32
                                : VK_FORMAT_A2B10G10R10_SINT_PACK32;
        else if(fmt.compType == CompType::SScaled)
          ret = fmt.BGRAOrder() ? VK_FORMAT_A2R10G10B10_SSCALED_PACK32
                                : VK_FORMAT_A2B10G10R10_SSCALED_PACK32;
        break;
      case ResourceFormatType::R11G11B10: ret = VK_FORMAT_B10G11R11_UFLOAT_PACK32; break;
      case ResourceFormatType::R5G6B5:
        ret = fmt.BGRAOrder() ? VK_FORMAT_B5G6R5_UNORM_PACK16 : VK_FORMAT_R5G6B5_UNORM_PACK16;
        break;
      case ResourceFormatType::R5G5B5A1:
        ret = fmt.BGRAOrder() ? VK_FORMAT_B5G5R5A1_UNORM_PACK16 : VK_FORMAT_R5G5B5A1_UNORM_PACK16;
        break;
      case ResourceFormatType::R9G9B9E5: ret = VK_FORMAT_E5B9G9R9_UFLOAT_PACK32; break;
      case ResourceFormatType::R4G4B4A4:
        ret = fmt.BGRAOrder() ? VK_FORMAT_B4G4R4A4_UNORM_PACK16 : VK_FORMAT_R4G4B4A4_UNORM_PACK16;
        break;
      case ResourceFormatType::R4G4: ret = VK_FORMAT_R4G4_UNORM_PACK8; break;
      case ResourceFormatType::D16S8: ret = VK_FORMAT_D16_UNORM_S8_UINT; break;
      case ResourceFormatType::D24S8: ret = VK_FORMAT_D24_UNORM_S8_UINT; break;
      case ResourceFormatType::D32S8: ret = VK_FORMAT_D32_SFLOAT_S8_UINT; break;
      case ResourceFormatType::YUV8:
      {
        int subsampling = fmt.YUVSubsampling();
        int planeCount = fmt.YUVPlaneCount();

        // don't support anything but 3 components
        if(fmt.compCount != 3)
          return VK_FORMAT_UNDEFINED;

        if(subsampling == 444)
        {
          // only support 3-plane
          return planeCount == 3 ? VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM : VK_FORMAT_UNDEFINED;
        }
        else if(subsampling == 422)
        {
          if(planeCount == 1)
            return fmt.BGRAOrder() ? VK_FORMAT_B8G8R8G8_422_UNORM : VK_FORMAT_G8B8G8R8_422_UNORM;
          else if(planeCount == 2)
            return VK_FORMAT_G8_B8R8_2PLANE_422_UNORM;
          else if(planeCount == 3)
            return VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM;
        }
        else if(subsampling == 420)
        {
          if(planeCount == 2)
            return VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
          else if(planeCount == 3)
            return VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
          else
            return VK_FORMAT_UNDEFINED;
        }

        return VK_FORMAT_UNDEFINED;
      }
      case ResourceFormatType::YUV10:
      {
        int subsampling = fmt.YUVSubsampling();
        int planeCount = fmt.YUVPlaneCount();

        if(fmt.compCount == 1)
        {
          if(subsampling == 444 && planeCount == 1)
            return VK_FORMAT_R10X6_UNORM_PACK16;
          return VK_FORMAT_UNDEFINED;
        }
        else if(fmt.compCount == 2)
        {
          if(subsampling == 444 && planeCount == 1)
            return VK_FORMAT_R10X6G10X6_UNORM_2PACK16;
          return VK_FORMAT_UNDEFINED;
        }
        else if(fmt.compCount == 4)
        {
          if(subsampling == 444 && planeCount == 1)
            return VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16;
          return VK_FORMAT_UNDEFINED;
        }

        if(subsampling == 444)
        {
          // only support 3-plane
          return planeCount == 3 ? VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16
                                 : VK_FORMAT_UNDEFINED;
        }
        else if(subsampling == 422)
        {
          if(planeCount == 1)
            return fmt.BGRAOrder() ? VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16
                                   : VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16;
          else if(planeCount == 2)
            return VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16;
          else if(planeCount == 3)
            return VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16;
        }
        else if(subsampling == 420)
        {
          if(planeCount == 2)
            return VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16;
          else if(planeCount == 3)
            return VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16;
          else
            return VK_FORMAT_UNDEFINED;
        }

        return VK_FORMAT_UNDEFINED;
      }
      case ResourceFormatType::YUV12:
      {
        int subsampling = fmt.YUVSubsampling();
        int planeCount = fmt.YUVPlaneCount();

        if(fmt.compCount == 1)
        {
          if(subsampling == 444 && planeCount == 1)
            return VK_FORMAT_R12X4_UNORM_PACK16;
          return VK_FORMAT_UNDEFINED;
        }
        else if(fmt.compCount == 2)
        {
          if(subsampling == 444 && planeCount == 1)
            return VK_FORMAT_R12X4G12X4_UNORM_2PACK16;
          return VK_FORMAT_UNDEFINED;
        }
        else if(fmt.compCount == 4)
        {
          if(subsampling == 444 && planeCount == 1)
            return VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16;
          return VK_FORMAT_UNDEFINED;
        }

        if(subsampling == 444)
        {
          // only support 3-plane
          return planeCount == 3 ? VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16
                                 : VK_FORMAT_UNDEFINED;
        }
        else if(subsampling == 422)
        {
          if(planeCount == 1)
            return fmt.BGRAOrder() ? VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16
                                   : VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16;
          else if(planeCount == 2)
            return VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16;
          else if(planeCount == 3)
            return VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16;
        }
        else if(subsampling == 420)
        {
          if(planeCount == 2)
            return VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16;
          else if(planeCount == 3)
            return VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16;
          else
            return VK_FORMAT_UNDEFINED;
        }

        return VK_FORMAT_UNDEFINED;
      }
      case ResourceFormatType::YUV16:
      {
        int subsampling = fmt.YUVSubsampling();
        int planeCount = fmt.YUVPlaneCount();

        if(subsampling == 444)
        {
          // only support 3-plane
          return planeCount == 3 ? VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM : VK_FORMAT_UNDEFINED;
        }
        else if(subsampling == 422)
        {
          if(planeCount == 1)
            return fmt.BGRAOrder() ? VK_FORMAT_B16G16R16G16_422_UNORM
                                   : VK_FORMAT_G16B16G16R16_422_UNORM;
          else if(planeCount == 2)
            return VK_FORMAT_G16_B16R16_2PLANE_422_UNORM;
          else if(planeCount == 3)
            return VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM;
        }
        else if(subsampling == 420)
        {
          if(planeCount == 2)
            return VK_FORMAT_G16_B16R16_2PLANE_420_UNORM;
          else if(planeCount == 3)
            return VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM;
          else
            return VK_FORMAT_UNDEFINED;
        }

        return VK_FORMAT_UNDEFINED;
      }
      default: RDCERR("Unsupported resource format type %u", fmt.type); break;
    }
  }
  else if(fmt.compCount == 4)
  {
    if(fmt.SRGBCorrected())
    {
      ret = fmt.BGRAOrder() ? VK_FORMAT_B8G8R8A8_SRGB : VK_FORMAT_R8G8B8A8_SRGB;
    }
    else if(fmt.compByteWidth == 8)
    {
      if(fmt.compType == CompType::Float || fmt.compType == CompType::Double)
        ret = VK_FORMAT_R64G64B64A64_SFLOAT;
      else if(fmt.compType == CompType::SInt)
        ret = VK_FORMAT_R64G64B64A64_SINT;
      else if(fmt.compType == CompType::UInt)
        ret = VK_FORMAT_R64G64B64A64_UINT;
      else
        RDCERR("Unrecognised component type");
    }
    else if(fmt.compByteWidth == 4)
    {
      if(fmt.compType == CompType::Float)
        ret = VK_FORMAT_R32G32B32A32_SFLOAT;
      else if(fmt.compType == CompType::SInt)
        ret = VK_FORMAT_R32G32B32A32_SINT;
      else if(fmt.compType == CompType::UInt)
        ret = VK_FORMAT_R32G32B32A32_UINT;
      else
        RDCERR("Unrecognised component type");
    }
    else if(fmt.compByteWidth == 2)
    {
      if(fmt.compType == CompType::Float)
        ret = VK_FORMAT_R16G16B16A16_SFLOAT;
      else if(fmt.compType == CompType::SInt)
        ret = VK_FORMAT_R16G16B16A16_SINT;
      else if(fmt.compType == CompType::UInt)
        ret = VK_FORMAT_R16G16B16A16_UINT;
      else if(fmt.compType == CompType::SNorm)
        ret = VK_FORMAT_R16G16B16A16_SNORM;
      else if(fmt.compType == CompType::UNorm)
        ret = VK_FORMAT_R16G16B16A16_UNORM;
      else if(fmt.compType == CompType::SScaled)
        ret = VK_FORMAT_R16G16B16A16_SSCALED;
      else if(fmt.compType == CompType::UScaled)
        ret = VK_FORMAT_R16G16B16A16_USCALED;
      else
        RDCERR("Unrecognised component type");
    }
    else if(fmt.compByteWidth == 1)
    {
      if(fmt.compType == CompType::SInt)
        ret = fmt.BGRAOrder() ? VK_FORMAT_B8G8R8A8_SINT : VK_FORMAT_R8G8B8A8_SINT;
      else if(fmt.compType == CompType::UInt)
        ret = fmt.BGRAOrder() ? VK_FORMAT_B8G8R8A8_UINT : VK_FORMAT_R8G8B8A8_UINT;
      else if(fmt.compType == CompType::SNorm)
        ret = fmt.BGRAOrder() ? VK_FORMAT_B8G8R8A8_SNORM : VK_FORMAT_R8G8B8A8_SNORM;
      else if(fmt.compType == CompType::UNorm)
        ret = fmt.BGRAOrder() ? VK_FORMAT_B8G8R8A8_UNORM : VK_FORMAT_R8G8B8A8_UNORM;
      else if(fmt.compType == CompType::SScaled)
        ret = fmt.BGRAOrder() ? VK_FORMAT_B8G8R8A8_SSCALED : VK_FORMAT_R8G8B8A8_SSCALED;
      else if(fmt.compType == CompType::UScaled)
        ret = fmt.BGRAOrder() ? VK_FORMAT_B8G8R8A8_USCALED : VK_FORMAT_R8G8B8A8_USCALED;
      else
        RDCERR("Unrecognised component type");
    }
    else
    {
      RDCERR("Unrecognised 4-component byte width: %d", fmt.compByteWidth);
    }
  }
  else if(fmt.compCount == 3)
  {
    if(fmt.SRGBCorrected())
    {
      ret = fmt.BGRAOrder() ? VK_FORMAT_B8G8R8_SRGB : VK_FORMAT_R8G8B8_SRGB;
    }
    else if(fmt.compByteWidth == 8)
    {
      if(fmt.compType == CompType::Float || fmt.compType == CompType::Double)
        ret = VK_FORMAT_R64G64B64_SFLOAT;
      else if(fmt.compType == CompType::SInt)
        ret = VK_FORMAT_R64G64B64_SINT;
      else if(fmt.compType == CompType::UInt)
        ret = VK_FORMAT_R64G64B64_UINT;
      else
        RDCERR("Unrecognised component type");
    }
    else if(fmt.compByteWidth == 4)
    {
      if(fmt.compType == CompType::Float)
        ret = VK_FORMAT_R32G32B32_SFLOAT;
      else if(fmt.compType == CompType::SInt)
        ret = VK_FORMAT_R32G32B32_SINT;
      else if(fmt.compType == CompType::UInt)
        ret = VK_FORMAT_R32G32B32_UINT;
      else
        RDCERR("Unrecognised component type");
    }
    else if(fmt.compByteWidth == 2)
    {
      if(fmt.compType == CompType::Float)
        ret = VK_FORMAT_R16G16B16_SFLOAT;
      else if(fmt.compType == CompType::SInt)
        ret = VK_FORMAT_R16G16B16_SINT;
      else if(fmt.compType == CompType::UInt)
        ret = VK_FORMAT_R16G16B16_UINT;
      else if(fmt.compType == CompType::SNorm)
        ret = VK_FORMAT_R16G16B16_SNORM;
      else if(fmt.compType == CompType::UNorm)
        ret = VK_FORMAT_R16G16B16_UNORM;
      else if(fmt.compType == CompType::SScaled)
        ret = VK_FORMAT_R16G16B16_SSCALED;
      else if(fmt.compType == CompType::UScaled)
        ret = VK_FORMAT_R16G16B16_USCALED;
      else
        RDCERR("Unrecognised component type");
    }
    else if(fmt.compByteWidth == 1)
    {
      if(fmt.compType == CompType::SInt)
        ret = fmt.BGRAOrder() ? VK_FORMAT_B8G8R8_SINT : VK_FORMAT_R8G8B8_SINT;
      else if(fmt.compType == CompType::UInt)
        ret = fmt.BGRAOrder() ? VK_FORMAT_B8G8R8_UINT : VK_FORMAT_R8G8B8_UINT;
      else if(fmt.compType == CompType::SNorm)
        ret = fmt.BGRAOrder() ? VK_FORMAT_B8G8R8_SNORM : VK_FORMAT_R8G8B8_SNORM;
      else if(fmt.compType == CompType::UNorm)
        ret = fmt.BGRAOrder() ? VK_FORMAT_B8G8R8_UNORM : VK_FORMAT_R8G8B8_UNORM;
      else if(fmt.compType == CompType::SScaled)
        ret = fmt.BGRAOrder() ? VK_FORMAT_B8G8R8_SSCALED : VK_FORMAT_R8G8B8_SSCALED;
      else if(fmt.compType == CompType::UScaled)
        ret = fmt.BGRAOrder() ? VK_FORMAT_B8G8R8_USCALED : VK_FORMAT_R8G8B8_USCALED;
      else
        RDCERR("Unrecognised component type");
    }
    else
    {
      RDCERR("Unrecognised 3-component byte width: %d", fmt.compByteWidth);
    }
  }
  else if(fmt.compCount == 2)
  {
    if(fmt.SRGBCorrected())
    {
      ret = VK_FORMAT_R8G8_SRGB;
    }
    else if(fmt.compByteWidth == 8)
    {
      if(fmt.compType == CompType::Float || fmt.compType == CompType::Double)
        ret = VK_FORMAT_R64G64_SFLOAT;
      else if(fmt.compType == CompType::SInt)
        ret = VK_FORMAT_R64G64_SINT;
      else if(fmt.compType == CompType::UInt)
        ret = VK_FORMAT_R64G64_UINT;
      else
        RDCERR("Unrecognised component type");
    }
    else if(fmt.compByteWidth == 4)
    {
      if(fmt.compType == CompType::Float)
        ret = VK_FORMAT_R32G32_SFLOAT;
      else if(fmt.compType == CompType::SInt)
        ret = VK_FORMAT_R32G32_SINT;
      else if(fmt.compType == CompType::UInt)
        ret = VK_FORMAT_R32G32_UINT;
      else
        RDCERR("Unrecognised component type");
    }
    else if(fmt.compByteWidth == 2)
    {
      if(fmt.compType == CompType::Float)
        ret = VK_FORMAT_R16G16_SFLOAT;
      else if(fmt.compType == CompType::SInt)
        ret = VK_FORMAT_R16G16_SINT;
      else if(fmt.compType == CompType::UInt)
        ret = VK_FORMAT_R16G16_UINT;
      else if(fmt.compType == CompType::SNorm)
        ret = VK_FORMAT_R16G16_SNORM;
      else if(fmt.compType == CompType::UNorm)
        ret = VK_FORMAT_R16G16_UNORM;
      else if(fmt.compType == CompType::SScaled)
        ret = VK_FORMAT_R16G16_SSCALED;
      else if(fmt.compType == CompType::UScaled)
        ret = VK_FORMAT_R16G16_USCALED;
      else
        RDCERR("Unrecognised component type");
    }
    else if(fmt.compByteWidth == 1)
    {
      if(fmt.compType == CompType::SInt)
        ret = VK_FORMAT_R8G8_SINT;
      else if(fmt.compType == CompType::UInt)
        ret = VK_FORMAT_R8G8_UINT;
      else if(fmt.compType == CompType::SNorm)
        ret = VK_FORMAT_R8G8_SNORM;
      else if(fmt.compType == CompType::UNorm)
        ret = VK_FORMAT_R8G8_UNORM;
      else if(fmt.compType == CompType::SScaled)
        ret = VK_FORMAT_R8G8_SSCALED;
      else if(fmt.compType == CompType::UScaled)
        ret = VK_FORMAT_R8G8_USCALED;
      else
        RDCERR("Unrecognised component type");
    }
    else
    {
      RDCERR("Unrecognised 2-component byte width: %d", fmt.compByteWidth);
    }
  }
  else if(fmt.compCount == 1)
  {
    if(fmt.SRGBCorrected())
    {
      ret = VK_FORMAT_R8_SRGB;
    }
    else if(fmt.compByteWidth == 8)
    {
      if(fmt.compType == CompType::Float || fmt.compType == CompType::Double)
        ret = VK_FORMAT_R64_SFLOAT;
      else if(fmt.compType == CompType::SInt)
        ret = VK_FORMAT_R64_SINT;
      else if(fmt.compType == CompType::UInt)
        ret = VK_FORMAT_R64_UINT;
      else
        RDCERR("Unrecognised component type");
    }
    else if(fmt.compByteWidth == 4)
    {
      if(fmt.compType == CompType::Float)
        ret = VK_FORMAT_R32_SFLOAT;
      else if(fmt.compType == CompType::SInt)
        ret = VK_FORMAT_R32_SINT;
      else if(fmt.compType == CompType::UInt)
        ret = VK_FORMAT_R32_UINT;
      else if(fmt.compType == CompType::Depth)
        ret = VK_FORMAT_D32_SFLOAT;
      else
        RDCERR("Unrecognised component type");
    }
    else if(fmt.compByteWidth == 2)
    {
      if(fmt.compType == CompType::Float)
        ret = VK_FORMAT_R16_SFLOAT;
      else if(fmt.compType == CompType::SInt)
        ret = VK_FORMAT_R16_SINT;
      else if(fmt.compType == CompType::UInt)
        ret = VK_FORMAT_R16_UINT;
      else if(fmt.compType == CompType::SNorm)
        ret = VK_FORMAT_R16_SNORM;
      else if(fmt.compType == CompType::UNorm)
        ret = VK_FORMAT_R16_UNORM;
      else if(fmt.compType == CompType::Depth)
        ret = VK_FORMAT_D16_UNORM;
      else if(fmt.compType == CompType::UScaled)
        ret = VK_FORMAT_R16_USCALED;
      else if(fmt.compType == CompType::SScaled)
        ret = VK_FORMAT_R16_SSCALED;
      else
        RDCERR("Unrecognised component type");
    }
    else if(fmt.compByteWidth == 1)
    {
      if(fmt.compType == CompType::SInt)
        ret = VK_FORMAT_R8_SINT;
      else if(fmt.compType == CompType::UInt)
        ret = VK_FORMAT_R8_UINT;
      else if(fmt.compType == CompType::SNorm)
        ret = VK_FORMAT_R8_SNORM;
      else if(fmt.compType == CompType::UNorm)
        ret = VK_FORMAT_R8_UNORM;
      else if(fmt.compType == CompType::UScaled)
        ret = VK_FORMAT_R8_USCALED;
      else if(fmt.compType == CompType::SScaled)
        ret = VK_FORMAT_R8_SSCALED;
      else if(fmt.compType == CompType::Depth)
        ret = VK_FORMAT_S8_UINT;
      else
        RDCERR("Unrecognised component type");
    }
    else
    {
      RDCERR("Unrecognised 1-component byte width: %d", fmt.compByteWidth);
    }
  }
  else
  {
    RDCERR("Unrecognised component count: %d", fmt.compCount);
  }

  if(ret == VK_FORMAT_UNDEFINED)
    RDCERR("No known vulkan format corresponding to resource format!");

  return ret;
}

VkPrimitiveTopology MakeGXMVkPrimitiveTopology(Topology Topo)
{
  switch(Topo)
  {
    case Topology::LineLoop: RDCWARN("Unsupported primitive topology on Vulkan: %x", Topo); break;
    default: return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
    case Topology::PointList: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    case Topology::LineStrip: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    case Topology::LineList: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    case Topology::LineStrip_Adj: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
    case Topology::LineList_Adj: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
    case Topology::TriangleStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    case Topology::TriangleFan: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
    case Topology::TriangleList: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case Topology::TriangleStrip_Adj: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
    case Topology::TriangleList_Adj: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
    case Topology::PatchList_1CPs:
    case Topology::PatchList_2CPs:
    case Topology::PatchList_3CPs:
    case Topology::PatchList_4CPs:
    case Topology::PatchList_5CPs:
    case Topology::PatchList_6CPs:
    case Topology::PatchList_7CPs:
    case Topology::PatchList_8CPs:
    case Topology::PatchList_9CPs:
    case Topology::PatchList_10CPs:
    case Topology::PatchList_11CPs:
    case Topology::PatchList_12CPs:
    case Topology::PatchList_13CPs:
    case Topology::PatchList_14CPs:
    case Topology::PatchList_15CPs:
    case Topology::PatchList_16CPs:
    case Topology::PatchList_17CPs:
    case Topology::PatchList_18CPs:
    case Topology::PatchList_19CPs:
    case Topology::PatchList_20CPs:
    case Topology::PatchList_21CPs:
    case Topology::PatchList_22CPs:
    case Topology::PatchList_23CPs:
    case Topology::PatchList_24CPs:
    case Topology::PatchList_25CPs:
    case Topology::PatchList_26CPs:
    case Topology::PatchList_27CPs:
    case Topology::PatchList_28CPs:
    case Topology::PatchList_29CPs:
    case Topology::PatchList_30CPs:
    case Topology::PatchList_31CPs:
    case Topology::PatchList_32CPs: return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
  }

  return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
}

void GXMGPUBuffer::Create(WrappedGXM *driver, VkDevice dev, VkDeviceSize size, uint32_t ringSize,
                       uint32_t flags)
{
  m_pDriver = driver;
  device = dev;
  createFlags = flags;

  align = (VkDeviceSize)driver->GetVulkanState().m_DeviceProperties.limits.minUniformBufferOffsetAlignment;

  // for simplicity, consider the non-coherent atom size also an alignment requirement
  align = AlignUp(align, driver->GetVulkanState().m_DeviceProperties.limits.nonCoherentAtomSize);

  sz = size;
  // offset must be aligned, so ensure we have at least ringSize
  // copies accounting for that
  totalsize = AlignUp(size, align) * RDCMAX(1U, ringSize);
  curoffset = 0;

  ringCount = ringSize;

  VkBufferCreateInfo bufInfo = {
      VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, NULL, 0, totalsize, 0,
  };

  bufInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  bufInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  bufInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

  if(flags & eGPUBufferVBuffer)
    bufInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

  if(flags & eGPUBufferIBuffer)
    bufInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

  if(flags & eGPUBufferSSBO)
    bufInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

  if(flags & eGPUBufferIndirectBuffer)
    bufInfo.usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

  if(flags & eGPUBufferAddressable)
    bufInfo.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

  VkResult vkr = vkCreateBuffer(dev, &bufInfo, NULL, &buf);
  RDCASSERTEQUAL(vkr, VK_SUCCESS);

  VkMemoryRequirements mrq = {};
  vkGetBufferMemoryRequirements(dev, buf, &mrq);

  VkMemoryAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, NULL, mrq.size, 0};

  if(flags & eGPUBufferReadback)
    allocInfo.memoryTypeIndex = driver->GetReadbackMemoryIndex(mrq.memoryTypeBits);
  else if(flags & eGPUBufferGPULocal)
    allocInfo.memoryTypeIndex = driver->GetGPULocalMemoryIndex(mrq.memoryTypeBits);
  else
    allocInfo.memoryTypeIndex = driver->GetUploadMemoryIndex(mrq.memoryTypeBits);

  /*bool useBufferAddressKHR = driver->GetExtensions(NULL).ext_KHR_buffer_device_address;

  VkMemoryAllocateFlagsInfo memFlags = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO};
  if(useBufferAddressKHR && (flags & eGPUBufferAddressable))
  {
    allocInfo.pNext = &memFlags;
    memFlags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
  }*/

  vkr = vkAllocateMemory(dev, &allocInfo, NULL, &mem);
  RDCASSERTEQUAL(vkr, VK_SUCCESS);

  vkr = vkBindBufferMemory(dev, buf, mem, 0);
  RDCASSERTEQUAL(vkr, VK_SUCCESS);
}

void GXMGPUBuffer::FillDescriptor(VkDescriptorBufferInfo &desc)
{
  desc.buffer = buf;
  desc.offset = 0;
  desc.range = sz;
}

void *GXMGPUBuffer::Map(uint32_t *bindoffset, VkDeviceSize usedsize)
{
  VkDeviceSize offset = bindoffset ? curoffset : 0;
  VkDeviceSize size = usedsize > 0 ? usedsize : sz;

  // align the size so we always consume coherent atoms
  size = AlignUp(size, align);

  // wrap around the ring as soon as the 'sz' would overflow. This is because if we're using dynamic
  // offsets in the descriptor the range is still set to that fixed size and the validation
  // complains if we go off the end (even if it's unused). Rather than constantly update the
  // descriptor, we just conservatively wrap and waste the last bit of space.
  if(offset + sz > totalsize)
    offset = 0;
  RDCASSERT(offset + size <= totalsize);

  // offset must be aligned
  curoffset = AlignUp(offset + size, align);

  if(bindoffset)
    *bindoffset = (uint32_t)offset;

  mapoffset = offset;

  void *ptr = NULL;
  VkResult vkr = vkMapMemory(device, mem, offset, size, 0, (void **)&ptr);
  RDCASSERTEQUAL(vkr, VK_SUCCESS);

  if(createFlags & eGPUBufferReadback)
  {
    VkMappedMemoryRange range = {
        VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE, NULL, mem, offset, size,
    };

    vkr = vkInvalidateMappedMemoryRanges(device, 1, &range);
    RDCASSERTEQUAL(vkr, VK_SUCCESS);
  }

  return ptr;
}

void *GXMGPUBuffer::Map(VkDeviceSize &bindoffset, VkDeviceSize usedsize)
{
  uint32_t offs = 0;

  void *ret = Map(&offs, usedsize);

  bindoffset = offs;

  return ret;
}

void GXMGPUBuffer::Unmap()
{
  if(!(createFlags & eGPUBufferReadback) && !(createFlags & eGPUBufferGPULocal))
  {
    VkMappedMemoryRange range = {
        VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE, NULL, mem, mapoffset, VK_WHOLE_SIZE,
    };

    VkResult vkr = vkFlushMappedMemoryRanges(device, 1, &range);
    RDCASSERTEQUAL(vkr, VK_SUCCESS);
  }

  vkUnmapMemory(device, mem);
}

bool IsDepthOrStencilFormatGXM(VkFormat f)
{
  switch(f)
  {
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_X8_D24_UNORM_PACK32:
    case VK_FORMAT_D32_SFLOAT:
    case VK_FORMAT_S8_UINT:
    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT: return true;
    default: break;
  }

  return false;
}
