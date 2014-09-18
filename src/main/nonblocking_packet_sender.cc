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

#include "nonblocking_packet_sender.h"

namespace kinetic {

using std::string;
using std::shared_ptr;
using std::unique_ptr;
using std::move;
using std::make_pair;

NonblockingSender::NonblockingSender(shared_ptr<SocketWrapperInterface> socket_wrapper,
    shared_ptr<NonblockingReceiverInterface> receiver,
    shared_ptr<NonblockingPacketWriterFactoryInterface> packet_writer_factory,
    HmacProvider hmac_provider,
    const ConnectionOptions &connection_options)
    : socket_wrapper_(socket_wrapper), receiver_(receiver),
    packet_writer_factory_(packet_writer_factory), hmac_provider_(hmac_provider),
    connection_options_(connection_options), sequence_number_(0), current_writer_(nullptr),
    handler_(nullptr) {}

void NonblockingSender::Enqueue(unique_ptr<Message> message, unique_ptr<Command> command,
    const shared_ptr<const string> value, unique_ptr<HandlerInterface> handler,
    HandlerKey handler_key) {

    command->mutable_header()->set_connectionid(receiver_->connection_id());
    command->mutable_header()->set_sequence(sequence_number_++);
    /* COMMAND PART OF MESSAGE IS FINALIZED */
    message->set_commandbytes(command->SerializeAsString());

    if(message->authtype() == com::seagate::kinetic::client::proto::Message_AuthType_HMACAUTH){
        message->mutable_hmacauth()->set_identity(connection_options_.user_id);
        message->mutable_hmacauth()->set_hmac(hmac_provider_.ComputeHmac(*message, connection_options_.hmac_key));
    }

    unique_ptr<Request> request(new Request());
    request->message = move(message);
    request->command = move(command);
    request->value = value;
    request->handler = move(handler);
    request->handler_key = handler_key;

    request_queue_.push_back(move(request));
}

NonblockingSender::~NonblockingSender() {
    while (!request_queue_.empty()) {
        unique_ptr<Request> request = move(request_queue_.front());
        request_queue_.pop_front();
        request->handler->Error(
            KineticStatus(StatusCode::CLIENT_SHUTDOWN, "Sender shutdown"),
            nullptr);
    }
}

NonblockingPacketServiceStatus NonblockingSender::Send() {
    while (true) {
        if (!current_writer_) {
            if (request_queue_.empty()) {
                return kIdle;
            }

            // Start working on the next thing on the request queue
            unique_ptr<Request> request = move(request_queue_.front());
            request_queue_.pop_front();
            message_sequence_ = request->command->header().sequence();
            handler_key_ = request->handler_key;
            current_writer_ = move(packet_writer_factory_->CreateWriter(socket_wrapper_,
                move(request->message), request->value));
            handler_ = move(request->handler);
        }

        NonblockingStringStatus status = current_writer_->Write();
        if (status != kDone) {
            if (status == kInProgress) {
                return kIoWait;
            }

            CHECK_EQ(kFailed, status);

            handler_->Error(
                    KineticStatus(StatusCode::CLIENT_IO_ERROR, "I/O write error"), nullptr);
            handler_.reset();

            while (!request_queue_.empty()) {
                unique_ptr<Request> request = move(request_queue_.front());
                request_queue_.pop_front();
                request->handler->Error(KineticStatus(StatusCode::CLIENT_IO_ERROR,
                    "I/O write error"), nullptr);
            }
            return kError;
        }

        // We're done with this request
        current_writer_.reset();

        if (!receiver_->Enqueue(handler_, message_sequence_, handler_key_)) {
            LOG(WARNING) << "Could not enqueue handler; already had a handler for sequence " <<
                message_sequence_ << " and handler key " << handler_key_;
            handler_->Error(KineticStatus(StatusCode::CLIENT_INTERNAL_ERROR,
                "Could not enqueue handler"), nullptr);
        }
        handler_.reset();
    }
}

bool NonblockingSender::Remove(HandlerKey key) {
    for (auto it = request_queue_.begin(); it != request_queue_.end(); it++) {
        if ((*it)->handler_key == key) {
            request_queue_.erase(it);
            return true;
        }
    }
    return false;
}

} // namespace kinetic
