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

#ifndef KINETIC_CPP_CLIENT_KEY_RANGE_ITERATOR_H_
#define KINETIC_CPP_CLIENT_KEY_RANGE_ITERATOR_H_

#include <string>
#include <vector>

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
