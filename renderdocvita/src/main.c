#include <vitasdk.h>
#include <taihen.h>

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <inttypes.h>

#include <psp2/gxm.h>

#include "logging.h"
#include "sfo.h"
#include "network.h"
#include "serializer.h"

#include "kernel/api.h"
#include "inject/util.h"

#define LOG_LOC "ux0:/tai/vitahook.log"
#define APP_DIR "ux0:/app"

typedef int(*threadfunc_t)(SceSize args, void* init);

SceUID createThread(const char *thread_name, threadfunc_t thread_func, size_t user_data_size, void* user_data) 
{
    SceUID thid = sceKernelCreateThread(thread_name, thread_func, 0x40, 0x4000, 0, 0, NULL);
    if (thid < 0)
    {
        return -1;
    }

    int res = sceKernelStartThread(thid, user_data_size, user_data);
    if (res < 0)
    {
        sceKernelDeleteThread(thid);
        return -1;
    }

    return thid;
}

enum PacketType
{
  ePacket_Noop = 1,
  ePacket_Handshake,
  ePacket_Busy,
  ePacket_NewCapture,
  ePacket_APIUse,
  ePacket_TriggerCapture,
  ePacket_CopyCapture,
  ePacket_DeleteCapture,
  ePacket_QueueCapture,
  ePacket_NewChild,
  ePacket_CaptureProgress,
  ePacket_CycleActiveWindow,
  ePacket_CapturableWindowCount
};

enum RemoteServerPacket
{
  eRemoteServer_Noop = 1,
  eRemoteServer_Handshake,
  eRemoteServer_VersionMismatch,
  eRemoteServer_Busy,

  eRemoteServer_Ping,
  eRemoteServer_RemoteDriverList,
  eRemoteServer_TakeOwnershipCapture,
  eRemoteServer_CopyCaptureToRemote,
  eRemoteServer_CopyCaptureFromRemote,
  eRemoteServer_OpenLog,
  eRemoteServer_LogOpenProgress,
  eRemoteServer_LogOpened,
  eRemoteServer_HasCallstacks,
  eRemoteServer_InitResolver,
  eRemoteServer_ResolverProgress,
  eRemoteServer_GetResolve,
  eRemoteServer_CloseLog,
  eRemoteServer_HomeDir,
  eRemoteServer_ListDir,
  eRemoteServer_ExecuteAndInject,
  eRemoteServer_ShutdownServer,
  eRemoteServer_GetDriverName,
  eRemoteServer_GetSectionCount,
  eRemoteServer_FindSectionByName,
  eRemoteServer_FindSectionByType,
  eRemoteServer_GetSectionProperties,
  eRemoteServer_GetSectionContents,
  eRemoteServer_WriteSection,
  eRemoteServer_GetAvailableGPUs,
  eRemoteServer_RemoteServerCount,
};

#define RENDERDOC_FIRST_TARGET_CONTROL_PORT 38920
#define RENDERDOC_LAST_TARGET_CONTROL_PORT 38927
#define RENDERDOC_SERVER_PORT 39920

static void WriteListDir(int* socket, uint32_t bytesWritten)
{
    uint64_t numDirs = 0;
    SceUID dfd = sceIoDopen(APP_DIR);
    if (dfd >= 0) {
        SceIoDirent dirEntry = {};
        int res;
        do {
            res = sceIoDread(dfd, &dirEntry);
            if (res > 0) { 
                ++numDirs;
            }
        } while(res > 0);
    }
    sceIoDclose(dfd);

    WriteUint64(socket, numDirs);
    bytesWritten += 8;

    dfd = sceIoDopen(APP_DIR);
    if (dfd >= 0) {
        SceIoDirent dirEntry = {};
        int res;
        do {
            res = sceIoDread(dfd, &dirEntry);
            if (res > 0) 
            {
                char path[128];
                sceClibSnprintf(path, 128, APP_DIR "/%s/sce_sys/param.sfo", dirEntry.d_name);
                SceUID fd = sceIoOpen(path, SCE_O_RDONLY, 0777);
                if (fd >= 0) {
                    SceOff length = sceIoLseek(fd, 0, SCE_SEEK_END);

                    sceIoLseek(fd, 0, SCE_SEEK_SET);
                    uint8_t buffer[4 * 1024];
                    sceIoRead(fd, buffer, length);
                    char title[128];
                    uint32_t str_len = 0;
                    getTitle(buffer, 0, length, (uint8_t*)&title[0], &str_len);
                    //analizeSFO(buffer, 0, length);
                    sceClibSnprintf(title, 128, "%s (%s)", title, dirEntry.d_name);
                    str_len = (uint32_t)strlen((const char*)&title[0]);

                    WriteString(socket, (uint8_t*)&title[0], str_len);
                    bytesWritten += 4;
                    bytesWritten += str_len;

                    WriteUint32(socket, 4);
                    bytesWritten += 4;

                    WriteUint32(socket, 0);
                    bytesWritten += 4;

                    WriteUint64(socket, 0);
                    bytesWritten += 8;
                }
                sceIoClose(fd);
            }
            ++numDirs;
        } while(res > 0);
    }
    sceIoDclose(dfd);
    
    uint32_t padding = 64 - (bytesWritten % 64);
    uint8_t bytes[64] = {};
    Write(socket, bytes, padding);
}

static SceUID threadId;
static SceUID multicastThreadId;
static int running = 0;

static void RespondToClient(int* socket, enum RemoteServerPacket type)
{
    switch (type)
    {
    case eRemoteServer_Handshake:
    {
        //LOG("Respond to Handshake\n");

        uint32_t handshake = eRemoteServer_Handshake;
        WriteUint32(socket, handshake);

        uint32_t chunkSize = 0;
        WriteUint32(socket, chunkSize);

        uint32_t version = 1007;
        WriteUint32(socket, version);

        uint32_t len = 6;
        uint8_t ps_str[] = "PSVita";
        WriteString(socket, ps_str, len);
        
        SceUID pid = sceKernelGetProcessId();

        WriteUint32(socket, (uint32_t)pid);

        uint8_t alignment[38] = {};
        Write(socket, alignment, 38);
    } break;
    case eRemoteServer_Ping:
    {
        uint32_t ping = eRemoteServer_Ping;
        WriteUint32(socket, ping);

        uint8_t padding[60] = {};
        Write(socket, padding, 60);
    } break;
    case eRemoteServer_ListDir:
    {
        LOG("Respond to ListDir\n");
        uint32_t listDir = eRemoteServer_ListDir;
        WriteUint32(socket, listDir); // 4

        uint32_t chunkSize = 0;
        WriteUint32(socket, chunkSize);

        WriteListDir(socket, 8);
    } break;
    case eRemoteServer_ExecuteAndInject:
    {

    }
    default:
        break;
    }
}

static void ReceiveFromClient(int* socket, enum RemoteServerPacket type)
{
    switch (type)
    {
    case eRemoteServer_Handshake:
    {
        //LOG("Received Handshake\n");
        
        uint32_t chunkSize = 0;
        ReadUint32(socket, &chunkSize);

        //LOG("Received chunkSize: %" PRIu32 "\n", chunkSize);

        uint32_t version;
        ReadUint32(socket, &version);

        //LOG("Received version: %" PRIu32 "\n", version);

        uint8_t padding[52];

        Read(socket, padding, 52);
    } break;
    case eRemoteServer_Ping:
    {
        //LOG("Received Ping\n");
    } break;
    case eRemoteServer_ListDir:
    {
        LOG("Received List Dir\n");

        uint32_t chunkSize = 0;
        ReadUint32(socket, &chunkSize);
    } break;
    case eRemoteServer_ExecuteAndInject:
    {
        LOG("Received Execute and Inject\n");

        uint32_t chunkSize = 0;
        ReadUint32(socket, &chunkSize);

        uint8_t str[16];
        memset(str, 0, 16);
        uint32_t length;
        ReadUint32(socket, &length);
        Read(socket, &str[0], length);

        vitaHookSetCurrentTitle((char*)&str[0], length);

        LOG("Received execution of %s of length %" PRIu32 "\n", str, length);

        char uri[32];
        sceClibSnprintf(uri, 32, "psgm:play?titleid=%s", str);

        if (sceAppMgrLaunchAppByUri(0x20000, uri) < 0) {
            LOG("Could not start app %s\n", str);
            break;
        }

        SceUID pid = -1;
        sceAppMgrGetIdByName(&pid, (char *)&str[0]);
        while (pid < 0)
        {
            sceAppMgrGetIdByName(&pid, (char *)&str[0]);
            sceKernelDelayThread(5 * 1000);
        }
        
        LOG("Started app %s with pid: %" PRIi32 "\n", str, pid);

        //hook = taiHookFunctionImportForKernel(pid, &ref, "SceGxm", 0x0D0AA0CB, 0xBC059AFC, &sceGxmDraw_hooked);
        //vitaHookInstallHooks(pid);
    }
    break;
    default:
        LOG("Received unknown RemoteServer Packet: %" PRIu32 "\n", (uint32_t)type);
        break;
    }

    RespondToClient(socket, type);
}

struct Client
{
    int client_socket;
};

static int sClientThreadInit(SceSize args, void *init)
{
    if (args < 1)
    {
        return 0;
    }

    struct Client* client_sock = (struct Client*)init;
    int client = client_sock->client_socket;

    while(client != -1)
    {
        uint32_t type = 0;
        ReadUint32(&client, &type);

        if (type == 0)
        {
            if (client != -1){
                uint8_t bytes[64];
                RecvDataNonBlocking(&client, bytes, 64);
                sceKernelDelayThread(5 * 1000);
            }
            continue;
        }

        ReceiveFromClient(&client, (enum RemoteServerPacket)type);
        sceKernelDelayThread(5 * 1000);
    }

    return 0;
}

static int sThreadInit(SceSize args, void *init)
{
    uint32_t port = RENDERDOC_SERVER_PORT;
    int socket = CreateSocket("0.0.0.0", port & 0xffff, 4);

    while (socket < 0) 
    {
        socket = CreateSocket("0.0.0.0", port & 0xffff, 4);
    }

    if (socket >= 0) 
    {
        running = 1;
    }

    while(running)
    {
        int client = AcceptClient(&socket, 0);

        if (client < 0) 
        {
            if (!(socket >= 0))
            {
                LOG("Error in accept - shutting down server\n");
                sceNetSocketClose(socket);
                return 0;
            }
            sceKernelDelayThread(5 * 1000);
            continue;
        }
        else {
            //LOG("accepted connection to client\n");
        }

        struct Client client_sock = {};
        client_sock.client_socket = client;

        SceUID clientThreadId = createThread("RemoteServerClientThread", &sClientThreadInit, sizeof(client_sock), &client_sock);

        int threadStatus;
        SceUInt timeout = (SceUInt)-1;
        sceKernelWaitThreadEnd(clientThreadId, &threadStatus, &timeout);

        sceKernelDeleteThread(clientThreadId);
    }

    return 0;
}

static int sMulticastInit(SceSize args, void *init)
{
  int multicast_socket = -1;
  int sock_running = 0;

  do {
    multicast_socket = CreateMulticastSocket(0xEFFFFFFA, 1900);

    sceKernelDelayThread(5 * 1000);

  } while(multicast_socket < 0);

  if (multicast_socket >= 0) 
  {
    sock_running = 1;
  }

  while(sock_running)
  {
    SceNetSockaddrIn in_addr = {};
    in_addr.sin_family = SCE_NET_AF_INET;
    in_addr.sin_port = sceNetHtons(1900);
    in_addr.sin_addr.s_addr = sceNetHtonl(0xEFFFFFFA);

    uint8_t str[] = "Hello World";
    sceNetSendto(multicast_socket, str, 11, 0, (const SceNetSockaddr *)&in_addr, sizeof(in_addr));
    sceKernelDelayThread(500 * 1000);
  }

  return 0;
}

int _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp) {
    LOG("vitahook\n");

    threadId = createThread("RemoteServerThread", &sThreadInit, 0, NULL);
    multicastThreadId = createThread("MulticastThread", &sMulticastInit, 0, NULL);

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize args, void *argp) {
    LOG("end vitahook\n");
    return SCE_KERNEL_STOP_SUCCESS;
}
