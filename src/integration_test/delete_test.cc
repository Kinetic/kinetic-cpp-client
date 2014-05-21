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
#include "integration_test.h"
#include "matchers.h"

namespace kinetic {

using ::testing::Assign;
using ::testing::StrictMock;
using com::seagate::kinetic::client::proto::Message_Algorithm_SHA1;

using std::make_shared;
using std::shared_ptr;

TEST_F(IntegrationTest, DeleteNonexistent) {
    auto delete_callback = make_shared<StrictMock<MockSimpleCallback>>();
    nonblocking_connection_->Delete("nonexistent key", "", WriteMode::REQUIRE_SAME_VERSION, delete_callback);
    EXPECT_CALL(*delete_callback,
        Failure(KineticStatusEq(StatusCode::REMOTE_NOT_FOUND, "Key not found")))
        .WillOnce(Assign(&done_, true));
    RunSelectLoop();
}

TEST_F(IntegrationTest, DeleteWithWrongVersion) {
    // Write a new value with version "v1"
    auto put_callback = make_shared<StrictMock<MockPutCallback>>();
    auto record = make_shared<KineticRecord>(make_shared<string>("value"),
        make_shared<string>("v1"), make_shared<string>("tag"), Message_Algorithm_SHA1);
    nonblocking_connection_->Put(make_shared<string>("key"),  make_shared<string>(""),
        WriteMode::REQUIRE_SAME_VERSION, record, put_callback);
    WaitForSuccessSharedPtr(put_callback);

    // Attempt to delete version "v2"
    auto delete_callback = make_shared<StrictMock<MockSimpleCallback>>();
    nonblocking_connection_->Delete("key", "v2", WriteMode::REQUIRE_SAME_VERSION, delete_callback);
    EXPECT_CALL(*delete_callback,
        Failure(KineticStatusEq(StatusCode::REMOTE_VERSION_MISMATCH, "Version mismatch")))
        .WillOnce(Assign(&done_, true));

    RunSelectLoop();
}

TEST_F(IntegrationTest, DeleteWithMissingVersion) {
    // Write a new value with version "v1"
    auto put_callback = make_shared<StrictMock<MockPutCallback>>();
    auto record = make_shared<KineticRecord>(make_shared<string>("value"),
        make_shared<string>("v1"), make_shared<string>("tag"), Message_Algorithm_SHA1);
    nonblocking_connection_->Put(make_shared<string>("key"),  make_shared<string>(""),
        WriteMode::REQUIRE_SAME_VERSION, record, put_callback);
    WaitForSuccessSharedPtr(put_callback);

    // Attempt to delete version "" (empty)
    auto delete_callback = make_shared<StrictMock<MockSimpleCallback>>();
    nonblocking_connection_->Delete("key", "", WriteMode::REQUIRE_SAME_VERSION, delete_callback);
    EXPECT_CALL(*delete_callback,
        Failure(KineticStatusEq(StatusCode::REMOTE_VERSION_MISMATCH, "Version mismatch")))
        .WillOnce(Assign(&done_, true));

    RunSelectLoop();
}

TEST_F(IntegrationTest, SuccessfulDelete) {
    // Write a new value
    auto put_callback = make_shared<StrictMock<MockPutCallback>>();
    auto record = make_shared<KineticRecord>(make_shared<string>("value"),
        make_shared<string>("v1"), make_shared<string>("tag"), Message_Algorithm_SHA1);
    nonblocking_connection_->Put(make_shared<string>("key"), make_shared<string>(""),
        WriteMode::REQUIRE_SAME_VERSION, record, put_callback);
    WaitForSuccessSharedPtr(put_callback);

    // Delete it
    auto delete_callback = make_shared<StrictMock<MockSimpleCallback>>();
    nonblocking_connection_->Delete("key", "v1", WriteMode::REQUIRE_SAME_VERSION, delete_callback);
    EXPECT_CALL(*delete_callback, Success()).WillOnce(Assign(&done_, true));

    RunSelectLoop();
}

TEST_F(IntegrationTest, ForcedDelete) {
    // Write a new value with version "v1"
    auto put_callback = make_shared<StrictMock<MockPutCallback>>();
    auto record = make_shared<KineticRecord>(make_shared<string>("value"),
        make_shared<string>("v1"), make_shared<string>("tag"), Message_Algorithm_SHA1);
    nonblocking_connection_->Put(make_shared<string>("key"), make_shared<string>(""),
        WriteMode::REQUIRE_SAME_VERSION, record, put_callback);
    WaitForSuccessSharedPtr(put_callback);

    // Delete with wrong version should succeed if we set the force flag
    auto delete_callback = make_shared<StrictMock<MockSimpleCallback>>();
    nonblocking_connection_->Delete("key", "", WriteMode::IGNORE_VERSION, delete_callback);
    EXPECT_CALL(*delete_callback, Success()).WillOnce(Assign(&done_, true));

    RunSelectLoop();
}

TEST_F(IntegrationTest, ForcedDeleteNonexistent) {
    // Delete of a nonexistent key should succeed if we set the force flag
    auto delete_callback = make_shared<StrictMock<MockSimpleCallback>>();
    nonblocking_connection_->Delete("key", "", WriteMode::IGNORE_VERSION, delete_callback);
    EXPECT_CALL(*delete_callback, Success()).WillOnce(Assign(&done_, true));
    RunSelectLoop();
}

}  // namespace kinetic
