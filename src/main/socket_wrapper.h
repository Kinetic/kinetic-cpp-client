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

#ifndef KINETIC_CPP_CLIENT_SOCKET_WRAPPER_H_
#define KINETIC_CPP_CLIENT_SOCKET_WRAPPER_H_

#include "socket_wrapper_interface.h"
#include "kinetic/connection_options.h"


namespace kinetic {

class SocketWrapper : public SocketWrapperInterface {
  public:
    explicit SocketWrapper(const std::string &host, int port, bool use_ssl, bool nonblocking = false);
    bool Connect();
    int  fd();
    SSL *getSSL();
    ~SocketWrapper();

  private:
    bool ConnectSSL();

    SSL_CTX * ctx_;
    SSL * ssl_;
    std::string host_;
    int port_;
    bool nonblocking_;
    int fd_;
};

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_SOCKET_WRAPPER_H_
