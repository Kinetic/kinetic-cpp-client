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

#ifndef KINETIC_CPP_CLIENT_KEY_RANGE_ITERATOR_H_
#define KINETIC_CPP_CLIENT_KEY_RANGE_ITERATOR_H_

#include <string>
#include <vector>
#include <memory>

#include "kinetic/blocking_kinetic_connection.h"

namespace kinetic {

using std::string;
using std::vector;
using std::unique_ptr;

class BlockingKineticConnection;

class KeyRangeIterator : public std::iterator<std::forward_iterator_tag, string> {
    public:
    KeyRangeIterator();
    explicit KeyRangeIterator(BlockingKineticConnection* p,
            unsigned int framesz,
            string start,
            bool start_inclusive,
            string end,
            bool end_inclusive);
    KeyRangeIterator(KeyRangeIterator const& rhs); // NOLINT
    KeyRangeIterator& operator=(KeyRangeIterator const& rhs);
    ~KeyRangeIterator();

    bool operator==(KeyRangeIterator const& rhs) const;
    bool operator!=(KeyRangeIterator const& rhs) const;

    KeyRangeIterator& operator++();
    KeyRangeIterator operator++(int);

    const string& operator*() const;
    string* operator->();

    private:
    BlockingKineticConnection* bconn_;
    string first_;
    bool first_inc_;
    string last_;
    bool last_inc_;
    unsigned int framesz_;
    bool reverse_order_;
    int relpos_;
    bool eol_;
    unique_ptr<vector<string>> keys_;

    void next_frame();
    void advance();
};

KeyRangeIterator KeyRangeEnd();

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_KEY_RANGE_ITERATOR_H_
