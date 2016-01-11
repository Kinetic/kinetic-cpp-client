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
