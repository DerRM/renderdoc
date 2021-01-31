#include "gxm_common.h"

Topology MakePrimitiveTopology(SceGxmPrimitiveType Topo)
{
  switch(Topo)
  {
    default: return Topology::Unknown;
    case SCE_GXM_PRIMITIVE_POINTS: return Topology::PointList;
    case SCE_GXM_PRIMITIVE_LINES: return Topology::LineList;
    case SCE_GXM_PRIMITIVE_TRIANGLE_STRIP: return Topology::TriangleStrip;
    case SCE_GXM_PRIMITIVE_TRIANGLE_FAN: return Topology::TriangleFan;
    case SCE_GXM_PRIMITIVE_TRIANGLES: return Topology::TriangleList;
  }
}
