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

#include <memory>

#include "integration_test.h"

namespace kinetic {

using std::shared_ptr;
using std::make_shared;
using std::vector;

using com::seagate::kinetic::client::proto::Command_Algorithm_CRC32;
using com::seagate::kinetic::client::proto::Command_Algorithm_SHA1;

TEST_F(IntegrationTest, BlockingSmoketest) {
    ASSERT_TRUE(blocking_connection_->NoOp().ok());

    auto record1 = make_shared<KineticRecord>(make_shared<string>("value1"),
        make_shared<string>("v1"), make_shared<string>("t1"), Command_Algorithm_CRC32);
    KineticStatus kineticStatus = blocking_connection_->Put("key1", "", WriteMode::IGNORE_VERSION, *record1);
    ASSERT_TRUE(kineticStatus.ok());

    auto record2 = make_shared<KineticRecord>(make_shared<string>("value2"),
        make_shared<string>("v2"), make_shared<string>("t2"), Command_Algorithm_SHA1);
    ASSERT_TRUE(blocking_connection_->Put(make_shared<string>("key2"), make_shared<string>(""),
       WriteMode::IGNORE_VERSION, record2).ok());

    auto record3 = make_shared<KineticRecord>(make_shared<string>("value3"),
        make_shared<string>("v3"), make_shared<string>("t3"), Command_Algorithm_CRC32);
    ASSERT_TRUE(blocking_connection_->Put(make_shared<string>("key3"), make_shared<string>(""),
       WriteMode::IGNORE_VERSION, record3).ok());

    KeyRangeIterator it = blocking_connection_->IterateKeyRange("key1", true, "key3", false, 1);
    ASSERT_EQ("key1", *it);
    ++it;
    ASSERT_EQ("key2", *it);
    ++it;
    ASSERT_EQ(KeyRangeEnd(), it);


    std::unique_ptr<KineticRecord> result_record;
    ASSERT_TRUE(blocking_connection_->Get("key2", result_record).ok());
    EXPECT_EQ("value2", *(result_record->value()));
    EXPECT_EQ("v2", *(result_record->version()));
    EXPECT_EQ("t2", *(result_record->tag()));
    EXPECT_EQ(Command_Algorithm_SHA1, result_record->algorithm());

    unique_ptr<string> result_key(nullptr);
    ASSERT_TRUE(blocking_connection_->GetNext("key1", result_key, result_record).ok());
    EXPECT_EQ("key2", *result_key);
    EXPECT_EQ("value2", *(result_record->value()));
    EXPECT_EQ("v2", *(result_record->version()));
    EXPECT_EQ("t2", *(result_record->tag()));
    EXPECT_EQ(Command_Algorithm_SHA1, result_record->algorithm());

    ASSERT_TRUE(blocking_connection_->GetPrevious("key3", result_key, result_record).ok());
    EXPECT_EQ("key2", *result_key);
    EXPECT_EQ("value2", *(result_record->value()));
    EXPECT_EQ("v2", *(result_record->version()));
    EXPECT_EQ("t2", *(result_record->tag()));
    EXPECT_EQ(Command_Algorithm_SHA1, result_record->algorithm());

    unique_ptr<string> result_version(nullptr);
    ASSERT_TRUE(blocking_connection_->GetVersion("key3", result_version).ok());
    EXPECT_EQ("v3", *result_version);

    unique_ptr<vector<string>> actual_keys(new vector<string>);
    vector<string> expected_keys = {"key2", "key3"};
    KineticStatus status =
        blocking_connection_->GetKeyRange("key1", false, "key3", true, false, 2, actual_keys);
    ASSERT_TRUE(status.ok());
    EXPECT_EQ(expected_keys, *actual_keys);

    ASSERT_TRUE(blocking_connection_->Delete("key1", "", WriteMode::IGNORE_VERSION).ok());

    ASSERT_EQ(blocking_connection_->Get("key1", result_record).statusCode(), StatusCode::REMOTE_NOT_FOUND);

    ASSERT_TRUE(blocking_ssl_connection_->InstantErase("").ok());

    ASSERT_EQ(blocking_connection_->Get("key3", result_record).statusCode(), StatusCode::REMOTE_NOT_FOUND);

    blocking_connection_->SetClientClusterVersion(33);

    ASSERT_EQ(blocking_connection_->Get("key1", result_record).statusCode(),
        StatusCode::REMOTE_CLUSTER_VERSION_MISMATCH);
}

} // namespace kinetic
