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

#include "kinetic/outgoing_value.h"

#include <fcntl.h>
#include <unistd.h>

#include <algorithm>

#include "glog/logging.h"

#include "kinetic/reader_writer.h"

namespace kinetic {

OutgoingStringValue::OutgoingStringValue(const std::string &s)
    : s_(s) {}

size_t OutgoingStringValue::size() const {
    return s_.size();
}

bool OutgoingStringValue::TransferToSocket(int fd, int* err) const {
    ReaderWriter reader_writer(fd);
    return reader_writer.Write(s_.data(), s_.size());
}

bool OutgoingStringValue::ToString(std::string *result, int* err) const {
    *result = s_;
    return true;
}

} // namespace kinetic
