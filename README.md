Introduction
============
This repo contains code for producing C and C++ kinetic clients.


Protocol Version
=================
The client is using version `2.0.1` of the [Kinetic-Protocol](https://github.com/Seagate/kinetic-protocol/releases/tag/2.0.1).


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

**Running tests**: To run the unit test suite, run `make test`. Tests results
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