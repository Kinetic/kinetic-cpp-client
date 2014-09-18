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

#include "gtest/gtest.h"

#include "kinetic_client.pb.h"
#include "kinetic/hmac_provider.h"

namespace kinetic {

using com::seagate::kinetic::client::proto::Message;
using com::seagate::kinetic::client::proto::Command;
using com::seagate::kinetic::client::proto::Command_Status_StatusCode_SUCCESS;
using com::seagate::kinetic::client::proto::Command_Status_StatusCode_INTERNAL_ERROR;
using com::seagate::kinetic::client::proto::Message_AuthType_HMACAUTH;

TEST(HmacProviderTest, ComputeHmacHandlesSimpleMessage) {
    HmacProvider hmac_provider;
    unsigned char expected_hmac_bytes[] = { 0x40, 0x5F, 0x94, 0x9F, 0xC3, 0x50,
        0xDC, 0x0B, 0x6A, 0x5A, 0x9D, 0x27, 0xA3, 0xCA, 0x44, 0x58, 0x9D, 0xB3,
        0x4A, 0xCD };
    std::string expected_hmac((char *)expected_hmac_bytes, sizeof(expected_hmac_bytes));

    Message response_message;
    Command command;
    command.mutable_status()->set_code(Command_Status_StatusCode_SUCCESS);
    response_message.set_commandbytes(command.SerializeAsString());

    std::string actual_hmac = hmac_provider.ComputeHmac(response_message, "asdfasdf");

    EXPECT_EQ(expected_hmac, actual_hmac);
}

TEST(HmacProviderTest, ComputeHmacOfEmptyMessage) {
    HmacProvider hmac_provider;
    unsigned char expected_hmac_bytes[] = {0xa7, 0x7a, 0x6a, 0xda, 0x5c, 0xe6,
        0x7c, 0xf7, 0xae, 0xe4, 0x8a, 0x79, 0xd4, 0x86, 0x6b, 0xb2, 0x71, 0x24,
        0x18, 0x15 };
    std::string expected_hmac((char *)expected_hmac_bytes, sizeof(expected_hmac_bytes));

    Message response_message;
    Command command;
    response_message.set_commandbytes(command.SerializeAsString());

    std::string actual_hmac =
        hmac_provider.ComputeHmac(response_message, "asdfasdf");

    EXPECT_EQ(expected_hmac, actual_hmac);
}


// Test that we cannot change any part of the message without changing the HMAC
TEST(HmacProviderTest, ComputeHmacIncludesAllFields) {
    HmacProvider hmac_provider;

    Message message;
    Command command;
    message.mutable_hmacauth()->set_identity(1);
    command.mutable_body()->mutable_keyvalue()->set_key("key");
    command.mutable_status()->set_code(Command_Status_StatusCode_SUCCESS);
    message.set_commandbytes(command.SerializeAsString());
    std::string hmac_key = "asdfasdf";
    std::string original_hmac = hmac_provider.ComputeHmac(message, hmac_key);
    std::string hmac;

    // Identical copy should have the same HMAC
    Message identical_copy(message);
    identical_copy.set_commandbytes(command.SerializeAsString());
    hmac = hmac_provider.ComputeHmac(identical_copy, hmac_key);
    EXPECT_EQ(original_hmac, hmac);

    // Change header
    Message message_with_different_header(message);
    command.mutable_header()->set_timeout(100);
    message_with_different_header.set_commandbytes(command.SerializeAsString());
    hmac = hmac_provider.ComputeHmac(message_with_different_header, hmac_key);
    EXPECT_NE(original_hmac, hmac);

    // Change body
    Message message_with_different_body(message);
    command.mutable_body()->mutable_keyvalue()->
        set_key("different key");
    message_with_different_body.set_commandbytes(command.SerializeAsString());
    hmac = hmac_provider.ComputeHmac(message_with_different_body, hmac_key);
    EXPECT_NE(original_hmac, hmac);

    // Change status
    Message message_with_different_status(message);
    command.mutable_status()->
        set_code(Command_Status_StatusCode_INTERNAL_ERROR);
    message_with_different_status.set_commandbytes(command.SerializeAsString());
    hmac = hmac_provider.ComputeHmac(message_with_different_status, hmac_key);
    EXPECT_NE(original_hmac, hmac);
}

TEST(HmacProviderTest, ValidateHmacReturnsFalseOnInvalidHmac) {
    HmacProvider hmac_provider;
    unsigned char hmac_bytes[] = { 0xff, 0x5F, 0x94, 0x9F, 0xC3, 0x50,
        0xDC, 0x0B, 0x6A, 0x5A, 0x9D, 0x27, 0xA3, 0xCA, 0x44, 0x58, 0x9D, 0xB3,
        0x4A, 0xCD };
    std::string hmac((char *) hmac_bytes, sizeof(hmac_bytes));

    Message message;
    Command command;
    command.mutable_status()->set_code(Command_Status_StatusCode_SUCCESS);
    message.set_commandbytes(command.SerializeAsString());

    message.set_authtype(Message_AuthType_HMACAUTH);
    message.mutable_hmacauth()->set_identity(1);
    message.mutable_hmacauth()->set_hmac(hmac);

    EXPECT_FALSE(hmac_provider.ValidateHmac(message, "asdfasdf"));
}


} // namespace kinetic
