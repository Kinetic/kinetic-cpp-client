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

#include <memory>
#include <stdexcept>
#include "kinetic/threadsafe_blocking_kinetic_connection.h"

namespace kinetic {

using std::shared_ptr;
using std::unique_ptr;
using std::string;
using std::make_shared;
using std::move;

ThreadsafeBlockingKineticConnection::ThreadsafeBlockingKineticConnection(
    unique_ptr<BlockingKineticConnection> connection) : connection_(std::move(connection)) {
}

ThreadsafeBlockingKineticConnection::~ThreadsafeBlockingKineticConnection() {}

KineticStatus ThreadsafeBlockingKineticConnection::NoOp() {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->NoOp();
}

void ThreadsafeBlockingKineticConnection::SetClientClusterVersion(int64_t cluster_version) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->SetClientClusterVersion(cluster_version);
}

KineticStatus ThreadsafeBlockingKineticConnection::Get(const shared_ptr<const string> key,
                                                       unique_ptr<KineticRecord> &record) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->Get(key, record);
}

KineticStatus ThreadsafeBlockingKineticConnection::Get(const string &key,
                                                       unique_ptr<KineticRecord> &record) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->Get(key, record);
}

KineticStatus ThreadsafeBlockingKineticConnection::Put(const shared_ptr<const string> key,
                                                       const shared_ptr<const string> current_version,
                                                       WriteMode mode,
                                                       const shared_ptr<const KineticRecord> record,
                                                       PersistMode persistMode) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->Put(key, current_version, mode, record, persistMode);
}

KineticStatus ThreadsafeBlockingKineticConnection::Put(const string &key,
                                                       const string &current_version,
                                                       WriteMode mode,
                                                       const KineticRecord &record,
                                                       PersistMode persistMode) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->Put(key, current_version, mode, record, persistMode);
}

KineticStatus ThreadsafeBlockingKineticConnection::Put(const shared_ptr<const string> key,
                                                       const shared_ptr<const string> current_version,
                                                       WriteMode mode,
                                                       const shared_ptr<const KineticRecord> record) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->Put(key, current_version, mode, record);
}

KineticStatus ThreadsafeBlockingKineticConnection::Put(const string &key,
                                                       const string &current_version,
                                                       WriteMode mode,
                                                       const KineticRecord &record) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->Put(key, current_version, mode, record);
}

KineticStatus ThreadsafeBlockingKineticConnection::Delete(const shared_ptr<const string> key,
                                                          const shared_ptr<const string> version,
                                                          WriteMode mode,
                                                          PersistMode persistMode) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->Delete(key, version, mode, persistMode);
}

KineticStatus ThreadsafeBlockingKineticConnection::Delete(const string &key,
                                                          const string &version,
                                                          WriteMode mode,
                                                          PersistMode persistMode) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->Delete(key, version, mode, persistMode);
}

KineticStatus ThreadsafeBlockingKineticConnection::Delete(const shared_ptr<const string> key,
                                                          const shared_ptr<const string> version,
                                                          WriteMode mode) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->Delete(key, version, mode);
}

KineticStatus ThreadsafeBlockingKineticConnection::Delete(const string &key,
                                                          const string &version,
                                                          WriteMode mode) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->Delete(key, version, mode);
}

KineticStatus ThreadsafeBlockingKineticConnection::InstantErase(const shared_ptr<string> pin) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->InstantErase(pin);
}

KineticStatus ThreadsafeBlockingKineticConnection::InstantErase(const string &pin) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->InstantErase(pin);
}

KineticStatus ThreadsafeBlockingKineticConnection::SecureErase(const shared_ptr<string> pin) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->InstantErase(pin);
}

KineticStatus ThreadsafeBlockingKineticConnection::SecureErase(const string &pin) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->InstantErase(pin);
}

KineticStatus ThreadsafeBlockingKineticConnection::SetClusterVersion(int64_t new_cluster_version) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->SetClusterVersion(new_cluster_version);
}

KineticStatus ThreadsafeBlockingKineticConnection::GetLog(unique_ptr<DriveLog> &drive_log) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->GetLog(drive_log);
}

KineticStatus ThreadsafeBlockingKineticConnection::GetLog(const vector<Command_GetLog_Type> &types,
                                                          unique_ptr<DriveLog> &drive_log) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->GetLog(types, drive_log);
}

KineticStatus ThreadsafeBlockingKineticConnection::Flush() {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->Flush();
}

KineticStatus ThreadsafeBlockingKineticConnection::UpdateFirmware(const shared_ptr<const string> new_firmware) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->UpdateFirmware(new_firmware);
}

KineticStatus ThreadsafeBlockingKineticConnection::SetACLs(const shared_ptr<const list<ACL>> acls) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->SetACLs(acls);
}

KineticStatus ThreadsafeBlockingKineticConnection::SetErasePIN(const shared_ptr<const string> new_pin,
                                                               const shared_ptr<const string> current_pin) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->SetErasePIN(new_pin, current_pin);
}

KineticStatus ThreadsafeBlockingKineticConnection::SetErasePIN(const string &new_pin,
                                                               const string &current_pin) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->SetErasePIN(new_pin, current_pin);
}

KineticStatus ThreadsafeBlockingKineticConnection::SetLockPIN(const shared_ptr<const string> new_pin,
                                                              const shared_ptr<const string> current_pin) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->SetLockPIN(new_pin, current_pin);
}

KineticStatus ThreadsafeBlockingKineticConnection::SetLockPIN(const string &new_pin,
                                                              const string &current_pin) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->SetLockPIN(new_pin, current_pin);
}

KineticStatus ThreadsafeBlockingKineticConnection::LockDevice(const shared_ptr<string> pin) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->LockDevice(pin);
}

KineticStatus ThreadsafeBlockingKineticConnection::LockDevice(const string &pin) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->LockDevice(pin);
}

KineticStatus ThreadsafeBlockingKineticConnection::UnlockDevice(const shared_ptr<string> pin) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->UnlockDevice(pin);
}

KineticStatus ThreadsafeBlockingKineticConnection::UnlockDevice(const string &pin) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->UnlockDevice(pin);
}

KineticStatus ThreadsafeBlockingKineticConnection::GetNext(const shared_ptr<const string> key,
                                                           unique_ptr<string> &actual_key,
                                                           unique_ptr<KineticRecord> &record) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->GetNext(key, actual_key, record);
}

KineticStatus ThreadsafeBlockingKineticConnection::GetNext(const string &key,
                                                           unique_ptr<string> &actual_key,
                                                           unique_ptr<KineticRecord> &record) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->GetNext(key, actual_key, record);
}

KineticStatus ThreadsafeBlockingKineticConnection::GetPrevious(const shared_ptr<const string> key,
                                                               unique_ptr<string> &actual_key,
                                                               unique_ptr<KineticRecord> &record) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->GetPrevious(key, actual_key, record);
}

KineticStatus ThreadsafeBlockingKineticConnection::GetPrevious(const string &key,
                                                               unique_ptr<string> &actual_key,
                                                               unique_ptr<KineticRecord> &record) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->GetPrevious(key, actual_key, record);
}

KineticStatus ThreadsafeBlockingKineticConnection::GetVersion(const shared_ptr<const string> key,
                                                              unique_ptr<string> &version) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->GetVersion(key, version);
}

KineticStatus ThreadsafeBlockingKineticConnection::GetVersion(const string &key,
                                                              unique_ptr<string> &version) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->GetVersion(key, version);
}

KineticStatus ThreadsafeBlockingKineticConnection::GetKeyRange(const shared_ptr<const string> start_key,
                                                               bool start_key_inclusive,
                                                               const shared_ptr<const string> end_key,
                                                               bool end_key_inclusive,
                                                               bool reverse_results,
                                                               int32_t max_results,
                                                               unique_ptr<vector<string>> &keys) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->GetKeyRange(start_key,
                                    start_key_inclusive,
                                    end_key,
                                    end_key_inclusive,
                                    reverse_results,
                                    max_results,
                                    keys);
}

KineticStatus ThreadsafeBlockingKineticConnection::GetKeyRange(const string &start_key,
                                                               bool start_key_inclusive,
                                                               const string &end_key,
                                                               bool end_key_inclusive,
                                                               bool reverse_results,
                                                               int32_t max_results,
                                                               unique_ptr<vector<string>> &keys) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->GetKeyRange(start_key,
                                    start_key_inclusive,
                                    end_key,
                                    end_key_inclusive,
                                    reverse_results,
                                    max_results,
                                    keys);
}

KeyRangeIterator ThreadsafeBlockingKineticConnection::IterateKeyRange(const shared_ptr<const string> start_key,
                                                                      bool start_key_inclusive,
                                                                      const shared_ptr<const string> end_key,
                                                                      bool end_key_inclusive,
                                                                      unsigned int frame_size) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->IterateKeyRange(start_key, start_key_inclusive, end_key, end_key_inclusive, frame_size);
}

KeyRangeIterator ThreadsafeBlockingKineticConnection::IterateKeyRange(const string &start_key,
                                                                      bool start_key_inclusive,
                                                                      const string &end_key,
                                                                      bool end_key_inclusive,
                                                                      unsigned int frame_size) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->IterateKeyRange(start_key, start_key_inclusive, end_key, end_key_inclusive, frame_size);
}

KineticStatus ThreadsafeBlockingKineticConnection::P2PPush(const shared_ptr<const P2PPushRequest> push_request,
                                                           unique_ptr<vector<KineticStatus>> &operation_statuses) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->P2PPush(push_request, operation_statuses);
}

KineticStatus ThreadsafeBlockingKineticConnection::P2PPush(const P2PPushRequest &push_request,
                                                           unique_ptr<vector<KineticStatus>> &operation_statuses) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->P2PPush(push_request, operation_statuses);
}

KineticStatus ThreadsafeBlockingKineticConnection::MediaScan(const shared_ptr<const string> start_key,
                                                             bool start_key_inclusive,
                                                             const shared_ptr<const string> end_key,
                                                             bool end_key_inclusive,
                                                             int32_t max_results,
                                                             unique_ptr<string> &last_handled_key,
                                                             unique_ptr<vector<string>> &keys) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->MediaScan(start_key,
                                  start_key_inclusive,
                                  end_key,
                                  end_key_inclusive,
                                  max_results,
                                  last_handled_key,
                                  keys);
}

KineticStatus ThreadsafeBlockingKineticConnection::MediaScan(const string &start_key,
                                                             bool start_key_inclusive,
                                                             const string &end_key,
                                                             bool end_key_inclusive,
                                                             int32_t max_results,
                                                             unique_ptr<string> &last_handled_key,
                                                             unique_ptr<vector<string>> &keys) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->MediaScan(start_key,
                                  start_key_inclusive,
                                  end_key,
                                  end_key_inclusive,
                                  max_results,
                                  last_handled_key,
                                  keys);
}

KineticStatus ThreadsafeBlockingKineticConnection::MediaOptimize(const shared_ptr<const string> start_key,
                                                                 bool start_key_inclusive,
                                                                 const shared_ptr<const string> end_key,
                                                                 bool end_key_inclusive,
                                                                 unique_ptr<string> &last_handled_key) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->MediaOptimize(start_key, start_key_inclusive, end_key, end_key_inclusive, last_handled_key);
}

KineticStatus ThreadsafeBlockingKineticConnection::MediaOptimize(const string &start_key,
                                                                 bool start_key_inclusive,
                                                                 const string &end_key,
                                                                 bool end_key_inclusive,
                                                                 unique_ptr<string> &last_handled_key) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->MediaOptimize(start_key, start_key_inclusive, end_key, end_key_inclusive, last_handled_key);
}

}// namespace kinetic
