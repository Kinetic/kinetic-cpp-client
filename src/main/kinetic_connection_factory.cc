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

#include "kinetic/kinetic_connection_factory.h"
#include "socket_wrapper.h"
#include "nonblocking_packet_service.h"
#include <exception>
#include <stdexcept>


namespace kinetic {

KineticConnectionFactory NewKineticConnectionFactory() {
    HmacProvider hmac_provider;
    return KineticConnectionFactory(hmac_provider);
}

KineticConnectionFactory::KineticConnectionFactory(
        HmacProvider hmac_provider)
    : hmac_provider_(hmac_provider) {}


Status KineticConnectionFactory::NewNonblockingConnection(
        const ConnectionOptions& options,
        unique_ptr<NonblockingKineticConnection>& connection) {
    return doNewConnection(options, connection);
}

Status KineticConnectionFactory::NewNonblockingConnection(
        const ConnectionOptions& options,
        shared_ptr<NonblockingKineticConnection>& connection) {
    unique_ptr<NonblockingKineticConnection> nbc;
    Status status = doNewConnection(options, nbc);
    if (status.ok())
        connection.reset(nbc.release());
    return status;
}

Status KineticConnectionFactory::NewThreadsafeNonblockingConnection(
        const ConnectionOptions& options,
        unique_ptr<ThreadsafeNonblockingKineticConnection>& connection) {
    unique_ptr<NonblockingKineticConnection> nbc;
    Status status = doNewConnection(options, nbc);
    if(status.ok())
        connection.reset(new ThreadsafeNonblockingKineticConnection(std::move(nbc)));
    return status;
}

Status KineticConnectionFactory::NewThreadsafeNonblockingConnection(
        const ConnectionOptions& options,
        shared_ptr<ThreadsafeNonblockingKineticConnection>& connection) {
    unique_ptr<NonblockingKineticConnection> nbc;
    Status status = doNewConnection(options, nbc);
    if(status.ok())
        connection.reset(new ThreadsafeNonblockingKineticConnection(std::move(nbc)));
    return status;
}

Status KineticConnectionFactory::NewBlockingConnection(
        const ConnectionOptions& options,
        unique_ptr<BlockingKineticConnection>& connection,
        unsigned int network_timeout_seconds) {
    unique_ptr<NonblockingKineticConnection> nbc;
    Status status = doNewConnection(options, nbc);
    if(status.ok())
       connection.reset(new BlockingKineticConnection(std::move(nbc), network_timeout_seconds));
    return status;
}

Status KineticConnectionFactory::NewBlockingConnection(
        const ConnectionOptions& options,
        shared_ptr<BlockingKineticConnection>& connection,
        unsigned int network_timeout_seconds) {
    unique_ptr<NonblockingKineticConnection> nbc;
    Status status = doNewConnection(options, nbc);
    if(status.ok())
       connection.reset(new BlockingKineticConnection(std::move(nbc), network_timeout_seconds));
    return status;
}

Status KineticConnectionFactory::NewThreadsafeBlockingConnection(
        const ConnectionOptions& options,
        unique_ptr<ThreadsafeBlockingKineticConnection>& connection,
        unsigned int network_timeout_seconds) {
    unique_ptr<BlockingKineticConnection> bc;
    Status status = NewBlockingConnection(options, bc, network_timeout_seconds);
    if(status.ok())
       connection.reset(new ThreadsafeBlockingKineticConnection(std::move(bc)));
    return status;
}

Status KineticConnectionFactory::NewThreadsafeBlockingConnection(
        const ConnectionOptions& options,
        shared_ptr<ThreadsafeBlockingKineticConnection>& connection,
        unsigned int network_timeout_seconds) {
    unique_ptr<BlockingKineticConnection> bc;
    Status status = NewBlockingConnection(options, bc, network_timeout_seconds);
    if(status.ok())
       connection.reset(new ThreadsafeBlockingKineticConnection(std::move(bc)));
    return status;
}

Status KineticConnectionFactory::doNewConnection(
        ConnectionOptions const& options,
        unique_ptr <NonblockingKineticConnection>& connection) {
    try{
        auto socket_wrapper = make_shared<SocketWrapper>(options.host, options.port, options.use_ssl, true);
        if (!socket_wrapper->Connect())
            throw std::runtime_error("Could not connect to socket.");

        shared_ptr<NonblockingReceiverInterface> receiver;
        receiver = shared_ptr<NonblockingReceiverInterface>(new NonblockingReceiver(socket_wrapper, hmac_provider_, options));

        auto writer_factory =
            shared_ptr<NonblockingPacketWriterFactoryInterface>(new NonblockingPacketWriterFactory());
        auto sender = unique_ptr<NonblockingSenderInterface>(new NonblockingSender(socket_wrapper,
                                                                                   receiver,
                                                                                   writer_factory,
                                                                                   hmac_provider_,
                                                                                   options));

        NonblockingPacketService *service = new NonblockingPacketService(socket_wrapper, move(sender), receiver);
        connection.reset(new NonblockingKineticConnection(service));

    } catch(std::exception& e){
           return Status::makeInternalError("Connection error: "+std::string(e.what()));
    }
    return Status::makeOk();
}
} // namespace kinetic
