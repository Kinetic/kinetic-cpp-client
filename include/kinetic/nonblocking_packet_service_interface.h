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

#ifndef KINETIC_CPP_CLIENT_NONBLOCKING_PACKET_SERVICE_INTERFACE_H_
#define KINETIC_CPP_CLIENT_NONBLOCKING_PACKET_SERVICE_INTERFACE_H_

#include <sys/select.h>
#include <memory>

#include "kinetic/kinetic_status.h"
#include "kinetic_client.pb.h"

namespace kinetic {

using com::seagate::kinetic::client::proto::Message;

using std::shared_ptr;
using std::unique_ptr;
using std::string;

typedef uint64_t HandlerKey;

// Instances of this cannot be re-used for multiple requests as they are deleted after processing.
class HandlerInterface {
    public:
    virtual ~HandlerInterface() {}

    // response is re-used, so make sure to copy everything you need out of it
    virtual void Handle(const Message &response, unique_ptr<const string> value) = 0;
    virtual void Error(KineticStatus error) = 0;
};

class NonblockingPacketServiceInterface {
    public:
    virtual ~NonblockingPacketServiceInterface() {}
    // message is modified in this call hierarchy
    virtual HandlerKey Submit(unique_ptr<Message> message, const shared_ptr<const string> value,
            unique_ptr<HandlerInterface> handler) = 0;
    virtual bool Run(fd_set *read_fds, fd_set *write_fds, int *nfds) = 0;
    virtual bool Remove(HandlerKey handler_key) = 0;
};

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_NONBLOCKING_PACKET_SERVICE_INTERFACE_H_
