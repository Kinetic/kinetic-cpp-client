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

#ifndef KINETIC_CPP_CLIENT_NONBLOCKING_STRING_H_
#define KINETIC_CPP_CLIENT_NONBLOCKING_STRING_H_

#include <string>
#include <memory>

#include "protobufutil/common.h"

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
    NonblockingStringReader(int fd, size_t size, unique_ptr<const string> &s);
    ~NonblockingStringReader();
    NonblockingStringStatus Read();

    private:
    const int fd_;
    const size_t size_;
    unique_ptr<const string> &s_;
    char *const buf_;
    size_t bytes_read_;
    DISALLOW_COPY_AND_ASSIGN(NonblockingStringReader);
};

class NonblockingStringWriter {
    public:
    NonblockingStringWriter(int fd, const shared_ptr<const string> s);
    NonblockingStringStatus Write();

    private:
    int fd_;
    const shared_ptr<const string> s_;
    size_t bytes_written_;
    DISALLOW_COPY_AND_ASSIGN(NonblockingStringWriter);
};

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_NONBLOCKING_STRING_H_
