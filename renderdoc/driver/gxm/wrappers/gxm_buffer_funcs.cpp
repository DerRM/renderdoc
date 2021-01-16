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

#include <inttypes.h>

#include "../gxm_driver.h"
#include "common/common.h"

#pragma region Buffers

template <typename SerialiserType>
bool WrappedGXM::Serialise_sceGxmMapMemory(SerialiserType &ser, void *base, SceSize size,
                                           SceGxmMemoryAttribFlags attr)
{
  SERIALISE_ELEMENT_TYPED(uint32_t, base);
  SERIALISE_ELEMENT(size);
  SERIALISE_ELEMENT(attr);

  SERIALISE_CHECK_READ_ERRORS();

  return true;
}

template <typename SerialiserType>
bool WrappedGXM::Serialise_sceGxmUnmapMemory(SerialiserType &ser, void *base)
{
  SERIALISE_ELEMENT_TYPED(uint32_t, base);

  SERIALISE_CHECK_READ_ERRORS();

  return true;
}

template <typename SerialiserType>
bool WrappedGXM::Serialise_sceGxmReserveVertexDefaultUniformBuffer(SerialiserType &ser,
                                                                   SceGxmContext *context,
                                                                   void **uniformBuffer)
{
  SERIALISE_ELEMENT_TYPED(uint32_t, context);
  SERIALISE_ELEMENT_TYPED(uint32_t, uniformBuffer);

  RDCLOG("sceGxmReserveVertexDefaultUniformBuffer(context: 0x%x, uniformBuffer: 0x%x)", context,
         uniformBuffer);

  return true;
}

template <typename SerialiserType>
bool WrappedGXM::Serialise_sceGxmReserveFragmentDefaultUniformBuffer(SerialiserType &ser,
                                                                     SceGxmContext *context,
                                                                     void **uniformBuffer)
{
  SERIALISE_ELEMENT_TYPED(uint32_t, context);
  SERIALISE_ELEMENT_TYPED(uint32_t, uniformBuffer);

  RDCLOG("sceGxmReserveFragmentDefaultUniformBuffer(context: 0x%x, uniformBuffer: 0x%x)", context,
         uniformBuffer);

  return true;
}

template <typename SerialiserType>
bool WrappedGXM::Serialise_sceGxmSetUniformDataF(SerialiserType &ser, void *uniformBuffer,
                                                 const SceGxmProgramParameter *parameter,
                                                 unsigned int componentOffset,
                                                 unsigned int componentCount, const float *sourceData)
{
  SERIALISE_ELEMENT_TYPED(uint32_t, uniformBuffer);
  SERIALISE_ELEMENT_TYPED(uint32_t, parameter);
  SERIALISE_ELEMENT(componentOffset);
  SERIALISE_ELEMENT(componentCount);
  SERIALISE_ELEMENT_TYPED(uint32_t, sourceData);

  RDCLOG("sceGxmSetUniformDataF(uniformBuffer, 0x%x, parameter: 0x%x, componentOffset: %" PRIu32
         ", componentCount: %" PRIu32 ", sourceData: 0x%x)",
         uniformBuffer, parameter, componentOffset, componentCount, sourceData);

  return true;
}

template <typename SerialiserType>
bool WrappedGXM::Serialise_sceGxmSetVertexStream(SerialiserType &ser, SceGxmContext *context,
                                                 unsigned int streamIndex, const void *streamData)
{
  SERIALISE_ELEMENT_TYPED(uint32_t, context);
  SERIALISE_ELEMENT(streamIndex);
  SERIALISE_ELEMENT_TYPED(uint32_t, streamData);

  RDCLOG("sceGxmSetVertexStream(context: 0x%x, streamIndex: %" PRIu32 ", streamData: 0x%x)",
         context, streamIndex, streamData);

  return true;
}

INSTANTIATE_FUNCTION_SERIALISED(int, sceGxmMapMemory, void *base, SceSize size,
                                SceGxmMemoryAttribFlags attr);
INSTANTIATE_FUNCTION_SERIALISED(int, sceGxmUnmapMemory, void *base);
INSTANTIATE_FUNCTION_SERIALISED(int, sceGxmReserveVertexDefaultUniformBuffer,
                                SceGxmContext *context, void **uniformBuffer);
INSTANTIATE_FUNCTION_SERIALISED(int, sceGxmReserveFragmentDefaultUniformBuffer,
                                SceGxmContext *context, void **uniformBuffer);
INSTANTIATE_FUNCTION_SERIALISED(int, sceGxmSetUniformDataF, void *uniformBuffer,
                                const SceGxmProgramParameter *parameter, unsigned int componentOffset,
                                unsigned int componentCount, const float *sourceData);
INSTANTIATE_FUNCTION_SERIALISED(int, sceGxmSetVertexStream, SceGxmContext *context,
                                unsigned int streamIndex, const void *streamData);