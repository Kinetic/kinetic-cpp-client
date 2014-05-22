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

#include <mutex>

#include "kinetic/nonblocking_kinetic_connection.h"
#include "protobufutil/common.h"

namespace kinetic {

/// Kinetic connection class variant that synchronizes concurrent access and allows non-blocking
/// IO. Instead of constructing this class directly users should harness the
/// KineticConnectionFactory
class ThreadsafeNonblockingKineticConnection : public NonblockingKineticConnection {
    public:
    explicit ThreadsafeNonblockingKineticConnection(NonblockingPacketServiceInterface *service);
    ~ThreadsafeNonblockingKineticConnection();

    /// Attempts to make progress on any outstanding requests. read_fds, write_fds, and nfds will
    /// be populated based on any FDs that are waiting for IO. They can then be passed in to
    /// a select() like interface to wait for data to be available. Any callbacks that need
    /// to be invoked based on input Run receives will be called on the thread that called Run.
    /// @param[out] read_fds
    /// @param[out] write_fds
    /// @param[out] nfds
    /// @return true if everything went well; false otherwise. If this method returns false
    /// the connection is broken and needs to be discarded
    bool Run(fd_set *read_fds, fd_set *write_fds, int *nfds);

    /// If the drive has a non-zero cluster version, requests will fail unless the developer
    /// tells the client the correct cluster version using this method.
    void SetClientClusterVersion(int64_t cluster_version);

    HandlerKey NoOp(const shared_ptr<SimpleCallbackInterface> callback);
    HandlerKey Get(const shared_ptr<const string> key,
            const shared_ptr<GetCallbackInterface> callback);
    HandlerKey GetNext(const shared_ptr<const string> key,
            const shared_ptr<GetCallbackInterface>callback);
    HandlerKey GetPrevious(const shared_ptr<const string> key,
            const shared_ptr<GetCallbackInterface>callback);
    HandlerKey GetVersion(const shared_ptr<const string> key,
            const shared_ptr<GetVersionCallbackInterface> callback);
    HandlerKey GetKeyRange(const shared_ptr<const string> start_key,
        bool start_key_inclusive,
        const shared_ptr<const string> end_key,
        bool end_key_inclusive,
        bool reverse_results,
        int32_t max_results,
        const shared_ptr<GetKeyRangeCallbackInterface> callback);
    HandlerKey Put(const shared_ptr<const string> key,
        const shared_ptr<const string> current_version, WriteMode mode,
        const shared_ptr<const KineticRecord> record,
        const shared_ptr<PutCallbackInterface> callback,
        PersistMode persistMode);
    HandlerKey Delete(
            const shared_ptr<const string> key,
            const shared_ptr<const string> version,
            WriteMode mode,
            const shared_ptr<SimpleCallbackInterface> callback,
            PersistMode persistMode);
    HandlerKey InstantSecureErase(const shared_ptr<string> pin,
        const shared_ptr<SimpleCallbackInterface> callback);
    HandlerKey SetClusterVersion(int64_t new_cluster_version,
        const shared_ptr<SimpleCallbackInterface> callback);
    HandlerKey GetLog(const shared_ptr<GetLogCallbackInterface> callback);
    HandlerKey UpdateFirmware(const shared_ptr<const string> new_firmware,
        const shared_ptr<SimpleCallbackInterface> callback);
    HandlerKey SetACLs(const shared_ptr<const list<ACL>> acls,
        const shared_ptr<SimpleCallbackInterface> callback);
    HandlerKey SetPIN(const shared_ptr<const string> new_pin,
        const shared_ptr<const string> current_pin,
        const shared_ptr<SimpleCallbackInterface> callback);
    HandlerKey P2PPush(const shared_ptr<const P2PPushRequest> push_request,
        const shared_ptr<P2PPushCallbackInterface> callback);
    bool RemoveHandler(HandlerKey handler_key);

    private:
    std::mutex mutex_;

    DISALLOW_COPY_AND_ASSIGN(ThreadsafeNonblockingKineticConnection);
};

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_THREADSAFE_NONBLOCKING_CONNECTION_H_
