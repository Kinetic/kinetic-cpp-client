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

#include "gtest/gtest.h"

#include <unistd.h>

#include <string>

#include "kinetic/incoming_value.h"
#include "kinetic/outgoing_value.h"

namespace kinetic {

TEST(StringValueTest, TransferToFileWorks) {
    // Write IncomingStringValue into a pipe
    IncomingStringValue value("a");
    int output_pipe[2];
    ASSERT_EQ(0, pipe(output_pipe));
    ASSERT_TRUE(value.TransferToFile(output_pipe[1]));

    // Check that the pipe contains the right value
    char result;
    ASSERT_EQ(1, read(output_pipe[0], &result, 1));
    ASSERT_EQ('a', result);

    ASSERT_EQ(0, close(output_pipe[0]));
    ASSERT_EQ(0, close(output_pipe[1]));
}

TEST(StringValueTest, ToStringWorks) {
    IncomingStringValue value("a");
    std::string result;
    ASSERT_TRUE(value.ToString(&result));
    ASSERT_EQ("a", result);
}

TEST(StringValueTest, ToStringDepletesObject) {
    IncomingStringValue value("a");
    std::string result;
    ASSERT_TRUE(value.ToString(&result));
    ASSERT_FALSE(value.ToString(&result));
}

TEST(StringValueTest, TransferToFileDepletesObject) {
    // Write IncomingStringValue into a pipe
    IncomingStringValue value("a");
    int output_pipe[2];
    ASSERT_EQ(0, pipe(output_pipe));
    ASSERT_TRUE(value.TransferToFile(output_pipe[1]));

    // Verify that further operations fail
    std::string result;
    ASSERT_FALSE(value.ToString(&result));

    ASSERT_EQ(0, close(output_pipe[0]));
    ASSERT_EQ(0, close(output_pipe[1]));
}

TEST(StringValueTest, ConsumeDepletesObject) {
    IncomingStringValue value("a");
    value.Consume();
    std::string result;
    ASSERT_FALSE(value.ToString(&result));
}

TEST(OutgoingStringValueTest, SizeWorks) {
    OutgoingStringValue value("abc");
    ASSERT_EQ(3u, value.size());
}

TEST(OutgoingStringValueTest, TransferToSocketWorks) {
    // Write OutgoingStringValue into a pipe
    OutgoingStringValue value("a");
    int output_pipe[2];
    int err;
    ASSERT_EQ(0, pipe(output_pipe));
    ASSERT_TRUE(value.TransferToSocket(output_pipe[1], &err));

    // Check that the pipe contains the right value
    char result;
    ASSERT_EQ(1, read(output_pipe[0], &result, 1));
    ASSERT_EQ('a', result);

    ASSERT_EQ(0, close(output_pipe[0]));
    ASSERT_EQ(0, close(output_pipe[1]));
}

TEST(OutgoingStringValueTest, ToStringWorks) {
    OutgoingStringValue value("abc");
    std::string s;
    int err;
    ASSERT_TRUE(value.ToString(&s, &err));
    ASSERT_EQ("abc", s);
}

} // namespace kinetic
