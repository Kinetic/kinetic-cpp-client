[![Build Status](https://travis-ci.org/Seagate/kinetic-cpp-client.svg)](https://travis-ci.org/Seagate/kinetic-cpp-client)
Introduction
============
This repo contains code for producing C and C++ kinetic clients.

Temporary Branch Information
============
Please note that this branch supplies the generated protobuf files because the release for protocol version 3.0.6 is not final at the moment. 
Before this branch hits master, all files in src/main/generated/* should be removed (as well as this notice) and the CMakeLists.txt edited to set kinetic-proto target version to 3.0.6. 

Protocol Version
=================
The client is using version `3.0.6` of the [Kinetic-Protocol](https://github.com/Seagate/kinetic-protocol).

Dependencies
============
* CMake
* Valgrind for memory tests
* Doxygen/graphviz for generating documentation

Initial Setup
=============
1. Install any missing dependencies
1. Run `cmake .` to build a static library, or `cmake . -DBUILD_SHARED_LIBS=true` to build a shared library.
1. Run `make`

Common Developer Tasks
======================

**Building the lib**: `make`. It will be in `libkinetic_client.a`

**Running tests**: To run the unit test suite, run `make check`. Tests results
will appear on stdout and a JUnit report be written to `gtestresults.xml`

There is also an integration test suite. This suite reads the environment
variable `KINETIC_PATH` to determine a simulator executable to run tests
against. If that variable is not set, it instead assumes that a Kinetic server
is running on port 8123 on `localhost`. To run the integration tests, set
`KINETIC_PATH` if appropriate and run `make integration_test`. This will write
a JUnit report to `integrationresults.xml`.

**Running tests with leak check**: Run `make test_valgrind` for the unit test
suite or `make integration_test_valgrind` for the integration test suite.

**Checking code style**: `make lint`. Violations will be printed on stdout.

**Generating documentation**: `make doc`. HTML documentation will be generated in `docs/`

**Apply licenses**: Run something like `./apply_license.sh my_new_file.cc` or `./apply_license.sh src/*.h`

How to cross-compile
====================

Please note that cross-compile support is only to build static or shared library, not for tests.
1. Build and install all the dependencies outside, such as `openssl, protobuf, glog, gmock`
2. Run `cmake . -DBUILD_NATIVE=no" -DCMAKE_LIBRARY_PATH="path/to/lib/directory" -DCMAKE_INCLUDE_PATH="path/to/include/directory"` to build static library
or run `cmake . -DBUILD_NATIVE=no -DBUILD_SHARED_LIBS=true -DCMAKE_LIBRARY_PATH="path/to/lib/directory" -DCMAKE_INCLUDE_PATH="path/to/include/directory"` to build shared library.
3. Run `make`

"LIB_INSTALL_DIR" and "INC_INSTALL_DIR" cmake variables can also be provided to provide install path for "libkinetic_client.a" or "libkinetic_client.so"
