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

#ifndef KINETIC_CPP_CLIENT_MOCK_HMAC_PROVIDER_H_
#define KINETIC_CPP_CLIENT_MOCK_HMAC_PROVIDER_H_

#include "kinetic/kinetic.h"

namespace com {
namespace seagate {
namespace kinetic {

class MockHmacProvider : public HmacProvider {
    public:
    MockHmacProvider() {}
    MOCK_CONST_METHOD2(ComputeHmac,
      std::string(const proto::Message& message, const std::string& key));
    MOCK_CONST_METHOD2(ValidateHmac, bool(const proto::Message& message, const std::string& key));
};

} // namespace kinetic
} // namespace seagate
} // namespace com

#endif  // KINETIC_CPP_CLIENT_MOCK_HMAC_PROVIDER_H_
