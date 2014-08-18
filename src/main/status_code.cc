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

#include "kinetic/status_code.h"

namespace kinetic {

using com::seagate::kinetic::client::proto::Command_Status_StatusCode;
using com::seagate::kinetic::client::proto::Command_Status_StatusCode_INVALID_STATUS_CODE;
using com::seagate::kinetic::client::proto::Command_Status_StatusCode_HMAC_FAILURE;
using com::seagate::kinetic::client::proto::Command_Status_StatusCode_NOT_AUTHORIZED;
using com::seagate::kinetic::client::proto::Command_Status_StatusCode_NOT_FOUND;
using com::seagate::kinetic::client::proto::Command_Status_StatusCode_INTERNAL_ERROR;
using com::seagate::kinetic::client::proto::Command_Status_StatusCode_HEADER_REQUIRED;
using com::seagate::kinetic::client::proto::Command_Status_StatusCode_SUCCESS;
using com::seagate::kinetic::client::proto::Command_Status_StatusCode_VERSION_FAILURE;
using com::seagate::kinetic::client::proto::Command_Status_StatusCode_VERSION_MISMATCH;
using com::seagate::kinetic::client::proto::Command_Status_StatusCode_SERVICE_BUSY;
using com::seagate::kinetic::client::proto::Command_Status_StatusCode_EXPIRED;
using com::seagate::kinetic::client::proto::Command_Status_StatusCode_DATA_ERROR;
using com::seagate::kinetic::client::proto::Command_Status_StatusCode_PERM_DATA_ERROR;
using com::seagate::kinetic::client::proto::Command_Status_StatusCode_REMOTE_CONNECTION_ERROR;
using com::seagate::kinetic::client::proto::Command_Status_StatusCode_NO_SPACE;
using com::seagate::kinetic::client::proto::Command_Status_StatusCode_NO_SUCH_HMAC_ALGORITHM;
using com::seagate::kinetic::client::proto::Command_Status_StatusCode_INVALID_REQUEST;
using com::seagate::kinetic::client::proto::Command_Status_StatusCode_NESTED_OPERATION_ERRORS;

StatusCode ConvertFromProtoStatus(Command_Status_StatusCode status) {
    switch (status) {
        case Command_Status_StatusCode_SUCCESS:
            return StatusCode::OK;
        case Command_Status_StatusCode_HMAC_FAILURE:
            return StatusCode::REMOTE_HMAC_ERROR;
        case Command_Status_StatusCode_NOT_AUTHORIZED:
            return StatusCode::REMOTE_NOT_AUTHORIZED;
        case Command_Status_StatusCode_VERSION_FAILURE:
            return StatusCode::REMOTE_CLUSTER_VERSION_MISMATCH;
        case Command_Status_StatusCode_INTERNAL_ERROR:
            return StatusCode::REMOTE_INTERNAL_ERROR;
        case Command_Status_StatusCode_HEADER_REQUIRED:
            return StatusCode::REMOTE_HEADER_REQUIRED;
        case Command_Status_StatusCode_NOT_FOUND:
            return StatusCode::REMOTE_NOT_FOUND;
        case Command_Status_StatusCode_VERSION_MISMATCH:
            return StatusCode::REMOTE_VERSION_MISMATCH;
        case Command_Status_StatusCode_SERVICE_BUSY:
            return StatusCode::REMOTE_SERVICE_BUSY;
        case Command_Status_StatusCode_EXPIRED:
            return StatusCode::REMOTE_EXPIRED;
        case Command_Status_StatusCode_DATA_ERROR:
            return StatusCode::REMOTE_DATA_ERROR;
        case Command_Status_StatusCode_PERM_DATA_ERROR:
            return StatusCode::REMOTE_PERM_DATA_ERROR;
        case Command_Status_StatusCode_REMOTE_CONNECTION_ERROR:
            return StatusCode::REMOTE_REMOTE_CONNECTION_ERROR;
        case Command_Status_StatusCode_NO_SPACE:
            return StatusCode::REMOTE_NO_SPACE;
        case Command_Status_StatusCode_NO_SUCH_HMAC_ALGORITHM:
            return StatusCode::REMOTE_NO_SUCH_HMAC_ALGORITHM;
        case Command_Status_StatusCode_INVALID_REQUEST:
            return StatusCode::REMOTE_INVALID_REQUEST;
        case Command_Status_StatusCode_INVALID_STATUS_CODE:
            return StatusCode::CLIENT_INTERNAL_ERROR;
        case Command_Status_StatusCode_NESTED_OPERATION_ERRORS:
            return StatusCode::REMOTE_NESTED_OPERATION_ERRORS;
        default:
            return StatusCode::REMOTE_OTHER_ERROR;
    }
}

Command_Status_StatusCode ConvertToProtoStatus(StatusCode status) {
    switch (status) {
        case StatusCode::OK:
            return Command_Status_StatusCode_SUCCESS;
        case StatusCode::REMOTE_HMAC_ERROR:
            return Command_Status_StatusCode_HMAC_FAILURE;
        case StatusCode::REMOTE_NOT_AUTHORIZED:
            return Command_Status_StatusCode_NOT_AUTHORIZED;
        case StatusCode::REMOTE_CLUSTER_VERSION_MISMATCH:
            return Command_Status_StatusCode_VERSION_FAILURE;
        case StatusCode::REMOTE_INTERNAL_ERROR:
            return Command_Status_StatusCode_INTERNAL_ERROR;
        case StatusCode::REMOTE_HEADER_REQUIRED:
            return Command_Status_StatusCode_HEADER_REQUIRED;
        case StatusCode::REMOTE_NOT_FOUND:
            return Command_Status_StatusCode_NOT_FOUND;
        case StatusCode::REMOTE_VERSION_MISMATCH:
            return Command_Status_StatusCode_VERSION_MISMATCH;
        case StatusCode::REMOTE_SERVICE_BUSY:
            return Command_Status_StatusCode_SERVICE_BUSY;
        case StatusCode::REMOTE_EXPIRED:
            return Command_Status_StatusCode_EXPIRED;
        case StatusCode::REMOTE_DATA_ERROR:
            return Command_Status_StatusCode_DATA_ERROR;
        case StatusCode::REMOTE_PERM_DATA_ERROR:
            return Command_Status_StatusCode_PERM_DATA_ERROR;
        case StatusCode::REMOTE_REMOTE_CONNECTION_ERROR:
            return Command_Status_StatusCode_REMOTE_CONNECTION_ERROR;
        case StatusCode::REMOTE_NO_SPACE:
            return Command_Status_StatusCode_NO_SPACE;
        case StatusCode::REMOTE_NO_SUCH_HMAC_ALGORITHM:
            return Command_Status_StatusCode_NO_SUCH_HMAC_ALGORITHM;
        case StatusCode::REMOTE_INVALID_REQUEST:
            return Command_Status_StatusCode_INVALID_REQUEST;
        case StatusCode::REMOTE_NESTED_OPERATION_ERRORS:
            return Command_Status_StatusCode_NESTED_OPERATION_ERRORS;
        default:
            return Command_Status_StatusCode_INTERNAL_ERROR;
    }
}

} // namespace kinetic
