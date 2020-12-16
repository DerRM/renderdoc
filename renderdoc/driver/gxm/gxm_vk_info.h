#pragma once

#include "gxm_common.h"

struct SpecConstant
{
  SpecConstant() = default;
  SpecConstant(uint32_t id, uint64_t val, size_t size) : specID(id), value(val), dataSize(size) {}
  uint32_t specID = 0;
  uint64_t value = 0;
  size_t dataSize = 0;
};

enum GXMVkDynamicStateIndex
{
  VkDynamicViewport,
  VkDynamicScissor,
  VkDynamicLineWidth,
  VkDynamicDepthBias,
  VkDynamicBlendConstants,
  VkDynamicDepthBounds,
  VkDynamicStencilCompareMask,
  VkDynamicStencilWriteMask,
  VkDynamicStencilReference,
  VkDynamicViewportWScalingNV,
  VkDynamicDiscardRectangleEXT,
  VkDynamicSampleLocationsEXT,
  VkDynamicViewportShadingRatePaletteNV,
  VkDynamicViewportCoarseSampleOrderNV,
  VkDynamicExclusiveScissorNV,
  VkDynamicLineStippleEXT,
  VkDynamicCount,
};

VkDynamicState ConvertDynamicState(GXMVkDynamicStateIndex idx);
GXMVkDynamicStateIndex ConvertDynamicState(VkDynamicState state);

struct GXMVkCreationInfo
{
  struct Pipeline
  {
    VkPipelineLayout layout;
    VkRenderPass renderpass;
    uint32_t subpass;

    // a variant of the pipeline that uses subpass 0, used for when we are replaying in isolation.
    // See loadRPs in the RenderPass info
    VkPipeline subpass0pipe;

    // VkGraphicsPipelineCreateInfo
    VkPipelineCreateFlags flags;

    // VkPipelineShaderStageCreateInfo
    struct Shader
    {
      Shader() : refl(NULL), mapping(NULL)/*, patchData(NULL)*/ {}
      VkShaderModule module;
      rdcstr entryPoint;
      ShaderReflection *refl;
      ShaderBindpointMapping *mapping;
      //SPIRVPatchData *patchData;

      rdcarray<SpecConstant> specialization;
    };
    Shader shaders[6];

    // VkPipelineVertexInputStateCreateInfo
    struct Binding
    {
      uint32_t vbufferBinding;
      uint32_t bytestride;
      bool perInstance;
    };
    rdcarray<Binding> vertexBindings;

    struct Attribute
    {
      uint32_t location;
      uint32_t binding;
      VkFormat format;
      uint32_t byteoffset;
    };
    rdcarray<Attribute> vertexAttrs;

    // VkPipelineInputAssemblyStateCreateInfo
    VkPrimitiveTopology topology;
    bool primitiveRestartEnable;

    // VkPipelineTessellationStateCreateInfo
    uint32_t patchControlPoints;

    // VkPipelineTessellationDomainOriginStateCreateInfo
    VkTessellationDomainOrigin tessellationDomainOrigin;

    // VkPipelineViewportStateCreateInfo
    uint32_t viewportCount;
    rdcarray<VkViewport> viewports;
    rdcarray<VkRect2D> scissors;

    // VkPipelineRasterizationStateCreateInfo
    bool depthClampEnable;
    bool rasterizerDiscardEnable;
    VkPolygonMode polygonMode;
    VkCullModeFlags cullMode;
    VkFrontFace frontFace;
    bool depthBiasEnable;
    float depthBiasConstantFactor;
    float depthBiasClamp;
    float depthBiasSlopeFactor;
    float lineWidth;

    // VkPipelineMultisampleStateCreateInfo
    VkSampleCountFlagBits rasterizationSamples;
    bool sampleShadingEnable;
    float minSampleShading;
    VkSampleMask sampleMask;
    bool alphaToCoverageEnable;
    bool alphaToOneEnable;

    // VkPipelineDepthStencilStateCreateInfo
    bool depthTestEnable;
    bool depthWriteEnable;
    VkCompareOp depthCompareOp;
    bool depthBoundsEnable;
    bool stencilTestEnable;
    VkStencilOpState front;
    VkStencilOpState back;
    float minDepthBounds;
    float maxDepthBounds;

    // VkPipelineColorBlendStateCreateInfo
    bool logicOpEnable;
    VkLogicOp logicOp;
    float blendConst[4];

    struct Attachment
    {
      bool blendEnable;

      struct BlendOp
      {
        VkBlendFactor Source;
        VkBlendFactor Destination;
        VkBlendOp Operation;
      } blend, alphaBlend;

      uint8_t channelWriteMask;
    };
    rdcarray<Attachment> attachments;

    // VkPipelineDynamicStateCreateInfo
    bool dynamicStates[VkDynamicCount];
  };
  std::map<ResourceId, Pipeline> m_Pipeline;

};