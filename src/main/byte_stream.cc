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

#include "kinetic/byte_stream.h"

#include <errno.h>
#include <unistd.h>

#include "glog/logging.h"

#include "kinetic/incoming_value.h"
#include "kinetic/outgoing_value.h"
#include "kinetic/reader_writer.h"

namespace kinetic {

PlainByteStream::PlainByteStream(int fd, IncomingValueFactoryInterface &value_factory)
    : fd_(fd), value_factory_(value_factory) {}

bool PlainByteStream::Read(void *buf, size_t n) {
    ReaderWriter reader_writer(fd_);
    int err;
    return reader_writer.Read(buf, n, &err);
}

bool PlainByteStream::Write(const void *buf, size_t n) {
    ReaderWriter reader_writer(fd_);
    return reader_writer.Write(buf, n);
}

IncomingValueInterface *PlainByteStream::ReadValue(size_t n) {
    return value_factory_.NewValue(fd_, n);
}

bool PlainByteStream::WriteValue(const OutgoingValueInterface &value, int* err) {
    return value.TransferToSocket(fd_, err);
}

SslByteStream::SslByteStream(SSL *ssl) : ssl_(ssl) {}

SslByteStream::~SslByteStream() {
}

bool SslByteStream::Read(void *buf, size_t n) {
    if (n == 0) {
        // SSL_read with 0 bytes causes openssl to get really upset in mysterious ways
        return true;
    }

    // To be able to move the pointed as data arrives we need to cast it to a complete
    // c type.
    uint8_t* byte_buffer = static_cast<uint8_t*>(buf);

    while (n > 0) {
      int bytes_read = SSL_read(ssl_, byte_buffer, n);

      if (bytes_read > 0) {
        // If SSL_read succeeds it returns the number of bytes read
        n -= bytes_read;
        byte_buffer += bytes_read;
      } else {
        // Return values of 0 or <0 indicate an error in SSL_read
        LOG(WARNING) << "Failed to read " << n << " bytes over SSL connection";
        return false;
      }
    }

    return true;
}

bool SslByteStream::Write(const void *buf, size_t n) {
    if (n == 0) {
        // It's not clear whether SSL_write can handle a write of 0 bytes
        return true;
    }

    // To be able to move the pointed as data arrives we need to cast it to a complete
    // c type.
    const uint8_t* byte_buffer = static_cast<const uint8_t*>(buf);

    while (n > 0) {
      int bytes_written = SSL_write(ssl_, byte_buffer, n);

      if (bytes_written > 0) {
        // If SSL_read succeeds it returns the number of bytes read
        n -= bytes_written;
        byte_buffer += bytes_written;
      } else {
        // Return values of 0 or <0 indicate an error in SSL_read
        LOG(WARNING) << "Failed to write " << n << " bytes over SSL connection";
        return false;
      }
    }

    return true;
}

IncomingValueInterface *SslByteStream::ReadValue(size_t n) {
    // We can't use splice since we're using SSL here--instead we fall back on
    // just copying the value into a IncomingStringValue object.
    char *buf = new char[n];
    if (!Read(buf, n)) {
        delete[] buf;
        return NULL;
    }
    std::string value(buf, n);
    delete[] buf;
    return new IncomingStringValue(value);
}

bool SslByteStream::WriteValue(const OutgoingValueInterface &value, int* err) {
    std::string s;
    if (!value.ToString(&s, err)) {
        return false;
    }
    return Write(s.data(), s.size());
}

} // namespace kinetic
