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
