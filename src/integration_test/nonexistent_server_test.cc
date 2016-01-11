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

#include "gtest/gtest.h"
#include "kinetic/message_stream.h"

#include "kinetic/kinetic.h"
#include "gtest/gtest.h"

namespace kinetic {

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
