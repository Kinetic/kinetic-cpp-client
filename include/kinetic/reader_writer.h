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
