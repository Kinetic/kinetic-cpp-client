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

#ifndef KINETIC_CPP_CLIENT_READER_WRITER_H_
#define KINETIC_CPP_CLIENT_READER_WRITER_H_

#include <stddef.h>

#include "common.h"

namespace kinetic {

class ReaderWriter {
    public:
    explicit ReaderWriter(int fd);
    bool Read(void *buf, size_t n, int* err);
    bool Write(const void *buf, size_t n);

    private:
    int fd_;
    DISALLOW_COPY_AND_ASSIGN(ReaderWriter);
};

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_READER_WRITER_H_
