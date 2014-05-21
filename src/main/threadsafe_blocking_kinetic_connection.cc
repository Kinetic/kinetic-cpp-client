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
        shared_ptr<ThreadsafeNonblockingKineticConnection> nonblocking_connection, unsigned int network_timeout_seconds)
    : BlockingKineticConnection(nonblocking_connection, network_timeout_seconds) {}

ThreadsafeBlockingKineticConnection::~ThreadsafeBlockingKineticConnection() {}

KineticStatus ThreadsafeBlockingKineticConnection::NoOp() {
    std::lock_guard<std::mutex> guard(mutex_);
    return BlockingKineticConnection::NoOp();
}

void ThreadsafeBlockingKineticConnection::SetClientClusterVersion(int64_t cluster_version) {
    std::lock_guard<std::mutex> guard(mutex_);
    return BlockingKineticConnection::SetClientClusterVersion(cluster_version);
}

KineticStatus ThreadsafeBlockingKineticConnection::Get(const shared_ptr<const string> key,
    unique_ptr<KineticRecord>& record) {
    std::lock_guard<std::mutex> guard(mutex_);
    return BlockingKineticConnection::Get(key, record);
}

KineticStatus ThreadsafeBlockingKineticConnection::Put(const shared_ptr<const string> key,
        const shared_ptr<const string> current_version, WriteMode mode,
        const shared_ptr<const KineticRecord> record,
        PersistMode persistMode) {
    std::lock_guard<std::mutex> guard(mutex_);
    return BlockingKineticConnection::Put(key, current_version, mode, record, persistMode);
}

KineticStatus ThreadsafeBlockingKineticConnection::Delete(const shared_ptr<const string> key,
        const shared_ptr<const string> version, WriteMode mode, PersistMode persistMode) {
        std::lock_guard<std::mutex> guard(mutex_);
        return BlockingKineticConnection::Delete(key, version, mode, persistMode);
}

KineticStatus ThreadsafeBlockingKineticConnection::InstantSecureErase(const shared_ptr<string> pin) {
    std::lock_guard<std::mutex> guard(mutex_);
    return BlockingKineticConnection::InstantSecureErase(pin);
}

KineticStatus ThreadsafeBlockingKineticConnection::SetClusterVersion(int64_t new_cluster_version) {
    std::lock_guard<std::mutex> guard(mutex_);
    return BlockingKineticConnection::SetClusterVersion(new_cluster_version);
}

KineticStatus ThreadsafeBlockingKineticConnection::GetLog(unique_ptr<DriveLog>& drive_log) {
    std::lock_guard<std::mutex> guard(mutex_);
    return BlockingKineticConnection::GetLog(drive_log);
}

KineticStatus ThreadsafeBlockingKineticConnection::UpdateFirmware(const shared_ptr<const string>
        new_firmware) {
    std::lock_guard<std::mutex> guard(mutex_);
    return BlockingKineticConnection::UpdateFirmware(new_firmware);
}

KineticStatus ThreadsafeBlockingKineticConnection::SetACLs(const shared_ptr<const list<ACL>> acls) {
    std::lock_guard<std::mutex> guard(mutex_);
    return BlockingKineticConnection::SetACLs(acls);
}

KineticStatus ThreadsafeBlockingKineticConnection::SetPin(const shared_ptr<const string> new_pin,
    const shared_ptr<const string> current_pin) {
    std::lock_guard<std::mutex> guard(mutex_);
    return BlockingKineticConnection::SetPin(new_pin, current_pin);
}

KineticStatus ThreadsafeBlockingKineticConnection::GetNext(const shared_ptr<const string> key,
        unique_ptr<string>& actual_key, unique_ptr<KineticRecord>& record) {
    std::lock_guard<std::mutex> guard(mutex_);
    return BlockingKineticConnection::GetNext(key, actual_key, record);
}

KineticStatus ThreadsafeBlockingKineticConnection::GetPrevious(const shared_ptr<const string> key,
        unique_ptr<string>& actual_key, unique_ptr<KineticRecord>& record) {
    std::lock_guard<std::mutex> guard(mutex_);
    return BlockingKineticConnection::GetNext(key, actual_key, record);
}

KineticStatus ThreadsafeBlockingKineticConnection::GetVersion(const shared_ptr<const string> key,
        unique_ptr<string>& version) {
    std::lock_guard<std::mutex> guard(mutex_);
    return BlockingKineticConnection::GetVersion(key, version);
}

KineticStatus ThreadsafeBlockingKineticConnection::GetKeyRange(const shared_ptr<const string> start_key,
        bool start_key_inclusive,
        const shared_ptr<const string> end_key,
        bool end_key_inclusive,
        bool reverse_results,
        int32_t max_results,
        unique_ptr<vector<string>>& keys) {
    std::lock_guard<std::mutex> guard(mutex_);
    return BlockingKineticConnection::GetKeyRange(start_key,
            start_key_inclusive,
            end_key,
            end_key_inclusive,
            reverse_results,
            max_results,
            keys);
}

KeyRangeIterator ThreadsafeBlockingKineticConnection::IterateKeyRange(
        const shared_ptr<const string> start_key,
        bool start_key_inclusive,
        const shared_ptr<const string> end_key,
        bool end_key_inclusive,
        unsigned int frame_size) {
    std::lock_guard<std::mutex> guard(mutex_);
    return BlockingKineticConnection::IterateKeyRange(start_key,
            start_key_inclusive,
            end_key,
            end_key_inclusive,
            frame_size);
}

KineticStatus ThreadsafeBlockingKineticConnection::P2PPush(
        const shared_ptr<const P2PPushRequest> push_request,
        unique_ptr<vector<KineticStatus>>& operation_statuses) {
    std::lock_guard<std::mutex> guard(mutex_);
    return BlockingKineticConnection::P2PPush(push_request, operation_statuses);
}

} // namespace kinetic
