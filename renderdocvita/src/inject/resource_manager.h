#ifndef RENDERDOC_VITA_RES_MANAGER_H_
#define RENDERDOC_VITA_RES_MANAGER_H_

#include "file_manager.h"
#include <driver/gxm/gxm_types.h>

struct Resource {
    GXMType type;
    uint32_t id;
    uint32_t size;

    Resource();
    Resource(GXMType type);
};

inline Resource::Resource()
: type(GXMType::SceGXMUnknown)
{
}

inline Resource::Resource(GXMType type)
: type(type)
{
}

struct TextureResource : Resource {
    TextureResource();
};

inline TextureResource::TextureResource() 
: Resource(GXMType::SceGxmTexture)
{
}

struct ShaderPatcherIdResource : Resource {
    ShaderPatcherIdResource();
};

inline ShaderPatcherIdResource::ShaderPatcherIdResource()
: Resource(GXMType::SceGxmShaderPatcherId)
{
}

struct ProgramResource : Resource {
    ProgramResource();
};

inline ProgramResource::ProgramResource()
: Resource(GXMType::SceGxmProgram)
{
}

struct VertexProgramResource : Resource {
    VertexProgramResource();
};

inline VertexProgramResource::VertexProgramResource()
: Resource(GXMType::SceGxmVertexProgram)
{
}

struct FragmentProgramResource : Resource {
    FragmentProgramResource();
};

inline FragmentProgramResource::FragmentProgramResource()
: Resource(GXMType::SceGxmFragmentProgram)
{
}

class ResourceManager {
public:
    ResourceManager();
    void init(File file);
    Resource find(uint32_t key);
    void insert(Resource* resource);
    void add(Resource* resource);
    void remove(Resource* resource);
    bool contains(uint32_t key);
private:
    File m_file;
    uint32_t getFilePos(Resource* resource);
};

#endif
