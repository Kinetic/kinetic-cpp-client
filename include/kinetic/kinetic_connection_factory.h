/**
 * Copyright 2013-2015 Seagate Technology LLC.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at
 * https://mozilla.org/MP:/2.0/.
 *
 * This program is distributed in the hope that it will be useful,
 * but is provided AS-IS, WITHOUT ANY WARRANTY; including without
 * the implied warranty of MERCHANTABILITY, NON-INFRINGEMENT or
 * FITNESS FOR A PARTICULAR PURPOSE. See the Mozilla Public
 * License for more details.
 *
 * See www.openkinetic.org for more project information
 */

#ifndef KINETIC_CPP_CLIENT_KINETIC_CONNECTION_FACTORY_H_
#define KINETIC_CPP_CLIENT_KINETIC_CONNECTION_FACTORY_H_

#include "kinetic/connection_options.h"
#include "kinetic/hmac_provider.h"
#include "kinetic/blocking_kinetic_connection.h"
#include "kinetic/nonblocking_kinetic_connection.h"
#include "kinetic/threadsafe_nonblocking_connection.h"
#include "kinetic/threadsafe_blocking_kinetic_connection.h"
#include "kinetic/status.h"
#include <memory>

namespace kinetic {

using std::unique_ptr;

/// Factory class that builds KineticConnection instances. Rather than use the constructor
/// developers should use NewKineticConnectionFactory.
class KineticConnectionFactory {
    public:
    explicit KineticConnectionFactory(HmacProvider hmac_provider);
    virtual ~KineticConnectionFactory() {}

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
            unique_ptr <ThreadsafeNonblockingKineticConnection>& connection);

    virtual Status NewThreadsafeNonblockingConnection(
            const ConnectionOptions& options,
            shared_ptr <ThreadsafeNonblockingKineticConnection>& connection);

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
            unique_ptr <ThreadsafeBlockingKineticConnection>& connection,
            unsigned int network_timeout_seconds);

    virtual Status NewThreadsafeBlockingConnection(
            const ConnectionOptions& options,
            shared_ptr <ThreadsafeBlockingKineticConnection>& connection,
            unsigned int network_timeout_seconds);


    private:
    HmacProvider hmac_provider_;
    Status doNewConnection(
            ConnectionOptions const& options,
            unique_ptr <NonblockingKineticConnection>& connection);
};

/// Helper method that creates a new KineticConnectionFactory with
/// reasonable defaults
KineticConnectionFactory NewKineticConnectionFactory();

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_KINETIC_CONNECTION_FACTORY_H_
