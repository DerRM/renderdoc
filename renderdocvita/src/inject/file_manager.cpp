#include "file_manager.h"

#define CHUNK_SIZE 2048

void File::open(const char* path) {
    m_path = path;
    reopen();
}

void File::setPath(const char* path) {
    m_path = path;
}

void File::reopen() {
    m_fd = sceIoOpen(m_path, SCE_O_RDWR | SCE_O_APPEND | SCE_O_CREAT, 0777);
    if (m_fd < 0) {
        LOG("could not open file: %s, error: %" PRIi32 "\n", m_path, m_fd);
    }
}

uint32_t File::write(const void* data, uint32_t size) {
    if (m_fd < 0) {
        LOG("invalid file handle\n");
        return 0;
    }

    int bytesWritten = 0;
    if ((bytesWritten = sceIoWrite(m_fd, data, size)) == 0) {
        LOG("could not write to file: %s, size to write: %" PRIu32 "\n", m_path, size);
        return 0;
    }

    return (uint32_t)bytesWritten;
}

uint32_t File::read(void* data, uint32_t size) {
    if (m_fd < 0) {
        LOG("invalid file handle\n");
        return 0;
    }

    int bytesRead = 0;
    if ((bytesRead = sceIoRead(m_fd, data, size)) == 0) {
        //LOG("could not read file: %s, size: %" PRIu32 "\n", m_path, size);
        return 0;
    }

    return (uint32_t)bytesRead;
}

#define TMP_FILE "ux0:/data/renderdoc/tmp"

void File::removeData(uint32_t file_pos, uint32_t size) {
    sceIoRemove(TMP_FILE);

    close();
    if ((m_fd = sceIoOpen(m_path, SCE_O_RDONLY, 0777)) < 0) {
        LOG("could not open file for reading\n");
        return;
    }
    SceUID m_tmp_fd = -1;
    if ((m_tmp_fd = sceIoOpen(TMP_FILE, SCE_O_WRONLY | SCE_O_CREAT, 0777)) < 0) {
        LOG("could not open tmp file for reading\n");
        return;
    }

    SceOff fileSize = sceIoLseek(m_fd, 0, SCE_SEEK_END);
    LOG("file size: %" PRIi64 "\n", fileSize);
    uint32_t remaining_file_size = (uint32_t)fileSize;
    SceOff pos = sceIoLseek(m_fd, 0, SCE_SEEK_SET);
    LOG("file pos: %" PRIi64 "\n", pos);

    uint8_t chunk[CHUNK_SIZE] = { 0 };
    uint32_t offset = 0;
    uint32_t buffer_size = 0;
    while(offset < file_pos) {

        if (file_pos - offset > CHUNK_SIZE) 
            buffer_size = CHUNK_SIZE;
        else
            buffer_size = file_pos - offset;

        int bytes = sceIoRead(m_fd, chunk, buffer_size);
        LOG("bytes read: %" PRIi32 "\n", bytes);
        bytes = sceIoWrite(m_tmp_fd, chunk, buffer_size);
        LOG("bytes written: %" PRIi32 "\n", bytes);

        offset += buffer_size;
    }

    remaining_file_size -= file_pos;
    remaining_file_size -= size;

    LOG("remaining file size: %" PRIu32 "\n", remaining_file_size);

    LOG("file_pos: %" PRIu32 "\n", file_pos);
    LOG("size: %" PRIu32 "\n", size);

    sceIoLseek(m_fd, size, SCE_SEEK_CUR);
    sceIoRead(m_fd, NULL, size);

    pos = sceIoLseek(m_fd, 0, SCE_SEEK_CUR);
    LOG("file pos: %" PRIi64 "\n", pos);

    offset = 0;
    buffer_size = 0;
    while(offset < remaining_file_size) {

        if (remaining_file_size - offset > CHUNK_SIZE) 
            buffer_size = CHUNK_SIZE;
        else
            buffer_size = remaining_file_size - offset;

        int bytes = sceIoRead(m_fd, chunk, buffer_size);
        LOG("bytes read: %" PRIi32 "\n", bytes);
        bytes = sceIoWrite(m_tmp_fd, chunk, buffer_size);
        LOG("bytes written: %" PRIi32 "\n", bytes);

        offset += buffer_size;
    }

    sceIoClose(m_tmp_fd);
    sceIoClose(m_fd);

    sceIoRemove(m_path);
    sceIoRename(TMP_FILE, m_path);
    m_fd = sceIoOpen(m_path, SCE_O_RDWR | SCE_O_APPEND, 0777);
}

void File::insertData(uint32_t file_pos, void* data, uint32_t size) {
    sceIoRemove(TMP_FILE);
    close();
    if ((m_fd = sceIoOpen(m_path, SCE_O_RDONLY, 0777)) < 0) {
        LOG("could not open file for reading\n");
        return;
    }
    
    SceUID m_tmp_fd = -1;
    if ((m_tmp_fd = sceIoOpen(TMP_FILE, SCE_O_RDWR | SCE_O_CREAT, 0777)) < 0) {
        LOG("could not open temporary file\n");
        return;
    }

    SceOff fileSize = sceIoLseek(m_fd, 0, SCE_SEEK_END);
    LOG("file size: %" PRIi64 "\n", fileSize);
    uint32_t remaining_file_size = (uint32_t)fileSize;
    sceIoLseek(m_fd, 0, SCE_SEEK_SET);

    uint8_t chunk[CHUNK_SIZE] = { 0 };
    uint32_t offset = 0;
    uint32_t buffer_size = 0;
    while(offset < file_pos) {

        if (file_pos - offset > CHUNK_SIZE) 
            buffer_size = CHUNK_SIZE;
        else
            buffer_size = file_pos - offset;

        int bytes = sceIoRead(m_fd, chunk, buffer_size);
        bytes = sceIoWrite(m_tmp_fd, chunk, buffer_size);

        offset += buffer_size;
    }

    remaining_file_size -= file_pos;

    offset = 0;
    buffer_size = 0;
    uint8_t* copy_buffer = (uint8_t*)data;
    while (offset < size) {
        if (size - offset > CHUNK_SIZE) 
            buffer_size = CHUNK_SIZE;
        else
            buffer_size = size - offset;

        int bytes = sceIoWrite(m_tmp_fd, &copy_buffer[offset], buffer_size);
        LOG("bytes written: %" PRIi32 "\n", bytes);

        offset += buffer_size;
    }

    offset = 0;
    buffer_size = 0;
    while(offset < remaining_file_size) {

        if (remaining_file_size - offset > CHUNK_SIZE) 
            buffer_size = CHUNK_SIZE;
        else
            buffer_size = remaining_file_size - offset;

        int bytes = sceIoRead(m_fd, chunk, buffer_size);
        bytes = sceIoWrite(m_tmp_fd, chunk, buffer_size);

        offset += buffer_size;
    }
    flush();

    sceIoClose(m_tmp_fd);
    sceIoClose(m_fd);

    sceIoRemove(m_path);
    sceIoRename(TMP_FILE, m_path);
    m_fd = sceIoOpen(m_path, SCE_O_RDWR | SCE_O_APPEND, 0777);
}

void File::replaceData(uint32_t file_pos, uint32_t old_size, void* new_data, uint32_t new_size) {
    if (old_size == new_size) {
        close();
        m_fd = sceIoOpen(m_path, SCE_O_RDWR, 0777);
        sceIoPwrite(m_fd, new_data, new_size, file_pos);
    }
    else {
        removeData(file_pos, old_size);
        insertData(file_pos, new_data, new_size);
    }
}

void File::flush() {
}

void File::remove() {

}

void File::skip(uint32_t size) {
    sceIoLseek(m_fd, size, SCE_SEEK_CUR);
}

void File::close() {
    sceIoClose(m_fd);
    m_fd = -1;
}

void File::reset() {
    sceIoLseek(m_fd, 0, SCE_SEEK_SET);
}