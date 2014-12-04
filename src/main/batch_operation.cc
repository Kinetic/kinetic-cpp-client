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

#include "kinetic/batch_operation.h"
#include <ctime>

namespace kinetic {

class FireAndForget : public SimpleCallbackInterface, public PutCallbackInterface {
    public:
    void Success() {}
    void Failure(KineticStatus error){}
};

class ResultCallback : public SimpleCallbackInterface {
private:
    KineticStatus result;
    bool done;

public:
    ResultCallback(): result(KineticStatus(StatusCode::CLIENT_IO_ERROR, "No Result")), done(false) {};
    void Success() {
        result = KineticStatus(StatusCode::OK, "");
        done = true;
    }
    void Failure(KineticStatus error) {
        result = error;
        done = true;
    }
    bool Finished(){
        return done;
    }
    KineticStatus getResult(){
        return result;
    }
};

BatchOperation::BatchOperation(std::shared_ptr<NonblockingKineticConnectionInterface> connection, PersistMode mode):
        batch_id(0), persistMode(mode), con(connection){
    con->BatchStart(std::make_shared<FireAndForget>(), &batch_id);
}

BatchOperation::~BatchOperation(){
    if(batch_id) Abort();
}

KineticStatus BatchOperation::Put(const string key, const string current_version, WriteMode mode, const shared_ptr<const KineticRecord> record){
    if(batch_id==0) return KineticStatus(StatusCode::CLIENT_INTERNAL_ERROR, "This Batch Operation is invalid.");
    con->BatchPutKey(batch_id, key, current_version, mode, record, std::make_shared<FireAndForget>(), persistMode);
    return KineticStatus(StatusCode::OK, "");
}

KineticStatus BatchOperation::Put(const shared_ptr<const string> key, const shared_ptr<const string> current_version, WriteMode mode, const shared_ptr<const KineticRecord> record){
    if(batch_id==0) return KineticStatus(StatusCode::CLIENT_INTERNAL_ERROR, "This Batch Operation is invalid.");
    con->BatchPutKey(batch_id, key, current_version, mode, record, std::make_shared<FireAndForget>(), persistMode);
    return KineticStatus(StatusCode::OK, "");
}

KineticStatus BatchOperation::Delete(const string key, const string version, WriteMode mode){
    if(batch_id==0) return KineticStatus(StatusCode::CLIENT_INTERNAL_ERROR, "This Batch Operation is invalid.");
    con->BatchDeleteKey(batch_id, key, version, mode, std::make_shared<FireAndForget>(), persistMode);
    return KineticStatus(StatusCode::OK, "");
}

KineticStatus BatchOperation::Delete(const shared_ptr<const string> key, const shared_ptr<const string> version, WriteMode mode){
    if(batch_id==0) return KineticStatus(StatusCode::CLIENT_INTERNAL_ERROR, "This Batch Operation is invalid.");
    con->BatchDeleteKey(batch_id, key, version, mode, std::make_shared<FireAndForget>(), persistMode);
    return KineticStatus(StatusCode::OK, "");
}

KineticStatus BatchOperation::getResult(const shared_ptr<SimpleCallbackInterface> callback){
    auto cb = std::dynamic_pointer_cast<ResultCallback>(callback);

    fd_set read_fds, write_fds;
    int num_fds = 0;

    con->Run(&read_fds, &write_fds, &num_fds);

    while(cb->Finished() == false){
        struct timeval tv;
        tv.tv_sec = 20;
        tv.tv_usec = 0;

        if(select(num_fds, &read_fds, &write_fds, NULL, &tv) <= 0)
            return KineticStatus(StatusCode::CLIENT_IO_ERROR, "Timeout");
        if(con->Run(&read_fds, &write_fds, &num_fds) == false)
            return KineticStatus(StatusCode::CLIENT_IO_ERROR, "Connection failed");
    }
    return cb->getResult();
}

KineticStatus BatchOperation::Commit(const shared_ptr<SimpleCallbackInterface> cb){
    if(batch_id==0) return KineticStatus(StatusCode::CLIENT_INTERNAL_ERROR, "This Batch Operation is invalid.");
    auto rcb = std::make_shared<ResultCallback>();
    con->BatchCommit(batch_id, cb ? cb : rcb);
    batch_id = 0;
    if(cb) return KineticStatus(StatusCode::OK, "");
    return this->getResult(rcb);
}

KineticStatus BatchOperation::Abort(const shared_ptr<SimpleCallbackInterface> cb){
    if(batch_id==0) return KineticStatus(StatusCode::CLIENT_INTERNAL_ERROR, "This Batch Operation is invalid.");
    auto rcb = std::make_shared<ResultCallback>();
    con->BatchAbort(batch_id, cb ? cb : rcb);
    batch_id = 0;
    if(cb) return KineticStatus(StatusCode::OK, "");
    return this->getResult(rcb);
}

}
