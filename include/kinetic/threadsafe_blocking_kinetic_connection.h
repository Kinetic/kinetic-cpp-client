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

#ifndef KINETIC_CPP_CLIENT_THREADSAFE_BLOCKING_KINETIC_CONNECTION_H_
#define KINETIC_CPP_CLIENT_THREADSAFE_BLOCKING_KINETIC_CONNECTION_H_

#include <memory>
#include <mutex>

#include "kinetic/blocking_kinetic_connection.h"
#include "kinetic/threadsafe_nonblocking_connection.h"
#include "kinetic/status.h"
#include "kinetic/kinetic_connection.h"
#include "kinetic/key_range_iterator.h"
#include "protobufutil/common.h"


namespace kinetic {

using std::shared_ptr;
using std::unique_ptr;
using std::string;
using std::vector;

class KeyRangeIterator;
class BlockingCallbackState;

class ThreadsafeBlockingKineticConnection : public BlockingKineticConnection {
    public:
    /// Takes ownership of the given NonblockingKineticConnection
    /// @param[in] nonblocking_connection   The underlying connection that will be used
    /// @param[in] network_timeout_seconds  If an operation goes more than network_timeout_seconds
    ///                                     seconds without receiving data the operation will fail
    explicit ThreadsafeBlockingKineticConnection(
        shared_ptr<ThreadsafeNonblockingKineticConnection> nonblocking_connection,
        unsigned int network_timeout_seconds);
    ~ThreadsafeBlockingKineticConnection();

    /// If the drive has a non-zero cluster version, requests will fail unless the developer
    /// tells the client the correct cluster version using this method.
    void SetClientClusterVersion(int64_t cluster_version);

    KineticStatus NoOp();

    KineticStatus Get(
            const shared_ptr<const string> key,
            unique_ptr<KineticRecord>& record);

    KineticStatus GetNext(
            const shared_ptr<const string> key,
            unique_ptr<string>& actual_key,
            unique_ptr<KineticRecord>& record);

    KineticStatus GetPrevious(const shared_ptr<const string> key,
            unique_ptr<string>& actual_key,
            unique_ptr<KineticRecord>& record);

    KineticStatus GetVersion(const shared_ptr<const string> key,
            unique_ptr<string>& version);


    KineticStatus GetKeyRange(const shared_ptr<const string> start_key,
            bool start_key_inclusive,
            const shared_ptr<const string> end_key,
            bool end_key_inclusive,
            bool reverse_results,
            int32_t max_results,
            unique_ptr<vector<string>>& keys);

    KeyRangeIterator IterateKeyRange(const shared_ptr<const string> start_key,
            bool start_key_inclusive,
            const shared_ptr<const string> end_key,
            bool end_key_inclusive,
            unsigned int frame_size);

    KineticStatus Put(const shared_ptr<const string> key,
            const shared_ptr<const string> current_version, WriteMode mode,
            const shared_ptr<const KineticRecord> record,
            PersistMode persistMode);

    KineticStatus Delete(const shared_ptr<const string> key,
            const shared_ptr<const string> version, WriteMode mode, PersistMode persistMode);

    KineticStatus InstantSecureErase(const shared_ptr<string> pin);

    KineticStatus SetClusterVersion(int64_t cluster_version);

    KineticStatus GetLog(unique_ptr<DriveLog>& drive_log);

    KineticStatus UpdateFirmware(const shared_ptr<const string> new_firmware);

    KineticStatus SetACLs(const shared_ptr<const list<ACL>> acls);

    KineticStatus SetPin(const shared_ptr<const string> new_pin,
            const shared_ptr<const string> current_pin = make_shared<string>());

    KineticStatus P2PPush(const shared_ptr<const P2PPushRequest> push_request,
            unique_ptr<vector<KineticStatus>>& operation_statuses);

    private:
    std::mutex mutex_;
};

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_THREADSAFE_BLOCKING_KINETIC_CONNECTION_H_
