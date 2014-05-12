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

#include "kinetic/threadsafe_blocking_kinetic_connection.h"
#include "kinetic/kinetic_connection_factory.h"
#include "kinetic/threadsafe_nonblocking_connection.h"
#include "socket_wrapper.h"
#include "nonblocking_packet_service.h"

namespace kinetic {

using palominolabs::protobufutil::MessageStreamFactory;

KineticConnectionFactory NewKineticConnectionFactory() {
    HmacProvider hmac_provider;
    return KineticConnectionFactory(hmac_provider);
}

KineticConnectionFactory::KineticConnectionFactory(
        HmacProvider hmac_provider)
    : hmac_provider_(hmac_provider) {}


Status KineticConnectionFactory::NewConnection(
        const ConnectionOptions &options,
        unsigned int network_timeout_seconds,
        unique_ptr<ConnectionHandle>& connection) {
    return doNewConnection(options, network_timeout_seconds, connection, false);
}

Status KineticConnectionFactory::NewThreadsafeConnection(
        const ConnectionOptions &options,
        unsigned int network_timeout_seconds,
        unique_ptr<ConnectionHandle>& connection) {
    return doNewConnection(options, network_timeout_seconds, connection, true);
}

Status KineticConnectionFactory::doNewConnection(ConnectionOptions const &options,
        unsigned int network_timeout_seconds,
        unique_ptr<ConnectionHandle>& connection, bool threadsafe) {
    auto socket_wrapper = make_shared<SocketWrapper>(options.host, options.port, true);

    if (!socket_wrapper->Connect()) {
        return Status::makeInternalError("Connection error");
    }

    auto receiver = shared_ptr<NonblockingReceiverInterface>(new NonblockingReceiver(socket_wrapper,
        hmac_provider_, options));
    auto writer_factory =
        unique_ptr<NonblockingPacketWriterFactoryInterface>(new NonblockingPacketWriterFactory());
    auto sender = unique_ptr<NonblockingSenderInterface>(new NonblockingSender(socket_wrapper,
        receiver, move(writer_factory), hmac_provider_, options));

    NonblockingPacketService *service =
        new NonblockingPacketService(socket_wrapper, move(sender), receiver);

    if (threadsafe) {
        NonblockingKineticConnection* nonblocking_connection =
                new ThreadsafeNonblockingKineticConnection(service);
        ThreadsafeBlockingKineticConnection* blocking_connection =
                new ThreadsafeBlockingKineticConnection(nonblocking_connection, network_timeout_seconds);
        connection.reset(new ConnectionHandle(blocking_connection, nonblocking_connection));
    } else {
        NonblockingKineticConnection* nonblocking_connection =
                new NonblockingKineticConnection(service);
        BlockingKineticConnection* blocking_connection =
                new BlockingKineticConnection(nonblocking_connection, network_timeout_seconds);
        connection.reset(new ConnectionHandle(blocking_connection, nonblocking_connection));
    }


    return Status::makeOk();
}
} // namespace kinetic
