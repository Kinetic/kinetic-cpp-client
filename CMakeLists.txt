cmake_minimum_required(VERSION 2.8.6)
project(kinetic_cpp_client CXX C)

#
# Add a coverage mode build type
#
set(CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
set(CMAKE_C_FLAGS_COVERAGE "${CMAKE_C_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE string
    "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel Coverage"
    FORCE)

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
  set(OPENSSL_CONFIGURE_COMMAND ./Configure darwin64-x86_64-cc)
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

set(GENERATED_SOURCES_PATH ${kinetic_cpp_client_BINARY_DIR}/src/main/generated)

set(PREFIX "${CMAKE_BINARY_DIR}/vendor")
set(EXTERNAL_PREFIX "${kinetic_cpp_client_BINARY_DIR}/vendor")

include(ExternalProject)

set(KINETIC_PROTO_VERSION "3.0.0")
set(KINETIC_PROTO_MD5 "85ca027b870811a297c1f6d792498934")

ExternalProject_add(
    kinetic-proto
    PREFIX ${PREFIX}
    URL "${kinetic_cpp_client_SOURCE_DIR}/tarballs/kinetic-protocol-${KINETIC_PROTO_VERSION}.tar.gz"
    URL_MD5 ${KINETIC_PROTO_MD5}
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

ExternalProject_add(
    gflags
    PREFIX ${EXTERNAL_PREFIX}
    URL "${kinetic_cpp_client_SOURCE_DIR}/tarballs/gflags-2.0-no-svn-files.tar.gz"
    URL_MD5 "9084829124e02a7e6be0f0f824523423"
    CONFIGURE_COMMAND ../gflags/configure --prefix=${EXTERNAL_PREFIX} --enable-static ${CONFIG_HOST_FLAG} ${CHILD_MAKE_FLAGS} ${PIC_MAKE_FLAGS}
)

ExternalProject_add(
    glog
    PREFIX ${EXTERNAL_PREFIX}
    URL "${kinetic_cpp_client_SOURCE_DIR}/tarballs/glog-0.3.3.tar.gz"
    URL_MD5 "a6fd2c22f8996846e34c763422717c18"
    PATCH_COMMAND sh ${kinetic_cpp_client_SOURCE_DIR}/patches/apply-glog-patches.sh ${kinetic_cpp_client_SOURCE_DIR}
    CONFIGURE_COMMAND ../glog/configure --prefix=${EXTERNAL_PREFIX} --with-gflags=${EXTERNAL_PREFIX} --enable-static ${CONFIG_HOST_FLAG} ${CHILD_MAKE_FLAGS} ${PIC_MAKE_FLAGS}
    DEPENDS gflags
)

ExternalProject_add(
    gtest
    PREFIX ${EXTERNAL_PREFIX}
    URL "${kinetic_cpp_client_SOURCE_DIR}/tarballs/gtest-1.6.0.zip"
    URL_MD5 "4577b49f2973c90bf9ba69aa8166b786"
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ${CMAKE_CXX_COMPILER} -DGTEST_USE_OWN_TR1_TUPLE=1 -I../gtest -I../gtest/include -c ../gtest/src/gtest-all.cc && ar -rv libgtest.a gtest-all.o && ranlib libgtest.a
    INSTALL_COMMAND ""
)

ExternalProject_add(
    gmock
    PREFIX ${EXTERNAL_PREFIX}
    URL "${kinetic_cpp_client_SOURCE_DIR}/tarballs/gmock-1.6.0.zip"
    URL_MD5 "f547f47321ca88d3965ca2efdcc2a3c1"
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ${CMAKE_CXX_COMPILER} -DGTEST_USE_OWN_TR1_TUPLE=1 -I../gmock -I../gmock/include -I../gtest -I../gtest/include -c ../gmock/src/gmock-all.cc && ar -rv libgmock.a gmock-all.o && ranlib libgmock.a
    INSTALL_COMMAND ""
    DEPENDS gtest
)

ExternalProject_add(
    openssl
    PREFIX ${EXTERNAL_PREFIX}
    URL "${kinetic_cpp_client_SOURCE_DIR}/tarballs/openssl-1.0.1g.tar.gz"
    URL_MD5 "de62b43dfcd858e66a74bee1c834e959"
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ${OPENSSL_CONFIGURE_COMMAND} --prefix=${EXTERNAL_PREFIX} ${BUILD_PIC_COMPILER_FLAG}
    BUILD_COMMAND touch apps/openssl && touch openssl.pc && make ${CHILD_MAKE_FLAGS} build_libs libssl.pc libcrypto.pc
    INSTALL_COMMAND make install_sw
)

# The protobuf build requires the existence of a protoc binary that can be
# executed on the host machine. To handle cross compilation, we always build
# protobuf once for the host so that we have a suitable copy of protoc.
ExternalProject_add(
    protoc
    PREFIX ${EXTERNAL_PREFIX}/host
    URL "${kinetic_cpp_client_SOURCE_DIR}/tarballs/protobuf-2.5.0.tar.bz2"
    URL_MD5 "a72001a9067a4c2c4e0e836d0f92ece4"
    CONFIGURE_COMMAND ../protoc/configure --prefix=${EXTERNAL_PREFIX}/host --enable-static
)

# Protobuf code generation rules
set(PROTOC_PATH "${PREFIX}/host/bin/protoc")
set(PROTO_DIR "${CMAKE_BINARY_DIR}/vendor/src/kinetic-proto")
set(PROTO_ORIG_PATH "${PROTO_DIR}/kinetic.proto")
set(PROTO_MODIFIED_PATH "${PROTO_DIR}/kinetic_client.proto")
ExternalProject_add(
    protobuf
    PREFIX ${EXTERNAL_PREFIX}
    URL "${kinetic_cpp_client_SOURCE_DIR}/tarballs/protobuf-2.5.0.tar.bz2"
    URL_MD5 "a72001a9067a4c2c4e0e836d0f92ece4"
    CONFIGURE_COMMAND ../protobuf/configure --prefix=${EXTERNAL_PREFIX} --enable-static --with-protoc=${PROTOC_PATH} ${CONFIG_HOST_FLAG} ${CHILD_MAKE_FLAGS} ${PIC_MAKE_FLAGS}
    DEPENDS protoc
)

add_custom_command(
    COMMENT "Compiling protobuf"
    OUTPUT ${GENERATED_SOURCES_PATH}/kinetic_client.pb.h ${GENERATED_SOURCES_PATH}/kinetic_client.pb.cc
    COMMAND mkdir -p ${GENERATED_SOURCES_PATH} && sed 's/com\\.seagate\\.kinetic\\.proto/com.seagate.kinetic.client.proto/' ${PROTO_ORIG_PATH} > ${PROTO_MODIFIED_PATH} && ${PROTOC_PATH} -I=${PROTO_DIR} --cpp_out=${GENERATED_SOURCES_PATH} ${PROTO_MODIFIED_PATH}
    DEPENDS kinetic-proto protoc protobuf
)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${GENERATED_SOURCES_PATH})

include_directories(
    include
    src/main

    src/test/mock
    src/test

    ${GENERATED_SOURCES_PATH}
    ${EXTERNAL_PREFIX}/include
    ${EXTERNAL_PREFIX}/src/gmock/include
    ${EXTERNAL_PREFIX}/src/gtest/include
)

set(LIBRARY_DEPENDENCIES
    kinetic_client
    ${CMAKE_BINARY_DIR}/vendor/lib/libglog.a
    ${CMAKE_BINARY_DIR}/vendor/lib/libgflags.a
    ${CMAKE_BINARY_DIR}/vendor/lib/libssl.a
    ${CMAKE_BINARY_DIR}/vendor/lib/libcrypto.a
    ${CMAKE_BINARY_DIR}/vendor/lib/libprotobuf.a
    ${CMAKE_BINARY_DIR}/vendor/src/gtest/libgtest.a
    ${CMAKE_BINARY_DIR}/vendor/src/gmock/libgmock.a
    ${CMAKE_THREAD_LIBS_INIT}
    dl
)

# If libunwind is installed it is used by glog and thus has to linked. 
# Otherwise glog uses the standard glibc unwinder and there is no dependency.
find_library(LIBUNWIND "unwind")
if(LIBUNWIND)
    set(LIBRARY_DEPENDENCIES 
        ${LIBRARY_DEPENDENCIES} 
        ${LIBUNWIND}
    )
endif()

add_library(kinetic_client
    ${GENERATED_SOURCES_PATH}/kinetic_client.pb.cc
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
    src/main/status_code.cc
    src/main/byte_stream.cc
    src/main/incoming_string_value.cc
    src/main/message_stream.cc
    src/main/outgoing_string_value.cc
    src/main/reader_writer.cc
    src/main/key_range_iterator.cc
)
add_dependencies(kinetic_client openssl)

add_executable(${TEST_BINARY}
    src/test/kinetic_cpp_client_test.cc
    src/test/nonblocking_kinetic_connection_test.cc
    src/test/nonblocking_packet_service_test.cc
    src/test/nonblocking_packet_sender_test.cc
    src/test/nonblocking_packet_receiver_test.cc
    src/test/nonblocking_packet_test.cc
    src/test/nonblocking_string_test.cc
    src/test/hmac_provider_test.cc
    src/test/message_stream_test.cc
    src/test/string_value_test.cc
)
target_link_libraries(${TEST_BINARY} ${LIBRARY_DEPENDENCIES})

add_executable(${INTEGRATION_TEST_BINARY}
    src/integration_test/delete_test.cc
    src/integration_test/get_test.cc
    src/integration_test/nonexistent_server_test.cc
    src/integration_test/put_test.cc
    src/integration_test/blocking_smoketest.cc
    src/test/kinetic_cpp_client_test.cc
)
target_link_libraries(${INTEGRATION_TEST_BINARY} ${LIBRARY_DEPENDENCIES})

# Rule for running unit tests
add_custom_target(check
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
configure_file(${kinetic_cpp_client_SOURCE_DIR}/Doxyfile ${kinetic_cpp_client_BINARY_DIR}/Doxyfile)
add_custom_target(doc
    doxygen ${kinetic_cpp_client_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${kinetic_cpp_client_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM
)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES docs)

# Rule for linting
configure_file(${kinetic_cpp_client_SOURCE_DIR}/bin/lint.sh ${kinetic_cpp_client_BINARY_DIR}/bin/lint.sh @ONLY)
add_custom_target(lint
    ${kinetic_cpp_client_BINARY_DIR}/bin/lint.sh
    WORKING_DIRECTORY ${kinetic_cpp_client_BINARY_DIR}
    COMMENT "Running style checker" VERBATIM
)
