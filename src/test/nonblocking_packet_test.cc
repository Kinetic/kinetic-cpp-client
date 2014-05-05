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

#include <unistd.h>

#include "gtest/gtest.h"

#include "kinetic_client.pb.h"
#include "nonblocking_packet.h"

namespace kinetic {

using std::make_shared;
using std::unique_ptr;
using std::move;
using std::string;

TEST(NonblockingPacketWriterTest, EmptyMessageAndValue) {
    int fds[2];
    ASSERT_EQ(0, pipe(fds));
    unique_ptr<Message> message(new Message());
    auto value = make_shared<string>("");
    NonblockingPacketWriter request(fds[1], move(message), move(value));
    ASSERT_EQ(kDone, request.Write());
    ASSERT_EQ(0, close(fds[1]));

    char header[10];
    // We attempt to read 10 bytes but we only expect there to be 9
    ASSERT_EQ(9, read(fds[0], header, sizeof(header)));
    ASSERT_EQ('F', header[0]);  // magic byte
    // Message length
    ASSERT_EQ(0u, *reinterpret_cast<uint32_t *>(header + 1));
    // Value length
    ASSERT_EQ(0u, *reinterpret_cast<uint32_t *>(header + 5));

    ASSERT_EQ(0, close(fds[0]));
}

TEST(NonblockingPacketReaderTest, EmptyMessageAndValue) {
    // Create a pipe and write the 9-byte header into it
    int fds[2];
    ASSERT_EQ(0, pipe(fds));
    char header[9] = { 'F', 0, 0, 0, 0, 0, 0, 0, 0 };
    ASSERT_EQ(9, write(fds[1], header, sizeof(header)));
    ASSERT_EQ(0, close(fds[1]));

    // We should now be able to read it using a NonblockingPacketReader
    Message message;
    unique_ptr<const string> value;
    NonblockingPacketReader response(fds[0], &message, value);
    ASSERT_EQ(kDone, response.Read());
    ASSERT_EQ("", *value);

    ASSERT_EQ(0, close(fds[0]));
}

}  // namespace kinetic
