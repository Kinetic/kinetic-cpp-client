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

#include <arpa/inet.h>
#include <string.h>

#include "google/protobuf/descriptor.pb.h"
#include "gtest/gtest.h"

#include "kinetic/common.h"
#include "kinetic/incoming_value.h"
#include "kinetic/message_stream.h"
#include "kinetic/outgoing_value.h"

namespace kinetic {

using ::google::protobuf::FileDescriptorProto;

class ArrayByteStream : public ByteStreamInterface {
    public:
    explicit ArrayByteStream(
        const void* input_data,
        void* output_data) :
            input_data_(input_data), output_data_(output_data),
            input_index_(0), output_index_(0) {}

    bool Read(void *buf, size_t n) {
        memcpy(buf, reinterpret_cast<const char *>(input_data_) + input_index_, n);
        input_index_ += n;
        return true;
    }

    bool Write(const void *buf, size_t n) {
        memcpy(reinterpret_cast<char *>(output_data_) + output_index_, buf, n);
        output_index_ += n;
        return true;
    }

    IncomingValueInterface *ReadValue(size_t n) {
        char *buf = new char[n];
        if (!Read(buf, n)) {
            delete[] buf;
            return NULL;
        }
        std::string value(buf, n);
        delete[] buf;
        return new IncomingStringValue(value);
    }

    bool WriteValue(const OutgoingValueInterface &value, int* err) {
        std::string s;
        if (!value.ToString(&s, err)) {
            return false;
        }
        return Write(s.data(), s.size());
    }

    private:
    const void *input_data_;
    void *output_data_;
    size_t input_index_;
    size_t output_index_;
    DISALLOW_COPY_AND_ASSIGN(ArrayByteStream);
};

class MessageStreamTest : public ::testing::Test {
    protected:
    MessageStreamTest() {}
    // MessageStream works with any kind of protocol buffer, so let's use
    // FileDescriptorProto since that is readily available.
    FileDescriptorProto message_;
};

TEST_F(MessageStreamTest, ReadMessageReturnsFalseOnInvalidMagic) {
    uint8_t data[] = {'o', 'o', 'o'};
    uint8_t buffer[1024];
    ArrayByteStream* byte_stream = new ArrayByteStream(data, buffer);
    MessageStream message_stream(1000, byte_stream);

    IncomingValueInterface* value;
    ASSERT_EQ(MessageStream::MessageStreamReadStatus_INTERNAL_ERROR,
        message_stream.ReadMessage(&message_, &value));
}

TEST_F(MessageStreamTest, ReadMessageParsesFrameWithEmptyValue) {
    uint8_t data[] = {'F', 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t buffer[1024];
    ArrayByteStream* byte_stream = new ArrayByteStream(data, buffer);
    MessageStream message_stream(1000, byte_stream);

    IncomingValueInterface* value = NULL;
    ASSERT_EQ(MessageStream::MessageStreamReadStatus_SUCCESS,
        message_stream.ReadMessage(&message_, &value));

    ASSERT_TRUE(value != NULL);

    delete value;
}

TEST_F(MessageStreamTest, ReadMessageParsesFrameWithNonemptyValue) {
    uint8_t data[] = {'F', 0, 0, 0, 0, 0, 0, 0, 3, 'H', 'a', 'i'};
    uint8_t buffer[1024];
    ArrayByteStream* byte_stream = new ArrayByteStream(data, buffer);
    MessageStream message_stream(1000, byte_stream);

    IncomingValueInterface* value = NULL;
    ASSERT_EQ(MessageStream::MessageStreamReadStatus_SUCCESS,
        message_stream.ReadMessage(&message_, &value));
    ASSERT_NE(value, (IncomingValueInterface*)NULL);

    std::string value_string;
    ASSERT_TRUE(value->ToString(&value_string));
    ASSERT_EQ("Hai", value_string);

    delete value;
}

TEST_F(MessageStreamTest, WriteMessageSendsCorrectDataForEmptyValue) {
    uint8_t data[0];
    uint8_t buffer[1024];
    ArrayByteStream* byte_stream = new ArrayByteStream(data, buffer);
    MessageStream message_stream(1000, byte_stream);
    int err;

    ASSERT_EQ(message_stream.WriteMessage(message_, OutgoingStringValue(""), &err), 0);

    EXPECT_EQ('F', buffer[0]) << "Incorrect magic value";
    EXPECT_EQ(0U, *(uint32_t*)(buffer + 1)) << "Wrong message length";
    EXPECT_EQ(0U, *(uint32_t*)(buffer + 5)) << "Wrong value length";
}

TEST_F(MessageStreamTest, WriteMessageSendsCorrectDataForNonemptyValue) {
    uint8_t data[0];
    uint8_t buffer[1024] = {0};
    ArrayByteStream* byte_stream = new ArrayByteStream(data, buffer);
    MessageStream message_stream(1000, byte_stream);
    int err;

    std::string message_value = "What";

    ASSERT_EQ(message_stream.WriteMessage(message_, OutgoingStringValue(message_value), &err), 0);

    EXPECT_EQ('F', buffer[0]) << "Incorrect magic value";
    EXPECT_EQ(0U, *(uint32_t*)(buffer + 1)) << "Wrong message length";
    EXPECT_EQ(htonl(message_value.size()), *(uint32_t*)(buffer + 5)) << "Wrong value length";

    std::string actual_value((char *)(buffer + 9), 4);
    EXPECT_EQ(message_value, actual_value);
}

TEST_F(MessageStreamTest, ReadMessageWithEmptyValueResultsInValidNonNullValue) {
    uint8_t data[] = {'F', 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t buffer[1024];
    ArrayByteStream* byte_stream = new ArrayByteStream(data, buffer);
    MessageStream message_stream(1000, byte_stream);

    IncomingValueInterface* value = NULL;
    ASSERT_EQ(MessageStream::MessageStreamReadStatus_SUCCESS,
        message_stream.ReadMessage(&message_, &value));

    ASSERT_NE((IncomingValueInterface*)NULL, value);

    EXPECT_EQ((size_t) 0, value->size());

    delete value;
}

TEST_F(MessageStreamTest, ReadMessageFailsIfMessageTooLarge) {
    uint8_t data[] = {'F', 0, 0, 0, 10, 0, 0, 0, 0};
    uint8_t buffer[1024];
    ArrayByteStream* byte_stream = new ArrayByteStream(data, buffer);
    MessageStream message_stream(1, byte_stream);

    IncomingValueInterface* value = NULL;
    ASSERT_EQ(MessageStream::MessageStreamReadStatus_TOO_LARGE,
        message_stream.ReadMessage(&message_, &value));
}

} // namespace kinetic
