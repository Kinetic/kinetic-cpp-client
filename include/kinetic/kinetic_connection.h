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
