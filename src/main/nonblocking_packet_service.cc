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

#include "nonblocking_packet_service.h"

namespace kinetic {

using std::string;
using std::shared_ptr;
using std::unique_ptr;
using std::move;
using std::make_pair;

NonblockingPacketService::NonblockingPacketService(
        shared_ptr<SocketWrapperInterface> socket_wrapper,
        unique_ptr<NonblockingSenderInterface> sender,
        shared_ptr<NonblockingReceiverInterface> receiver)
    : socket_wrapper_(socket_wrapper), sender_(move(sender)), receiver_(receiver),
        failed_(false), next_key_(0) {}

NonblockingPacketService::~NonblockingPacketService() {
    CleanUp();
}

HandlerKey NonblockingPacketService::Submit(unique_ptr<Message> message,
        const shared_ptr<const string> value, unique_ptr<HandlerInterface> handler) {
    HandlerKey key = next_key_++;

    if (failed_) {
        handler->Error(KineticStatus(StatusCode::CLIENT_SHUTDOWN, "Client already shut down"));
    } else {
        sender_->Enqueue(move(message), value, move(handler), key);
    }

    return key;
}

bool NonblockingPacketService::Run(fd_set *read_fds, fd_set *write_fds, int *nfds) {
    if (failed_) {
        return false;
    }
    NonblockingPacketServiceStatus sender_status = sender_->Send();
    if (sender_status == kError) {
        CleanUp();
        return false;
    }
    NonblockingPacketServiceStatus receiver_status = receiver_->Receive();
    if (receiver_status == kError) {
        CleanUp();
        return false;
    }
    FD_ZERO(read_fds);
    FD_ZERO(write_fds);
    *nfds = 0;
    if (sender_status == kIoWait) {
        FD_SET(socket_wrapper_->fd(), write_fds);
        *nfds = socket_wrapper_->fd() + 1;
    }
    if (receiver_status == kIoWait) {
        FD_SET(socket_wrapper_->fd(), read_fds);
        *nfds = socket_wrapper_->fd() + 1;
    }
    return true;
}

// Free all allocated resources and mark the service as having encountered an
// irrecoverable error. This function exists so that in the event of an error
// we can close the connection immediately instead of leaving it open until the
// destructor is called.
void NonblockingPacketService::CleanUp() {
    failed_ = true;
}

bool NonblockingPacketService::Remove(HandlerKey handler_key) {
    return sender_->Remove(handler_key) || receiver_->Remove(handler_key);
}


} // namespace kinetic
