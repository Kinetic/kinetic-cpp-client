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
    NonblockingPacketServiceInterface *service) : NonblockingKineticConnection(service) {
}

ThreadsafeNonblockingKineticConnection::~ThreadsafeNonblockingKineticConnection() {
}

bool ThreadsafeNonblockingKineticConnection::Run(fd_set *read_fds, fd_set *write_fds, int *nfds) {
    std::lock_guard<std::mutex> guard(mutex_);
    return NonblockingKineticConnection::Run(read_fds, write_fds, nfds);
}

void ThreadsafeNonblockingKineticConnection::SetClientClusterVersion(int64_t cluster_version) {
    std::lock_guard<std::mutex> guard(mutex_);
    return NonblockingKineticConnection::SetClientClusterVersion(cluster_version);
}

HandlerKey ThreadsafeNonblockingKineticConnection::NoOp(const shared_ptr<SimpleCallbackInterface> callback) {
    std::lock_guard<std::mutex> guard(mutex_);
    return NonblockingKineticConnection::NoOp(callback);
}

HandlerKey ThreadsafeNonblockingKineticConnection::Get(const shared_ptr<const string> key,
    const shared_ptr<GetCallbackInterface> callback) {
    std::lock_guard<std::mutex> guard(mutex_);
    return NonblockingKineticConnection::Get(key, callback);
}

HandlerKey ThreadsafeNonblockingKineticConnection::GetNext(const shared_ptr<const string> key,
    const shared_ptr<GetCallbackInterface> callback) {
    std::lock_guard<std::mutex> guard(mutex_);
    return NonblockingKineticConnection::GetNext(key, callback);
}

HandlerKey ThreadsafeNonblockingKineticConnection::GetPrevious(const shared_ptr<const string> key,
    const shared_ptr<GetCallbackInterface> callback) {
    std::lock_guard<std::mutex> guard(mutex_);
    return NonblockingKineticConnection::GetPrevious(key, callback);
}

HandlerKey ThreadsafeNonblockingKineticConnection::GetVersion(const shared_ptr<const string> key,
    const shared_ptr<GetVersionCallbackInterface> callback) {
    std::lock_guard<std::mutex> guard(mutex_);
    return NonblockingKineticConnection::GetVersion(key, callback);
}

HandlerKey ThreadsafeNonblockingKineticConnection::GetKeyRange(
        const shared_ptr<const string> start_key,
    bool start_key_inclusive,
    const shared_ptr<const string> end_key,
    bool end_key_inclusive,
    bool reverse_results,
    int32_t max_results,
    const shared_ptr<GetKeyRangeCallbackInterface> callback) {
    std::lock_guard<std::mutex> guard(mutex_);
    return NonblockingKineticConnection::GetKeyRange(start_key, start_key_inclusive, end_key,
        end_key_inclusive, reverse_results, max_results, callback);
}

HandlerKey ThreadsafeNonblockingKineticConnection::Put(const shared_ptr<const string> key,
    const shared_ptr<const string> current_version, WriteMode mode,
    const shared_ptr<const KineticRecord> record,
    const shared_ptr<PutCallbackInterface> callback,
    PersistMode persistMode) {
    std::lock_guard<std::mutex> guard(mutex_);
    return NonblockingKineticConnection::Put(key, current_version, mode, record, callback, persistMode);
}

HandlerKey ThreadsafeNonblockingKineticConnection::Delete(const shared_ptr<const string> key,
    const shared_ptr<const string> version, WriteMode mode,
    const shared_ptr<SimpleCallbackInterface> callback,
    PersistMode persistMode) {
    std::lock_guard<std::mutex> guard(mutex_);
    return NonblockingKineticConnection::Delete(key, version, mode, callback, persistMode);
}

HandlerKey ThreadsafeNonblockingKineticConnection::InstantSecureErase(const shared_ptr<string> pin,
    const shared_ptr<SimpleCallbackInterface> callback) {
    std::lock_guard<std::mutex> guard(mutex_);
    return NonblockingKineticConnection::InstantSecureErase(pin, callback);
}

HandlerKey ThreadsafeNonblockingKineticConnection::SetClusterVersion(int64_t new_cluster_version,
    const shared_ptr<SimpleCallbackInterface> callback) {
    std::lock_guard<std::mutex> guard(mutex_);
    return NonblockingKineticConnection::SetClusterVersion(new_cluster_version, callback);
}

HandlerKey ThreadsafeNonblockingKineticConnection::GetLog(
    const shared_ptr<GetLogCallbackInterface> callback) {
    std::lock_guard<std::mutex> guard(mutex_);
    return NonblockingKineticConnection::GetLog(callback);
}

HandlerKey ThreadsafeNonblockingKineticConnection::UpdateFirmware(
    const shared_ptr<const string> new_firmware,
    const shared_ptr<SimpleCallbackInterface> callback) {
    std::lock_guard<std::mutex> guard(mutex_);
    return NonblockingKineticConnection::UpdateFirmware(new_firmware, callback);
}

HandlerKey ThreadsafeNonblockingKineticConnection::SetACLs(const shared_ptr<const list<ACL>> acls,
    const shared_ptr<SimpleCallbackInterface> callback) {
    std::lock_guard<std::mutex> guard(mutex_);
    return NonblockingKineticConnection::SetACLs(acls, callback);
}

HandlerKey ThreadsafeNonblockingKineticConnection::SetPIN(const shared_ptr<const string> new_pin,
    const shared_ptr<const string> current_pin,
    const shared_ptr<SimpleCallbackInterface> callback) {
    std::lock_guard<std::mutex> guard(mutex_);
    return NonblockingKineticConnection::SetPIN(new_pin, current_pin, callback);
}

HandlerKey ThreadsafeNonblockingKineticConnection::P2PPush(
    const shared_ptr<const P2PPushRequest> push_request,
    const shared_ptr<P2PPushCallbackInterface> callback) {
    std::lock_guard<std::mutex> guard(mutex_);
    return NonblockingKineticConnection::P2PPush(push_request, callback);
}

bool ThreadsafeNonblockingKineticConnection::RemoveHandler(HandlerKey handler_key) {
    std::lock_guard<std::mutex> guard(mutex_);
    return NonblockingKineticConnection::RemoveHandler(handler_key);
}

} // namespace kinetic
