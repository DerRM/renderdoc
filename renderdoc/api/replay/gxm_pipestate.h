#pragma once

#include "common_pipestate.h"

namespace GXMPipe
{
DOCUMENT(R"(Describes the configuration for a single vertex attribute.

.. note:: If old-style vertex attrib pointer setup was used for the vertex attributes then it will
  be decomposed into 1:1 attributes and buffers.
)");
struct VertexAttribute
{
  DOCUMENT("");
  VertexAttribute() = default;
  VertexAttribute(const VertexAttribute &) = default;
  VertexAttribute &operator=(const VertexAttribute &) = default;

  bool operator==(const VertexAttribute &o) const
  {
    return enabled == o.enabled && format == o.format &&
           !memcmp(&genericValue, &o.genericValue, sizeof(genericValue)) &&
           vertexBufferSlot == o.vertexBufferSlot && byteOffset == o.byteOffset;
  }
  bool operator<(const VertexAttribute &o) const
  {
    if(!(enabled == o.enabled))
      return enabled < o.enabled;
    if(!(format == o.format))
      return format < o.format;
    if(memcmp(&genericValue, &o.genericValue, sizeof(genericValue)) < 0)
      return true;
    if(!(vertexBufferSlot == o.vertexBufferSlot))
      return vertexBufferSlot < o.vertexBufferSlot;
    if(!(byteOffset == o.byteOffset))
      return byteOffset < o.byteOffset;
    return false;
  }
  DOCUMENT("``True`` if this vertex attribute is enabled.");
  bool enabled = false;
  DOCUMENT("The :class:`ResourceFormat` of the vertex attribute.");
  ResourceFormat format;

  DOCUMENT("A :class:`PixelValue` containing the generic value of a vertex attribute.");
  PixelValue genericValue;

  DOCUMENT("The vertex buffer input slot where the data is sourced from.");
  uint32_t vertexBufferSlot = 0;
  DOCUMENT(R"(The byte offset from the start of the vertex data in the vertex buffer from
:data:`vertexBufferSlot`.
)");
  uint32_t byteOffset = 0;
};

DOCUMENT("Describes a single GXM vertex buffer.");
struct VertexBuffer
{
  DOCUMENT("");
  VertexBuffer() = default;
  VertexBuffer(const VertexBuffer &) = default;
  VertexBuffer &operator=(const VertexBuffer &) = default;

   bool operator==(const VertexBuffer &o) const
  {
    return resourceId == o.resourceId && byteStride == o.byteStride && byteOffset == o.byteOffset;
  }
  bool operator<(const VertexBuffer &o) const
  {
    if(!(resourceId == o.resourceId))
      return resourceId < o.resourceId;
    if(!(byteStride == o.byteStride))
      return byteStride < o.byteStride;
    if(!(byteOffset == o.byteOffset))
      return byteOffset < o.byteOffset;
    return false;
  }
  DOCUMENT("The :class:`ResourceId` of the buffer bound to this slot.");
  ResourceId resourceId;

  DOCUMENT("The byte stride between the start of one set of vertex data and the next.");
  uint32_t byteStride = 0;
  DOCUMENT("The byte offset from the start of the buffer to the beginning of the vertex data.");
  uint32_t byteOffset = 0;
};

DOCUMENT("Describes the setup for fixed-function vertex input fetch.");
struct VertexInput
{
  DOCUMENT("");
  VertexInput() = default;
  VertexInput(const VertexInput &) = default;
  VertexInput &operator=(const VertexInput &) = default;

  DOCUMENT("The :class:`ResourceId` of the vertex array object that's bound.");
  ResourceId vertexArrayObject;

  DOCUMENT("A list of :class:`GLVertexAttribute` with the vertex attributes.");
  rdcarray<VertexAttribute> attributes;

  DOCUMENT("A list of :class:`GLVertexBuffer` with the vertex buffers.");
  rdcarray<VertexBuffer> vertexBuffers;

  DOCUMENT("The :class:`ResourceId` of the index buffer.");
  ResourceId indexBuffer;
  DOCUMENT("``True`` if primitive restart is enabled for strip primitives.");
  bool primitiveRestart = false;
  DOCUMENT("The index value to use to indicate a strip restart.");
  uint32_t restartIndex = 0;

  DOCUMENT(R"(``True`` if the provoking vertex is the last one in the primitive.

``False`` if the provoking vertex is the first one.
)");
  bool provokingVertexLast = false;
};

DOCUMENT("The full current GXM pipeline state.")
struct State
{
  DOCUMENT("A :class:`GXMVertexInput` describing the vertex input stage.");
  VertexInput vertexInput;
};
}    // namespace GXMPipe