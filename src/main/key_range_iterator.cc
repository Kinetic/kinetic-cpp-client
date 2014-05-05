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

#include "kinetic/key_range_iterator.h"
#include <stdexcept>

namespace kinetic {

using std::string;
using std::vector;

KeyRangeIterator KeyRangeEnd() {
    KeyRangeIterator it;
    return it;
}

KeyRangeIterator::KeyRangeIterator()
    : bconn_(NULL),
    first_(""),
    first_inc_(false),
    last_(""),
    last_inc_(false),
    framesz_(1),
    reverse_order_(false),
    relpos_(-1),
    eol_(true),
    keys_() { }

KeyRangeIterator::KeyRangeIterator(
        BlockingKineticConnection* p,
        unsigned int framesz,
        string start,
        bool start_inclusive,
        string end,
        bool end_inclusive)
    : bconn_(p),
    first_(start),
    first_inc_(start_inclusive),
    last_(end),
    last_inc_(end_inclusive),
    framesz_(framesz),
    reverse_order_(false),
    relpos_(-1),
    eol_(false),
    keys_() {
    this->next_frame();
}

KeyRangeIterator::KeyRangeIterator(const KeyRangeIterator& rhs)
    : bconn_(rhs.bconn_),
    first_(rhs.first_),
    first_inc_(rhs.first_inc_),
    last_(rhs.last_),
    last_inc_(rhs.last_inc_),
    framesz_(rhs.framesz_),
    reverse_order_(rhs.reverse_order_),
    relpos_(rhs.relpos_),
    eol_(rhs.eol_),
    keys_() {
    if (rhs.keys_.get() != NULL) {
        this->keys_ = unique_ptr<vector<string>>(new vector<string>(*rhs.keys_));
    }
}


KeyRangeIterator& KeyRangeIterator::operator=(KeyRangeIterator const& rhs) {
    if (&rhs != this) {
        this->bconn_ = rhs.bconn_;
        this->first_ = rhs.first_;
        this->first_inc_ = rhs.first_inc_;
        this->last_ = rhs.last_;
        this->last_inc_ = rhs.last_inc_;
        this->framesz_ = rhs.framesz_;
        this->reverse_order_ = rhs.reverse_order_;
        this->relpos_ = rhs.relpos_;
        this->eol_ = rhs.eol_;
        this->keys_.reset();
        if (rhs.keys_.get() != NULL) {
            this->keys_ = unique_ptr<vector<string>>(new vector<string>(*rhs.keys_));
        }
    }
    return *this;
}

KeyRangeIterator::~KeyRangeIterator() {}

bool KeyRangeIterator::operator==(KeyRangeIterator const& rhs) const {
    return ((rhs.eol_ == true && this->eol_ == true)
            || (rhs.relpos_ != -1 && this->relpos_ != -1
                    && rhs.keys_.get() != NULL && this->keys_.get() != NULL
                    && (*rhs.keys_)[rhs.relpos_] == (*this->keys_)[this->relpos_]));
}

bool KeyRangeIterator::operator!=(
        KeyRangeIterator const& rhs) const {
    return !(*this == rhs);
}

KeyRangeIterator& KeyRangeIterator::operator++() {
    if (this->relpos_ == -1 || this->keys_.get() == NULL) {
        throw std::runtime_error("Iterator is in a bad state");
    }

    this->advance();

    return *this;
}

KeyRangeIterator KeyRangeIterator::operator++(int unused) {
    if (this->relpos_ == -1 || this->keys_.get() == NULL) {
        throw std::runtime_error("Iterator is in a bad state");
    }

    KeyRangeIterator copy(*this);

    this->advance();

    return copy;
}

std::string* KeyRangeIterator::operator->() {
    if (this->relpos_ == -1 || this->keys_.get() == NULL) {
        throw std::runtime_error("Iterator is in a bad state");
    }
    if (this->eol_) {
        throw std::out_of_range("Iterator is out of bounds.");
    }
    return &(*this->keys_)[this->relpos_];
}

const std::string& KeyRangeIterator::operator*() const {
    if (this->relpos_ == -1 || this->keys_.get() == NULL) {
        throw std::runtime_error("Iterator is in a bad state");
    }
    if (this->eol_) {
        throw std::out_of_range("Iterator is out of bounds.");
    }
    return (*this->keys_)[this->relpos_];
}

void KeyRangeIterator::next_frame() {
    // Update the moving boundary key if it's not the first ever load
    if (this->relpos_ != -1 && this->keys_.get() != NULL) {
        this->first_ = (*this->keys_)[this->keys_->size() - 1];
        this->first_inc_ = false;
    }

    kinetic::KineticStatus status = this->bconn_->GetKeyRange(
            this->first_, this->first_inc_,
            this->last_, this->last_inc_,
            this->reverse_order_, this->framesz_,
            this->keys_);
    if (!status.ok()) {
        this->relpos_ = -1; // ERROR
        throw std::runtime_error(status.message());
    }

    this->relpos_ = 0;
    if (this->keys_.get() == NULL || this->keys_->size() == 0) {
        this->eol_ = true;
    }
}

void KeyRangeIterator::advance() {
    this->relpos_++;
    if (this->relpos_ == -1
            || this->relpos_ == static_cast<int>(this->keys_->size())) {
        this->next_frame();
    }
}

} // namespace kinetic
