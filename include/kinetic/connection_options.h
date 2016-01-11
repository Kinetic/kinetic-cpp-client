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

#ifndef KINETIC_CPP_CLIENT_CONNECTION_OPTIONS_H_
#define KINETIC_CPP_CLIENT_CONNECTION_OPTIONS_H_

#include <string>

namespace kinetic {

/// Use this struct to pass all connection options to the KineticConnectionFactory.
struct ConnectionOptions {
  /// The host name or IP address of the kinetic server.
  std::string host;

  /// The port the kinetic server is running on.
  int port;

  /// If true secure all TCP traffic to the server using TLSv1. Otherwise
  /// all trafic will be plain-text TCP.
  bool use_ssl;

  /// The ID of the user to connect as.
  int user_id;

  /// The HMAC key of the user specified in user_id.
  std::string hmac_key;
};


} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_CONNECTION_OPTIONS_H_
