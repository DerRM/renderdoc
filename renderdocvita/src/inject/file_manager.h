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
    void reopen();

    template<typename T>
    uint32_t read(T& data);

    template<typename T>
    uint32_t write(T const& data);

    uint32_t read(void* data, uint32_t size);
    uint32_t write(const void* data, uint32_t size);
    void copy(File const& file, uint32_t offset = 0, uint32_t size = 0);
    void remove();
    void removeData(uint32_t file_pos, uint32_t size);
    void insertData(uint32_t file_pos, void* data, uint32_t size);
    void replaceData(uint32_t file_pos, uint32_t old_size, void* new_data, uint32_t new_size);
    void close();
    void flush();
    void skip(uint32_t size);
    void reset();
private:
    const char* m_path = NULL;
    SceUID m_fd = -1;
};

template<typename T>
uint32_t File::write(T const& data) {
    return write(&data, sizeof(T));
}

template<typename T>
uint32_t File::read(T& data) {
    return read(&data, sizeof(T));
}

#endif
