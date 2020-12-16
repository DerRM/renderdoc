#include "gxm_vk_info.h"

VkDynamicState ConvertDynamicState(GXMVkDynamicStateIndex idx)
{
  switch(idx)
  {
    case VkDynamicViewport: return VK_DYNAMIC_STATE_VIEWPORT;
    case VkDynamicScissor: return VK_DYNAMIC_STATE_SCISSOR;
    case VkDynamicLineWidth: return VK_DYNAMIC_STATE_LINE_WIDTH;
    case VkDynamicDepthBias: return VK_DYNAMIC_STATE_DEPTH_BIAS;
    case VkDynamicBlendConstants: return VK_DYNAMIC_STATE_BLEND_CONSTANTS;
    case VkDynamicDepthBounds: return VK_DYNAMIC_STATE_DEPTH_BOUNDS;
    case VkDynamicStencilCompareMask: return VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK;
    case VkDynamicStencilWriteMask: return VK_DYNAMIC_STATE_STENCIL_WRITE_MASK;
    case VkDynamicStencilReference: return VK_DYNAMIC_STATE_STENCIL_REFERENCE;
    case VkDynamicViewportWScalingNV: return VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV;
    case VkDynamicDiscardRectangleEXT: return VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT;
    case VkDynamicSampleLocationsEXT: return VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT;
    case VkDynamicViewportShadingRatePaletteNV:
      return VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV;
    case VkDynamicViewportCoarseSampleOrderNV:
      return VK_DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV;
    case VkDynamicExclusiveScissorNV: return VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV;
    case VkDynamicLineStippleEXT: return VK_DYNAMIC_STATE_LINE_STIPPLE_EXT;
    case VkDynamicCount: break;
  }

  RDCERR("Unexpected vulkan dynamic state index %u", idx);

  return VK_DYNAMIC_STATE_MAX_ENUM;
}

GXMVkDynamicStateIndex ConvertDynamicState(VkDynamicState state)
{
  switch(state)
  {
    case VK_DYNAMIC_STATE_VIEWPORT: return VkDynamicViewport;
    case VK_DYNAMIC_STATE_SCISSOR: return VkDynamicScissor;
    case VK_DYNAMIC_STATE_LINE_WIDTH: return VkDynamicLineWidth;
    case VK_DYNAMIC_STATE_DEPTH_BIAS: return VkDynamicDepthBias;
    case VK_DYNAMIC_STATE_BLEND_CONSTANTS: return VkDynamicBlendConstants;
    case VK_DYNAMIC_STATE_DEPTH_BOUNDS: return VkDynamicDepthBounds;
    case VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK: return VkDynamicStencilCompareMask;
    case VK_DYNAMIC_STATE_STENCIL_WRITE_MASK: return VkDynamicStencilWriteMask;
    case VK_DYNAMIC_STATE_STENCIL_REFERENCE: return VkDynamicStencilReference;
    case VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV: return VkDynamicViewportWScalingNV;
    case VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT: return VkDynamicDiscardRectangleEXT;
    case VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT: return VkDynamicSampleLocationsEXT;
    case VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV:
      return VkDynamicViewportShadingRatePaletteNV;
    case VK_DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV:
      return VkDynamicViewportCoarseSampleOrderNV;
    case VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV: return VkDynamicExclusiveScissorNV;
    case VK_DYNAMIC_STATE_LINE_STIPPLE_EXT: return VkDynamicLineStippleEXT;
    case VK_DYNAMIC_STATE_RANGE_SIZE:
    case VK_DYNAMIC_STATE_MAX_ENUM: break;
  }

  RDCERR("Unexpected vulkan state %u", state);

  return VkDynamicCount;
}