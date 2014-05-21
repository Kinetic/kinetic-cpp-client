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
#include <sys/select.h>
#include <errno.h>
#include <stdexcept>

#include "kinetic/blocking_kinetic_connection.h"


namespace kinetic {

using std::shared_ptr;
using std::unique_ptr;
using std::string;
using std::make_shared;
using std::move;

BlockingKineticConnection::BlockingKineticConnection(
        shared_ptr<NonblockingKineticConnection> nonblocking_connection, unsigned int network_timeout_seconds)
    : nonblocking_connection_(nonblocking_connection),
    network_timeout_seconds_(network_timeout_seconds) {}

BlockingKineticConnection::BlockingKineticConnection(
        unique_ptr<NonblockingKineticConnection> nonblocking_connection, unsigned int network_timeout_seconds)
    : nonblocking_connection_(shared_ptr<NonblockingKineticConnection>(nonblocking_connection.release())),
    network_timeout_seconds_(network_timeout_seconds) {}

BlockingKineticConnection::~BlockingKineticConnection() {}

class BlockingCallbackState {
    friend class BlockingKineticConnection;

    public:
    BlockingCallbackState() : done_(false), success_(false),
        error_(KineticStatus(StatusCode::OK, "default -- never seen")) { }

    virtual ~BlockingCallbackState() {}

    protected:
    bool done_;
    bool success_;
    KineticStatus error_;

    void OnSuccess() {
        done_ = true;
        success_ = true;
    }

    void OnError(KineticStatus error) {
        done_ = true;
        success_ = false;
        error_ = error;
    }
};

class SimpleCallback : public SimpleCallbackInterface, public BlockingCallbackState {
    public:
    virtual void Success() {
        OnSuccess();
    }

    virtual void Failure(KineticStatus error) {
        OnError(error);
    }

    private:
};

KineticStatus BlockingKineticConnection::NoOp() {
    auto handler = make_shared<SimpleCallback>();
    return RunOperation(handler, nonblocking_connection_->NoOp(handler));
}

class BlockingGetCallback : public GetCallbackInterface, public BlockingCallbackState {
    public:
    BlockingGetCallback(
            unique_ptr<string>& actual_key,
            unique_ptr<KineticRecord>& record,
            bool want_actual_key)
        : actual_key_(actual_key), record_(record),
        want_actual_key_(want_actual_key) {}
    virtual void Success(const string &key, unique_ptr<KineticRecord> record) {
        OnSuccess();

        if (want_actual_key_) {
            if (actual_key_) {
                *actual_key_ = key;
            } else {
                actual_key_.reset(new string(key));
            }
        }

        record_ = std::move(record);
    }

    virtual void Failure(KineticStatus error) {
        OnError(error);
    }

    private:
    unique_ptr<string>& actual_key_;
    unique_ptr<KineticRecord>& record_;
    bool want_actual_key_;
};

class BlockingGetVersionCallback : public GetVersionCallbackInterface,
        public BlockingCallbackState {
    public:
    explicit BlockingGetVersionCallback(unique_ptr<string>& version)
    : version_(version) {}

    virtual void Success(const std::string& version) {
        OnSuccess();

        if (version_) {
            *version_ = version;
        } else {
            version_.reset(new string(version));
        }
    }

    virtual void Failure(KineticStatus error) {
        OnError(error);
    }

    private:
    unique_ptr<string>& version_;
};

class BlockingGetKeyRangeCallback : public GetKeyRangeCallbackInterface,
        public BlockingCallbackState {
    public:
    explicit BlockingGetKeyRangeCallback(unique_ptr<vector<string>>& keys)
    :  keys_(keys) {}

    virtual void Success(unique_ptr<vector<string>> keys) {
        OnSuccess();

        keys_ = move(keys);
    }

    virtual void Failure(KineticStatus error) {
        OnError(error);
    }

    private:
    unique_ptr<vector<string>>& keys_;
};

void BlockingKineticConnection::SetClientClusterVersion(int64_t cluster_version) {
    nonblocking_connection_->SetClientClusterVersion(cluster_version);
}

KineticStatus BlockingKineticConnection::Get(const shared_ptr<const string> key,
    unique_ptr<KineticRecord>& record) {
    unique_ptr<string> actual_key(nullptr);
    auto handler = make_shared<BlockingGetCallback>(actual_key, record, false);
    return RunOperation(handler, nonblocking_connection_->Get(key, handler));
}

KineticStatus BlockingKineticConnection::Get(const string& key, unique_ptr<KineticRecord>& record) {
    return this->Get(make_shared<string>(key), record);
}

class BlockingPutCallback : public PutCallbackInterface, public BlockingCallbackState {
    public:
    virtual void Success() {
        OnSuccess();
    }

    virtual void Failure(KineticStatus error) {
        OnError(error);
    }

    private:
};

KineticStatus BlockingKineticConnection::Put(const shared_ptr<const string> key,
        const shared_ptr<const string> current_version, WriteMode mode,
        const shared_ptr<const KineticRecord> record,
        PersistMode persistMode) {
    auto handler = make_shared<BlockingPutCallback>();

    return RunOperation(handler,
            nonblocking_connection_->Put(key, current_version, mode, record, handler, persistMode));
}

KineticStatus BlockingKineticConnection::Put(const string& key,
        const string& current_version, WriteMode mode,
        const KineticRecord& record,
        PersistMode persistMode) {
    return this->Put(make_shared<string>(key), make_shared<string>(current_version), mode,
        make_shared<KineticRecord>(record), persistMode);
}

KineticStatus BlockingKineticConnection::Put(const shared_ptr<const string> key,
        const shared_ptr<const string> current_version, WriteMode mode,
        const shared_ptr<const KineticRecord> record) {
    auto handler = make_shared<BlockingPutCallback>();

    // Rely on nonblocking_connection to handle the default PersistMode case
    return RunOperation(handler,
            nonblocking_connection_->Put(key, current_version, mode, record, handler));
}

KineticStatus BlockingKineticConnection::Put(const string& key,
        const string& current_version, WriteMode mode,
        const KineticRecord& record) {
    return this->Put(make_shared<string>(key), make_shared<string>(current_version), mode,
        make_shared<KineticRecord>(record));
}

KineticStatus BlockingKineticConnection::Delete(const shared_ptr<const string> key,
    const shared_ptr<const string> version, WriteMode mode, PersistMode persistMode) {
    auto callback = make_shared<SimpleCallback>();
    return RunOperation(callback, nonblocking_connection_->Delete(key, version, mode,
            callback, persistMode));
}

KineticStatus BlockingKineticConnection::Delete(const string& key, const string& version,
    WriteMode mode, PersistMode persistMode) {
    return this->Delete(make_shared<string>(key),
            make_shared<string>(version), mode, persistMode);
}

KineticStatus BlockingKineticConnection::Delete(const shared_ptr<const string> key,
    const shared_ptr<const string> version, WriteMode mode) {
    auto callback = make_shared<SimpleCallback>();
    // Let the nonblocking_connection handle the default persistOption
    return RunOperation(callback, nonblocking_connection_->Delete(key, version, mode, callback));
}

KineticStatus BlockingKineticConnection::Delete(const string& key, const string& version,
    WriteMode mode) {
    return this->Delete(make_shared<string>(key), make_shared<string>(version), mode);
}


KineticStatus BlockingKineticConnection::InstantSecureErase(const shared_ptr<string> pin) {
    auto callback = make_shared<SimpleCallback>();
    return RunOperation(callback, nonblocking_connection_->InstantSecureErase(pin, callback));
}

KineticStatus BlockingKineticConnection::InstantSecureErase(const string& pin) {
    return this->InstantSecureErase(make_shared<string>(pin));
}

KineticStatus BlockingKineticConnection::SetClusterVersion(int64_t new_cluster_version) {
    auto callback = make_shared<SimpleCallback>();
    return RunOperation(callback,
            nonblocking_connection_->SetClusterVersion(new_cluster_version, callback));
}

class GetLogCallback : public GetLogCallbackInterface, public BlockingCallbackState {
    public:
    explicit GetLogCallback(unique_ptr<DriveLog>& drive_log) :  drive_log_(drive_log) {}

    virtual void Success(unique_ptr<DriveLog> drive_log) {
        OnSuccess();
        drive_log_ = std::move(drive_log);
    }
    virtual void Failure(KineticStatus error) {
        OnError(error);
    }

    private:
    unique_ptr<DriveLog>& drive_log_;
};

KineticStatus BlockingKineticConnection::GetLog(unique_ptr<DriveLog>& drive_log) {
    auto callback = make_shared<GetLogCallback>(drive_log);
    return RunOperation(callback, nonblocking_connection_->GetLog(callback));
}

KineticStatus BlockingKineticConnection::UpdateFirmware(const shared_ptr<const string>
        new_firmware) {
    auto callback = make_shared<SimpleCallback>();
    return RunOperation(callback, nonblocking_connection_->UpdateFirmware(new_firmware, callback));
}

KineticStatus BlockingKineticConnection::SetACLs(const shared_ptr<const list<ACL>> acls) {
    auto callback = make_shared<SimpleCallback>();
    return RunOperation(callback, nonblocking_connection_->SetACLs(acls, callback));
}

KineticStatus BlockingKineticConnection::SetPin(const shared_ptr<const string> new_pin,
    const shared_ptr<const string> current_pin) {
    auto callback = make_shared<SimpleCallback>();
    return RunOperation(callback, nonblocking_connection_->SetPIN(new_pin, current_pin, callback));
}

KineticStatus BlockingKineticConnection::SetPin(const string& new_pin, const string& current_pin) {
    return this->SetPin(make_shared<string>(new_pin), make_shared<string>(current_pin));
}

KineticStatus BlockingKineticConnection::GetNext(const shared_ptr<const string> key,
        unique_ptr<string>& actual_key, unique_ptr<KineticRecord>& record) {
    auto callback = make_shared<BlockingGetCallback>(actual_key, record, true);
    return RunOperation(callback, nonblocking_connection_->GetNext(key, callback));
}

KineticStatus BlockingKineticConnection::GetNext(const string& key,
        unique_ptr<string>& actual_key, unique_ptr<KineticRecord>& record) {
    return this->GetNext(make_shared<string>(key), actual_key, record);
}

KineticStatus BlockingKineticConnection::GetPrevious(const shared_ptr<const string> key,
        unique_ptr<string>& actual_key, unique_ptr<KineticRecord>& record) {
    auto callback = make_shared<BlockingGetCallback>(actual_key, record,
            true);
    return RunOperation(callback, nonblocking_connection_->GetPrevious(key, callback));
}

KineticStatus BlockingKineticConnection::GetPrevious(const string& key,
        unique_ptr<string>& actual_key, unique_ptr<KineticRecord>& record) {
    return this->GetPrevious(make_shared<string>(key), actual_key, record);
}

KineticStatus BlockingKineticConnection::GetVersion(const shared_ptr<const string> key,
        unique_ptr<string>& version) {
    auto callback = make_shared<BlockingGetVersionCallback>(version);
    return RunOperation(callback, nonblocking_connection_->GetVersion(key, callback));
}

KineticStatus BlockingKineticConnection::GetVersion(const string& key,
        unique_ptr<string>& version) {
    return this->GetVersion(make_shared<string>(key), version);
}

KineticStatus BlockingKineticConnection::GetKeyRange(const shared_ptr<const string> start_key,
        bool start_key_inclusive,
        const shared_ptr<const string> end_key,
        bool end_key_inclusive,
        bool reverse_results,
        int32_t max_results,
        unique_ptr<vector<string>>& keys) {
    auto callback = make_shared<BlockingGetKeyRangeCallback>(keys);

    return RunOperation(callback,
            nonblocking_connection_->GetKeyRange(start_key,
                    start_key_inclusive,
                    end_key,
                    end_key_inclusive,
                    reverse_results,
                    max_results,
                    callback));
}

KineticStatus BlockingKineticConnection::GetKeyRange(const string& start_key,
        bool start_key_inclusive,
        const string& end_key,
        bool end_key_inclusive,
        bool reverse_results,
        int32_t max_results,
        unique_ptr<vector<string>>& keys) {
    return this->GetKeyRange(make_shared<string>(start_key),
        start_key_inclusive, make_shared<string>(end_key),
        end_key_inclusive, reverse_results, max_results,
        keys);
}


KeyRangeIterator BlockingKineticConnection::IterateKeyRange(
        const shared_ptr<const string> start_key,
        bool start_key_inclusive,
        const shared_ptr<const string> end_key,
        bool end_key_inclusive,
        unsigned int frame_size) {
    KeyRangeIterator it(this,
            frame_size,
            *start_key,
            start_key_inclusive,
            *end_key,
            end_key_inclusive);
    return it;
}

KeyRangeIterator BlockingKineticConnection::IterateKeyRange(const string& start_key,
        bool start_key_inclusive,
        const string& end_key,
        bool end_key_inclusive,
        unsigned int frame_size) {
    return this->IterateKeyRange(make_shared<string>(start_key),
        start_key_inclusive, make_shared<string>(end_key),
        end_key_inclusive, frame_size);
}

KineticStatus BlockingKineticConnection::P2PPush(const P2PPushRequest& push_request,
        unique_ptr<vector<KineticStatus>>& operation_statuses) {
    return this->P2PPush(make_shared<P2PPushRequest>(push_request), operation_statuses);
}

class BlockingP2PPushCallback : public P2PPushCallbackInterface, public BlockingCallbackState {
    public:
    explicit BlockingP2PPushCallback(unique_ptr<vector<KineticStatus>>& statuses)
    : statuses_(statuses) {}
    virtual void Success(unique_ptr<vector<KineticStatus>> statuses) {
        OnSuccess();
        statuses_ = std::move(statuses);
    }
    virtual void Failure(KineticStatus error) {
        OnError(error);
    }

    private:
    unique_ptr<vector<KineticStatus>>& statuses_;
};

KineticStatus BlockingKineticConnection::P2PPush(
        const shared_ptr<const P2PPushRequest> push_request,
        unique_ptr<vector<KineticStatus>>& operation_statuses) {
    auto callback = make_shared<BlockingP2PPushCallback>(operation_statuses);
    return RunOperation(callback,
            nonblocking_connection_->P2PPush(push_request, callback));
}

KineticStatus BlockingKineticConnection::RunOperation(
        shared_ptr<BlockingCallbackState> callback,
        HandlerKey handler_key) {
    fd_set read_fds, write_fds;
    int nfds;

    if (!nonblocking_connection_->Run(&read_fds, &write_fds, &nfds)) {
        nonblocking_connection_->RemoveHandler(handler_key);
        return KineticStatus(StatusCode::CLIENT_IO_ERROR, "Connection failed");
    }

    while (!(callback->done_)) {
        struct timeval tv;
        tv.tv_sec = network_timeout_seconds_;
        tv.tv_usec = 0;

        int number_ready_fds = select(nfds, &read_fds, &write_fds, NULL, &tv);
        if (number_ready_fds < 0) {
            // select() returned an error
            nonblocking_connection_->RemoveHandler(handler_key);
            return KineticStatus(StatusCode::CLIENT_IO_ERROR, strerror(errno));
        } else if (number_ready_fds == 0) {
            // select() returned before any sockets were ready meaning the connection timed out
            nonblocking_connection_->RemoveHandler(handler_key);
            return KineticStatus(StatusCode::CLIENT_IO_ERROR, "Network timeout");
        }

        // At least one FD was ready meaning that the connection is ready
        // to make some progress
        if (!nonblocking_connection_->Run(&read_fds, &write_fds, &nfds)) {
            nonblocking_connection_->RemoveHandler(handler_key);
            return KineticStatus(StatusCode::CLIENT_IO_ERROR, "Connection failed");
        }
    }

    // done was set, meaning handler was invoked and therefore removed internally

    if (callback->success_) {
        return KineticStatus(StatusCode::OK, "");
    } else {
        return callback->error_;
    }
}

} // namespace kinetic
