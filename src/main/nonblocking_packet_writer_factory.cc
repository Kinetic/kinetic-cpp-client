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

#include "nonblocking_packet.h"

namespace kinetic {

using std::make_shared;
using std::shared_ptr;
using std::unique_ptr;
using std::move;
using std::string;

unique_ptr<NonblockingPacketWriterInterface> NonblockingPacketWriterFactory::CreateWriter(
        shared_ptr<SocketWrapperInterface> socket_wrapper,
        unique_ptr<const Message> message,
        const shared_ptr<const string> value) {
    return unique_ptr<NonblockingPacketWriterInterface>(
            new NonblockingPacketWriter(socket_wrapper, move(message), value));
}

} // namespace kinetic
