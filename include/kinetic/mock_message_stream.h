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

#ifndef KINETIC_CPP_CLIENT_MOCK_MESSAGE_STREAM_H_
#define KINETIC_CPP_CLIENT_MOCK_MESSAGE_STREAM_H_

#include "gmock/gmock.h"
#include "google/protobuf/message.h"
#include "openssl/ssl.h"

#include "byte_stream.h"
#include "common.h"
#include "incoming_value.h"

namespace kinetic {

class MockMessageStream : public MessageStreamInterface {
    public:
    MockMessageStream() {}
    MOCK_METHOD2(ReadMessage, MessageStreamReadStatus(::google::protobuf::Message *message,
        IncomingValueInterface** value));
    MOCK_METHOD2(WriteMessage, int(const ::google::protobuf::Message &message,
        const OutgoingValueInterface& value));
    MOCK_METHOD0(BytesRead, int64_t());
    MOCK_METHOD0(BytesWritten, int64_t());
};

class MockMessageStreamFactory : public MessageStreamFactoryInterface {
    public:
    MockMessageStreamFactory() {}
    ~MockMessageStreamFactory() {}
    MOCK_METHOD4(NewMessageStream,
        bool(int fd, bool use_ssl, SSL *ssl, uint32_t max_message_size_bytes,
            MessageStreamInterface **message_stream));
};

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_MOCK_MESSAGE_STREAM_H_
