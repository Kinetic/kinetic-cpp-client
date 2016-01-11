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

#ifndef KINETIC_CPP_CLIENT_HMAC_PROVIDER_H_
#define KINETIC_CPP_CLIENT_HMAC_PROVIDER_H_

#include "kinetic_client.pb.h"

namespace kinetic {

using com::seagate::kinetic::client::proto::Message;

/// Wrapper class that handles computing HMACs. The supplied implementation uses openssl,
/// but users can supply an alternate implementation that uses a different library (e. g. one
/// providing specialized HW accelaration)
class HmacProvider {
    public:
    HmacProvider();
    virtual std::string ComputeHmac(const Message& message,
        const std::string& key) const;
    virtual bool ValidateHmac(const Message& message,
        const std::string& key) const;
};

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_HMAC_PROVIDER_H_
