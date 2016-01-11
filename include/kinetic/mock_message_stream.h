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
