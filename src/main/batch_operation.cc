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

#include "kinetic/batch_operation.h"
#include <ctime>

namespace kinetic {

class FireAndForget : public SimpleCallbackInterface, public PutCallbackInterface {
 public:
    void Success() {}
    void Failure(KineticStatus error) {}
};

class ResultCallback : public SimpleCallbackInterface {
 private:
    KineticStatus result;
    bool done;

 public:
    ResultCallback(): result(KineticStatus(StatusCode::CLIENT_IO_ERROR, "No Result")), done(false) {}
    void Success() {
        result = KineticStatus(StatusCode::OK, "");
        done = true;
    }
    void Failure(KineticStatus error) {
        result = error;
        done = true;
    }
    bool Finished() {
        return done;
    }
    KineticStatus getResult() {
        return result;
    }
};

BatchOperation::BatchOperation(std::shared_ptr<NonblockingKineticConnectionInterface> connection)
:batch_id(0), con(connection), invalid(KineticStatus(StatusCode::CLIENT_INTERNAL_ERROR,
        "This Batch Operation is invalid.")) {
    con->BatchStart(std::make_shared<FireAndForget>(), &batch_id);
}

BatchOperation::~BatchOperation() {
    if (batch_id) Abort();
}

KineticStatus BatchOperation::Put(const string key, const string current_version,
        WriteMode mode, const shared_ptr<const KineticRecord> record) {
    if (batch_id == 0) return invalid;
    con->BatchPutKey(batch_id, key, current_version, mode, record);
    return KineticStatus(StatusCode::OK, "");
}

KineticStatus BatchOperation::Put(const shared_ptr<const string> key, const shared_ptr<const string> current_version,
        WriteMode mode, const shared_ptr<const KineticRecord> record) {
    if (batch_id == 0) return invalid;
    con->BatchPutKey(batch_id, key, current_version, mode, record);
    return KineticStatus(StatusCode::OK, "");
}

KineticStatus BatchOperation::Delete(const string key,
        const string version, WriteMode mode) {
    if (batch_id == 0) return invalid;
    con->BatchDeleteKey(batch_id, key, version, mode);
    return KineticStatus(StatusCode::OK, "");
}

KineticStatus BatchOperation::Delete(const shared_ptr<const string> key,
        const shared_ptr<const string> version, WriteMode mode) {
    if (batch_id == 0) return invalid;
    con->BatchDeleteKey(batch_id, key, version, mode);
    return KineticStatus(StatusCode::OK, "");
}

KineticStatus BatchOperation::getResult(const shared_ptr<SimpleCallbackInterface> callback) {
    auto cb = std::dynamic_pointer_cast<ResultCallback>(callback);

    fd_set read_fds, write_fds;
    int num_fds = 0;

    con->Run(&read_fds, &write_fds, &num_fds);

    while (cb->Finished() == false) {
        struct timeval tv;
        tv.tv_sec = 20;
        tv.tv_usec = 0;

        if (select(num_fds, &read_fds, &write_fds, NULL, &tv) <= 0)
            return KineticStatus(StatusCode::CLIENT_IO_ERROR, "Timeout");
        if (con->Run(&read_fds, &write_fds, &num_fds) == false)
            return KineticStatus(StatusCode::CLIENT_IO_ERROR, "Connection failed");
    }
    return cb->getResult();
}

KineticStatus BatchOperation::Commit(const shared_ptr<SimpleCallbackInterface> cb) {
    if (batch_id == 0) return invalid;
    auto rcb = std::make_shared<ResultCallback>();
    con->BatchCommit(batch_id, cb ? cb : rcb);
    batch_id = 0;
    if (cb) return KineticStatus(StatusCode::OK, "");
    printf("BatchOperation::Commit\n");;
    return this->getResult(rcb);
}

KineticStatus BatchOperation::Abort(const shared_ptr<SimpleCallbackInterface> cb) {
    if (batch_id == 0) return invalid;
    auto rcb = std::make_shared<ResultCallback>();
    con->BatchAbort(batch_id, cb ? cb : rcb);
    batch_id = 0;
    if (cb) return KineticStatus(StatusCode::OK, "");
    return this->getResult(rcb);
}

} // namespace kinetic
