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

#ifndef KINETIC_CPP_CLIENT_SOCKET_WRAPPER_INTERFACE_H_
#define KINETIC_CPP_CLIENT_SOCKET_WRAPPER_INTERFACE_H_

namespace kinetic {

/// Simple wrapper around a socket FD that closes the FD
/// in the destructor
class SocketWrapperInterface {
  public:
    /// Actually open the connection to the socket. The details
    /// of the host/port/protocol to connect on depend on the
    /// implementation. Returns true if the connection succeeds
    /// and the socket is ready for reading/writing
    virtual bool Connect() = 0;

    /// Returns the FD
    virtual int fd() = 0;

    /// The destructor should close the FD if it was opened
    /// by connect
    virtual ~SocketWrapperInterface() {}
};

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_SOCKET_WRAPPER_INTERFACE_H_
