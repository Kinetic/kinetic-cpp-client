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

#ifndef KINETIC_CPP_CLIENT_STATUS_H_
#define KINETIC_CPP_CLIENT_STATUS_H_

#include <string>

namespace kinetic {

using std::string;

/// Indicates the success/failure of an operation. Frequently when calling
/// a Kinetic client method you'll do something like:
/// \code
/// CHECK(some_class->operation().ok());
/// \endcode
/// or maybe
/// \code
/// if (some_class->operation().notOk()) {
///   handleTheError();
/// }
/// \endcode
class Status {
    public:
        /// Helper for easily making a Status object indicating success
        static Status makeOk() {
            return Status("");
        }

        /// Helper for easily making a Status object indicating a generic error
        static Status makeInternalError(std::string error_message) {
            return Status(error_message);
        }

        /// True if the operation succeeded
        bool ok() const {
            return error_message_ == "";
        }

        /// False if anything went wrong. Details available in ToString
        bool notOk() const {
            return !ok();
        }

        /// Returns a human-friendly description of what went wrong, or
        /// "OK" if there was no error
        std::string ToString() const {
            return error_message_;
        }

    protected:
        /// The error as an std::string or an empty std::string if the
        /// operation succeeded
        explicit Status(std::string error_message)
                : error_message_(error_message) {
        }

    private:
        string error_message_;
};

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_STATUS_H_
