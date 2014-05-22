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

#ifndef KINETIC_CPP_CLIENT_KINETIC_CONNECTION_H_
#define KINETIC_CPP_CLIENT_KINETIC_CONNECTION_H_

namespace kinetic {

enum class WriteMode {
    /// If specified, PUTs will succeed even if the key already exists and has a different
    /// version
    IGNORE_VERSION,

    /// Requires the existing version match the given expected value. This can be used to
    /// implement compare-and-swap type operations
    REQUIRE_SAME_VERSION
};

enum class PersistMode {
    /// This request is made persistent before returning
    WRITE_THROUGH,
    /// This request can be made persistent when the device chooses, or when a subsequent
    /// request with PersistMode FLUSH is issued on this connection
    WRITE_BACK,
    /// All pending information that has not been persisted will be before returning
    FLUSH
};

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_KINETIC_CONNECTION_H_
