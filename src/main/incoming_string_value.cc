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

#include "kinetic/incoming_value.h"

#include <fcntl.h>
#include <unistd.h>

#include <algorithm>

#include "glog/logging.h"

#include "kinetic/reader_writer.h"

namespace kinetic {

IncomingStringValue::IncomingStringValue(const std::string &s)
    : s_(s), defunct_(false) {}

size_t IncomingStringValue::size() {
    return s_.size();
}

bool IncomingStringValue::TransferToFile(int fd) {
    if (defunct_) {
        return false;
    }
    defunct_ = true;

    ReaderWriter reader_writer(fd);
    return reader_writer.Write(s_.data(), s_.size());
}

bool IncomingStringValue::ToString(std::string *result) {
    if (defunct_) {
        return false;
    }
    defunct_ = true;

    *result = s_;
    return true;
}

void IncomingStringValue::Consume() {
    defunct_ = true;
}

} // namespace kinetic
