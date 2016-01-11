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

HandlerKey NonblockingPacketService::Submit(unique_ptr<Message> message, unique_ptr<Command> command,
        const shared_ptr<const string> value, unique_ptr<HandlerInterface> handler) {
    HandlerKey key = next_key_++;

    if (failed_) {
        handler->Error(
                KineticStatus(StatusCode::CLIENT_SHUTDOWN, "Client already shut down"), nullptr);
    } else {
        sender_->Enqueue(move(message), move(command), value, move(handler), key);
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
