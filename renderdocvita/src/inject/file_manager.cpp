#include "file_manager.h"

#define CHUNK_SIZE 2048

void File::open(const char* path) {
    m_path = path;
    if (kuIoOpen(m_path, SCE_O_WRONLY | SCE_O_APPEND, &m_fd) < 0) {
        LOG("could not open file: %s\n", m_path);
    }
}

uint32_t File::write(const void* data, uint32_t size) {
    if (m_fd < 0) {
        LOG("invalid file handle\n");
        return 0;
    }

    int bytesWritten = 0;
    if ((bytesWritten = kuIoWrite(m_fd, data, size)) == 0) {
        LOG("could not write to file: %s\n", m_path);
        return 0;
    }

    return (uint32_t)bytesWritten;
}

#define TMP_FILE "ux0:/data/renderdoc/tmp"

void File::removeData(uint32_t file_pos, uint32_t size) {
    kuIoRemove(TMP_FILE);
    SceUID m_tmp_fd = -1;
    if (kuIoOpen(TMP_FILE, SCE_O_RDWR | SCE_O_CREAT, &m_tmp_fd) < 0) {
        LOG("could not open temporary file\n");
        return;
    }

    close();
    if (kuIoOpen(m_path, SCE_O_RDONLY, &m_fd) < 0) {
        LOG("could not open file for reading\n");
        return;
    }

    kuIoLseek(m_fd, 0, SCE_SEEK_END);
    SceOff fileSize = 0;
    kuIoTell(m_fd, &fileSize);
    LOG("file size: %" PRIi64 "\n", fileSize);
    uint32_t remaining_file_size = (uint32_t)fileSize;
    kuIoLseek(m_fd, 0, SCE_SEEK_SET);

    uint8_t chunk[CHUNK_SIZE] = { 0 };
    uint32_t offset = 0;
    uint32_t buffer_size = 0;
    while(offset < file_pos) {

        if (file_pos - offset > CHUNK_SIZE) 
            buffer_size = CHUNK_SIZE;
        else
            buffer_size = file_pos - offset;

        int bytes = kuIoRead(m_fd, chunk, buffer_size);
        LOG("bytes read: %" PRIi32 "\n", bytes);
        bytes = kuIoWrite(m_tmp_fd, chunk, buffer_size);
        LOG("bytes written: %" PRIi32 "\n", bytes);

        offset += buffer_size;
    }

    remaining_file_size -= file_pos;
    remaining_file_size -= size;

    LOG("remaining file size: %" PRIu32 "\n", remaining_file_size);

    LOG("file_pos: %" PRIu32 "\n", file_pos);
    LOG("size: %" PRIu32 "\n", size);


    //kuIoLseek(m_fd, file_pos, SCE_SEEK_SET);
    flush();
    kuIoLseek(m_fd, size, SCE_SEEK_CUR);
    flush();

    offset = 0;
    buffer_size = 0;
    while(offset < remaining_file_size) {

        if (remaining_file_size - offset > CHUNK_SIZE) 
            buffer_size = CHUNK_SIZE;
        else
            buffer_size = remaining_file_size - offset;

        int bytes = kuIoRead(m_fd, chunk, buffer_size);
        LOG("bytes read: %" PRIi32 "\n", bytes);
        bytes = kuIoWrite(m_tmp_fd, chunk, buffer_size);
        LOG("bytes written: %" PRIi32 "\n", bytes);


        offset += buffer_size;
    }

    flush();

    kuIoClose(m_tmp_fd);
    kuIoClose(m_fd);

    kuIoRemove(m_path);
    kuIoRename(TMP_FILE, m_path);
    kuIoOpen(m_path, SCE_O_WRONLY | SCE_O_APPEND, &m_fd);
}

void File::flush() {
    SceOff pos;
    kuIoTell(m_fd, &pos);
}

void File::remove() {

}

void File::close() {
    kuIoClose(m_fd);
    m_fd = -1;
}
