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
    KineticStatus(const StatusCode code,
      const string& message,
      const int64_t expected_cluster_version = 0) :
        code_(code),
        message_(message),
        expected_cluster_version_(expected_cluster_version) {}

    KineticStatus(const KineticStatus& status) :
      code_(status.code_),
      message_(status.message_),
      expected_cluster_version_(status.expected_cluster_version_) {}

    bool ok() const {
        return code_ == StatusCode::OK;
    }

    StatusCode statusCode() const {
        return code_;
    }

    const string& message() const {
        return message_;
    }

    int64_t expected_cluster_version() const {
      return expected_cluster_version_;
    }

    void operator=(const KineticStatus& other) {
        code_ = other.code_;
        message_ = other.message_;
        expected_cluster_version_ = other.expected_cluster_version_;
    }

    private:
    StatusCode code_;
    string message_;
    int64_t expected_cluster_version_;
};

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_KINETIC_STATUS_H_
