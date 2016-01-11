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

#ifndef KINETIC_CPP_CLIENT_NONBLOCKING_PACKET_SERVICE_INTERFACE_H_
#define KINETIC_CPP_CLIENT_NONBLOCKING_PACKET_SERVICE_INTERFACE_H_

#include <sys/select.h>
#include <memory>

#include "kinetic/kinetic_status.h"
#include "kinetic_client.pb.h"

namespace kinetic {

using com::seagate::kinetic::client::proto::Message;
using com::seagate::kinetic::client::proto::Command;

using std::shared_ptr;
using std::unique_ptr;
using std::string;

typedef uint64_t HandlerKey;

// Instances of this cannot be re-used for multiple requests as they are deleted after processing.
class HandlerInterface {
    public:
    virtual ~HandlerInterface() {}

    // response is re-used, so make sure to copy everything you need out of it
    virtual void Handle(const Command &response, unique_ptr<const string> value) = 0;
    virtual void Error(KineticStatus error, Command const * const response) = 0;
};

class NonblockingPacketServiceInterface {
    public:
    virtual ~NonblockingPacketServiceInterface() {}
    // message is modified in this call hierarchy
    virtual HandlerKey Submit(unique_ptr<Message> message, unique_ptr<Command> command,
            const shared_ptr<const string> value, unique_ptr<HandlerInterface> handler) = 0;
    virtual bool Run(fd_set *read_fds, fd_set *write_fds, int *nfds) = 0;
    virtual bool Remove(HandlerKey handler_key) = 0;
};

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_NONBLOCKING_PACKET_SERVICE_INTERFACE_H_
