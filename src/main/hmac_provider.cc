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
    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);
    HMAC_Init_ex(&ctx, key.c_str(), key.length(), EVP_sha1(), NULL);

    if (message.commandbytes().length() != 0) {
        uint32_t message_length_bigendian = htonl(message.commandbytes().length());
        HMAC_Update(&ctx, reinterpret_cast<unsigned char *>(&message_length_bigendian),
            sizeof(uint32_t));
        HMAC_Update(&ctx, reinterpret_cast<const unsigned char *>(message.commandbytes().c_str()),
            message.commandbytes().length());
    }

    unsigned char result[SHA_DIGEST_LENGTH];
    unsigned int result_length = SHA_DIGEST_LENGTH;
    HMAC_Final(&ctx, result, &result_length);
    HMAC_CTX_cleanup(&ctx);

    return std::string(reinterpret_cast<char *>(result), result_length);
}

bool HmacProvider::ValidateHmac(const Message& message, const std::string& key) const {
    std::string correct_hmac(ComputeHmac(message, key));

    if (!message.has_hmacauth()) {
        return false;
    }

    const std::string &provided_hmac = message.hmacauth().hmac();

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
