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

#ifndef BLOCKING_KINETIC_CONNECTION_INTERFACE_H_
#define BLOCKING_KINETIC_CONNECTION_INTERFACE_H_

#include "kinetic/status.h"
#include "kinetic/kinetic_connection.h"
#include "kinetic/key_range_iterator.h"
#include "kinetic/common.h"
#include "kinetic/nonblocking_kinetic_connection.h"
#include <memory>
#include <string>
#include <vector>

namespace kinetic {

using std::shared_ptr;
using std::unique_ptr;
using std::string;
using std::vector;

class KeyRangeIterator;
class BlockingCallbackState;


class BlockingKineticConnectionInterface {
    public:
    virtual ~BlockingKineticConnectionInterface(){};

    virtual void SetClientClusterVersion(int64_t cluster_version) = 0;

    virtual KineticStatus NoOp() = 0;
    virtual KineticStatus Get(
            const shared_ptr<const string> key,
            unique_ptr<KineticRecord>& record) = 0;
    virtual KineticStatus Get(const string& key, unique_ptr<KineticRecord>& record) = 0;
    virtual KineticStatus GetNext(
            const shared_ptr<const string> key,
            unique_ptr<string>& actual_key,
            unique_ptr<KineticRecord>& record) = 0;
    virtual KineticStatus GetNext(
            const string& key,
            unique_ptr<string>& actual_key,
            unique_ptr<KineticRecord>& record) = 0;
    virtual KineticStatus GetPrevious(const shared_ptr<const string> key,
            unique_ptr<string>& actual_key,
            unique_ptr<KineticRecord>& record) = 0;
    virtual KineticStatus GetPrevious(const string& key,
            unique_ptr<string>& actual_key,
            unique_ptr<KineticRecord>& record) = 0;
    virtual KineticStatus GetVersion(const shared_ptr<const string> key,
            unique_ptr<string>& version) = 0;
    virtual KineticStatus GetVersion(const string& key, unique_ptr<string>& version) = 0;
    virtual KineticStatus GetKeyRange(const shared_ptr<const string> start_key,
            bool start_key_inclusive,
            const shared_ptr<const string> end_key,
            bool end_key_inclusive,
            bool reverse_results,
            int32_t max_results,
            unique_ptr<vector<string>>& keys) = 0;
    virtual KineticStatus GetKeyRange(const string& start_key,
            bool start_key_inclusive,
            const string& end_key,
            bool end_key_inclusive,
            bool reverse_results,
            int32_t max_results,
            unique_ptr<vector<string>>& keys) = 0;
    virtual KeyRangeIterator IterateKeyRange(const shared_ptr<const string> start_key,
            bool start_key_inclusive,
            const shared_ptr<const string> end_key,
            bool end_key_inclusive,
            unsigned int frame_size) = 0;
    virtual KeyRangeIterator IterateKeyRange(const string& start_key,
            bool start_key_inclusive,
            const string& end_key,
            bool end_key_inclusive,
            unsigned int frame_size) = 0;
    virtual KineticStatus Put(const shared_ptr<const string> key,
            const shared_ptr<const string> current_version, WriteMode mode,
            const shared_ptr<const KineticRecord> record,
            PersistMode persistMode) = 0;
    virtual KineticStatus Put(const string& key,
            const string& current_version, WriteMode mode,
            const KineticRecord& record,
            PersistMode persistMode) = 0;
    virtual KineticStatus Put(const shared_ptr<const string> key,
            const shared_ptr<const string> current_version, WriteMode mode,
            const shared_ptr<const KineticRecord> record) = 0;
    virtual KineticStatus Put(const string& key,
            const string& current_version, WriteMode mode,
            const KineticRecord& record) = 0;
    virtual KineticStatus Delete(const shared_ptr<const string> key,
            const shared_ptr<const string> version, WriteMode mode, PersistMode persistMode) = 0;
    virtual KineticStatus Delete(const string& key, const string& version,
            WriteMode mode, PersistMode persistMode) = 0;
    virtual KineticStatus Delete(const shared_ptr<const string> key,
            const shared_ptr<const string> version, WriteMode mode) = 0;
    virtual KineticStatus Delete(const string& key, const string& version, WriteMode mode) = 0;
    virtual KineticStatus GetLog(unique_ptr<DriveLog>& drive_log) = 0;
    virtual KineticStatus GetLog(const vector<Command_GetLog_Type>& types, unique_ptr<DriveLog>& drive_log) = 0;
    virtual KineticStatus P2PPush(const P2PPushRequest& push_request,
            unique_ptr<vector<KineticStatus>>& operation_statuses) = 0;
    virtual KineticStatus P2PPush(const shared_ptr<const P2PPushRequest> push_request,
            unique_ptr<vector<KineticStatus>>& operation_statuses) = 0;

    virtual KineticStatus SetClusterVersion(int64_t cluster_version) = 0;
    virtual KineticStatus UpdateFirmware(const shared_ptr<const string> new_firmware) = 0;
    virtual KineticStatus SetACLs(const shared_ptr<const list<ACL>> acls) = 0;

    virtual KineticStatus SetErasePIN(const shared_ptr<const string> new_pin,
            const shared_ptr<const string> current_pin = make_shared<string>()) = 0;
    virtual KineticStatus SetErasePIN(const string& new_pin, const string& current_pin) = 0;
    virtual KineticStatus SetLockPIN(const shared_ptr<const string> new_pin,
            const shared_ptr<const string> current_pin = make_shared<string>()) = 0;
    virtual KineticStatus SetLockPIN(const string& new_pin, const string& current_pin) = 0;
    virtual KineticStatus InstantErase(const shared_ptr<string> pin) = 0;
    virtual KineticStatus InstantErase(const string& pin) = 0;
    virtual KineticStatus SecureErase(const shared_ptr<string> pin) = 0;
    virtual KineticStatus SecureErase(const string& pin) = 0;
    virtual KineticStatus LockDevice(const shared_ptr<string> pin) = 0;
    virtual KineticStatus LockDevice(const string& pin) = 0;
    virtual KineticStatus UnlockDevice(const shared_ptr<string> pin) = 0;
    virtual KineticStatus UnlockDevice(const string& pin) = 0;

};

} // namespace kinetic



#endif /* BLOCKING_KINETIC_CONNECTION_INTERFACE_H_ */
