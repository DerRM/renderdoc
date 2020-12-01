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

#include "core/resource_manager.h"
#include "gxm_common.h"

enum NullInitialiser
{
  MakeNullResource
};

enum GXMNamespace
{
  eResUnknown = 0,
  eResTexture,
  eResFramebuffer,
  eResBuffer,
  eResShader,
  eResProgram,
};

struct GXMResource
{
  GXMResource()
  {
    Namespace = eResUnknown;
  }
  GXMResource(NullInitialiser)
  {
    Namespace = eResUnknown;
  }
  GXMResource(GXMNamespace n)
  {
    Namespace = n;
  }

  GXMNamespace Namespace;

  bool operator==(const GXMResource &o) const
  {
    return Namespace == o.Namespace;
  }

  bool operator!=(const GXMResource &o) const { return !(*this == o); }
  bool operator<(const GXMResource &o) const
  {
    //if(Namespace != o.Namespace)
      return Namespace < o.Namespace;
  }
};

inline GXMResource TextureRes()
{
  return GXMResource(eResTexture);
}
inline GXMResource FramebufferRes()
{
  return GXMResource(eResFramebuffer);
}
inline GXMResource BufferRes()
{
  return GXMResource(eResBuffer);
}
inline GXMResource ShaderRes()
{
  return GXMResource(eResShader);
}
inline GXMResource ProgramRes()
{
  return GXMResource(eResProgram);
}

struct GXMResourceRecord : public ResourceRecord
{
  static const NullInitialiser NullResource = MakeNullResource;
};
