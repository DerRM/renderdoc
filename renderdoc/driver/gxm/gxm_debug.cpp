#include "gxm_replay.h"
#include "gxm_vk_shader_cache.h"
/*
void GXMReplay::OverlayRendering::Init(WrappedGXM *driver, VkDescriptorPool descriptorPool)
{
  VulkanShaderCache *shaderCache = driver->GetShaderCache();

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

  uint32_t supportedSampleCounts = driver->GetDeviceProps().limits.framebufferColorSampleCounts;

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

  RDCASSERTEQUAL((uint32_t)driver->GetDeviceProps().limits.framebufferColorSampleCounts,
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

void GXMReplay::OverlayRendering::Destroy(WrappedGXM *driver)
{
  if(ImageMem == VK_NULL_HANDLE)
    return;

  vkFreeMemory(driver->m_vulkanState.m_Device, ImageMem, NULL);
  vkDestroyImage(driver->m_vulkanState.m_Device, Image, NULL);
  vkDestroyImageView(driver->m_vulkanState.m_Device, ImageView, NULL);
  vkDestroyFramebuffer(driver->m_vulkanState.m_Device, NoDepthFB, NULL);
  vkDestroyRenderPass(driver->m_vulkanState.m_Device, NoDepthRP, NULL);

  vkDestroyDescriptorSetLayout(driver->m_vulkanState.m_Device, m_QuadDescSetLayout, NULL);
  vkDestroyPipelineLayout(driver->m_vulkanState.m_Device, m_QuadResolvePipeLayout, NULL);
  for(size_t i = 0; i < ARRAY_COUNT(m_QuadResolvePipeline); i++)
    vkDestroyPipeline(driver->m_vulkanState.m_Device, m_QuadResolvePipeline[i], NULL);

  vkDestroyDescriptorSetLayout(driver->m_vulkanState.m_Device, m_CheckerDescSetLayout, NULL);
  vkDestroyPipelineLayout(driver->m_vulkanState.m_Device, m_CheckerPipeLayout, NULL);
  for(size_t i = 0; i < ARRAY_COUNT(m_CheckerF16Pipeline); i++)
    vkDestroyPipeline(driver->m_vulkanState.m_Device, m_CheckerF16Pipeline[i], NULL);
  vkDestroyPipeline(driver->m_vulkanState.m_Device, m_CheckerPipeline, NULL);
  vkDestroyPipeline(driver->m_vulkanState.m_Device, m_CheckerMSAAPipeline, NULL);

  m_CheckerUBO.Destroy();

  m_TriSizeUBO.Destroy();
  vkDestroyDescriptorSetLayout(driver->m_vulkanState.m_Device, m_TriSizeDescSetLayout, NULL);
  vkDestroyPipelineLayout(driver->m_vulkanState.m_Device, m_TriSizePipeLayout, NULL);
}
*/