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

#ifndef KINETIC_CPP_CLIENT_CONNECTION_HANDLE_H_
#define KINETIC_CPP_CLIENT_CONNECTION_HANDLE_H_

#include "kinetic/blocking_kinetic_connection.h"
#include "kinetic/nonblocking_kinetic_connection.h"
#include "protobufutil/common.h"

namespace kinetic {

/// Represents a single connection to a single Kinetic drive and allows interacting with
/// it in a blocking or non-blocking manner. Blocking and non-blocking operations can be
/// interleaved freely.
class ConnectionHandle {
    public:
    /// Creates a new ConnectionHandle and takes ownership of the given BlockingKineticConnection
    /// and the given nonblocking_connection. Instead of creating instances of ConnectionHandle
    /// directly use the KineticConnectionFactory
    ConnectionHandle(
            BlockingKineticConnection* blocking_connection,
            NonblockingKineticConnection* nonblocking_connection);

    ~ConnectionHandle();

    /// Returns a BlockingKineticConnection reference for using the underlying connection with a
    /// blocking interface
    BlockingKineticConnection& blocking();

    /// Returns a NonblockingKineticConnection reference for using the underlying connection with a
    /// blocking interface
    NonblockingKineticConnection& nonblocking();

    private:
    BlockingKineticConnection* blocking_connection_;
    NonblockingKineticConnection* nonblocking_connection_;
    DISALLOW_COPY_AND_ASSIGN(ConnectionHandle);
};

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_CONNECTION_HANDLE_H_
