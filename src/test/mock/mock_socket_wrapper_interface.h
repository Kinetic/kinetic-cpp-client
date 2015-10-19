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

#ifndef KINETIC_CPP_CLIENT_MOCK_SOCKET_WRAPPER_INTERFACE_H_
#define KINETIC_CPP_CLIENT_MOCK_SOCKET_WRAPPER_INTERFACE_H_

#include "kinetic/message_stream.h"

#include "socket_wrapper_interface.h"

namespace kinetic {

class MockSocketWrapperInterface : public SocketWrapperInterface {
    public:
    MockSocketWrapperInterface() {}
    MOCK_METHOD0(Connect, bool());
    MOCK_METHOD0(fd, int());
    MOCK_METHOD0(getSSL, SSL*());
};

}  // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_MOCK_SOCKET_WRAPPER_INTERFACE_H_
