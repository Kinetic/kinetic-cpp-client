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
