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

#ifndef KINETIC_CPP_CLIENT_NONBLOCKING_PACKET_SERVICE_H_
#define KINETIC_CPP_CLIENT_NONBLOCKING_PACKET_SERVICE_H_

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
#include "nonblocking_packet_sender.h"

namespace kinetic {
using com::seagate::kinetic::client::proto::Message;
using com::seagate::kinetic::client::proto::Message_Status_StatusCode;

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
    HandlerKey Submit(unique_ptr<Message> message, const shared_ptr<const string> value,
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
