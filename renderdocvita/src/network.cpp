
#include <stdio.h>
#include <psp2/kernel/threadmgr.h>

#include "logging.h"
#include "network.h"

#ifdef __cplusplus
extern "C" {
#endif

uint32_t timeoutMS = 5000;
static char message[64] = { 0 };

const char* sce_net_error_string(SceNetErrorCode err)
{
  switch(err)
  {
    /*case SCE_NET_ERROR_EAGAIN: */
    case SCE_NET_ERROR_EWOULDBLOCK: return "SCE_NET_ERROR_EWOULDBLOCK: Operation would block.";
    case SCE_NET_ERROR_EINVAL: return "SCE_NET_ERROR_EINVAL: Invalid argument.";
    case SCE_NET_ERROR_EADDRINUSE: return "SCE_NET_ERROR_EADDRINUSE: Address already in use.";
    case SCE_NET_ERROR_ECONNRESET: return "SCE_NET_ERROR_ECONNRESET: A connection was forcibly closed by a peer.";
    case SCE_NET_ERROR_EINPROGRESS: return "SCE_NET_ERROR_EINPROGRESS: Operation now in progress.";
    case SCE_NET_ERROR_EPIPE: return "SCE_NET_ERROR_EPIPE: Broken pipe.";
    case SCE_NET_ERROR_EINTR:
      return "SCE_NET_ERROR_EINTR: The function was interrupted by a signal that was caught, before any data was "
             "available.";
    case SCE_NET_ERROR_ETIMEDOUT: return "SCE_NET_ERROR_ETIMEDOUT: A socket operation timed out.";
    case SCE_NET_ERROR_ECONNABORTED: return "SCE_NET_ERROR_ECONNABORTED: A connection has been aborted.";
    case SCE_NET_ERROR_ECONNREFUSED: return "SCE_NET_ERROR_ECONNREFUSED: A connection was refused.";
    case SCE_NET_ERROR_EHOSTDOWN: return "SCE_NET_ERROR_EHOSTDOWN: Host is down.";
    case SCE_NET_ERROR_EHOSTUNREACH: return "SCE_NET_ERROR_EHOSTUNREACH: No route to host.";
    default: {
      sceClibSnprintf(message, 64, "0x%x" PRIu32, (uint32_t)err);
      return message;
    }
  }

  //return "Unknown error";
}

void Shutdown(int* socket)
{
  if (*socket != -1) {
    sceNetShutdown(*socket, SCE_NET_SHUT_RDWR);
    sceNetSocketClose(*socket);
    *socket = -1;
  }
}

int AcceptClient(int* socket, uint32_t timeoutMilliseconds)
{
    do 
    {
        SceNetSockaddr addr;
        unsigned int len = sizeof(addr);

        int res = sceNetAccept(*socket, &addr, &len);

        if (res < 0) 
        {
          if ((unsigned int)(res) != SCE_NET_ERROR_EWOULDBLOCK 
              && (unsigned int)(res) != SCE_NET_ERROR_EAGAIN 
              && (unsigned int)(res) != SCE_NET_ERROR_EINTR)
          {
              LOG("accept: %s\n", sce_net_error_string((SceNetErrorCode)res));
              Shutdown(socket);
          } 
        }
        else {
          int nonblock = 1;
          sceNetSetsockopt(res, SCE_NET_SOL_SOCKET, SCE_NET_SO_NBIO, &nonblock, sizeof(nonblock));

          int nodelay = 1;
          sceNetSetsockopt(res, SCE_NET_IPPROTO_TCP, SCE_NET_TCP_NODELAY, &nodelay, sizeof(nodelay));

          return res;
        }

        const uint32_t sleeptime = 4;

        sceKernelDelayThread(sleeptime * 1000);

        if (sleeptime < timeoutMilliseconds)
          timeoutMilliseconds -= sleeptime;
        else
          timeoutMilliseconds = UINT32_C(0);
    } while (timeoutMilliseconds);

    return -1;
}

int CreateSocket(const char* bindaddr, uint16_t port, int queueSize)
{
    char socketName[32];
    sprintf(socketName, "renderdoc_%d", port);

    int socket = sceNetSocket(socketName, SCE_NET_AF_INET, SCE_NET_SOCK_STREAM, 0);

    int yes = 1;
    sceNetSetsockopt(socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_REUSEADDR, &yes, sizeof(yes));

    if (socket < 0)
    {
        LOG("Failed to open abstract socket: %s\n", socketName);
        return -1;
    }

    SceNetSockaddrIn addr = {};
    int resolver = sceNetResolverCreate("resolver", NULL, 0);
    sceNetResolverStartNtoa(resolver, bindaddr, (SceNetInAddr *)&addr, 0, 0, 0);

    addr.sin_family = SCE_NET_AF_INET;
    addr.sin_port = sceNetHtons(port);

    int result = sceNetBind(socket, (const SceNetSockaddr *)&addr, sizeof(addr));
    if (result < 0)
    {
        LOG("Failed to create abstract socket: %s\n", socketName);
        sceNetSocketClose(socket);
        return -1;
    }
    LOG("Created and bind socket: %d\n", socket);

    result = sceNetListen(socket, queueSize);
    if (result < 0) 
    {
        LOG("Failed to listen on %s\n", socketName);
        sceNetSocketClose(socket);
        return -1;
    }

    int nonblock = 1;
    sceNetSetsockopt(socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_NBIO, &nonblock, sizeof(nonblock));

    return socket;
}

int SendDataBlocking(int* socket, const void *buf, uint32_t length)
{
  //LOG("SendDataBlocking buf: %p, length: %" PRIu32 "\n", buf, length);
  if (length == 0)
    return 1;

  uint32_t sent = 0;

  char *src = (char *)buf;

  int nonblock = 0;
  sceNetSetsockopt(*socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_NBIO, &nonblock, sizeof(nonblock));

  int oldtimeout = 0;
  unsigned int len = sizeof(oldtimeout);
  sceNetGetsockopt(*socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_SNDTIMEO, &oldtimeout, &len);

  sceNetSetsockopt(*socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_SNDTIMEO, &timeoutMS, sizeof(timeoutMS));

  while (sent < length)
  {
    int ret = sceNetSend(*socket, src, length - sent, 0);

    if (ret <= 0)
    {
      if ((unsigned int)(ret) == SCE_NET_ERROR_EINTR)
      {
        continue;
      }
      else if((unsigned int)(ret) == SCE_NET_ERROR_EWOULDBLOCK
              || (unsigned int)(ret) == SCE_NET_ERROR_EAGAIN)
      {
        LOG("Timeout in recv\n");
        Shutdown(socket);
        return 0;
      }
      else
      {
        LOG("send: %s\n", sce_net_error_string((SceNetErrorCode)ret));
        Shutdown(socket);
        return 0;
      }
    }

    sent += ret;
    src += ret;
  }

  nonblock = 1;
  sceNetSetsockopt(*socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_NBIO, &nonblock, sizeof(nonblock));

  sceNetSetsockopt(*socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_SNDTIMEO, &oldtimeout, sizeof(oldtimeout));

  //RDCASSERT(sent == length);

  return 1;
}

bool IsRecvDataWaiting(int* socket)
{
  char dummy;
  int ret = sceNetRecv(*socket, &dummy, 1, SCE_NET_MSG_PEEK);

  if (ret == 0)
  {
    Shutdown(socket);
    return false;
  }
  else if (ret <= 0)
  {
    if ((unsigned int)(ret) == SCE_NET_ERROR_EINTR 
        || (unsigned int)(ret) == SCE_NET_ERROR_EWOULDBLOCK
        || (unsigned int)(ret) == SCE_NET_ERROR_EAGAIN)
    {
      ret = 0;
    }
    else
    {
      LOG("recv: %s\n", sce_net_error_string((SceNetErrorCode)ret));
      Shutdown(socket);
      return false;
    }
  }

  return ret > 0;
}

int RecvDataBlocking(int* socket, void *buf, uint32_t length)
{
  uint32_t timeoutMS = 5000 * 1000;
  //LOG("RecvDataBlocking buf: %p, length: %" PRIu32 "\n", buf, length);
  if (length == 0)
    return 1;

  uint32_t received = 0;

  char *dst = (char *)buf;

  int nonblock = 0;
  sceNetSetsockopt(*socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_NBIO, &nonblock, sizeof(nonblock));

  int oldtimeout = 0;
  unsigned int len = sizeof(oldtimeout);
  sceNetGetsockopt(*socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_RCVTIMEO, &oldtimeout, &len);

  sceNetSetsockopt(*socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_RCVTIMEO, &timeoutMS, sizeof(timeoutMS));

  while (received < length)
  {
    int ret = sceNetRecv(*socket, dst, length - received, 0);

    if (ret == 0)
    {
      Shutdown(socket);
      return 0;
    }
    else if (ret < 0)
    {
      if ((unsigned int)(ret) == SCE_NET_ERROR_EINTR)
      {
        continue;
      }
      else if((unsigned int)(ret) == SCE_NET_ERROR_EWOULDBLOCK
              || (unsigned int)(ret) == SCE_NET_ERROR_EAGAIN)
      {
        LOG("Timeout in recv\n");
        Shutdown(socket);
        return 0;
      }
      else
      {
        LOG("recv: %s\n", sce_net_error_string((SceNetErrorCode)ret));
        Shutdown(socket);
        return 0;
      }
    }

    received += ret;
    dst += ret;
  }

  nonblock = 1;
  sceNetSetsockopt(*socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_NBIO, &nonblock, sizeof(nonblock));

  sceNetSetsockopt(*socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_RCVTIMEO, &oldtimeout, sizeof(oldtimeout));

  //assert(received == length);

  return 1;
}

int RecvDataNonBlocking(int* socket, void *buf, uint32_t length)
{
  //LOG("RecvDataNonBlocking buf: %p, length: %" PRIu32 "\n", buf, length);
  if (length == 0)
    return 1;

  int ret = sceNetRecv(*socket, (void *)buf, length, 0);

  if (ret > 0)
  {
    length = (uint32_t)ret;
  }
  else
  {
    length = 0;

    if ((unsigned int)(ret) == SCE_NET_ERROR_EWOULDBLOCK
        || (unsigned int)(ret) == SCE_NET_ERROR_EAGAIN
        || (unsigned int)(ret) == SCE_NET_ERROR_EINTR)
    {
      return 1;
    }
    else {
      LOG("recv: %s : %" PRIu32 "\n", sce_net_error_string((SceNetErrorCode)ret), (uint32_t)ret);
      Shutdown(socket);
      return 0;
    }
  }

  return 1;
}

int CreateMulticastSocket(uint32_t bindaddr, uint16_t port)
{
    char socketName[32];
    sprintf(socketName, "multicast_%d", port);

    int socket = sceNetSocket(socketName, SCE_NET_AF_INET, SCE_NET_SOCK_DGRAM, 0);

    int yes = 1;
    sceNetSetsockopt(socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_REUSEADDR, &yes, sizeof(yes));

    //sceNetSetsockopt(socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_REUSEPORT, &yes, sizeof(yes));

    if (socket < 0)
    {
        LOG("Failed to open multicast socket: %s\n", socketName);
        return -1;
    }

    SceNetSockaddrIn in_addr = {};
    in_addr.sin_family = SCE_NET_AF_INET;
    in_addr.sin_port = port;
    in_addr.sin_addr.s_addr = sceNetHtonl(SCE_NET_INADDR_ANY);

    int result = sceNetBind(socket, (const SceNetSockaddr *)&in_addr, sizeof(in_addr));
    if (result < 0)
    {
        LOG("Failed to create multicast socket: %s\n", socketName);
        sceNetSocketClose(socket);
        return -1;
    }
    LOG("Created and bind multicast socket: %d\n", socket);

    SceNetIpMreq mc;
    mc.imr_multiaddr.s_addr = bindaddr;
    mc.imr_interface.s_addr = sceNetHtonl(SCE_NET_INADDR_ANY);
    sceNetSetsockopt(socket, SCE_NET_IPPROTO_IP, SCE_NET_IP_ADD_MEMBERSHIP, (const void*)&mc, sizeof(mc));

    uint8_t ttl = 255;
    sceNetSetsockopt(socket, SCE_NET_IPPROTO_IP, SCE_NET_IP_MULTICAST_TTL, (const void*)&ttl, sizeof(ttl));

    return socket;
}

int CreateLoopbackSocket(uint16_t port, int listen)
{
    char socketName[32];
    sprintf(socketName, "loopback_%d", port);

    int socket = sceNetSocket(socketName, SCE_NET_AF_INET, SCE_NET_SOCK_DGRAM, 0);


    if (socket < 0)
    {
        LOG("Failed to open abstract socket: %s\n", socketName);
        return -1;
    }

    if (listen) {
        SceNetSockaddrIn in_addr = {};
        in_addr.sin_family = SCE_NET_AF_INET;
        in_addr.sin_port = port;
        in_addr.sin_addr.s_addr = sceNetHtonl(SCE_NET_INADDR_LOOPBACK);
        
        int yes = 1;
        sceNetSetsockopt(socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_REUSEADDR, &yes, sizeof(yes));
        
        int result = sceNetBind(socket, (const SceNetSockaddr *)&in_addr, sizeof(in_addr));
        if (result < 0)
        {
            LOG("Failed to create abstract socket: %s\n", socketName);
            sceNetSocketClose(socket);
            return -1;
        }
        LOG("Created and bind socket: %d\n", socket);
    }

    return socket;
}

#ifdef __cplusplus
}
#endif
