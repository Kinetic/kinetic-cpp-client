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

#ifndef KINETIC_CPP_CLIENT_NONBLOCKING_PACKET_SENDER_H_
#define KINETIC_CPP_CLIENT_NONBLOCKING_PACKET_SENDER_H_

#include <sys/select.h>
#include <cstdint>

#include <queue>
#include <unordered_map>
#include <glog/logging.h>

#include "kinetic/nonblocking_packet_service_interface.h"
#include "kinetic/connection_options.h"
#include "kinetic/hmac_provider.h"
#include "kinetic_client.pb.h"
#include "nonblocking_packet.h"
#include "socket_wrapper_interface.h"
#include "nonblocking_packet_receiver.h"

namespace kinetic {

using com::seagate::kinetic::client::proto::Message;
using com::seagate::kinetic::client::proto::Command;
using com::seagate::kinetic::client::proto::Command_Status_StatusCode;

using std::string;
using std::unique_ptr;
using std::deque;
using std::pair;
using std::unordered_map;

class NonblockingSenderInterface {
    public:
    virtual ~NonblockingSenderInterface() {}
    // The HandlerKey returned will be unique for the lifespan of the Sender instance.
    virtual void Enqueue(unique_ptr<Message> message, unique_ptr<Command> command, const shared_ptr<const string> value,
            unique_ptr<HandlerInterface> handler, HandlerKey handler_key) = 0;
    virtual NonblockingPacketServiceStatus Send() = 0;
    // remove the handler if it hasn't already started being processed. Returns true if a handler
    // actually was removed.
    virtual bool Remove(HandlerKey key) = 0;
};

class NonblockingSender : public NonblockingSenderInterface {
    public:
    NonblockingSender(shared_ptr<SocketWrapperInterface> socket_wrapper,
        shared_ptr<NonblockingReceiverInterface> receiver,
        shared_ptr<NonblockingPacketWriterFactoryInterface> packet_writer_factory,
        HmacProvider hmac_provider, const ConnectionOptions &connection_options);
    ~NonblockingSender();
    void Enqueue(unique_ptr<Message> message, unique_ptr<Command> command, const shared_ptr<const string> value,
            unique_ptr<HandlerInterface> handler, HandlerKey handler_key);
    NonblockingPacketServiceStatus Send();
    bool Remove(HandlerKey key);

    private:
    struct Request {
        unique_ptr<const Message> message;
        unique_ptr<const Command> command;
        shared_ptr<const string> value;
        unique_ptr<HandlerInterface> handler;
        HandlerKey handler_key;
    };

    shared_ptr<SocketWrapperInterface> socket_wrapper_;
    shared_ptr<NonblockingReceiverInterface> receiver_;
    shared_ptr<NonblockingPacketWriterFactoryInterface> packet_writer_factory_;
    HmacProvider hmac_provider_;
    ConnectionOptions connection_options_;
    int64_t sequence_number_;
    HandlerKey handler_key_;
    unique_ptr<NonblockingPacketWriterInterface> current_writer_;
    shared_ptr<HandlerInterface> handler_;
    deque<unique_ptr<Request>> request_queue_;
    google::int64 message_sequence_;
    DISALLOW_COPY_AND_ASSIGN(NonblockingSender);
};


} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_NONBLOCKING_PACKET_SENDER_H_
