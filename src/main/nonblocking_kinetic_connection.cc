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

#include "kinetic/nonblocking_kinetic_connection.h"
#include "nonblocking_packet_service.h"
#include <memory>
#include <glog/logging.h>

namespace kinetic {

using com::seagate::kinetic::client::proto::Message_MessageType_DELETE;
using com::seagate::kinetic::client::proto::Message_MessageType_GET;
using com::seagate::kinetic::client::proto::Message_MessageType_GETNEXT;
using com::seagate::kinetic::client::proto::Message_MessageType_GETPREVIOUS;
using com::seagate::kinetic::client::proto::Message_MessageType_GETKEYRANGE;
using com::seagate::kinetic::client::proto::Message_MessageType_GETVERSION;
using com::seagate::kinetic::client::proto::Message_MessageType_NOOP;
using com::seagate::kinetic::client::proto::Message_MessageType_PUT;
using com::seagate::kinetic::client::proto::Message_MessageType_SETUP;
using com::seagate::kinetic::client::proto::Message_MessageType_GETLOG;
using com::seagate::kinetic::client::proto::Message_MessageType_SECURITY;
using com::seagate::kinetic::client::proto::Message_MessageType_PEER2PEERPUSH;
using com::seagate::kinetic::client::proto::Message_Status_StatusCode_NOT_AUTHORIZED;
using com::seagate::kinetic::client::proto::Message_Status_StatusCode_NOT_FOUND;
using com::seagate::kinetic::client::proto::Message_Status_StatusCode_SUCCESS;
using com::seagate::kinetic::client::proto::Message_GetLog_Type_UTILIZATIONS;
using com::seagate::kinetic::client::proto::Message_GetLog_Type_TEMPERATURES;
using com::seagate::kinetic::client::proto::Message_GetLog_Type_CAPACITIES;
using com::seagate::kinetic::client::proto::Message_GetLog_Type_CONFIGURATION;
using com::seagate::kinetic::client::proto::Message_GetLog_Type_STATISTICS;
using com::seagate::kinetic::client::proto::Message_GetLog_Type_MESSAGES;
using com::seagate::kinetic::client::proto::Message_GetLog_Type_LIMITS;
using com::seagate::kinetic::client::proto::Message_Security_ACL;
using com::seagate::kinetic::client::proto::Message_Security_ACL_Permission;
using com::seagate::kinetic::client::proto::Message_Security_ACL_Scope;
using com::seagate::kinetic::client::proto::Message_Security_ACL_HMACAlgorithm_HmacSHA1;
using com::seagate::kinetic::client::proto::Message_Status;
using com::seagate::kinetic::client::proto::Message_P2POperation;
using com::seagate::kinetic::client::proto::Message_Synchronization;
using com::seagate::kinetic::client::proto::Message_Synchronization_FLUSH;
using com::seagate::kinetic::client::proto::Message_Synchronization_WRITEBACK;
using com::seagate::kinetic::client::proto::Message_Synchronization_WRITETHROUGH;

using std::shared_ptr;
using std::string;
using std::make_shared;
using std::unique_ptr;
using std::iterator;
using std::move;

GetHandler::GetHandler(const shared_ptr<GetCallbackInterface> callback)
    : callback_(callback) {}

void GetHandler::Handle(const Message &response, unique_ptr<const string> value) {
    unique_ptr<KineticRecord> record(new KineticRecord(shared_ptr<const string>(value.release()),
         make_shared<string>(response.command().body().keyvalue().dbversion()),
         make_shared<string>(response.command().body().keyvalue().tag()),
            response.command().body().keyvalue().algorithm()));
    callback_->Success(response.command().body().keyvalue().key(), move(record));
}

void GetHandler::Error(KineticStatus error) {
    callback_->Failure(error);
}

GetVersionHandler::GetVersionHandler(const shared_ptr<GetVersionCallbackInterface> callback)
    : callback_(callback) {}

void GetVersionHandler::Handle(const Message &response, unique_ptr<const string> value) {
    callback_->Success(response.command().body().keyvalue().dbversion());
}

void GetVersionHandler::Error(KineticStatus error) {
    callback_->Failure(error);
}

GetKeyRangeHandler::GetKeyRangeHandler(const shared_ptr<GetKeyRangeCallbackInterface> callback)
    : callback_(callback) {}

void GetKeyRangeHandler::Handle(const Message &response, unique_ptr<const string> value) {
    int raw_size = response.command().body().range().key_size();
    CHECK_GE(raw_size, 0);
    size_t key_size = (size_t) raw_size;

    unique_ptr<vector<string>> keys(new vector<string>);
    keys->reserve(key_size);

    for (size_t i = 0; i < key_size; i++) {
        keys->push_back(response.command().body().range().key(i));
    }

    callback_->Success(move(keys));
}

void GetKeyRangeHandler::Error(KineticStatus error) {
    callback_->Failure(error);
}

PutHandler::PutHandler(const shared_ptr<PutCallbackInterface> callback)
    : callback_(callback) {}

void PutHandler::Handle(const Message &response, unique_ptr<const string> value) {
    callback_->Success();
}

void PutHandler::Error(KineticStatus error) {
    callback_->Failure(error);
}

SimpleHandler::SimpleHandler(const shared_ptr<SimpleCallbackInterface> callback)
    : callback_(callback) {}

void SimpleHandler::Handle(const Message &response, unique_ptr<const string> value) {
    callback_->Success();
}

void SimpleHandler::Error(KineticStatus error) {
    callback_->Failure(error);
}

GetLogHandler::GetLogHandler(const shared_ptr<GetLogCallbackInterface> callback)
    : callback_(callback) {}

void GetLogHandler::Handle(const Message& response, unique_ptr<const string> value) {
    auto getlog = response.command().body().getlog();
    auto configuration = getlog.configuration();
    unique_ptr<DriveLog> drive_log(new DriveLog);
    drive_log->configuration.vendor = configuration.vendor();
    drive_log->configuration.model = configuration.model();
    drive_log->configuration.serial_number = configuration.serialnumber();
    drive_log->configuration.version = configuration.version();
    drive_log->configuration.source_hash = configuration.sourcehash();
    drive_log->configuration.compilation_date = configuration.compilationdate();
    drive_log->configuration.port = configuration.port();
    drive_log->configuration.tls_port = configuration.tlsport();

    auto capacity = getlog.capacity();
    drive_log->capacity.nominal_capacity_in_bytes = capacity.nominalcapacityinbytes();
    drive_log->capacity.portion_full = capacity.portionfull();

    auto limits = getlog.limits();
    drive_log->limits.max_key_size = limits.maxkeysize();
    drive_log->limits.max_value_size = limits.maxvaluesize();
    drive_log->limits.max_version_size = limits.maxversionsize();
    drive_log->limits.max_tag_size = limits.maxtagsize();
    drive_log->limits.max_connections = limits.maxconnections();
    drive_log->limits.max_outstanding_read_requests = limits.maxoutstandingreadrequests();
    drive_log->limits.max_outstanding_write_requests = limits.maxoutstandingwriterequests();
    drive_log->limits.max_message_size = limits.maxmessagesize();

    for (int i = 0; i < getlog.statistics_size(); i++) {
        OperationStatistic statistic;
        statistic.name = Message_MessageType_Name(getlog.statistics(i).messagetype());
        statistic.count = getlog.statistics(i).count();
        statistic.bytes = getlog.statistics(i).bytes();
        drive_log->operation_statistics.push_back(statistic);
    }

    for (int i = 0; i < getlog.utilization_size(); i++) {
        Utilization utilization;
        utilization.name = getlog.utilization(i).name();
        utilization.percent = getlog.utilization(i).value();
        drive_log->utilizations.push_back(utilization);
    }

    for (int i = 0; i < getlog.temperature_size(); i++) {
        Temperature temperature;

        temperature.name = getlog.temperature(i).name();
        temperature.current_degc = getlog.temperature(i).current();
        temperature.target_degc = getlog.temperature(i).target();
        temperature.max_degc = getlog.temperature(i).maximum();
        temperature.min_degc = getlog.temperature(i).minimum();

        drive_log->temperatures.push_back(temperature);
    }

    drive_log->messages = getlog.messages();

    callback_->Success(move(drive_log));
}

void GetLogHandler::Error(KineticStatus error) {
    callback_->Failure(error);
}

P2PPushHandler::P2PPushHandler(const shared_ptr<P2PPushCallbackInterface> callback)
    : callback_(callback) {}

void P2PPushHandler::Handle(const Message& response, unique_ptr<const string> value) {
    unique_ptr<vector<KineticStatus>> statuses(new vector<KineticStatus>());

    Message_P2POperation const &p2pop = response.command().body().p2poperation();
    statuses->reserve(p2pop.operation_size());

    for (int i = 0; i < p2pop.operation_size(); i++) {
        Message_Status const &status = p2pop.operation(i).status();

        statuses->push_back(
                KineticStatus(ConvertProtoStatus(status.code()), status.statusmessage()));
    }

    callback_->Success(move(statuses));
}

void P2PPushHandler::Error(KineticStatus error) {
    callback_->Failure(error);
}

NonblockingKineticConnection::NonblockingKineticConnection(
        NonblockingPacketServiceInterface *service)
    : service_(service), empty_str_(make_shared<string>("")), cluster_version_(0) {}

NonblockingKineticConnection::~NonblockingKineticConnection() {
    delete service_;
}

bool NonblockingKineticConnection::Run(fd_set *read_fds, fd_set *write_fds, int *nfds) {
    return service_->Run(read_fds, write_fds, nfds);
}

void NonblockingKineticConnection::SetClientClusterVersion(int64_t cluster_version) {
    cluster_version_ = cluster_version;
}

unique_ptr<Message> NonblockingKineticConnection::NewMessage(Message_MessageType message_type) {
    unique_ptr<Message> msg(new Message());
    msg->mutable_command()->mutable_header()->set_messagetype(message_type);
    msg->mutable_command()->mutable_header()->set_clusterversion(cluster_version_);

    return move(msg);
}

HandlerKey NonblockingKineticConnection::NoOp(const shared_ptr<SimpleCallbackInterface> callback) {
    unique_ptr<SimpleHandler> handler(new SimpleHandler(callback));
    unique_ptr<Message> request = NewMessage(Message_MessageType_NOOP);
    return service_->Submit(move(request), empty_str_, move(handler));
}

HandlerKey NonblockingKineticConnection::Get(const shared_ptr<const string> key,
    const shared_ptr<GetCallbackInterface> callback) {
    return GenericGet(key, callback, Message_MessageType_GET);
}

HandlerKey NonblockingKineticConnection::Get(const string key,
        const shared_ptr<GetCallbackInterface> callback) {
    return this->Get(make_shared<string>(key), callback);
}

HandlerKey NonblockingKineticConnection::GetNext(const shared_ptr<const string> key,
    const shared_ptr<GetCallbackInterface> callback) {
    return GenericGet(key, callback, Message_MessageType_GETNEXT);
}

HandlerKey NonblockingKineticConnection::GetNext(const string key,
    const shared_ptr<GetCallbackInterface> callback) {
    return this->GetNext(make_shared<string>(key), callback);
}

HandlerKey NonblockingKineticConnection::GetPrevious(const shared_ptr<const string> key,
    const shared_ptr<GetCallbackInterface> callback) {
    return GenericGet(key, callback, Message_MessageType_GETPREVIOUS);
}

HandlerKey NonblockingKineticConnection::GetPrevious(const string key,
    const shared_ptr<GetCallbackInterface> callback) {
    return this->GetPrevious(make_shared<string>(key), callback);
}

HandlerKey NonblockingKineticConnection::GetVersion(const shared_ptr<const string> key,
    const shared_ptr<GetVersionCallbackInterface> callback) {
    unique_ptr<GetVersionHandler> handler(new GetVersionHandler(callback));
    unique_ptr<Message> request = NewMessage(Message_MessageType_GETVERSION);
    request->mutable_command()->mutable_body()->mutable_keyvalue()->set_key(*key);
    return service_->Submit(move(request), empty_str_, move(handler));
}

HandlerKey NonblockingKineticConnection::GetVersion(const string key,
    const shared_ptr<GetVersionCallbackInterface> callback) {
    return this->GetVersion(make_shared<string>(key), callback);
}

HandlerKey NonblockingKineticConnection::GetKeyRange(const shared_ptr<const string> start_key,
        bool start_key_inclusive,
        const shared_ptr<const string> end_key,
        bool end_key_inclusive,
        bool reverse_results,
        int32_t max_results,
        const shared_ptr<GetKeyRangeCallbackInterface> callback) {
    unique_ptr<GetKeyRangeHandler> handler(new GetKeyRangeHandler(callback));
    unique_ptr<Message> request = NewMessage(Message_MessageType_GETKEYRANGE);

    request->mutable_command()->mutable_body()->mutable_range()->set_startkey(*start_key);
    request->mutable_command()->mutable_body()->mutable_range()->set_startkeyinclusive(
            start_key_inclusive);
    request->mutable_command()->mutable_body()->mutable_range()->set_endkey(*end_key);
    request->mutable_command()->mutable_body()->mutable_range()->set_endkeyinclusive(
            end_key_inclusive);
    request->mutable_command()->mutable_body()->mutable_range()->set_reverse(reverse_results);
    request->mutable_command()->mutable_body()->mutable_range()->set_maxreturned(max_results);

    return service_->Submit(move(request), empty_str_, move(handler));
}

HandlerKey NonblockingKineticConnection::GetKeyRange(const string start_key,
    bool start_key_inclusive,
    const string end_key,
    bool end_key_inclusive,
    bool reverse_results,
    int32_t max_results,
    const shared_ptr<GetKeyRangeCallbackInterface> callback) {
    return this->GetKeyRange(make_shared<string>(start_key), start_key_inclusive,
        make_shared<string>(end_key), end_key_inclusive, reverse_results, max_results, callback);
}

HandlerKey NonblockingKineticConnection::Put(const shared_ptr<const string> key,
    const shared_ptr<const string> current_version, WriteMode mode,
    const shared_ptr<const KineticRecord> record,
    const shared_ptr<PutCallbackInterface> callback,
    PersistMode persistMode) {
    unique_ptr<PutHandler> handler(new PutHandler(callback));
    unique_ptr<Message> request = NewMessage(Message_MessageType_PUT);

    bool force = mode == WriteMode::IGNORE_VERSION;
    request->mutable_command()->mutable_body()->mutable_keyvalue()->set_key(*key);
    request->mutable_command()->mutable_body()->mutable_keyvalue()->set_dbversion(
            *current_version);
    request->mutable_command()->mutable_body()->mutable_keyvalue()->set_force(force);

    if (record->version().get() != nullptr) {
        request->mutable_command()->mutable_body()->mutable_keyvalue()->set_newversion(
            *(record->version()));
    }

    request->mutable_command()->mutable_body()->mutable_keyvalue()->set_tag(*(record->tag()));
    request->mutable_command()->mutable_body()->mutable_keyvalue()->set_algorithm(
            record->algorithm());

    request->mutable_command()->mutable_body()->mutable_keyvalue()->set_synchronization(
            this->GetSynchronizationForPersistMode(persistMode));

    return service_->Submit(move(request), record->value(), move(handler));
}

HandlerKey NonblockingKineticConnection::Put(const string key,
    const string current_version, WriteMode mode,
    const shared_ptr<const KineticRecord> record,
    const shared_ptr<PutCallbackInterface> callback,
    PersistMode persistMode) {
    return this->Put(make_shared<string>(key), make_shared<string>(current_version), mode, record,
        callback, persistMode);
}


HandlerKey NonblockingKineticConnection::Put(const shared_ptr<const string> key,
        const shared_ptr<const string> current_version, WriteMode mode,
        const shared_ptr<const KineticRecord> record,
        const shared_ptr<PutCallbackInterface> callback) {
    // Default to the WRITE_BACK case, which performs better but does
    // not guarantee immediate persistence
    return this->Put(key, current_version, mode, record, callback, PersistMode::WRITE_BACK);
}

HandlerKey NonblockingKineticConnection::Put(const string key,
    const string current_version, WriteMode mode,
    const shared_ptr<const KineticRecord> record,
    const shared_ptr<PutCallbackInterface> callback) {
    return this->Put(make_shared<string>(key), make_shared<string>(current_version), mode, record,
        callback);
}

HandlerKey NonblockingKineticConnection::Delete(const shared_ptr<const string> key,
    const shared_ptr<const string> version, WriteMode mode,
    const shared_ptr<SimpleCallbackInterface> callback,
    PersistMode persistMode) {
    unique_ptr<SimpleHandler> handler(new SimpleHandler(callback));
    unique_ptr<Message> request = NewMessage(Message_MessageType_DELETE);

    bool force = mode == WriteMode::IGNORE_VERSION;
    request->mutable_command()->mutable_body()->mutable_keyvalue()->set_key(*key);
    // TODO(marshall) handle null version
    request->mutable_command()->mutable_body()->mutable_keyvalue()->set_dbversion(*version);
    request->mutable_command()->mutable_body()->mutable_keyvalue()->set_force(force);
    request->mutable_command()->mutable_body()->mutable_keyvalue()->set_synchronization(
            this->GetSynchronizationForPersistMode(persistMode));

    return service_->Submit(move(request), empty_str_, move(handler));
}

HandlerKey NonblockingKineticConnection::Delete(const string key, const string version,
        WriteMode mode, const shared_ptr<SimpleCallbackInterface> callback,
        PersistMode persistMode) {
    return this->Delete(make_shared<string>(key), make_shared<string>(version),
            mode, callback, persistMode);
}

HandlerKey NonblockingKineticConnection::Delete(const shared_ptr<const string> key,
        const shared_ptr<const string> version, WriteMode mode,
        const shared_ptr<SimpleCallbackInterface> callback) {
    // Default to the WRITE_BACK case, which performs better but does
    // not guarantee immediate persistence
    return this->Delete(key, version, mode, callback, PersistMode::WRITE_BACK);
}

HandlerKey NonblockingKineticConnection::Delete(const string key, const string version,
        WriteMode mode, const shared_ptr<SimpleCallbackInterface> callback) {
    return this->Delete(make_shared<string>(key), make_shared<string>(version), mode, callback);
}

HandlerKey NonblockingKineticConnection::InstantSecureErase(const shared_ptr<string> pin,
        const shared_ptr<SimpleCallbackInterface> callback) {
    unique_ptr<SimpleHandler> handler(new SimpleHandler(callback));
    unique_ptr<Message> request = NewMessage(Message_MessageType_SETUP);
    request->mutable_command()->mutable_body()->mutable_setup()->set_instantsecureerase(true);
    if (pin != NULL) {
        request->mutable_command()->mutable_body()->mutable_setup()->set_pin(*pin);
    }
    return service_->Submit(move(request), empty_str_, move(handler));
}

HandlerKey NonblockingKineticConnection::InstantSecureErase(const string pin,
    const shared_ptr<SimpleCallbackInterface> callback) {
    return this->InstantSecureErase(make_shared<string>(pin), callback);
}

HandlerKey NonblockingKineticConnection::GenericGet(const shared_ptr<const string> key,
    const shared_ptr<GetCallbackInterface> callback, Message_MessageType message_type) {
    unique_ptr<GetHandler> handler(new GetHandler(callback));
    unique_ptr<Message> request = NewMessage(message_type);
    request->mutable_command()->mutable_body()->mutable_keyvalue()->set_key(*key);
    return service_->Submit(move(request), empty_str_, move(handler));
}

HandlerKey NonblockingKineticConnection::SetClusterVersion(int64_t new_cluster_version,
    const shared_ptr<SimpleCallbackInterface> callback) {
    unique_ptr<SimpleHandler> handler(new SimpleHandler(callback));
    unique_ptr<Message> request = NewMessage(Message_MessageType_SETUP);
    request->mutable_command()->mutable_body()->mutable_setup()->set_newclusterversion(
            new_cluster_version);
    return service_->Submit(move(request), empty_str_, move(handler));
}

HandlerKey NonblockingKineticConnection::GetLog(
        const shared_ptr<GetLogCallbackInterface> callback) {
    unique_ptr<Message> request = NewMessage(Message_MessageType_GETLOG);

    auto mutable_getlog = request->mutable_command()->mutable_body()->mutable_getlog();
    mutable_getlog->add_type(Message_GetLog_Type_UTILIZATIONS);
    mutable_getlog->add_type(Message_GetLog_Type_TEMPERATURES);
    mutable_getlog->add_type(Message_GetLog_Type_CAPACITIES);
    mutable_getlog->add_type(Message_GetLog_Type_CONFIGURATION);
    mutable_getlog->add_type(Message_GetLog_Type_STATISTICS);
    mutable_getlog->add_type(Message_GetLog_Type_MESSAGES);
    mutable_getlog->add_type(Message_GetLog_Type_LIMITS);

    unique_ptr<GetLogHandler> handler(new GetLogHandler(callback));
    return service_->Submit(move(request), empty_str_, move(handler));
}

HandlerKey NonblockingKineticConnection::UpdateFirmware(
        const shared_ptr<const string> new_firmware,
        const shared_ptr<SimpleCallbackInterface> callback) {
    unique_ptr<Message> request = NewMessage(Message_MessageType_SETUP);

    request->mutable_command()->mutable_body()->mutable_setup()->set_firmwaredownload(true);

    unique_ptr<SimpleHandler> handler(new SimpleHandler(callback));
    return service_->Submit(move(request), new_firmware, move(handler));
}

HandlerKey NonblockingKineticConnection::SetACLs(const shared_ptr<const list<ACL>> acls,
        const shared_ptr<SimpleCallbackInterface> callback) {
    unique_ptr<Message> request = NewMessage(Message_MessageType_SECURITY);

    for (auto it = acls->begin(); it != acls->end(); ++it) {
        Message_Security_ACL *acl =
                request->mutable_command()->mutable_body()->mutable_security()->add_acl();
        acl->set_identity(it->identity);
        acl->set_key(it->hmac_key);
        acl->set_hmacalgorithm(Message_Security_ACL_HMACAlgorithm_HmacSHA1);

        for (auto scope_it = it->scopes.begin(); scope_it != it->scopes.end(); ++scope_it) {
            Message_Security_ACL_Scope * scope = acl->add_scope();
            scope->set_offset(scope_it->offset);
            scope->set_value(scope_it->value);

            for (auto permission_it = scope_it->permissions.begin();
                 permission_it != scope_it->permissions.end();
                 ++permission_it) {
                Message_Security_ACL_Permission permission;
                switch (*permission_it) {
                    case READ:
                        permission = com::seagate::kinetic::client::proto::Message_Security_ACL_Permission_READ;
                        break;
                    case WRITE:
                        permission = com::seagate::kinetic::client::proto::Message_Security_ACL_Permission_WRITE;
                        break;
                    case DELETE:
                        permission = com::seagate::kinetic::client::proto::Message_Security_ACL_Permission_DELETE;
                        break;
                    case RANGE:
                        permission = com::seagate::kinetic::client::proto::Message_Security_ACL_Permission_RANGE;
                        break;
                    case SETUP:
                        permission = com::seagate::kinetic::client::proto::Message_Security_ACL_Permission_SETUP;
                        break;
                    case P2POP:
                        permission = com::seagate::kinetic::client::proto::Message_Security_ACL_Permission_P2POP;
                        break;
                    case GETLOG:
                        permission = com::seagate::kinetic::client::proto::Message_Security_ACL_Permission_GETLOG;
                        break;
                    case SECURITY:
                        permission = com::seagate::kinetic::client::proto::Message_Security_ACL_Permission_SECURITY;
                        break;
                }
                scope->add_permission(permission);
            }
        }
    }

    unique_ptr<SimpleHandler> handler(new SimpleHandler(callback));
    return service_->Submit(move(request), empty_str_, move(handler));
}

HandlerKey NonblockingKineticConnection::SetPIN(const shared_ptr<const string> new_pin,
    const shared_ptr<const string> current_pin,
        const shared_ptr<SimpleCallbackInterface> callback) {
    unique_ptr<Message> request = NewMessage(Message_MessageType_SETUP);
    request->mutable_command()->mutable_body()->mutable_setup()->set_setpin(*new_pin);

    if (current_pin) {
        request->mutable_command()->mutable_body()->mutable_setup()->set_pin(*current_pin);
    }

    unique_ptr<SimpleHandler> handler(new SimpleHandler(callback));
    return service_->Submit(move(request), empty_str_, move(handler));
}

HandlerKey NonblockingKineticConnection::SetPIN(const string new_pin, const string current_pin,
    const shared_ptr<SimpleCallbackInterface> callback) {
    return this->SetPIN(make_shared<string>(new_pin), make_shared<string>(current_pin), callback);
}

HandlerKey NonblockingKineticConnection::P2PPush(const P2PPushRequest& push_request,
        const shared_ptr<P2PPushCallbackInterface> callback) {
    return this->P2PPush(make_shared<P2PPushRequest>(push_request), callback);
}

void NonblockingKineticConnection::PopulateP2PMessage(
        Message_P2POperation *mutable_p2pop, const shared_ptr<const P2PPushRequest> push_request) {
    mutable_p2pop->mutable_peer()->set_hostname(push_request->host);
    mutable_p2pop->mutable_peer()->set_port(push_request->port);

    for (auto it = push_request->operations.begin(); it != push_request->operations.end(); ++it) {
        auto op = mutable_p2pop->add_operation();
        op->set_key(it->key);
        op->set_version(it->version);

        if (!it->newKey.empty() && it->newKey != it->key) {
            op->set_newkey(it->newKey);
        }
        op->set_force(it->force);
    }

    for (auto it = push_request->requests.begin(); it != push_request->requests.end(); ++it) {
        auto req = mutable_p2pop->add_operation()->mutable_p2pop();

        PopulateP2PMessage(req, make_shared<P2PPushRequest>(*it));
    }
}

HandlerKey NonblockingKineticConnection::P2PPush(
        const shared_ptr<const P2PPushRequest> push_request,
        const shared_ptr<P2PPushCallbackInterface> callback) {
    unique_ptr<Message> request = NewMessage(Message_MessageType_PEER2PEERPUSH);

    auto mutable_p2pop = request->mutable_command()->mutable_body()->mutable_p2poperation();

    PopulateP2PMessage(mutable_p2pop, push_request);

    unique_ptr<P2PPushHandler> handler(new P2PPushHandler(callback));
    return service_->Submit(move(request), empty_str_, move(handler));
}

bool NonblockingKineticConnection::RemoveHandler(HandlerKey handler_key) {
    return service_->Remove(handler_key);
}

Message_Synchronization NonblockingKineticConnection::GetSynchronizationForPersistMode(PersistMode persistMode) {
    Message_Synchronization sync_option;
    switch (persistMode) {
        case PersistMode::WRITE_BACK:
            sync_option = Message_Synchronization_WRITEBACK;
            break;
        case PersistMode::WRITE_THROUGH:
            sync_option = Message_Synchronization_WRITETHROUGH;
            break;
        case PersistMode::FLUSH:
            sync_option = Message_Synchronization_FLUSH;
            break;
    }
    return sync_option;
}

} // namespace kinetic
