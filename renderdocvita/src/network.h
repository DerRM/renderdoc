#ifndef __VITA_HOOK_NETWORK_H__
#define __VITA_HOOK_NETWORK_H__

#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <psp2/net/net.h>

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t timeoutMS;

int AcceptClient(int* socket, uint32_t timeoutMilliseconds);
int CreateSocket(const char* bindaddr, uint16_t port, int queueSize);
int SendDataBlocking(int* socket, const void *buf, uint32_t length);
int RecvDataBlocking(int* socket, void *buf, uint32_t length);
int RecvDataNonBlocking(int* socket, void *buf, uint32_t length);
int CreateMulticastSocket(uint32_t bindaddr, uint16_t port);
int CreateLoopbackSocket(uint16_t port, int listen);
void Shutdown(int* socket);
bool IsRecvDataWaiting(int* socket);

const char* sce_net_error_string(SceNetErrorCode err);

#ifdef __cplusplus
}
#endif

#endif
