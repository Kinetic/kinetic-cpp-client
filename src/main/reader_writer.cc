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

#include "kinetic/reader_writer.h"

#include <errno.h>
#include <unistd.h>

#include "glog/logging.h"

const uint32_t SOCKET_TIMEOUT = 20000;  //10 seconds

namespace kinetic {

ReaderWriter::ReaderWriter(int fd) : fd_(fd) {}

bool ReaderWriter::Read(void *buf, size_t n, int* err) {
    size_t bytes_read = 0;
    uint32_t socket_timeout = 0;
    while (bytes_read < n && socket_timeout < SOCKET_TIMEOUT) {
        int status = read(fd_, reinterpret_cast<char *>(buf) + bytes_read, n - bytes_read);
        if (status == -1 && errno == EINTR) {
            continue;
        } else if (status == -1 && (errno == EAGAIN || errno == EWOULDBLOCK )) {
	    //Wait for 500us;
	    usleep(500);
            socket_timeout++;
            continue;
        } else if (status < 0) {
            *err = errno;
            PLOG(WARNING) << "Failed to read from socket";
            return false;
        }
        if (status == 0) {
            LOG(WARNING) << "Unexpected EOF. Socket (TX) may be closed by Peer";
            return false;
        }
        bytes_read += status;
    }
    if (socket_timeout >= SOCKET_TIMEOUT) {
        LOG(INFO) << "Peer is slow to transmit";
        return false;
    }
    return true;
}

bool ReaderWriter::Write(const void *buf, size_t n) {
    size_t bytes_written = 0;
    uint32_t socket_timeout = 0;
    while (bytes_written < n && socket_timeout < SOCKET_TIMEOUT) {
        int status = write(fd_, reinterpret_cast<const char *>(buf) + bytes_written,
            n - bytes_written);
        if (status == -1 && errno == EINTR) {
            continue;
        } else if (status == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
	    usleep(500);
	    socket_timeout++;
            continue;
        } else if (status < 0) {	
            PLOG(WARNING) << "Failed to write to socket";
            return false;
        }
        if (status == 0) {
            LOG(WARNING) << "Unexpected EOF, Socket(RX) may be closed by Peer";
            return false;
        }
        bytes_written += status;
    }
    if (socket_timeout >= SOCKET_TIMEOUT) {
        LOG(INFO) << " Peer is slow to receive";
        return false;
    }

    return true;
}

} // namespace kinetic
