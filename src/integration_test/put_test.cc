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

namespace kinetic {

using ::testing::Assign;
using ::testing::StrictMock;
using ::testing::SaveArg;
using ::testing::_;
using com::seagate::kinetic::client::proto::Message_Algorithm_SHA1;
using com::seagate::kinetic::client::proto::Message_Algorithm_CRC64;

using std::make_shared;
using std::string;

TEST_F(IntegrationTest, Write) {
    auto put_callback = make_shared<StrictMock<MockPutCallback>>();
    auto get_callback = make_shared<StrictMock<MockGetCallback>>();
    auto record = make_shared<KineticRecord>("value", "version", "tag", Message_Algorithm_SHA1);
    nonblocking_connection_->Put("key", "", WriteMode::REQUIRE_SAME_VERSION, record, put_callback);
    WaitForSuccessSharedPtr(put_callback);
    nonblocking_connection_->Get("key", get_callback);
    EXPECT_CALL(*get_callback, Success_("key",
            KineticRecordEq("value", "version", "tag", Message_Algorithm_SHA1)))
        .WillOnce(Assign(&done_, true));
    RunSelectLoop();
}

TEST_F(IntegrationTest, Write_NoTag) {
    auto put_callback = make_shared<StrictMock<MockPutCallback>>();
    auto get_callback = make_shared<StrictMock<MockGetCallback>>();
    auto record = make_shared<KineticRecord>("value", "version", "", Message_Algorithm_SHA1);
    nonblocking_connection_->Put(make_shared<string>("key"), make_shared<string>(""),
        WriteMode::REQUIRE_SAME_VERSION, record, put_callback);
    WaitForSuccessSharedPtr(put_callback);
    nonblocking_connection_->Get("key", get_callback);
    EXPECT_CALL(*get_callback, Success_("key",
            KineticRecordEq("value", "version", "", Message_Algorithm_SHA1)))
            .WillOnce(Assign(&done_, true));
    RunSelectLoop();
}

TEST_F(IntegrationTest, WriteWithWrongVersion) {
    // Given that "key" is not present, attempt a PUT to "key" expecting a
    // current version of "v2"
    auto put_callback = make_shared<StrictMock<MockPutCallback>>();
    auto record = make_shared<KineticRecord>("value", "version", "tag", Message_Algorithm_SHA1);
    nonblocking_connection_->Put(make_shared<string>("key"), make_shared<string>("v2"),
        WriteMode::REQUIRE_SAME_VERSION, record, put_callback);
    EXPECT_CALL(*put_callback,
        Failure(KineticStatusEq(StatusCode::REMOTE_VERSION_MISMATCH, "Version mismatch")))
        .WillOnce(Assign(&done_, true));
    RunSelectLoop();
}

TEST_F(IntegrationTest, Update) {
    // Write a new key-value pair
    auto write_callback = make_shared<StrictMock<MockPutCallback>>();
    auto initial_record = make_shared<KineticRecord>("initial value", "v1", "initial tag",
        Message_Algorithm_SHA1);
    nonblocking_connection_->Put(make_shared<string>("key"), make_shared<string>(""),
        WriteMode::REQUIRE_SAME_VERSION, initial_record,
        write_callback);
    WaitForSuccessSharedPtr(write_callback);

    // Update that key-value pair
    auto update_callback = make_shared<StrictMock<MockPutCallback>>();
    auto new_record = make_shared<KineticRecord>("new value", "v2", "new tag",
        Message_Algorithm_CRC64);
    nonblocking_connection_->Put(make_shared<string>("key"), make_shared<string>("v1"),
        WriteMode::REQUIRE_SAME_VERSION, new_record, update_callback);
    WaitForSuccessSharedPtr(update_callback);

    // Verify that the update is now visible
    auto get_callback = make_shared<StrictMock<MockGetCallback>>();
    nonblocking_connection_->Get("key", get_callback);
    EXPECT_CALL(*get_callback, Success_("key",
            KineticRecordEq("new value", "v2", "new tag", Message_Algorithm_CRC64)))
            .WillOnce(Assign(&done_, true));
    RunSelectLoop();
}

TEST_F(IntegrationTest, UpdateWithWrongVersion) {
    // Write a new key-value pair
    auto write_callback = make_shared<StrictMock<MockPutCallback>>();
    auto initial_record = make_shared<KineticRecord>("initial value", "v1", "initial tag",
        Message_Algorithm_SHA1);
    nonblocking_connection_->Put(make_shared<string>("key"), make_shared<string>(""),
        WriteMode::REQUIRE_SAME_VERSION, initial_record, write_callback);
    WaitForSuccessSharedPtr(write_callback);

    // Attempt to update it with the wrong version
    auto update_callback = make_shared<StrictMock<MockPutCallback>>();
    auto new_record = make_shared<KineticRecord>("new value", "v2", "new tag",
        Message_Algorithm_SHA1);
    nonblocking_connection_->Put(make_shared<string>("key"), make_shared<string>("wrong version"),
        WriteMode::REQUIRE_SAME_VERSION, new_record, update_callback);
    EXPECT_CALL(*update_callback,
        Failure(KineticStatusEq(StatusCode::REMOTE_VERSION_MISMATCH, "Version mismatch")))
        .WillOnce(Assign(&done_, true));

    RunSelectLoop();
}

TEST_F(IntegrationTest, InsertWithNullVersion) {
    // Write a new key-value pair
    auto write_callback = make_shared<StrictMock<MockPutCallback>>();
    shared_ptr<string> version(nullptr);

    auto initial_record = make_shared<KineticRecord>(make_shared<string>("value"),
        version, make_shared<string>("tag"), Message_Algorithm_SHA1);
    nonblocking_connection_->Put(make_shared<string>("key"), make_shared<string>(""),
        WriteMode::REQUIRE_SAME_VERSION, initial_record, write_callback);
    WaitForSuccessSharedPtr(write_callback);

    auto get_callback = make_shared<StrictMock<MockGetCallback>>();
    nonblocking_connection_->Get("key", get_callback);
    // TODO(marshall) should the version really be empty string here? Should we express null?
    EXPECT_CALL(*get_callback, Success_("key",
        KineticRecordEq("value", "", "tag", Message_Algorithm_SHA1)))
        .WillOnce(Assign(&done_, true));
    RunSelectLoop();
}

TEST_F(IntegrationTest, ForcedWrite) {
    // "key" is not currently present, but we can do a PUT expecting version
    // "v1" provided that we set the force flag
    auto put_callback = make_shared<StrictMock<MockPutCallback>>();
    auto record = make_shared<KineticRecord>("value", "v2", "tag", Message_Algorithm_SHA1);
    nonblocking_connection_->Put(make_shared<string>("key"), make_shared<string>("v1"),
        WriteMode::IGNORE_VERSION, record, put_callback);
    EXPECT_CALL(*put_callback, Success()).WillOnce(Assign(&done_, true));
    RunSelectLoop();
}

TEST_F(IntegrationTest, ForcedUpdate) {
    // Write a new key-value pair
    auto write_callback = make_shared<StrictMock<MockPutCallback>>();
    auto initial_record = make_shared<KineticRecord>("initial value", "v1", "initial tag",
        Message_Algorithm_SHA1);
    nonblocking_connection_->Put(make_shared<string>("key"), make_shared<string>(""),
        WriteMode::REQUIRE_SAME_VERSION, initial_record, write_callback);
    WaitForSuccessSharedPtr(write_callback);

    // We can update it with an incorrect or empty version, provided that we
    // set the force flag
    auto update_callback = make_shared<StrictMock<MockPutCallback>>();
    auto new_record = make_shared<KineticRecord>("new value", "", "new tag",
        Message_Algorithm_SHA1);
    nonblocking_connection_->Put(make_shared<string>("key"), make_shared<string>("wrong version"),
        WriteMode::IGNORE_VERSION, new_record, update_callback);
    EXPECT_CALL(*update_callback, Success()).WillOnce(Assign(&done_, true));

    RunSelectLoop();
}

}  // namespace kinetic
