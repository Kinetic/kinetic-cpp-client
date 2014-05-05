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

#ifndef KINETIC_CPP_CLIENT_NONBLOCKING_PACKET_RECEIVER_H_
#define KINETIC_CPP_CLIENT_NONBLOCKING_PACKET_RECEIVER_H_

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

namespace kinetic {

using com::seagate::kinetic::client::proto::Message;
using com::seagate::kinetic::client::proto::Message_Status_StatusCode;

using std::string;
using std::unique_ptr;
using std::deque;
using std::pair;
using std::unordered_map;

StatusCode ConvertProtoStatus(Message_Status_StatusCode status);

enum NonblockingPacketServiceStatus {
    kIdle,      // nothing to do
    kIoWait,    // waiting for I/O to become possible
    kError      // irrecoverable error
};

class NonblockingReceiverInterface {
    public:
    virtual ~NonblockingReceiverInterface() {}
    // The HandlerKey must be unique across the lifespan of the receiver.
    virtual bool Enqueue(shared_ptr<HandlerInterface> handler, google::int64 sequence,
            HandlerKey handler_key) = 0;
    virtual NonblockingPacketServiceStatus Receive() = 0;
    virtual int64_t connection_id() = 0;
    virtual bool Remove(HandlerKey key) = 0;
};

class NonblockingReceiver : public NonblockingReceiverInterface {
    public:
    explicit NonblockingReceiver(shared_ptr<SocketWrapperInterface> socket_wrapper,
        HmacProvider hmac_provider, const ConnectionOptions &connection_options);
    ~NonblockingReceiver();
    bool Enqueue(shared_ptr<HandlerInterface> handler, google::int64 sequence,
            HandlerKey handler_key);
    NonblockingPacketServiceStatus Receive();
    int64_t connection_id();
    bool Remove(HandlerKey key);

    private:
    void CallAllErrorHandlers(KineticStatus error);

    shared_ptr<SocketWrapperInterface> socket_wrapper_;
    HmacProvider hmac_provider_;
    ConnectionOptions connection_options_;
    NonblockingPacketReader *nonblocking_response_;
    int64_t connection_id_;
    shared_ptr<HandlerInterface> handler_;
    Message response_;
    unique_ptr<const string> value_;
    unordered_map<google::protobuf::int64, pair<shared_ptr<HandlerInterface>, HandlerKey>> map_;
    // handler_key is separate from message sequence so that we don't tie handler identification
    // semantics to the message sequencing, since message sequence semantics are outside of our
    // control.
    unordered_map<HandlerKey, google::protobuf::int64> handler_to_message_seq_map_;
    DISALLOW_COPY_AND_ASSIGN(NonblockingReceiver);
};

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_NONBLOCKING_PACKET_RECEIVER_H_
