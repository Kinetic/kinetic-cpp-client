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

#ifndef KINETIC_CPP_CLIENT_BLOCKING_KINETIC_CONNECTION_H_
#define KINETIC_CPP_CLIENT_BLOCKING_KINETIC_CONNECTION_H_

#include <memory>

#include "kinetic/nonblocking_kinetic_connection.h"
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

class BlockingKineticConnection {
    public:
    /// Takes ownership of the given NonblockingKineticConnection
    /// @param[in] nonblocking_connection   The underlying connection that will be used
    /// @param[in] network_timeout_seconds  If an operation goes more than network_timeout_seconds
    ///                                     seconds without receiving data the operation will fail
    explicit BlockingKineticConnection(
            shared_ptr<NonblockingKineticConnection> nonblocking_connection,
        unsigned int network_timeout_seconds);

    explicit BlockingKineticConnection(
            unique_ptr<NonblockingKineticConnection> nonblocking_connection,
            unsigned int network_timeout_seconds);

    virtual ~BlockingKineticConnection();

    /// If the drive has a non-zero cluster version, requests will fail unless the developer
    /// tells the client the correct cluster version using this method.
    virtual void SetClientClusterVersion(int64_t cluster_version);

    virtual KineticStatus NoOp();

    virtual KineticStatus Get(
            const shared_ptr<const string> key,
            unique_ptr<KineticRecord>& record);

    virtual KineticStatus Get(const string& key, unique_ptr<KineticRecord>& record);

    virtual KineticStatus GetNext(
            const shared_ptr<const string> key,
            unique_ptr<string>& actual_key,
            unique_ptr<KineticRecord>& record);

    virtual KineticStatus GetNext(
            const string& key,
            unique_ptr<string>& actual_key,
            unique_ptr<KineticRecord>& record);

    virtual KineticStatus GetPrevious(const shared_ptr<const string> key,
            unique_ptr<string>& actual_key,
            unique_ptr<KineticRecord>& record);

    virtual KineticStatus GetPrevious(const string& key,
            unique_ptr<string>& actual_key,
            unique_ptr<KineticRecord>& record);

    virtual KineticStatus GetVersion(const shared_ptr<const string> key,
            unique_ptr<string>& version);

    virtual KineticStatus GetVersion(const string& key, unique_ptr<string>& version);

    virtual KineticStatus GetKeyRange(const shared_ptr<const string> start_key,
            bool start_key_inclusive,
            const shared_ptr<const string> end_key,
            bool end_key_inclusive,
            bool reverse_results,
            int32_t max_results,
            unique_ptr<vector<string>>& keys);

    virtual KineticStatus GetKeyRange(const string& start_key,
            bool start_key_inclusive,
            const string& end_key,
            bool end_key_inclusive,
            bool reverse_results,
            int32_t max_results,
            unique_ptr<vector<string>>& keys);


    virtual KeyRangeIterator IterateKeyRange(const shared_ptr<const string> start_key,
            bool start_key_inclusive,
            const shared_ptr<const string> end_key,
            bool end_key_inclusive,
            unsigned int frame_size);

    virtual KeyRangeIterator IterateKeyRange(const string& start_key,
            bool start_key_inclusive,
            const string& end_key,
            bool end_key_inclusive,
            unsigned int frame_size);

    virtual KineticStatus Put(const shared_ptr<const string> key,
            const shared_ptr<const string> current_version, WriteMode mode,
            const shared_ptr<const KineticRecord> record,
            PersistMode persistMode);

    virtual KineticStatus Put(const string& key,
            const string& current_version, WriteMode mode,
            const KineticRecord& record,
            PersistMode persistMode);

    virtual KineticStatus Put(const shared_ptr<const string> key,
            const shared_ptr<const string> current_version, WriteMode mode,
            const shared_ptr<const KineticRecord> record);

    virtual KineticStatus Put(const string& key,
            const string& current_version, WriteMode mode,
            const KineticRecord& record);

    virtual KineticStatus Delete(const shared_ptr<const string> key,
            const shared_ptr<const string> version, WriteMode mode, PersistMode persistMode);

    virtual KineticStatus Delete(const string& key, const string& version,
            WriteMode mode, PersistMode persistMode);

    virtual KineticStatus Delete(const shared_ptr<const string> key,
            const shared_ptr<const string> version, WriteMode mode);

    virtual KineticStatus Delete(const string& key, const string& version, WriteMode mode);

    virtual KineticStatus InstantSecureErase(const shared_ptr<string> pin);

    virtual KineticStatus InstantSecureErase(const string& pin);

    virtual KineticStatus SetClusterVersion(int64_t cluster_version);

    virtual KineticStatus GetLog(unique_ptr<DriveLog>& drive_log);

    virtual KineticStatus UpdateFirmware(const shared_ptr<const string> new_firmware);

    virtual KineticStatus SetACLs(const shared_ptr<const list<ACL>> acls);

    virtual KineticStatus SetPin(const shared_ptr<const string> new_pin,
            const shared_ptr<const string> current_pin = make_shared<string>());

    virtual KineticStatus SetPin(const string& new_pin, const string& current_pin);

    virtual KineticStatus P2PPush(const P2PPushRequest& push_request,
            unique_ptr<vector<KineticStatus>>& operation_statuses);

    virtual KineticStatus P2PPush(const shared_ptr<const P2PPushRequest> push_request,
            unique_ptr<vector<KineticStatus>>& operation_statuses);

    private:
    KineticStatus RunOperation(shared_ptr<BlockingCallbackState> callback, HandlerKey handler_key);

    /// Helper method for translating a StatusCode from the drive into an API client KineticStatus
    /// object
    KineticStatus GetKineticStatus(StatusCode code);
    shared_ptr<NonblockingKineticConnection> nonblocking_connection_;
    const unsigned int network_timeout_seconds_;
    DISALLOW_COPY_AND_ASSIGN(BlockingKineticConnection);
    };

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_BLOCKING_KINETIC_CONNECTION_H_
