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

#include "nonblocking_packet_receiver.h"

namespace kinetic {

using com::seagate::kinetic::client::proto::Message_Status_StatusCode;
using com::seagate::kinetic::client::proto::Message_Status_StatusCode_INVALID_STATUS_CODE;
using com::seagate::kinetic::client::proto::Message_Status_StatusCode_HMAC_FAILURE;
using com::seagate::kinetic::client::proto::Message_Status_StatusCode_NOT_AUTHORIZED;
using com::seagate::kinetic::client::proto::Message_Status_StatusCode_NOT_FOUND;
using com::seagate::kinetic::client::proto::Message_Status_StatusCode_INTERNAL_ERROR;
using com::seagate::kinetic::client::proto::Message_Status_StatusCode_HEADER_REQUIRED;
using com::seagate::kinetic::client::proto::Message_Status_StatusCode_SUCCESS;
using com::seagate::kinetic::client::proto::Message_Status_StatusCode_VERSION_FAILURE;
using com::seagate::kinetic::client::proto::Message_Status_StatusCode_VERSION_MISMATCH;
using com::seagate::kinetic::client::proto::Message_Status_StatusCode_SERVICE_BUSY;
using com::seagate::kinetic::client::proto::Message_Status_StatusCode_EXPIRED;
using com::seagate::kinetic::client::proto::Message_Status_StatusCode_DATA_ERROR;
using com::seagate::kinetic::client::proto::Message_Status_StatusCode_PERM_DATA_ERROR;
using com::seagate::kinetic::client::proto::Message_Status_StatusCode_REMOTE_CONNECTION_ERROR;
using com::seagate::kinetic::client::proto::Message_Status_StatusCode_NO_SPACE;
using com::seagate::kinetic::client::proto::Message_Status_StatusCode_NO_SUCH_HMAC_ALGORITHM;
using com::seagate::kinetic::client::proto::Message_Status_StatusCode_INVALID_REQUEST;

using std::string;
using std::shared_ptr;
using std::unique_ptr;
using std::move;
using std::make_pair;


StatusCode ConvertProtoStatus(Message_Status_StatusCode status) {
    switch (status) {
        case Message_Status_StatusCode_SUCCESS:
            return StatusCode::OK;
        case Message_Status_StatusCode_HMAC_FAILURE:
            return StatusCode::REMOTE_HMAC_ERROR;
        case Message_Status_StatusCode_NOT_AUTHORIZED:
            return StatusCode::REMOTE_NOT_AUTHORIZED;
        case Message_Status_StatusCode_VERSION_FAILURE:
            return StatusCode::REMOTE_CLUSTER_VERSION_MISMATCH;
        case Message_Status_StatusCode_INTERNAL_ERROR:
            return StatusCode::REMOTE_INTERNAL_ERROR;
        case Message_Status_StatusCode_HEADER_REQUIRED:
            return StatusCode::REMOTE_HEADER_REQUIRED;
        case Message_Status_StatusCode_NOT_FOUND:
            return StatusCode::REMOTE_NOT_FOUND;
        case Message_Status_StatusCode_VERSION_MISMATCH:
            return StatusCode::REMOTE_VERSION_MISMATCH;
        case Message_Status_StatusCode_SERVICE_BUSY:
            return StatusCode::REMOTE_SERVICE_BUSY;
        case Message_Status_StatusCode_EXPIRED:
            return StatusCode::REMOTE_EXPIRED;
        case Message_Status_StatusCode_DATA_ERROR:
            return StatusCode::REMOTE_DATA_ERROR;
        case Message_Status_StatusCode_PERM_DATA_ERROR:
            return StatusCode::REMOTE_PERM_DATA_ERROR;
        case Message_Status_StatusCode_REMOTE_CONNECTION_ERROR:
            return StatusCode::REMOTE_REMOTE_CONNECTION_ERROR;
        case Message_Status_StatusCode_NO_SPACE:
            return StatusCode::REMOTE_NO_SPACE;
        case Message_Status_StatusCode_NO_SUCH_HMAC_ALGORITHM:
            return StatusCode::REMOTE_NO_SUCH_HMAC_ALGORITHM;
        case Message_Status_StatusCode_INVALID_REQUEST:
            return StatusCode::REMOTE_INVALID_REQUEST;
        case Message_Status_StatusCode_INVALID_STATUS_CODE:
            return StatusCode::CLIENT_INTERNAL_ERROR;
        default:
            return StatusCode::REMOTE_OTHER_ERROR;
    }
}

KineticStatus GetKineticStatus(StatusCode code) {
    switch (code) {
        case StatusCode::CLIENT_IO_ERROR:
            return KineticStatus(code, "IO error");
        case StatusCode::CLIENT_SHUTDOWN:
            return KineticStatus(code, "Client shutdown");
        case StatusCode::PROTOCOL_ERROR_RESPONSE_NO_ACKSEQUENCE:
            return KineticStatus(code, "Response did not contain ack sequence");
        case StatusCode::CLIENT_RESPONSE_HMAC_VERIFICATION_ERROR:
            return KineticStatus(code, "Response HMAC verification failed");
        case StatusCode::REMOTE_HMAC_ERROR:
            return KineticStatus(code, "Remote HMAC verification failed");
        case StatusCode::REMOTE_NOT_AUTHORIZED:
            return KineticStatus(code, "Not authorized");
        case StatusCode::REMOTE_CLUSTER_VERSION_MISMATCH:
            return KineticStatus(code, "Cluster version mismatch");
        case StatusCode::REMOTE_INTERNAL_ERROR:
            return KineticStatus(code, "Remote internal error");
        case StatusCode::REMOTE_HEADER_REQUIRED:
            return KineticStatus(code, "Request requires a header to be set");
        case StatusCode::REMOTE_NOT_FOUND:
            return KineticStatus(code, "Key not found");
        case StatusCode::REMOTE_VERSION_MISMATCH:
            return KineticStatus(code, "Version mismatch");
        case StatusCode::REMOTE_SERVICE_BUSY:
            return KineticStatus(code, "Remote service is busy");
        case StatusCode::REMOTE_EXPIRED:
            return KineticStatus(code, "Remote timeout");
        case StatusCode::REMOTE_DATA_ERROR:
            return KineticStatus(code, "Remote transient data error");
        case StatusCode::REMOTE_PERM_DATA_ERROR:
            return KineticStatus(code, "Remote permanent data error");
        case StatusCode::REMOTE_REMOTE_CONNECTION_ERROR:
            return KineticStatus(code, "Remote connection to peer failed");
        case StatusCode::REMOTE_NO_SPACE:
            return KineticStatus(code, "No space left");
        case StatusCode::REMOTE_NO_SUCH_HMAC_ALGORITHM:
            return KineticStatus(code, "Unknown HMAC algorithm");
        default:
            return KineticStatus(code, "Internal Error");
    }
}

NonblockingReceiver::NonblockingReceiver(shared_ptr<SocketWrapperInterface> socket_wrapper,
    HmacProvider hmac_provider, const ConnectionOptions &connection_options)
: socket_wrapper_(socket_wrapper), hmac_provider_(hmac_provider),
connection_options_(connection_options), nonblocking_response_(NULL),
connection_id_(time(NULL)), handler_(NULL) {
    CHECK_NE(-1, connection_id_);
}

NonblockingReceiver::~NonblockingReceiver() {
    if (nonblocking_response_ != NULL) {
        delete nonblocking_response_;
    }
    CallAllErrorHandlers(KineticStatus(StatusCode::CLIENT_SHUTDOWN, "Receiver shutdown"));
}

bool NonblockingReceiver::Enqueue(shared_ptr<HandlerInterface> handler, google::int64 sequence,
    HandlerKey handler_key) {
    auto seq_to_handler_res = map_.insert(make_pair(sequence, make_pair(handler, handler_key)));
    if (!seq_to_handler_res.second) {
        LOG(WARNING) << "Found existing handler for sequence " << sequence;
        return false;
    }
    auto handler_key_to_seq_res =
        handler_to_message_seq_map_.insert(make_pair(handler_key, sequence));
    if (!handler_key_to_seq_res.second) {
        LOG(WARNING) << "Found existing sequence " << sequence << " for handler_key "
            << handler_key;
        auto handler_map_entry = map_.find(sequence);
        CHECK(handler_map_entry != map_.end())
                    << "Couldn't find just-inserted handler map entry for sequence "
                    << sequence;
        map_.erase(handler_map_entry);
        return false;
    }
    return true;
}


NonblockingPacketServiceStatus NonblockingReceiver::Receive() {
    while (true) {
        if (nonblocking_response_ == NULL) {
            if (map_.empty()) {
                return kIdle;
            }

            // Start working on the next thing in the request queue
            nonblocking_response_ = new NonblockingPacketReader(
                socket_wrapper_->fd(), &response_, value_);
        }

        NonblockingStringStatus status = nonblocking_response_->Read();
        if (status != kDone) {
            if (status == kInProgress) {
                return kIoWait;
            }
            CallAllErrorHandlers(KineticStatus(StatusCode::CLIENT_IO_ERROR, "I/O read error"));
            return kError;
        }

        // We're done receiving this response
        delete nonblocking_response_;
        nonblocking_response_ = NULL;
        if (!hmac_provider_.ValidateHmac(response_, connection_options_.hmac_key)) {
            LOG(INFO) << "Response HMAC mismatch";
            CallAllErrorHandlers(KineticStatus(StatusCode::CLIENT_RESPONSE_HMAC_VERIFICATION_ERROR,
                "Response HMAC mismatch"));
            return kError;
        }
        if (response_.command().header().has_connectionid()) {
            connection_id_ = response_.command().header().connectionid();
        }
        if (!response_.command().header().has_acksequence()) {
            LOG(INFO) << "Got response without an acksequence";
            CallAllErrorHandlers(KineticStatus(StatusCode::PROTOCOL_ERROR_RESPONSE_NO_ACKSEQUENCE,
                "Response had no acksequence"));
            return kError;
        }

        auto find_result = map_.find(response_.command().header().acksequence());
        if (find_result == map_.end()) {
            LOG(WARNING) << "Couldn't find a handler for acksequence " <<
                response_.command().header().acksequence();

            continue;
        }
        auto handler_pair = find_result->second;
        handler_ = handler_pair.first;
        map_.erase(find_result);

        CHECK_EQ((size_t) 1, handler_to_message_seq_map_.erase(handler_pair.second))
                << "Couldn't delete handler key to sequence entry for handler_key "
                << handler_pair.second;

        if (response_.command().status().code() == Message_Status_StatusCode_SUCCESS) {
            handler_->Handle(response_, move(value_));
        } else {
            handler_->Error(GetKineticStatus(ConvertProtoStatus(
                    response_.command().status().code())));
        }

        handler_.reset();
    }
}

int64_t NonblockingReceiver::connection_id() {
    return connection_id_;
}

void NonblockingReceiver::CallAllErrorHandlers(KineticStatus error) {
    if (handler_) {
        handler_->Error(error);
        handler_.reset();
    }

    auto iter = map_.begin();
    while (iter != map_.end()) {
        auto handler_pair = iter->second;
        shared_ptr<HandlerInterface> handler = handler_pair.first;
        HandlerKey handler_key = handler_pair.second;

        CHECK_EQ((size_t) 1, handler_to_message_seq_map_.erase(handler_key))
                << "Couldn't delete handler to sequence entry for handler_key " << handler_key;

        handler->Error(error);
        handler.reset();
        iter++;
    }
    map_.clear();
}

bool NonblockingReceiver::Remove(HandlerKey key) {
    auto handler_key_to_seq = handler_to_message_seq_map_.find(key);
    if (handler_key_to_seq == handler_to_message_seq_map_.end()) {
        return false;
    }

    google::protobuf::int64 seq = handler_key_to_seq->second;

    handler_to_message_seq_map_.erase(handler_key_to_seq);

    auto seq_to_handler = map_.find(seq);
    CHECK(seq_to_handler != map_.end()) << "Handler key " << handler_key_to_seq->first
        << " mapped to seq " << seq << " but no handler entry for that seq";

    auto handler_pair = seq_to_handler->second;
    auto handler_key = handler_pair.second;
    CHECK_EQ(handler_key, key);
    map_.erase(seq_to_handler);

    return true;
}

} // namespace kinetic
