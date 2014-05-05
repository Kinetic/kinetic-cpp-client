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
    void Handle(const Message &response, unique_ptr<const string> value) {
        Handle_(response, *value);
    }
    MOCK_METHOD2(Handle_, void(const Message &response, const string& value));
    MOCK_METHOD1(Error, void(KineticStatus error));
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
    void Enqueue(unique_ptr<Message> message, const shared_ptr<const string> value,
        unique_ptr<HandlerInterface> handler, HandlerKey handler_key) {
        Enqueue_(*message, value, handler.get(), handler_key);
    }
    MOCK_METHOD4(Enqueue_, void(const Message& message, const shared_ptr<const string> value,
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
    unique_ptr<NonblockingPacketWriterInterface> CreateWriter(int fd,
        unique_ptr<const Message> message, const shared_ptr<const string> value) {
        return unique_ptr<NonblockingPacketWriterInterface>(
            CreateWriter_(fd, *message, value));
    }

    MOCK_METHOD3(CreateWriter_,  NonblockingPacketWriterInterface* (const int fd,
        const Message& message, const shared_ptr<const string> value));
};

class MockNonblockingPacketService : public NonblockingPacketServiceInterface {
    public:
    HandlerKey Submit(unique_ptr<Message> message, const shared_ptr<const string> value,
            unique_ptr<HandlerInterface> handler) {
        return Submit_(*message, value, handler.get());
    }
    MOCK_METHOD3(Submit_, HandlerKey(const Message &message, const shared_ptr<const string> value,
    HandlerInterface* handler));
    MOCK_METHOD3(Run, bool(fd_set *read_fds, fd_set *write_fds, int *nfds));
    MOCK_METHOD1(Remove, bool(HandlerKey handler_key));
};

} // namespace kinetic
#endif  // KINETIC_CPP_CLIENT_MOCK_NONBLOCKING_PACKET_SERVICE_H_
