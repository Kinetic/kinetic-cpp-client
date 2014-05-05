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

#include "nonblocking_packet.h"

namespace kinetic {

using std::make_shared;
using std::shared_ptr;
using std::unique_ptr;
using std::move;
using std::string;

unique_ptr<NonblockingPacketWriterInterface> NonblockingPacketWriterFactory::CreateWriter(int fd,
    unique_ptr<const Message> message, const shared_ptr<const string> value) {
    return
        unique_ptr<NonblockingPacketWriterInterface>(
            new NonblockingPacketWriter(fd, move(message), value));
}

} // namespace kinetic
