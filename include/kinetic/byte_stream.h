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
    virtual bool WriteValue(const OutgoingValueInterface &value) = 0;
};

class PlainByteStream : public ByteStreamInterface {
    public:
    PlainByteStream(int fd, IncomingValueFactoryInterface &value_factory);
    bool Read(void *buf, size_t n);
    bool Write(const void *buf, size_t n);
    IncomingValueInterface *ReadValue(size_t n);
    bool WriteValue(const OutgoingValueInterface &value);

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
    bool WriteValue(const OutgoingValueInterface &value);

    private:
    SSL *ssl_;
    DISALLOW_COPY_AND_ASSIGN(SslByteStream);
};

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_BYTE_STREAM_H_
