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
#include "protobufutil/message_stream.h"

#include "kinetic/kinetic.h"
#include "gtest/gtest.h"

namespace kinetic {

using palominolabs::protobufutil::MessageStreamFactory;
using std::unique_ptr;

TEST(NonexistentServerTest, NonexistentServer) {
    kinetic::KineticConnectionFactory kinetic_connection_factory = NewKineticConnectionFactory();
    ConnectionOptions options;
    options.host = "localhost";
    options.port = 4025;
    options.use_ssl = false;
    options.user_id = 1;
    options.hmac_key = "asdfasdf";

    unique_ptr<NonblockingKineticConnection> connection;
    ASSERT_FALSE(kinetic_connection_factory.NewNonblockingConnection(options, connection).ok());
    ASSERT_FALSE(connection);
}

}  // namespace kinetic
