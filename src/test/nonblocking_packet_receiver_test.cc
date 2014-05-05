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
using com::seagate::kinetic::client::proto::Message_MessageType_GET_RESPONSE;
using com::seagate::kinetic::client::proto::Message_Status_StatusCode_SUCCESS;

using std::string;
using std::make_shared;

class NonblockingReceiverTest : public ::testing::Test {
    protected:
    // Create a pipe that we can use to feed data to the NonblockingReceiver
    void SetUp() {
        ASSERT_EQ(0, pipe(fds_));
    }

    void TearDown() {
        ASSERT_EQ(0, close(fds_[0]));
        ASSERT_EQ(0, close(fds_[1]));
    }

    // Write a packet into the pipe
    void WritePacket(const Message &message, const std::string &value) {
        std::string serialized_message;
        ASSERT_TRUE(message.SerializeToString(&serialized_message));
        ASSERT_EQ(1, write(fds_[1], "F", 1));
        uint32_t message_length = htonl(serialized_message.size());
        ASSERT_EQ(4, write(fds_[1], reinterpret_cast<char *>(&message_length), 4));
        uint32_t value_length = htonl(value.size());
        ASSERT_EQ(4, write(fds_[1], reinterpret_cast<char *>(&value_length), 4));
        ASSERT_EQ(static_cast<ssize_t>(serialized_message.size()), write(fds_[1],
            serialized_message.data(), serialized_message.size()));
        ASSERT_EQ(static_cast<ssize_t>(value.size()), write(fds_[1], value.data(), value.size()));
    }

    int fds_[2];
    HmacProvider hmac_provider_;

    void defaultReceiverSetup(Message &message,
            shared_ptr<MockSocketWrapperInterface> socket_wrapper, ConnectionOptions &options) {
        message.mutable_command()->mutable_status()->set_code(Message_Status_StatusCode_SUCCESS);
        message.mutable_command()->mutable_header()->set_acksequence(33);
        message.set_hmac(hmac_provider_.ComputeHmac(message, "key"));

        WritePacket(message, "value");
        EXPECT_CALL(*socket_wrapper, fd()).WillRepeatedly(Return(fds_[0]));

        options.user_id = 3;
        options.hmac_key = "key";
    }
};

TEST_F(NonblockingReceiverTest, SimpleMessageAndValue) {
    Message message;
    message.mutable_command()->mutable_status()->set_code(Message_Status_StatusCode_SUCCESS);
    message.mutable_command()->mutable_header()->set_acksequence(33);
    message.set_hmac(hmac_provider_.ComputeHmac(message, "key"));
    WritePacket(message, "value");
    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    EXPECT_CALL(*socket_wrapper, fd()).WillRepeatedly(Return(fds_[0]));
    ConnectionOptions options;
    options.user_id = 3;
    options.hmac_key = "key";
    NonblockingReceiver receiver(socket_wrapper, hmac_provider_, options);

    auto handler = make_shared<MockHandler>();
    EXPECT_CALL(*handler, Handle_(_, "value"));
    ASSERT_TRUE(receiver.Enqueue(handler, 33, 0));
    ASSERT_EQ(kIdle, receiver.Receive());
}

TEST_F(NonblockingReceiverTest, ReceiveResponsesOutOfOrder) {
    Message message;
    message.mutable_command()->mutable_status()->set_code(Message_Status_StatusCode_SUCCESS);
    message.mutable_command()->mutable_header()->set_acksequence(44);
    message.set_hmac(hmac_provider_.ComputeHmac(message, "key"));
    WritePacket(message, "value2");

    message.mutable_command()->mutable_header()->set_acksequence(33);
    message.set_hmac(hmac_provider_.ComputeHmac(message, "key"));
    WritePacket(message, "value");

    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    EXPECT_CALL(*socket_wrapper, fd()).WillRepeatedly(Return(fds_[0]));
    ConnectionOptions options;
    options.user_id = 3;
    options.hmac_key = "key";
    NonblockingReceiver receiver(socket_wrapper, hmac_provider_, options);

    auto handler1 = make_shared<MockHandler>();
    auto handler2 = make_shared<MockHandler>();
    EXPECT_CALL(*handler1, Handle_(_, "value"));
    EXPECT_CALL(*handler2, Handle_(_, "value2"));
    ASSERT_TRUE(receiver.Enqueue(handler1, 33, 0));
    ASSERT_TRUE(receiver.Enqueue(handler2, 44, 1));
    ASSERT_EQ(kIdle, receiver.Receive());
}

TEST_F(NonblockingReceiverTest, CallsErrorWhenNoAckSequence) {
    Message message;
    message.mutable_command()->mutable_status()->set_code(Message_Status_StatusCode_SUCCESS);
    message.set_hmac(hmac_provider_.ComputeHmac(message, "key"));
    WritePacket(message, "value");
    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    EXPECT_CALL(*socket_wrapper, fd()).WillRepeatedly(Return(fds_[0]));
    ConnectionOptions options;
    options.user_id = 3;
    options.hmac_key = "key";
    NonblockingReceiver receiver(socket_wrapper, hmac_provider_, options);

    auto handler = make_shared<MockHandler>();
    EXPECT_CALL(*handler, Error(KineticStatusEq(StatusCode::PROTOCOL_ERROR_RESPONSE_NO_ACKSEQUENCE,
        "Response had no acksequence")));
    ASSERT_TRUE(receiver.Enqueue(handler, 33, 0));
    ASSERT_EQ(kError, receiver.Receive());
}

TEST_F(NonblockingReceiverTest, SetsConnectionId) {
    // The receiver should adjust its connection ID to whatever the server
    // decides it should be.
    Message message;
    message.mutable_command()->mutable_header()->set_connectionid(42);
    message.set_hmac(hmac_provider_.ComputeHmac(message, "key"));
    WritePacket(message, "");
    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    EXPECT_CALL(*socket_wrapper, fd()).WillRepeatedly(Return(fds_[0]));
    ConnectionOptions options;
    options.user_id = 3;
    options.hmac_key = "key";
    NonblockingReceiver receiver(socket_wrapper, hmac_provider_, options);

    auto handler = make_shared<NiceMock<MockHandler>>();
    ASSERT_TRUE(receiver.Enqueue(handler, 0, 0));
    receiver.Receive();
    ASSERT_EQ(42, receiver.connection_id());
}

TEST_F(NonblockingReceiverTest, HandlesReadError) {
    const char header[] = { 'E' };  // invalid magic character
    ASSERT_EQ(static_cast<ssize_t>(sizeof(header)), write(fds_[1], header, sizeof(header)));
    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    EXPECT_CALL(*socket_wrapper, fd()).WillRepeatedly(Return(fds_[0]));
    ConnectionOptions options;
    options.user_id = 3;
    options.hmac_key = "key";
    NonblockingReceiver receiver(socket_wrapper, hmac_provider_, options);

    auto handler = make_shared<MockHandler>();
    EXPECT_CALL(*handler, Error(KineticStatusEq(StatusCode::CLIENT_IO_ERROR, "I/O read error")));
    ASSERT_TRUE(receiver.Enqueue(handler, 0, 0));
    ASSERT_EQ(kError, receiver.Receive());
}

TEST_F(NonblockingReceiverTest, HandlesHmacError) {
    Message message;
    message.set_hmac("wrong_hmac");
    WritePacket(message, "");
    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    EXPECT_CALL(*socket_wrapper, fd()).WillRepeatedly(Return(fds_[0]));
    ConnectionOptions options;
    options.user_id = 3;
    options.hmac_key = "key";
    NonblockingReceiver receiver(socket_wrapper, hmac_provider_, options);

    auto handler = make_shared<MockHandler>();
    EXPECT_CALL(*handler, Error(KineticStatusEq(StatusCode::CLIENT_RESPONSE_HMAC_VERIFICATION_ERROR,
        "Response HMAC mismatch")));
    ASSERT_TRUE(receiver.Enqueue(handler, 0, 0));
    ASSERT_EQ(kError, receiver.Receive());
}

TEST_F(NonblockingReceiverTest, ErrorCausesAllEnqueuedRequestsToFail) {
    // If we encounter an error such as an invalid magic character or incorrect
    // HMAC, there's not much point in continuing the connection, so the
    // receiver should execute the failure callback on all enqueued requests.
    const char header[] = { 'E' };  // invalid magic character
    ASSERT_EQ(static_cast<ssize_t>(sizeof(header)), write(fds_[1], header, sizeof(header)));
    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    EXPECT_CALL(*socket_wrapper, fd()).WillRepeatedly(Return(fds_[0]));
    ConnectionOptions options;
    options.user_id = 3;
    options.hmac_key = "key";
    NonblockingReceiver receiver(socket_wrapper, hmac_provider_, options);

    auto handler1 = make_shared<MockHandler>();
    auto handler2 = make_shared<MockHandler>();
    EXPECT_CALL(*handler1, Error(KineticStatusEq(StatusCode::CLIENT_IO_ERROR, "I/O read error")));
    EXPECT_CALL(*handler2, Error(KineticStatusEq(StatusCode::CLIENT_IO_ERROR, "I/O read error")));
    ASSERT_TRUE(receiver.Enqueue(handler1, 0, 0));
    ASSERT_TRUE(receiver.Enqueue(handler2, 1, 1));
    ASSERT_EQ(kError, receiver.Receive());
}

TEST_F(NonblockingReceiverTest, DestructorDeletesOutstandingRequests) {
    // When the receiver's destructor is called, it should execute the error
    // callback on any outstanding requests and also delete their handlers.
    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    ConnectionOptions options;
    options.user_id = 3;
    options.hmac_key = "key";
    NonblockingReceiver *receiver = new NonblockingReceiver(socket_wrapper,
        hmac_provider_, options);

    auto handler1 = make_shared<MockHandler>();
    auto handler2 = make_shared<MockHandler>();
    EXPECT_CALL(*handler1, Error(KineticStatusEq(StatusCode::CLIENT_SHUTDOWN,
        "Receiver shutdown")));
    EXPECT_CALL(*handler2, Error(KineticStatusEq(StatusCode::CLIENT_SHUTDOWN,
        "Receiver shutdown")));
    ASSERT_TRUE(receiver->Enqueue(handler1, 0, 0));
    ASSERT_TRUE(receiver->Enqueue(handler2, 1, 1));
    delete receiver;
}

TEST_F(NonblockingReceiverTest, RemoveInvalidHandlerKeyDoesntPerturbNormalOperation) {
    Message message;
    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    ConnectionOptions options;
    defaultReceiverSetup(message, socket_wrapper, options);
    NonblockingReceiver receiver(socket_wrapper, hmac_provider_, options);

    auto handler = make_shared<StrictMock<MockHandler>>();
    EXPECT_CALL(*handler, Handle_(_, "value"));
    ASSERT_TRUE(receiver.Enqueue(handler, 33, 0));

    ASSERT_FALSE(receiver.Remove(9732412));

    // existing handler should still be called

    ASSERT_EQ(kIdle, receiver.Receive());
}


TEST_F(NonblockingReceiverTest, RemoveValidHandlerKeyDeregistersHandler) {
    Message message;
    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    ConnectionOptions options;
    defaultReceiverSetup(message, socket_wrapper, options);
    NonblockingReceiver receiver(socket_wrapper, hmac_provider_, options);

    auto handler1 = make_shared<StrictMock<MockHandler>>();
    auto handler2 = make_shared<StrictMock<MockHandler>>();
    EXPECT_CALL(*handler1, Handle_(_, "value"));
    ASSERT_TRUE(receiver.Enqueue(handler1, 33, 0));
    ASSERT_TRUE(receiver.Enqueue(handler2, 34, 1));

    ASSERT_TRUE(receiver.Remove(1));

    // existing handler should still be called

    ASSERT_EQ(kIdle, receiver.Receive());
}

TEST_F(NonblockingReceiverTest, EnqueueReturnsFalseWhenReUsingHandlerKey) {
    Message message;
    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    ConnectionOptions options;
    defaultReceiverSetup(message, socket_wrapper, options);
    NonblockingReceiver receiver(socket_wrapper, hmac_provider_, options);

    auto handler1 = make_shared<StrictMock<MockHandler>>();
    auto handler2 = make_shared<StrictMock<MockHandler>>();
    EXPECT_CALL(*handler1, Handle_(_, "value"));
    ASSERT_TRUE(receiver.Enqueue(handler1, 33, 0));
    ASSERT_FALSE(receiver.Enqueue(handler2, 34, 0));

    // existing handler should still be called

    ASSERT_EQ(kIdle, receiver.Receive());
}

TEST_F(NonblockingReceiverTest, EnqueueDoesntSaveHandlerWhenErroneouslyReUsingHandlerKey) {
    Message message;
    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    ConnectionOptions options;
    defaultReceiverSetup(message, socket_wrapper, options);
    NonblockingReceiver receiver(socket_wrapper, hmac_provider_, options);

    auto handler1 = make_shared<StrictMock<MockHandler>>();
    auto handler2 = make_shared<StrictMock<MockHandler>>();
    EXPECT_CALL(*handler1, Handle_(_, "value"));
    ASSERT_TRUE(receiver.Enqueue(handler1, 33, 0));

    // the handler should not be recorded since the handler key is a dup
    ASSERT_FALSE(receiver.Enqueue(handler2, 34, 0));

    // handler1 should still be called

    ASSERT_EQ(kIdle, receiver.Receive());
}

TEST_F(NonblockingReceiverTest,
        EnqueueWithDuplicateHandlerKeyDoesntPreventSubsequentMessageSeqReuse) {
    Message message;
    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    ConnectionOptions options;
    defaultReceiverSetup(message, socket_wrapper, options);
    NonblockingReceiver receiver(socket_wrapper, hmac_provider_, options);

    auto handler1 = make_shared<StrictMock<MockHandler>>();
    auto handler2 = make_shared<StrictMock<MockHandler>>();
    EXPECT_CALL(*handler1, Handle_(_, "value"));
    ASSERT_TRUE(receiver.Enqueue(handler1, 33, 0));

    // the handler should not be recorded since the handler key is a dup
    ASSERT_FALSE(receiver.Enqueue(handler2, 34, 0));

    // handler1 should still be called

    ASSERT_EQ(kIdle, receiver.Receive());

    // this handler uses the same message seq as the one whose handler key prevented enqueuing
    auto handler3 = make_shared<StrictMock<MockHandler>>();
    EXPECT_CALL(*handler3, Handle_(_, "value2"));
    ASSERT_TRUE(receiver.Enqueue(handler3, 34, 1));

    message.mutable_command()->mutable_status()->set_code(Message_Status_StatusCode_SUCCESS);
    message.mutable_command()->mutable_header()->set_acksequence(34);
    message.set_hmac(hmac_provider_.ComputeHmac(message, "key"));
    WritePacket(message, "value2");

    ASSERT_EQ(kIdle, receiver.Receive());
}

TEST_F(NonblockingReceiverTest, ExecutingHandlerRemovesHandlerKey) {
    Message message;
    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    ConnectionOptions options;
    defaultReceiverSetup(message, socket_wrapper, options);
    NonblockingReceiver receiver(socket_wrapper, hmac_provider_, options);

    auto handler = make_shared<StrictMock<MockHandler>>();
    EXPECT_CALL(*handler, Handle_(_, "value"));
    ASSERT_TRUE(receiver.Enqueue(handler, 33, 0));

    ASSERT_EQ(kIdle, receiver.Receive());

    ASSERT_FALSE(receiver.Remove(0));
}

}  // namespace kinetic
