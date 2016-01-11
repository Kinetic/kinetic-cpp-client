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

#ifndef KINETIC_CPP_CLIENT_COPYING_SSL_STREAM_H_
#define KINETIC_CPP_CLIENT_COPYING_SSL_STREAM_H_

#include "google/protobuf/io/zero_copy_stream_impl_lite.h"
#include "openssl/ssl.h"
#include "kinetic/common.h"

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
