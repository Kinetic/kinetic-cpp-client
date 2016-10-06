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

#include "kinetic/nonblocking_kinetic_connection.h"
#include "nonblocking_packet_service.h"
#include <memory>
#include <glog/logging.h>

namespace kinetic {

using namespace com::seagate::kinetic::client::proto;
using std::shared_ptr;
using std::string;
using std::make_shared;
using std::unique_ptr;
using std::iterator;
using std::move;

GetHandler::GetHandler(const shared_ptr<GetCallbackInterface> callback) : callback_(callback) {}

void GetHandler::Handle(const Command &response,
                        unique_ptr<const string> value) {
    unique_ptr<KineticRecord> record(new KineticRecord(shared_ptr<const string>(value.release()),
                                                       make_shared<string>(response.body().keyvalue().dbversion()),
                                                       make_shared<string>(response.body().keyvalue().tag()),
                                                       response.body().keyvalue().algorithm()));
    callback_->Success(response.body().keyvalue().key(), move(record));
}

void GetHandler::Error(KineticStatus error,
                       Command const *const response) {
    callback_->Failure(error);
}

GetVersionHandler::GetVersionHandler(const shared_ptr<GetVersionCallbackInterface> callback) : callback_(callback) {}

void GetVersionHandler::Handle(const Command &response,
                               unique_ptr<const string> value) {
    callback_->Success(response.body().keyvalue().dbversion());
}

void GetVersionHandler::Error(KineticStatus error,
                              Command const *const response) {
    callback_->Failure(error);
}

GetKeyRangeHandler::GetKeyRangeHandler(const shared_ptr<GetKeyRangeCallbackInterface> callback) : callback_(callback) {}

void GetKeyRangeHandler::Handle(const Command &response,
                                unique_ptr<const string> value) {
    int raw_size = response.body().range().keys_size();
    CHECK_GE(raw_size, 0);
    size_t key_size = (size_t) raw_size;

    unique_ptr<vector<string>> keys(new vector<string>);
    keys->reserve(key_size);

    for (size_t i = 0; i < key_size; i++) {
        keys->push_back(response.body().range().keys(i));
    }

    callback_->Success(move(keys));
}

void GetKeyRangeHandler::Error(KineticStatus error,
                               Command const *const response) {
    callback_->Failure(error);
}

MediaScanHandler::MediaScanHandler(const shared_ptr<MediaScanCallbackInterface> callback) : callback_(callback) {}

void MediaScanHandler::Handle(const Command &response,
                              unique_ptr<const string> value) {
    int raw_size = response.body().range().keys_size();
    CHECK_GE(raw_size, 0);
    size_t key_size = (size_t) raw_size;

    unique_ptr<vector<string>> keys(new vector<string>);
    keys->reserve(key_size);

    for (size_t i = 0; i < key_size; i++) {
        keys->push_back(response.body().range().keys(i));
    }
    callback_->Success(move(keys), response.body().range().endkey());
}

void MediaScanHandler::Error(KineticStatus error,
                             Command const *const response) {
    callback_->Failure(error);
}

MediaOptimizeHandler::MediaOptimizeHandler(const shared_ptr<MediaOptimizeCallbackInterface> callback) : callback_(
    callback) {}

void MediaOptimizeHandler::Handle(const Command &response,
                                  unique_ptr<const string> value) {
    callback_->Success(response.body().range().endkey());
}

void MediaOptimizeHandler::Error(KineticStatus error,
                                 Command const *const response) {
    callback_->Failure(error);
}

PutHandler::PutHandler(const shared_ptr<PutCallbackInterface> callback) : callback_(callback) {}

void PutHandler::Handle(const Command &response,
                        unique_ptr<const string> value) {
    callback_->Success();
}

void PutHandler::Error(KineticStatus error,
                       Command const *const response) {
    callback_->Failure(error);
}

SimpleHandler::SimpleHandler(const shared_ptr<SimpleCallbackInterface> callback) : callback_(callback) {}

void SimpleHandler::Handle(const Command &response,
                           unique_ptr<const string> value) {
    callback_->Success();
}

void SimpleHandler::Error(KineticStatus error,
                          Command const *const response) {
    callback_->Failure(error);
}

GetLogHandler::GetLogHandler(const shared_ptr<GetLogCallbackInterface> callback) : callback_(callback) {}

void GetLogHandler::Handle(const Command &response,
                           unique_ptr<const string> value) {
    auto getlog = response.body().getlog();
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
        statistic.name = Command_MessageType_Name(getlog.statistics(i).messagetype());
        statistic.count = getlog.statistics(i).count();
        statistic.bytes = getlog.statistics(i).bytes();
        drive_log->operation_statistics.push_back(statistic);
    }

    for (int i = 0; i < getlog.utilizations_size(); i++) {
        Utilization utilization;
        utilization.name = getlog.utilizations(i).name();
        utilization.percent = getlog.utilizations(i).value();
        drive_log->utilizations.push_back(utilization);
    }

    for (int i = 0; i < getlog.temperatures_size(); i++) {
        Temperature temperature;

        temperature.name = getlog.temperatures(i).name();
        temperature.current_degc = getlog.temperatures(i).current();
        temperature.target_degc = getlog.temperatures(i).target();
        temperature.max_degc = getlog.temperatures(i).maximum();
        temperature.min_degc = getlog.temperatures(i).minimum();

        drive_log->temperatures.push_back(temperature);
    }

    drive_log->messages = getlog.messages();

    callback_->Success(move(drive_log));
}

void GetLogHandler::Error(KineticStatus error,
                          Command const *const response) {
    callback_->Failure(error);
}

P2PPushHandler::P2PPushHandler(const shared_ptr<P2PPushCallbackInterface> callback) : callback_(callback) {}

void P2PPushHandler::Handle(const Command &response,
                            unique_ptr<const string> value) {
    unique_ptr<vector<KineticStatus>> statuses(new vector<KineticStatus>());

    Command_P2POperation const &p2pop = response.body().p2poperation();
    statuses->reserve(p2pop.operation_size());

    for (int i = 0; i < p2pop.operation_size(); i++) {
        Command_Status const &status = p2pop.operation(i).status();

        statuses->push_back(KineticStatus(ConvertFromProtoStatus(status.code()), status.statusmessage()));
    }

    callback_->Success(move(statuses), response);
}

void P2PPushHandler::Error(KineticStatus error,
                           Command const *const response) {
    callback_->Failure(error, response);
}

NonblockingKineticConnection::NonblockingKineticConnection(NonblockingPacketServiceInterface *service) : service_(
    service), empty_str_(make_shared<string>("")), cluster_version_(0) {}

NonblockingKineticConnection::~NonblockingKineticConnection() {
    delete service_;
}

bool NonblockingKineticConnection::Run(fd_set *read_fds,
                                       fd_set *write_fds,
                                       int *nfds) {
    return service_->Run(read_fds, write_fds, nfds);
}

void NonblockingKineticConnection::SetClientClusterVersion(int64_t cluster_version) {
    cluster_version_ = cluster_version;
}

unique_ptr<Command> NonblockingKineticConnection::NewCommand(Command_MessageType message_type) {
    unique_ptr<Command> cmd(new Command());
    cmd->mutable_header()->set_messagetype(message_type);
    cmd->mutable_header()->set_clusterversion(cluster_version_);
    return move(cmd);
}

HandlerKey NonblockingKineticConnection::NoOp(const shared_ptr<SimpleCallbackInterface> callback) {
    unique_ptr<SimpleHandler> handler(new SimpleHandler(callback));

    unique_ptr<Message> msg(new Message());
    msg->set_authtype(Message_AuthType_HMACAUTH);

    unique_ptr<Command> request = NewCommand(Command_MessageType_NOOP);
    return service_->Submit(move(msg), move(request), empty_str_, move(handler));
}

HandlerKey NonblockingKineticConnection::Get(const shared_ptr<const string> key,
                                             const shared_ptr<GetCallbackInterface> callback) {
    return GenericGet(key, callback, Command_MessageType_GET);
}

HandlerKey NonblockingKineticConnection::Get(const string key,
                                             const shared_ptr<GetCallbackInterface> callback) {
    return this->Get(make_shared<string>(key), callback);
}

HandlerKey NonblockingKineticConnection::GetNext(const shared_ptr<const string> key,
                                                 const shared_ptr<GetCallbackInterface> callback) {
    return GenericGet(key, callback, Command_MessageType_GETNEXT);
}

HandlerKey NonblockingKineticConnection::GetNext(const string key,
                                                 const shared_ptr<GetCallbackInterface> callback) {
    return this->GetNext(make_shared<string>(key), callback);
}

HandlerKey NonblockingKineticConnection::GetPrevious(const shared_ptr<const string> key,
                                                     const shared_ptr<GetCallbackInterface> callback) {
    return GenericGet(key, callback, Command_MessageType_GETPREVIOUS);
}

HandlerKey NonblockingKineticConnection::GetPrevious(const string key,
                                                     const shared_ptr<GetCallbackInterface> callback) {
    return this->GetPrevious(make_shared<string>(key), callback);
}

HandlerKey NonblockingKineticConnection::GetVersion(const shared_ptr<const string> key,
                                                    const shared_ptr<GetVersionCallbackInterface> callback) {
    unique_ptr<GetVersionHandler> handler(new GetVersionHandler(callback));
    unique_ptr<Message> msg(new Message());
    msg->set_authtype(Message_AuthType_HMACAUTH);

    unique_ptr<Command> request = NewCommand(Command_MessageType_GETVERSION);
    request->mutable_body()->mutable_keyvalue()->set_key(*key);

    return service_->Submit(move(msg), move(request), empty_str_, move(handler));
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

    unique_ptr<Message> msg(new Message());
    msg->set_authtype(Message_AuthType_HMACAUTH);

    unique_ptr<Command> request = NewCommand(Command_MessageType_GETKEYRANGE);
    request->mutable_body()->mutable_range()->set_startkey(*start_key);
    request->mutable_body()->mutable_range()->set_startkeyinclusive(start_key_inclusive);
    request->mutable_body()->mutable_range()->set_endkey(*end_key);
    request->mutable_body()->mutable_range()->set_endkeyinclusive(end_key_inclusive);
    request->mutable_body()->mutable_range()->set_reverse(reverse_results);
    request->mutable_body()->mutable_range()->set_maxreturned(max_results);

    return service_->Submit(move(msg), move(request), empty_str_, move(handler));
}

HandlerKey NonblockingKineticConnection::GetKeyRange(const string start_key,
                                                     bool start_key_inclusive,
                                                     const string end_key,
                                                     bool end_key_inclusive,
                                                     bool reverse_results,
                                                     int32_t max_results,
                                                     const shared_ptr<GetKeyRangeCallbackInterface> callback) {
    return this->GetKeyRange(make_shared<string>(start_key),
                             start_key_inclusive,
                             make_shared<string>(end_key),
                             end_key_inclusive,
                             reverse_results,
                             max_results,
                             callback);
}

HandlerKey NonblockingKineticConnection::Put(const shared_ptr<const string> key,
                                             const shared_ptr<const string> current_version,
                                             WriteMode mode,
                                             const shared_ptr<const KineticRecord> record,
                                             const shared_ptr<PutCallbackInterface> callback,
                                             PersistMode persistMode) {
    unique_ptr<PutHandler> handler(new PutHandler(callback));

    unique_ptr<Message> msg(new Message());
    msg->set_authtype(Message_AuthType_HMACAUTH);

    unique_ptr<Command> request = NewCommand(Command_MessageType_PUT);

    bool force = mode == WriteMode::IGNORE_VERSION;
    request->mutable_body()->mutable_keyvalue()->set_key(*key);
    request->mutable_body()->mutable_keyvalue()->set_dbversion(*current_version);
    request->mutable_body()->mutable_keyvalue()->set_force(force);

    if (record->version().get() != NULL) {
        request->mutable_body()->mutable_keyvalue()->set_newversion(*(record->version()));
    }

    request->mutable_body()->mutable_keyvalue()->set_tag(*(record->tag()));
    request->mutable_body()->mutable_keyvalue()->set_algorithm(record->algorithm());

    request->mutable_body()->mutable_keyvalue()->set_synchronization(GetSynchronizationForPersistMode(persistMode));

    return service_->Submit(move(msg), move(request), record->value(), move(handler));
}

HandlerKey NonblockingKineticConnection::Put(const string key,
                                             const string current_version,
                                             WriteMode mode,
                                             const shared_ptr<const KineticRecord> record,
                                             const shared_ptr<PutCallbackInterface> callback,
                                             PersistMode persistMode) {
    return this->Put(make_shared<string>(key),
                     make_shared<string>(current_version),
                     mode,
                     record,
                     callback,
                     persistMode);
}

HandlerKey NonblockingKineticConnection::Put(const shared_ptr<const string> key,
                                             const shared_ptr<const string> current_version,
                                             WriteMode mode,
                                             const shared_ptr<const KineticRecord> record,
                                             const shared_ptr<PutCallbackInterface> callback) {
    // Default to the WRITE_BACK case, which performs better but does
    // not guarantee immediate persistence
    return this->Put(key, current_version, mode, record, callback, PersistMode::WRITE_BACK);
}

HandlerKey NonblockingKineticConnection::Put(const string key,
                                             const string current_version,
                                             WriteMode mode,
                                             const shared_ptr<const KineticRecord> record,
                                             const shared_ptr<PutCallbackInterface> callback) {
    return this->Put(make_shared<string>(key), make_shared<string>(current_version), mode, record, callback);
}

HandlerKey NonblockingKineticConnection::Delete(const shared_ptr<const string> key,
                                                const shared_ptr<const string> version,
                                                WriteMode mode,
                                                const shared_ptr<SimpleCallbackInterface> callback,
                                                PersistMode persistMode) {
    unique_ptr<SimpleHandler> handler(new SimpleHandler(callback));

    unique_ptr<Message> msg(new Message());
    msg->set_authtype(Message_AuthType_HMACAUTH);

    unique_ptr<Command> request = NewCommand(Command_MessageType_DELETE);

    bool force = mode == WriteMode::IGNORE_VERSION;
    request->mutable_body()->mutable_keyvalue()->set_key(*key);
    // TODO(marshall) handle null version
    request->mutable_body()->mutable_keyvalue()->set_dbversion(*version);
    request->mutable_body()->mutable_keyvalue()->set_force(force);
    request->mutable_body()->mutable_keyvalue()->set_synchronization(GetSynchronizationForPersistMode(persistMode));

    return service_->Submit(move(msg), move(request), empty_str_, move(handler));
}

HandlerKey NonblockingKineticConnection::Delete(const string key,
                                                const string version,
                                                WriteMode mode,
                                                const shared_ptr<SimpleCallbackInterface> callback,
                                                PersistMode persistMode) {
    return this->Delete(make_shared<string>(key), make_shared<string>(version), mode, callback, persistMode);
}

HandlerKey NonblockingKineticConnection::Delete(const shared_ptr<const string> key,
                                                const shared_ptr<const string> version,
                                                WriteMode mode,
                                                const shared_ptr<SimpleCallbackInterface> callback) {
    // Default to the WRITE_BACK case, which performs better but does
    // not guarantee immediate persistence
    return this->Delete(key, version, mode, callback, PersistMode::WRITE_BACK);
}

HandlerKey NonblockingKineticConnection::Delete(const string key,
                                                const string version,
                                                WriteMode mode,
                                                const shared_ptr<SimpleCallbackInterface> callback) {
    return this->Delete(make_shared<string>(key), make_shared<string>(version), mode, callback);
}

HandlerKey NonblockingKineticConnection::SecureErase(const shared_ptr<string> pin,
                                                     const shared_ptr<SimpleCallbackInterface> callback) {
    unique_ptr<SimpleHandler> handler(new SimpleHandler(callback));

    unique_ptr<Message> msg(new Message());
    msg->set_authtype(Message_AuthType_PINAUTH);
    if (pin) msg->mutable_pinauth()->set_pin(*pin);

    unique_ptr<Command> request = NewCommand(Command_MessageType_PINOP);
    request->mutable_body()->mutable_pinop()->set_pinoptype(Command_PinOperation_PinOpType_SECURE_ERASE_PINOP);

    return service_->Submit(move(msg), move(request), empty_str_, move(handler));
}

HandlerKey NonblockingKineticConnection::SecureErase(const string pin,
                                                     const shared_ptr<SimpleCallbackInterface> callback) {
    return this->SecureErase(make_shared<string>(pin), callback);
}

HandlerKey NonblockingKineticConnection::InstantErase(const shared_ptr<string> pin,
                                                      const shared_ptr<SimpleCallbackInterface> callback) {
    unique_ptr<SimpleHandler> handler(new SimpleHandler(callback));

    unique_ptr<Message> msg(new Message());
    msg->set_authtype(Message_AuthType_PINAUTH);
    if (pin) msg->mutable_pinauth()->set_pin(*pin);

    unique_ptr<Command> request = NewCommand(Command_MessageType_PINOP);
    request->mutable_body()->mutable_pinop()->set_pinoptype(Command_PinOperation_PinOpType_ERASE_PINOP);

    return service_->Submit(move(msg), move(request), empty_str_, move(handler));
}

HandlerKey NonblockingKineticConnection::InstantErase(const string pin,
                                                      const shared_ptr<SimpleCallbackInterface> callback) {
    return this->InstantErase(make_shared<string>(pin), callback);
}

HandlerKey NonblockingKineticConnection::LockDevice(const shared_ptr<string> pin,
                                                    const shared_ptr<SimpleCallbackInterface> callback) {
    unique_ptr<SimpleHandler> handler(new SimpleHandler(callback));

    unique_ptr<Message> msg(new Message());
    msg->set_authtype(Message_AuthType_PINAUTH);
    if (pin) msg->mutable_pinauth()->set_pin(*pin);

    unique_ptr<Command> request = NewCommand(Command_MessageType_PINOP);
    request->mutable_body()->mutable_pinop()->set_pinoptype(Command_PinOperation_PinOpType_LOCK_PINOP);
    return service_->Submit(move(msg), move(request), empty_str_, move(handler));
}

HandlerKey NonblockingKineticConnection::LockDevice(const string pin,
                                                    const shared_ptr<SimpleCallbackInterface> callback) {
    return this->LockDevice(make_shared<string>(pin), callback);
}

HandlerKey NonblockingKineticConnection::UnlockDevice(const shared_ptr<string> pin,
                                                      const shared_ptr<SimpleCallbackInterface> callback) {
    unique_ptr<SimpleHandler> handler(new SimpleHandler(callback));

    unique_ptr<Message> msg(new Message());
    msg->set_authtype(Message_AuthType_PINAUTH);
    if (pin) msg->mutable_pinauth()->set_pin(*pin);

    unique_ptr<Command> request = NewCommand(Command_MessageType_PINOP);
    request->mutable_body()->mutable_pinop()->set_pinoptype(Command_PinOperation_PinOpType_UNLOCK_PINOP);
    return service_->Submit(move(msg), move(request), empty_str_, move(handler));
}

HandlerKey NonblockingKineticConnection::UnlockDevice(const string pin,
                                                      const shared_ptr<SimpleCallbackInterface> callback) {
    return this->UnlockDevice(make_shared<string>(pin), callback);
}

HandlerKey NonblockingKineticConnection::GenericGet(const shared_ptr<const string> key,
                                                    const shared_ptr<GetCallbackInterface> callback,
                                                    Command_MessageType message_type) {
    unique_ptr<GetHandler> handler(new GetHandler(callback));

    unique_ptr<Message> msg(new Message());
    msg->set_authtype(Message_AuthType_HMACAUTH);
    unique_ptr<Command> request = NewCommand(message_type);

    request->mutable_body()->mutable_keyvalue()->set_key(*key);
    return service_->Submit(move(msg), move(request), empty_str_, move(handler));
}

HandlerKey NonblockingKineticConnection::Flush(const shared_ptr<SimpleCallbackInterface> callback) {
    unique_ptr<Message> msg(new Message());
    msg->set_authtype(Message_AuthType_HMACAUTH);
    unique_ptr<Command> request = NewCommand(Command_MessageType_FLUSHALLDATA);

    unique_ptr<SimpleHandler> handler(new SimpleHandler(callback));
    return service_->Submit(move(msg), move(request), empty_str_, move(handler));
}

HandlerKey NonblockingKineticConnection::SetClusterVersion(int64_t new_cluster_version,
                                                           const shared_ptr<SimpleCallbackInterface> callback) {
    unique_ptr<SimpleHandler> handler(new SimpleHandler(callback));

    unique_ptr<Message> msg(new Message());
    msg->set_authtype(Message_AuthType_HMACAUTH);
    unique_ptr<Command> request = NewCommand(Command_MessageType_SETUP);

    request->mutable_body()->mutable_setup()->set_newclusterversion(new_cluster_version);
    return service_->Submit(move(msg), move(request), empty_str_, move(handler));
}

HandlerKey NonblockingKineticConnection::GetLog(const shared_ptr<GetLogCallbackInterface> callback) {
    vector<Command_GetLog_Type> types;
    types.push_back(Command_GetLog_Type_UTILIZATIONS);
    types.push_back(Command_GetLog_Type_TEMPERATURES);
    types.push_back(Command_GetLog_Type_CAPACITIES);
    types.push_back(Command_GetLog_Type_CONFIGURATION);
    types.push_back(Command_GetLog_Type_STATISTICS);
    types.push_back(Command_GetLog_Type_MESSAGES);
    types.push_back(Command_GetLog_Type_LIMITS);

    return GetLog(types, callback);
}

HandlerKey NonblockingKineticConnection::GetLog(const vector<Command_GetLog_Type> &types,
                                                const shared_ptr<GetLogCallbackInterface> callback) {
    unique_ptr<Message> msg(new Message());
    msg->set_authtype(Message_AuthType_HMACAUTH);
    unique_ptr<Command> request = NewCommand(Command_MessageType_GETLOG);

    for (auto iter = types.begin(); iter != types.end(); ++iter) {
        auto mutable_getlog = request->mutable_body()->mutable_getlog();
        mutable_getlog->add_types(*iter);
    }

    unique_ptr<GetLogHandler> handler(new GetLogHandler(callback));
    return service_->Submit(move(msg), move(request), empty_str_, move(handler));
}

HandlerKey NonblockingKineticConnection::UpdateFirmware(const shared_ptr<const string> new_firmware,
                                                        const shared_ptr<SimpleCallbackInterface> callback) {
    unique_ptr<Message> msg(new Message());
    msg->set_authtype(Message_AuthType_HMACAUTH);
    unique_ptr<Command> request = NewCommand(Command_MessageType_SETUP);

    request->mutable_body()->mutable_setup()->set_firmwaredownload(true);

    unique_ptr<SimpleHandler> handler(new SimpleHandler(callback));
    return service_->Submit(move(msg), move(request), new_firmware, move(handler));
}

HandlerKey NonblockingKineticConnection::SetACLs(const shared_ptr<const list<ACL>> acls,
                                                 const shared_ptr<SimpleCallbackInterface> callback) {
    unique_ptr<Message> msg(new Message());
    msg->set_authtype(Message_AuthType_HMACAUTH);
    unique_ptr<Command> request = NewCommand(Command_MessageType_SECURITY);

    for (auto it = acls->begin(); it != acls->end(); ++it) {
        Command_Security_ACL *acl = request->mutable_body()->mutable_security()->add_acl();
        acl->set_identity(it->identity);
        acl->set_key(it->hmac_key);
        acl->set_hmacalgorithm(Command_Security_ACL_HMACAlgorithm_HmacSHA1);

        for (auto scope_it = it->scopes.begin(); scope_it != it->scopes.end(); ++scope_it) {
            Command_Security_ACL_Scope *scope = acl->add_scope();
            scope->set_offset(scope_it->offset);
            scope->set_value(scope_it->value);

            for (auto permission_it = scope_it->permissions.begin(); permission_it != scope_it->permissions.end();
                 ++permission_it) {
                Command_Security_ACL_Permission permission;
                switch (*permission_it) {
                    case READ:permission = com::seagate::kinetic::client::proto::Command_Security_ACL_Permission_READ;
                        break;
                    case WRITE:permission = com::seagate::kinetic::client::proto::Command_Security_ACL_Permission_WRITE;
                        break;
                    case DELETE:
                        permission = com::seagate::kinetic::client::proto::Command_Security_ACL_Permission_DELETE;
                        break;
                    case RANGE:permission = com::seagate::kinetic::client::proto::Command_Security_ACL_Permission_RANGE;
                        break;
                    case SETUP:permission = com::seagate::kinetic::client::proto::Command_Security_ACL_Permission_SETUP;
                        break;
                    case P2POP:permission = com::seagate::kinetic::client::proto::Command_Security_ACL_Permission_P2POP;
                        break;
                    case GETLOG:
                        permission = com::seagate::kinetic::client::proto::Command_Security_ACL_Permission_GETLOG;
                        break;
                    case SECURITY:
                        permission = com::seagate::kinetic::client::proto::Command_Security_ACL_Permission_SECURITY;
                        break;
                }
                scope->add_permission(permission);
            }
        }
    }

    unique_ptr<SimpleHandler> handler(new SimpleHandler(callback));
    return service_->Submit(move(msg), move(request), empty_str_, move(handler));
}

HandlerKey NonblockingKineticConnection::SetLockPIN(const shared_ptr<const string> new_pin,
                                                    const shared_ptr<const string> current_pin,
                                                    const shared_ptr<SimpleCallbackInterface> callback) {
    unique_ptr<Message> msg(new Message());
    msg->set_authtype(Message_AuthType_HMACAUTH);

    unique_ptr<Command> request = NewCommand(Command_MessageType_SECURITY);
    if (current_pin)
        request->mutable_body()->mutable_security()->set_oldlockpin(*current_pin);
    if (new_pin)
        request->mutable_body()->mutable_security()->set_newlockpin(*new_pin);

    unique_ptr<SimpleHandler> handler(new SimpleHandler(callback));
    return service_->Submit(move(msg), move(request), empty_str_, move(handler));
}

HandlerKey NonblockingKineticConnection::SetLockPIN(const string new_pin,
                                                    const string current_pin,
                                                    const shared_ptr<SimpleCallbackInterface> callback) {
    return this->SetLockPIN(make_shared<string>(new_pin), make_shared<string>(current_pin), callback);
}

HandlerKey NonblockingKineticConnection::SetErasePIN(const shared_ptr<const string> new_pin,
                                                     const shared_ptr<const string> current_pin,
                                                     const shared_ptr<SimpleCallbackInterface> callback) {
    unique_ptr<Message> msg(new Message());
    msg->set_authtype(Message_AuthType_HMACAUTH);

    unique_ptr<Command> request = NewCommand(Command_MessageType_SECURITY);
    if (current_pin)
        request->mutable_body()->mutable_security()->set_olderasepin(*current_pin);
    if (new_pin)
        request->mutable_body()->mutable_security()->set_newerasepin(*new_pin);

    unique_ptr<SimpleHandler> handler(new SimpleHandler(callback));
    return service_->Submit(move(msg), move(request), empty_str_, move(handler));
}

HandlerKey NonblockingKineticConnection::SetErasePIN(const string new_pin,
                                                     const string current_pin,
                                                     const shared_ptr<SimpleCallbackInterface> callback) {
    return this->SetErasePIN(make_shared<string>(new_pin), make_shared<string>(current_pin), callback);
}

HandlerKey NonblockingKineticConnection::P2PPush(const P2PPushRequest &push_request,
                                                 const shared_ptr<P2PPushCallbackInterface> callback) {
    return this->P2PPush(make_shared<P2PPushRequest>(push_request), callback);
}

void NonblockingKineticConnection::PopulateP2PMessage(Command_P2POperation *mutable_p2pop,
                                                      const shared_ptr<const P2PPushRequest> push_request) {
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

        if (it->request != NULL) {
            auto req = op->mutable_p2pop();
            PopulateP2PMessage(req, it->request);
        }
    }
}

HandlerKey NonblockingKineticConnection::P2PPush(const shared_ptr<const P2PPushRequest> push_request,
                                                 const shared_ptr<P2PPushCallbackInterface> callback) {
    unique_ptr<Message> msg(new Message());
    msg->set_authtype(Message_AuthType_HMACAUTH);
    unique_ptr<Command> request = NewCommand(Command_MessageType_PEER2PEERPUSH);

    auto mutable_p2pop = request->mutable_body()->mutable_p2poperation();
    PopulateP2PMessage(mutable_p2pop, push_request);

    unique_ptr<P2PPushHandler> handler(new P2PPushHandler(callback));
    return service_->Submit(move(msg), move(request), empty_str_, move(handler));
}

HandlerKey NonblockingKineticConnection::MediaScan(const shared_ptr<const string> start_key,
                                                   bool start_key_inclusive,
                                                   const shared_ptr<const string> end_key,
                                                   bool end_key_inclusive,
                                                   int32_t max_results,
                                                   const shared_ptr<MediaScanCallbackInterface> callback) {
    unique_ptr<MediaScanHandler> handler(new MediaScanHandler(callback));

    unique_ptr<Message> msg(new Message());
    msg->set_authtype(Message_AuthType_HMACAUTH);

    unique_ptr<Command> request = NewCommand(Command_MessageType_MEDIASCAN);
    request->mutable_header()->set_priority(Command_Priority_LOWER);

    request->mutable_body()->mutable_range()->set_startkey(*start_key);
    request->mutable_body()->mutable_range()->set_startkeyinclusive(start_key_inclusive);
    request->mutable_body()->mutable_range()->set_endkey(*end_key);
    request->mutable_body()->mutable_range()->set_endkeyinclusive(end_key_inclusive);
    request->mutable_body()->mutable_range()->set_maxreturned(max_results);

    return service_->Submit(move(msg), move(request), empty_str_, move(handler));
}

HandlerKey NonblockingKineticConnection::MediaScan(const string start_key,
                                                   bool start_key_inclusive,
                                                   const string end_key,
                                                   bool end_key_inclusive,
                                                   int32_t max_results,
                                                   const shared_ptr<MediaScanCallbackInterface> callback) {
    return this->MediaScan(make_shared<string>(start_key),
                           start_key_inclusive,
                           make_shared<string>(end_key),
                           end_key_inclusive,
                           max_results,
                           callback);
}

HandlerKey NonblockingKineticConnection::MediaOptimize(const shared_ptr<const string> start_key,
                                                       bool start_key_inclusive,
                                                       const shared_ptr<const string> end_key,
                                                       bool end_key_inclusive,
                                                       const shared_ptr<MediaOptimizeCallbackInterface> callback) {
    unique_ptr<MediaOptimizeHandler> handler(new MediaOptimizeHandler(callback));

    unique_ptr<Message> msg(new Message());
    msg->set_authtype(Message_AuthType_HMACAUTH);

    unique_ptr<Command> request = NewCommand(Command_MessageType_MEDIAOPTIMIZE);
    request->mutable_header()->set_priority(Command_Priority_LOWER);

    request->mutable_body()->mutable_range()->set_startkey(*start_key);
    request->mutable_body()->mutable_range()->set_startkeyinclusive(start_key_inclusive);
    request->mutable_body()->mutable_range()->set_endkey(*end_key);
    request->mutable_body()->mutable_range()->set_endkeyinclusive(end_key_inclusive);

    return service_->Submit(move(msg), move(request), empty_str_, move(handler));
}

HandlerKey NonblockingKineticConnection::MediaOptimize(const string start_key,
                                                       bool start_key_inclusive,
                                                       const string end_key,
                                                       bool end_key_inclusive,
                                                       const shared_ptr<MediaOptimizeCallbackInterface> callback) {
    return this->MediaOptimize(make_shared<string>(start_key),
                               start_key_inclusive,
                               make_shared<string>(end_key),
                               end_key_inclusive,
                               callback);
}

bool NonblockingKineticConnection::RemoveHandler(HandlerKey handler_key) {
    return service_->Remove(handler_key);
}

Command_Synchronization NonblockingKineticConnection::GetSynchronizationForPersistMode(PersistMode persistMode) {
    Command_Synchronization sync_option;
    switch (persistMode) {
        case PersistMode::WRITE_BACK:sync_option = Command_Synchronization_WRITEBACK;
            break;
        case PersistMode::WRITE_THROUGH:sync_option = Command_Synchronization_WRITETHROUGH;
            break;
        case PersistMode::FLUSH:sync_option = Command_Synchronization_FLUSH;
            break;
    }
    return sync_option;
}

} // namespace kinetic
