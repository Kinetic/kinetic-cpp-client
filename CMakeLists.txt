cmake_minimum_required(VERSION 2.8.6)
project(kinetic_cpp_client CXX C)

find_package (Threads)

set(CMAKE_POSITION_INDEPENDENT_CODE ON) 

option(BUILD_FOR_ARM "Build for ARM instead of x86" off)
option(BUILD_PIC "Build static PIC" off)

set(BUILD_PIC_COMPILER_FLAGS "")

if(BUILD_PIC)
  set(BUILD_PIC_COMPILER_FLAGS "-fPIC")
endif(BUILD_PIC)

if(APPLE)
  # On OSX we must explicitly specify that we want to build for x86-64
  set(OPENSSL_CONFIGURE ./Configure darwin64-x86_64-cc)
else(APPLE)
  if(${BUILD_FOR_ARM})
    set(CMAKE_C_COMPILER "arm-marvell-linux-gnueabi-gcc")
    set(CMAKE_CXX_COMPILER "arm-marvell-linux-gnueabi-g++")
    set(CMAKE_RANLIB "arm-marvell-linux-gnueabi-ranlib")
    set(OPENSSL_CONFIGURE_COMMAND ../openssl/Configure linux-armv4)
    set(CONFIG_HOST_FLAG --host=arm)
    set(CHILD_MAKE_FLAGS CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER} RANLIB=${CMAKE_RANLIB})
  else(${BUILD_FOR_ARM})
    set(CMAKE_C_COMPILER "gcc")
    set(CMAKE_CXX_COMPILER "g++")
    set(OPENSSL_CONFIGURE_COMMAND ../openssl/config -DPURIFY)
  endif(${BUILD_FOR_ARM})
endif(APPLE)

set(CMAKE_CXX_FLAGS "--std=c++0x -Wall -Wextra -Werror -Wno-unknown-warning-option -Wno-unused-parameter -Wno-null-dereference -Wno-unused-local-typedefs -DGTEST_USE_OWN_TR1_TUPLE=1 ${BUILD_PIC_COMPILER_FLAGS}")

set(TEST_BINARY "kinetic_client_test")
set(TEST_BINARY_PATH ${kinetic_cpp_client_BINARY_DIR}/${TEST_BINARY})
set(INTEGRATION_TEST_BINARY "kinetic_integration_test")
set(INTEGRATION_TEST_BINARY_PATH ${kinetic_cpp_client_BINARY_DIR}/${INTEGRATION_TEST_BINARY})
set(TEST_LIBRARIES
    glog
    gtest
    gmock
    openssl
)

set(GENERATED_SOURCES_PATH ${kinetic_cpp_client_SOURCE_DIR}/src/main/generated)

set(PREFIX "${CMAKE_BINARY_DIR}/vendor")

include(ExternalProject)

set(PROTOBUFUTIL_VERSION "0.2.3")
set(PROTOBUFUTIL_MD5 "b60a0ff47222ab70e0ddb7a8e6ef6900")

ExternalProject_add(
    protobufutil
    PREFIX ${PREFIX}
    # cmake's use of curl doesn't work right on macs, so we use a DOWNLOAD_COMMAND instead
    # be warned... cmake's arg parsing is really simplistic: it just splits on spaces regardless of quotes.
    DOWNLOAD_COMMAND curl -L https://github.com/palominolabs/protobuf-util/archive/${PROTOBUFUTIL_VERSION}.tar.gz -o protobufutil-src.tar.gz && openssl md5 protobufutil-src.tar.gz | grep -q ${PROTOBUFUTIL_MD5} && mkdir -p protobufutil && tar -xz --strip-components 1 -C protobufutil -f protobufutil-src.tar.gz
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND cmake -DBUILD_FOR_ARM=${BUILD_FOR_ARM} -DBUILD_PIC=${BUILD_PIC}
    INSTALL_COMMAND ""
)

set(KINETIC_PROTO_VERSION "2.0.2")
set(KINETIC_PROTO_MD5 "f5ff4cbea0e909362c46032fb3775c49")

ExternalProject_add(
    kinetic-proto
    PREFIX ${PREFIX}
    DOWNLOAD_COMMAND curl -L https://github.com/Seagate/kinetic-protocol/archive/${KINETIC_PROTO_VERSION}.tar.gz -o kinetic-proto.tar.gz && openssl md5 kinetic-proto.tar.gz | grep -q ${KINETIC_PROTO_MD5} && rm -rf kinetic-proto && mkdir -p kinetic-proto && tar -xz --strip-components 1 -C kinetic-proto -f kinetic-proto.tar.gz
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

# Protobuf code generation rules
set(PROTOC_PATH "${PREFIX}/src/protobufutil/vendor/host/bin/protoc")
set(PROTO_DIR "${CMAKE_BINARY_DIR}/vendor/src/kinetic-proto")
set(PROTO_ORIG_PATH "${PROTO_DIR}/kinetic.proto")
set(PROTO_MODIFIED_PATH "${PROTO_DIR}/kinetic_client.proto")
add_custom_command(
    COMMENT "Compiling protobuf"
    OUTPUT ${GENERATED_SOURCES_PATH}/kinetic_client.pb.h ${GENERATED_SOURCES_PATH}/kinetic_client.pb.cc
    COMMAND mkdir -p ${GENERATED_SOURCES_PATH} && sed 's/com\\.seagate\\.kinetic\\.proto/com.seagate.kinetic.client.proto/' ${PROTO_ORIG_PATH} > ${PROTO_MODIFIED_PATH} && ${PROTOC_PATH} -I=${PROTO_DIR} --cpp_out=${GENERATED_SOURCES_PATH} ${PROTO_MODIFIED_PATH}
    DEPENDS protobufutil kinetic-proto
)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${GENERATED_SOURCES_PATH})

include_directories(
    include
    src/main/generated
    src/main

    src/test/mock
    src/test

    ${CMAKE_BINARY_DIR}/vendor/src/protobufutil/include
    ${CMAKE_BINARY_DIR}/vendor/src/protobufutil/vendor/include
    ${CMAKE_BINARY_DIR}/vendor/src/protobufutil/vendor/src/gmock/include
    ${CMAKE_BINARY_DIR}/vendor/src/protobufutil/vendor/src/gtest/include
)

set(LIBRARY_DEPENDENCIES
    kinetic_client
    ${CMAKE_BINARY_DIR}/vendor/src/protobufutil/libprotobufutil.a
    ${CMAKE_BINARY_DIR}/vendor/src/protobufutil/vendor/lib/libglog.a
    ${CMAKE_BINARY_DIR}/vendor/src/protobufutil/vendor/lib/libgflags.a
    ${CMAKE_BINARY_DIR}/vendor/src/protobufutil/vendor/lib/libssl.a
    ${CMAKE_BINARY_DIR}/vendor/src/protobufutil/vendor/lib/libcrypto.a
    ${CMAKE_BINARY_DIR}/vendor/src/protobufutil/vendor/lib/libprotobuf.a
    ${CMAKE_BINARY_DIR}/vendor/src/protobufutil/vendor/src/gtest/libgtest.a
    ${CMAKE_BINARY_DIR}/vendor/src/protobufutil/vendor/src/gmock/libgmock.a
    ${CMAKE_THREAD_LIBS_INIT}
    dl
)

add_library(kinetic_client
    src/main/generated/kinetic_client.pb.cc
    src/main/hmac_provider.cc
    src/main/kinetic_connection_factory.cc
    src/main/nonblocking_kinetic_connection.cc
    src/main/threadsafe_nonblocking_kinetic_connection.cc
    src/main/nonblocking_packet.cc
    src/main/nonblocking_packet_writer_factory.cc
    src/main/nonblocking_packet_service.cc
    src/main/nonblocking_packet_sender.cc
    src/main/nonblocking_packet_receiver.cc
    src/main/nonblocking_string.cc
    src/main/socket_wrapper.cc
    src/main/blocking_kinetic_connection.cc
    src/main/threadsafe_blocking_kinetic_connection.cc

    src/main/key_range_iterator.cc
)
add_dependencies(kinetic_client
    glog
    openssl
    protobuf
)

add_executable(${TEST_BINARY}
    src/test/kinetic_cpp_client_test.cc
    src/test/nonblocking_kinetic_connection_test.cc
    src/test/nonblocking_packet_service_test.cc
    src/test/nonblocking_packet_sender_test.cc
    src/test/nonblocking_packet_receiver_test.cc
    src/test/nonblocking_packet_test.cc
    src/test/nonblocking_string_test.cc
    src/test/hmac_provider_test.cc
)
add_dependencies(${TEST_BINARY} ${TEST_LIBRARIES})
target_link_libraries(${TEST_BINARY} ${LIBRARY_DEPENDENCIES})

add_executable(${INTEGRATION_TEST_BINARY}
    src/integration_test/delete_test.cc
    src/integration_test/get_test.cc
    src/integration_test/nonexistent_server_test.cc
    src/integration_test/put_test.cc
    src/integration_test/blocking_smoketest.cc
    src/test/kinetic_cpp_client_test.cc
)
add_dependencies(${INTEGRATION_TEST_BINARY} ${TEST_LIBRARIES})
target_link_libraries(${INTEGRATION_TEST_BINARY} ${LIBRARY_DEPENDENCIES})

# Rule for running unit tests
add_custom_target(test
    COMMAND ${TEST_BINARY_PATH} --gtest_output=xml:gtestresults.xml
    DEPENDS ${TEST_BINARY_PATH}
)

# Rule for running integration tests
add_custom_target(integration_test
    COMMAND ${INTEGRATION_TEST_BINARY_PATH} --gtest_output=xml:integrationresults.xml
    DEPENDS ${INTEGRATION_TEST_BINARY_PATH}
)

# Rules for running unit and integration tests under Valgrind
add_custom_target(test_valgrind
    COMMAND valgrind --leak-check=full --show-reachable=yes --track-fds=yes --suppressions=${kinetic_cpp_client_SOURCE_DIR}/valgrind_linux.supp ${TEST_BINARY_PATH}
    DEPENDS ${TEST_BINARY_PATH}
)
add_custom_target(integration_test_valgrind
    COMMAND valgrind --leak-check=full --show-reachable=yes --track-fds=yes --suppressions=${kinetic_cpp_client_SOURCE_DIR}/valgrind_linux.supp ${INTEGRATION_TEST_BINARY_PATH}
    DEPENDS ${INTEGRATION_TEST_BINARY_PATH}
)

# Rule for generating docs
add_custom_target(doc
    doxygen ${kinetic_cpp_client_SOURCE_DIR}/Doxyfile
    WORKING_DIRECTORY ${kinetic_cpp_client_SOURCE_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM
)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES docs)

# Rule for linting
add_custom_target(lint
    ./bin/lint.sh
    WORKING_DIRECTORY ${kinetic_cpp_client_SOURCE_DIR}
    COMMENT "Running style checker" VERBATIM
)
