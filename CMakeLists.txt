cmake_minimum_required(VERSION 2.8.6)
project(kinetic_cpp_client CXX C)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
include(ExternalProject)

################################################################################
# Compiler flags.
set(CMAKE_CXX_FLAGS "--std=c++0x -Wall -Wextra -Werror -Wno-unknown-warning-option -Wno-unused-parameter -Wno-null-dereference -Wno-unused-local-typedefs -fPIC")
if (APPLE)
    # Ignore deprecated warnings due to OpenSSL
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated")
endif ()

################################################################################
# Library Versioning. Patch version equals commit number.
execute_process(
        COMMAND git log --oneline
        COMMAND wc -l
        COMMAND tr -d [:space:]
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMITS
)
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 3)
set(PROJECT_VERSION_PATCH ${GIT_COMMITS})
set(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})
message(STATUS "Project version set to ${PROJECT_VERSION}")

################################################################################
# Configuration.
option(BUILD_TEST "Build test executables." off)
option(PTHREAD_LOCKS "Register pthread locks with OpenSSL for thread-safety." on)
message(STATUS "Build Options:
    BUILD_TEST=${BUILD_TEST} PTHREAD_LOCKS=${PTHREAD_LOCKS}")

if (PTHREAD_LOCKS)
    add_definitions("-DUSE_PTHREAD_LOCKS")
endif ()

################################################################################
# Dependencies can either be pre-installed (recommended, especially for OpenSSL) or can be build automatically at
# compile time and statically linked into the kinetic c++ library. Standard behavior is to link everything that can be
# found dynamically. You may manually set LIB_STATIC to force static linkage of a library.
option(GLOG_STATIC OFF)
option(GFLAGS_STATIC OFF)
option(PROTOBUF_STATIC OFF)
option(OPENSSL_STATIC OFF)

find_package(OpenSSL)
find_package(Protobuf)
find_package(Gflags)
find_package(Glog)

if (NOT GLOG_FOUND)
    set(GLOG_STATIC ON)
endif()
if (NOT GFLAGS_FOUND)
    set(GFLAGS_STATIC ON)
endif()
if (NOT PROTOBUF_FOUND)
    set(PROTOBUF_STATIC ON)
endif()
if (NOT OPENSSL_FOUND)
    set(OPENSSL_STATIC ON)
endif()

message(STATUS "Dependency Configuration:
    GLOG_STATIC=${GLOG_STATIC} GFLAGS_STATIC=${GFLAGS_STATIC} PROTOBUF_STATIC=${PROTOBUF_STATIC} OPENSSL_STATIC=${OPENSSL_STATIC}")

set(STATIC_PREFIX ${kinetic_cpp_client_BINARY_DIR}/vendor)
set(STATIC_LIBDIR ${STATIC_PREFIX}/lib)

if (GFLAGS_STATIC)
    ExternalProject_add(
            gflags
            PREFIX ${STATIC_PREFIX}
            URL "${kinetic_cpp_client_SOURCE_DIR}/tarballs/gflags-2.0-no-svn-files.tar.gz"
            URL_MD5 "9084829124e02a7e6be0f0f824523423"
            CONFIGURE_COMMAND ../gflags/configure --prefix=${STATIC_PREFIX} --libdir=${STATIC_LIBDIR} --enable-static --with-pic
    )
    set(GFLAGS_LIBRARIES ${STATIC_LIBDIR}/libgflags.a)
    set(GFLAGS_INCLUDE_DIR ${STATIC_PREFIX}/include)
endif()

if (GLOG_STATIC)
    if(GFLAGS_STATIC)
        ExternalProject_add(
                glog
                PREFIX ${STATIC_PREFIX}
                URL "${kinetic_cpp_client_SOURCE_DIR}/tarballs/glog-0.3.3.tar.gz"
                URL_MD5 "a6fd2c22f8996846e34c763422717c18"
                PATCH_COMMAND sh ${kinetic_cpp_client_SOURCE_DIR}/patches/apply-glog-patches.sh ${kinetic_cpp_client_SOURCE_DIR}
                CONFIGURE_COMMAND ../glog/configure --prefix=${STATIC_PREFIX} --with-gflags=${STATIC_PREFIX} --libdir=${STATIC_LIBDIR} --enable-static --with-pic
                DEPENDS gflags
        )
    else()
        ExternalProject_add(
                glog
                PREFIX ${STATIC_PREFIX}
                URL "${kinetic_cpp_client_SOURCE_DIR}/tarballs/glog-0.3.3.tar.gz"
                URL_MD5 "a6fd2c22f8996846e34c763422717c18"
                PATCH_COMMAND sh ${kinetic_cpp_client_SOURCE_DIR}/patches/apply-glog-patches.sh ${kinetic_cpp_client_SOURCE_DIR}
                CONFIGURE_COMMAND ../glog/configure --prefix=${STATIC_PREFIX} --libdir=${STATIC_LIBDIR} --enable-static --with-pic
        )
    endif()
    add_definitions(-DGOOGLE_STRIP_LOG=2)
    set(GLOG_LIBRARIES ${STATIC_LIBDIR}/libglog.a)
    set(GLOG_INCLUDE_DIR ${STATIC_PREFIX}/include)
endif ()

if (OPENSSL_STATIC)
    if(APPLE)
        # On OSX we must explicitly specify that we want to build for x86-64
        set(OPENSSL_CONFIGURE_COMMAND ./Configure darwin64-x86_64-cc)
    else()
        set(OPENSSL_CONFIGURE_COMMAND ../openssl/config -DPURIFY)
    endif()
    ExternalProject_add(
            openssl
            PREFIX ${STATIC_PREFIX}
            URL "${kinetic_cpp_client_SOURCE_DIR}/tarballs/openssl-1.0.1g.tar.gz"
            URL_MD5 "de62b43dfcd858e66a74bee1c834e959"
            BUILD_IN_SOURCE 1
            CONFIGURE_COMMAND ${OPENSSL_CONFIGURE_COMMAND} --prefix=${STATIC_PREFIX} -fPIC
            BUILD_COMMAND touch apps/openssl && touch openssl.pc && make build_libs libssl.pc libcrypto.pc
            INSTALL_COMMAND make install_sw
    )
    set(OPENSSL_LIBRARIES  ${STATIC_LIBDIR}/libssl.a ${STATIC_LIBDIR}/libcrypto.a)
    set(OPENSSL_INCLUDE_DIR ${STATIC_PREFIX}/include)
endif ()

if (PROTOBUF_STATIC)
    # The protobuf build requires the existence of a protoc binary.
    ExternalProject_add(
            protoc
            PREFIX ${STATIC_PREFIX}/host
            URL "${kinetic_cpp_client_SOURCE_DIR}/tarballs/protobuf-2.5.0.tar.bz2"
            URL_MD5 "a72001a9067a4c2c4e0e836d0f92ece4"
            CONFIGURE_COMMAND ../protoc/configure --prefix=${STATIC_PREFIX}/host --enable-static
    )
    ExternalProject_add(
            protobuf
            PREFIX ${STATIC_PREFIX}
            URL "${kinetic_cpp_client_SOURCE_DIR}/tarballs/protobuf-2.5.0.tar.bz2"
            URL_MD5 "a72001a9067a4c2c4e0e836d0f92ece4"
            CONFIGURE_COMMAND ../protobuf/configure --with-protoc=${PROTOC_PATH} --prefix=${STATIC_PREFIX} --libdir=${STATIC_LIBDIR} --enable-static --with-pic
            DEPENDS protoc
    )
    set(PROTOBUF_LIBRARIES ${STATIC_LIBDIR}/libprotobuf.a)
    set(PROTOBUF_INCLUDE_DIR ${STATIC_PREFIX}/include)
    set(PROTOC_PATH "${STATIC_PREFIX}/host/bin/protoc")
else()
    set(PROTOC_PATH "protoc")
endif ()

# If libunwind is installed it is used by glog and thus has to linked.
# Otherwise glog uses the standard glibc unwinder and there is no dependency.
find_library(LIBUNWIND "unwind")
if (LIBUNWIND)
    set(GLOG_LIBRARIES ${GLOG_LIBRARIES} ${LIBUNWIND})
endif ()

################################################################################
# Download and compile google protobuf kinetic protocol definition
set(GENERATED_SOURCES_PATH ${kinetic_cpp_client_SOURCE_DIR}/src/main/generated)
ExternalProject_add(kinetic-proto
        PREFIX proto
        GIT_REPOSITORY https://github.com/Seagate/kinetic-protocol
        GIT_TAG 3.0.5
        UPDATE_COMMAND mkdir -p ${GENERATED_SOURCES_PATH}
        CONFIGURE_COMMAND ""
        BUILD_IN_SOURCE 1
        BUILD_COMMAND sed s/com\\.seagate\\.kinetic\\.proto/com.seagate.kinetic.client.proto/ kinetic.proto > kinetic_client.proto
        INSTALL_COMMAND cp kinetic_client.proto ${GENERATED_SOURCES_PATH}/kinetic_client.proto
        )
add_custom_command(
        COMMENT "Compiling protobuf"
        OUTPUT ${GENERATED_SOURCES_PATH}/kinetic_client.pb.cc ${GENERATED_SOURCES_PATH}/kinetic_client.pb.h
        COMMAND ${PROTOC_PATH} --proto_path=${GENERATED_SOURCES_PATH} --cpp_out=${GENERATED_SOURCES_PATH} ${GENERATED_SOURCES_PATH}/kinetic_client.proto
        COMMAND cp ${GENERATED_SOURCES_PATH}/kinetic_client.pb.h ${kinetic_cpp_client_SOURCE_DIR}/include/kinetic
        DEPENDS kinetic-proto
)
add_custom_target(proto-compile
        DEPENDS ${GENERATED_SOURCES_PATH}/kinetic_client.pb.cc ${GENERATED_SOURCES_PATH}/kinetic_client.pb.h
        )
if (PROTOBUF_STATIC)
    add_dependencies(proto-compile protobuf)
endif()
set_source_files_properties(${GENERATED_SOURCES_PATH}/kinetic_client.pb.cc PROPERTIES GENERATED TRUE)

################################################################################
# Main library compilation
include_directories(
        include
        src/main
        ${GENERATED_SOURCES_PATH}
        ${PROTOBUF_INCLUDE_DIR}
        ${OPENSSL_INCLUDE_DIR}
        ${GFLAGS_INCLUDE_DIR}
        ${GLOG_INCLUDE_DIR}
)

set(KINETIC_SRC
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

add_library(kinetic_client_static STATIC ${KINETIC_SRC})
add_library(kinetic_client SHARED ${KINETIC_SRC})

add_dependencies(kinetic_client_static proto-compile)
add_dependencies(kinetic_client proto-compile)
if (GLOG_STATIC)
    add_dependencies(kinetic_client_static glog)
    add_dependencies(kinetic_client glog)
endif ()
if (GFLAGS_STATIC)
    add_dependencies(kinetic_client_static gflags)
    add_dependencies(kinetic_client gflags)
endif()
if (OPENSSL_STATIC)
    add_dependencies(kinetic_client_static openssl)
    add_dependencies(kinetic_client openssl)
endif()

target_link_libraries(kinetic_client_static
        ${OPENSSL_LIBRARIES}
        ${PROTOBUF_LIBRARIES}
        ${GLOG_LIBRARIES}
        ${GFLAGS_LIBRARIES}
        )
target_link_libraries(kinetic_client
        ${OPENSSL_LIBRARIES}
        ${PROTOBUF_LIBRARIES}
        ${GLOG_LIBRARIES}
        ${GFLAGS_LIBRARIES}
        )

set_target_properties(kinetic_client PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})

get_property(LIB64 GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS)
set(LIBSUFFIX "")
if (NOT APPLE AND "${LIB64}" STREQUAL "TRUE" AND "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
    set(LIBSUFFIX 64)
endif ()
message(STATUS "Library installation directory is set to ${CMAKE_INSTALL_PREFIX}/lib${LIBSUFFIX}")

install(TARGETS kinetic_client
        LIBRARY
        DESTINATION lib${LIBSUFFIX})
install(DIRECTORY ${kinetic_cpp_client_SOURCE_DIR}/include/
        DESTINATION include)

################################################################################
# Rule for generating docs
configure_file(${kinetic_cpp_client_SOURCE_DIR}/Doxyfile ${kinetic_cpp_client_BINARY_DIR}/Doxyfile)
add_custom_target(doc
        doxygen ${kinetic_cpp_client_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${kinetic_cpp_client_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
        )
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES docs)

################################################################################
# Test-suite compilation and custom make targets
if (BUILD_TEST)
    find_package(Threads REQUIRED)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGTEST_USE_OWN_TR1_TUPLE=1")

    ExternalProject_add(
            gtest
            PREFIX vendor
            URL "${kinetic_cpp_client_SOURCE_DIR}/tarballs/gtest-1.6.0.zip"
            URL_MD5 "4577b49f2973c90bf9ba69aa8166b786"
            BUILD_IN_SOURCE 1
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ${CMAKE_CXX_COMPILER} -DGTEST_USE_OWN_TR1_TUPLE=1 -I../gtest -I../gtest/include -c ../gtest/src/gtest-all.cc && ar -rv libgtest.a gtest-all.o && ranlib libgtest.a
            INSTALL_COMMAND ""
    )

    ExternalProject_add(
            gmock
            PREFIX vendor
            URL "${kinetic_cpp_client_SOURCE_DIR}/tarballs/gmock-1.6.0.zip"
            URL_MD5 "f547f47321ca88d3965ca2efdcc2a3c1"
            BUILD_IN_SOURCE 1
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ${CMAKE_CXX_COMPILER} -DGTEST_USE_OWN_TR1_TUPLE=1 -I../gmock -I../gmock/include -I../gtest -I../gtest/include -c ../gmock/src/gmock-all.cc && ar -rv libgmock.a gmock-all.o && ranlib libgmock.a
            INSTALL_COMMAND ""
            DEPENDS gtest
    )

    include_directories(
            src/test/mock
            src/test
            ${kinetic_cpp_client_BINARY_DIR}/vendor/src/gmock/include
            ${kinetic_cpp_client_BINARY_DIR}/vendor/src/gtest/include
    )

    add_executable(kinetic_client_test
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
    add_dependencies(kinetic_client_test kinetic_client gtest gmock)

    target_link_libraries(kinetic_client_test
            kinetic_client
            ${CMAKE_BINARY_DIR}/vendor/src/gtest/libgtest.a
            ${CMAKE_BINARY_DIR}/vendor/src/gmock/libgmock.a
            ${CMAKE_THREAD_LIBS_INIT}
            ${CMAKE_DL_LIBS}
            )

    add_executable(kinetic_integration_test
            src/integration_test/delete_test.cc
            src/integration_test/get_test.cc
            src/integration_test/nonexistent_server_test.cc
            src/integration_test/put_test.cc
            src/integration_test/blocking_smoketest.cc
            src/test/kinetic_cpp_client_test.cc
            )
    add_dependencies(kinetic_integration_test kinetic_client gtest gmock)

    target_link_libraries(kinetic_integration_test
            kinetic_client
            ${CMAKE_BINARY_DIR}/vendor/src/gtest/libgtest.a
            ${CMAKE_BINARY_DIR}/vendor/src/gmock/libgmock.a
            ${CMAKE_THREAD_LIBS_INIT}
            ${CMAKE_DL_LIBS}
            )

    # Rules for running unit and integration tests
    add_custom_target(check
            COMMAND ${kinetic_cpp_client_BINARY_DIR}/kinetic_client_test --gtest_output=xml:gtestresults.xml
            DEPENDS ${kinetic_cpp_client_BINARY_DIR}/kinetic_client_test
            )
    add_custom_target(integration_test
            COMMAND ${kinetic_cpp_client_BINARY_DIR}/kinetic_integration_test --gtest_output=xml:integrationresults.xml
            DEPENDS ${kinetic_cpp_client_BINARY_DIR}/kinetic_integration_test
            )

    # Rules for running unit and integration tests under Valgrind
    add_custom_target(test_valgrind
            COMMAND valgrind --leak-check=full --show-reachable=yes --track-fds=yes --suppressions=${kinetic_cpp_client_SOURCE_DIR}/valgrind_linux.supp ${kinetic_cpp_client_BINARY_DIR}/kinetic_client_test
            DEPENDS ${kinetic_cpp_client_BINARY_DIR}/kinetic_client_test
            )
    add_custom_target(integration_test_valgrind
            COMMAND valgrind --leak-check=full --show-reachable=yes --track-fds=yes --suppressions=${kinetic_cpp_client_SOURCE_DIR}/valgrind_linux.supp ${kinetic_cpp_client_BINARY_DIR}/kinetic_integration_test
            DEPENDS ${kinetic_cpp_client_BINARY_DIR}/kinetic_integration_test
            )
endif ()


################################################################################
# RPM generation rules
set(CPACK_PACKAGE_CONTACT "paul.h.lensing@seagate.com")
set(CPACK_PACKAGE_NAME ${CMAKE_PROJECT_NAME})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "kinetic c++ client")
set(CPACK_PACKAGE_VENDOR "Kinetic Open Storage Project")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")

set(CPACK_RPM_PACKAGE_LICENSE "Mozilla Public License, v. 2.0")
set(CPACK_RPM_PACKAGE_DESCRIPTION "A c++ client for the kinetic protocol. See https://www.openkinetic.org/ for information on the Kinetic Open Storage Project. Source code available at https://github.com/Kinetic/kinetic-cpp-client")
set(CPACK_RPM_PACKAGE_GROUP "Development/Libraries")
if (NOT GLOG_STATIC)
    set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES} google-glog")
endif ()
if (NOT GFLAGS_STATIC)
    set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES} gflags")
endif ()
if (NOT PROTOBUF_STATIC)
    set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES} protobuf")
endif ()
if (NOT OPENSSL_STATIC)
    set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES} openssl")
endif ()
set(CPACK_GENERATOR "RPM")
include(CPack)
