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

#ifndef KINETIC_CPP_CLIENT_COPYING_SSL_STREAM_H_
#define KINETIC_CPP_CLIENT_COPYING_SSL_STREAM_H_

#include "google/protobuf/io/zero_copy_stream_impl_lite.h"
#include "openssl/ssl.h"
#include "protobufutil/common.h"

namespace kinetic {

using google::protobuf::io::CopyingInputStream;
using google::protobuf::io::CopyingOutputStream;

/* These classes represent SSL input and output streams. Note that they assume
 * an SSL connection has already been established via SSL_accept or
 * SSL_connect. */

class CopyingSslInputStream : public CopyingInputStream {
    public:
    explicit CopyingSslInputStream(SSL *ssl);
    virtual int Read(void *buffer, int size);

    private:
    SSL *ssl_;
    DISALLOW_COPY_AND_ASSIGN(CopyingSslInputStream);
};

class CopyingSslOutputStream : public CopyingOutputStream {
    public:
    explicit CopyingSslOutputStream(SSL *ssl);
    virtual bool Write(const void *buffer, int size);

    private:
    SSL *ssl_;
    DISALLOW_COPY_AND_ASSIGN(CopyingSslOutputStream);
};

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_COPYING_SSL_STREAM_H_
