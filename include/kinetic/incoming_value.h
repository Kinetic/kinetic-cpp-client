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

#ifndef KINETIC_CPP_CLIENT_INCOMING_VALUE_H_
#define KINETIC_CPP_CLIENT_INCOMING_VALUE_H_

#include <string>

#include "common.h"

namespace kinetic {

/*
 * IncomingValueInterface represents a byte-array value arriving in a PUT
 * request. This can take the form of either a string in memory or a socket
 * file descriptor from which we can read a specified number of bytes.
 */
class IncomingValueInterface {
    public:
    virtual ~IncomingValueInterface() {}

    virtual size_t size() = 0;

    /*
     * TransferToFile transfers the contents of the value to a file represented
     * by the given file descriptor. As soon as this method has been called,
     * the object should be considered defunct and all further calls to
     * TransferToFile and ToString will fail.
     */
    virtual bool TransferToFile(int fd) = 0;

    /*
     * ToString copies the value to the string pointed to by the result
     * parameter.
     */
    virtual bool ToString(std::string *result) = 0;

    /*
     * Consume does whatever is necessary to consume the resources underlying
     * the value. In the SpliceableValue implementation this consists of
     * reading the appropriate bytes from the socket and throwing them away.
     * After this method has been called, the object should be considered
     * defunct and all further calls to TransferToFile and ToString will fail.
     */
    virtual void Consume() = 0;
};

/*
 * IncomingStringValue represents a value stored internally as a plain string.
 * It's preferable to use SpliceableValue whenever possible because of its
 * performance benefits.
 */
class IncomingStringValue : public IncomingValueInterface {
    public:
    explicit IncomingStringValue(const std::string &s);
    size_t size();
    bool TransferToFile(int fd);
    bool ToString(std::string *result);
    void Consume();

    private:
    const std::string s_;
    bool defunct_;
    DISALLOW_COPY_AND_ASSIGN(IncomingStringValue);
};

class IncomingValueFactoryInterface {
    public:
    virtual ~IncomingValueFactoryInterface() {}
    virtual IncomingValueInterface *NewValue(int fd, size_t n) = 0;
};

} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_INCOMING_VALUE_H_
