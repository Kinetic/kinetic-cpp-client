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

#ifndef KINETIC_CPP_CLIENT_NONBLOCKING_PACKET_SERVICE_H_
#define KINETIC_CPP_CLIENT_NONBLOCKING_PACKET_SERVICE_H_

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
#include "nonblocking_packet_sender.h"

namespace kinetic {
using com::seagate::kinetic::client::proto::Message;
using com::seagate::kinetic::client::proto::Command;
using com::seagate::kinetic::client::proto::Command_Status_StatusCode;

using std::string;
using std::unique_ptr;
using std::deque;
using std::pair;
using std::unordered_map;

class NonblockingPacketService : public NonblockingPacketServiceInterface {
    public:
    NonblockingPacketService(shared_ptr<SocketWrapperInterface> socket_wrapper,
        unique_ptr<NonblockingSenderInterface> sender,
        shared_ptr<NonblockingReceiverInterface> receiver);
    ~NonblockingPacketService();
    // handler instances cannot be reused
    HandlerKey Submit(unique_ptr<Message> message, unique_ptr<Command> command, const shared_ptr<const string> value,
        unique_ptr<HandlerInterface> handler);
    bool Run(fd_set *read_fds, fd_set *write_fds, int *nfds);
    bool Remove(HandlerKey handler_key);

    private:
    shared_ptr<SocketWrapperInterface> socket_wrapper_;
    unique_ptr<NonblockingSenderInterface> sender_;
    shared_ptr<NonblockingReceiverInterface> receiver_;
    bool failed_;
    HandlerKey next_key_;
    void CleanUp();
    DISALLOW_COPY_AND_ASSIGN(NonblockingPacketService);
};

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_NONBLOCKING_PACKET_SERVICE_H_
