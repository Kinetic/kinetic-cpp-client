[![Build Status](https://travis-ci.org/Kinetic/kinetic-cpp-client.svg?branch=master)](https://travis-ci.org/Kinetic/kinetic-cpp-client)

Introduction
============
This repository contains a C++ kinetic client implementation.

Protocol Version
=================
The client is using version `3.0.5` of the [Kinetic-Protocol](https://github.com/Kinetic/kinetic-protocol).


Dependencies
============
* cmake 
* gcc 4.4 or higher. Or another c++ compiler supporting at least the c++0x feature set provided by gcc 4.4.

### Optional Dependencies 

* glog, gflags, openssl, protobuf, protobuf-compiler

If a library is not found installed on the system, it will be build and linked statically into the kinetic-cpp-client library. Static linkage may be convenient, but will drastically increase build time for the kinetic-cpp-client and may land you in linker-hell should you attempt to use the same libraries in your application.

It is **strongly recommended** to at least install the openssl and protobuf libraries. On most platforms, package managers will install the corresponding packages with their development files for you. E.g. 
 * yum install openssl openssl-devel protobuf-devel 
 * apt-get install openssl libssl-dev libprotobuf-dev protobuf-compiler  
 * brew install openssl protobuf

You may force static linkage by setting the corresponding `LIBNAME_STATIC` variable. This may be desirable if you plan deploying the compiled library on other machines. As an example, `cmake -DPROTOBUF_STATIC=ON /path/to/git` will link protobuf statically into the kinetic-cpp-client even if it is installed on the system that is compiling the library. 

### Other
* doxygen/graphviz for generating documentation
* valgrind for running tests with leak detection (3.12+ required to run with openssl)

Compilation
============
1. Install any missing dependencies
2. Clone this git repository 
3. Create a build directory. If you want you can use the cloned git repository as your build directory, but using a separate directory is recommended in order to cleanly separate sources and generated files. 
4. From your build directory call `cmake /path/to/git`, if you're using the cloned git repository as your build directory this would be `cmake .` 
5. Run `make`

#### BUILD OPTIONS:
The following cmake options modify build behavior. You may change an option using the `cmake -DOPTION=ON/OFF /path/to/git` syntax. 

+ PTHREAD_LOCKS (default: on): If set, pthread locks are registered with the OpenSSL library to ensure thread safety. Unset if you are compiling in a non-pthread environment or otherwise wish to register locks yourself from your application. 
+ BUILD_TEST (default: off): Builds unit and integration tests.

Versioning
============
The library follows the Major.Minor.Patch versioning scheme, `.so` versioning is done automatically during compilation. 

Changes to the *Major* version indicate that the library interface has changed. Different *Major* versions of the library will not be binary compatible. 

Changes to the *Minor* version number indicate that backwards compatible changes to the interface have ocurred. Usually this means that additional functions are available. It resets to zero if the *Major* version changes. 

The patch version equals the git commit number to allow easily associating an installed library version with a particular git commit. It does not reset if the *Major* or *Minor* versions change.  


Installation
============
 `sudo make install` 

RPM PACKAGE GENERATION: 
============
`make package`

Common Developer Tasks
======================

**Running tests**: Ensure you built the test binaries (BUILD_TEST cmake option has to be set).

To run the unit test suite, run `make check`. Tests results
will appear on stdout and a JUnit report be written to `gtestresults.xml`

There is also an integration test suite. This suite reads the environment
variable `KINETIC_PATH` to determine a simulator executable to run tests
against. If that variable is not set, it instead assumes that a Kinetic server
is running on port 8123 on `localhost`. To run the integration tests, set
`KINETIC_PATH` if appropriate and run `make integration_test`. This will write
a JUnit report to `integrationresults.xml`.

**Running tests with leak check**: Run `make test_valgrind` for the unit test
suite or `make integration_test_valgrind` for the integration test suite.

**Generating documentation**: `make doc`. HTML documentation will be generated in `docs/`
