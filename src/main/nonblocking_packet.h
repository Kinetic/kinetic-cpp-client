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

#ifndef KINETIC_CPP_CLIENT_NONBLOCKING_PACKET_H_
#define KINETIC_CPP_CLIENT_NONBLOCKING_PACKET_H_

#include <memory>

#include "kinetic/common.h"

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
    NonblockingPacketWriter(shared_ptr<SocketWrapperInterface> socket_wrapper, unique_ptr<const Message> message,
            const shared_ptr<const string> value);
    ~NonblockingPacketWriter();
    NonblockingStringStatus Write();

    private:
    bool TransitionFromMagic();
    void TransitionFromMessageLength();
    void TransitionFromValueLength();
    void TransitionFromMessage();
    void TransitionFromValue();
    shared_ptr<SocketWrapperInterface> socket_wrapper_;
    unique_ptr<const Message> message_;
    const shared_ptr<const string> value_;
    State state_;
    NonblockingStringWriter *writer_;
    std::string serialized_message_;
    DISALLOW_COPY_AND_ASSIGN(NonblockingPacketWriter);
};

class NonblockingPacketReader {
    public:
    NonblockingPacketReader(shared_ptr<SocketWrapperInterface> socket_wrapper, Message* response, unique_ptr<const string>& value);
    ~NonblockingPacketReader();
    NonblockingStringStatus Read();

    private:
    bool TransitionFromMagic();
    void TransitionFromMessageLength();
    void TransitionFromValueLength();
    void TransitionFromMessage();
    bool TransitionFromValue();
    shared_ptr<SocketWrapperInterface> socket_wrapper_;
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
    virtual unique_ptr<NonblockingPacketWriterInterface> CreateWriter(shared_ptr<SocketWrapperInterface> socket_wrapper,
        unique_ptr<const Message> message, const shared_ptr<const string> value) = 0;
};

class NonblockingPacketWriterFactory : public NonblockingPacketWriterFactoryInterface {
    public:
    unique_ptr<NonblockingPacketWriterInterface> CreateWriter(shared_ptr<SocketWrapperInterface> socket_wrapper,
        unique_ptr<const Message> message, const shared_ptr<const string> value);
};

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_NONBLOCKING_PACKET_H_
