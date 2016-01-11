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

#include "nonblocking_string.h"

#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include "glog/logging.h"

namespace kinetic {

using std::string;
using std::unique_ptr;
using std::move;

NonblockingStringReader::NonblockingStringReader(shared_ptr<SocketWrapperInterface> socket_wrapper,
        size_t size, unique_ptr<const string> &s)
        : socket_wrapper_(socket_wrapper), size_(size), s_(s), buf_(new char[size]), bytes_read_(0) {}

NonblockingStringReader::~NonblockingStringReader() {
    delete[] buf_;
}

NonblockingStringStatus NonblockingStringReader::Read() {
    while (bytes_read_ < size_) {
        int status = 0;
        if (socket_wrapper_->getSSL()) {
             status = SSL_read(socket_wrapper_->getSSL(), buf_ + bytes_read_, size_ - bytes_read_);
        } else {
            status = read(socket_wrapper_->fd(), buf_ + bytes_read_, size_ - bytes_read_);
        }
        if (status == 0) {
            // Unexpected EOF
            return kFailed;
        }
        if (status < 0) {
            if (errno == EINTR) {
                continue;
            }
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return kInProgress;
            }
            // Encountered an irrecoverable error
            return kFailed;
        }
        bytes_read_ += status;
    }

    CHECK_EQ(bytes_read_, size_);
    // it'd be nice if we could use make_unique
    unique_ptr<const string> p(new string(buf_, size_));
    s_ = move(p);
    return kDone;
}

NonblockingStringWriter::NonblockingStringWriter(shared_ptr<SocketWrapperInterface> socket_wrapper,
        const shared_ptr<const string> s)
    : socket_wrapper_(socket_wrapper), s_(s), bytes_written_(0) {}

NonblockingStringStatus NonblockingStringWriter::Write() {
    while (bytes_written_ < s_->size()) {
        int flags = 0;
        // Prevent sending SIGPIPE signal on Linux. SIGPIPE is undesirable because the library
        // client will crash if the remote server closes the connection.
        #ifdef MSG_NOSIGNAL
        flags |= MSG_NOSIGNAL;
        #endif

        // We need to use send() for sockets but tests use pipes so we can't assume sockets.
        // To deal with this annoyance detect whether the FD is a socket and use the write
        // send/write
        struct stat statbuf;
        if (fstat(socket_wrapper_->fd(), &statbuf)) {
            PLOG(ERROR) << "Unable to fstat socket";
            return kFailed;
        }
        int status;
        if (S_ISSOCK(statbuf.st_mode)) {
            if (socket_wrapper_->getSSL())
                status = SSL_write(socket_wrapper_->getSSL(), s_->data() + bytes_written_, s_->size() - bytes_written_);
            else
            status = send(
                socket_wrapper_->fd(),
                s_->data() + bytes_written_,
                s_->size() - bytes_written_,
                flags);
        } else {
            if (socket_wrapper_->getSSL())
                status = SSL_write(socket_wrapper_->getSSL(), s_->data() + bytes_written_, s_->size() - bytes_written_);
            else
                status = write(socket_wrapper_->fd(), s_->data() + bytes_written_, s_->size() - bytes_written_);
        }
        if (status == 0) {
            return kFailed;
        }
        if (status < 0) {
            if (errno == EINTR) {
                continue;
            }
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return kInProgress;
            }
            return kFailed;
        }
        bytes_written_ += status;
    }

    CHECK_EQ(bytes_written_, s_->size());
    return kDone;
}

} // namespace kinetic
