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

#ifndef KINETIC_CPP_CLIENT_SOCKET_WRAPPER_H_
#define KINETIC_CPP_CLIENT_SOCKET_WRAPPER_H_

#include "socket_wrapper_interface.h"
#include "kinetic/connection_options.h"

namespace kinetic {

class SocketWrapper : public SocketWrapperInterface {
  public:
    explicit SocketWrapper(const std::string &host, int port, bool nonblocking = false);
    bool Connect();
    int fd();
    ~SocketWrapper();
  private:
    std::string host_;
    int port_;
    bool nonblocking_;
    int fd_;
};

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_SOCKET_WRAPPER_H_
