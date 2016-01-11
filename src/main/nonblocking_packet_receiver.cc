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

#include "nonblocking_packet_receiver.h"
#include <exception>
#include <stdexcept>
#include <ctime>

namespace kinetic {

using com::seagate::kinetic::client::proto::Message_AuthType_UNSOLICITEDSTATUS;
using com::seagate::kinetic::client::proto::Command_Status_StatusCode_SUCCESS;
using std::string;
using std::shared_ptr;
using std::unique_ptr;
using std::move;
using std::make_pair;



KineticStatus GetKineticStatus(StatusCode code, int64_t expected_cluster_version) {
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
            return KineticStatus(code, "Cluster version mismatch", expected_cluster_version);
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
        case StatusCode::REMOTE_NESTED_OPERATION_ERRORS:
            return KineticStatus(code, "Operation completed but has nested errors");
        default:
            return KineticStatus(code, "Internal Error");
    }
}

class HandshakeHandler : public HandlerInterface {
 public:
    bool done;
    bool success;
    HandshakeHandler() : done(false), success(false) {}
    void Handle(const Command &response, unique_ptr<const string> value) {
        done = success = true;
    }
    void Error(KineticStatus error, Command const * const response) {
        done = true;
    }
};

NonblockingReceiver::NonblockingReceiver(shared_ptr<SocketWrapperInterface> socket_wrapper,
    HmacProvider hmac_provider, const ConnectionOptions &connection_options)
: socket_wrapper_(socket_wrapper), hmac_provider_(hmac_provider),
connection_options_(connection_options), nonblocking_response_(NULL),
connection_id_(0), handler_(NULL) {
    shared_ptr<HandshakeHandler> hh = std::make_shared<HandshakeHandler>();
    map_.insert(make_pair(-1, make_pair(hh, -1)));
    handler_to_message_seq_map_.insert(make_pair(-1, -1));

    auto start = std::time(0);

    while (true) {
        if (Receive() == kError)
            break;
        if (hh->done)
            break;
        auto now = std::time(0);
        if (now-start > 30)
            break;
    }
    if (!hh->success)
        throw std::runtime_error("Could not complete handshake.");
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
                socket_wrapper_, &message_, value_);
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

        if (message_.has_hmacauth())
        if (!hmac_provider_.ValidateHmac(message_, connection_options_.hmac_key)) {
            LOG(INFO) << "Response HMAC mismatch";
            CallAllErrorHandlers(KineticStatus(StatusCode::CLIENT_RESPONSE_HMAC_VERIFICATION_ERROR,
                "Response HMAC mismatch"));
            return kIdle;
        }
        if (!command_.ParseFromString(message_.commandbytes())) {
            CallAllErrorHandlers(KineticStatus(StatusCode::CLIENT_IO_ERROR, "I/O read error parsing proto::Command"));
            return kError;
        }
        if (command_.header().has_connectionid()) {
            connection_id_ = command_.header().connectionid();
        }

        if (message_.authtype() == Message_AuthType_UNSOLICITEDSTATUS)
            command_.mutable_header()->set_acksequence(-1);

        if (!command_.header().has_acksequence()) {
            LOG(INFO) << "Got response without an acksequence";
            CallAllErrorHandlers(KineticStatus(StatusCode::PROTOCOL_ERROR_RESPONSE_NO_ACKSEQUENCE,
                "Response had no acksequence"));
            return kIdle;
        }

        auto find_result = map_.find(command_.header().acksequence());
        if (find_result == map_.end()) {
            LOG(WARNING) << "Couldn't find a handler for acksequence " <<
                command_.header().acksequence();
            continue;
        }
        auto handler_pair = find_result->second;
        handler_ = handler_pair.first;
        map_.erase(find_result);

        CHECK_EQ((size_t) 1, handler_to_message_seq_map_.erase(handler_pair.second))
                << "Couldn't delete handler key to sequence entry for handler_key "
                << handler_pair.second;

        if (command_.status().code() == Command_Status_StatusCode_SUCCESS) {
            handler_->Handle(command_, move(value_));
        } else {
            handler_->Error(GetKineticStatus(ConvertFromProtoStatus(
                    command_.status().code()), command_.header().clusterversion()),
                    &command_);
        }

        handler_.reset();
    }
}

int64_t NonblockingReceiver::connection_id() {
    return connection_id_;
}

void NonblockingReceiver::CallAllErrorHandlers(KineticStatus error) {
    if (handler_) {
        handler_->Error(error, nullptr);
        handler_.reset();
    }

    auto iter = map_.begin();
    while (iter != map_.end()) {
        auto handler_pair = iter->second;
        shared_ptr<HandlerInterface> handler = handler_pair.first;
        HandlerKey handler_key = handler_pair.second;

        CHECK_EQ((size_t) 1, handler_to_message_seq_map_.erase(handler_key))
                << "Couldn't delete handler to sequence entry for handler_key " << handler_key;

        handler->Error(error, nullptr);
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
