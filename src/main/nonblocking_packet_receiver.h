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
using com::seagate::kinetic::client::proto::Command;
using com::seagate::kinetic::client::proto::Command_Status_StatusCode;

using std::string;
using std::unique_ptr;
using std::deque;
using std::pair;
using std::unordered_map;

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
    Message message_;
    Command command_;
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
