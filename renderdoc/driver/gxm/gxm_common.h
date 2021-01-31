/******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2019-2020 Baldur Karlsson
 * Copyright (c) 2014 Crytek
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ******************************************************************************/

#pragma once

#include "common/common.h"
#include "core/core.h"
#include "maths/vec.h"

#include "gxm_core.h"
#include "gxm_enum.h"

#include "official/gxm.h"

#include "gxm_types.h"
#include "gxm_vk_loader.h"

#define IMPLEMENT_FUNCTION_SERIALISED(ret, func, ...) \
  ret func(__VA_ARGS__);                              \
  template <typename SerialiserType>                  \
  bool CONCAT(Serialise_, func)(SerialiserType & ser, ##__VA_ARGS__);

#define INSTANTIATE_FUNCTION_SERIALISED(ret, func, ...)                                   \
  template bool WrappedGXM::CONCAT(Serialise_, func(ReadSerialiser &ser, ##__VA_ARGS__)); \
  template bool WrappedGXM::CONCAT(Serialise_, func(WriteSerialiser &ser, ##__VA_ARGS__));

// enable this to cause every internal QueueSubmit to immediately call DeviceWaitIdle(), and to only
// submit one command buffer at once to narrow down the cause of device lost errors
#define SINGLE_FLUSH_VALIDATE OPTION_OFF

Topology MakePrimitiveTopology(SceGxmPrimitiveType Topo);

extern void *LoadVulkanLibrary();

typedef enum SceDisplayPixelFormat
{
  SCE_DISPLAY_PIXELFORMAT_A8B8G8R8 = 0x00000000U
} SceDisplayPixelFormat;

class WrappedGXM;

struct GPUBuffer
{
  enum CreateFlags
  {
    eGPUBufferReadback = 0x1,
    eGPUBufferVBuffer = 0x2,
    eGPUBufferIBuffer = 0x4,
    eGPUBufferSSBO = 0x8,
    eGPUBufferGPULocal = 0x10,
    eGPUBufferIndirectBuffer = 0x20,
    eGPUBufferAddressable = 0x40,
  };

  void Create(WrappedGXM *driver, VkDevice dev, VkDeviceSize size, uint32_t ringSize, uint32_t flags);
  void Destroy();

  void FillDescriptor(VkDescriptorBufferInfo &desc);

  size_t GetRingCount() { return size_t(ringCount); }
  void *Map(VkDeviceSize &bindoffset, VkDeviceSize usedsize = 0);
  void *Map(uint32_t *bindoffset = NULL, VkDeviceSize usedsize = 0);
  void Unmap();

  VkDeviceSize sz = 0;
  VkBuffer buf = VK_NULL_HANDLE;
  VkDeviceMemory mem = VK_NULL_HANDLE;

  // uniform buffer alignment requirement
  VkDeviceSize align = 0;

  // for handling ring allocations
  VkDeviceSize totalsize = 0;
  VkDeviceSize curoffset = 0;
  VkDeviceSize mapoffset = 0;

  uint32_t ringCount = 0;

  WrappedGXM *m_pDriver = NULL;
  VkDevice device = VK_NULL_HANDLE;
  uint32_t createFlags = 0;
};

ResourceFormat MakeAttributeFormat(SceGxmAttributeFormat fmt, uint8_t componentCount);

#define IsReplayingAndReading() (ser.IsReading() && IsReplayMode(m_State))

#define VULKAN_MESH_VIEW_SAMPLES VK_SAMPLE_COUNT_4_BIT

DECLARE_REFLECTION_ENUM(SceGxmIndexFormat);
DECLARE_REFLECTION_ENUM(SceGxmPrimitiveType);
DECLARE_REFLECTION_ENUM(SceGxmDepthFunc);
DECLARE_REFLECTION_ENUM(SceGxmStencilFunc);
DECLARE_REFLECTION_ENUM(SceGxmStencilOp);
DECLARE_REFLECTION_ENUM(SceGxmDepthWriteMode);
DECLARE_REFLECTION_ENUM(SceDisplayPixelFormat);
DECLARE_REFLECTION_ENUM(SceGxmMemoryAttribFlags);
DECLARE_REFLECTION_ENUM(SceGxmAttributeFormat);
DECLARE_REFLECTION_ENUM(GXMBufferType);
