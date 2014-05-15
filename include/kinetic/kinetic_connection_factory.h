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

#ifndef KINETIC_CPP_CLIENT_KINETIC_CONNECTION_FACTORY_H_
#define KINETIC_CPP_CLIENT_KINETIC_CONNECTION_FACTORY_H_

#include "protobufutil/message_stream.h"

#include "kinetic/connection_options.h"
#include "kinetic/hmac_provider.h"
#include "kinetic/blocking_kinetic_connection.h"
#include "kinetic/nonblocking_kinetic_connection.h"
#include "kinetic/threadsafe_nonblocking_connection.h"
#include "kinetic/threadsafe_blocking_kinetic_connection.h"
#include "kinetic/status.h"

namespace kinetic {

using palominolabs::protobufutil::MessageStreamFactoryInterface;
using std::unique_ptr;

/// Factory class that builds KineticConnection instances. Rather than use the constructor
/// developers should use NewKineticConnectionFactory.
class KineticConnectionFactory {
    public:
    explicit KineticConnectionFactory(HmacProvider hmac_provider);

    /// Creates and opens a new nonblocking connection using the given options. If the returned
    /// Status indicates success then the connection is ready to perform
    /// actions and the caller should delete it when done using it. If the
    /// Status indicates failure, then no connection will be created and
    /// the caller must not attempt to use or delete it.
    ///
    /// @param[in] options                  Specifies host, port, user id, etc
    /// @param[in] network_timeout_seconds  If an operation goes more than this many seconds without
    ///                                     data the operation fails
    /// @param[out] connection              Populated with a NonblockingKineticConnection if the request
    ///                                     succeeds
    virtual Status NewNonblockingConnection(
            const ConnectionOptions& options,
            unique_ptr <NonblockingKineticConnection>& connection);

    virtual Status NewNonblockingConnection(
            const ConnectionOptions& options,
            shared_ptr <NonblockingKineticConnection>& connection);

    /// Like NewNonblockingConnection, except the connection is safe for use by multiple threads.
    virtual Status NewThreadsafeNonblockingConnection(
            const ConnectionOptions& options,
            unique_ptr <NonblockingKineticConnection>& connection);

    virtual Status NewThreadsafeNonblockingConnection(
            const ConnectionOptions& options,
            shared_ptr <NonblockingKineticConnection>& connection);

    /// Creates and opens a new blocking connection using the given options. If the returned
    /// Status indicates success then the connection is ready to perform
    /// actions and the caller should delete it when done using it. If the
    /// Status indicates failure, then no connection will be created and
    /// the caller must not attempt to use or delete it.
    ///
    /// @param[in] options                  Specifies host, port, user id, etc
    /// @param[in] network_timeout_seconds  If an operation goes more than this many seconds without
    ///                                     data the operation fails
    /// @param[out] connection              Populated with a BlockingKineticConnection if the request
    ///                                     succeeds
    virtual Status NewBlockingConnection(
            const ConnectionOptions& options,
            unique_ptr <BlockingKineticConnection>& connection,
            unsigned int network_timeout_seconds);

    virtual Status NewBlockingConnection(
            const ConnectionOptions& options,
            shared_ptr <BlockingKineticConnection>& connection,
            unsigned int network_timeout_seconds);

    /// Like NewBlockingConnection, except the connection is safe for use by multiple threads
    virtual Status NewThreadsafeBlockingConnection(
            const ConnectionOptions& options,
            unique_ptr <BlockingKineticConnection>& connection,
            unsigned int network_timeout_seconds);

    virtual Status NewThreadsafeBlockingConnection(
            const ConnectionOptions& options,
            shared_ptr <BlockingKineticConnection>& connection,
            unsigned int network_timeout_seconds);

    private:
    HmacProvider hmac_provider_;
    Status doNewConnection(
            ConnectionOptions const& options,
            unique_ptr <NonblockingKineticConnection>& connection,
            bool threadsafe);
};

/// Helper method that creates a new KineticConnectionFactory with
/// reasonable defaults
KineticConnectionFactory NewKineticConnectionFactory();

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_KINETIC_CONNECTION_FACTORY_H_
