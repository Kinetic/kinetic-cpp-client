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

#ifndef KINETIC_CPP_CLIENT_NONBLOCKING_KINETIC_CONNECTION_H_
#define KINETIC_CPP_CLIENT_NONBLOCKING_KINETIC_CONNECTION_H_

#include <sys/select.h>

#include <string>
#include <memory>
#include <cinttypes>

#include "protobufutil/common.h"

#include "kinetic_client.pb.h"
#include "kinetic/kinetic_record.h"
#include "kinetic/drive_log.h"
#include "kinetic/acls.h"
#include "kinetic/kinetic_connection.h"
#include "kinetic/nonblocking_packet_service_interface.h"
#include "kinetic/kinetic_status.h"

namespace kinetic {

using com::seagate::kinetic::client::proto::Message_MessageType;

using com::seagate::kinetic::client::proto::Message_P2POperation;

using com::seagate::kinetic::client::proto::Message_Synchronization;

using std::shared_ptr;
using std::unique_ptr;
using std::string;
using std::make_shared;
using std::list;
using std::vector;

// SimpleCallbackInterface is used for operations that return success or an
// error but do not otherwise return a result.
class SimpleCallbackInterface {
    public:
    virtual ~SimpleCallbackInterface() {}
    virtual void Success() = 0;
    virtual void Failure(KineticStatus error) = 0;
};

class SimpleHandler : public HandlerInterface {
    public:
    explicit SimpleHandler(const shared_ptr<SimpleCallbackInterface> callback);
    void Handle(const Message &response, unique_ptr<const string> value);
    void Error(KineticStatus error);

    private:
    const shared_ptr<SimpleCallbackInterface> callback_;
    DISALLOW_COPY_AND_ASSIGN(SimpleHandler);
};

class GetCallbackInterface {
    public:
    virtual ~GetCallbackInterface() {}
    virtual void Success(const std::string &key, std::unique_ptr<KineticRecord> record) = 0;
    virtual void Failure(KineticStatus error) = 0;
};

class GetHandler : public HandlerInterface {
    public:
    explicit GetHandler(const shared_ptr<GetCallbackInterface> callback);
    void Handle(const Message &response, unique_ptr<const string> value);
    void Error(KineticStatus error);

    private:
    const shared_ptr<GetCallbackInterface> callback_;
    DISALLOW_COPY_AND_ASSIGN(GetHandler);
};

class GetVersionCallbackInterface {
    public:
    virtual ~GetVersionCallbackInterface() {}
    virtual void Success(const std::string &version) = 0;
    virtual void Failure(KineticStatus error) = 0;
};

class GetVersionHandler : public HandlerInterface {
    public:
    explicit GetVersionHandler(const shared_ptr<GetVersionCallbackInterface> callback);
    void Handle(const Message &response, unique_ptr<const string> value);
    void Error(KineticStatus error);

    private:
    const shared_ptr<GetVersionCallbackInterface> callback_;
    DISALLOW_COPY_AND_ASSIGN(GetVersionHandler);
};

class GetKeyRangeCallbackInterface {
    public:
    virtual ~GetKeyRangeCallbackInterface() {}
    virtual void Success(unique_ptr<vector<string>> keys) = 0;
    virtual void Failure(KineticStatus error) = 0;
};

class GetKeyRangeHandler : public HandlerInterface {
    public:
    explicit GetKeyRangeHandler(const shared_ptr<GetKeyRangeCallbackInterface> callback);
    void Handle(const Message &response, unique_ptr<const string> value);
    void Error(KineticStatus error);

    private:
    const shared_ptr<GetKeyRangeCallbackInterface>  callback_;
    DISALLOW_COPY_AND_ASSIGN(GetKeyRangeHandler);
};

class PutCallbackInterface {
    public:
    virtual ~PutCallbackInterface() {}
    virtual void Success() = 0;
    virtual void Failure(KineticStatus error) = 0;
};


class PutHandler : public HandlerInterface {
    public:
    explicit PutHandler(const shared_ptr<PutCallbackInterface> callback);
    void Handle(const Message &response, unique_ptr<const string> value);
    void Error(KineticStatus error);

    private:
    const shared_ptr<PutCallbackInterface> callback_;
    DISALLOW_COPY_AND_ASSIGN(PutHandler);
};

class GetLogCallbackInterface {
    public:
    virtual ~GetLogCallbackInterface() {}
    virtual void Success(unique_ptr<DriveLog> drive_log) = 0;
    virtual void Failure(KineticStatus error) = 0;
};

class GetLogHandler : public HandlerInterface {
    public:
    explicit GetLogHandler(const shared_ptr<GetLogCallbackInterface> callback);
    void Handle(const Message& response, unique_ptr<const string> value);
    void Error(KineticStatus error);

    private:
    const shared_ptr<GetLogCallbackInterface> callback_;
    DISALLOW_COPY_AND_ASSIGN(GetLogHandler);
};

class P2PPushCallbackInterface {
    public:
    virtual ~P2PPushCallbackInterface() {}
    virtual void Success(unique_ptr<vector<KineticStatus>> operation_statuses) = 0;
    virtual void Failure(KineticStatus error) = 0;
};

class P2PPushHandler : public HandlerInterface {
    public:
    explicit P2PPushHandler(const shared_ptr<P2PPushCallbackInterface> callback);
    void Handle(const Message& response, unique_ptr<const string> value);
    void Error(KineticStatus error);

    private:
    const shared_ptr<P2PPushCallbackInterface> callback_;
    DISALLOW_COPY_AND_ASSIGN(P2PPushHandler);
};

struct P2PPushRequest;

/// Represents a single P2P copy operation
struct P2PPushOperation {
    /// The key to copy
    string key;

    /// The expected version on the remote peer. The same as specifying a version with a regular
    /// put
    string version;

    /// Optionally specify that the key should be copied to a different key on the remote drive.
    /// Leave blank to P2P copy to the same key.
    string newKey;

    /// Ignore version on the remote drive. The same as specifying force with a regular put.
    bool force;
};

/// Represents a collection of P2P operations
struct P2PPushRequest {
    /// Host/IP address of the target drive
    string host;

    /// Port of the target drive
    int port;

    /// Operations to perform against the drive specified above
    vector<P2PPushOperation> operations;

    /// P2P operations to execute on the drive specified above. This lets
    /// you set up a pipeline of P2P push. For example, a client can push
    /// a set of keys to A, and in the same request instruct A to push
    /// keys to B, and so on.
    vector<P2PPushRequest> requests;
};

class NonblockingKineticConnection {
    public:
    explicit NonblockingKineticConnection(NonblockingPacketServiceInterface *service);
    virtual ~NonblockingKineticConnection();
    virtual bool Run(fd_set *read_fds, fd_set *write_fds, int *nfds);
    virtual void SetClientClusterVersion(int64_t cluster_version);

    virtual HandlerKey NoOp(const shared_ptr<SimpleCallbackInterface> callback);
    virtual HandlerKey Get(const string key, const shared_ptr<GetCallbackInterface> callback);
    virtual HandlerKey Get(const shared_ptr<const string> key,
        const shared_ptr<GetCallbackInterface> callback);
    virtual HandlerKey GetNext(const shared_ptr<const string> key,
        const shared_ptr<GetCallbackInterface> callback);
    virtual HandlerKey GetNext(const string key,
        const shared_ptr<GetCallbackInterface> callback);
    virtual HandlerKey GetPrevious(const shared_ptr<const string> key,
        const shared_ptr<GetCallbackInterface> callback);
    virtual HandlerKey GetPrevious(const string key,
        const shared_ptr<GetCallbackInterface> callback);
    virtual HandlerKey GetVersion(const shared_ptr<const string> key,
        const shared_ptr<GetVersionCallbackInterface> callback);
    virtual HandlerKey GetVersion(const string key,
        const shared_ptr<GetVersionCallbackInterface> callback);
    virtual HandlerKey GetKeyRange(const shared_ptr<const string> start_key,
        bool start_key_inclusive,
        const shared_ptr<const string> end_key,
        bool end_key_inclusive,
        bool reverse_results,
        int32_t max_results,
        const shared_ptr<GetKeyRangeCallbackInterface> callback);
    virtual HandlerKey GetKeyRange(const string start_key,
        bool start_key_inclusive,
        const string end_key,
        bool end_key_inclusive,
        bool reverse_results,
        int32_t max_results,
        const shared_ptr<GetKeyRangeCallbackInterface> callback);
    virtual HandlerKey Put(const shared_ptr<const string> key,
        const shared_ptr<const string> current_version, WriteMode mode,
        const shared_ptr<const KineticRecord> record,
        const shared_ptr<PutCallbackInterface> callback);
    virtual HandlerKey Put(const string key,
        const string current_version, WriteMode mode,
        const shared_ptr<const KineticRecord> record,
        const shared_ptr<PutCallbackInterface> callback);
    virtual HandlerKey Put(const shared_ptr<const string> key,
        const shared_ptr<const string> current_version, WriteMode mode,
        const shared_ptr<const KineticRecord> record,
        const shared_ptr<PutCallbackInterface> callback,
        PersistMode persistMode);
    virtual HandlerKey Put(const string key,
        const string current_version, WriteMode mode,
        const shared_ptr<const KineticRecord> record,
        const shared_ptr<PutCallbackInterface> callback,
        PersistMode persistMode);
    virtual HandlerKey Delete(const shared_ptr<const string> key,
            const shared_ptr<const string> version, WriteMode mode,
            const shared_ptr<SimpleCallbackInterface> callback, PersistMode persistMode);
    virtual HandlerKey Delete(const string key, const string version, WriteMode mode,
        const shared_ptr<SimpleCallbackInterface> callback, PersistMode persistMode);
    virtual HandlerKey Delete(const shared_ptr<const string> key,
            const shared_ptr<const string> version, WriteMode mode,
            const shared_ptr<SimpleCallbackInterface> callback);
    virtual HandlerKey Delete(const string key, const string version, WriteMode mode,
        const shared_ptr<SimpleCallbackInterface> callback);
    virtual HandlerKey InstantSecureErase(const shared_ptr<string> pin,
        const shared_ptr<SimpleCallbackInterface> callback);
    virtual HandlerKey InstantSecureErase(const string pin,
        const shared_ptr<SimpleCallbackInterface> callback);
    virtual HandlerKey SetClusterVersion(int64_t new_cluster_version,
        const shared_ptr<SimpleCallbackInterface> callback);
    virtual HandlerKey GetLog(const shared_ptr<GetLogCallbackInterface> callback);
    virtual HandlerKey UpdateFirmware(const shared_ptr<const string> new_firmware,
        const shared_ptr<SimpleCallbackInterface> callback);
    virtual HandlerKey SetACLs(const shared_ptr<const list<ACL>> acls,
        const shared_ptr<SimpleCallbackInterface> callback);
    virtual HandlerKey SetPIN(const shared_ptr<const string> new_pin,
        const shared_ptr<const string> current_pin,
        const shared_ptr<SimpleCallbackInterface> callback);
    virtual HandlerKey SetPIN(const string new_pin,
        const string current_pin,
        const shared_ptr<SimpleCallbackInterface> callback);
    virtual HandlerKey P2PPush(const P2PPushRequest& push_request,
        const shared_ptr<P2PPushCallbackInterface> callback);
    virtual HandlerKey P2PPush(const shared_ptr<const P2PPushRequest> push_request,
        const shared_ptr<P2PPushCallbackInterface> callback);
    virtual bool RemoveHandler(HandlerKey handler_key);

    private:
    HandlerKey GenericGet(const shared_ptr<const string> key,
        const shared_ptr<GetCallbackInterface> callback, Message_MessageType message_type);
    void PopulateP2PMessage(Message_P2POperation *mutable_p2pop,
        const shared_ptr<const P2PPushRequest> push_request);
    unique_ptr<Message> NewMessage(Message_MessageType message_type);
    Message_Synchronization GetSynchronizationForPersistMode(PersistMode persistMode);

    NonblockingPacketServiceInterface *service_;
    const shared_ptr<const string> empty_str_;

    int64_t cluster_version_;

    DISALLOW_COPY_AND_ASSIGN(NonblockingKineticConnection);
};

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_NONBLOCKING_KINETIC_CONNECTION_H_
