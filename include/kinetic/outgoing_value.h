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

#ifndef KINETIC_CPP_CLIENT_OUTGOING_VALUE_H_
#define KINETIC_CPP_CLIENT_OUTGOING_VALUE_H_

#include <string>

#include "common.h"

namespace kinetic {

class OutgoingValueInterface {
    public:
    virtual ~OutgoingValueInterface() {}
    virtual size_t size() const = 0;
    virtual bool TransferToSocket(int fd, int* err) const = 0;
    virtual bool ToString(std::string *result, int* err) const = 0;
};

class OutgoingStringValue : public OutgoingValueInterface {
    public:
    explicit OutgoingStringValue(const std::string &s);
    size_t size() const;
    bool TransferToSocket(int fd, int* err) const;
    bool ToString(std::string *result, int* err) const;

    private:
    const std::string s_;
    DISALLOW_COPY_AND_ASSIGN(OutgoingStringValue);
};

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_OUTGOING_VALUE_H_
