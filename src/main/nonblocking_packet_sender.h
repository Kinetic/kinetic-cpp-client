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

#ifndef KINETIC_CPP_CLIENT_NONBLOCKING_PACKET_SENDER_H_
#define KINETIC_CPP_CLIENT_NONBLOCKING_PACKET_SENDER_H_

#include <sys/select.h>
#include <cstdint>

#include <queue>
#include <unordered_map>
#include <glog/logging.h>

#include "gmock/gmock.h"

#include "kinetic/nonblocking_packet_service_interface.h"
#include "kinetic/connection_options.h"
#include "kinetic/hmac_provider.h"
#include "kinetic_client.pb.h"
#include "nonblocking_packet.h"
#include "socket_wrapper_interface.h"
#include "nonblocking_packet_receiver.h"

namespace kinetic {

using com::seagate::kinetic::client::proto::Message;
using com::seagate::kinetic::client::proto::Message_Status_StatusCode;

using std::string;
using std::unique_ptr;
using std::deque;
using std::pair;
using std::unordered_map;

class NonblockingSenderInterface {
    public:
    virtual ~NonblockingSenderInterface() {}
    // The HandlerKey returned will be unique for the lifespan of the Sender instance.
    virtual void Enqueue(unique_ptr<Message> message, const shared_ptr<const string> value,
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
    void Enqueue(unique_ptr<Message> message, const shared_ptr<const string> value,
            unique_ptr<HandlerInterface> handler, HandlerKey handler_key);
    NonblockingPacketServiceStatus Send();
    bool Remove(HandlerKey key);

    private:
    struct Request {
        unique_ptr<const Message> message;
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
