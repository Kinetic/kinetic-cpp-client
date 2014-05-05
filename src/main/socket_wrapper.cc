/*
 * kinetic-cpp-client
 * Copyright (C) 2014 Seagate Technology.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "glog/logging.h"

#include "socket_wrapper.h"

using kinetic::SocketWrapper;

SocketWrapper::SocketWrapper(const std::string &host, int port, bool nonblocking)
    : host_(host), port_(port), nonblocking_(nonblocking), fd_(-1) {}

    SocketWrapper::~SocketWrapper() {
        if (fd_ == -1) {
            LOG(INFO) << "Not connected so no cleanup needed";
        } else {
            LOG(INFO) << "Closing socket with fd " << fd_;
            if (close(fd_)) {
                PLOG(ERROR) << "Error closing socket fd " << fd_;
            }
        }
    }

bool SocketWrapper::Connect() {
    LOG(INFO) << "Connecting to " << host_
        << ":" << port_;

    // Lookup host IP
    struct hostent *he;
    struct in_addr **addr_list;

    if (!(he = gethostbyname(host_.c_str()))) {
        PLOG(ERROR) << "Unable to resolve host " << host_;
        return false;
    }

    addr_list = (struct in_addr**)he->h_addr_list;
    if (!addr_list[0]) {
        PLOG(ERROR) << "Unable to resolve host " << host_;
        return false;
    }

    // Establish the connection
    struct sockaddr_in server_name;

    fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (fd_ == -1) {
        PLOG(ERROR) << "Unable to create socket";
        return false;
    }

    // os x won't let us set close-on-exec when creating the socket, so set it separately
    int current_fd_flags = fcntl(fd_, F_GETFD);
    if (current_fd_flags == -1) {
        PLOG(ERROR) << "Failed to get socket fd flags";
        return false;
    }
    if (fcntl(fd_, F_SETFD, current_fd_flags | FD_CLOEXEC) == -1) {
        PLOG(ERROR) << "Failed to set socket close-on-exit";
        return false;
    }

    // On BSD-like systems we can set SO_NOSIGPIPE on the socket to prevent it from sending a
    // PIPE signal and bringing down the whole application if the server closes the socket
    // forcibly
#ifdef SO_NOSIGPIPE
    int set = 1;
    int setsockopt_result = setsockopt(fd_, SOL_SOCKET, SO_NOSIGPIPE, &set, sizeof(set));
    // Allow ENOTSOCK because it allows tests to use pipes instead of real sockets
    if (setsockopt_result != 0 && setsockopt_result != ENOTSOCK) {
        PLOG(ERROR) << "Failed to set SO_NOSIGPIPE on socket";
        return false;
    }
#endif

    server_name.sin_family = AF_INET;
    server_name.sin_addr = *addr_list[0];
    server_name.sin_port = htons(port_);

    if (connect(fd_, (struct sockaddr*)&server_name, sizeof(server_name)) == -1) {
        PLOG(WARNING) << "Unable to connect";
        return false;
    }

    if (nonblocking_ && fcntl(fd_, F_SETFL, O_NONBLOCK) != 0) {
        PLOG(ERROR) << "Failed to set socket nonblocking";
        return false;
    }

    return true;
}

int SocketWrapper::fd() {
    return fd_;
}
