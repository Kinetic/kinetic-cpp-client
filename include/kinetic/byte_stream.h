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

#ifndef KINETIC_CPP_CLIENT_BYTE_STREAM_H_
#define KINETIC_CPP_CLIENT_BYTE_STREAM_H_

#include "openssl/ssl.h"

#include "common.h"
#include "incoming_value.h"
#include "outgoing_value.h"

namespace kinetic {

class ByteStreamInterface {
    public:
    virtual ~ByteStreamInterface() {}
    virtual bool Read(void *buf, size_t n) = 0;
    virtual bool Write(const void *buf, size_t n) = 0;
    virtual IncomingValueInterface *ReadValue(size_t n) = 0;
    virtual bool WriteValue(const OutgoingValueInterface &value, int* err) = 0;
};

class PlainByteStream : public ByteStreamInterface {
    public:
    PlainByteStream(int fd, IncomingValueFactoryInterface &value_factory);
    bool Read(void *buf, size_t n);
    bool Write(const void *buf, size_t n);
    IncomingValueInterface *ReadValue(size_t n);
    bool WriteValue(const OutgoingValueInterface &value, int* err);

    private:
    int fd_;
    IncomingValueFactoryInterface &value_factory_;
    DISALLOW_COPY_AND_ASSIGN(PlainByteStream);
};

class SslByteStream : public ByteStreamInterface {
    public:
    explicit SslByteStream(SSL *ssl);
    virtual ~SslByteStream();
    bool Read(void *buf, size_t n);
    bool Write(const void *buf, size_t n);
    IncomingValueInterface *ReadValue(size_t n);
    bool WriteValue(const OutgoingValueInterface &value, int* err);

    private:
    SSL *ssl_;
    DISALLOW_COPY_AND_ASSIGN(SslByteStream);
};

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_BYTE_STREAM_H_
