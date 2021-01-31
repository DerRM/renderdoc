#include "gxm_debug.h"
#include "gxm_replay.h"
#include "gxm_vk_shader_cache.h"

#define VULKAN
#include "data/glsl/glsl_ubos_cpp.h"

static void create(WrappedGXM *driver, const char *objName, const int line, VkSampler *sampler,
                   VkFilter samplerFilter)
{
  VkSamplerCreateInfo sampInfo = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};

  sampInfo.minFilter = sampInfo.magFilter = samplerFilter;
  sampInfo.mipmapMode = samplerFilter == VK_FILTER_NEAREST ? VK_SAMPLER_MIPMAP_MODE_NEAREST
                                                           : VK_SAMPLER_MIPMAP_MODE_LINEAR;
  sampInfo.addressModeU = sampInfo.addressModeV = sampInfo.addressModeW =
      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  sampInfo.maxLod = 128.0f;

  VkResult vkr = vkCreateSampler(driver->GetVulkanState().m_Device, &sampInfo, NULL, sampler);
  if(vkr != VK_SUCCESS)
    RDCERR("Failed creating object %s at line %i, vkr was %s", objName, line, ToStr(vkr).c_str());
}

static void create(WrappedGXM *driver, const char *objName, const int line,
                   VkDescriptorSetLayout *descLayout,
                   std::initializer_list<VkDescriptorSetLayoutBinding> bindings)
{
  VkDescriptorSetLayoutCreateInfo descsetLayoutInfo = {
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      NULL,
      0,
      (uint32_t)bindings.size(),
      bindings.begin(),
  };

  VkResult vkr =
      vkCreateDescriptorSetLayout(driver->GetVulkanState().m_Device, &descsetLayoutInfo, NULL, descLayout);
  if(vkr != VK_SUCCESS)
    RDCERR("Failed creating object %s at line %i, vkr was %s", objName, line, ToStr(vkr).c_str());
}

static void create(WrappedGXM *driver, const char *objName, const int line,
                   VkPipelineLayout *pipeLayout, VkDescriptorSetLayout setLayout, uint32_t pushBytes)
{
  VkPipelineLayoutCreateInfo pipeLayoutInfo = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};

  VkPushConstantRange push = {VK_SHADER_STAGE_ALL, 0, pushBytes};

  if(pushBytes > 0)
  {
    pipeLayoutInfo.pPushConstantRanges = &push;
    pipeLayoutInfo.pushConstantRangeCount = 1;
  }

  pipeLayoutInfo.pSetLayouts = &setLayout;
  pipeLayoutInfo.setLayoutCount = 1;

  VkResult vkr = vkCreatePipelineLayout(driver->GetVulkanState().m_Device, &pipeLayoutInfo, NULL, pipeLayout);
  if(vkr != VK_SUCCESS)
    RDCERR("Failed creating object %s at line %i, vkr was %s", objName, line, ToStr(vkr).c_str());
}

static void create(WrappedGXM *driver, const char *objName, const int line,
                   VkRenderPass *renderPass, VkFormat attachFormat,
                   VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT,
                   VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
{
  VkAttachmentDescription attDesc = {0,
                                     attachFormat,
                                     sampleCount,
                                     VK_ATTACHMENT_LOAD_OP_LOAD,
                                     VK_ATTACHMENT_STORE_OP_STORE,
                                     VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                     VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                     layout,
                                     layout};

  VkAttachmentReference attRef = {0, layout};

  VkSubpassDescription sub = {
      0, VK_PIPELINE_BIND_POINT_GRAPHICS,
      0, NULL,       // inputs
      1, &attRef,    // color
  };

  if(IsDepthOrStencilFormatGXM(attachFormat))
  {
    attDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;

    sub.colorAttachmentCount = 0;
    sub.pColorAttachments = NULL;
    sub.pDepthStencilAttachment = &attRef;
  }

  VkRenderPassCreateInfo rpinfo = {
      VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, NULL, 0, 1, &attDesc, 1, &sub,
  };

  VkResult vkr = vkCreateRenderPass(driver->GetVulkanState().m_Device, &rpinfo, NULL, renderPass);
  if(vkr != VK_SUCCESS)
    RDCERR("Failed creating object %s at line %i, vkr was %s", objName, line, ToStr(vkr).c_str());

  //driver->GetResourceManager()->SetInternalResource(GetResID(*renderPass));
}

static void create(WrappedGXM *driver, const char *objName, const int line,
                   VkDescriptorSet *descSet, VkDescriptorPool pool, VkDescriptorSetLayout setLayout)
{
  VkDescriptorSetAllocateInfo descSetAllocInfo = {
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, NULL, pool, 1, &setLayout,
  };

  // don't expect this to fail (or if it does then it should be immediately obvious, not transient).
  VkResult vkr = vkAllocateDescriptorSets(driver->GetVulkanState().m_Device, &descSetAllocInfo, descSet);
  if(vkr != VK_SUCCESS)
    RDCERR("Failed creating object %s at line %i, vkr was %s", objName, line, ToStr(vkr).c_str());
}

// a simpler one-shot descriptor containing anything we might want to vary in a graphics pipeline
struct ConciseGraphicsPipeline
{
  // misc
  VkRenderPass renderPass;
  VkPipelineLayout pipeLayout;
  VkShaderModule vertex;
  VkShaderModule fragment;

  // dynamic state
  std::initializer_list<VkDynamicState> dynstates;

  // msaa
  VkSampleCountFlagBits sampleCount;
  bool sampleRateShading;

  // depth stencil
  bool depthEnable;
  bool stencilEnable;
  VkStencilOp stencilOp;

  // color blend
  bool colourOutput;
  bool blendEnable;
  VkBlendFactor srcBlend;
  VkBlendFactor dstBlend;
  uint32_t writeMask;
};

static void create(WrappedGXM *driver, const char *objName, const int line, VkPipeline *pipe,
                   const ConciseGraphicsPipeline &info)
{
  // if the module didn't compile, this pipeline is not be supported. Silently don't create it, code
  // later should handle the missing pipeline as indicating lack of support
  if(info.vertex == VK_NULL_HANDLE || info.fragment == VK_NULL_HANDLE)
    return;

  // first configure the structs that contain parameters derived from the info parameter

  const VkPipelineShaderStageCreateInfo shaderStages[2] = {
      {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, NULL, 0, VK_SHADER_STAGE_VERTEX_BIT,
       info.vertex, "main", NULL},
      {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, NULL, 0, VK_SHADER_STAGE_FRAGMENT_BIT,
       info.fragment, "main", NULL},
  };

  const VkPipelineDynamicStateCreateInfo dynamicState = {
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      NULL,
      0,
      (uint32_t)info.dynstates.size(),
      info.dynstates.begin(),
  };

  VkPipelineMultisampleStateCreateInfo msaa = {
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
  };

  msaa.rasterizationSamples = info.sampleCount;
  if(info.sampleRateShading)
  {
    msaa.minSampleShading = 1.0f;
    msaa.sampleShadingEnable = true;
  }

  const VkPipelineDepthStencilStateCreateInfo depthStencil = {
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      NULL,
      0,
      info.depthEnable,
      info.depthEnable,
      VK_COMPARE_OP_ALWAYS,
      false,
      info.stencilEnable,
      {info.stencilOp, info.stencilOp, info.stencilOp, VK_COMPARE_OP_ALWAYS, 0xff, 0xff, 0},
      {info.stencilOp, info.stencilOp, info.stencilOp, VK_COMPARE_OP_ALWAYS, 0xff, 0xff, 0},
      0.0f,
      1.0f,
  };

  const VkPipelineColorBlendAttachmentState colAttach = {
      info.blendEnable,
      // colour blending
      info.srcBlend,
      info.dstBlend,
      VK_BLEND_OP_ADD,
      // alpha blending
      info.srcBlend,
      info.dstBlend,
      VK_BLEND_OP_ADD,
      // write mask
      info.writeMask,
  };

  const VkPipelineColorBlendStateCreateInfo colorBlend = {
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      NULL,
      0,
      false,
      VK_LOGIC_OP_NO_OP,
      info.colourOutput ? 1U : 0U,
      &colAttach,
      {1.0f, 1.0f, 1.0f, 1.0f},
  };

  // below this point, structs are not affected by the info

  const VkPipelineVertexInputStateCreateInfo vertexInput = {
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
  };

  VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
  };

  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

  VkPipelineViewportStateCreateInfo viewScissor = {
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
  viewScissor.viewportCount = viewScissor.scissorCount = 1;

  // add default scissor, if scissor is dynamic this will be ignored.
  VkRect2D scissor = {{0, 0}, {16384, 16384}};
  viewScissor.pScissors = &scissor;

  // can't really make a sensible one-size-fits-all default viewport like we can with scissors, so
  // make it small.
  VkViewport viewport = {0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f};
  viewScissor.pViewports = &viewport;

  VkPipelineRasterizationStateCreateInfo raster = {
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
  };

  raster.frontFace = VK_FRONT_FACE_CLOCKWISE;
  raster.lineWidth = 1.0f;

  const VkGraphicsPipelineCreateInfo graphicsPipeInfo = {
      VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      NULL,
      0,
      2,
      shaderStages,
      &vertexInput,
      &inputAssembly,
      NULL,    // tess
      &viewScissor,
      &raster,
      &msaa,
      &depthStencil,
      &colorBlend,
      &dynamicState,
      info.pipeLayout,
      info.renderPass,
      0,                 // sub pass
      VK_NULL_HANDLE,    // base pipeline handle
      -1,                // base pipeline index
  };

  VkResult vkr = vkCreateGraphicsPipelines(driver->GetVulkanState().m_Device, VK_NULL_HANDLE, 1,
                                                   &graphicsPipeInfo, NULL, pipe);
  if(vkr != VK_SUCCESS)
    RDCERR("Failed creating object %s at line %i, vkr was %s", objName, line, ToStr(vkr).c_str());
}

// utility macro that lets us check for VkResult failures inside the utility helpers while
// preserving context from outside
#define CREATE_OBJECT(obj, ...) create(driver, #obj, __LINE__, &obj, __VA_ARGS__)

void GXMReplay::MeshRendering::Init(WrappedGXM *driver, VkDescriptorPool descriptorPool)
{
  CREATE_OBJECT(DescSetLayout,
                {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL, NULL}});

  CREATE_OBJECT(PipeLayout, DescSetLayout, 0);
  CREATE_OBJECT(DescSet, descriptorPool, DescSetLayout);

  UBO.Create(driver, driver->m_vulkanState.m_Device, sizeof(MeshUBOData), 16, 0);
  BBoxVB.Create(driver, driver->m_vulkanState.m_Device, sizeof(Vec4f) * 128, 16,
                GXMGPUBuffer::eGPUBufferVBuffer);

  Vec4f TLN = Vec4f(-1.0f, 1.0f, 0.0f, 1.0f);    // TopLeftNear, etc...
  Vec4f TRN = Vec4f(1.0f, 1.0f, 0.0f, 1.0f);
  Vec4f BLN = Vec4f(-1.0f, -1.0f, 0.0f, 1.0f);
  Vec4f BRN = Vec4f(1.0f, -1.0f, 0.0f, 1.0f);

  Vec4f TLF = Vec4f(-1.0f, 1.0f, 1.0f, 1.0f);
  Vec4f TRF = Vec4f(1.0f, 1.0f, 1.0f, 1.0f);
  Vec4f BLF = Vec4f(-1.0f, -1.0f, 1.0f, 1.0f);
  Vec4f BRF = Vec4f(1.0f, -1.0f, 1.0f, 1.0f);

  Vec4f axisFrustum[] = {
      // axis marker vertices
      Vec4f(0.0f, 0.0f, 0.0f, 1.0f),
      Vec4f(1.0f, 0.0f, 0.0f, 1.0f),
      Vec4f(0.0f, 0.0f, 0.0f, 1.0f),
      Vec4f(0.0f, 1.0f, 0.0f, 1.0f),
      Vec4f(0.0f, 0.0f, 0.0f, 1.0f),
      Vec4f(0.0f, 0.0f, 1.0f, 1.0f),

      // frustum vertices
      TLN,
      TRN,
      TRN,
      BRN,
      BRN,
      BLN,
      BLN,
      TLN,

      TLN,
      TLF,
      TRN,
      TRF,
      BLN,
      BLF,
      BRN,
      BRF,

      TLF,
      TRF,
      TRF,
      BRF,
      BRF,
      BLF,
      BLF,
      TLF,
  };

  // doesn't need to be ring'd as it's immutable
  AxisFrustumVB.Create(driver, driver->m_vulkanState.m_Device, sizeof(axisFrustum), 1,
                       GXMGPUBuffer::eGPUBufferVBuffer);

  Vec4f *axisData = (Vec4f *)AxisFrustumVB.Map();

  memcpy(axisData, axisFrustum, sizeof(axisFrustum));

  AxisFrustumVB.Unmap();

  VkDescriptorBufferInfo meshrender = {};

  UBO.FillDescriptor(meshrender);

  VkWriteDescriptorSet writes[] = {
      {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, NULL, DescSet, 0, 0, 1,
       VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, NULL, &meshrender, NULL},
  };

  VkDevice dev = driver->m_vulkanState.m_Device;

  vkUpdateDescriptorSets(dev, ARRAY_COUNT(writes), writes, 0, NULL);
}

void GXMReplay::GeneralMisc::Init(WrappedGXM *driver, VkDescriptorPool descriptorPool)
{
  VkResult vkr = VK_SUCCESS;

  VkDescriptorPoolSize descPoolTypes[] = {
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 320},
      {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 32},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 128},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 128},
      {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 64},
      {VK_DESCRIPTOR_TYPE_SAMPLER, 32},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 32},
  };

  VkDescriptorPoolCreateInfo descPoolInfo = {
      VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      NULL,
      0,
      32,
      ARRAY_COUNT(descPoolTypes),
      &descPoolTypes[0],
  };

  // create descriptor pool
  vkr = vkCreateDescriptorPool(driver->GetVulkanState().m_Device, &descPoolInfo, NULL, &DescriptorPool);
  RDCASSERTEQUAL(vkr, VK_SUCCESS);

  CREATE_OBJECT(PointSampler, VK_FILTER_NEAREST);
}

GXMDebugManager::GXMDebugManager(WrappedGXM *driver)
{
  if(RenderDoc::Inst().GetCrashHandler())
    RenderDoc::Inst().GetCrashHandler()->RegisterMemoryRegion(this, sizeof(GXMDebugManager));

  m_pDriver = driver;

  m_Device = m_pDriver->GetVulkanState().m_Device;

  /*
  VkDevice dev = m_Device;

  VkResult vkr = VK_SUCCESS;
  
  // we need just one descriptor for MS<->Array
  VkDescriptorPoolSize poolTypes[] = {
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2},
      {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1},
  };

  VkDescriptorPoolCreateInfo poolInfo = {
      VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      NULL,
      0,
      1,
      ARRAY_COUNT(poolTypes),
      &poolTypes[0],
  };

  CREATE_OBJECT(m_ArrayMSSampler, VK_FILTER_NEAREST);

  rm->SetInternalResource(GetResID(m_ArrayMSSampler));

  vkr = vkCreateDescriptorPool(dev, &poolInfo, NULL, &m_ArrayMSDescriptorPool);
  RDCASSERTEQUAL(vkr, VK_SUCCESS);

  rm->SetInternalResource(GetResID(m_ArrayMSDescriptorPool));

  CREATE_OBJECT(m_ArrayMSDescSetLayout,
                {
                    {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, NULL},
                    {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, NULL},
                    {2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, NULL},
                });

  rm->SetInternalResource(GetResID(m_ArrayMSDescSetLayout));

  CREATE_OBJECT(m_ArrayMSPipeLayout, m_ArrayMSDescSetLayout, sizeof(Vec4u));

  rm->SetInternalResource(GetResID(m_ArrayMSPipeLayout));

  //////////////////////////////////////////////////////////////////
  // Color MS to Array copy (via compute)

  CREATE_OBJECT(m_MS2ArrayPipe, m_ArrayMSPipeLayout,
                shaderCache->GetBuiltinModule(BuiltinShader::MS2ArrayCS));
  CREATE_OBJECT(m_Array2MSPipe, m_ArrayMSPipeLayout,
                shaderCache->GetBuiltinModule(BuiltinShader::Array2MSCS));

  rm->SetInternalResource(GetResID(m_MS2ArrayPipe));
  rm->SetInternalResource(GetResID(m_Array2MSPipe));

  //////////////////////////////////////////////////////////////////
  // Depth MS to Array copy (via graphics)

  // need a dummy UINT texture to fill the binding when we don't have a stencil aspect to copy.
  // unfortunately there's no single guaranteed UINT format that can be sampled as MSAA, so we try a
  // few since hopefully we'll find one that will work.
  VkFormat attemptFormats[] = {VK_FORMAT_R8G8B8A8_UINT,     VK_FORMAT_R8_UINT,
                               VK_FORMAT_S8_UINT,           VK_FORMAT_D32_SFLOAT_S8_UINT,
                               VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT};

  for(VkFormat f : attemptFormats)
  {
    VkImageAspectFlags viewAspectMask =
        IsStencilFormat(f) ? VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    VkImageAspectFlags barrierAspectMask = viewAspectMask;

    if(IsDepthAndStencilFormat(f) && (barrierAspectMask & VK_IMAGE_ASPECT_STENCIL_BIT))
      barrierAspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;

    VkFormatProperties props = {};
    driver->vkGetPhysicalDeviceFormatProperties(driver->GetPhysDev(), f, &props);

    if(!(props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))
      continue;

    VkImageCreateInfo imInfo = {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        NULL,
        0,
        VK_IMAGE_TYPE_2D,
        f,
        {1, 1, 1},
        1,
        1,
        VK_SAMPLE_COUNT_1_BIT,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        NULL,
        VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VkImageFormatProperties imgprops = {};
    vkr = vkGetPhysicalDeviceImageFormatProperties(driver->GetVulkanState().m_Gpu, f,
                                                           imInfo.imageType, imInfo.tiling,
                                                           imInfo.usage, imInfo.flags, &imgprops);

    if(vkr == VK_ERROR_FORMAT_NOT_SUPPORTED)
      continue;

    // if it doesn't support MSAA, bail out
    if(imgprops.sampleCounts == VK_SAMPLE_COUNT_1_BIT)
      continue;

    vkr = vkCreateImage(dev, &imInfo, NULL, &m_DummyStencilImage[0]);
    RDCASSERTEQUAL(vkr, VK_SUCCESS);

    rm->SetInternalResource(GetResID(m_DummyStencilImage[0]));

    imInfo.samples = VK_SAMPLE_COUNT_2_BIT;

    // MoltenVK seems to only support 4/8 samples and not 2...
    if(imgprops.sampleCounts & VK_SAMPLE_COUNT_2_BIT)
      imInfo.samples = VK_SAMPLE_COUNT_2_BIT;
    else if(imgprops.sampleCounts & VK_SAMPLE_COUNT_4_BIT)
      imInfo.samples = VK_SAMPLE_COUNT_4_BIT;
    else if(imgprops.sampleCounts & VK_SAMPLE_COUNT_8_BIT)
      imInfo.samples = VK_SAMPLE_COUNT_8_BIT;
    else if(imgprops.sampleCounts & VK_SAMPLE_COUNT_16_BIT)
      imInfo.samples = VK_SAMPLE_COUNT_16_BIT;
    else if(imgprops.sampleCounts & VK_SAMPLE_COUNT_32_BIT)
      imInfo.samples = VK_SAMPLE_COUNT_32_BIT;
    else
      RDCWARN("Can't find supported MSAA sample count");

    RDCASSERT(imgprops.sampleCounts & imInfo.samples, imgprops.sampleCounts, imInfo.samples);

    vkr = vkCreateImage(dev, &imInfo, NULL, &m_DummyStencilImage[1]);
    RDCASSERTEQUAL(vkr, VK_SUCCESS);

    rm->SetInternalResource(GetResID(m_DummyStencilImage[1]));

    VkMemoryRequirements mrq[2] = {};
    vkGetImageMemoryRequirements(dev, m_DummyStencilImage[0], &mrq[0]);
    vkGetImageMemoryRequirements(dev, m_DummyStencilImage[1], &mrq[1]);

    uint32_t memoryTypeBits = (mrq[0].memoryTypeBits & mrq[1].memoryTypeBits);

    // assume we have some memory type available in common
    RDCASSERT(memoryTypeBits, mrq[0].memoryTypeBits, mrq[1].memoryTypeBits);

    // allocate memory
    VkMemoryAllocateInfo allocInfo = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        NULL,
        AlignUp(mrq[0].size, mrq[1].alignment) + mrq[1].size,
        driver->GetGPULocalMemoryIndex(memoryTypeBits),
    };

    vkr = vkAllocateMemory(dev), &allocInfo, NULL, &m_DummyStencilMemory);
    RDCASSERTEQUAL(vkr, VK_SUCCESS);

    rm->SetInternalResource(GetResID(m_DummyStencilMemory));

    vkr = vkBindImageMemory(dev, m_DummyStencilImage[0], m_DummyStencilMemory, 0);
    RDCASSERTEQUAL(vkr, VK_SUCCESS);

    vkr = vkBindImageMemory(dev, m_DummyStencilImage[1], m_DummyStencilMemory,
                                    AlignUp(mrq[0].size, mrq[1].alignment));
    RDCASSERTEQUAL(vkr, VK_SUCCESS);

    VkImageViewCreateInfo viewInfo = {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        NULL,
        0,
        m_DummyStencilImage[0],
        VK_IMAGE_VIEW_TYPE_2D_ARRAY,
        f,
        {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
         VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
        {
            viewAspectMask,
            0,
            1,
            0,
            1,
        },
    };

    vkr = vkCreateImageView(dev, &viewInfo, NULL, &m_DummyStencilView[0]);
    RDCASSERTEQUAL(vkr, VK_SUCCESS);

    rm->SetInternalResource(GetResID(m_DummyStencilView[0]));

    viewInfo.image = m_DummyStencilImage[1];

    vkr = vkCreateImageView(dev, &viewInfo, NULL, &m_DummyStencilView[1]);
    RDCASSERTEQUAL(vkr, VK_SUCCESS);

    rm->SetInternalResource(GetResID(m_DummyStencilView[1]));

    VkCommandBuffer cmd = driver->GetNextCmd();

    VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, NULL,
                                          VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};

    vkr = vkBeginCommandBuffer(cmd, &beginInfo);
    RDCASSERTEQUAL(vkr, VK_SUCCESS);

    // need to update image layout into valid state
    VkImageMemoryBarrier barrier = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        NULL,
        0,
        VK_ACCESS_SHADER_READ_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        m_DummyStencilImage[0],
        {barrierAspectMask, 0, 1, 0, 1},
    };

    DoPipelineBarrier(cmd, 1, &barrier);

    barrier.image = m_DummyStencilImage[1];

    DoPipelineBarrier(cmd, 1, &barrier);

    vkEndCommandBuffer(cmd);

    break;
  }

  if(m_DummyStencilImage[0] == VK_NULL_HANDLE)
  {
    RDCERR("Couldn't find any integer format we could generate a dummy multisampled image with");
  }

  CREATE_OBJECT(m_ArrayMSDescSet, m_ArrayMSDescriptorPool, m_ArrayMSDescSetLayout);

  rm->SetInternalResource(GetResID(m_ArrayMSDescSet));

  VkFormat formats[] = {
      VK_FORMAT_D16_UNORM,         VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_X8_D24_UNORM_PACK32,
      VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT,        VK_FORMAT_D32_SFLOAT_S8_UINT,
  };

  VkSampleCountFlagBits sampleCounts[] = {
      VK_SAMPLE_COUNT_2_BIT,
      VK_SAMPLE_COUNT_4_BIT,
      VK_SAMPLE_COUNT_8_BIT,
      VK_SAMPLE_COUNT_16_BIT,
  };

  RDCCOMPILE_ASSERT(ARRAY_COUNT(m_DepthMS2ArrayPipe) == ARRAY_COUNT(formats),
                    "Array count mismatch");
  RDCCOMPILE_ASSERT(ARRAY_COUNT(m_DepthArray2MSPipe) == ARRAY_COUNT(formats),
                    "Array count mismatch");
  RDCCOMPILE_ASSERT(ARRAY_COUNT(m_DepthArray2MSPipe[0]) == ARRAY_COUNT(sampleCounts),
                    "Array count mismatch");

  // we use VK_IMAGE_LAYOUT_GENERAL here because it matches the expected layout for the
  // non-depth copy, which uses a storage image.
  VkImageLayout rpLayout = VK_IMAGE_LAYOUT_GENERAL;

  for(size_t f = 0; f < ARRAY_COUNT(formats); f++)
  {
    // if the format isn't supported at all, bail out and don't try to create anything
    if(!(m_pDriver->GetFormatProperties(formats[f]).optimalTilingFeatures &
         VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
    {
      RDCDEBUG("Depth copies MSAA -> Array not supported for format %s", ToStr(formats[f]).c_str());
      continue;
    }

    VkRenderPass depthMS2ArrayRP = VK_NULL_HANDLE;

    CREATE_OBJECT(depthMS2ArrayRP, formats[f], VK_SAMPLE_COUNT_1_BIT, rpLayout);

    ConciseGraphicsPipeline depthPipeInfo = {
        depthMS2ArrayRP,
        m_ArrayMSPipeLayout,
        shaderCache->GetBuiltinModule(BuiltinShader::BlitVS),
        shaderCache->GetBuiltinModule(BuiltinShader::DepthMS2ArrayFS),
        {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_STENCIL_REFERENCE},
        VK_SAMPLE_COUNT_1_BIT,
        false,    // sampleRateShading
        true,     // depthEnable
        true,     // stencilEnable
        VK_STENCIL_OP_REPLACE,
        false,    // colourOutput
        false,    // blendEnable
        VK_BLEND_FACTOR_ONE,
        VK_BLEND_FACTOR_ZERO,
        0xf,    // writeMask
    };

    CREATE_OBJECT(m_DepthMS2ArrayPipe[f], depthPipeInfo);

    rm->SetInternalResource(GetResID(m_DepthMS2ArrayPipe[f]));

    vkDestroyRenderPass(dev, depthMS2ArrayRP, NULL);

    if(!m_pDriver->GetDeviceFeatures().sampleRateShading)
    {
      RDCDEBUG("No depth Array -> MSAA copies can be supported without sample rate shading");
      continue;
    }

    for(size_t s = 0; s < ARRAY_COUNT(sampleCounts); s++)
    {
      // if this sample count isn't supported, don't create it
      if(!(m_pDriver->GetDeviceProps().limits.framebufferDepthSampleCounts &
           (uint32_t)sampleCounts[s]))
      {
        RDCDEBUG("Depth copies Array -> MSAA not supported for sample count %u on format %s",
                 sampleCounts[s], ToStr(formats[f]).c_str());
        continue;
      }

      VkRenderPass depthArray2MSRP;

      CREATE_OBJECT(depthArray2MSRP, formats[f], sampleCounts[s], rpLayout);

      depthPipeInfo.fragment = shaderCache->GetBuiltinModule(BuiltinShader::DepthArray2MSFS);
      depthPipeInfo.renderPass = depthArray2MSRP;
      depthPipeInfo.sampleCount = sampleCounts[s];
      depthPipeInfo.sampleRateShading = true;

      CREATE_OBJECT(m_DepthArray2MSPipe[f][s], depthPipeInfo);

      rm->SetInternalResource(GetResID(m_DepthArray2MSPipe[f][s]));

      vkDestroyRenderPass(dev, depthArray2MSRP, NULL);
    }
  }

  // we only need this during replay, so don't create otherwise.
  if(RenderDoc::Inst().IsReplayApp())
    m_ReadbackWindow.Create(driver, dev, STAGE_BUFFER_BYTE_SIZE, 1, GPUBuffer::eGPUBufferReadback);
    */
}

GXMDebugManager::~GXMDebugManager()
{
  VkDevice dev = m_Device;

  for(auto it = m_CachedMeshPipelines.begin(); it != m_CachedMeshPipelines.end(); ++it)
    for(uint32_t i = 0; i < MeshDisplayPipelines::ePipe_Count; i++)
      vkDestroyPipeline(dev, it->second.pipes[i], NULL);
}

MeshDisplayPipelines GXMDebugManager::CacheMeshDisplayPipelines(VkPipelineLayout pipeLayout,
                                                                const MeshFormat &primary,
                                                                const MeshFormat &secondary)
{
  // generate a key to look up the map
  uint64_t key = 0;

  uint64_t bit = 0;

  if(primary.indexByteStride == 4)
    key |= 1ULL << bit;
  bit++;

  RDCASSERT((uint32_t)primary.topology < 64);
  key |= uint64_t((uint32_t)primary.topology & 0x3f) << bit;
  bit += 6;

  VkFormat primaryFmt = MakeGXMVkFormat(primary.format);
  VkFormat secondaryFmt = secondary.vertexResourceId == ResourceId()
                              ? VK_FORMAT_UNDEFINED
                              : MakeGXMVkFormat(secondary.format);

  RDCCOMPILE_ASSERT(VK_FORMAT_RANGE_SIZE <= 255,
                    "Mesh pipeline cache key needs an extra bit for format");

  key |= uint64_t((uint32_t)primaryFmt & 0xff) << bit;
  bit += 8;

  key |= uint64_t((uint32_t)secondaryFmt & 0xff) << bit;
  bit += 8;

  RDCASSERT(primary.vertexByteStride <= 0xffff);
  key |= uint64_t((uint32_t)primary.vertexByteStride & 0xffff) << bit;
  bit += 16;

  if(secondary.vertexResourceId != ResourceId())
  {
    RDCASSERT(secondary.vertexByteStride <= 0xffff);
    key |= uint64_t((uint32_t)secondary.vertexByteStride & 0xffff) << bit;
  }
  bit += 16;

  if(primary.instanced)
    key |= 1ULL << bit;
  bit++;

  if(secondary.instanced)
    key |= 1ULL << bit;
  bit++;

  if(primary.allowRestart)
    key |= 1ULL << bit;
  bit++;

  // only 64 bits, make sure they all fit
  RDCASSERT(bit < 64);

  MeshDisplayPipelines &cache = m_CachedMeshPipelines[key];

  if(cache.pipes[(uint32_t)SolidShade::NoSolid] != VK_NULL_HANDLE)
    return cache;

  VkResult vkr = VK_SUCCESS;

  // should we try and evict old pipelines from the cache here?
  // or just keep them forever

  VkVertexInputBindingDescription binds[] = {
      // primary
      {0, primary.vertexByteStride,
       primary.instanced ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX},
      // secondary
      {1, secondary.vertexByteStride,
       secondary.instanced ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX}};

  RDCASSERT(primaryFmt != VK_FORMAT_UNDEFINED);

  VkVertexInputAttributeDescription vertAttrs[] = {
      // primary
      {
          0,
          0,
          primaryFmt,
          0,
      },
      // secondary
      {
          1,
          0,
          primaryFmt,
          0,
      },
  };

  VkPipelineVertexInputStateCreateInfo vi = {
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, NULL, 0, 1, binds, 2, vertAttrs,
  };

  VkPipelineShaderStageCreateInfo stages[3] = {
      {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, NULL, 0, VK_SHADER_STAGE_ALL_GRAPHICS,
       VK_NULL_HANDLE, "main", NULL},
      {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, NULL, 0, VK_SHADER_STAGE_ALL_GRAPHICS,
       VK_NULL_HANDLE, "main", NULL},
      {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, NULL, 0, VK_SHADER_STAGE_ALL_GRAPHICS,
       VK_NULL_HANDLE, "main", NULL},
  };

  VkPipelineInputAssemblyStateCreateInfo ia = {
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      NULL,
      0,
      primary.topology >= Topology::PatchList ? VK_PRIMITIVE_TOPOLOGY_POINT_LIST
                                              : MakeGXMVkPrimitiveTopology(primary.topology),
      false,
  };

  ia.primitiveRestartEnable = primary.allowRestart && SupportsRestart(primary.topology);

  VkRect2D scissor = {{0, 0}, {16384, 16384}};

  VkPipelineViewportStateCreateInfo vp = {
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, NULL, 0, 1, NULL, 1, &scissor};

  VkPipelineRasterizationStateCreateInfo rs = {
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      NULL,
      0,
      false,
      false,
      VK_POLYGON_MODE_FILL,
      VK_CULL_MODE_NONE,
      VK_FRONT_FACE_CLOCKWISE,
      false,
      0.0f,
      0.0f,
      0.0f,
      1.0f,
  };

  VkPipelineMultisampleStateCreateInfo msaa = {
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      NULL,
      0,
      VULKAN_MESH_VIEW_SAMPLES,
      false,
      0.0f,
      NULL,
      false,
      false};

  VkPipelineDepthStencilStateCreateInfo ds = {
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      NULL,
      0,
      true,
      true,
      VK_COMPARE_OP_LESS_OR_EQUAL,
      false,
      false,
      {VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_ALWAYS, 0, 0, 0},
      {VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_ALWAYS, 0, 0, 0},
      0.0f,
      1.0f,
  };

  VkPipelineColorBlendAttachmentState attState = {
      false,
      VK_BLEND_FACTOR_ONE,
      VK_BLEND_FACTOR_ZERO,
      VK_BLEND_OP_ADD,
      VK_BLEND_FACTOR_ONE,
      VK_BLEND_FACTOR_ZERO,
      VK_BLEND_OP_ADD,
      0xf,
  };

  VkPipelineColorBlendStateCreateInfo cb = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                                            NULL,
                                            0,
                                            false,
                                            VK_LOGIC_OP_NO_OP,
                                            1,
                                            &attState,
                                            {1.0f, 1.0f, 1.0f, 1.0f}};

  VkDynamicState dynstates[] = {VK_DYNAMIC_STATE_VIEWPORT};

  VkPipelineDynamicStateCreateInfo dyn = {
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      NULL,
      0,
      ARRAY_COUNT(dynstates),
      dynstates,
  };

  VkRenderPass rp;    // compatible render pass

  {
    VkAttachmentDescription attDesc[] = {
        {0, VK_FORMAT_R8G8B8A8_SRGB, VULKAN_MESH_VIEW_SAMPLES, VK_ATTACHMENT_LOAD_OP_LOAD,
         VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
         VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {0, VK_FORMAT_D32_SFLOAT, VULKAN_MESH_VIEW_SAMPLES, VK_ATTACHMENT_LOAD_OP_LOAD,
         VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
         VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
         VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL},
    };

    VkAttachmentReference attRef = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkAttachmentReference dsRef = {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    VkSubpassDescription sub = {
        0,      VK_PIPELINE_BIND_POINT_GRAPHICS,
        0,      NULL,       // inputs
        1,      &attRef,    // color
        NULL,               // resolve
        &dsRef,             // depth-stencil
        0,      NULL,       // preserve
    };

    VkRenderPassCreateInfo rpinfo = {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        NULL,
        0,
        2,
        attDesc,
        1,
        &sub,
        0,
        NULL,    // dependencies
    };

    vkCreateRenderPass(m_Device, &rpinfo, NULL, &rp);
  }

  VkGraphicsPipelineCreateInfo pipeInfo = {
      VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      NULL,
      0,
      2,
      stages,
      &vi,
      &ia,
      NULL,    // tess
      &vp,
      &rs,
      &msaa,
      &ds,
      &cb,
      &dyn,
      pipeLayout,
      rp,
      0,                 // sub pass
      VK_NULL_HANDLE,    // base pipeline handle
      0,                 // base pipeline index
  };

  // wireframe pipeline
  stages[0].module = m_pDriver->GetShaderCache()->GetBuiltinModule(BuiltinShader::MeshVS);
  stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  stages[1].module = m_pDriver->GetShaderCache()->GetBuiltinModule(BuiltinShader::MeshFS);
  stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;

  rs.polygonMode = VK_POLYGON_MODE_LINE;
  rs.lineWidth = 1.0f;
  ds.depthTestEnable = false;

  // to be friendlier to implementations that don't support LINE polygon mode, when the topology is
  // already lines, we can just use fill. This is most commonly used for the helpers
  if(primary.topology == Topology::LineList)
    rs.polygonMode = VK_POLYGON_MODE_FILL;

  // if the device doesn't support non-solid fill mode, fall back to fill. We don't try to patch
  // index buffers for mesh render since it's not worth the trouble - mesh rendering happens locally
  // and typically the local machine is capable enough to do line raster.
  if(!m_pDriver->GetVulkanState().m_DeviceFeatures.fillModeNonSolid)
  {
    RDCWARN("Can't render mesh wireframes without non-solid fill mode support");
    rs.polygonMode = VK_POLYGON_MODE_FILL;
  }

  vkr = vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipeInfo, NULL,
                                  &cache.pipes[MeshDisplayPipelines::ePipe_Wire]);
  RDCASSERTEQUAL(vkr, VK_SUCCESS);

  ds.depthTestEnable = true;

  vkr = vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipeInfo, NULL,
                                  &cache.pipes[MeshDisplayPipelines::ePipe_WireDepth]);
  RDCASSERTEQUAL(vkr, VK_SUCCESS);

  // solid shading pipeline
  rs.polygonMode = VK_POLYGON_MODE_FILL;
  ds.depthTestEnable = false;

  vkr = vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipeInfo, NULL,
                                  &cache.pipes[MeshDisplayPipelines::ePipe_Solid]);
  RDCASSERTEQUAL(vkr, VK_SUCCESS);

  ds.depthTestEnable = true;

  vkr = vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipeInfo, NULL,
                                  &cache.pipes[MeshDisplayPipelines::ePipe_SolidDepth]);
  RDCASSERTEQUAL(vkr, VK_SUCCESS);

  if(secondary.vertexResourceId != ResourceId())
  {
    // pull secondary information from second vertex buffer
    vertAttrs[1].binding = 1;
    vertAttrs[1].format = secondaryFmt;
    RDCASSERT(secondaryFmt != VK_FORMAT_UNDEFINED);

    vi.vertexBindingDescriptionCount = 2;

    vkr = vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipeInfo, NULL,
                                    &cache.pipes[MeshDisplayPipelines::ePipe_Secondary]);
    RDCASSERTEQUAL(vkr, VK_SUCCESS);
  }

  vertAttrs[1].binding = 0;
  vi.vertexBindingDescriptionCount = 1;

  // flat lit pipeline, needs geometry shader to calculate face normals
  stages[2].module = m_pDriver->GetShaderCache()->GetBuiltinModule(BuiltinShader::MeshGS);
  stages[2].stage = VK_SHADER_STAGE_GEOMETRY_BIT;
  pipeInfo.stageCount = 3;

  if(stages[2].module != VK_NULL_HANDLE && ia.topology != VK_PRIMITIVE_TOPOLOGY_POINT_LIST)
  {
    vkr = vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipeInfo, NULL,
                                    &cache.pipes[MeshDisplayPipelines::ePipe_Lit]);
    RDCASSERTEQUAL(vkr, VK_SUCCESS);
  }

  vkDestroyRenderPass(m_Device, rp, NULL);

  return cache;
}

void GXMReplay::OverlayRendering::Init(WrappedGXM *driver, VkDescriptorPool descriptorPool)
{
  GXMVkShaderCache *shaderCache = driver->GetShaderCache();

  VkRenderPass SRGBA8RP = VK_NULL_HANDLE;
  VkRenderPass SRGBA8MSRP = VK_NULL_HANDLE;

  CREATE_OBJECT(SRGBA8RP, VK_FORMAT_R8G8B8A8_SRGB);
  CREATE_OBJECT(SRGBA8MSRP, VK_FORMAT_R8G8B8A8_SRGB, VULKAN_MESH_VIEW_SAMPLES);

  CREATE_OBJECT(m_CheckerDescSetLayout,
                {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL, NULL}});

  CREATE_OBJECT(m_QuadDescSetLayout,
                {
                    {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, NULL},
                });

  CREATE_OBJECT(m_TriSizeDescSetLayout,
                {
                    {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL, NULL},
                    {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL, NULL},
                });

  CREATE_OBJECT(m_CheckerPipeLayout, m_CheckerDescSetLayout, 0);
  CREATE_OBJECT(m_QuadResolvePipeLayout, m_QuadDescSetLayout, 0);
  CREATE_OBJECT(m_TriSizePipeLayout, m_TriSizeDescSetLayout, 0);
  CREATE_OBJECT(m_QuadDescSet, descriptorPool, m_QuadDescSetLayout);
  CREATE_OBJECT(m_TriSizeDescSet, descriptorPool, m_TriSizeDescSetLayout);
  CREATE_OBJECT(m_CheckerDescSet, descriptorPool, m_CheckerDescSetLayout);

  m_CheckerUBO.Create(driver, driver->m_vulkanState.m_Device, 128, 10, 0);
  RDCCOMPILE_ASSERT(sizeof(CheckerboardUBOData) <= 128, "checkerboard UBO size");

  m_TriSizeUBO.Create(driver, driver->m_vulkanState.m_Device, sizeof(Vec4f), 4096, 0);

  ConciseGraphicsPipeline pipeInfo = {
      SRGBA8RP,
      m_CheckerPipeLayout,
      shaderCache->GetBuiltinModule(BuiltinShader::BlitVS),
      shaderCache->GetBuiltinModule(BuiltinShader::CheckerboardFS),
      {VK_DYNAMIC_STATE_VIEWPORT},
      VK_SAMPLE_COUNT_1_BIT,
      false,    // sampleRateShading
      false,    // depthEnable
      false,    // stencilEnable
      VK_STENCIL_OP_KEEP,
      true,     // colourOutput
      false,    // blendEnable
      VK_BLEND_FACTOR_SRC_ALPHA,
      VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
      0xf,    // writeMask
  };

  CREATE_OBJECT(m_CheckerPipeline, pipeInfo);

  pipeInfo.renderPass = SRGBA8MSRP;
  pipeInfo.sampleCount = VULKAN_MESH_VIEW_SAMPLES;

  CREATE_OBJECT(m_CheckerMSAAPipeline, pipeInfo);

  uint32_t samplesHandled = 0;

  RDCCOMPILE_ASSERT(ARRAY_COUNT(m_CheckerF16Pipeline) == ARRAY_COUNT(m_QuadResolvePipeline),
                    "Arrays are mismatched in size!");

  uint32_t supportedSampleCounts = driver->GetVulkanState().m_DeviceProperties.limits.framebufferColorSampleCounts;

  for(size_t i = 0; i < ARRAY_COUNT(m_CheckerF16Pipeline); i++)
  {
    VkSampleCountFlagBits samples = VkSampleCountFlagBits(1 << i);

    if((supportedSampleCounts & (uint32_t)samples) == 0)
      continue;

    VkRenderPass RGBA16MSRP = VK_NULL_HANDLE;

    CREATE_OBJECT(RGBA16MSRP, VK_FORMAT_R16G16B16A16_SFLOAT, samples);

    if(RGBA16MSRP != VK_NULL_HANDLE)
      samplesHandled |= (uint32_t)samples;
    else
      continue;

    // if we this sample count is supported then create a pipeline
    pipeInfo.renderPass = RGBA16MSRP;
    pipeInfo.sampleCount = VkSampleCountFlagBits(1 << i);

    // set up outline pipeline configuration
    pipeInfo.blendEnable = true;
    pipeInfo.fragment = shaderCache->GetBuiltinModule(BuiltinShader::CheckerboardFS);
    pipeInfo.pipeLayout = m_CheckerPipeLayout;

    CREATE_OBJECT(m_CheckerF16Pipeline[i], pipeInfo);

    // set up quad resolve pipeline configuration
    pipeInfo.blendEnable = false;
    pipeInfo.fragment = shaderCache->GetBuiltinModule(BuiltinShader::QuadResolveFS);
    pipeInfo.pipeLayout = m_QuadResolvePipeLayout;

    if(pipeInfo.fragment != VK_NULL_HANDLE &&
       shaderCache->GetBuiltinModule(BuiltinShader::QuadWriteFS) != VK_NULL_HANDLE)
    {
      CREATE_OBJECT(m_QuadResolvePipeline[i], pipeInfo);
    }

    vkDestroyRenderPass(driver->m_vulkanState.m_Device, RGBA16MSRP, NULL);
  }

  RDCASSERTEQUAL((uint32_t)driver->GetVulkanState().m_DeviceProperties.limits.framebufferColorSampleCounts,
                 samplesHandled);

  VkDescriptorBufferInfo checkerboard = {};
  m_CheckerUBO.FillDescriptor(checkerboard);

  VkWriteDescriptorSet writes[] = {
      {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, NULL, m_CheckerDescSet, 0, 0, 1,
       VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, NULL, &checkerboard, NULL},
  };

  VkDevice dev = driver->m_vulkanState.m_Device;

  vkUpdateDescriptorSets(dev, ARRAY_COUNT(writes), writes, 0, NULL);

  vkDestroyRenderPass(driver->m_vulkanState.m_Device, SRGBA8RP, NULL);
  vkDestroyRenderPass(driver->m_vulkanState.m_Device, SRGBA8MSRP, NULL);
}
