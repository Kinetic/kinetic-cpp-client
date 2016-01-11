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

#ifndef KINETIC_CPP_CLIENT_ACLS_H_
#define KINETIC_CPP_CLIENT_ACLS_H_

#include <string>
#include <list>

namespace kinetic {

typedef enum { READ, WRITE, DELETE, RANGE, SETUP, P2POP, GETLOG, SECURITY } Permission;

/// The permissions apply to keys that have value at offset. For example, if offset is 0 and value
/// is "log_server" the user will have the specified permissions over keys "log_server:today" and
/// "log_server:yesterday" but not "user_profile:123".
typedef struct {
    int64_t offset;
    std::string value;

    /// The operations the user can perform on the specified key range
    std::list<Permission> permissions;
} Scope;

typedef struct {
    /// User ID
    int64_t identity;

    /// Key to use for authenticating requests
    std::string hmac_key;

    /// List of scopes the user can operate in. For instance, a user might only have permission
    /// to write to a particular key range.
    std::list<Scope> scopes;
} ACL;

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_ACLS_H_
