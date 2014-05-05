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

#ifndef KINETIC_CPP_CLIENT_STATUS_CODE_H_
#define KINETIC_CPP_CLIENT_STATUS_CODE_H_

namespace kinetic {

using std::string;

enum class StatusCode {
    OK, // Not an error
    CLIENT_IO_ERROR,
    CLIENT_SHUTDOWN,
    CLIENT_INTERNAL_ERROR,
    CLIENT_RESPONSE_HMAC_VERIFICATION_ERROR,
    REMOTE_HMAC_ERROR,
    REMOTE_NOT_AUTHORIZED,
    REMOTE_CLUSTER_VERSION_MISMATCH,
    REMOTE_INVALID_REQUEST,
    REMOTE_INTERNAL_ERROR,
    REMOTE_HEADER_REQUIRED,
    REMOTE_NOT_FOUND,
    REMOTE_VERSION_MISMATCH,
    REMOTE_SERVICE_BUSY,
    REMOTE_EXPIRED,
    REMOTE_DATA_ERROR,
    REMOTE_PERM_DATA_ERROR,
    REMOTE_REMOTE_CONNECTION_ERROR,
    REMOTE_NO_SPACE,
    REMOTE_NO_SUCH_HMAC_ALGORITHM,
    REMOTE_OTHER_ERROR,
    PROTOCOL_ERROR_RESPONSE_NO_ACKSEQUENCE
};

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_STATUS_CODE_H_
