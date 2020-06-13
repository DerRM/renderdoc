/******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2019-2020 Baldur Karlsson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ******************************************************************************/

#include <inttypes.h>
#include "common/common.h"
#include "common/formatting.h"
#include "os/os_specific.h"

#include "vita_network.h"

static rdcstr sce_net_error_string(int err)
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
    default: break;
  }

  return StringFormat::Fmt("Unknown error %d", err);
}

namespace Network
{
void Init()
{
  sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
  SceNetInitParam netParams = {};
  int size = 1 * 1024 * 1024;
  netParams.size = size;
  netParams.memory = malloc(size);
  netParams.flags = 0;

  sceNetInit(&netParams);

  sceNetCtlInit();
}

void Shutdown()
{
  sceNetCtlTerm();
  sceNetTerm();
  sceSysmoduleUnloadModule(SCE_SYSMODULE_NET);
}

Socket::~Socket()
{
  Shutdown();
}

void Socket::Shutdown()
{
  if (Connected())
  {
    sceNetShutdown(socket, SCE_NET_SHUT_RDWR);
    sceNetSocketClose(socket);
    socket = -1;
  }
}

bool Socket::Connected() const
{
  return socket > 0;
}

Socket *Socket::AcceptClient(uint32_t timeoutMilliseconds)
{
  do 
  {
    SceNetSockaddr addr;
    unsigned int len = sizeof(addr);

    int res = sceNetAccept(socket, &addr, &len);

    if (res < 0) 
    {
      if (static_cast<unsigned int>(res) != SCE_NET_ERROR_EWOULDBLOCK 
          && static_cast<unsigned int>(res) != SCE_NET_ERROR_EAGAIN 
          && static_cast<unsigned int>(res) != SCE_NET_ERROR_EINTR)
      {
        RDCWARN("accept: %s", sce_net_error_string(res).c_str());
        Shutdown();
      } 
    }
    else {
      int nonblock = 1;
      sceNetSetsockopt(res, SCE_NET_SOL_SOCKET, SCE_NET_SO_NBIO, &nonblock, sizeof(nonblock));

      int nodelay = 1;
      sceNetSetsockopt(res, SCE_NET_IPPROTO_TCP, SCE_NET_TCP_NODELAY, &nodelay, sizeof(nodelay));

      return new Socket((ptrdiff_t)res);
    }

    const uint32_t sleeptime = 4;

    Threading::Sleep(sleeptime);

    if (sleeptime < timeoutMilliseconds)
      timeoutMilliseconds -= sleeptime;
    else
      timeoutMilliseconds = UINT32_C(0);
  } while (timeoutMilliseconds);

  return NULL;
}

bool Socket::SendDataBlocking(const void *buf, uint32_t length)
{
  RDCDEBUG("SendDataBlocking buf: %p, length: %" PRIu32, buf, length);
  if (length == 0)
    return true;

  uint32_t sent = 0;

  char *src = (char *)buf;

  int nonblock = 0;
  sceNetSetsockopt(socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_NBIO, &nonblock, sizeof(nonblock));

  int oldtimeout = 0;
  unsigned int len = sizeof(oldtimeout);
  sceNetGetsockopt(socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_SNDTIMEO, &oldtimeout, &len);

  sceNetSetsockopt(socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_SNDTIMEO, &timeoutMS, sizeof(timeoutMS));

  while (sent < length)
  {
    int ret = sceNetSend(socket, src, length - sent, 0);

    if (ret == 0)
    {
      Shutdown();
      return false;
    }
    else if (ret < 0)
    {
      if (static_cast<unsigned int>(ret) == SCE_NET_ERROR_EINTR)
      {
        continue;
      }
      else if(static_cast<unsigned int>(ret) == SCE_NET_ERROR_EWOULDBLOCK
              || static_cast<unsigned int>(ret) == SCE_NET_ERROR_EAGAIN)
      {
        RDCWARN("Timeout in recv");
        Shutdown();
        return false;
      }
      else
      {
        RDCWARN("send: %s", sce_net_error_string(ret).c_str());
        Shutdown();
        return false;
      }
    }

    sent += ret;
    src += ret;
  }

  nonblock = 1;
  sceNetSetsockopt(socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_NBIO, &nonblock, sizeof(nonblock));

  sceNetSetsockopt(socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_SNDTIMEO, &oldtimeout, sizeof(oldtimeout));

  RDCASSERT(sent == length);

  return true;
}

bool Socket::IsRecvDataWaiting()
{
  return true;
}

bool Socket::RecvDataNonBlocking(void *buf, uint32_t &length)
{
  RDCDEBUG("RecvDataNonBlocking buf: %p, length: %" PRIu32, buf, length);
  if (length == 0)
    return true;

  int ret = sceNetRecv(socket, (void *)buf, length, 0);

  if (ret > 0)
  {
    length = (uint32_t)ret;
  }
  else
  {
    length = 0;

    if (static_cast<unsigned int>(ret) == SCE_NET_ERROR_EWOULDBLOCK
        || static_cast<unsigned int>(ret) == SCE_NET_ERROR_EAGAIN
        || static_cast<unsigned int>(ret) == SCE_NET_ERROR_EINTR)
    {
      return true;
    }
    else {
      RDCWARN("recv: %s", sce_net_error_string(ret).c_str());
      Shutdown();
      return false;
    }
  }

  return true;
}

bool Socket::RecvDataBlocking(void *buf, uint32_t length)
{
  RDCDEBUG("RecvDataBlocking buf: %p, length: %" PRIu32, buf, length);
  if (length == 0)
    return true;

  uint32_t received = 0;

  char *dst = (char *)buf;

  int nonblock = 0;
  sceNetSetsockopt(socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_NBIO, &nonblock, sizeof(nonblock));

  int oldtimeout = 0;
  unsigned int len = sizeof(oldtimeout);
  sceNetGetsockopt(socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_RCVTIMEO, &oldtimeout, &len);

  sceNetSetsockopt(socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_RCVTIMEO, &timeoutMS, sizeof(timeoutMS));

  while (received < length)
  {
    int ret = sceNetRecv(socket, dst, length - received, 0);

    if (ret == 0)
    {
      Shutdown();
      return false;
    }
    else if (ret < 0)
    {
      if (static_cast<unsigned int>(ret) == SCE_NET_ERROR_EINTR)
      {
        continue;
      }
      else if(static_cast<unsigned int>(ret) == SCE_NET_ERROR_EWOULDBLOCK
              || static_cast<unsigned int>(ret) == SCE_NET_ERROR_EAGAIN)
      {
        RDCWARN("Timeout in recv");
        Shutdown();
        return false;
      }
      else
      {
        RDCWARN("recv: %s", sce_net_error_string(ret).c_str());
        Shutdown();
        return false;
      }
    }

    received += ret;
    dst += ret;
  }

  nonblock = 1;
  sceNetSetsockopt(socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_NBIO, &nonblock, sizeof(nonblock));

  sceNetSetsockopt(socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_RCVTIMEO, &oldtimeout, sizeof(oldtimeout));

  RDCASSERT(received == length);

  return true;
}

uint32_t Socket::GetRemoteIP() const
{
  return GetIPFromTCPSocket((int)socket);
}

uint32_t GetIPFromTCPSocket(int socket)
{
  SceNetSockaddr addr = {};
  unsigned int len = sizeof(addr);

  sceNetGetpeername(socket, &addr, &len);

  uint32_t add_uint[2] = {0};
  memcpy(add_uint, addr.sa_data, sizeof(addr.sa_data));

  uint32_t ip = ((add_uint[0] >> 16) & 0xff) << 24 | ((add_uint[0] >> 24) & 0xff) << 16 | ((add_uint[1] >> 0) & 0xff) << 8 | ((add_uint[1] >> 8) & 0xff);

  return ip;
}

Socket *CreateAbstractServerSocket(uint16_t port, int queuesize)
{
  return NULL;
}

Socket *CreateClientSocket(const char *host, uint16_t port, int timeoutMS)
{
  return NULL;
}

Socket *CreateServerSocket(const char *bindaddr, uint16_t port, int queuesize)
{
  return CreateTCPServerSocket(bindaddr, port, queuesize);
}

Socket *CreateTCPServerSocket(const char* bindaddr, uint16_t port, int queueSize)
{
  char socketName[32];
  sprintf(socketName, "renderdoc_%d", port);

  int socket = sceNetSocket(socketName, SCE_NET_AF_INET, SCE_NET_SOCK_STREAM, 0);

  int yes = 1;
  sceNetSetsockopt(socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_REUSEADDR, &yes, sizeof(yes));

  if (socket < 0)
  {
    return NULL;
  }

  SceNetSockaddrIn addr = {};
  int resolver = sceNetResolverCreate("resolver", NULL, 0);
  sceNetResolverStartNtoa(resolver, bindaddr, (SceNetInAddr *)&addr, 0, 0, 0);

  addr.sin_family = SCE_NET_AF_INET;
  addr.sin_port = sceNetHtons(port);

  int result = sceNetBind(socket, (const SceNetSockaddr *)&addr, sizeof(addr));
  if (result < 0)
  {
    RDCWARN("Failed to create abstract socket: %s", socketName);
    sceNetSocketClose(socket);
    return NULL;
  }
  RDCLOG("Created and bind socket: %d", socket);

  result = sceNetListen(socket, queueSize);
  if (result < 0) 
  {
    RDCWARN("Failed to listen on %s", socketName);
    sceNetSocketClose(socket);
    return NULL;
  }

  int nonblock = 1;
  sceNetSetsockopt(socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_NBIO, &nonblock, sizeof(nonblock));

  return new Socket((ptrdiff_t)socket);
}

bool ParseIPRangeCIDR(const char *str, uint32_t &ip, uint32_t &mask)
{
  return true;
}
};