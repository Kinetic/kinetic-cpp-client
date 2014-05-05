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

#ifndef KINETIC_CPP_CLIENT_KINETIC_STATUS_H_
#define KINETIC_CPP_CLIENT_KINETIC_STATUS_H_

#include "kinetic/status_code.h"

namespace kinetic {

using std::string;

/// Indicates whether a Kinetic operation (get, put, security, etc) put succeeded
/// or failed. Unlike Status it provides details like whether the failure resulted
/// from a version or an HMAC error
class KineticStatus {
    public:
    KineticStatus(const StatusCode code, const string& message) : code_(code), message_(message) {
    }

    KineticStatus(const KineticStatus& status) : code_(status.code_), message_(status.message_) {
    }

    bool ok() const {
        return code_ == StatusCode::OK;
    }

    StatusCode statusCode() const {
        return code_;
    }

    const string& message() const {
        return message_;
    }

    void operator=(const KineticStatus& other) {
        code_ = other.code_;
        message_ = other.message_;
    }

    private:
    StatusCode code_;
    string message_;
};

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_KINETIC_STATUS_H_

