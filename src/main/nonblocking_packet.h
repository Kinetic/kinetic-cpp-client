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

#ifndef KINETIC_CPP_CLIENT_NONBLOCKING_PACKET_H_
#define KINETIC_CPP_CLIENT_NONBLOCKING_PACKET_H_

#include <memory>

#include "protobufutil/common.h"

#include "kinetic_client.pb.h"
#include "nonblocking_string.h"

namespace kinetic {

using std::shared_ptr;
using std::unique_ptr;
using std::string;

using com::seagate::kinetic::client::proto::Message;

enum State {
    kMagic,
    kMessageLength,
    kValueLength,
    kMessage,
    kValue,
    kFinished
};

class NonblockingPacketWriterInterface {
    public:
    virtual ~NonblockingPacketWriterInterface() {}
    virtual NonblockingStringStatus Write() = 0;
};

class NonblockingPacketWriter : public NonblockingPacketWriterInterface {
    public:
    NonblockingPacketWriter(int fd, unique_ptr<const Message> message,
            const shared_ptr<const string> value);
    ~NonblockingPacketWriter();
    NonblockingStringStatus Write();

    private:
    bool TransitionFromMagic();
    void TransitionFromMessageLength();
    void TransitionFromValueLength();
    void TransitionFromMessage();
    void TransitionFromValue();
    const int fd_;
    unique_ptr<const Message> message_;
    const shared_ptr<const string> value_;
    State state_;
    NonblockingStringWriter *writer_;
    std::string serialized_message_;
    DISALLOW_COPY_AND_ASSIGN(NonblockingPacketWriter);
};

class NonblockingPacketReader {
    public:
    NonblockingPacketReader(int fd, Message* response, unique_ptr<const string>& value);
    ~NonblockingPacketReader();
    NonblockingStringStatus Read();

    private:
    bool TransitionFromMagic();
    void TransitionFromMessageLength();
    void TransitionFromValueLength();
    void TransitionFromMessage();
    bool TransitionFromValue();
    const int fd_;
    Message* const response_;
    State state_;
    unique_ptr<const string>& value_;
    unique_ptr<const string> magic_;
    unique_ptr<const string> message_length_;
    unique_ptr<const string> value_length_;
    unique_ptr<const string> message_;
    NonblockingStringReader* reader_;
    DISALLOW_COPY_AND_ASSIGN(NonblockingPacketReader);
};

class NonblockingPacketWriterFactoryInterface {
    public:
    virtual ~NonblockingPacketWriterFactoryInterface() {}
    virtual unique_ptr<NonblockingPacketWriterInterface> CreateWriter(int fd,
        unique_ptr<const Message> message, const shared_ptr<const string> value) = 0;
};

class NonblockingPacketWriterFactory : public NonblockingPacketWriterFactoryInterface {
    public:
    unique_ptr<NonblockingPacketWriterInterface> CreateWriter(int fd,
        unique_ptr<const Message> message, const shared_ptr<const string> value);
};

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_NONBLOCKING_PACKET_H_
