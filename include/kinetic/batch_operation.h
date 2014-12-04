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

#ifndef BATCH_OPERATION_H_
#define BATCH_OPERATION_H_

#include "kinetic/nonblocking_kinetic_connection_interface.h"

namespace kinetic {

using std::string;
using std::shared_ptr;


/* Note that not supplying an argument to Commit and Abort is perfectly fine, in this case the batch will be
 * executed in blocking mode. */
class BatchOperation final {
private:
    int batch_id;
    PersistMode persistMode;
    shared_ptr<NonblockingKineticConnectionInterface> con;
    KineticStatus getResult(const shared_ptr<SimpleCallbackInterface> callback);

public:
    KineticStatus Put   (const string key, const string current_version, WriteMode mode, const shared_ptr<const KineticRecord> record);
    KineticStatus Put   (const shared_ptr<const string> key, const shared_ptr<const string> current_version, WriteMode mode, const shared_ptr<const KineticRecord> record);
    KineticStatus Delete(const string key, const string version, WriteMode mode);
    KineticStatus Delete(const shared_ptr<const string> key, const shared_ptr<const string> version, WriteMode mode);

    KineticStatus Commit(const shared_ptr<SimpleCallbackInterface> callback = nullptr);
    KineticStatus Abort (const shared_ptr<SimpleCallbackInterface> callback = nullptr);

    explicit BatchOperation(std::shared_ptr<NonblockingKineticConnectionInterface> connection, PersistMode mode = PersistMode::WRITE_BACK);
    ~BatchOperation();
};

};

#endif /* BATCH_OPERATION_H_ */
