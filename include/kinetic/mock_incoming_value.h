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

#ifndef KINETIC_CPP_CLIENT_MOCK_INCOMING_VALUE_H_
#define KINETIC_CPP_CLIENT_MOCK_INCOMING_VALUE_H_

#include <string>

#include "gmock/gmock.h"

#include "common.h"

namespace kinetic {

class MockIncomingValue : public IncomingValueInterface {
    public:
    MockIncomingValue() {}
    MOCK_METHOD0(size, size_t());
    MOCK_METHOD1(TransferToFile, bool(int fd));
    MOCK_METHOD1(ToString, bool(std::string *result));
    MOCK_METHOD0(Consume, void());
};

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_MOCK_INCOMING_VALUE_H_
