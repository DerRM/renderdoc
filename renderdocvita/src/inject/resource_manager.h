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
, id(0)
, size(0)
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
    SceGxmShaderPatcherId programId;
    uint32_t programLength;
    const SceGxmProgram* program;
};

inline ProgramResource::ProgramResource()
: Resource(GXMType::SceGxmProgram)
{
}

struct VertexProgramResource : Resource {
    VertexProgramResource();
    SceGxmVertexProgram* vertexProgram;
    SceGxmShaderPatcherId programId;
    SceGxmVertexAttribute attributes[SCE_GXM_MAX_VERTEX_ATTRIBUTES];
    uint32_t attributeCount;
    SceGxmVertexStream streams[SCE_GXM_MAX_VERTEX_STREAMS];
    uint32_t streamCount;
};

inline VertexProgramResource::VertexProgramResource()
: Resource(GXMType::SceGxmVertexProgram)
, vertexProgram(NULL)
, programId(0)
, attributeCount(0)
, streamCount(0)
{
    memset(attributes, 0, sizeof(SceGxmVertexAttribute) * SCE_GXM_MAX_VERTEX_ATTRIBUTES);
    memset(streams, 0, sizeof(SceGxmVertexStream) * SCE_GXM_MAX_VERTEX_STREAMS);
}

struct FragmentProgramResource : Resource {
    FragmentProgramResource();
    SceGxmFragmentProgram* fragmentProgram;
    SceGxmShaderPatcherId programId;
    SceGxmOutputRegisterFormat outputFormat;
    SceGxmMultisampleMode multisampleMode;
    uint32_t hasBlendInfo;
    SceGxmBlendInfo blendInfo;
    const SceGxmProgram* vertexProgram;
};

inline FragmentProgramResource::FragmentProgramResource()
: Resource(GXMType::SceGxmFragmentProgram)
, hasBlendInfo(0)
{
}

#define MAX_KEY_COUNT 1024

class ResourceManager {
public:
    ResourceManager();
    void init(File file);
    void find(GXMType type, uint32_t key, Resource* resource);
    void insert(Resource* resource);
    void add(Resource* resource);
    void remove(Resource* resource);
    bool contains(uint32_t key);
    void deinit();
private:
    File m_file;
    uint32_t getFilePos(Resource* resource);
    uint32_t m_cached_keys[MAX_KEY_COUNT] = { 0 };
    uint32_t m_key_current_index = 0;
};

#endif
