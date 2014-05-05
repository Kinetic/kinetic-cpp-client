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

NonblockingStringReader::NonblockingStringReader(int fd, size_t size, unique_ptr<const string> &s)
        : fd_(fd), size_(size), s_(s), buf_(new char[size]), bytes_read_(0) {}

NonblockingStringReader::~NonblockingStringReader() {
    delete[] buf_;
}

NonblockingStringStatus NonblockingStringReader::Read() {
    while (bytes_read_ < size_) {
        int status = read(fd_, buf_ + bytes_read_, size_ - bytes_read_);
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

NonblockingStringWriter::NonblockingStringWriter(int fd, const shared_ptr<const string> s)
    : fd_(fd), s_(s), bytes_written_(0) {}

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
        if (fstat(fd_, &statbuf)) {
            PLOG(ERROR) << "Unable to fstat socket";
            return kFailed;
        }
        int status;
        if (S_ISSOCK(statbuf.st_mode)) {
            status = send(
                fd_,
                s_->data() + bytes_written_,
                s_->size() - bytes_written_,
                flags);
        } else {
            status = write(fd_, s_->data() + bytes_written_, s_->size() - bytes_written_);
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
