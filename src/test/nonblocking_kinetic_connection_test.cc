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

#include "kinetic/kinetic.h"
#include "matchers.h"

#include "nonblocking_packet_service.h"
#include "mock_callbacks.h"
#include "mock_nonblocking_packet_service.h"

namespace kinetic {

using com::seagate::kinetic::client::proto::Command_Algorithm_SHA1;
using com::seagate::kinetic::client::proto::Command_MessageType_DELETE;
using com::seagate::kinetic::client::proto::Command_MessageType_GET;
using com::seagate::kinetic::client::proto::Command_MessageType_GETNEXT;
using com::seagate::kinetic::client::proto::Command_MessageType_GETPREVIOUS;
using com::seagate::kinetic::client::proto::Command_MessageType_GETKEYRANGE;
using com::seagate::kinetic::client::proto::Command_MessageType_GETVERSION;
using com::seagate::kinetic::client::proto::Command_MessageType_NOOP;
using com::seagate::kinetic::client::proto::Command_MessageType_PUT;
using com::seagate::kinetic::client::proto::Command_MessageType_SETUP;
using com::seagate::kinetic::client::proto::Command_MessageType_GETLOG;
using com::seagate::kinetic::client::proto::Command_MessageType_SECURITY;
using com::seagate::kinetic::client::proto::Command_MessageType_PEER2PEERPUSH;
using com::seagate::kinetic::client::proto::Command_MessageType_PINOP;
using com::seagate::kinetic::client::proto::Command_Status_StatusCode_SUCCESS;
using com::seagate::kinetic::client::proto::Command_Status_StatusCode_INTERNAL_ERROR;
using com::seagate::kinetic::client::proto::Command_GetLog_Type_UTILIZATIONS;
using com::seagate::kinetic::client::proto::Command_GetLog_Type_TEMPERATURES;
using com::seagate::kinetic::client::proto::Command_GetLog_Type_CAPACITIES;
using com::seagate::kinetic::client::proto::Command_GetLog_Type_CONFIGURATION;
using com::seagate::kinetic::client::proto::Command_GetLog_Type_STATISTICS;
using com::seagate::kinetic::client::proto::Command_GetLog_Type_MESSAGES;
using com::seagate::kinetic::client::proto::Command_GetLog_Type_LIMITS;
using com::seagate::kinetic::client::proto::Command_Security;
using com::seagate::kinetic::client::proto::Command_Security_ACL;
using com::seagate::kinetic::client::proto::Command_Security_ACL_Scope;
using com::seagate::kinetic::client::proto::Command_Security_ACL_Permission_READ;
using com::seagate::kinetic::client::proto::Command_Security_ACL_Permission_WRITE;
using com::seagate::kinetic::client::proto::Command_Security_ACL_Permission_DELETE;
using com::seagate::kinetic::client::proto::Command_Security_ACL_Permission_RANGE;
using com::seagate::kinetic::client::proto::Command_Security_ACL_Permission_SETUP;
using com::seagate::kinetic::client::proto::Command_Security_ACL_Permission_P2POP;
using com::seagate::kinetic::client::proto::Command_Security_ACL_Permission_GETLOG;
using com::seagate::kinetic::client::proto::Command_Security_ACL_Permission_SECURITY;
using com::seagate::kinetic::client::proto::Command_Security_ACL_HMACAlgorithm_HmacSHA1;
using com::seagate::kinetic::client::proto::Command_Status;
using com::seagate::kinetic::client::proto::Command_P2POperation;
using com::seagate::kinetic::client::proto::Command_Synchronization;
using com::seagate::kinetic::client::proto::Command_Synchronization_FLUSH;
using com::seagate::kinetic::client::proto::Command_Synchronization_WRITEBACK;
using com::seagate::kinetic::client::proto::Command_Synchronization_WRITETHROUGH;
using com::seagate::kinetic::client::proto::Command_PinOperation_PinOpType_ERASE_PINOP;

using ::testing::_;
using ::testing::DoAll;
using ::testing::SaveArg;
using ::testing::StrictMock;
using ::testing::NiceMock;
using ::testing::Return;

using std::make_shared;
using std::unique_ptr;
using std::shared_ptr;
using std::string;
using std::list;
using std::move;

class NonblockingKineticConnectionTest : public ::testing::Test {
    protected:
    NonblockingKineticConnectionTest()
            : packet_service_(new MockNonblockingPacketService), connection_(packet_service_) {
    }

    MockNonblockingPacketService* packet_service_;
    NonblockingKineticConnection connection_;
};

TEST_F(NonblockingKineticConnectionTest, NoOpWorks) {
    Command message;
    EXPECT_CALL(*packet_service_, Submit_(_, _, StringSharedPtrEq(""), _))
            .WillOnce(DoAll(SaveArg<1>(&message), Return(0)));
    shared_ptr<SimpleCallbackInterface> callback;
    connection_.NoOp(callback);

    ASSERT_EQ(Command_MessageType_NOOP, message.header().messagetype());
}

TEST_F(NonblockingKineticConnectionTest, GetWorks) {
    Command message;
    EXPECT_CALL(*packet_service_, Submit_(_, _, StringSharedPtrEq(""), _))
            .WillOnce(DoAll(SaveArg<1>(&message), Return(0)));
    shared_ptr<GetCallbackInterface> callback;
    connection_.Get("key", callback);

    ASSERT_EQ(Command_MessageType_GET, message.header().messagetype());
    ASSERT_EQ(0, message.header().clusterversion());
    ASSERT_EQ("key", message.body().keyvalue().key());
}

TEST_F(NonblockingKineticConnectionTest, GetWithClusterVersionWorks) {
    connection_.SetClientClusterVersion(123);
    Command message;
    EXPECT_CALL(*packet_service_, Submit_(_, _, StringSharedPtrEq(""), _))
            .WillOnce(DoAll(SaveArg<1>(&message), Return(0)));
    shared_ptr<GetCallbackInterface> callback;
    connection_.Get("key", callback);

    ASSERT_EQ(Command_MessageType_GET, message.header().messagetype());
    ASSERT_EQ(123, message.header().clusterversion());
    ASSERT_EQ("key", message.body().keyvalue().key());
}

TEST_F(NonblockingKineticConnectionTest, GetNextWorks) {
    Command message;
    EXPECT_CALL(*packet_service_, Submit_(_, _, StringSharedPtrEq(""), _)).WillOnce(
            DoAll(SaveArg<1>(&message), Return(0)));
    shared_ptr<GetCallbackInterface> callback;
    connection_.GetNext("key", callback);

    ASSERT_EQ(Command_MessageType_GETNEXT, message.header().messagetype());
    ASSERT_EQ("key", message.body().keyvalue().key());
}

TEST_F(NonblockingKineticConnectionTest, GetPreviousWorks) {
    Command message;
    EXPECT_CALL(*packet_service_, Submit_(_, _, StringSharedPtrEq(""), _)).WillOnce(
            DoAll(SaveArg<1>(&message), Return(0)));
    connection_.GetPrevious("key", NULL);

    ASSERT_EQ(Command_MessageType_GETPREVIOUS, message.header().messagetype());
    ASSERT_EQ("key", message.body().keyvalue().key());
}

TEST_F(NonblockingKineticConnectionTest, GetVersionWorks) {
    Command message;
    EXPECT_CALL(*packet_service_, Submit_(_, _, StringSharedPtrEq(""), _)).WillOnce(
            DoAll(SaveArg<1>(&message), Return(0)));
    connection_.GetVersion("key", NULL);

    ASSERT_EQ(Command_MessageType_GETVERSION, message.header().messagetype());
    ASSERT_EQ("key", message.body().keyvalue().key());
}

TEST_F(NonblockingKineticConnectionTest, DeleteWorks) {
    Command message;
    EXPECT_CALL(*packet_service_, Submit_(_, _, StringSharedPtrEq(""), _)).WillOnce(
            DoAll(SaveArg<1>(&message), Return(0)));
    connection_.Delete("key", "version", WriteMode::IGNORE_VERSION, NULL);

    ASSERT_EQ(Command_MessageType_DELETE, message.header().messagetype());
    ASSERT_EQ("key", message.body().keyvalue().key());
    ASSERT_EQ("version", message.body().keyvalue().dbversion());
    ASSERT_TRUE(message.body().keyvalue().force());
    ASSERT_EQ(Command_Synchronization_WRITEBACK, message.body().keyvalue().synchronization());
}

TEST_F(NonblockingKineticConnectionTest, PutWorks) {
    Command message;
    EXPECT_CALL(*packet_service_, Submit_(_, _, StringSharedPtrEq("value"), _)).WillOnce(
            DoAll(SaveArg<1>(&message), Return(0)));
    auto record = make_shared<KineticRecord>("value", "new_version", "tag", Command_Algorithm_SHA1);
    shared_ptr<PutCallbackInterface> callback;
    connection_.Put(make_shared<string>("key"), make_shared<string>("old_version"), WriteMode::IGNORE_VERSION,
            record, callback);

    ASSERT_EQ(Command_MessageType_PUT, message.header().messagetype());
    ASSERT_EQ("key", message.body().keyvalue().key());
    ASSERT_EQ("old_version", message.body().keyvalue().dbversion());
    ASSERT_TRUE(message.body().keyvalue().force());
    ASSERT_EQ("new_version", message.body().keyvalue().newversion());
    ASSERT_EQ("tag", message.body().keyvalue().tag());
    ASSERT_EQ(Command_Algorithm_SHA1, message.body().keyvalue().algorithm());
    ASSERT_EQ(Command_Synchronization_WRITEBACK, message.body().keyvalue().synchronization());
}

TEST_F(NonblockingKineticConnectionTest, GetKeyRangeWorks) {
    Command message;
    EXPECT_CALL(*packet_service_, Submit_(_, _, StringSharedPtrEq(""), _)).WillOnce(
            DoAll(SaveArg<1>(&message), Return(0)));
    connection_.GetKeyRange("first", true, "last", false, true, 1234, NULL);

    ASSERT_EQ(Command_MessageType_GETKEYRANGE, message.header().messagetype());
    ASSERT_EQ("first", message.body().range().startkey());
    ASSERT_TRUE(message.body().range().startkeyinclusive());
    ASSERT_EQ("last", message.body().range().endkey());
    ASSERT_FALSE(message.body().range().endkeyinclusive());
    ASSERT_TRUE(message.body().range().reverse());
    ASSERT_EQ(1234, message.body().range().maxreturned());
}

TEST_F(NonblockingKineticConnectionTest, GetKeyRangeParsesResult) {
    auto mock_getkeyrange_callback = make_shared<MockGetKeyRangeCallback>();
    GetKeyRangeHandler handler(mock_getkeyrange_callback);

    Command response;
    response.mutable_body()->mutable_range()->add_keys("foo");
    response.mutable_body()->mutable_range()->add_keys("bar");
    response.mutable_body()->mutable_range()->add_keys("baz");

    vector<string> expected_keys;
    expected_keys.push_back("foo");
    expected_keys.push_back("bar");
    expected_keys.push_back("baz");

    EXPECT_CALL(*mock_getkeyrange_callback, Success_(VectorStringPtrEq(expected_keys)));

    unique_ptr<const string> empty_str(new string(""));
    handler.Handle(response, move(empty_str));
}

TEST_F(NonblockingKineticConnectionTest, InstantEraseWorksWithNullPin) {
    Command message;
    EXPECT_CALL(*packet_service_, Submit_(_, _, StringSharedPtrEq(""), _)).WillOnce(
            DoAll(SaveArg<1>(&message), Return(0)));
    shared_ptr<string> null_ptr(nullptr);
    shared_ptr<MockSimpleCallback> null_callback(nullptr);
    connection_.InstantErase(null_ptr, null_callback);

    ASSERT_EQ(Command_MessageType_PINOP, message.header().messagetype());
    ASSERT_EQ(Command_PinOperation_PinOpType_ERASE_PINOP, message.body().pinop().pinoptype());
}

TEST_F(NonblockingKineticConnectionTest, InstantEraseWorksWithNonNullPin) {
    const std::string pin("1234");
    Command message;
    EXPECT_CALL(*packet_service_, Submit_(_, _, StringSharedPtrEq(""), _)).WillOnce(
            DoAll(SaveArg<1>(&message), Return(0)));
    shared_ptr<MockSimpleCallback> null_callback(nullptr);
    connection_.InstantErase(pin, nullptr);

    ASSERT_EQ(Command_MessageType_PINOP, message.header().messagetype());
    ASSERT_EQ(Command_PinOperation_PinOpType_ERASE_PINOP, message.body().pinop().pinoptype());
}

TEST_F(NonblockingKineticConnectionTest, SetClusterVersionSendsCorrectVersion) {
    Command message;
    EXPECT_CALL(*packet_service_, Submit_(_, _, StringSharedPtrEq(""), _)).WillOnce(
            DoAll(SaveArg<1>(&message), Return(0)));
    connection_.SetClusterVersion(1234, NULL);

    EXPECT_EQ(Command_MessageType_SETUP, message.header().messagetype());
    EXPECT_EQ(1234, message.body().setup().newclusterversion());
}

TEST_F(NonblockingKineticConnectionTest, SetClusterVersionReturnsTrueForSuccess) {
    auto mock_simple_callback = make_shared<MockSimpleCallback>();
    SimpleHandler handler(mock_simple_callback);

    Command response;
    response.mutable_status()->set_code(Command_Status_StatusCode_SUCCESS);

    EXPECT_CALL(*mock_simple_callback, Success());

    unique_ptr<const string> empty_str(new string(""));
    handler.Handle(response, move(empty_str));
}

TEST_F(NonblockingKineticConnectionTest, GetLogBuildsCorrectMessage) {
    Command message;
    EXPECT_CALL(*packet_service_, Submit_(_, _, StringSharedPtrEq(""), _)).WillOnce(
            DoAll(SaveArg<1>(&message), Return(0)));
    connection_.GetLog(NULL);

    EXPECT_EQ(Command_MessageType_GETLOG, message.header().messagetype());
    EXPECT_EQ(7, message.body().getlog().types_size());
    EXPECT_EQ(Command_GetLog_Type_UTILIZATIONS, message.body().getlog().types(0));
    EXPECT_EQ(Command_GetLog_Type_TEMPERATURES, message.body().getlog().types(1));
    EXPECT_EQ(Command_GetLog_Type_CAPACITIES, message.body().getlog().types(2));
    EXPECT_EQ(Command_GetLog_Type_CONFIGURATION, message.body().getlog().types(3));
    EXPECT_EQ(Command_GetLog_Type_STATISTICS, message.body().getlog().types(4));
    EXPECT_EQ(Command_GetLog_Type_MESSAGES, message.body().getlog().types(5));
    EXPECT_EQ(Command_GetLog_Type_LIMITS, message.body().getlog().types(6));
}

TEST_F(NonblockingKineticConnectionTest, GetLogParsesMessageCorrectly) {
    Command response;
    response.mutable_body()->mutable_getlog()
            ->mutable_capacity()->set_nominalcapacityinbytes(123);
    response.mutable_body()->mutable_getlog()
            ->mutable_capacity()->set_portionfull(0.5);

    response.mutable_body()->mutable_getlog()
            ->mutable_configuration()->set_vendor("vendor");
    response.mutable_body()->mutable_getlog()
            ->mutable_configuration()->set_model("model");
    response.mutable_body()->mutable_getlog()
            ->mutable_configuration()->set_serialnumber("sn");
    response.mutable_body()->mutable_getlog()
            ->mutable_configuration()->set_version("version");
    response.mutable_body()->mutable_getlog()
            ->mutable_configuration()->set_port(9999);
    response.mutable_body()->mutable_getlog()
            ->mutable_configuration()->set_tlsport(8888);
    response.mutable_body()->mutable_getlog()
            ->mutable_configuration()->set_compilationdate("compilation date");
    response.mutable_body()->mutable_getlog()
            ->mutable_configuration()->set_sourcehash("hash");

    auto operation_statistic = response.mutable_body()->mutable_getlog()->add_statistics();
    operation_statistic->set_messagetype(Command_MessageType_GETLOG);
    operation_statistic->set_count(111);
    operation_statistic->set_bytes(1024);

    auto temperature = response.mutable_body()->mutable_getlog()->add_temperatures();
    temperature->set_name("coldthing");
    temperature->set_current(8);
    temperature->set_minimum(9);
    temperature->set_maximum(10);
    temperature->set_target(11);

    auto utilization = response.mutable_body()
            ->mutable_getlog()->add_utilizations();
    utilization->set_name("foo");
    utilization->set_value(0.5);

    response.mutable_body()->mutable_getlog()->set_messages("Some messages");

    auto mock_getlog_callback = make_shared<MockGetLogCallback>();
    GetLogHandler handler(mock_getlog_callback);

    DriveLog drive_log;
    EXPECT_CALL(*mock_getlog_callback, Success_(_)).WillOnce(SaveArg<0>(&drive_log));

    unique_ptr<const string> empty_str(new string(""));
    handler.Handle(response, move(empty_str));

    EXPECT_EQ((uint64_t) 123, drive_log.capacity.nominal_capacity_in_bytes);
    EXPECT_EQ((float) 0.5, drive_log.capacity.portion_full);

    EXPECT_EQ("vendor", drive_log.configuration.vendor);
    EXPECT_EQ("model", drive_log.configuration.model);
    EXPECT_EQ("sn", drive_log.configuration.serial_number);
    EXPECT_EQ("version", drive_log.configuration.version);
    EXPECT_EQ(9999, drive_log.configuration.port);
    EXPECT_EQ(8888, drive_log.configuration.tls_port);
    EXPECT_EQ("compilation date", drive_log.configuration.compilation_date);
    EXPECT_EQ("hash", drive_log.configuration.source_hash);

    ASSERT_EQ(1U, drive_log.temperatures.size());
    EXPECT_EQ("coldthing", drive_log.temperatures.front().name);
    EXPECT_EQ(8, drive_log.temperatures.front().current_degc);
    EXPECT_EQ(9, drive_log.temperatures.front().min_degc);
    EXPECT_EQ(10, drive_log.temperatures.front().max_degc);
    EXPECT_EQ(11, drive_log.temperatures.front().target_degc);

    ASSERT_EQ(1U, drive_log.utilizations.size());
    EXPECT_EQ("foo", drive_log.utilizations.front().name);
    EXPECT_EQ(0.5f, drive_log.utilizations.front().percent);

    ASSERT_EQ(1U, drive_log.operation_statistics.size());
    EXPECT_EQ("GETLOG", drive_log.operation_statistics.front().name);
    EXPECT_EQ((uint64_t) 111, drive_log.operation_statistics.front().count);
    EXPECT_EQ((uint64_t) 1024, drive_log.operation_statistics.front().bytes);

    ASSERT_EQ("Some messages", drive_log.messages);
}

TEST_F(NonblockingKineticConnectionTest, FirmwareUpdateSendsFirmwareContents) {
    Command message;
    EXPECT_CALL(*packet_service_, Submit_(_, _, StringSharedPtrEq("the new firmware"), _)).WillOnce(
            DoAll(SaveArg<1>(&message), Return(0)));
    connection_.UpdateFirmware(make_shared<string>("the new firmware"), NULL);

    EXPECT_EQ(Command_MessageType_SETUP, message.header().messagetype());
    EXPECT_EQ(true, message.body().setup().firmwaredownload());
}

TEST_F(NonblockingKineticConnectionTest, SetACLsBuildsCorrectMessage) {
    Scope scope1;
    scope1.offset = 0;
    scope1.value = "prefix";
    scope1.permissions = {READ, WRITE};
    Scope scope2;
    scope2.offset = 100;
    scope2.value = "thing";
    scope2.permissions = {READ, WRITE, DELETE, RANGE, SETUP, P2POP, GETLOG, SECURITY};
    std::list<Scope> acl1_scopes = {scope1, scope2};

    ACL acl1;
    acl1.identity = 1234;
    acl1.hmac_key = "the first key";
    acl1.scopes = acl1_scopes;

    Scope scope3;
    scope3.offset = 0;
    scope3.value = "";
    scope3.permissions = {};
    ACL acl2;
    acl2.identity = 5678;
    acl2.hmac_key = "second key";
    acl2.scopes = {scope3};

    auto acls = make_shared<list<ACL>>();
    acls->push_back(acl1);
    acls->push_back(acl2);

    Command message;
    EXPECT_CALL(*packet_service_, Submit_(_, _, StringSharedPtrEq(""), _)).WillOnce(
            DoAll(SaveArg<1>(&message), Return(0)));
    shared_ptr<MockSimpleCallback> null_callback(nullptr);
    connection_.SetACLs(acls, null_callback);

    EXPECT_EQ(Command_MessageType_SECURITY, message.header().messagetype());
    Command_Security const& security = message.body().security();

    ASSERT_EQ(2, security.acl_size());

    Command_Security_ACL const& proto_acl1 = security.acl(0);
    EXPECT_EQ(1234, proto_acl1.identity());
    EXPECT_EQ("the first key", proto_acl1.key());
    EXPECT_EQ(Command_Security_ACL_HMACAlgorithm_HmacSHA1, proto_acl1.hmacalgorithm());
    ASSERT_EQ(2, proto_acl1.scope_size());
    Command_Security_ACL_Scope const& proto_acl1_scope1 = proto_acl1.scope(0);
    EXPECT_EQ(0, proto_acl1_scope1.offset());
    EXPECT_EQ("prefix", proto_acl1_scope1.value());
    ASSERT_EQ(2, proto_acl1_scope1.permission_size());
    EXPECT_EQ(Command_Security_ACL_Permission_READ, proto_acl1_scope1.permission(0));
    EXPECT_EQ(Command_Security_ACL_Permission_WRITE, proto_acl1_scope1.permission(1));
    Command_Security_ACL_Scope const& proto_acl1_scope2 = proto_acl1.scope(1);
    EXPECT_EQ(100, proto_acl1_scope2.offset());
    EXPECT_EQ("thing", proto_acl1_scope2.value());
    ASSERT_EQ(8, proto_acl1_scope2.permission_size());
    EXPECT_EQ(Command_Security_ACL_Permission_READ, proto_acl1_scope2.permission(0));
    EXPECT_EQ(Command_Security_ACL_Permission_WRITE, proto_acl1_scope2.permission(1));
    EXPECT_EQ(Command_Security_ACL_Permission_DELETE, proto_acl1_scope2.permission(2));
    EXPECT_EQ(Command_Security_ACL_Permission_RANGE, proto_acl1_scope2.permission(3));
    EXPECT_EQ(Command_Security_ACL_Permission_SETUP, proto_acl1_scope2.permission(4));
    EXPECT_EQ(Command_Security_ACL_Permission_P2POP, proto_acl1_scope2.permission(5));
    EXPECT_EQ(Command_Security_ACL_Permission_GETLOG, proto_acl1_scope2.permission(6));
    EXPECT_EQ(Command_Security_ACL_Permission_SECURITY, proto_acl1_scope2.permission(7));

    Command_Security_ACL const& proto_acl2 = security.acl(1);
    EXPECT_EQ(5678, proto_acl2.identity());
    EXPECT_EQ("second key", proto_acl2.key());
    EXPECT_EQ(Command_Security_ACL_HMACAlgorithm_HmacSHA1, proto_acl2.hmacalgorithm());
    ASSERT_EQ(1, proto_acl2.scope_size());
    Command_Security_ACL_Scope const& proto_acl2_scope1 = proto_acl2.scope(0);
    EXPECT_EQ(0, proto_acl2_scope1.offset());
    EXPECT_EQ("", proto_acl2_scope1.value());
    EXPECT_EQ(0, proto_acl2_scope1.permission_size());
}


TEST_F(NonblockingKineticConnectionTest, SetErasePinBuildsCorrectMessageForNoCurrentPin) {
    Command message;
    EXPECT_CALL(*packet_service_, Submit_(_, _, StringSharedPtrEq(""), _)).WillOnce(
            DoAll(SaveArg<1>(&message), Return(0)));
    shared_ptr<MockSimpleCallback> null_callback(nullptr);
    shared_ptr<string> null_str(nullptr);
    connection_.SetErasePIN(make_shared<string>("newnewnew"), null_str, null_callback);

    EXPECT_EQ(Command_MessageType_SECURITY, message.header().messagetype());
    EXPECT_EQ("newnewnew", message.body().security().newerasepin());
    EXPECT_FALSE(message.body().security().has_olderasepin());
}

TEST_F(NonblockingKineticConnectionTest, SetErasePinBuildsCorrectMessageIfCurrentPin) {
    auto oldpin = "oldoldold";
    Command message;
    EXPECT_CALL(*packet_service_, Submit_(_, _, StringSharedPtrEq(""), _)).WillOnce(
            DoAll(SaveArg<1>(&message), Return(0)));
    shared_ptr<MockSimpleCallback> null_callback(nullptr);
    connection_.SetErasePIN("newnewnew", oldpin, null_callback);

    EXPECT_EQ(Command_MessageType_SECURITY, message.header().messagetype());
    EXPECT_EQ(message.body().security().newerasepin(),"newnewnew");
    EXPECT_EQ(message.body().security().olderasepin(),oldpin);
}


TEST_F(NonblockingKineticConnectionTest, P2PPushBuildsCorrectMessage) {
    Command message;
    EXPECT_CALL(*packet_service_, Submit_(_, _, StringSharedPtrEq(""), _)).WillOnce(
            DoAll(SaveArg<1>(&message), Return(0)));


    auto double_nested_request = make_shared<P2PPushRequest>();
    double_nested_request->host = "baz.tld";
    double_nested_request->port = 1236;

    P2PPushOperation double_nested_op;
    double_nested_op.key = "double_nested_key1";
    double_nested_request->operations.push_back(double_nested_op);

    auto nested_request = make_shared<P2PPushRequest>();
    nested_request->host = "bar.tld";
    nested_request->port = 1235;

    P2PPushOperation nested_op;
    nested_op.key = "nested_key1";
    nested_op.request = double_nested_request;
    nested_request->operations.push_back(nested_op);

    P2PPushRequest request;
    request.host = "foo.tld";
    request.port = 1234;

    P2PPushOperation op1;
    op1.key = "key1";
    op1.request = nested_request;
    request.operations.push_back(op1);

    P2PPushOperation op2;
    op2.key = "key2";
    op2.force = true;
    request.operations.push_back(op2);

    P2PPushOperation op3;
    op3.key = "key3";
    op3.newKey = "otherkey";
    request.operations.push_back(op3);

    connection_.P2PPush(request, NULL);

    EXPECT_EQ(Command_MessageType_PEER2PEERPUSH, message.header().messagetype());

    EXPECT_EQ("foo.tld", message.body().p2poperation().peer().hostname());
    EXPECT_EQ(1234, message.body().p2poperation().peer().port());
    EXPECT_FALSE(message.body().p2poperation().peer().tls());

    auto operation0 = message.body().p2poperation().operation(0);
    ASSERT_EQ(3, message.body().p2poperation().operation_size());
    EXPECT_EQ("key1", operation0.key());
    EXPECT_FALSE(operation0.has_newkey());

    EXPECT_EQ("bar.tld", operation0.p2pop().peer().hostname());
    EXPECT_EQ(1235, operation0.p2pop().peer().port());
    ASSERT_EQ(1, operation0.p2pop().operation_size());

    auto operation0_nested = operation0.p2pop().operation(0);
    EXPECT_EQ("nested_key1", operation0_nested.key());

    EXPECT_EQ("baz.tld", operation0_nested.p2pop().peer().hostname());
    EXPECT_EQ(1236, operation0_nested.p2pop().peer().port());
    ASSERT_EQ(1, operation0_nested.p2pop().operation_size());

    auto operation0_double_nested = operation0_nested.p2pop().operation(0);
    EXPECT_EQ("double_nested_key1", operation0_double_nested.key());

    EXPECT_EQ("key2", message.body().p2poperation().operation(1).key());
    EXPECT_FALSE(message.body().p2poperation().operation(1).has_newkey());
    EXPECT_TRUE(message.body().p2poperation().operation(1).force());

    EXPECT_EQ("key3", message.body().p2poperation().operation(2).key());
    EXPECT_EQ("otherkey", message.body().p2poperation().operation(2).newkey());
}

TEST_F(NonblockingKineticConnectionTest, P2PHandlerInterpretsStatusCorrectly) {
    Command response;
    Command_P2POperation* mutable_op =
            response.mutable_body()->mutable_p2poperation();
    Command_Status* status = mutable_op->add_operation()->mutable_status();
    status->set_code(com::seagate::kinetic::client::proto::Command_Status_StatusCode_SUCCESS);

    status = mutable_op->add_operation()->mutable_status();
    status->set_code(com::seagate::kinetic::client::proto::Command_Status_StatusCode_INTERNAL_ERROR);
    status->set_statusmessage("Bugger");

    status = mutable_op->add_operation()->mutable_status();
    status->set_code(com::seagate::kinetic::client::proto::Command_Status_StatusCode_SUCCESS);

    auto callback = make_shared<MockP2PPushCallback>();
    P2PPushHandler handler(callback);

    vector<KineticStatus> statuses;

    EXPECT_CALL(*callback, Success_(_, _)).WillOnce(SaveArg<0>(&statuses));

    unique_ptr<const string> empty_str(new string(""));
    handler.Handle(response, move(empty_str));

    ASSERT_EQ(3U, statuses.size());
    EXPECT_TRUE(statuses[0].ok());
    EXPECT_EQ(StatusCode::REMOTE_INTERNAL_ERROR, statuses[1].statusCode());
    EXPECT_EQ("Bugger", statuses[1].message());
    EXPECT_TRUE(statuses[2].ok());
}

TEST_F(NonblockingKineticConnectionTest, RemoveHandlerCallsService) {
    EXPECT_CALL(*packet_service_, Remove(42)).WillOnce(Return(true));
    ASSERT_TRUE(connection_.RemoveHandler(42));
}

}  // namespace kinetic
