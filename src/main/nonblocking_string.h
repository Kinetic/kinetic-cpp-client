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

#ifndef KINETIC_CPP_CLIENT_NONBLOCKING_STRING_H_
#define KINETIC_CPP_CLIENT_NONBLOCKING_STRING_H_

#include <string>
#include <memory>

#include "kinetic/common.h"
#include "socket_wrapper_interface.h"

namespace kinetic {

using std::shared_ptr;
using std::unique_ptr;
using std::string;

enum NonblockingStringStatus {
    kInProgress,
    kDone,
    kFailed
};

class NonblockingStringReader {
    public:
    NonblockingStringReader(shared_ptr<SocketWrapperInterface> socket_wrapper,
            size_t size, unique_ptr<const string> &s);
    ~NonblockingStringReader();
    NonblockingStringStatus Read();

    private:
    shared_ptr<SocketWrapperInterface> socket_wrapper_;
    const size_t size_;
    unique_ptr<const string> &s_;
    char *const buf_;
    size_t bytes_read_;
    DISALLOW_COPY_AND_ASSIGN(NonblockingStringReader);
};

class NonblockingStringWriter {
    public:
    NonblockingStringWriter(shared_ptr<SocketWrapperInterface> socket_wrapper, const shared_ptr<const string> s);
    NonblockingStringStatus Write();

    private:
    shared_ptr<SocketWrapperInterface> socket_wrapper_;
    const shared_ptr<const string> s_;
    size_t bytes_written_;
    DISALLOW_COPY_AND_ASSIGN(NonblockingStringWriter);
};

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_NONBLOCKING_STRING_H_
