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

#ifndef KINETIC_CPP_CLIENT_MOCK_NONBLOCKING_PACKET_SERVICE_H_
#define KINETIC_CPP_CLIENT_MOCK_NONBLOCKING_PACKET_SERVICE_H_

#include "kinetic/nonblocking_packet_service_interface.h"

namespace kinetic {
using std::string;
using std::unique_ptr;
using std::queue;

class MockHandler : public HandlerInterface {
    public:
    MockHandler() {}
    void Handle(const Command &response, unique_ptr<const string> value) {
        Handle_(response, *value);
    }
    MOCK_METHOD2(Handle_, void(const Command &response, const string& value));
    MOCK_METHOD2
    (Error, void(KineticStatus error, Command const * const response));
};

class MockNonblockingReceiver : public NonblockingReceiverInterface {
    public:
    bool Enqueue(shared_ptr<HandlerInterface> handler, google::int64 sequence,
            HandlerKey handler_key) {
        return Enqueue_(handler.get(), sequence, handler_key);
    }
    MOCK_METHOD3(Enqueue_, bool(HandlerInterface *handler, google::int64 sequence,
            HandlerKey handler_key));
    MOCK_METHOD0(Receive, NonblockingPacketServiceStatus());
    MOCK_METHOD0(connection_id, int64_t());
    MOCK_METHOD1(Remove, bool(HandlerKey key));
};

class MockNonblockingSender : public NonblockingSenderInterface {
    public:
    void Enqueue(unique_ptr<Message> message, unique_ptr<Command> command, const shared_ptr<const string> value,
        unique_ptr<HandlerInterface> handler, HandlerKey handler_key) {
        Enqueue_(*message, *command, value, handler.get(), handler_key);
    }
    MOCK_METHOD5(Enqueue_, void(const Message& message, const Command& command, const shared_ptr<const string> value,
        HandlerInterface *handler, HandlerKey handler_key));
    MOCK_METHOD0(Send, NonblockingPacketServiceStatus());
    MOCK_METHOD1(Remove, bool(HandlerKey key));
};

class MockNonblockingPacketWriter : public NonblockingPacketWriterInterface {
    public:
    MOCK_METHOD0(Write, NonblockingStringStatus());
};

class MockNonblockingPacketWriterFactory : public NonblockingPacketWriterFactoryInterface {
    public:
    unique_ptr<NonblockingPacketWriterInterface> CreateWriter(shared_ptr<SocketWrapperInterface> socket_wrapper,
        unique_ptr<const Message> message, const shared_ptr<const string> value) {
        return unique_ptr<NonblockingPacketWriterInterface>(
            CreateWriter_(socket_wrapper, *message, value));
    }

    MOCK_METHOD3(CreateWriter_,  NonblockingPacketWriterInterface* (shared_ptr<SocketWrapperInterface> socket_wrapper,
        const Message& message, const shared_ptr<const string> value));
};

class MockNonblockingPacketService : public NonblockingPacketServiceInterface {
    public:
    HandlerKey Submit(unique_ptr<Message> message, unique_ptr<Command> command, const shared_ptr<const string> value,
            unique_ptr<HandlerInterface> handler) {
        return Submit_(*message, *command, value, handler.get());
    }
    MOCK_METHOD4(Submit_, HandlerKey(const Message &message, const Command &command, const shared_ptr<const string> value,
    HandlerInterface* handler));
    MOCK_METHOD3(Run, bool(fd_set *read_fds, fd_set *write_fds, int *nfds));
    MOCK_METHOD1(Remove, bool(HandlerKey handler_key));
};

} // namespace kinetic
#endif  // KINETIC_CPP_CLIENT_MOCK_NONBLOCKING_PACKET_SERVICE_H_
