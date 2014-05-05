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
