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

#ifndef KINETIC_CPP_CLIENT_OUTGOING_VALUE_H_
#define KINETIC_CPP_CLIENT_OUTGOING_VALUE_H_

#include <string>

#include "common.h"

namespace kinetic {

class OutgoingValueInterface {
    public:
    virtual ~OutgoingValueInterface() {}
    virtual size_t size() const = 0;
    virtual bool TransferToSocket(int fd) const = 0;
    virtual bool ToString(std::string *result) const = 0;
};

class OutgoingStringValue : public OutgoingValueInterface {
    public:
    explicit OutgoingStringValue(const std::string &s);
    size_t size() const;
    bool TransferToSocket(int fd) const;
    bool ToString(std::string *result) const;

    private:
    const std::string s_;
    DISALLOW_COPY_AND_ASSIGN(OutgoingStringValue);
};

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_OUTGOING_VALUE_H_
