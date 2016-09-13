[![Build Status](https://travis-ci.org/Kinetic/kinetic-cpp-client.svg?branch=master)](https://travis-ci.org/Kinetic/kinetic-cpp-client)

Introduction
============
This repository contains a C++ kinetic client implementation.

Protocol Version
=================
The client is using version `3.0.0` of the [Kinetic-Protocol](https://github.com/Kinetic/kinetic-protocol).


Dependencies
============

### Running 
* openssl and protobuf libraries
* optional (see [build options](#build-options)): glog and gflags libraries 

### Building 
* cmake 
* gcc 4.4 or higher. Or other c++ compiler supporting at least the c++0x feature set provided by gcc 4.4.
* openssl and protobuf headers
* protobuf-compiler
* optional (see [build options](#build-options)): glog and gflags headers 

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
The following cmake options modify build behavior. All options are enabled by default, they may be disabled using the `cmake -DOPTION=OFF /path/to/git` syntax. 

+ GOOGLE_STATIC: Builds static glog and gflags libraries and links them into the generated kinetic-cpp-client library. This makes life a little easier as the google libraries do not have to be installed separately. To prevent linker errors, unset if you are using either library in the application linking the kinetic-cpp-client library.   
+ PTHREAD_LOCKS: If set, pthread locks are registered with the OpenSSL library to ensure thread safety. Unset if you are compiling in a non-pthread environment or otherwise wish to register locks yourself from your application. 
+ BUILD_TEST: Builds unit and integration tests.

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

**Running tests**: To run the unit test suite, run `make check`. This make target will only be generated if BUILD_TEST cmake option is set. Tests results
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
