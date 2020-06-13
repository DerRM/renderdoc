#ifndef __VITA_HOOK_SERIALIZER_H__
#define __VITA_HOOK_SERIALIZER_H__

#include <stdint.h>
#include <inttypes.h>

int Write(int* socket, void *data, uint32_t numBytes);
int WriteUint32(int* socket, uint32_t val);
int WriteUint64(int* socket, uint64_t val);
int WriteString(int* socket, uint8_t *str, uint32_t length);
int WriteBool(int* socket, uint8_t bool);

int Read(int* socket, void *data, uint32_t numBytes);
int ReadUint32(int* socket, uint32_t *val);
int ReadString(int* socket, uint8_t *str, uint32_t *length);
int ReadBool(int* socket, uint8_t *bool);

#endif
