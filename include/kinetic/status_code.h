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

#ifndef KINETIC_CPP_CLIENT_STATUS_CODE_H_
#define KINETIC_CPP_CLIENT_STATUS_CODE_H_

#include "kinetic_client.pb.h"

namespace kinetic {

using com::seagate::kinetic::client::proto::Command_Status_StatusCode;
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
    PROTOCOL_ERROR_RESPONSE_NO_ACKSEQUENCE,
    REMOTE_NESTED_OPERATION_ERRORS,
    REMOTE_INVALID_BATCH
};

StatusCode ConvertFromProtoStatus(Command_Status_StatusCode status);
Command_Status_StatusCode ConvertToProtoStatus(StatusCode status);

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_STATUS_CODE_H_
