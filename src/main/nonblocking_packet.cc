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

#include "nonblocking_packet.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>

#include "glog/logging.h"

namespace kinetic {

using std::make_shared;
using std::string;

NonblockingPacketWriter::NonblockingPacketWriter(shared_ptr<SocketWrapperInterface> socket_wrapper, unique_ptr<const Message> message,
    const shared_ptr<const string> value)
    : socket_wrapper_(socket_wrapper), message_(move(message)), value_(value), state_(kMagic),
    writer_(new NonblockingStringWriter(socket_wrapper_, make_shared<string>("F"))) {}

NonblockingPacketWriter::~NonblockingPacketWriter() {
    if (writer_ != NULL) {
        delete writer_;
    }
}

NonblockingStringStatus NonblockingPacketWriter::Write() {
    struct stat statbuf;
    if (fstat(socket_wrapper_->fd(), &statbuf)) {
        PLOG(ERROR) << "Unable to fstat socket";
        return kFailed;
    }
#ifndef __APPLE__
    if (S_ISSOCK(statbuf.st_mode)) {
        int optval = 1;
        setsockopt(socket_wrapper_->fd(), IPPROTO_TCP, TCP_CORK, &optval, sizeof(optval));
    }
#endif

    while (true) {
        NonblockingStringStatus status = writer_->Write();
        if (status != kDone) {
            return status;
        }

        // Transition to the next state
        switch (state_) {
            case kMagic:
                if (!TransitionFromMagic()) {
                    return kFailed;
                }
                break;
            case kMessageLength:
                TransitionFromMessageLength();
                break;
            case kValueLength:
                TransitionFromValueLength();
                break;
            case kMessage:
                TransitionFromMessage();
                break;
            case kValue:
                TransitionFromValue();
                break;
            case kFinished:
                if (fstat(socket_wrapper_->fd(), &statbuf)) {
                    PLOG(ERROR) << "Unable to fstat socket";
                    return kFailed;
                }
                if (S_ISSOCK(statbuf.st_mode)) {
                    int optval = 0;
#ifndef __APPLE__
                    setsockopt(socket_wrapper_->fd(), IPPROTO_TCP, TCP_CORK, &optval, sizeof(optval));
#endif
                    optval = 1;
                    setsockopt(socket_wrapper_->fd(), IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
                }
                return kDone;
            default:
                CHECK(false);
        }
    }
}

bool NonblockingPacketWriter::TransitionFromMagic() {
    // Move on to the writing the message length
    if (!message_->SerializeToString(&serialized_message_)) {
        // Serialization can fail if the message is missing required fields
        return false;
    }
    uint32_t size = htonl(serialized_message_.size());
    delete writer_;
    std::string encoded_size(reinterpret_cast<char *>(&size), sizeof(size));
    writer_ = new NonblockingStringWriter(socket_wrapper_, make_shared<string>(encoded_size));
    state_ = kMessageLength;
    return true;
}

void NonblockingPacketWriter::TransitionFromMessageLength() {
    // Move on to writing the value length
    uint32_t size = htonl(value_->size());
    delete writer_;
    std::string encoded_size(reinterpret_cast<char *>(&size), sizeof(size));
    writer_ = new NonblockingStringWriter(socket_wrapper_, make_shared<string>(encoded_size));
    state_ = kValueLength;
}

void NonblockingPacketWriter::TransitionFromValueLength() {
    // Move on to writing the serialized message
    delete writer_;
    writer_ = new NonblockingStringWriter(socket_wrapper_, make_shared<string>(serialized_message_));
    state_ = kMessage;
}

void NonblockingPacketWriter::TransitionFromMessage() {
    // Move on to writing the value
    delete writer_;
    writer_ = new NonblockingStringWriter(socket_wrapper_, value_);
    state_ = kValue;
}

void NonblockingPacketWriter::TransitionFromValue() {
    // We're done!
    state_ = kFinished;
}

NonblockingPacketReader::NonblockingPacketReader(shared_ptr<SocketWrapperInterface> socket_wrapper, Message* response,
        unique_ptr<const string> &value)
    : socket_wrapper_(socket_wrapper), response_(response), state_(kMagic), value_(value), magic_(),
    reader_(new NonblockingStringReader(socket_wrapper_, 1, magic_)) {
}

NonblockingPacketReader::~NonblockingPacketReader() {
    if (reader_ != NULL) {
        delete reader_;
    }
}

NonblockingStringStatus NonblockingPacketReader::Read() {
    while (true) {
        NonblockingStringStatus status = reader_->Read();
        if (status != kDone) {
            return status;
        }

        // Transition to the next state
        switch (state_) {
            case kMagic:
                if (!TransitionFromMagic()) {
                    return kFailed;
                }
                break;
            case kMessageLength:
                TransitionFromMessageLength();
                break;
            case kValueLength:
                TransitionFromValueLength();
                break;
            case kMessage:
                TransitionFromMessage();
                break;
            case kValue:
                if (!TransitionFromValue()) {
                    return kFailed;
                }
                break;
            case kFinished:
                return kDone;
                break;
            default:
                CHECK(false);
        }
    }
}

bool NonblockingPacketReader::TransitionFromMagic() {
    // Check the magic byte and move on to reading the message length
    if (*magic_ != "F") {
        return false;
    }
    delete reader_;
    reader_ = new NonblockingStringReader(socket_wrapper_, 4, message_length_);
    state_ = kMessageLength;
    return true;
}

void NonblockingPacketReader::TransitionFromMessageLength() {
    // Move on to reading the value length
    delete reader_;
    reader_ = new NonblockingStringReader(socket_wrapper_, 4, value_length_);
    state_ = kValueLength;
}

void NonblockingPacketReader::TransitionFromValueLength() {
    // Move on to reading the message
    delete reader_;
    CHECK_EQ(4u, message_length_->size());
    uint32_t length = ntohl(*reinterpret_cast<const uint32_t *>(message_length_->data()));
    reader_ = new NonblockingStringReader(socket_wrapper_, length, message_);
    state_ = kMessage;
}

void NonblockingPacketReader::TransitionFromMessage() {
    // Move on to reading the value
    delete reader_;
    CHECK_EQ(4u, value_length_->size());
    uint32_t length = ntohl(*reinterpret_cast<const uint32_t *>(value_length_->data()));
    reader_ = new NonblockingStringReader(socket_wrapper_, length, value_);
    state_ = kValue;
}

bool NonblockingPacketReader::TransitionFromValue() {
    // We're done!
    state_ = kFinished;
    return response_->ParseFromString(*message_);
}

} // namespace kinetic
