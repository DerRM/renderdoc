
#include <string.h>

#include "serializer.h"
#include "network.h"

int Write(int* socket, void *data, uint32_t numBytes)
{
    int success = SendDataBlocking(socket, data, numBytes);
    return success;
}

int WriteUint32(int* socket, uint32_t val)
{
    return Write(socket, &val, sizeof(uint32_t));
}

int WriteUint64(int* socket, uint64_t val)
{
    return Write(socket, &val, sizeof(uint64_t));
}

int WriteFloat(int* socket, float val)
{
    return Write(socket, &val, sizeof(float));
}

int WriteString(int* socket, uint8_t *str, uint32_t length)
{
    if (Write(socket, &length, 4)) {
        return Write(socket, str, length);
    }
    else {
        return 0;
    }
}

int WriteBool(int* socket, uint8_t val)
{
    return Write(socket, &val, 1);
}

#define BUFFER_SIZE 64 * 1024
static uint8_t readBuffer[BUFFER_SIZE];

int Read(int* socket, void *data, uint32_t numBytes)
{
    int success = RecvDataBlocking(socket, readBuffer, numBytes);
    if (success) {
        memcpy(data, readBuffer, numBytes);
    }
    return success;
}

int ReadUint32(int* socket, uint32_t *val)
{
    return Read(socket, val, sizeof(uint32_t));
}

int ReadString(int* socket, uint8_t *str, uint32_t *length)
{
    if (Read(socket, length, sizeof(uint32_t))) {
        return Read(socket, &str[0], *length);
    }
    else {
        return 0;
    }
}

int ReadBool(int* socket, uint8_t *val)
{
    return Read(socket, val, 1);
}
