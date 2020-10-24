#include "resource_manager.h"

ResourceManager::ResourceManager() 
{
}

void ResourceManager::init(File file) {
    m_file = file;
    m_file.open("ux0:/data/renderdoc/resources.bin");
}

void ResourceManager::find(GXMType type, uint32_t key, Resource* resource) {
    m_file.reset();

    while (true) {
        uint32_t bytesread = m_file.read(*resource);

        if (bytesread == 0) {
            break;
        }

        if (resource->id == key) {
            break;
        }

        m_file.skip(resource->size);
    }

    if (resource->id == key) {
        switch(type) {
            case GXMType::SceGxmVertexProgram: {
            VertexProgramResource* vertex_program = (VertexProgramResource*)resource;
            m_file.read(vertex_program->vertexProgram);
            m_file.read(vertex_program->programId);
            m_file.read(vertex_program->attributeCount);
            for (uint32_t attribute_index = 0; attribute_index < vertex_program->attributeCount; ++attribute_index) {
                m_file.read(vertex_program->attributes[attribute_index]);
            }
            m_file.read(vertex_program->streamCount);
            for (uint32_t stream_index = 0; stream_index < vertex_program->streamCount; ++stream_index) {
                m_file.read(vertex_program->streams[stream_index]);
            }
        } break;
        case GXMType::SceGxmFragmentProgram: {
            FragmentProgramResource* fragment_program = (FragmentProgramResource*)resource;
            m_file.read(fragment_program->fragmentProgram);
            m_file.read(fragment_program->programId);
            m_file.read(fragment_program->outputFormat);
            m_file.read(fragment_program->multisampleMode);
            m_file.read(fragment_program->vertexProgram);
            m_file.read(fragment_program->hasBlendInfo);
            if (fragment_program->hasBlendInfo) {
                m_file.read(fragment_program->blendInfo);
            }
        } break;
        case GXMType::SceGxmProgram: {
            ProgramResource* program = (ProgramResource*)resource;
            m_file.read(program->programId);
            m_file.read(program->programLength);
            m_file.read((SceGxmProgram*)program->program, program->programLength);
        } break;
            default:
                break;
        }
    }

    m_file.close();
    m_file.reopen();
}

void ResourceManager::insert(Resource* resource) {
    //m_file.insertData()
}

void ResourceManager::add(Resource* resource) {

    m_file.write(resource->type);
    m_file.write(resource->id);
    m_file.write(resource->size);

    switch(resource->type) {
        case GXMType::SceGxmVertexProgram: {
            VertexProgramResource* vertex_program = (VertexProgramResource*)resource;
            m_file.write(vertex_program->vertexProgram);
            m_file.write(vertex_program->programId);
            m_file.write(vertex_program->attributeCount);
            for (uint32_t attribute_index = 0; attribute_index < vertex_program->attributeCount; ++attribute_index) {
                m_file.write(vertex_program->attributes[attribute_index]);
            }
            m_file.write(vertex_program->streamCount);
            for (uint32_t stream_index = 0; stream_index < vertex_program->streamCount; ++stream_index) {
                m_file.write(vertex_program->streams[stream_index]);
            }
        } break;
        case GXMType::SceGxmFragmentProgram: {
            FragmentProgramResource* fragment_program = (FragmentProgramResource*)resource;
            m_file.write(fragment_program->fragmentProgram);
            m_file.write(fragment_program->programId);
            m_file.write(fragment_program->outputFormat);
            m_file.write(fragment_program->multisampleMode);
            m_file.write(fragment_program->vertexProgram);
            m_file.write(fragment_program->hasBlendInfo);
            if (fragment_program->hasBlendInfo) {
                m_file.write(fragment_program->blendInfo);
            }
        } break;
        case GXMType::SceGxmProgram: {
            ProgramResource* program = (ProgramResource*)resource;
            m_file.write(program->programId);
            m_file.write(program->programLength);
            m_file.write(program->program, program->programLength);
        } break;
        default:
            break;
    }

    m_file.close();
    m_file.reopen();
}

void ResourceManager::remove(Resource* resource) {
    if (!contains(resource->id)) {
        return;
    }

    uint32_t pos = getFilePos(resource);
    m_file.removeData(pos, resource->size);
}

bool ResourceManager::contains(uint32_t key) {
    /*Resource res = find(key);
    if (res.id == key) {
        return true;
    }*/
    return false;
}

uint32_t ResourceManager::getFilePos(Resource* resource) {
    return 0;
}