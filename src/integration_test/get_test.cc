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

#include "integration_test.h"
#include "matchers.h"

namespace kinetic {

using ::testing::_;
using ::testing::Assign;
using ::testing::InvokeWithoutArgs;
using ::testing::SaveArg;
using ::testing::StrictMock;
using ::testing::MatcherInterface;
using ::testing::Matcher;
using com::seagate::kinetic::client::proto::Message_Algorithm_SHA1;
using com::seagate::kinetic::client::proto::Message_Algorithm_CRC64;

using std::make_shared;
using std::string;

TEST_F(IntegrationTest, GetNonexistent) {
    auto callback = make_shared<StrictMock<MockGetCallback>>();
    nonblocking_connection_->Get("nonexistent key", callback);
    EXPECT_CALL(*callback,
        Failure(KineticStatusEq(StatusCode::REMOTE_NOT_FOUND, "Key not found")))
        .WillOnce(Assign(&done_, true)); RunSelectLoop();
}

TEST_F(IntegrationTest, GetOk) {
    auto put_callback = make_shared<StrictMock<MockPutCallback>>();
    auto record1 = make_shared<KineticRecord>("value1", "v1", "tag1", Message_Algorithm_SHA1);
    nonblocking_connection_->Put(make_shared<string>("key1"), make_shared<string>(""),
            WriteMode::REQUIRE_SAME_VERSION, record1, put_callback);
    WaitForSuccessSharedPtr(put_callback);

    auto callback = make_shared<StrictMock<MockGetCallback>>();
    nonblocking_connection_->Get("key1", callback);
    EXPECT_CALL(*callback,
        Success_("key1", KineticRecordEq("value1", "v1", "tag1", Message_Algorithm_SHA1)))
        .WillOnce(Assign(&done_, true));
    RunSelectLoop();
}

TEST_F(IntegrationTest, GetNext_KeyExistsAndHasSuccessor) {
    // Write keys "key1" and "key2"
    auto put_callback = make_shared<StrictMock<MockPutCallback>>();
    auto record1 = make_shared<KineticRecord>("value1", "v1", "tag1", Message_Algorithm_SHA1);
    auto record2 = make_shared<KineticRecord>("value2", "v2", "tag2", Message_Algorithm_SHA1);
    nonblocking_connection_->Put(make_shared<string>("key1"), make_shared<string>(""),
        WriteMode::REQUIRE_SAME_VERSION, record1, put_callback);
    WaitForSuccessSharedPtr(put_callback);
    nonblocking_connection_->Put(make_shared<string>("key2"), make_shared<string>(""),
        WriteMode::REQUIRE_SAME_VERSION, record2, put_callback);
    WaitForSuccessSharedPtr(put_callback);

    // Call GetNext on "key1" and verify that we get "key2" and its value
    auto get_callback = make_shared<StrictMock<MockGetCallback>>();
    nonblocking_connection_->GetNext("key1", get_callback);
    EXPECT_CALL(*get_callback, Success_("key2",
            KineticRecordEq("value2", "v2", "tag2", Message_Algorithm_SHA1)))
        .WillOnce(Assign(&done_, true));

    RunSelectLoop();
}

TEST_F(IntegrationTest, GetManyOutstandingRequests) {
    auto put_callback = make_shared<StrictMock<MockPutCallback>>();
    auto record1 = make_shared<KineticRecord>("value1", "v1", "tag1", Message_Algorithm_CRC64);
    nonblocking_connection_->Put(make_shared<string>("key1"), make_shared<string>(""),
        WriteMode::REQUIRE_SAME_VERSION, record1, put_callback);
    WaitForSuccessSharedPtr(put_callback);

    uint32_t num_reqs = 2000;
    uint32_t count = 0;
    Incrementor incr(num_reqs, &done_, &count);

    std::vector<shared_ptr<StrictMock<MockGetCallback>>> callbacks;

    for (uint32_t i = 0; i < num_reqs; i++) {
        auto get_callback = make_shared<StrictMock<MockGetCallback>>();
        callbacks.push_back(get_callback);

        nonblocking_connection_->Get("key1", get_callback);
        EXPECT_CALL(*get_callback, Success_("key1",
                KineticRecordEq("value1", "v1", "tag1", Message_Algorithm_CRC64)))
            .WillOnce(InvokeWithoutArgs(incr));
    }

    RunSelectLoop();
}

TEST_F(IntegrationTest, GetNext_KeyExistsAndDoesNotHaveSuccessor) {
    // Write "key1"
    auto put_callback = make_shared<StrictMock<MockPutCallback>>();
    auto record = make_shared<KineticRecord>("value", "version", "tag", Message_Algorithm_SHA1);
    nonblocking_connection_->Put(make_shared<string>("key1"), make_shared<string>(""),
        WriteMode::REQUIRE_SAME_VERSION, record, put_callback);
    WaitForSuccessSharedPtr(put_callback);

    // Call GetNext on "key1" and verify that we get "not found"
    auto get_callback = make_shared<StrictMock<MockGetCallback>>();
    nonblocking_connection_->GetNext("key1", get_callback);
    WaitForFailure(get_callback, StatusCode::REMOTE_NOT_FOUND, "Key not found");
}

TEST_F(IntegrationTest, GetNext_KeyDoesNotExistAndHasSuccessor) {
    // Write "key2"
    auto put_callback = make_shared<StrictMock<MockPutCallback>>();
    auto record = make_shared<KineticRecord>("value", "version", "tag", Message_Algorithm_CRC64);
    nonblocking_connection_->Put(make_shared<string>("key2"), make_shared<string>(""),
        WriteMode::REQUIRE_SAME_VERSION, record, put_callback);
    WaitForSuccessSharedPtr(put_callback);

    // Call GetNext on "key1" and verify that we get "key2" and its value
    auto get_callback = make_shared<StrictMock<MockGetCallback>>();
    nonblocking_connection_->GetNext("key1", get_callback);
    EXPECT_CALL(*get_callback, Success_("key2",
            KineticRecordEq("value", "version", "tag", Message_Algorithm_CRC64)))
        .WillOnce(Assign(&done_, true));

    RunSelectLoop();
}

TEST_F(IntegrationTest, GetNext_KeyDoesNotExistAndDoesNotHaveSuccessor) {
    // Call GetNext on "key1" and verify that we get "not found"
    auto callback = make_shared<StrictMock<MockGetCallback>>();
    nonblocking_connection_->GetNext("key1", callback);
    EXPECT_CALL(*callback, Failure(KineticStatusEq(StatusCode::REMOTE_NOT_FOUND, "Key not found")))
        .WillOnce(Assign(&done_, true));

    RunSelectLoop();
}

TEST_F(IntegrationTest, GetPrevious_KeyExistsAndHasPredecessor) {
    // Write keys "key1" and "key2"
    auto put_callback = make_shared<StrictMock<MockPutCallback>>();
    auto record1 = make_shared<KineticRecord>("value1", "v1", "tag1", Message_Algorithm_SHA1);
    auto record2 = make_shared<KineticRecord>("value2", "v2", "tag2", Message_Algorithm_SHA1);
    nonblocking_connection_->Put(make_shared<string>("key1"), make_shared<string>(""),
        WriteMode::REQUIRE_SAME_VERSION, record1, put_callback);
    WaitForSuccessSharedPtr(put_callback);
    nonblocking_connection_->Put(make_shared<string>("key2"), make_shared<string>(""),
        WriteMode::REQUIRE_SAME_VERSION, record2, put_callback);
    WaitForSuccessSharedPtr(put_callback);

    // Call GetPrevious on "key2" and verify that we get "key1" and its value
    auto get_callback = make_shared<StrictMock<MockGetCallback>>();
    nonblocking_connection_->GetPrevious("key2", get_callback);
    EXPECT_CALL(*get_callback, Success_("key1",
        KineticRecordEq("value1", "v1", "tag1", Message_Algorithm_SHA1)))
        .WillOnce(Assign(&done_, true));

    RunSelectLoop();
}

TEST_F(IntegrationTest, GetPrevious_KeyExistsAndDoesNotHavePredecessor) {
    // Write "key2"
    auto put_callback = make_shared<StrictMock<MockPutCallback>>();
    auto record = make_shared<KineticRecord>("value", "version", "tag", Message_Algorithm_SHA1);
    nonblocking_connection_->Put(make_shared<string>("key2"), make_shared<string>(""),
        WriteMode::REQUIRE_SAME_VERSION, record, put_callback);
    WaitForSuccessSharedPtr(put_callback);

    // Call GetPrevious on "key2" and verify that we get "not found"
    auto get_callback = make_shared<StrictMock<MockGetCallback>>();
    nonblocking_connection_->GetPrevious("key2", get_callback);
    EXPECT_CALL(*get_callback, Failure(KineticStatusEq(StatusCode::REMOTE_NOT_FOUND,
        "Key not found"))) .WillOnce(Assign(&done_, true));

    RunSelectLoop();
}

TEST_F(IntegrationTest, GetPrevious_KeyDoesNotExistAndHasPredecessor) {
    // Write "key1"
    auto put_callback = make_shared<StrictMock<MockPutCallback>>();
    auto record = make_shared<KineticRecord>("value", "version", "tag", Message_Algorithm_SHA1);
    nonblocking_connection_->Put(make_shared<string>("key1"), make_shared<string>(""),
        WriteMode::REQUIRE_SAME_VERSION, record, put_callback);
    WaitForSuccessSharedPtr(put_callback);

    // Call GetPrevious on "key2" and verify that we get "key1" and its value
    auto get_callback = make_shared<StrictMock<MockGetCallback>>();
    nonblocking_connection_->GetPrevious("key2", get_callback);
    EXPECT_CALL(*get_callback, Success_("key1",
            KineticRecordEq("value", "version", "tag", Message_Algorithm_SHA1)))
        .WillOnce(Assign(&done_, true));

    RunSelectLoop();
}

TEST_F(IntegrationTest, GetPrevious_KeyDoesNotExistAndDoesNotHavePredecessor) {
    // Call GetPrevious on "key2" and verify that we get "not found"
    auto callback = make_shared<StrictMock<MockGetCallback>>();
    nonblocking_connection_->GetPrevious("key2", callback);
    EXPECT_CALL(*callback, Failure(KineticStatusEq(StatusCode::REMOTE_NOT_FOUND, "Key not found")))
        .WillOnce(Assign(&done_, true));

    RunSelectLoop();
}

TEST_F(IntegrationTest, GetVersionNonexistent) {
    auto get_version_callback = make_shared<StrictMock<MockGetVersionCallback>>();
    nonblocking_connection_->GetVersion("key", get_version_callback);
    EXPECT_CALL(*get_version_callback,
        Failure(KineticStatusEq(StatusCode::REMOTE_NOT_FOUND, "Key not found")))
        .WillOnce(Assign(&done_, true));
    RunSelectLoop();
}

TEST_F(IntegrationTest, GetVersion) {
    auto put_callback = make_shared<StrictMock<MockPutCallback>>();
    auto get_version_callback = make_shared<StrictMock<MockGetVersionCallback>>();
    auto record = make_shared<KineticRecord>("value", "version", "tag", Message_Algorithm_SHA1);
    nonblocking_connection_->Put(make_shared<string>("key"), make_shared<string>(""),
        WriteMode::REQUIRE_SAME_VERSION, record, put_callback);
    WaitForSuccessSharedPtr(put_callback);

    nonblocking_connection_->GetVersion("key", get_version_callback);
    EXPECT_CALL(*get_version_callback, Success("version")).WillOnce(Assign(&done_, true));
    RunSelectLoop();
}

TEST_F(IntegrationTest, GetKeyRange) {
    auto put_callback = make_shared<StrictMock<MockPutCallback>>();
    // TODO(marshall) fails on java simulator with empty value
    auto record = make_shared<KineticRecord>("v", "", "", Message_Algorithm_SHA1);
    nonblocking_connection_->Put(make_shared<string>("k0"), make_shared<string>(""),
        WriteMode::REQUIRE_SAME_VERSION, record, put_callback);
    WaitForSuccessSharedPtr(put_callback);
    nonblocking_connection_->Put(make_shared<string>("k1"), make_shared<string>(""),
        WriteMode::REQUIRE_SAME_VERSION, record, put_callback);
    WaitForSuccessSharedPtr(put_callback);
    nonblocking_connection_->Put(make_shared<string>("k2"), make_shared<string>(""),
        WriteMode::REQUIRE_SAME_VERSION, record, put_callback);
    WaitForSuccessSharedPtr(put_callback);
    nonblocking_connection_->Put(make_shared<string>("k3"), make_shared<string>(""),
        WriteMode::REQUIRE_SAME_VERSION, record, put_callback);
    WaitForSuccessSharedPtr(put_callback);

    auto getkeyrange_callback = make_shared<StrictMock<MockGetKeyRangeCallback>>();
    vector<string> expected_keys;
    expected_keys.push_back("k1");
    expected_keys.push_back("k2");

    EXPECT_CALL(*getkeyrange_callback, Success_(VectorStringPtrEq(expected_keys)))
        .WillOnce(Assign(&done_, true));
    nonblocking_connection_->GetKeyRange("k1", true, "k3", true, false, 2, getkeyrange_callback);
    RunSelectLoop();
}

}  // namespace kinetic
