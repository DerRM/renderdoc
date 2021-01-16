#include "gxm_replay.h"

GXMReplay::OutputWindow::OutputWindow()
    : m_WindowSystem(WindowingSystem::Unknown), width(0), height(0)
{
  WINDOW_HANDLE_INIT;

  hasDepth = false;

  failures = recreatePause = 0;

  surface = VK_NULL_HANDLE;

  swap = VK_NULL_HANDLE;
  for(size_t i = 0; i < ARRAY_COUNT(colimg); i++)
    colimg[i] = VK_NULL_HANDLE;

  bb = VK_NULL_HANDLE;
  bbmem = VK_NULL_HANDLE;
  bbview = VK_NULL_HANDLE;

  resolveimg = VK_NULL_HANDLE;
  resolvemem = VK_NULL_HANDLE;

  dsimg = VK_NULL_HANDLE;
  dsmem = VK_NULL_HANDLE;
  dsview = VK_NULL_HANDLE;

  fb = VK_NULL_HANDLE;
  fbdepth = VK_NULL_HANDLE;
  rp = VK_NULL_HANDLE;
  rpdepth = VK_NULL_HANDLE;

  numImgs = 0;
  curidx = 0;

  VkImageMemoryBarrier t = {
      VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      NULL,
      0,
      0,
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_QUEUE_FAMILY_IGNORED,
      VK_QUEUE_FAMILY_IGNORED,
      VK_NULL_HANDLE,
      {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
  };
  for(size_t i = 0; i < ARRAY_COUNT(colBarrier); i++)
    colBarrier[i] = t;

  bbBarrier = t;

  t.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  depthBarrier = t;
  depthBarrier.srcAccessMask = depthBarrier.dstAccessMask =
      VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
}

static bool IsSRGBFormat(VkFormat f)
{
  switch(f)
  {
    case VK_FORMAT_R8_SRGB:
    case VK_FORMAT_R8G8_SRGB:
    case VK_FORMAT_R8G8B8_SRGB:
    case VK_FORMAT_B8G8R8_SRGB:
    case VK_FORMAT_R8G8B8A8_SRGB:
    case VK_FORMAT_B8G8R8A8_SRGB:
    case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
    case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
    case VK_FORMAT_BC2_SRGB_BLOCK:
    case VK_FORMAT_BC3_SRGB_BLOCK:
    case VK_FORMAT_BC7_SRGB_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
    case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
    case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
    case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
    case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
    case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
    case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:
    case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:
    case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG:
    case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG: return true;
    default: break;
  }

  return false;
}

void GXMReplay::OutputWindow::Create(WrappedGXM* driver, bool depth)
{
  hasDepth = depth;

  VkSwapchainKHR old = swap;
  swap = VK_NULL_HANDLE;

  VkSurfaceKHR oldsurf = surface;
  surface = VK_NULL_HANDLE;

  Destroy(driver);

  surface = oldsurf;

  fresh = true;

  if (surface == VK_NULL_HANDLE && m_WindowSystem != WindowingSystem::Headless)
  {
    CreateSurface(driver);
  }

  // sensible defaults
  VkFormat imformat = VK_FORMAT_B8G8R8A8_SRGB;
  VkPresentModeKHR presentmode = VK_PRESENT_MODE_FIFO_KHR;
  VkColorSpaceKHR imcolspace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

  VkResult vkr = VK_SUCCESS;

  if(m_WindowSystem != WindowingSystem::Headless)
  {
    VkSurfaceCapabilitiesKHR capabilities;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(driver->m_vulkanState.m_Gpu, surface,
                                                           &capabilities);

    RDCASSERT(capabilities.supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    // AMD didn't report this capability for a while. If the assert fires for you, update
    // your drivers!
    RDCASSERT(capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    // check format and present mode from driver
    {
      uint32_t numFormats = 0;

      vkr = vkGetPhysicalDeviceSurfaceFormatsKHR(driver->m_vulkanState.m_Gpu, surface, &numFormats, NULL);
      RDCASSERTEQUAL(vkr, VK_SUCCESS);

      if(numFormats > 0)
      {
        VkSurfaceFormatKHR *formats = new VkSurfaceFormatKHR[numFormats];

        vkr = vkGetPhysicalDeviceSurfaceFormatsKHR(driver->m_vulkanState.m_Gpu, surface, &numFormats, formats);
        RDCASSERTEQUAL(vkr, VK_SUCCESS);

        if(numFormats == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
        {
          // 1 entry with undefined means no preference, just use our default
          imformat = VK_FORMAT_B8G8R8A8_SRGB;
          imcolspace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        }
        else
        {
          // try and find a format with SRGB correction
          imformat = VK_FORMAT_UNDEFINED;
          imcolspace = formats[0].colorSpace;

          for(uint32_t i = 0; i < numFormats; i++)
          {
            if(IsSRGBFormat(formats[i].format))
            {
              imformat = formats[i].format;
              imcolspace = formats[i].colorSpace;
              RDCASSERT(imcolspace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
              break;
            }
          }

          if(imformat == VK_FORMAT_UNDEFINED)
          {
            RDCWARN("Couldn't find SRGB correcting output swapchain format");
            imformat = formats[0].format;
          }
        }

        SAFE_DELETE_ARRAY(formats);
      }

      uint32_t numModes = 0;

      vkr = vkGetPhysicalDeviceSurfacePresentModesKHR(driver->m_vulkanState.m_Gpu, surface, &numModes, NULL);
      RDCASSERTEQUAL(vkr, VK_SUCCESS);

      if(numModes > 0)
      {
        VkPresentModeKHR *modes = new VkPresentModeKHR[numModes];

        vkr = vkGetPhysicalDeviceSurfacePresentModesKHR(driver->m_vulkanState.m_Gpu, surface, &numModes, modes);
        RDCASSERTEQUAL(vkr, VK_SUCCESS);

        // If mailbox mode is available, use it, as is the lowest-latency non-
        // tearing mode.  If not, try IMMEDIATE which will usually be available,
        // and is fastest (though it tears).  If not, fall back to FIFO which is
        // always available.
        for(size_t i = 0; i < numModes; i++)
        {
          if(modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
          {
            presentmode = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
          }

          if(modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
            presentmode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        }

        SAFE_DELETE_ARRAY(modes);
      }
    }

    VkBool32 supported = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(driver->m_vulkanState.m_Gpu, driver->m_vulkanState.m_QueueFamilyIndex,
                                                      surface, &supported);

    // can't really recover from this anyway
    RDCASSERT(supported);

    VkSwapchainCreateInfoKHR swapInfo = {
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        NULL,
        0,
        surface,
        2,
        imformat,
        imcolspace,
        {width, height},
        1,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        NULL,
        VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        presentmode,
        true,
        old,
    };

    vkr = vkCreateSwapchainKHR(driver->m_vulkanState.m_Device, &swapInfo, NULL, &swap);
    RDCASSERTEQUAL(vkr, VK_SUCCESS);

    if(old != VK_NULL_HANDLE)
    {
      vkDestroySwapchainKHR(driver->m_vulkanState.m_Device, old, NULL);
    }

    if(swap == VK_NULL_HANDLE)
    {
      RDCERR("Failed to create swapchain. %d consecutive failures!", failures);
      failures++;

      // do some sort of backoff.

      // the first time, try to recreate again next frame
      if(failures == 1)
        recreatePause = 0;
      // the next few times, wait 200 'frames' between attempts
      else if(failures < 10)
        recreatePause = 100;
      // otherwise, only reattempt very infrequently. A resize will
      // always retrigger a recreate, so ew probably don't want to
      // try again
      else
        recreatePause = 1000;

      return;
    }

    failures = 0;

    vkr = vkGetSwapchainImagesKHR(driver->m_vulkanState.m_Device, swap, &numImgs, NULL);
    RDCASSERTEQUAL(vkr, VK_SUCCESS);

    RDCASSERT(numImgs <= 8, numImgs);

    VkImage *imgs = new VkImage[numImgs];
    vkr = vkGetSwapchainImagesKHR(driver->m_vulkanState.m_Device, swap, &numImgs, imgs);
    RDCASSERTEQUAL(vkr, VK_SUCCESS);

    for(size_t i = 0; i < numImgs; i++)
    {
      colimg[i] = imgs[i];
      colBarrier[i].image = colimg[i];
      colBarrier[i].oldLayout = colBarrier[i].newLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    }

    delete[] imgs;
  }

  curidx = 0;

  // for our 'fake' backbuffer, create in RGBA8
  imformat = VK_FORMAT_R8G8B8A8_SRGB;

  if(depth)
  {
    VkImageCreateInfo imInfo = {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        NULL,
        0,
        VK_IMAGE_TYPE_2D,
        VK_FORMAT_D32_SFLOAT,
        {width, height, 1},
        1,
        1,
        VULKAN_MESH_VIEW_SAMPLES,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        NULL,
        VK_IMAGE_LAYOUT_UNDEFINED,
    };

    vkr = vkCreateImage(driver->m_vulkanState.m_Device, &imInfo, NULL, &dsimg);
    RDCASSERTEQUAL(vkr, VK_SUCCESS);

    VkMemoryRequirements mrq = {0};

    vkGetImageMemoryRequirements(driver->m_vulkanState.m_Device, dsimg, &mrq);

    VkMemoryAllocateInfo allocInfo = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        NULL,
        mrq.size,
        driver->GetGPULocalMemoryIndex(mrq.memoryTypeBits),
    };

    vkr = vkAllocateMemory(driver->m_vulkanState.m_Device, &allocInfo, NULL, &dsmem);
    RDCASSERTEQUAL(vkr, VK_SUCCESS);


    vkr = vkBindImageMemory(driver->m_vulkanState.m_Device, dsimg, dsmem, 0);
    RDCASSERTEQUAL(vkr, VK_SUCCESS);

    depthBarrier.image = dsimg;
    depthBarrier.oldLayout = depthBarrier.newLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImageViewCreateInfo info = {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        NULL,
        0,
        dsimg,
        VK_IMAGE_VIEW_TYPE_2D,
        VK_FORMAT_D32_SFLOAT,
        {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
         VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
        {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1},
    };

    vkr = vkCreateImageView(driver->m_vulkanState.m_Device, &info, NULL, &dsview);
    RDCASSERTEQUAL(vkr, VK_SUCCESS);

    // create resolve target, since it must precisely match the pre-resolve format, it doesn't allow
    // any format conversion.
    imInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imInfo.format = imformat;
    imInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    vkr = vkCreateImage(driver->m_vulkanState.m_Device, &imInfo, NULL, &resolveimg);
    RDCASSERTEQUAL(vkr, VK_SUCCESS);

    vkGetImageMemoryRequirements(driver->m_vulkanState.m_Device, resolveimg, &mrq);

    allocInfo.allocationSize = mrq.size;
    allocInfo.memoryTypeIndex = driver->GetGPULocalMemoryIndex(mrq.memoryTypeBits);

    vkr = vkAllocateMemory(driver->m_vulkanState.m_Device, &allocInfo, NULL, &resolvemem);
    RDCASSERTEQUAL(vkr, VK_SUCCESS);

    vkr = vkBindImageMemory(driver->m_vulkanState.m_Device, resolveimg, resolvemem, 0);
    RDCASSERTEQUAL(vkr, VK_SUCCESS);
  }

  {
    VkAttachmentDescription attDesc[] = {
        {0, imformat, depth ? VULKAN_MESH_VIEW_SAMPLES : VK_SAMPLE_COUNT_1_BIT,
         VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
         VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {0, VK_FORMAT_D32_SFLOAT, depth ? VULKAN_MESH_VIEW_SAMPLES : VK_SAMPLE_COUNT_1_BIT,
         VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
         VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
         VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL}};

    VkAttachmentReference attRef = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkAttachmentReference dsRef = {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    VkSubpassDescription sub = {
        0,    VK_PIPELINE_BIND_POINT_GRAPHICS,
        0,    NULL,       // inputs
        1,    &attRef,    // color
        NULL,             // resolve
        NULL,             // depth-stencil
        0,    NULL,       // preserve
    };

    VkRenderPassCreateInfo rpinfo = {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        NULL,
        0,
        1,
        attDesc,
        1,
        &sub,
        0,
        NULL,    // dependencies
    };

    vkr = vkCreateRenderPass(driver->m_vulkanState.m_Device, &rpinfo, NULL, &rp);
    RDCASSERTEQUAL(vkr, VK_SUCCESS);

    if(dsimg != VK_NULL_HANDLE)
    {
      sub.pDepthStencilAttachment = &dsRef;

      rpinfo.attachmentCount = 2;

      vkr = vkCreateRenderPass(driver->m_vulkanState.m_Device, &rpinfo, NULL, &rpdepth);
      RDCASSERTEQUAL(vkr, VK_SUCCESS);
    }
  }

  {
    VkImageCreateInfo imInfo = {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        NULL,
        0,
        VK_IMAGE_TYPE_2D,
        imformat,
        {width, height, 1},
        1,
        1,
        depth ? VULKAN_MESH_VIEW_SAMPLES : VK_SAMPLE_COUNT_1_BIT,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        NULL,
        VK_IMAGE_LAYOUT_UNDEFINED,
    };

    vkr = vkCreateImage(driver->m_vulkanState.m_Device, &imInfo, NULL, &bb);
    RDCASSERTEQUAL(vkr, VK_SUCCESS);

    VkMemoryRequirements mrq = {0};

    vkGetImageMemoryRequirements(driver->m_vulkanState.m_Device, bb, &mrq);

    VkMemoryAllocateInfo allocInfo = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        NULL,
        mrq.size,
        driver->GetGPULocalMemoryIndex(mrq.memoryTypeBits),
    };

    vkr = vkAllocateMemory(driver->m_vulkanState.m_Device, &allocInfo, NULL, &bbmem);
    RDCASSERTEQUAL(vkr, VK_SUCCESS);

    vkr = vkBindImageMemory(driver->m_vulkanState.m_Device, bb, bbmem, 0);
    RDCASSERTEQUAL(vkr, VK_SUCCESS);

    bbBarrier.image = bb;
    bbBarrier.oldLayout = bbBarrier.newLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  }

  {
    VkImageViewCreateInfo info = {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        NULL,
        0,
        bb,
        VK_IMAGE_VIEW_TYPE_2D,
        imformat,
        {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
         VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
        {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
    };

    vkr = vkCreateImageView(driver->m_vulkanState.m_Device, &info, NULL, &bbview);
    RDCASSERTEQUAL(vkr, VK_SUCCESS);

    {
      VkFramebufferCreateInfo fbinfo = {
          VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
          NULL,
          0,
          rp,
          1,
          &bbview,
          (uint32_t)width,
          (uint32_t)height,
          1,
      };

      vkr = vkCreateFramebuffer(driver->m_vulkanState.m_Device, &fbinfo, NULL, &fb);
      RDCASSERTEQUAL(vkr, VK_SUCCESS);
    }

    if(dsimg != VK_NULL_HANDLE)
    {
      VkImageView views[] = {bbview, dsview};
      VkFramebufferCreateInfo fbinfo = {
          VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
          NULL,
          0,
          rpdepth,
          2,
          views,
          (uint32_t)width,
          (uint32_t)height,
          1,
      };

      vkr = vkCreateFramebuffer(driver->m_vulkanState.m_Device, &fbinfo, NULL, &fbdepth);
      RDCASSERTEQUAL(vkr, VK_SUCCESS);
    }
  }
}

void GXMReplay::OutputWindow::Destroy(WrappedGXM* driver)
{
  vkDeviceWaitIdle(driver->m_vulkanState.m_Device);

  if(bb != VK_NULL_HANDLE)
  {
    vkDestroyRenderPass(driver->m_vulkanState.m_Device, rp, NULL);
    rp = VK_NULL_HANDLE;

    vkDestroyImage(driver->m_vulkanState.m_Device, bb, NULL);

    vkDestroyImageView(driver->m_vulkanState.m_Device, bbview, NULL);
    vkFreeMemory(driver->m_vulkanState.m_Device, bbmem, NULL);
    vkDestroyFramebuffer(driver->m_vulkanState.m_Device, fb, NULL);

    bb = VK_NULL_HANDLE;
    bbview = VK_NULL_HANDLE;
    bbmem = VK_NULL_HANDLE;
    fb = VK_NULL_HANDLE;
  }

  // not owned - freed with the swapchain
  for(size_t i = 0; i < ARRAY_COUNT(colimg); i++)
  {
    colimg[i] = VK_NULL_HANDLE;
  }

  if(dsimg != VK_NULL_HANDLE)
  {
    vkDestroyRenderPass(driver->m_vulkanState.m_Device, rpdepth, NULL);
    rpdepth = VK_NULL_HANDLE;

    vkDestroyImage(driver->m_vulkanState.m_Device, dsimg, NULL);

    vkDestroyImageView(driver->m_vulkanState.m_Device, dsview, NULL);
    vkFreeMemory(driver->m_vulkanState.m_Device, dsmem, NULL);
    vkDestroyFramebuffer(driver->m_vulkanState.m_Device, fbdepth, NULL);

    vkDestroyImage(driver->m_vulkanState.m_Device, resolveimg, NULL);
    vkFreeMemory(driver->m_vulkanState.m_Device, resolvemem, NULL);

    resolveimg = VK_NULL_HANDLE;
    resolvemem = VK_NULL_HANDLE;
    dsview = VK_NULL_HANDLE;
    dsimg = VK_NULL_HANDLE;
    dsmem = VK_NULL_HANDLE;
    fbdepth = VK_NULL_HANDLE;
    rpdepth = VK_NULL_HANDLE;
  }

  if(swap != VK_NULL_HANDLE)
  {
    vkDestroySwapchainKHR(driver->m_vulkanState.m_Device, swap, NULL);
  }

  if(surface != VK_NULL_HANDLE)
  {
    vkDestroySurfaceKHR(driver->m_vulkanState.m_Instance, surface, NULL);
    surface = VK_NULL_HANDLE;
  }
}

static void DoPipelineBarrier(VkCommandBuffer cmd, size_t count, const VkImageMemoryBarrier *barriers)
{
  RDCASSERT(cmd != VK_NULL_HANDLE);
  vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                   VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                                   NULL,                          // global memory barriers
                                   0, NULL,                       // buffer memory barriers
                                   (uint32_t)count, barriers);    // image memory barriers
}

void GXMReplay::BindOutputWindow(uint64_t id, bool depth)
{
  m_ActiveWinID = id;
  //m_BindDepth = depth;

  auto it = m_OutputWindows.find(id);
  if(id == 0 || it == m_OutputWindows.end())
    return;

  OutputWindow &outw = it->second;

  // if the swapchain failed to create, do nothing. We will try to recreate it
  // again in CheckResizeOutputWindow (once per render 'frame')
  if(outw.m_WindowSystem != WindowingSystem::Headless && outw.swap == VK_NULL_HANDLE)
    return;

  m_DebugWidth = (int32_t)outw.width;
  m_DebugHeight = (int32_t)outw.height;

  VkDevice dev = m_pDriver->m_vulkanState.m_Device;
  VkCommandBuffer cmd = m_pDriver->GetNextCmd();
  VkResult vkr = VK_SUCCESS;

  // if we have a swapchain, acquire the next image.
  if(outw.swap != VK_NULL_HANDLE)
  {
    // semaphore is short lived, so not wrapped, if it's cached (ideally)
    // then it should be wrapped
    VkSemaphore sem;
    VkPipelineStageFlags stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    VkSemaphoreCreateInfo semInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, NULL, 0};

    vkr = vkCreateSemaphore(dev, &semInfo, NULL, &sem);
    RDCASSERTEQUAL(vkr, VK_SUCCESS);

    vkr = vkAcquireNextImageKHR(dev, outw.swap, UINT64_MAX, sem, VK_NULL_HANDLE,
                                  &outw.curidx);

    if(vkr == VK_ERROR_OUT_OF_DATE_KHR)
    {
      // force a swapchain recreate.
      outw.width = 0;
      outw.height = 0;

      CheckResizeOutputWindow(id);

      // then try again to acquire.
      vkr = vkAcquireNextImageKHR(dev, outw.swap, UINT64_MAX, sem, VK_NULL_HANDLE,
                                    &outw.curidx);
    }

    if(vkr == VK_SUBOPTIMAL_KHR)
      vkr = VK_SUCCESS;

    RDCASSERTEQUAL(vkr, VK_SUCCESS);

    VkSubmitInfo submitInfo = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        NULL,
        1,
        &sem,
        &stage,
        0,
        NULL,    // cmd buffers
        0,
        NULL,    // signal semaphores
    };

    vkr = vkQueueSubmit(m_pDriver->m_vulkanState.m_Queue, 1, &submitInfo, VK_NULL_HANDLE);
    RDCASSERTEQUAL(vkr, VK_SUCCESS);

    vkQueueWaitIdle(m_pDriver->m_vulkanState.m_Queue);

    vkDestroySemaphore(dev, sem, NULL);
  }

  VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, NULL,
                                        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};

  vkr = vkBeginCommandBuffer(cmd, &beginInfo);
  RDCASSERTEQUAL(vkr, VK_SUCCESS);

  outw.depthBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  // first time rendering to the backbuffer, clear it, since our typical render pass
  // is set to LOAD_OP_LOAD
  if(outw.fresh)
  {
    outw.bbBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    outw.bbBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    DoPipelineBarrier(cmd, 1, &outw.bbBarrier);
    float black[] = {0.0f, 0.0f, 0.0f, 0.0f};
    vkCmdClearColorImage(cmd, outw.bb, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           (VkClearColorValue *)black, 1, &outw.bbBarrier.subresourceRange);

    outw.bbBarrier.oldLayout = outw.bbBarrier.newLayout;
    outw.bbBarrier.srcAccessMask = outw.bbBarrier.dstAccessMask;

    outw.fresh = false;
  }

  outw.bbBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  outw.bbBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  outw.colBarrier[outw.curidx].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  outw.colBarrier[outw.curidx].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

  DoPipelineBarrier(cmd, 1, &outw.bbBarrier);
  if(outw.colimg[0] != VK_NULL_HANDLE)
    DoPipelineBarrier(cmd, 1, &outw.colBarrier[outw.curidx]);
  if(outw.dsimg != VK_NULL_HANDLE)
    DoPipelineBarrier(cmd, 1, &outw.depthBarrier);

  outw.depthBarrier.oldLayout = outw.depthBarrier.newLayout;
  outw.bbBarrier.oldLayout = outw.bbBarrier.newLayout;
  outw.bbBarrier.srcAccessMask = outw.bbBarrier.dstAccessMask;
  outw.colBarrier[outw.curidx].oldLayout = outw.colBarrier[outw.curidx].newLayout;
  outw.colBarrier[outw.curidx].srcAccessMask = outw.colBarrier[outw.curidx].dstAccessMask;

  vkEndCommandBuffer(cmd);

#if ENABLED(SINGLE_FLUSH_VALIDATE)
  m_pDriver->SubmitCmds();
#endif
}

void GXMReplay::FlipOutputWindow(uint64_t id)
{
  auto it = m_OutputWindows.find(id);
  if(id == 0 || it == m_OutputWindows.end())
  return;

  OutputWindow &outw = it->second;

  // if the swapchain failed to create, do nothing. We will try to recreate it
  // again in CheckResizeOutputWindow (once per render 'frame')
  if(outw.swap == VK_NULL_HANDLE)
    return;

  VkCommandBuffer cmd = m_pDriver->GetNextCmd();

  VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, NULL,
                                        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};

  VkResult vkr = vkBeginCommandBuffer(cmd, &beginInfo);
  RDCASSERTEQUAL(vkr, VK_SUCCESS);

  // ensure rendering has completed before copying
  outw.bbBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  outw.bbBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
  outw.bbBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
  DoPipelineBarrier(cmd, 1, &outw.bbBarrier);
  DoPipelineBarrier(cmd, 1, &outw.colBarrier[outw.curidx]);
  outw.bbBarrier.oldLayout = outw.bbBarrier.newLayout;
  outw.bbBarrier.srcAccessMask = 0;
  outw.bbBarrier.dstAccessMask = 0;

  VkImageBlit blit = {
      {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
      {
          {0, 0, 0},
          {(int32_t)outw.width, (int32_t)outw.height, 1},
      },
      {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
      {
          {0, 0, 0},
          {(int32_t)outw.width, (int32_t)outw.height, 1},
      },
  };

  VkImage blitSource = outw.bb;

  if(outw.dsimg != VK_NULL_HANDLE)
  {
    VkImageResolve resolve = {
        {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1}, {0, 0, 0},
        {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1}, {0, 0, 0},
        {outw.width, outw.height, 1},
    };

    VkImageMemoryBarrier resolveBarrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                                           NULL,
                                           VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT,
                                           VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT,
                                           VK_IMAGE_LAYOUT_UNDEFINED,
                                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                           VK_QUEUE_FAMILY_IGNORED,
                                           VK_QUEUE_FAMILY_IGNORED,
                                           outw.resolveimg,
                                           {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};

    // discard previous contents of resolve buffer and finish any work with it.
    DoPipelineBarrier(cmd, 1, &resolveBarrier);

    // resolve from the backbuffer to resolve buffer (identical format)
    vkCmdResolveImage(cmd, outw.bb, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                        outw.resolveimg, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &resolve);

    // wait for resolve to finish before we blit
    blitSource = outw.resolveimg;

    resolveBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    resolveBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    DoPipelineBarrier(cmd, 1, &resolveBarrier);
  }

  vkCmdBlitImage(cmd, blitSource, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   outw.colimg[outw.curidx], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit,
                   VK_FILTER_NEAREST);

  outw.bbBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
  outw.bbBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  outw.bbBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  outw.colBarrier[outw.curidx].newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  // make sure copy has completed before present
  outw.colBarrier[outw.curidx].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  outw.colBarrier[outw.curidx].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

  DoPipelineBarrier(cmd, 1, &outw.bbBarrier);
  DoPipelineBarrier(cmd, 1, &outw.colBarrier[outw.curidx]);

  outw.bbBarrier.oldLayout = outw.bbBarrier.newLayout;
  outw.bbBarrier.srcAccessMask = outw.bbBarrier.dstAccessMask;
  outw.colBarrier[outw.curidx].oldLayout = outw.colBarrier[outw.curidx].newLayout;

  outw.colBarrier[outw.curidx].srcAccessMask = 0;
  outw.colBarrier[outw.curidx].dstAccessMask = 0;

  vkEndCommandBuffer(cmd);

  // submit all the cmds we recorded
  m_pDriver->SubmitCmds();

  VkPresentInfoKHR presentInfo = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                                  NULL,
                                  0,
                                  NULL,    // wait semaphores
                                  1,
                                  &outw.swap,
                                  &outw.curidx,
                                  &vkr};

  VkResult retvkr = vkQueuePresentKHR(m_pDriver->m_vulkanState.m_Queue, &presentInfo);

  m_pDriver->FlushQ();

  if(retvkr == VK_ERROR_OUT_OF_DATE_KHR)
  {
    // this will check the current extent and use that if possible
    outw.Create(m_pDriver, outw.hasDepth);

    outw.outofdate = true;
  }
}

bool GXMReplay::CheckResizeOutputWindow(uint64_t id)
{
  if(id == 0 || m_OutputWindows.find(id) == m_OutputWindows.end())
    return false;

  OutputWindow &outw = m_OutputWindows[id];

  if(outw.m_WindowSystem == WindowingSystem::Unknown ||
     outw.m_WindowSystem == WindowingSystem::Headless)
    return false;

  int32_t w, h;
  GetOutputWindowDimensions(id, w, h);

  if((uint32_t)w != outw.width || (uint32_t)h != outw.height)
  {
    outw.width = w;
    outw.height = h;

    if(outw.width > 0 && outw.height > 0)
      outw.Create(m_pDriver, outw.hasDepth);

    return true;
  }

  if(outw.swap == VK_NULL_HANDLE && outw.width > 0 && outw.height > 0)
  {
    if(outw.recreatePause <= 0)
      outw.Create(m_pDriver, outw.hasDepth);
    else
      outw.recreatePause--;

    return true;
  }

  if(outw.outofdate)
  {
    outw.outofdate = false;
    return true;
  }

  return false;
}