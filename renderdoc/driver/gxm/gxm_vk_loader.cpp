
#include "common/threading.h"
#include "os/os_specific.h"

#include "gxm_vk_loader.h"

#define CallDefine(name) \
CONCAT(PFN_vk, name) CONCAT(vk, name) = nullptr

CallDefine(GetDeviceProcAddr);
CallDefine(GetInstanceProcAddr);
CallDefine(EnumerateInstanceExtensionProperties);
CallDefine(EnumerateInstanceLayerProperties);
CallDefine(CreateInstance);
CallDefine(DestroyInstance);
CallDefine(EnumeratePhysicalDevices);
CallDefine(GetPhysicalDeviceFeatures);
CallDefine(GetPhysicalDeviceImageFormatProperties);
CallDefine(GetPhysicalDeviceFormatProperties);
CallDefine(GetPhysicalDeviceSparseImageFormatProperties);
CallDefine(GetPhysicalDeviceProperties);
CallDefine(GetPhysicalDeviceQueueFamilyProperties);
CallDefine(GetPhysicalDeviceMemoryProperties);

CallDefine(EnumerateDeviceExtensionProperties);
CallDefine(EnumerateDeviceLayerProperties);

CallDefine(CreateDevice);
CallDefine(DestroyDevice);
CallDefine(GetDeviceQueue);
CallDefine(QueueSubmit);
CallDefine(QueueWaitIdle);
CallDefine(DeviceWaitIdle);
CallDefine(AllocateMemory);
CallDefine(FreeMemory);
CallDefine(MapMemory);
CallDefine(UnmapMemory);
CallDefine(FlushMappedMemoryRanges);
CallDefine(InvalidateMappedMemoryRanges);
CallDefine(GetDeviceMemoryCommitment);
CallDefine(BindBufferMemory);
CallDefine(BindImageMemory);
CallDefine(QueueBindSparse);
CallDefine(CreateBuffer);
CallDefine(DestroyBuffer);
CallDefine(CreateBufferView);
CallDefine(DestroyBufferView);
CallDefine(CreateImage);
CallDefine(DestroyImage);
CallDefine(GetImageSubresourceLayout);
CallDefine(GetBufferMemoryRequirements);
CallDefine(GetImageMemoryRequirements);
CallDefine(GetImageSparseMemoryRequirements);
CallDefine(CreateImageView);
CallDefine(DestroyImageView);
CallDefine(CreateShaderModule);
CallDefine(DestroyShaderModule);
CallDefine(CreateGraphicsPipelines);
CallDefine(CreateComputePipelines);
CallDefine(DestroyPipeline);
CallDefine(CreatePipelineCache);
CallDefine(GetPipelineCacheData);
CallDefine(MergePipelineCaches);
CallDefine(DestroyPipelineCache);
CallDefine(CreatePipelineLayout);
CallDefine(DestroyPipelineLayout);
#undef CreateSemaphore
CallDefine(CreateSemaphore);
CallDefine(DestroySemaphore);
CallDefine(CreateFence);
CallDefine(GetFenceStatus);
CallDefine(ResetFences);
CallDefine(WaitForFences);
CallDefine(DestroyFence);
#undef CreateEvent
CallDefine(CreateEvent);
CallDefine(GetEventStatus);
CallDefine(ResetEvent);
CallDefine(SetEvent);
CallDefine(DestroyEvent);
CallDefine(CreateQueryPool);
CallDefine(GetQueryPoolResults);
CallDefine(DestroyQueryPool);
CallDefine(CreateSampler);
CallDefine(DestroySampler);
CallDefine(CreateDescriptorSetLayout);
CallDefine(DestroyDescriptorSetLayout);
CallDefine(CreateDescriptorPool);
CallDefine(ResetDescriptorPool);
CallDefine(DestroyDescriptorPool);
CallDefine(AllocateDescriptorSets);
CallDefine(UpdateDescriptorSets);
CallDefine(FreeDescriptorSets);
CallDefine(GetRenderAreaGranularity);
CallDefine(CreateCommandPool);
CallDefine(DestroyCommandPool);
CallDefine(ResetCommandPool);
CallDefine(AllocateCommandBuffers);
CallDefine(FreeCommandBuffers);
CallDefine(BeginCommandBuffer);
CallDefine(EndCommandBuffer);
CallDefine(ResetCommandBuffer);
CallDefine(CmdBindPipeline);
CallDefine(CmdSetViewport);
CallDefine(CmdSetScissor);
CallDefine(CmdSetLineWidth);
CallDefine(CmdSetDepthBias);
CallDefine(CmdSetBlendConstants);
CallDefine(CmdSetDepthBounds);
CallDefine(CmdSetStencilCompareMask);
CallDefine(CmdSetStencilWriteMask);
CallDefine(CmdSetStencilReference);
CallDefine(CmdBindDescriptorSets);
CallDefine(CmdBindVertexBuffers);
CallDefine(CmdBindIndexBuffer);
CallDefine(CmdDraw);
CallDefine(CmdDrawIndirect);
CallDefine(CmdDrawIndexed);
CallDefine(CmdDrawIndexedIndirect);
CallDefine(CmdDispatch);
CallDefine(CmdDispatchIndirect);
CallDefine(CmdCopyBufferToImage);
CallDefine(CmdCopyImageToBuffer);
CallDefine(CmdCopyBuffer);
CallDefine(CmdCopyImage);
CallDefine(CmdBlitImage);
CallDefine(CmdResolveImage);
CallDefine(CmdUpdateBuffer);
CallDefine(CmdFillBuffer);
CallDefine(CmdPushConstants);
CallDefine(CmdClearColorImage);
CallDefine(CmdClearDepthStencilImage);
CallDefine(CmdClearAttachments);
CallDefine(CmdPipelineBarrier);
CallDefine(CmdWriteTimestamp);
CallDefine(CmdCopyQueryPoolResults);
CallDefine(CmdBeginQuery);
CallDefine(CmdEndQuery);
CallDefine(CmdResetQueryPool);
CallDefine(CmdSetEvent);
CallDefine(CmdResetEvent);
CallDefine(CmdWaitEvents);
CallDefine(CreateFramebuffer);
CallDefine(DestroyFramebuffer);
CallDefine(CreateRenderPass);
CallDefine(DestroyRenderPass);
CallDefine(CmdBeginRenderPass);
CallDefine(CmdNextSubpass);
CallDefine(CmdExecuteCommands);
CallDefine(CmdEndRenderPass);

#if defined(VK_USE_PLATFORM_WIN32_KHR)

CallDefine(CreateWin32SurfaceKHR);
CallDefine(GetPhysicalDeviceWin32PresentationSupportKHR);

#endif

CallDefine(DestroySurfaceKHR);
CallDefine(GetPhysicalDeviceSurfaceSupportKHR);
CallDefine(GetPhysicalDeviceSurfaceCapabilitiesKHR);
CallDefine(GetPhysicalDeviceSurfaceFormatsKHR);
CallDefine(GetPhysicalDeviceSurfacePresentModesKHR);

CallDefine(CreateSwapchainKHR);
CallDefine(DestroySwapchainKHR);
CallDefine(GetSwapchainImagesKHR);
CallDefine(AcquireNextImageKHR);
CallDefine(QueuePresentKHR);

CallDefine(CreateDebugUtilsMessengerEXT);
CallDefine(DestroyDebugUtilsMessengerEXT);

void InitialiseVulkanCalls(void *vulkanModule)
{
#undef CallInit
#define CallInit(name) \
CONCAT(vk, name) = (CONCAT(PFN_vk, name))Process::GetFunctionAddress(vulkanModule, STRINGIZE(CONCAT(vk, name)))

  CallInit(GetInstanceProcAddr);
  CallInit(GetDeviceProcAddr);
  CallInit(CreateInstance);
  CallInit(DestroyInstance);
  CallInit(EnumerateInstanceExtensionProperties);
  CallInit(EnumerateInstanceLayerProperties);
}

void InitialiseVulkanInstanceCalls(VkInstance instance)
{
#undef CallInit
#define CallInit(name) \
  CONCAT(vk, name) = (CONCAT(PFN_vk, name))vkGetInstanceProcAddr(instance, STRINGIZE(CONCAT(vk, name)))

  CallInit(EnumeratePhysicalDevices);
  CallInit(GetPhysicalDeviceFeatures);
  CallInit(GetPhysicalDeviceImageFormatProperties);
  CallInit(GetPhysicalDeviceFormatProperties);
  CallInit(GetPhysicalDeviceSparseImageFormatProperties);
  CallInit(GetPhysicalDeviceProperties);
  CallInit(GetPhysicalDeviceQueueFamilyProperties);
  CallInit(GetPhysicalDeviceMemoryProperties);

  CallInit(EnumerateDeviceExtensionProperties);
  CallInit(EnumerateDeviceLayerProperties);

  CallInit(CreateDevice);
  CallInit(DestroyDevice);

#if defined(VK_USE_PLATFORM_WIN32_KHR)

  CallInit(CreateWin32SurfaceKHR);
  CallInit(GetPhysicalDeviceWin32PresentationSupportKHR);

#endif

  CallInit(DestroySurfaceKHR);
  CallInit(GetPhysicalDeviceSurfaceSupportKHR);
  CallInit(GetPhysicalDeviceSurfaceCapabilitiesKHR);
  CallInit(GetPhysicalDeviceSurfaceFormatsKHR);
  CallInit(GetPhysicalDeviceSurfacePresentModesKHR);

  CallInit(CreateDebugUtilsMessengerEXT);
  CallInit(DestroyDebugUtilsMessengerEXT);
}

void InitialiseVulkanDeviceCalls(VkDevice device)
{
#undef CallInit
#define CallInit(name) \
  CONCAT(vk, name) = (CONCAT(PFN_vk, name))vkGetDeviceProcAddr(device, STRINGIZE(CONCAT(vk, name)))

  CallInit(GetDeviceQueue);
  CallInit(QueueSubmit);
  CallInit(QueueWaitIdle);
  CallInit(DeviceWaitIdle);
  CallInit(AllocateMemory);
  CallInit(FreeMemory);
  CallInit(MapMemory);
  CallInit(UnmapMemory);
  CallInit(FlushMappedMemoryRanges);
  CallInit(InvalidateMappedMemoryRanges);
  CallInit(GetDeviceMemoryCommitment);
  CallInit(BindBufferMemory);
  CallInit(BindImageMemory);
  CallInit(QueueBindSparse);
  CallInit(CreateBuffer);
  CallInit(DestroyBuffer);
  CallInit(CreateBufferView);
  CallInit(DestroyBufferView);
  CallInit(CreateImage);
  CallInit(DestroyImage);
  CallInit(GetImageSubresourceLayout);
  CallInit(GetBufferMemoryRequirements);
  CallInit(GetImageMemoryRequirements);
  CallInit(GetImageSparseMemoryRequirements);
  CallInit(CreateImageView);
  CallInit(DestroyImageView);
  CallInit(CreateShaderModule);
  CallInit(DestroyShaderModule);
  CallInit(CreateGraphicsPipelines);
  CallInit(CreateComputePipelines);
  CallInit(DestroyPipeline);
  CallInit(CreatePipelineCache);
  CallInit(GetPipelineCacheData);
  CallInit(MergePipelineCaches);
  CallInit(DestroyPipelineCache);
  CallInit(CreatePipelineLayout);
  CallInit(DestroyPipelineLayout);
#undef CreateSemaphore
  CallInit(CreateSemaphore);
  CallInit(DestroySemaphore);
  CallInit(CreateFence);
  CallInit(GetFenceStatus);
  CallInit(ResetFences);
  CallInit(WaitForFences);
  CallInit(DestroyFence);
#undef CreateEvent
  CallInit(CreateEvent);
  CallInit(GetEventStatus);
  CallInit(ResetEvent);
  CallInit(SetEvent);
  CallInit(DestroyEvent);
  CallInit(CreateQueryPool);
  CallInit(GetQueryPoolResults);
  CallInit(DestroyQueryPool);
  CallInit(CreateSampler);
  CallInit(DestroySampler);
  CallInit(CreateDescriptorSetLayout);
  CallInit(DestroyDescriptorSetLayout);
  CallInit(CreateDescriptorPool);
  CallInit(ResetDescriptorPool);
  CallInit(DestroyDescriptorPool);
  CallInit(AllocateDescriptorSets);
  CallInit(UpdateDescriptorSets);
  CallInit(FreeDescriptorSets);
  CallInit(GetRenderAreaGranularity);
  CallInit(CreateCommandPool);
  CallInit(DestroyCommandPool);
  CallInit(ResetCommandPool);
  CallInit(AllocateCommandBuffers);
  CallInit(FreeCommandBuffers);
  CallInit(BeginCommandBuffer);
  CallInit(EndCommandBuffer);
  CallInit(ResetCommandBuffer);
  CallInit(CmdBindPipeline);
  CallInit(CmdSetViewport);
  CallInit(CmdSetScissor);
  CallInit(CmdSetLineWidth);
  CallInit(CmdSetDepthBias);
  CallInit(CmdSetBlendConstants);
  CallInit(CmdSetDepthBounds);
  CallInit(CmdSetStencilCompareMask);
  CallInit(CmdSetStencilWriteMask);
  CallInit(CmdSetStencilReference);
  CallInit(CmdBindDescriptorSets);
  CallInit(CmdBindVertexBuffers);
  CallInit(CmdBindIndexBuffer);
  CallInit(CmdDraw);
  CallInit(CmdDrawIndirect);
  CallInit(CmdDrawIndexed);
  CallInit(CmdDrawIndexedIndirect);
  CallInit(CmdDispatch);
  CallInit(CmdDispatchIndirect);
  CallInit(CmdCopyBufferToImage);
  CallInit(CmdCopyImageToBuffer);
  CallInit(CmdCopyBuffer);
  CallInit(CmdCopyImage);
  CallInit(CmdBlitImage);
  CallInit(CmdResolveImage);
  CallInit(CmdUpdateBuffer);
  CallInit(CmdFillBuffer);
  CallInit(CmdPushConstants);
  CallInit(CmdClearColorImage);
  CallInit(CmdClearDepthStencilImage);
  CallInit(CmdClearAttachments);
  CallInit(CmdPipelineBarrier);
  CallInit(CmdWriteTimestamp);
  CallInit(CmdCopyQueryPoolResults);
  CallInit(CmdBeginQuery);
  CallInit(CmdEndQuery);
  CallInit(CmdResetQueryPool);
  CallInit(CmdSetEvent);
  CallInit(CmdResetEvent);
  CallInit(CmdWaitEvents);
  CallInit(CreateFramebuffer);
  CallInit(DestroyFramebuffer);
  CallInit(CreateRenderPass);
  CallInit(DestroyRenderPass);
  CallInit(CmdBeginRenderPass);
  CallInit(CmdNextSubpass);
  CallInit(CmdExecuteCommands);
  CallInit(CmdEndRenderPass);

  CallInit(CreateSwapchainKHR);
  CallInit(DestroySwapchainKHR);
  CallInit(GetSwapchainImagesKHR);
  CallInit(AcquireNextImageKHR);
  CallInit(QueuePresentKHR);
}