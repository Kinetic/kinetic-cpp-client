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


#include "kinetic/blocking_kinetic_connection.h"
#include <mutex>

namespace kinetic {

using std::shared_ptr;
using std::unique_ptr;
using std::string;

class ThreadsafeBlockingKineticConnection : public BlockingKineticConnectionInterface {
    public:
    explicit ThreadsafeBlockingKineticConnection(
            unique_ptr<BlockingKineticConnection> connection);
    ~ThreadsafeBlockingKineticConnection();

     void SetClientClusterVersion(int64_t cluster_version);

     KineticStatus NoOp();

      KineticStatus Get(
              const shared_ptr<const string> key,
              unique_ptr<KineticRecord>& record);

      KineticStatus Get(const string& key, unique_ptr<KineticRecord>& record);

      KineticStatus GetNext(
              const shared_ptr<const string> key,
              unique_ptr<string>& actual_key,
              unique_ptr<KineticRecord>& record);

      KineticStatus GetNext(
              const string& key,
              unique_ptr<string>& actual_key,
              unique_ptr<KineticRecord>& record);

      KineticStatus GetPrevious(const shared_ptr<const string> key,
              unique_ptr<string>& actual_key,
              unique_ptr<KineticRecord>& record);

      KineticStatus GetPrevious(const string& key,
              unique_ptr<string>& actual_key,
              unique_ptr<KineticRecord>& record);

      KineticStatus GetVersion(const shared_ptr<const string> key,
              unique_ptr<string>& version);

      KineticStatus GetVersion(const string& key, unique_ptr<string>& version);

      KineticStatus GetKeyRange(const shared_ptr<const string> start_key,
              bool start_key_inclusive,
              const shared_ptr<const string> end_key,
              bool end_key_inclusive,
              bool reverse_results,
              int32_t max_results,
              unique_ptr<vector<string>>& keys);

      KineticStatus GetKeyRange(const string& start_key,
              bool start_key_inclusive,
              const string& end_key,
              bool end_key_inclusive,
              bool reverse_results,
              int32_t max_results,
              unique_ptr<vector<string>>& keys);

      KeyRangeIterator IterateKeyRange(const shared_ptr<const string> start_key,
              bool start_key_inclusive,
              const shared_ptr<const string> end_key,
              bool end_key_inclusive,
              unsigned int frame_size);

      KeyRangeIterator IterateKeyRange(const string& start_key,
              bool start_key_inclusive,
              const string& end_key,
              bool end_key_inclusive,
              unsigned int frame_size);

      KineticStatus Put(const shared_ptr<const string> key,
              const shared_ptr<const string> current_version, WriteMode mode,
              const shared_ptr<const KineticRecord> record,
              PersistMode persistMode);

      KineticStatus Put(const string& key,
              const string& current_version, WriteMode mode,
              const KineticRecord& record,
              PersistMode persistMode);

      KineticStatus Put(const shared_ptr<const string> key,
              const shared_ptr<const string> current_version, WriteMode mode,
              const shared_ptr<const KineticRecord> record);

      KineticStatus Put(const string& key,
              const string& current_version, WriteMode mode,
              const KineticRecord& record);

      KineticStatus Delete(const shared_ptr<const string> key,
              const shared_ptr<const string> version, WriteMode mode, PersistMode persistMode);

      KineticStatus Delete(const string& key, const string& version,
              WriteMode mode, PersistMode persistMode);

      KineticStatus Delete(const shared_ptr<const string> key,
              const shared_ptr<const string> version, WriteMode mode);

      KineticStatus Delete(const string& key, const string& version, WriteMode mode);

      KineticStatus GetLog(unique_ptr<DriveLog>& drive_log);

      KineticStatus GetLog(const vector<Command_GetLog_Type>& types, unique_ptr<DriveLog>& drive_log);

      KineticStatus P2PPush(const P2PPushRequest& push_request,
              unique_ptr<vector<KineticStatus>>& operation_statuses);

      KineticStatus P2PPush(const shared_ptr<const P2PPushRequest> push_request,
              unique_ptr<vector<KineticStatus>>& operation_statuses);

      KineticStatus SetClusterVersion(int64_t cluster_version);
      KineticStatus UpdateFirmware(const shared_ptr<const string> new_firmware);
      KineticStatus SetACLs(const shared_ptr<const list<ACL>> acls);

      KineticStatus SetErasePIN(const shared_ptr<const string> new_pin,
              const shared_ptr<const string> current_pin = make_shared<string>());
      KineticStatus SetErasePIN(const string& new_pin, const string& current_pin);
      KineticStatus SetLockPIN(const shared_ptr<const string> new_pin,
              const shared_ptr<const string> current_pin = make_shared<string>());
      KineticStatus SetLockPIN(const string& new_pin, const string& current_pin);
      KineticStatus InstantErase(const shared_ptr<string> pin);
      KineticStatus InstantErase(const string& pin);
      KineticStatus SecureErase(const shared_ptr<string> pin);
      KineticStatus SecureErase(const string& pin);
      KineticStatus LockDevice(const shared_ptr<string> pin);
      KineticStatus LockDevice(const string& pin);
      KineticStatus UnlockDevice(const shared_ptr<string> pin);
      KineticStatus UnlockDevice(const string& pin);


    private:
    std::recursive_mutex mutex_;
    unique_ptr<BlockingKineticConnection> connection_;
};

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_THREADSAFE_BLOCKING_KINETIC_CONNECTION_H_
