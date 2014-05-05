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

#include "kinetic/hmac_provider.h"

#include <list>
#include <arpa/inet.h>

#include <openssl/hmac.h>
#include <openssl/sha.h>
#include "glog/logging.h"

namespace kinetic {

using com::seagate::kinetic::client::proto::Message;

HmacProvider::HmacProvider() {}

std::string HmacProvider::ComputeHmac(const Message& message,
        const std::string& key) const {
    std::string input(message.command().SerializeAsString());

    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);
    HMAC_Init_ex(&ctx, key.c_str(), key.length(), EVP_sha1(), NULL);

    if (input.length() != 0) {
        uint32_t message_length_bigendian = htonl(input.length());
        HMAC_Update(&ctx, reinterpret_cast<unsigned char *>(&message_length_bigendian),
            sizeof(uint32_t));
        HMAC_Update(&ctx, reinterpret_cast<const unsigned char *>(input.c_str()),
            input.length());
    }

    unsigned char result[SHA_DIGEST_LENGTH];
    unsigned int result_length = SHA_DIGEST_LENGTH;
    HMAC_Final(&ctx, result, &result_length);
    HMAC_CTX_cleanup(&ctx);

    return std::string(reinterpret_cast<char *>(result), result_length);
}

bool HmacProvider::ValidateHmac(const Message& message,
        const std::string& key) const {
    std::string correct_hmac(ComputeHmac(message, key));

    if (!message.has_hmac()) {
        return false;
    }

    const std::string &provided_hmac = message.hmac();

    if (provided_hmac.length() != correct_hmac.length()) {
        return false;
    }

    int result = 0;
    for (size_t i = 0; i < correct_hmac.length(); i++) {
        result |= provided_hmac[i] ^ correct_hmac[i];
    }

    return result == 0;
}

} // namespace kinetic
