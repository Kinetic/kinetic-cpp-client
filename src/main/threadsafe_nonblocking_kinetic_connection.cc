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

#include "kinetic/threadsafe_nonblocking_connection.h"
#include <mutex>

namespace kinetic {

using std::shared_ptr;
using std::string;

ThreadsafeNonblockingKineticConnection::ThreadsafeNonblockingKineticConnection(
     unique_ptr<NonblockingKineticConnection> connection) {
    connection_ = std::move(connection);
}

ThreadsafeNonblockingKineticConnection::~ThreadsafeNonblockingKineticConnection() {
}

bool ThreadsafeNonblockingKineticConnection::Run(fd_set *read_fds, fd_set *write_fds, int *nfds) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->Run(read_fds, write_fds, nfds);
}

bool ThreadsafeNonblockingKineticConnection::RemoveHandler(HandlerKey handler_key) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->RemoveHandler(handler_key);
}

void ThreadsafeNonblockingKineticConnection::SetClientClusterVersion(int64_t cluster_version) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->SetClientClusterVersion(cluster_version);
}

HandlerKey ThreadsafeNonblockingKineticConnection::NoOp(const shared_ptr<SimpleCallbackInterface> callback) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->NoOp(callback);
}

HandlerKey ThreadsafeNonblockingKineticConnection::Get(const shared_ptr<const string> key, const shared_ptr<GetCallbackInterface> callback) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->Get(key, callback);
}

HandlerKey ThreadsafeNonblockingKineticConnection::Get(const string key, const shared_ptr<GetCallbackInterface> callback) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->Get(key, callback);
}


HandlerKey ThreadsafeNonblockingKineticConnection::GetNext(const string key, const shared_ptr<GetCallbackInterface> callback){
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->GetNext(key, callback);
}

HandlerKey ThreadsafeNonblockingKineticConnection::GetNext(const shared_ptr<const string> key,
    const shared_ptr<GetCallbackInterface> callback) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->GetNext(key, callback);
}

HandlerKey ThreadsafeNonblockingKineticConnection::GetPrevious(const shared_ptr<const string> key,
    const shared_ptr<GetCallbackInterface> callback) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->GetPrevious(key, callback);
}

HandlerKey ThreadsafeNonblockingKineticConnection::GetPrevious(const string key, const shared_ptr<GetCallbackInterface> callback){
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->GetPrevious(key, callback);
}

HandlerKey ThreadsafeNonblockingKineticConnection::GetVersion(const string key, const shared_ptr<GetVersionCallbackInterface> callback){
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->GetVersion(key, callback);
}

HandlerKey ThreadsafeNonblockingKineticConnection::GetVersion(const shared_ptr<const string> key,
    const shared_ptr<GetVersionCallbackInterface> callback) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->GetVersion(key, callback);
}

HandlerKey ThreadsafeNonblockingKineticConnection::GetKeyRange(
        const shared_ptr<const string> start_key,
    bool start_key_inclusive,
    const shared_ptr<const string> end_key,
    bool end_key_inclusive,
    bool reverse_results,
    int32_t max_results,
    const shared_ptr<GetKeyRangeCallbackInterface> callback) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->GetKeyRange(start_key, start_key_inclusive, end_key,
        end_key_inclusive, reverse_results, max_results, callback);
}

HandlerKey ThreadsafeNonblockingKineticConnection::GetKeyRange(const string start_key, bool start_key_inclusive,
      const string end_key, bool end_key_inclusive,
      bool reverse_results, int32_t max_results, const shared_ptr<GetKeyRangeCallbackInterface> callback){
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->GetKeyRange(start_key, start_key_inclusive, end_key,
        end_key_inclusive, reverse_results, max_results, callback);
}

HandlerKey ThreadsafeNonblockingKineticConnection::Put(const shared_ptr<const string> key, const shared_ptr<const string> current_version, WriteMode mode,
      const shared_ptr<const KineticRecord> record, const shared_ptr<PutCallbackInterface> callback){
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->Put(key, current_version, mode, record, callback);
}
HandlerKey ThreadsafeNonblockingKineticConnection::Put(const string key, const string current_version, WriteMode mode,
      const shared_ptr<const KineticRecord> record, const shared_ptr<PutCallbackInterface> callback){
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->Put(key, current_version, mode, record, callback);
}
HandlerKey ThreadsafeNonblockingKineticConnection::Put(const shared_ptr<const string> key, const shared_ptr<const string> current_version, WriteMode mode,
      const shared_ptr<const KineticRecord> record, const shared_ptr<PutCallbackInterface> callback,
      PersistMode persistMode){
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->Put(key, current_version, mode, record, callback, persistMode);
}
HandlerKey ThreadsafeNonblockingKineticConnection::Put(const string key, const string current_version, WriteMode mode,
      const shared_ptr<const KineticRecord> record, const shared_ptr<PutCallbackInterface> callback,
      PersistMode persistMode){
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->Put(key, current_version, mode, record, callback, persistMode);
}

HandlerKey ThreadsafeNonblockingKineticConnection::Delete(const shared_ptr<const string> key, const shared_ptr<const string> version, WriteMode mode,
          const shared_ptr<SimpleCallbackInterface> callback, PersistMode persistMode){
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->Delete(key, version, mode, callback, persistMode);
}
HandlerKey ThreadsafeNonblockingKineticConnection::Delete(const string key, const string version, WriteMode mode,
          const shared_ptr<SimpleCallbackInterface> callback, PersistMode persistMode){
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->Delete(key, version, mode, callback, persistMode);
}
HandlerKey ThreadsafeNonblockingKineticConnection::Delete(const shared_ptr<const string> key, const shared_ptr<const string> version, WriteMode mode,
          const shared_ptr<SimpleCallbackInterface> callback){
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->Delete(key, version, mode, callback);
}
HandlerKey ThreadsafeNonblockingKineticConnection::Delete(const string key, const string version, WriteMode mode,
      const shared_ptr<SimpleCallbackInterface> callback){
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->Delete(key, version, mode, callback);
}

HandlerKey ThreadsafeNonblockingKineticConnection::InstantErase(const string pin, const shared_ptr<SimpleCallbackInterface> callback){
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->InstantErase(pin, callback);
}
HandlerKey ThreadsafeNonblockingKineticConnection::InstantErase(const shared_ptr<string> pin,
    const shared_ptr<SimpleCallbackInterface> callback) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->InstantErase(pin, callback);
}

HandlerKey ThreadsafeNonblockingKineticConnection::SecureErase(const string pin, const shared_ptr<SimpleCallbackInterface> callback){
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->SecureErase(pin, callback);
}
HandlerKey ThreadsafeNonblockingKineticConnection::SecureErase(const shared_ptr<string> pin,
    const shared_ptr<SimpleCallbackInterface> callback) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->SecureErase(pin, callback);
}

HandlerKey ThreadsafeNonblockingKineticConnection::SetClusterVersion(int64_t new_cluster_version,
    const shared_ptr<SimpleCallbackInterface> callback) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->SetClusterVersion(new_cluster_version, callback);
}

HandlerKey ThreadsafeNonblockingKineticConnection::GetLog(
    const shared_ptr<GetLogCallbackInterface> callback) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->GetLog(callback);
}
HandlerKey ThreadsafeNonblockingKineticConnection::GetLog(const vector<Command_GetLog_Type>& types,
        const shared_ptr<GetLogCallbackInterface> callback) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->GetLog(types, callback);
}

HandlerKey ThreadsafeNonblockingKineticConnection::UpdateFirmware(
    const shared_ptr<const string> new_firmware,
    const shared_ptr<SimpleCallbackInterface> callback) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->UpdateFirmware(new_firmware, callback);
}

HandlerKey ThreadsafeNonblockingKineticConnection::SetACLs(const shared_ptr<const list<ACL>> acls,
    const shared_ptr<SimpleCallbackInterface> callback) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->SetACLs(acls, callback);
}

HandlerKey ThreadsafeNonblockingKineticConnection::SetErasePIN(const shared_ptr<const string> new_pin,
    const shared_ptr<const string> current_pin,
    const shared_ptr<SimpleCallbackInterface> callback) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->SetErasePIN(new_pin, current_pin, callback);
}

HandlerKey ThreadsafeNonblockingKineticConnection::SetErasePIN(const string new_pin, const string current_pin, const shared_ptr<SimpleCallbackInterface> callback) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->SetErasePIN(new_pin, current_pin, callback);
}

HandlerKey ThreadsafeNonblockingKineticConnection::LockDevice(const string pin, const shared_ptr<SimpleCallbackInterface> callback){
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->LockDevice(pin, callback);
}
HandlerKey ThreadsafeNonblockingKineticConnection::LockDevice(const shared_ptr<string> pin,
    const shared_ptr<SimpleCallbackInterface> callback) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->LockDevice(pin, callback);
}

HandlerKey ThreadsafeNonblockingKineticConnection::UnlockDevice(const string pin, const shared_ptr<SimpleCallbackInterface> callback){
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->UnlockDevice(pin, callback);
}
HandlerKey ThreadsafeNonblockingKineticConnection::UnlockDevice(const shared_ptr<string> pin,
    const shared_ptr<SimpleCallbackInterface> callback) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->UnlockDevice(pin, callback);
}

HandlerKey ThreadsafeNonblockingKineticConnection::SetLockPIN(const shared_ptr<const string> new_pin,
    const shared_ptr<const string> current_pin,
    const shared_ptr<SimpleCallbackInterface> callback) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->SetLockPIN(new_pin, current_pin, callback);
}

HandlerKey ThreadsafeNonblockingKineticConnection::SetLockPIN(const string new_pin, const string current_pin, const shared_ptr<SimpleCallbackInterface> callback) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->SetLockPIN(new_pin, current_pin, callback);
}

HandlerKey ThreadsafeNonblockingKineticConnection::P2PPush(
    const shared_ptr<const P2PPushRequest> push_request,
    const shared_ptr<P2PPushCallbackInterface> callback) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->P2PPush(push_request, callback);
}

HandlerKey ThreadsafeNonblockingKineticConnection::P2PPush(const P2PPushRequest& push_request,
        const shared_ptr<P2PPushCallbackInterface> callback) {
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    return connection_->P2PPush(push_request, callback);
}



} // namespace kinetic
