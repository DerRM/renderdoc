#ifndef RENDERDOC_VITA_FILE_MANAGER_H_
#define RENDERDOC_VITA_FILE_MANAGER_H_

#include <vitasdk.h>
#include <inttypes.h>
#include "../logging.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <kuio.h>

#ifdef __cplusplus
}
#endif

class File {
public:
    File() {}
    void open(const char* path);

    template<typename T>
    uint32_t write(T const& data);

    uint32_t write(const void* data, uint32_t size);
    void copy(File const& file, uint32_t offset = 0, uint32_t size = 0);
    void remove();
    void removeData(uint32_t file_pos, uint32_t size);
    void close();
    void flush();
private:
    const char* m_path = NULL;
    SceUID m_fd = -1;
};

template<typename T>
uint32_t File::write(T const& data) {
    if (m_fd < 0) {
        LOG("invalid file handle\n");
        return 0;
    }
    
    int bytesWritten = 0;
    if ((bytesWritten = kuIoWrite(m_fd, &data, sizeof(T))) == 0) {
        LOG("could not write to file: %s\n", m_path);
        return 0;
    }

    return (uint32_t)bytesWritten;
}

#endif
