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

#ifndef KINETIC_CPP_CLIENT_BATCH_OPERATION_H_
#define KINETIC_CPP_CLIENT_BATCH_OPERATION_H_

#include "kinetic/nonblocking_kinetic_connection_interface.h"

namespace kinetic {

using std::string;
using std::shared_ptr;


/* Note that not supplying an argument to Commit and Abort is perfectly fine, in this case the batch will be
 * executed in blocking mode. */
class BatchOperation {
 private:
    int batch_id;
    shared_ptr<NonblockingKineticConnectionInterface> con;
    KineticStatus invalid;

    KineticStatus getResult(const shared_ptr<SimpleCallbackInterface> callback);

 public:
    KineticStatus Put(const string key, const string current_version,
            WriteMode mode, const shared_ptr<const KineticRecord> record);
    KineticStatus Put(const shared_ptr<const string> key, const shared_ptr<const string> current_version,
            WriteMode mode, const shared_ptr<const KineticRecord> record);
    KineticStatus Delete(const string key, const string version, WriteMode mode);
    KineticStatus Delete(const shared_ptr<const string> key, const shared_ptr<const string> version, WriteMode mode);

    KineticStatus Commit(const shared_ptr<SimpleCallbackInterface> callback = nullptr);
    KineticStatus Abort(const shared_ptr<SimpleCallbackInterface> callback = nullptr);

    explicit BatchOperation(std::shared_ptr<NonblockingKineticConnectionInterface> connection);
    ~BatchOperation();
};

}; // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_BATCH_OPERATION_H_
