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
#include "kinetic/batch_operation.h"
#include "matchers.h"

namespace kinetic {

using ::testing::Assign;
using ::testing::StrictMock;
using com::seagate::kinetic::client::proto::Command_Algorithm_SHA1;

using std::make_shared;
using std::shared_ptr;


TEST_F(IntegrationTest, BatchOperationCommit) {
    auto bop = BatchOperation(nonblocking_connection_);
    auto record = make_shared<KineticRecord>(make_shared<string>("value"),
       make_shared<string>("v"), make_shared<string>("t"), Command_Algorithm_SHA1);

    unique_ptr<KineticRecord> readrecord;
    bop.Put("key1", "", WriteMode::REQUIRE_SAME_VERSION, record);
    bop.Put("key2", "", WriteMode::REQUIRE_SAME_VERSION, record);

    ASSERT_TRUE(bop.Commit().ok());
    ASSERT_TRUE(blocking_connection_->Get("key1", readrecord).ok());
    ASSERT_TRUE(blocking_connection_->Get("key2", readrecord).ok());
}

TEST_F(IntegrationTest, BatchOperationCommitAsync) {
    auto bop = BatchOperation(nonblocking_connection_);
    auto record = make_shared<KineticRecord>(make_shared<string>("value"),
       make_shared<string>("v"), make_shared<string>("t"), Command_Algorithm_SHA1);
    unique_ptr<KineticRecord> readrecord;

    bop.Put("key1", "", WriteMode::REQUIRE_SAME_VERSION, record);
    bop.Put("key2", "", WriteMode::REQUIRE_SAME_VERSION, record);

    auto simple_callback = make_shared<StrictMock<MockSimpleCallback>>();
    ASSERT_TRUE(bop.Commit(simple_callback).ok());
    WaitForSuccessSharedPtr(simple_callback);

    ASSERT_TRUE(blocking_connection_->Get("key1", readrecord).ok());
    ASSERT_TRUE(blocking_connection_->Get("key2", readrecord).ok());
}

TEST_F(IntegrationTest, BatchOperationCommitFailure) {
    auto bop = BatchOperation(nonblocking_connection_);
    auto record = make_shared<KineticRecord>(make_shared<string>("value"),
      make_shared<string>("v"), make_shared<string>("t"), Command_Algorithm_SHA1);
    unique_ptr<KineticRecord> readrecord;

    KineticStatus status = blocking_connection_->Put(make_shared<string>("key1"),
            make_shared<string>(""), WriteMode::REQUIRE_SAME_VERSION, record);
    ASSERT_TRUE(status.ok());

    bop.Put("key1", "x", WriteMode::REQUIRE_SAME_VERSION, record);
    bop.Put("key2", "",  WriteMode::IGNORE_VERSION, record);

    ASSERT_FALSE(bop.Commit().ok());
    ASSERT_TRUE(blocking_connection_->Get("key1", readrecord).ok());
    ASSERT_FALSE(blocking_connection_->Get("key2", readrecord).ok());
}

TEST_F(IntegrationTest, BatchOperationAbort) {
    auto bop = BatchOperation(nonblocking_connection_);
    auto record = make_shared<KineticRecord>(make_shared<string>("value"),
        make_shared<string>("v"), make_shared<string>("t"), Command_Algorithm_SHA1);
    unique_ptr<KineticRecord> readrecord;

    bop.Put("key1", "", WriteMode::IGNORE_VERSION, record);
    bop.Put("key2", "", WriteMode::IGNORE_VERSION, record);

    ASSERT_TRUE(bop.Abort().ok());
    ASSERT_FALSE(blocking_connection_->Get("key1", readrecord).ok());
    ASSERT_FALSE(blocking_connection_->Get("key2", readrecord).ok());
}

TEST_F(IntegrationTest, BatchOperationMulipleOperationSameKey) {
    auto bop = BatchOperation(nonblocking_connection_);
    auto record = make_shared<KineticRecord>(make_shared<string>("value"),
       make_shared<string>("v"), make_shared<string>("t"), Command_Algorithm_SHA1);
    unique_ptr<KineticRecord> readrecord;

    bop.Put("key1", "", WriteMode::REQUIRE_SAME_VERSION, record);
    bop.Put("key1", "v", WriteMode::REQUIRE_SAME_VERSION, record);

    ASSERT_FALSE(bop.Commit().ok());
    ASSERT_FALSE(blocking_connection_->Get("key1", readrecord).ok());
}

TEST_F(IntegrationTest, BatchOperationMultipleBatches) {
    auto bop1 = BatchOperation(nonblocking_connection_);
    auto bop2 = BatchOperation(nonblocking_connection_);
    auto bop3 = BatchOperation(nonblocking_connection_);

    auto record = make_shared<KineticRecord>(make_shared<string>("value"),
           make_shared<string>("v"), make_shared<string>("t"), Command_Algorithm_SHA1);
    unique_ptr<KineticRecord> readrecord;

    bop1.Put("key1", "",  WriteMode::REQUIRE_SAME_VERSION, record);
    bop2.Put("key2", "v", WriteMode::REQUIRE_SAME_VERSION, record);
    bop2.Put("key1", "v", WriteMode::REQUIRE_SAME_VERSION, record);
    bop3.Put("key2", "", WriteMode::REQUIRE_SAME_VERSION, record);

    ASSERT_TRUE(bop1.Commit().ok());
    ASSERT_TRUE(blocking_connection_->Get("key1", readrecord).ok());
    ASSERT_TRUE(bop3.Commit().ok());
    ASSERT_TRUE(blocking_connection_->Get("key2", readrecord).ok());
    ASSERT_TRUE(bop2.Commit().ok());
}

TEST_F(IntegrationTest, BatchOperationKeyDeleteFailure) {
    auto bop = BatchOperation(nonblocking_connection_);
    auto record = make_shared<KineticRecord>(make_shared<string>("value"),
       make_shared<string>("v"), make_shared<string>("t"), Command_Algorithm_SHA1);
    unique_ptr<KineticRecord> readrecord;

    KineticStatus status = blocking_connection_->Put(make_shared<string>("key1"),
                make_shared<string>(""), WriteMode::REQUIRE_SAME_VERSION, record);
    ASSERT_TRUE(status.ok());

    bop.Delete("key1", "x", WriteMode::REQUIRE_SAME_VERSION);
    bop.Put("key2", "", WriteMode::REQUIRE_SAME_VERSION, record);

    ASSERT_FALSE(bop.Commit().ok());
    ASSERT_TRUE(blocking_connection_->Get("key1", readrecord).ok());
    ASSERT_FALSE(blocking_connection_->Get("key2", readrecord).ok());
}

TEST_F(IntegrationTest, BatchOperationInvalidBatchId) {
    auto record = make_shared<KineticRecord>(make_shared<string>("value"),
          make_shared<string>("v1"), make_shared<string>("t"), Command_Algorithm_SHA1);
    nonblocking_connection_->BatchPutKey(1000, "key", "version", WriteMode::IGNORE_VERSION, record);
    ASSERT_TRUE(true);
}

} // namespace kinetic
