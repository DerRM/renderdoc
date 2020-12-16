#pragma once

#include "gxm_core.h"

#define CallDeclare(name) extern CONCAT(PFN_vk, name) CONCAT(vk, name)

CallDeclare(GetDeviceProcAddr);
CallDeclare(GetInstanceProcAddr);
CallDeclare(EnumerateInstanceExtensionProperties);
CallDeclare(EnumerateInstanceLayerProperties);
CallDeclare(CreateInstance);
CallDeclare(DestroyInstance);
CallDeclare(EnumeratePhysicalDevices);
CallDeclare(GetPhysicalDeviceFeatures);
CallDeclare(GetPhysicalDeviceImageFormatProperties);
CallDeclare(GetPhysicalDeviceFormatProperties);
CallDeclare(GetPhysicalDeviceSparseImageFormatProperties);
CallDeclare(GetPhysicalDeviceProperties);
CallDeclare(GetPhysicalDeviceQueueFamilyProperties);
CallDeclare(GetPhysicalDeviceMemoryProperties);

CallDeclare(EnumerateDeviceExtensionProperties);
CallDeclare(EnumerateDeviceLayerProperties);

CallDeclare(CreateDevice);
CallDeclare(DestroyDevice);
CallDeclare(GetDeviceQueue);
CallDeclare(QueueSubmit);
CallDeclare(QueueWaitIdle);
CallDeclare(DeviceWaitIdle);
CallDeclare(AllocateMemory);
CallDeclare(FreeMemory);
CallDeclare(MapMemory);
CallDeclare(UnmapMemory);
CallDeclare(FlushMappedMemoryRanges);
CallDeclare(InvalidateMappedMemoryRanges);
CallDeclare(GetDeviceMemoryCommitment);
CallDeclare(BindBufferMemory);
CallDeclare(BindImageMemory);
CallDeclare(QueueBindSparse);
CallDeclare(CreateBuffer);
CallDeclare(DestroyBuffer);
CallDeclare(CreateBufferView);
CallDeclare(DestroyBufferView);
CallDeclare(CreateImage);
CallDeclare(DestroyImage);
CallDeclare(GetImageSubresourceLayout);
CallDeclare(GetBufferMemoryRequirements);
CallDeclare(GetImageMemoryRequirements);
CallDeclare(GetImageSparseMemoryRequirements);
CallDeclare(CreateImageView);
CallDeclare(DestroyImageView);
CallDeclare(CreateShaderModule);
CallDeclare(DestroyShaderModule);
CallDeclare(CreateGraphicsPipelines);
CallDeclare(CreateComputePipelines);
CallDeclare(DestroyPipeline);
CallDeclare(CreatePipelineCache);
CallDeclare(GetPipelineCacheData);
CallDeclare(MergePipelineCaches);
CallDeclare(DestroyPipelineCache);
CallDeclare(CreatePipelineLayout);
CallDeclare(DestroyPipelineLayout);
#undef CreateSemaphore
CallDeclare(CreateSemaphore);
CallDeclare(DestroySemaphore);
CallDeclare(CreateFence);
CallDeclare(GetFenceStatus);
CallDeclare(ResetFences);
CallDeclare(WaitForFences);
CallDeclare(DestroyFence);
#undef CreateEvent
CallDeclare(CreateEvent);
CallDeclare(GetEventStatus);
CallDeclare(ResetEvent);
CallDeclare(SetEvent);
CallDeclare(DestroyEvent);
CallDeclare(CreateQueryPool);
CallDeclare(GetQueryPoolResults);
CallDeclare(DestroyQueryPool);
CallDeclare(CreateSampler);
CallDeclare(DestroySampler);
CallDeclare(CreateDescriptorSetLayout);
CallDeclare(DestroyDescriptorSetLayout);
CallDeclare(CreateDescriptorPool);
CallDeclare(ResetDescriptorPool);
CallDeclare(DestroyDescriptorPool);
CallDeclare(AllocateDescriptorSets);
CallDeclare(UpdateDescriptorSets);
CallDeclare(FreeDescriptorSets);
CallDeclare(GetRenderAreaGranularity);
CallDeclare(CreateCommandPool);
CallDeclare(DestroyCommandPool);
CallDeclare(ResetCommandPool);
CallDeclare(AllocateCommandBuffers);
CallDeclare(FreeCommandBuffers);
CallDeclare(BeginCommandBuffer);
CallDeclare(EndCommandBuffer);
CallDeclare(ResetCommandBuffer);
CallDeclare(CmdBindPipeline);
CallDeclare(CmdSetViewport);
CallDeclare(CmdSetScissor);
CallDeclare(CmdSetLineWidth);
CallDeclare(CmdSetDepthBias);
CallDeclare(CmdSetBlendConstants);
CallDeclare(CmdSetDepthBounds);
CallDeclare(CmdSetStencilCompareMask);
CallDeclare(CmdSetStencilWriteMask);
CallDeclare(CmdSetStencilReference);
CallDeclare(CmdBindDescriptorSets);
CallDeclare(CmdBindVertexBuffers);
CallDeclare(CmdBindIndexBuffer);
CallDeclare(CmdDraw);
CallDeclare(CmdDrawIndirect);
CallDeclare(CmdDrawIndexed);
CallDeclare(CmdDrawIndexedIndirect);
CallDeclare(CmdDispatch);
CallDeclare(CmdDispatchIndirect);
CallDeclare(CmdCopyBufferToImage);
CallDeclare(CmdCopyImageToBuffer);
CallDeclare(CmdCopyBuffer);
CallDeclare(CmdCopyImage);
CallDeclare(CmdBlitImage);
CallDeclare(CmdResolveImage);
CallDeclare(CmdUpdateBuffer);
CallDeclare(CmdFillBuffer);
CallDeclare(CmdPushConstants);
CallDeclare(CmdClearColorImage);
CallDeclare(CmdClearDepthStencilImage);
CallDeclare(CmdClearAttachments);
CallDeclare(CmdPipelineBarrier);
CallDeclare(CmdWriteTimestamp);
CallDeclare(CmdCopyQueryPoolResults);
CallDeclare(CmdBeginQuery);
CallDeclare(CmdEndQuery);
CallDeclare(CmdResetQueryPool);
CallDeclare(CmdSetEvent);
CallDeclare(CmdResetEvent);
CallDeclare(CmdWaitEvents);
CallDeclare(CreateFramebuffer);
CallDeclare(DestroyFramebuffer);
CallDeclare(CreateRenderPass);
CallDeclare(DestroyRenderPass);
CallDeclare(CmdBeginRenderPass);
CallDeclare(CmdNextSubpass);
CallDeclare(CmdExecuteCommands);
CallDeclare(CmdEndRenderPass);

#if defined(VK_USE_PLATFORM_WIN32_KHR)

CallDeclare(CreateWin32SurfaceKHR);
CallDeclare(GetPhysicalDeviceWin32PresentationSupportKHR);

#endif

CallDeclare(DestroySurfaceKHR);
CallDeclare(GetPhysicalDeviceSurfaceSupportKHR);
CallDeclare(GetPhysicalDeviceSurfaceCapabilitiesKHR);
CallDeclare(GetPhysicalDeviceSurfaceFormatsKHR);
CallDeclare(GetPhysicalDeviceSurfacePresentModesKHR);

CallDeclare(CreateSwapchainKHR);
CallDeclare(DestroySwapchainKHR);
CallDeclare(GetSwapchainImagesKHR);
CallDeclare(AcquireNextImageKHR);
CallDeclare(QueuePresentKHR);

CallDeclare(CreateDebugUtilsMessengerEXT);
CallDeclare(DestroyDebugUtilsMessengerEXT);

void InitialiseVulkanCalls(void *vulkanModule);
void InitialiseVulkanInstanceCalls(VkInstance instance);
void InitialiseVulkanDeviceCalls(VkDevice device);
