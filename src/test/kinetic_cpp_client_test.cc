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

#include <signal.h>

#include <iostream>

#include "gtest/gtest.h"
#include "glog/logging.h"

#include "kinetic_client.pb.h"
#include "mock_callbacks.h"

int main(int argc, char *argv[]) {
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();
    testing::InitGoogleTest(&argc, argv);
    // We need to ignore SIGPIPE because some tests deliberately write into
    // broken pipes. We don't want this to crash the entire test process.
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        std::cerr << "Failed to ignore SIGPIPE" << std::endl;
        exit(EXIT_FAILURE);
    }
    int status = RUN_ALL_TESTS();
    google::protobuf::ShutdownProtobufLibrary();
    google::ShutdownGoogleLogging();
    google::ShutDownCommandLineFlags();
    return status;
}
