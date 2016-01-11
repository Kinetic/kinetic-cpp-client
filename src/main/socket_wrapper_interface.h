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

#ifndef KINETIC_CPP_CLIENT_SOCKET_WRAPPER_INTERFACE_H_
#define KINETIC_CPP_CLIENT_SOCKET_WRAPPER_INTERFACE_H_

#include <openssl/ssl.h>

namespace kinetic {

/// Simple wrapper around a socket FD that closes the FD
/// in the destructor
class SocketWrapperInterface {
  public:
    /// Actually open the connection to the socket. The details
    /// of the host/port/protocol to connect on depend on the
    /// implementation. Returns true if the connection succeeds
    /// and the socket is ready for reading/writing
    virtual bool Connect() = 0;

    /// Returns the FD
    virtual int fd() = 0;

    /// Returns nullptr if SSL hasn't been initialized.
    virtual SSL* getSSL() = 0;

    /// The destructor should close the FD if it was opened
    /// by connect
    virtual ~SocketWrapperInterface() {}
};

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_SOCKET_WRAPPER_INTERFACE_H_
