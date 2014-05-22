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

using com::seagate::kinetic::client::proto::Message_Algorithm_SHA1;
using com::seagate::kinetic::client::proto::Message_MessageType_DELETE;
using com::seagate::kinetic::client::proto::Message_MessageType_GET;
using com::seagate::kinetic::client::proto::Message_MessageType_GETNEXT;
using com::seagate::kinetic::client::proto::Message_MessageType_GETPREVIOUS;
using com::seagate::kinetic::client::proto::Message_MessageType_GETKEYRANGE;
using com::seagate::kinetic::client::proto::Message_MessageType_GETVERSION;
using com::seagate::kinetic::client::proto::Message_MessageType_NOOP;
using com::seagate::kinetic::client::proto::Message_MessageType_PUT;
using com::seagate::kinetic::client::proto::Message_MessageType_SETUP;
using com::seagate::kinetic::client::proto::Message_MessageType_GETLOG;
using com::seagate::kinetic::client::proto::Message_MessageType_SECURITY;
using com::seagate::kinetic::client::proto::Message_MessageType_PEER2PEERPUSH;
using com::seagate::kinetic::client::proto::Message_Status_StatusCode_SUCCESS;
using com::seagate::kinetic::client::proto::Message_Status_StatusCode_INTERNAL_ERROR;
using com::seagate::kinetic::client::proto::Message_GetLog_Type_UTILIZATIONS;
using com::seagate::kinetic::client::proto::Message_GetLog_Type_TEMPERATURES;
using com::seagate::kinetic::client::proto::Message_GetLog_Type_CAPACITIES;
using com::seagate::kinetic::client::proto::Message_GetLog_Type_CONFIGURATION;
using com::seagate::kinetic::client::proto::Message_GetLog_Type_STATISTICS;
using com::seagate::kinetic::client::proto::Message_GetLog_Type_MESSAGES;
using com::seagate::kinetic::client::proto::Message_GetLog_Type_LIMITS;
using com::seagate::kinetic::client::proto::Message_Security;
using com::seagate::kinetic::client::proto::Message_Security_ACL;
using com::seagate::kinetic::client::proto::Message_Security_ACL_Scope;
using com::seagate::kinetic::client::proto::Message_Security_ACL_Permission_READ;
using com::seagate::kinetic::client::proto::Message_Security_ACL_Permission_WRITE;
using com::seagate::kinetic::client::proto::Message_Security_ACL_Permission_DELETE;
using com::seagate::kinetic::client::proto::Message_Security_ACL_Permission_RANGE;
using com::seagate::kinetic::client::proto::Message_Security_ACL_Permission_SETUP;
using com::seagate::kinetic::client::proto::Message_Security_ACL_Permission_P2POP;
using com::seagate::kinetic::client::proto::Message_Security_ACL_Permission_GETLOG;
using com::seagate::kinetic::client::proto::Message_Security_ACL_Permission_SECURITY;
using com::seagate::kinetic::client::proto::Message_Security_ACL_HMACAlgorithm_HmacSHA1;
using com::seagate::kinetic::client::proto::Message_Status;
using com::seagate::kinetic::client::proto::Message_P2POperation;
using com::seagate::kinetic::client::proto::Message_Synchronization;
using com::seagate::kinetic::client::proto::Message_Synchronization_FLUSH;
using com::seagate::kinetic::client::proto::Message_Synchronization_WRITEBACK;
using com::seagate::kinetic::client::proto::Message_Synchronization_WRITETHROUGH;

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
    Message message;
    EXPECT_CALL(*packet_service_, Submit_(_, StringSharedPtrEq(""), _))
            .WillOnce(DoAll(SaveArg<0>(&message), Return(0)));
    shared_ptr<SimpleCallbackInterface> callback;
    connection_.NoOp(callback);

    ASSERT_EQ(Message_MessageType_NOOP, message.command().header().messagetype());
}

TEST_F(NonblockingKineticConnectionTest, GetWorks) {
    Message message;
    EXPECT_CALL(*packet_service_, Submit_(_, StringSharedPtrEq(""), _))
            .WillOnce(DoAll(SaveArg<0>(&message), Return(0)));
    shared_ptr<GetCallbackInterface> callback;
    connection_.Get("key", callback);

    ASSERT_EQ(Message_MessageType_GET, message.command().header().messagetype());
    ASSERT_EQ(0, message.command().header().clusterversion());
    ASSERT_EQ("key", message.command().body().keyvalue().key());
}

TEST_F(NonblockingKineticConnectionTest, GetWithClusterVersionWorks) {
    connection_.SetClientClusterVersion(123);
    Message message;
    EXPECT_CALL(*packet_service_, Submit_(_, StringSharedPtrEq(""), _))
            .WillOnce(DoAll(SaveArg<0>(&message), Return(0)));
    shared_ptr<GetCallbackInterface> callback;
    connection_.Get("key", callback);

    ASSERT_EQ(Message_MessageType_GET, message.command().header().messagetype());
    ASSERT_EQ(123, message.command().header().clusterversion());
    ASSERT_EQ("key", message.command().body().keyvalue().key());
}

TEST_F(NonblockingKineticConnectionTest, GetNextWorks) {
    Message message;
    EXPECT_CALL(*packet_service_, Submit_(_, StringSharedPtrEq(""), _)).WillOnce(
            DoAll(SaveArg<0>(&message), Return(0)));
    shared_ptr<GetCallbackInterface> callback;
    connection_.GetNext("key", callback);

    ASSERT_EQ(Message_MessageType_GETNEXT, message.command().header().messagetype());
    ASSERT_EQ("key", message.command().body().keyvalue().key());
}

TEST_F(NonblockingKineticConnectionTest, GetPreviousWorks) {
    Message message;
    EXPECT_CALL(*packet_service_, Submit_(_, StringSharedPtrEq(""), _)).WillOnce(
            DoAll(SaveArg<0>(&message), Return(0)));
    connection_.GetPrevious("key", NULL);

    ASSERT_EQ(Message_MessageType_GETPREVIOUS, message.command().header().messagetype());
    ASSERT_EQ("key", message.command().body().keyvalue().key());
}

TEST_F(NonblockingKineticConnectionTest, GetVersionWorks) {
    Message message;
    EXPECT_CALL(*packet_service_, Submit_(_, StringSharedPtrEq(""), _)).WillOnce(
            DoAll(SaveArg<0>(&message), Return(0)));
    connection_.GetVersion("key", NULL);

    ASSERT_EQ(Message_MessageType_GETVERSION, message.command().header().messagetype());
    ASSERT_EQ("key", message.command().body().keyvalue().key());
}

TEST_F(NonblockingKineticConnectionTest, DeleteWorks) {
    Message message;
    EXPECT_CALL(*packet_service_, Submit_(_, StringSharedPtrEq(""), _)).WillOnce(
            DoAll(SaveArg<0>(&message), Return(0)));
    connection_.Delete("key", "version", WriteMode::IGNORE_VERSION, NULL);

    ASSERT_EQ(Message_MessageType_DELETE, message.command().header().messagetype());
    ASSERT_EQ("key", message.command().body().keyvalue().key());
    ASSERT_EQ("version", message.command().body().keyvalue().dbversion());
    ASSERT_TRUE(message.command().body().keyvalue().force());
    ASSERT_EQ(Message_Synchronization_WRITEBACK, message.command().body().keyvalue().synchronization());
}

TEST_F(NonblockingKineticConnectionTest, PutWorks) {
    Message message;
    EXPECT_CALL(*packet_service_, Submit_(_, StringSharedPtrEq("value"), _)).WillOnce(
            DoAll(SaveArg<0>(&message), Return(0)));
    auto record = make_shared<KineticRecord>("value", "new_version", "tag", Message_Algorithm_SHA1);
    shared_ptr<PutCallbackInterface> callback;
    connection_.Put(make_shared<string>("key"), make_shared<string>("old_version"), WriteMode::IGNORE_VERSION,
            record, callback);

    ASSERT_EQ(Message_MessageType_PUT, message.command().header().messagetype());
    ASSERT_EQ("key", message.command().body().keyvalue().key());
    ASSERT_EQ("old_version", message.command().body().keyvalue().dbversion());
    ASSERT_TRUE(message.command().body().keyvalue().force());
    ASSERT_EQ("new_version", message.command().body().keyvalue().newversion());
    ASSERT_EQ("tag", message.command().body().keyvalue().tag());
    ASSERT_EQ(Message_Algorithm_SHA1, message.command().body().keyvalue().algorithm());
    ASSERT_EQ(Message_Synchronization_WRITEBACK, message.command().body().keyvalue().synchronization());
}

TEST_F(NonblockingKineticConnectionTest, GetKeyRangeWorks) {
    Message message;
    EXPECT_CALL(*packet_service_, Submit_(_, StringSharedPtrEq(""), _)).WillOnce(
            DoAll(SaveArg<0>(&message), Return(0)));
    connection_.GetKeyRange("first", true, "last", false, true, 1234, NULL);

    ASSERT_EQ(Message_MessageType_GETKEYRANGE, message.command().header().messagetype());
    ASSERT_EQ("first", message.command().body().range().startkey());
    ASSERT_TRUE(message.command().body().range().startkeyinclusive());
    ASSERT_EQ("last", message.command().body().range().endkey());
    ASSERT_FALSE(message.command().body().range().endkeyinclusive());
    ASSERT_TRUE(message.command().body().range().reverse());
    ASSERT_EQ(1234, message.command().body().range().maxreturned());
}

TEST_F(NonblockingKineticConnectionTest, GetKeyRangeParsesResult) {
    auto mock_getkeyrange_callback = make_shared<MockGetKeyRangeCallback>();
    GetKeyRangeHandler handler(mock_getkeyrange_callback);

    Message response;
    response.mutable_command()->mutable_body()->mutable_range()->add_key("foo");
    response.mutable_command()->mutable_body()->mutable_range()->add_key("bar");
    response.mutable_command()->mutable_body()->mutable_range()->add_key("baz");

    vector<string> expected_keys;
    expected_keys.push_back("foo");
    expected_keys.push_back("bar");
    expected_keys.push_back("baz");

    EXPECT_CALL(*mock_getkeyrange_callback, Success_(VectorStringPtrEq(expected_keys)));

    unique_ptr<const string> empty_str(new string(""));
    handler.Handle(response, move(empty_str));
}

TEST_F(NonblockingKineticConnectionTest, InstantSecureEraseWorksWithNullPin) {
    Message message;
    EXPECT_CALL(*packet_service_, Submit_(_, StringSharedPtrEq(""), _)).WillOnce(
            DoAll(SaveArg<0>(&message), Return(0)));
    shared_ptr<string> null_ptr(nullptr);
    shared_ptr<MockSimpleCallback> null_callback(nullptr);
    connection_.InstantSecureErase(null_ptr, null_callback);

    ASSERT_EQ(Message_MessageType_SETUP, message.command().header().messagetype());
    ASSERT_TRUE(message.command().body().setup().instantsecureerase());
    ASSERT_EQ("", message.command().body().setup().pin());
}

TEST_F(NonblockingKineticConnectionTest, InstantSecureEraseWorksWithNonNullPin) {
    const std::string pin("1234");
    Message message;
    EXPECT_CALL(*packet_service_, Submit_(_, StringSharedPtrEq(""), _)).WillOnce(
            DoAll(SaveArg<0>(&message), Return(0)));
    shared_ptr<MockSimpleCallback> null_callback(nullptr);
    connection_.InstantSecureErase(pin, nullptr);

    ASSERT_EQ(Message_MessageType_SETUP, message.command().header().messagetype());
    ASSERT_TRUE(message.command().body().setup().instantsecureerase());
    ASSERT_EQ(pin, message.command().body().setup().pin());
}

TEST_F(NonblockingKineticConnectionTest, SetClusterVersionSendsCorrectVersion) {
    Message message;
    EXPECT_CALL(*packet_service_, Submit_(_, StringSharedPtrEq(""), _)).WillOnce(
            DoAll(SaveArg<0>(&message), Return(0)));
    connection_.SetClusterVersion(1234, NULL);

    EXPECT_EQ(Message_MessageType_SETUP, message.command().header().messagetype());
    EXPECT_EQ(1234, message.command().body().setup().newclusterversion());
}

TEST_F(NonblockingKineticConnectionTest, SetClusterVersionReturnsTrueForSuccess) {
    auto mock_simple_callback = make_shared<MockSimpleCallback>();
    SimpleHandler handler(mock_simple_callback);

    Message response;
    response.mutable_command()->mutable_status()->set_code(Message_Status_StatusCode_SUCCESS);

    EXPECT_CALL(*mock_simple_callback, Success());

    unique_ptr<const string> empty_str(new string(""));
    handler.Handle(response, move(empty_str));
}

TEST_F(NonblockingKineticConnectionTest, GetLogBuildsCorrectMessage) {
    Message message;
    EXPECT_CALL(*packet_service_, Submit_(_, StringSharedPtrEq(""), _)).WillOnce(
            DoAll(SaveArg<0>(&message), Return(0)));
    connection_.GetLog(NULL);

    EXPECT_EQ(Message_MessageType_GETLOG, message.command().header().messagetype());
    EXPECT_EQ(7, message.command().body().getlog().type_size());
    EXPECT_EQ(Message_GetLog_Type_UTILIZATIONS, message.command().body().getlog().type(0));
    EXPECT_EQ(Message_GetLog_Type_TEMPERATURES, message.command().body().getlog().type(1));
    EXPECT_EQ(Message_GetLog_Type_CAPACITIES, message.command().body().getlog().type(2));
    EXPECT_EQ(Message_GetLog_Type_CONFIGURATION, message.command().body().getlog().type(3));
    EXPECT_EQ(Message_GetLog_Type_STATISTICS, message.command().body().getlog().type(4));
    EXPECT_EQ(Message_GetLog_Type_MESSAGES, message.command().body().getlog().type(5));
    EXPECT_EQ(Message_GetLog_Type_LIMITS, message.command().body().getlog().type(6));
}

TEST_F(NonblockingKineticConnectionTest, GetLogParsesMessageCorrectly) {
    Message response;
    response.mutable_command()->mutable_body()->mutable_getlog()
            ->mutable_capacity()->set_nominalcapacityinbytes(123);
    response.mutable_command()->mutable_body()->mutable_getlog()
            ->mutable_capacity()->set_portionfull(0.5);

    response.mutable_command()->mutable_body()->mutable_getlog()
            ->mutable_configuration()->set_vendor("vendor");
    response.mutable_command()->mutable_body()->mutable_getlog()
            ->mutable_configuration()->set_model("model");
    response.mutable_command()->mutable_body()->mutable_getlog()
            ->mutable_configuration()->set_serialnumber("sn");
    response.mutable_command()->mutable_body()->mutable_getlog()
            ->mutable_configuration()->set_version("version");
    response.mutable_command()->mutable_body()->mutable_getlog()
            ->mutable_configuration()->set_port(9999);
    response.mutable_command()->mutable_body()->mutable_getlog()
            ->mutable_configuration()->set_tlsport(8888);
    response.mutable_command()->mutable_body()->mutable_getlog()
            ->mutable_configuration()->set_compilationdate("compilation date");
    response.mutable_command()->mutable_body()->mutable_getlog()
            ->mutable_configuration()->set_sourcehash("hash");

    auto operation_statistic = response.mutable_command()
            ->mutable_body()->mutable_getlog()->add_statistics();
    operation_statistic->set_messagetype(Message_MessageType_GETLOG);
    operation_statistic->set_count(111);
    operation_statistic->set_bytes(1024);

    auto temperature = response.mutable_command()->mutable_body()
            ->mutable_getlog()->add_temperature();
    temperature->set_name("coldthing");
    temperature->set_current(8);
    temperature->set_minimum(9);
    temperature->set_maximum(10);
    temperature->set_target(11);

    auto utilization = response.mutable_command()->mutable_body()
            ->mutable_getlog()->add_utilization();
    utilization->set_name("foo");
    utilization->set_value(0.5);

    response.mutable_command()->mutable_body()->mutable_getlog()->set_messages("Some messages");

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
    Message message;
    EXPECT_CALL(*packet_service_, Submit_(_, StringSharedPtrEq("the new firmware"), _)).WillOnce(
            DoAll(SaveArg<0>(&message), Return(0)));
    connection_.UpdateFirmware(make_shared<string>("the new firmware"), NULL);

    EXPECT_EQ(Message_MessageType_SETUP, message.command().header().messagetype());
    EXPECT_EQ(true, message.command().body().setup().firmwaredownload());
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

    Message message;
    EXPECT_CALL(*packet_service_, Submit_(_, StringSharedPtrEq(""), _)).WillOnce(
            DoAll(SaveArg<0>(&message), Return(0)));
    shared_ptr<MockSimpleCallback> null_callback(nullptr);
    connection_.SetACLs(acls, null_callback);

    EXPECT_EQ(Message_MessageType_SECURITY, message.command().header().messagetype());
    Message_Security const& security = message.command().body().security();

    ASSERT_EQ(2, security.acl_size());

    Message_Security_ACL const& proto_acl1 = security.acl(0);
    EXPECT_EQ(1234, proto_acl1.identity());
    EXPECT_EQ("the first key", proto_acl1.key());
    EXPECT_EQ(Message_Security_ACL_HMACAlgorithm_HmacSHA1, proto_acl1.hmacalgorithm());
    ASSERT_EQ(2, proto_acl1.scope_size());
    Message_Security_ACL_Scope const& proto_acl1_scope1 = proto_acl1.scope(0);
    EXPECT_EQ(0, proto_acl1_scope1.offset());
    EXPECT_EQ("prefix", proto_acl1_scope1.value());
    ASSERT_EQ(2, proto_acl1_scope1.permission_size());
    EXPECT_EQ(Message_Security_ACL_Permission_READ, proto_acl1_scope1.permission(0));
    EXPECT_EQ(Message_Security_ACL_Permission_WRITE, proto_acl1_scope1.permission(1));
    Message_Security_ACL_Scope const& proto_acl1_scope2 = proto_acl1.scope(1);
    EXPECT_EQ(100, proto_acl1_scope2.offset());
    EXPECT_EQ("thing", proto_acl1_scope2.value());
    ASSERT_EQ(8, proto_acl1_scope2.permission_size());
    EXPECT_EQ(Message_Security_ACL_Permission_READ, proto_acl1_scope2.permission(0));
    EXPECT_EQ(Message_Security_ACL_Permission_WRITE, proto_acl1_scope2.permission(1));
    EXPECT_EQ(Message_Security_ACL_Permission_DELETE, proto_acl1_scope2.permission(2));
    EXPECT_EQ(Message_Security_ACL_Permission_RANGE, proto_acl1_scope2.permission(3));
    EXPECT_EQ(Message_Security_ACL_Permission_SETUP, proto_acl1_scope2.permission(4));
    EXPECT_EQ(Message_Security_ACL_Permission_P2POP, proto_acl1_scope2.permission(5));
    EXPECT_EQ(Message_Security_ACL_Permission_GETLOG, proto_acl1_scope2.permission(6));
    EXPECT_EQ(Message_Security_ACL_Permission_SECURITY, proto_acl1_scope2.permission(7));

    Message_Security_ACL const& proto_acl2 = security.acl(1);
    EXPECT_EQ(5678, proto_acl2.identity());
    EXPECT_EQ("second key", proto_acl2.key());
    EXPECT_EQ(Message_Security_ACL_HMACAlgorithm_HmacSHA1, proto_acl2.hmacalgorithm());
    ASSERT_EQ(1, proto_acl2.scope_size());
    Message_Security_ACL_Scope const& proto_acl2_scope1 = proto_acl2.scope(0);
    EXPECT_EQ(0, proto_acl2_scope1.offset());
    EXPECT_EQ("", proto_acl2_scope1.value());
    EXPECT_EQ(0, proto_acl2_scope1.permission_size());
}


TEST_F(NonblockingKineticConnectionTest, SetPinBuildsCorrectMessageForNoCurrentPin) {
    Message message;
    EXPECT_CALL(*packet_service_, Submit_(_, StringSharedPtrEq(""), _)).WillOnce(
            DoAll(SaveArg<0>(&message), Return(0)));
    shared_ptr<MockSimpleCallback> null_callback(nullptr);
    shared_ptr<string> null_str(nullptr);
    connection_.SetPIN(make_shared<string>("newnewnew"), null_str, null_callback);

    EXPECT_EQ(Message_MessageType_SETUP, message.command().header().messagetype());
    EXPECT_EQ("newnewnew", message.command().body().setup().setpin());
    EXPECT_FALSE(message.command().body().setup().has_pin());
}

TEST_F(NonblockingKineticConnectionTest, SetPinBuildsCorrectMessageIfCurrentPin) {
    auto oldpin = "oldoldold";
    Message message;
    EXPECT_CALL(*packet_service_, Submit_(_, StringSharedPtrEq(""), _)).WillOnce(
            DoAll(SaveArg<0>(&message), Return(0)));
    shared_ptr<MockSimpleCallback> null_callback(nullptr);
    connection_.SetPIN("newnewnew", oldpin, null_callback);

    EXPECT_EQ(Message_MessageType_SETUP, message.command().header().messagetype());
    EXPECT_EQ("newnewnew", message.command().body().setup().setpin());
    EXPECT_EQ(oldpin, message.command().body().setup().pin());
}

TEST_F(NonblockingKineticConnectionTest, P2PPushBuildsCorrectMessage) {
    Message message;
    EXPECT_CALL(*packet_service_, Submit_(_, StringSharedPtrEq(""), _)).WillOnce(
            DoAll(SaveArg<0>(&message), Return(0)));

    P2PPushRequest request;
    request.host = "foo.tld";
    request.port = 1234;

    P2PPushOperation op1;
    op1.key = "key1";
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

    EXPECT_EQ(Message_MessageType_PEER2PEERPUSH, message.command().header().messagetype());

    EXPECT_EQ("foo.tld", message.command().body().p2poperation().peer().hostname());
    EXPECT_EQ(1234, message.command().body().p2poperation().peer().port());
    EXPECT_FALSE(message.command().body().p2poperation().peer().tls());

    ASSERT_EQ(3, message.command().body().p2poperation().operation_size());
    EXPECT_EQ("key1", message.command().body().p2poperation().operation(0).key());
    EXPECT_FALSE(message.command().body().p2poperation().operation(0).has_newkey());

    EXPECT_EQ("key2", message.command().body().p2poperation().operation(1).key());
    EXPECT_FALSE(message.command().body().p2poperation().operation(1).has_newkey());
    EXPECT_TRUE(message.command().body().p2poperation().operation(1).force());

    EXPECT_EQ("key3", message.command().body().p2poperation().operation(2).key());
    EXPECT_EQ("otherkey", message.command().body().p2poperation().operation(2).newkey());
}

TEST_F(NonblockingKineticConnectionTest, P2PHandlerInterpretsStatusCorrectly) {
    Message response;
    Message_P2POperation* mutable_op =
            response.mutable_command()->mutable_body()->mutable_p2poperation();
    Message_Status* status = mutable_op->add_operation()->mutable_status();
    status->set_code(com::seagate::kinetic::client::proto::Message_Status_StatusCode_SUCCESS);

    status = mutable_op->add_operation()->mutable_status();
    status->set_code(com::seagate::kinetic::client::proto::Message_Status_StatusCode_INTERNAL_ERROR);
    status->set_statusmessage("Bugger");

    status = mutable_op->add_operation()->mutable_status();
    status->set_code(com::seagate::kinetic::client::proto::Message_Status_StatusCode_SUCCESS);

    auto callback = make_shared<MockP2PPushCallback>();
    P2PPushHandler handler(callback);

    vector<KineticStatus> statuses;

    EXPECT_CALL(*callback, Success_(_)).WillOnce(SaveArg<0>(&statuses));

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
