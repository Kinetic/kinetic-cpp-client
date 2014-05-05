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
