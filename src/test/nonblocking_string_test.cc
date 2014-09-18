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

#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "kinetic/kinetic.h"
#include "nonblocking_string.h"
#include "mock_socket_wrapper_interface.h"
#include "matchers.h"


namespace kinetic {

using std::make_shared;
using std::string;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::StrictMock;

static void MakeNonblockingPipe(int *fds) {
    ASSERT_EQ(0, pipe(fds));
    ASSERT_EQ(0, fcntl(fds[0], F_SETFL, O_NONBLOCK));
    ASSERT_EQ(0, fcntl(fds[1], F_SETFL, O_NONBLOCK));
}

class NonblockingStringReaderTest : public ::testing::Test {
    protected:
    NonblockingStringReaderTest() : closed_write_end_(false) {}

    void SetUp() {
        MakeNonblockingPipe(fds_);
        socket_wrapper = make_shared<MockSocketWrapperInterface>();
        EXPECT_CALL(*socket_wrapper, fd()).WillRepeatedly(Return(fds_[0]));
    }

    void TearDown() {
        ASSERT_EQ(0, close(fds_[0]));
        if (!closed_write_end_) {
            ASSERT_EQ(0, close(fds_[1]));
        }
    }

    int fds_[2];
    shared_ptr<MockSocketWrapperInterface> socket_wrapper;
    bool closed_write_end_;
};

TEST_F(NonblockingStringReaderTest, ReadsZeroLengthString) {
    unique_ptr<const string> output;
    NonblockingStringReader reader(socket_wrapper, 0, output);
    ASSERT_EQ(kDone, reader.Read());
    ASSERT_EQ("", *output);
}

TEST_F(NonblockingStringReaderTest, ReadsStringOneByteAtATime) {
    unique_ptr<const string> output;
    NonblockingStringReader reader(socket_wrapper, 3, output);

    ASSERT_EQ(1, write(fds_[1], "a", 1));
    ASSERT_EQ(kInProgress, reader.Read());

    ASSERT_EQ(1, write(fds_[1], "b", 1));
    ASSERT_EQ(kInProgress, reader.Read());

    ASSERT_EQ(1, write(fds_[1], "c", 1));
    ASSERT_EQ(kDone, reader.Read());
    ASSERT_EQ("abc", *output);
}

TEST_F(NonblockingStringReaderTest, ReadsStringAllAtOnce) {
    std::string input("abc");
    unique_ptr<const string> output;
    ASSERT_EQ(static_cast<int>(input.size()), write(fds_[1], input.data(), input.size()));
    NonblockingStringReader reader(socket_wrapper, input.size(), output);
    ASSERT_EQ(kDone, reader.Read());
    ASSERT_EQ(input, *output);
}

TEST_F(NonblockingStringReaderTest, ReturnsErrorOnPrematureEof) {
    unique_ptr<const string> output;
    NonblockingStringReader reader(socket_wrapper, 1, output);

    // Close the write end early and verify that we get an error
    ASSERT_EQ(0, close(fds_[1]));
    closed_write_end_ = true;
    ASSERT_EQ(kFailed, reader.Read());
}

class NonblockingStringWriterTest : public ::testing::Test {
    protected:
    NonblockingStringWriterTest() : closed_read_end_(false) {}

    void SetUp() {
        MakeNonblockingPipe(fds_);
        socket_wrapper = make_shared<MockSocketWrapperInterface>();
        EXPECT_CALL(*socket_wrapper, fd()).WillRepeatedly(Return(fds_[1]));
    }

    void TearDown() {
        if (!closed_read_end_) {
            ASSERT_EQ(0, close(fds_[0]));
        }
        ASSERT_EQ(0, close(fds_[1]));
    }

    int fds_[2];
    shared_ptr<MockSocketWrapperInterface> socket_wrapper;
    bool closed_read_end_;
};

TEST_F(NonblockingStringWriterTest, WritesZeroLengthString) {
    auto s = make_shared<string>("");
    NonblockingStringWriter writer(socket_wrapper, s);
    ASSERT_EQ(kDone, writer.Write());

    // Verify that there's nothing to be read from the other end
    ASSERT_EQ(0, fcntl(fds_[0], F_SETFL, O_NONBLOCK));
    char c;
    ASSERT_EQ(-1, read(fds_[0], &c, 1));
    ASSERT_EQ(EAGAIN, errno);
}

TEST_F(NonblockingStringWriterTest, WritesStringAllAtOnce) {
    // A write of "abc" should succeed immediately without blocking
    auto s = make_shared<string>("abc");
    NonblockingStringWriter writer(socket_wrapper, s);
    ASSERT_EQ(kDone, writer.Write());

    char result[3];
    ASSERT_EQ(3, read(fds_[0], result, sizeof(result)));
    ASSERT_EQ('a', result[0]);
    ASSERT_EQ('b', result[1]);
    ASSERT_EQ('c', result[2]);
}

TEST_F(NonblockingStringWriterTest, ReturnsErrorIfWriteFails) {
    // Close the read end of the pipe and verify that subsequent writes fail
    ASSERT_EQ(0, close(fds_[0]));
    closed_read_end_ = true;

    auto s = make_shared<string>("abc");
    NonblockingStringWriter writer(socket_wrapper, s);
    ASSERT_EQ(kFailed, writer.Write());
}

}  // namespace kinetic
