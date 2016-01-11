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

#ifndef KINETIC_CPP_CLIENT_KINETIC_RECORD_H_
#define KINETIC_CPP_CLIENT_KINETIC_RECORD_H_

#include <memory>
#include "kinetic/common.h"
#include "kinetic_client.pb.h"

namespace kinetic {

using com::seagate::kinetic::client::proto::Command_Algorithm;
using std::shared_ptr;
using std::string;
using std::make_shared;


/// Encapsulates a single value stored in a Kinetic drive and the associated
/// metadata
class KineticRecord {
    public:
    KineticRecord(const shared_ptr<const string> value, const shared_ptr<const string> version,
            const shared_ptr<const string> tag, Command_Algorithm algorithm) :
            value_(value), version_(version), tag_(tag), algorithm_(
                    algorithm) {
    }
    KineticRecord(const string value, const string version, const string tag,
            Command_Algorithm algorithm) :
        value_(make_shared<string>(value)), version_(make_shared<string>(version)),
        tag_(make_shared<string>(tag)), algorithm_(algorithm) {
    }
    explicit KineticRecord(const KineticRecord& other) : value_(other.value_),
        version_(other.version_), tag_(other.tag_), algorithm_(other.algorithm_) {
    }

    /// The value itself
    const shared_ptr<const string> value() const {
        return value_;
    }

    /// The value's version
    const shared_ptr<const string> version() const {
        return version_;
    }

    /// An arbitrary tag, usually a hash or checksum of
    /// the value
    const shared_ptr<const string> tag() const {
        return tag_;
    }

    /// The algorithm used to generate the tag
    Command_Algorithm algorithm() const {
        return algorithm_;
    }

    private:
    const shared_ptr<const string> value_;
    const shared_ptr<const string> version_;
    const shared_ptr<const string> tag_;
    const Command_Algorithm algorithm_;
    // disallow operator=
    void operator=(const KineticRecord&);
};

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_KINETIC_RECORD_H_
