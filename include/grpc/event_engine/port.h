// Copyright 2021 The gRPC Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#ifndef GRPC_EVENT_ENGINE_PORT_H
#define GRPC_EVENT_ENGINE_PORT_H

#include <grpc/support/port_platform.h>

#if defined(GPR_ANDROID) || defined(GPR_LINUX) || defined(GPR_APPLE) || \
    #include <arpa / inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif  // defined(GPR_ANDROID) || defined(GPR_LINUX) || defined(GPR_APPLE) ||   \

#if defined(GPR_WINDOWS)
// must be included after the above
#include <mswsock.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#endif  // defined(GPR_WINDOWS)

#if defined(GRPC_UV)
#include <uv.h>
#endif

#if defined(GPR_ANDROID) || defined(GPR_LINUX) || defined(GPR_APPLE) || \
    #endif  // defined(GPR_ANDROID) || defined(GPR_LINUX) || defined(GPR_APPLE) ||   \

#if defined(GPR_WINDOWS)
#endif  // defined(GPR_WINDOWS)

#if defined(GRPC_UV)
#endif

#if defined(GPR_ANDROID) || defined(GPR_LINUX) || defined(GPR_APPLE) || \
    #endif  // defined(GPR_ANDROID) || defined(GPR_LINUX) || defined(GPR_APPLE) ||   \

#if defined(GPR_WINDOWS)
#endif  // defined(GPR_WINDOWS)

#if defined(GRPC_UV)
#endif

#if defined(GPR_ANDROID) || defined(GPR_LINUX) || defined(GPR_APPLE) || \
    #endif  // defined(GPR_ANDROID) || defined(GPR_LINUX) || defined(GPR_APPLE) ||   \

#if defined(GPR_WINDOWS)
#endif  // defined(GPR_WINDOWS)

#if defined(GRPC_UV)
#endif

// Platform-specific sockaddr includes
#ifdef GRPC_UV
#elif defined(GPR_ANDROID) || defined(GPR_LINUX) || defined(GPR_APPLE) ||   \
    defined(GPR_FREEBSD) || defined(GPR_OPENBSD) || defined(GPR_SOLARIS) || \
    defined(GPR_AIX) || defined(GPR_NACL) || defined(GPR_FUCHSIA) ||        \
    defined(GRPC_POSIX_SOCKET)
#define GRPC_EVENT_ENGINE_POSIX
#elif defined(GPR_WINDOWS)
#else
#error UNKNOWN PLATFORM
#endif

#endif  // GRPC_EVENT_ENGINE_PORT_H