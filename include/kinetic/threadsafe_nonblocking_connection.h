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

#ifndef KINETIC_CPP_CLIENT_THREADSAFE_NONBLOCKING_CONNECTION_H_
#define KINETIC_CPP_CLIENT_THREADSAFE_NONBLOCKING_CONNECTION_H_

#include "kinetic/nonblocking_kinetic_connection_interface.h"
#include "kinetic/nonblocking_kinetic_connection.h"
#include <mutex>

namespace kinetic {

/// Kinetic connection class variant that synchronizes concurrent access and allows non-blocking
/// IO. Instead of constructing this class directly users should harness the
/// KineticConnectionFactory
class ThreadsafeNonblockingKineticConnection : public NonblockingKineticConnectionInterface {

public:
    explicit ThreadsafeNonblockingKineticConnection(unique_ptr<NonblockingKineticConnection> connection);
    ~ThreadsafeNonblockingKineticConnection();
    bool Run(fd_set *read_fds, fd_set *write_fds, int *nfds);
    bool RemoveHandler(HandlerKey handler_key);
    void SetClientClusterVersion(int64_t cluster_version);

    HandlerKey NoOp(const shared_ptr<SimpleCallbackInterface> callback);
      HandlerKey Get(const string key, const shared_ptr<GetCallbackInterface> callback);
      HandlerKey Get(const shared_ptr<const string> key, const shared_ptr<GetCallbackInterface> callback);
      HandlerKey GetNext(const shared_ptr<const string> key, const shared_ptr<GetCallbackInterface> callback);
      HandlerKey GetNext(const string key, const shared_ptr<GetCallbackInterface> callback);
      HandlerKey GetPrevious(const shared_ptr<const string> key, const shared_ptr<GetCallbackInterface> callback);
      HandlerKey GetPrevious(const string key, const shared_ptr<GetCallbackInterface> callback);
      HandlerKey GetVersion(const shared_ptr<const string> key, const shared_ptr<GetVersionCallbackInterface> callback);
      HandlerKey GetVersion(const string key, const shared_ptr<GetVersionCallbackInterface> callback);
      HandlerKey GetKeyRange(const shared_ptr<const string> start_key, bool start_key_inclusive,
          const shared_ptr<const string> end_key, bool end_key_inclusive,
          bool reverse_results, int32_t max_results, const shared_ptr<GetKeyRangeCallbackInterface> callback);
      HandlerKey GetKeyRange(const string start_key, bool start_key_inclusive,
          const string end_key, bool end_key_inclusive,
          bool reverse_results, int32_t max_results, const shared_ptr<GetKeyRangeCallbackInterface> callback);
      HandlerKey Put(const shared_ptr<const string> key, const shared_ptr<const string> current_version, WriteMode mode,
          const shared_ptr<const KineticRecord> record, const shared_ptr<PutCallbackInterface> callback);
      HandlerKey Put(const string key, const string current_version, WriteMode mode,
          const shared_ptr<const KineticRecord> record, const shared_ptr<PutCallbackInterface> callback);
      HandlerKey Put(const shared_ptr<const string> key, const shared_ptr<const string> current_version, WriteMode mode,
          const shared_ptr<const KineticRecord> record, const shared_ptr<PutCallbackInterface> callback,
          PersistMode persistMode);
      HandlerKey Put(const string key, const string current_version, WriteMode mode,
          const shared_ptr<const KineticRecord> record, const shared_ptr<PutCallbackInterface> callback,
          PersistMode persistMode);
      HandlerKey Delete(const shared_ptr<const string> key, const shared_ptr<const string> version, WriteMode mode,
              const shared_ptr<SimpleCallbackInterface> callback, PersistMode persistMode);
      HandlerKey Delete(const string key, const string version, WriteMode mode,
              const shared_ptr<SimpleCallbackInterface> callback, PersistMode persistMode);
      HandlerKey Delete(const shared_ptr<const string> key, const shared_ptr<const string> version, WriteMode mode,
              const shared_ptr<SimpleCallbackInterface> callback);
      HandlerKey Delete(const string key, const string version, WriteMode mode,
          const shared_ptr<SimpleCallbackInterface> callback);
      HandlerKey P2PPush(const P2PPushRequest& push_request, const shared_ptr<P2PPushCallbackInterface> callback);
      HandlerKey P2PPush(const shared_ptr<const P2PPushRequest> push_request,
              const shared_ptr<P2PPushCallbackInterface> callback);
      HandlerKey GetLog(const shared_ptr<GetLogCallbackInterface> callback);
      HandlerKey GetLog(const vector<Command_GetLog_Type>& types, const shared_ptr<GetLogCallbackInterface> callback);

      HandlerKey UpdateFirmware(const shared_ptr<const string> new_firmware, const shared_ptr<SimpleCallbackInterface> callback);
      HandlerKey SetClusterVersion(int64_t new_cluster_version, const shared_ptr<SimpleCallbackInterface> callback);

      HandlerKey InstantErase(const shared_ptr<string> pin, const shared_ptr<SimpleCallbackInterface> callback);
      HandlerKey InstantErase(const string pin, const shared_ptr<SimpleCallbackInterface> callback);
      HandlerKey SecureErase(const shared_ptr<string> pin, const shared_ptr<SimpleCallbackInterface> callback);
      HandlerKey SecureErase(const string pin, const shared_ptr<SimpleCallbackInterface> callback);
      HandlerKey LockDevice(const shared_ptr<string> pin, const shared_ptr<SimpleCallbackInterface> callback);
      HandlerKey LockDevice(const string pin, const shared_ptr<SimpleCallbackInterface> callback);
      HandlerKey UnlockDevice(const shared_ptr<string> pin, const shared_ptr<SimpleCallbackInterface> callback);
      HandlerKey UnlockDevice(const string pin, const shared_ptr<SimpleCallbackInterface> callback);

      HandlerKey SetACLs(const shared_ptr<const list<ACL>> acls, const shared_ptr<SimpleCallbackInterface> callback);
      HandlerKey SetErasePIN(const shared_ptr<const string> new_pin, const shared_ptr<const string> current_pin,
          const shared_ptr<SimpleCallbackInterface> callback);
      HandlerKey SetErasePIN(const string new_pin, const string current_pin,
          const shared_ptr<SimpleCallbackInterface> callback);
      HandlerKey SetLockPIN(const shared_ptr<const string> new_pin, const shared_ptr<const string> current_pin,
              const shared_ptr<SimpleCallbackInterface> callback);
      HandlerKey SetLockPIN(const string new_pin, const string current_pin,
          const shared_ptr<SimpleCallbackInterface> callback);



    private:
    std::recursive_mutex mutex_;
    std::unique_ptr<NonblockingKineticConnection> connection_;
    DISALLOW_COPY_AND_ASSIGN(ThreadsafeNonblockingKineticConnection);
};

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_THREADSAFE_NONBLOCKING_CONNECTION_H_
