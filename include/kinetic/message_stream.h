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

#ifndef KINETIC_CPP_CLIENT_MESSAGE_STREAM_H_
#define KINETIC_CPP_CLIENT_MESSAGE_STREAM_H_

#include "google/protobuf/message.h"
#include "openssl/ssl.h"

#include "byte_stream.h"
#include "common.h"
#include "incoming_value.h"

namespace kinetic {

class MessageStreamInterface {
    public:
    typedef enum {
        MessageStreamReadStatus_SUCCESS,
        MessageStreamReadStatus_INTERNAL_ERROR,
        MessageStreamReadStatus_TOO_LARGE
    } MessageStreamReadStatus;

    virtual ~MessageStreamInterface() {}
    virtual MessageStreamReadStatus ReadMessage(::google::protobuf::Message *message,
        IncomingValueInterface** value) = 0;
    virtual int WriteMessage(const ::google::protobuf::Message &message,
        const OutgoingValueInterface& value, int* err) = 0;
};

class MessageStream : public MessageStreamInterface {
    public:
    explicit MessageStream(uint32_t max_message_size_bytes, ByteStreamInterface *byte_stream);
    ~MessageStream();
    MessageStreamReadStatus ReadMessage(::google::protobuf::Message *message,
        IncomingValueInterface** value);
    int WriteMessage(const ::google::protobuf::Message &message,
        const OutgoingValueInterface& value, int* err);

    private:
    bool ReadHeader(uint32_t *message_size, uint32_t *value_size);
    bool WriteHeader(uint32_t message_size, uint32_t value_size);
    uint32_t max_message_size_bytes_;
    ByteStreamInterface *byte_stream_;
    DISALLOW_COPY_AND_ASSIGN(MessageStream);
};

class MessageStreamFactoryInterface {
    public:
    virtual bool NewMessageStream(int fd, bool use_ssl, uint32_t max_message_size_bytes,
        MessageStreamInterface **message_stream) = 0;
    virtual ~MessageStreamFactoryInterface() {}
};

class MessageStreamFactory : public MessageStreamFactoryInterface {
    public:
    MessageStreamFactory(SSL_CTX *ssl_context, IncomingValueFactoryInterface &value_factory);
    bool NewMessageStream(int fd, bool use_ssl, uint32_t max_message_size_bytes,
        MessageStreamInterface **message_stream);
    virtual ~MessageStreamFactory() {}

    private:
    SSL_CTX *ssl_context_;
    IncomingValueFactoryInterface &value_factory_;
    DISALLOW_COPY_AND_ASSIGN(MessageStreamFactory);
};

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_MESSAGE_STREAM_H_
