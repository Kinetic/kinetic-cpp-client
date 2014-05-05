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
