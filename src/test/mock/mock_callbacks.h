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
    void Success(unique_ptr<vector<KineticStatus>> statuses, const Command& response) {
        Success_(*statuses, response);
    }

    MOCK_METHOD2(Success_, void(vector<KineticStatus> statuses, const Command& response));
    MOCK_METHOD2(Failure, void(KineticStatus error, Command const * const response));
};



} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_MOCK_CALLBACKS_H_
