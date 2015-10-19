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

#include <arpa/inet.h>
#include <unistd.h>

#include "gmock/gmock.h"

#include "kinetic/kinetic.h"
#include "nonblocking_packet_service.h"
#include "mock_socket_wrapper_interface.h"
#include "mock_nonblocking_packet_service.h"
#include "matchers.h"

namespace kinetic {

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::StrictMock;
using com::seagate::kinetic::client::proto::Command_MessageType_GET_RESPONSE;
using com::seagate::kinetic::client::proto::Command_Status_StatusCode_SUCCESS;

using std::string;
using std::make_shared;
using std::unique_ptr;

class NonblockingSenderTest : public ::testing::Test {
    protected:
    NonblockingSenderTest() : closed_read_end_(false),
    writer_factory_(unique_ptr<NonblockingPacketWriterFactoryInterface>(
        new NonblockingPacketWriterFactory())) {}
    // Create a pipe that we can use to feed data to the NonblockingReceiver
    void SetUp() {
        ASSERT_EQ(0, pipe(fds_));
    }

    void TearDown() {
        if (!closed_read_end_) {
            ASSERT_EQ(0, close(fds_[0]));
        }
        ASSERT_EQ(0, close(fds_[1]));
    }

    int fds_[2];
    bool closed_read_end_;
    HmacProvider hmac_provider_;
    shared_ptr<NonblockingPacketWriterFactoryInterface> writer_factory_;
};

TEST_F(NonblockingSenderTest, SimpleMessageAndValue) {
    // We enqueue a simple packet for a NonblockingSender to send. It should
    // transfer the correct bytes over the wire and then pass the handler along
    // to the NonblockingReceiver.
    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    EXPECT_CALL(*socket_wrapper, fd()).WillRepeatedly(Return(fds_[1]));
    ConnectionOptions options;
    options.user_id = 3;
    options.hmac_key = "key";
    unique_ptr<HandlerInterface> handler(new MockHandler());
    auto receiver = make_shared<MockNonblockingReceiver>();
    EXPECT_CALL(*receiver, Enqueue_(handler.get(), 0, 0)).WillOnce(Return(true));
    EXPECT_CALL(*receiver, connection_id()).WillRepeatedly(Return(1));

    EXPECT_CALL(*socket_wrapper, getSSL()).WillRepeatedly(Return((SSL*)0));
    NonblockingSender sender(socket_wrapper, receiver, writer_factory_, hmac_provider_,
        options);
    unique_ptr<Message> message(new Message());
    unique_ptr<Command> command(new Command());
    sender.Enqueue(move(message), move(command), make_shared<string>("value"), move(handler), 0);
    ASSERT_EQ(kIdle, sender.Send());

    // Check that the pipe now contains what it's supposed to
    char magic;
    ASSERT_EQ(1, read(fds_[0], &magic, 1));
    ASSERT_EQ('F', magic);

    uint32_t message_length;
    ASSERT_EQ(4, read(fds_[0], reinterpret_cast<char *>(&message_length), 4));
    message_length = ntohl(message_length);

    uint32_t value_length;
    ASSERT_EQ(4, read(fds_[0], reinterpret_cast<char *>(&value_length), 4));
    value_length = ntohl(value_length);

    char *serialized_message = new char[message_length];
    ASSERT_EQ(static_cast<ssize_t>(message_length),
        read(fds_[0], serialized_message, message_length));
    delete[] serialized_message;

    char *value = new char[value_length];
    ASSERT_EQ(static_cast<ssize_t>(value_length), read(fds_[0], value, value_length));
    std::string value_string(value, value_length);
    ASSERT_EQ("value", value_string);
    delete[] value;
}

TEST_F(NonblockingSenderTest, CallsErrorWhenCannotEnqueueHandler) {
    // We enqueue a simple packet for a NonblockingSender to send. It should
    // transfer the correct bytes over the wire and then pass the handler along
    // to the NonblockingReceiver.
    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    EXPECT_CALL(*socket_wrapper, fd()).WillRepeatedly(Return(fds_[1]));
    ConnectionOptions options;
    options.user_id = 3;
    options.hmac_key = "key";
    unique_ptr<MockHandler> handler(new MockHandler());
    auto receiver = make_shared<MockNonblockingReceiver>();
    EXPECT_CALL(*handler, Error(KineticStatusEq(StatusCode::CLIENT_INTERNAL_ERROR,
        "Could not enqueue handler"), NULL));
    EXPECT_CALL(*receiver, Enqueue_(handler.get(), 0, 0)).WillOnce(Return(false));
    EXPECT_CALL(*receiver, connection_id()).WillRepeatedly(Return(1));

    EXPECT_CALL(*socket_wrapper, getSSL()).WillRepeatedly(Return((SSL*)0));
    NonblockingSender sender(socket_wrapper, receiver, move(writer_factory_), hmac_provider_,
        options);

    unique_ptr<Message> message(new Message());
    unique_ptr<Command> command(new Command());
    sender.Enqueue(move(message), move(command), make_shared<string>("value"), move(handler), 0);
    ASSERT_EQ(kIdle, sender.Send());
}

TEST_F(NonblockingSenderTest, UsesCorrectConnectionId) {
    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    EXPECT_CALL(*socket_wrapper, fd()).WillRepeatedly(Return(fds_[1]));
    ConnectionOptions options;
    options.user_id = 3;
    options.hmac_key = "key";
    unique_ptr<HandlerInterface> handler(new MockHandler());
    auto receiver = make_shared<MockNonblockingReceiver>();
    EXPECT_CALL(*receiver, Enqueue_(handler.get(), 0, 0)).WillOnce(Return(true));
    EXPECT_CALL(*receiver, connection_id()).WillRepeatedly(Return(42));
    EXPECT_CALL(*socket_wrapper, getSSL()).WillRepeatedly(Return((SSL*) 0));
    NonblockingSender sender(socket_wrapper, receiver, move(writer_factory_), hmac_provider_,
        options);
    unique_ptr<Message> message(new Message());
    unique_ptr<Command> command(new Command());
    sender.Enqueue(move(message), move(command), make_shared<string>(""), move(handler), 0);
    ASSERT_EQ(kIdle, sender.Send());

    // Check that the resulting message has the right connection id
    char magic;
    ASSERT_EQ(1, read(fds_[0], &magic, 1));
    ASSERT_EQ('F', magic);

    uint32_t message_length;
    ASSERT_EQ(4, read(fds_[0], reinterpret_cast<char *>(&message_length), 4));
    message_length = ntohl(message_length);

    uint32_t value_length;
    ASSERT_EQ(4, read(fds_[0], reinterpret_cast<char *>(&value_length), 4));
    value_length = ntohl(value_length);

    char *serialized_message = new char[message_length];
    ASSERT_EQ(static_cast<ssize_t>(message_length),
        read(fds_[0], serialized_message, message_length));
    Message message_parsed;
    ASSERT_TRUE(message_parsed.ParseFromArray(serialized_message, message_length));
    Command command_parsed;
    command_parsed.ParseFromString(message_parsed.commandbytes());
    ASSERT_EQ(42, command_parsed.header().connectionid());
    delete[] serialized_message;
}

TEST_F(NonblockingSenderTest, HandlesWriteError) {
    // Close the read end of the pipe so that attempts to write into it fail
    ASSERT_EQ(0, close(fds_[0]));
    closed_read_end_ = true;

    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    EXPECT_CALL(*socket_wrapper, fd()).WillRepeatedly(Return(fds_[1]));
    ConnectionOptions options;
    options.user_id = 3;
    options.hmac_key = "key";
    auto receiver = make_shared<NiceMock<MockNonblockingReceiver>>();

    EXPECT_CALL(*socket_wrapper, getSSL()).WillRepeatedly(Return((SSL*) 0));
    NonblockingSender sender(socket_wrapper, receiver, move(writer_factory_), hmac_provider_,
        options);

    unique_ptr<MockHandler> handler(new MockHandler());
    EXPECT_CALL(*handler, Error(KineticStatusEq(
            StatusCode::CLIENT_IO_ERROR, "I/O write error"), NULL));
    unique_ptr<Message> message(new Message());
    unique_ptr<Command> command(new Command());
    sender.Enqueue(move(message), move(command), make_shared<string>(""), move(handler), 0);
    ASSERT_EQ(kError, sender.Send());
}

TEST_F(NonblockingSenderTest, MaintainsCorrectHandlerKeyWhenWriteDoesntCompleteOnFirstTry) {
    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    EXPECT_CALL(*socket_wrapper, fd()).WillRepeatedly(Return(fds_[1]));
    ConnectionOptions options;
    options.user_id = 3;
    options.hmac_key = "key";
    unique_ptr<HandlerInterface> handler1(new MockHandler());
    unique_ptr<HandlerInterface> handler2(new MockHandler());
    unique_ptr<HandlerInterface> handler3(new MockHandler());
    auto receiver = make_shared<MockNonblockingReceiver>();
    EXPECT_CALL(*receiver, Enqueue_(handler1.get(), 0, 0)).WillOnce(Return(true));
    EXPECT_CALL(*receiver, Enqueue_(handler2.get(), 1, 1)).WillOnce(Return(true));
    EXPECT_CALL(*receiver, Enqueue_(handler3.get(), 2, 2)).WillOnce(Return(true));
    EXPECT_CALL(*receiver, connection_id()).WillRepeatedly(Return(1));

    auto value = make_shared<string>("value");

    auto mock_writer1 = new StrictMock<MockNonblockingPacketWriter>();
    EXPECT_CALL(*mock_writer1, Write())
        .WillOnce(Return(kInProgress))
        .WillOnce(Return(kInProgress))
        .WillOnce(Return(kDone)); // finishes on third call to Send(), proceeds to next writer

    auto mock_writer2 = new StrictMock<MockNonblockingPacketWriter>();
    EXPECT_CALL(*mock_writer2, Write())
        .WillOnce(Return(kInProgress))
        .WillOnce(Return(kInProgress))
        .WillOnce(Return(kDone)); // finishes on fifth call to Send()

    auto mock_writer3 = new StrictMock<MockNonblockingPacketWriter>();
    EXPECT_CALL(*mock_writer3, Write())
        .WillOnce(Return(kInProgress))
        .WillOnce(Return(kDone)); // finishes on sixth call to Send()

    auto mock_factory = new StrictMock<MockNonblockingPacketWriterFactory>();

    EXPECT_CALL(*mock_factory, CreateWriter_(_, _, _))
        .WillOnce(Return(mock_writer1))
        .WillOnce(Return(mock_writer2))
        .WillOnce(Return(mock_writer3));

    EXPECT_CALL(*socket_wrapper, getSSL()).WillRepeatedly(Return((SSL*)0));
    NonblockingSender sender(socket_wrapper, receiver,
        shared_ptr<NonblockingPacketWriterFactoryInterface>(mock_factory), hmac_provider_,
        options);
    sender.Enqueue(move(unique_ptr<Message>(new Message())), move(unique_ptr<Command>(new Command())), value, move(handler1), 0);
    ASSERT_EQ(kIoWait, sender.Send());
    sender.Enqueue(move(unique_ptr<Message>(new Message())), move(unique_ptr<Command>(new Command())), value, move(handler2), 1);
    ASSERT_EQ(kIoWait, sender.Send());
    sender.Enqueue(move(unique_ptr<Message>(new Message())), move(unique_ptr<Command>(new Command())), value, move(handler3), 2);
    ASSERT_EQ(kIoWait, sender.Send());
    ASSERT_EQ(kIoWait, sender.Send());
    ASSERT_EQ(kIoWait, sender.Send());
    ASSERT_EQ(kIdle, sender.Send());
}


TEST_F(NonblockingSenderTest, ErrorCausesAllEnqueuedRequestsToFail) {
    // If an I/O error occurs, all enqueued requests should fail
    ASSERT_EQ(0, close(fds_[0]));
    closed_read_end_ = true;

    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    EXPECT_CALL(*socket_wrapper, fd()).WillRepeatedly(Return(fds_[1]));
    ConnectionOptions options;
    options.user_id = 3;
    options.hmac_key = "key";
    auto receiver = make_shared<NiceMock<MockNonblockingReceiver>>();
    EXPECT_CALL(*socket_wrapper, getSSL()).WillRepeatedly(Return((SSL*)0));
    NonblockingSender sender(socket_wrapper, receiver, move(writer_factory_), hmac_provider_,
        options);

    unique_ptr<MockHandler> handler1(new MockHandler());
    unique_ptr<MockHandler> handler2(new MockHandler());
    EXPECT_CALL(*handler1, Error(KineticStatusEq(
            StatusCode::CLIENT_IO_ERROR, "I/O write error"), NULL));
    EXPECT_CALL(*handler2, Error(KineticStatusEq(
            StatusCode::CLIENT_IO_ERROR, "I/O write error"), NULL));
    unique_ptr<Message> message(new Message());
    unique_ptr<Command> command(new Command());
    sender.Enqueue(move(message), move(command), make_shared<string>(""), move(handler1), 0);
    message.reset(new Message());
    command.reset(new Command());
    sender.Enqueue(move(message), move(command), make_shared<string>(""), move(handler2), 1);
    ASSERT_EQ(kError, sender.Send());
}

TEST_F(NonblockingSenderTest, DestructorDeletesOutstandingRequests) {
    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    EXPECT_CALL(*socket_wrapper, fd()).WillRepeatedly(Return(fds_[1]));
    ConnectionOptions options;
    options.user_id = 3;
    options.hmac_key = "key";
    auto receiver = make_shared<NiceMock<MockNonblockingReceiver>>();
    EXPECT_CALL(*socket_wrapper, getSSL()).WillRepeatedly(Return((SSL*)0));
    NonblockingSender *sender = new NonblockingSender(socket_wrapper, receiver,
        move(writer_factory_), hmac_provider_, options);

    unique_ptr<MockHandler> handler1(new MockHandler());
    unique_ptr<MockHandler> handler2(new MockHandler());
    EXPECT_CALL(*handler1, Error(KineticStatusEq(StatusCode::CLIENT_SHUTDOWN,
        "Sender shutdown"), NULL));
    EXPECT_CALL(*handler2, Error(KineticStatusEq(StatusCode::CLIENT_SHUTDOWN,
        "Sender shutdown"), NULL));
    unique_ptr<Message> message(new Message());
    unique_ptr<Command> command(new Command());
    sender->Enqueue(move(message), move(command), make_shared<string>(""), move(handler1), 0);
    message.reset(new Message());
    command.reset(new Command());
    sender->Enqueue(move(message), move(command), make_shared<string>(""), move(handler2), 1);

    delete sender;
}

TEST_F(NonblockingSenderTest, EnqueueAndRemoveDoesntInvoke) {
    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    EXPECT_CALL(*socket_wrapper, fd()).WillRepeatedly(Return(fds_[1]));
    ConnectionOptions options;
    options.user_id = 3;
    options.hmac_key = "key";
    auto receiver = make_shared<StrictMock<MockNonblockingReceiver>>();
    EXPECT_CALL(*receiver, connection_id()).WillRepeatedly(Return(1));

    EXPECT_CALL(*socket_wrapper, getSSL()).WillRepeatedly(Return((SSL*) 0));
    NonblockingSender sender(socket_wrapper, receiver, move(writer_factory_), hmac_provider_,
        options);

    unique_ptr<MockHandler> handler1(new StrictMock<MockHandler>());
    unique_ptr<MockHandler> handler2(new StrictMock<MockHandler>());
    unique_ptr<Message> message1(new Message());
    unique_ptr<Message> message2(new Message());
    unique_ptr<Command> command1(new Command());
    unique_ptr<Command> command2(new Command());

    // message sequence 1 means 2nd handler
    EXPECT_CALL(*receiver, Enqueue_(handler2.get(), 1, 1)).WillOnce(Return(true));

    sender.Enqueue(move(message1), move(command1), make_shared<string>(""), move(handler1), 0);
    sender.Enqueue(move(message2), move(command2), make_shared<string>(""), move(handler2), 1);

    // first handler should not get called
    ASSERT_TRUE(sender.Remove(0));

    ASSERT_EQ(kIdle, sender.Send());
}

TEST_F(NonblockingSenderTest, RemoveInvalidKeyReturnsFalse) {
    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    EXPECT_CALL(*socket_wrapper, fd()).WillRepeatedly(Return(fds_[1]));
    ConnectionOptions options;
    options.user_id = 3;
    options.hmac_key = "key";
    auto receiver = make_shared<StrictMock<MockNonblockingReceiver>>();
    EXPECT_CALL(*receiver, connection_id()).WillRepeatedly(Return(1));

    EXPECT_CALL(*socket_wrapper, getSSL()).WillRepeatedly(Return((SSL*)0));
    NonblockingSender sender(socket_wrapper, receiver, move(writer_factory_), hmac_provider_,
        options);

    ASSERT_FALSE(sender.Remove(1234567890));

    ASSERT_EQ(kIdle, sender.Send());
}

}  // namespace kinetic
