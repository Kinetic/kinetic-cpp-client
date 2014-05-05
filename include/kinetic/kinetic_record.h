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

#ifndef KINETIC_CPP_CLIENT_KINETIC_RECORD_H_
#define KINETIC_CPP_CLIENT_KINETIC_RECORD_H_

#include <memory>
#include "protobufutil/common.h"
#include "kinetic_client.pb.h"

namespace kinetic {

using com::seagate::kinetic::client::proto::Message_Algorithm;
using std::shared_ptr;
using std::string;
using std::make_shared;


/// Encapsulates a single value stored in a Kinetic drive and the associated
/// metadata
class KineticRecord {
    public:
    KineticRecord(const shared_ptr<const string> value, const shared_ptr<const string> version,
            const shared_ptr<const string> tag, Message_Algorithm algorithm) :
            value_(value), version_(version), tag_(tag), algorithm_(
                    algorithm) {
    }
    KineticRecord(const string value, const string version, const string tag,
        Message_Algorithm algorithm) :
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
    Message_Algorithm algorithm() const {
        return algorithm_;
    }

    private:
    const shared_ptr<const string> value_;
    const shared_ptr<const string> version_;
    const shared_ptr<const string> tag_;
    const Message_Algorithm algorithm_;
    // disallow operator=
    void operator=(const KineticRecord&);
};

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_KINETIC_RECORD_H_
