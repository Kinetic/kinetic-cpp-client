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

#ifndef KINETIC_CPP_CLIENT_MOCK_CALLBACKS_H_
#define KINETIC_CPP_CLIENT_MOCK_CALLBACKS_H_

#include "gmock/gmock.h"

#include "kinetic/nonblocking_kinetic_connection.h"
#include "kinetic/nonblocking_packet_service_interface.h"
#include "kinetic/kinetic_record.h"

namespace kinetic {

using std::make_shared;
using std::unique_ptr;
using std::shared_ptr;
using std::string;
using std::list;
using std::move;
using std::vector;

class MockSimpleCallback : public SimpleCallbackInterface {
    public:
    MOCK_METHOD0(Success, void());
    MOCK_METHOD1(Failure, void(KineticStatus error));
};

class MockGetCallback : public GetCallbackInterface {
    public:
    // This is needed because gmock doesn't support mocking methods with non-copyable
    // arguments
    void Success(const string& key, unique_ptr<KineticRecord> record) {
        Success_(key, record.get());
    }
    MOCK_METHOD2(Success_, void(const string &key, KineticRecord* record));
    MOCK_METHOD1(Failure, void(KineticStatus error));
};

class MockGetVersionCallback : public GetVersionCallbackInterface {
    public:
    MOCK_METHOD1(Success, void(const string &version));
    MOCK_METHOD1(Failure, void(KineticStatus error));
};


class MockGetKeyRangeCallback : public GetKeyRangeCallbackInterface {
    public:
    void Success(unique_ptr<vector<string>> keys) {
        Success_(keys.get());
    }
    MOCK_METHOD1(Success_, void(vector<string>* keys));
    MOCK_METHOD1(Failure, void(KineticStatus error));
};


class MockPutCallback : public PutCallbackInterface {
    public:
    MOCK_METHOD0(Success, void());
    MOCK_METHOD1(Failure, void(KineticStatus error));
};


class MockGetLogCallback : public GetLogCallbackInterface {
    public:
    void Success(unique_ptr<DriveLog> drive_log) {
        Success_(*drive_log);
    }

    MOCK_METHOD1(Success_, void(DriveLog drive_log));
    MOCK_METHOD1(Failure, void(KineticStatus error));
};

class MockP2PPushCallback : public P2PPushCallbackInterface {
    public:
    void Success(unique_ptr<vector<KineticStatus>> statuses) {
        Success_(*statuses);
    }

    MOCK_METHOD1(Success_, void(vector<KineticStatus> statuses));
    MOCK_METHOD1(Failure, void(KineticStatus error));
};



} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_MOCK_CALLBACKS_H_
