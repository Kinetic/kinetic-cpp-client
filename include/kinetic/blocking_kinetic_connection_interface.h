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

#ifndef KINETIC_CPP_CLIENT_BLOCKING_KINETIC_CONNECTION_INTERFACE_H_
#define KINETIC_CPP_CLIENT_BLOCKING_KINETIC_CONNECTION_INTERFACE_H_

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
    virtual ~BlockingKineticConnectionInterface() {}

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

    /* Batch Operation Support. BatchStart sets the batch id to be used for all further
     * requests added to the same batch operation. Every started batch should be either
     * committed or aborted.
     * Use the BatchOperation class defined in kinetic/batch_operation.h to minimize code
     * complexity. */
    virtual KineticStatus BatchStart(int * batch_id) = 0;
    virtual KineticStatus BatchPutKey(int batch_id, const shared_ptr<const string> key,
        const shared_ptr<const string> current_version, WriteMode mode,
        const shared_ptr<const KineticRecord> record) = 0;
    virtual KineticStatus BatchPutKey(int batch_id, const string key,
        const string current_version, WriteMode mode,
        const shared_ptr<const KineticRecord> record) = 0;
    virtual KineticStatus BatchDeleteKey(int batch_id, const shared_ptr<const string> key,
        const shared_ptr<const string> version, WriteMode mode) = 0;
    virtual KineticStatus BatchDeleteKey(int batch_id, const string key,
         const string version, WriteMode mode) = 0;
    virtual KineticStatus BatchCommit(int batch_id) = 0;
    virtual KineticStatus BatchAbort(int batch_id) = 0;
};

} // namespace kinetic


#endif  // KINETIC_CPP_CLIENT_BLOCKING_KINETIC_CONNECTION_INTERFACE_H_
