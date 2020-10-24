#include "resource_manager.h"

ResourceManager::ResourceManager() 
{
}

void ResourceManager::init(File file) {
    m_file = file;
    m_file.open("ux0:/data/renderdoc/resources.bin");
}

Resource ResourceManager::find(uint32_t key) {
    Resource res;
    
    while (res.id != key) {
        uint32_t bytesread = m_file.read(res);

        if (bytesread == 0) {
            break;
        }

        m_file.skip(res.size);
    }

    m_file.reset();
    
    if (res.id != key) {
        return Resource();
    }

    return res;
}

void ResourceManager::insert(Resource* resource) {
    //m_file.insertData()
}

void ResourceManager::add(Resource* resource) {
    m_file.write(*resource);
}

void ResourceManager::remove(Resource* resource) {
    if (!contains(resource->id)) {
        return;
    }

    uint32_t pos = getFilePos(resource);
    m_file.removeData(pos, resource->size);
}

bool ResourceManager::contains(uint32_t key) {
    Resource res = find(key);
    if (res.id == key) {
        return true;
    }
    return false;
}

uint32_t ResourceManager::getFilePos(Resource* resource) {
    return 0;
}