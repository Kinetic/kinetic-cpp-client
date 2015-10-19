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

#include <unistd.h>

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "kinetic/kinetic.h"
#include "mock_socket_wrapper_interface.h"
#include "matchers.h"
#include "kinetic_client.pb.h"
#include "nonblocking_packet.h"

namespace kinetic {

using std::make_shared;
using std::unique_ptr;
using std::move;
using std::string;
using ::testing::Return;

TEST(NonblockingPacketWriterTest, EmptyMessageAndValue) {
    int fds[2];
    ASSERT_EQ(0, pipe(fds));
    unique_ptr<Message> message(new Message());
    auto value = make_shared<string>("");
    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    EXPECT_CALL(*socket_wrapper, fd()).WillRepeatedly(Return(fds[1]));
    NonblockingPacketWriter request(socket_wrapper, move(message), move(value));
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
    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    EXPECT_CALL(*socket_wrapper, fd()).WillRepeatedly(Return(fds[0]));
    NonblockingPacketReader response(socket_wrapper, &message, value);
    ASSERT_EQ(kDone, response.Read());
    ASSERT_EQ("", *value);

    ASSERT_EQ(0, close(fds[0]));
}

}// namespace kinetic
