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

#ifndef KINETIC_CPP_CLIENT_MATCHERS_H_
#define KINETIC_CPP_CLIENT_MATCHERS_H_

#include <memory>

#include "gmock/gmock.h"

namespace kinetic {

using std::shared_ptr;
using std::string;
using std::vector;
using ::testing::MatcherInterface;
using ::testing::Matcher;

class StringSharedPtrMatcher : public MatcherInterface<shared_ptr<const string>> {
    public:
    StringSharedPtrMatcher(const std::string s) : s_(s) {}

    virtual bool MatchAndExplain(const shared_ptr<const string> other,
        testing::MatchResultListener *listener) const {
        if (s_ == *other) {
            return true;
        }
        *listener << "expected = <" << s_ << ">, actual = <" << *other << ">";
        return false;
    }

    virtual void DescribeTo(::std::ostream *os) const {
        *os << "s=" << s_;
    }

    private:
    const std::string s_;
};

inline Matcher<shared_ptr<const string>> StringSharedPtrEq(const string s) {
    return MakeMatcher(new StringSharedPtrMatcher(s));
};

class VectorStringPtrMatcher : public MatcherInterface<vector<string>*> {
    public:
    VectorStringPtrMatcher(vector<string> v) : v_(v) {}

    virtual bool MatchAndExplain(vector<string>* other,
            testing::MatchResultListener *listener) const {
        if (v_ == *other) {
            return true;
        }
        *listener << "expected = <" << v_.size() << ">, actual = <" << other->size() << ">";
        return false;
    }

    virtual void DescribeTo(::std::ostream *os) const {
        *os << "v=" << v_.size();
    }

    private:
    vector<string> v_;
};

inline Matcher<vector<string>*> VectorStringPtrEq(vector<string> v) {
    return MakeMatcher(new VectorStringPtrMatcher(v));
}

class KineticStatusMatcher : public MatcherInterface<KineticStatus> {
    public:
    KineticStatusMatcher(StatusCode code, string message) : code_(code), message_(message) {}

    virtual bool MatchAndExplain(KineticStatus other,
            testing::MatchResultListener *listener) const {
        if (code_ == other.statusCode() && message_ == other.message()) {
            return true;
        }

        *listener << "expected = <" << static_cast<int>(code_) << ":" << message_ << ">, actual = <"
            << static_cast<int>(other.statusCode()) << ":" << other.message() << ">";
        return false;
    }

    virtual void DescribeTo(::std::ostream *os) const {
        *os << "expected = <" << static_cast<int>(code_) << ":" << message_ << ">";
    }

    private:
    StatusCode code_;
    string message_;
};

inline Matcher<KineticStatus> KineticStatusEq(StatusCode code, string message) {
    return MakeMatcher(new KineticStatusMatcher(code, message));
}

} // namespace kinetic
#endif  // KINETIC_CPP_CLIENT_MATCHERS_H_
