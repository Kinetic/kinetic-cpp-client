cmake_minimum_required(VERSION 2.8.6)
project(kinetic_cpp_client CXX C)
include(ExternalProject)

execute_process(
        COMMAND git log --oneline
        COMMAND wc -l
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMITS
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 2)
set(PROJECT_VERSION_PATCH ${GIT_COMMITS})
set(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})
message(STATUS "Project version set to ${PROJECT_VERSION}")

find_package(OpenSSL REQUIRED)
find_package(Protobuf REQUIRED)
find_library(LIBUNWIND "unwind")

option(BUILD_TEST "Build test executables." on)
option(GOOGLE_STATIC "Build & link google libraries statically. If not set, glog and gflags have to be installed." on)
option(PTHREAD_LOCKS "Register pthread locks with OpenSSL for thread-safety." on)
message(STATUS "Set Options: BUILD_TEST=${BUILD_TEST} GOOGLE_STATIC=${GOOGLE_STATIC} PTHREAD_LOCKS=${PTHREAD_LOCKS}")

set(CMAKE_CXX_FLAGS "--std=c++0x -Wall -Wextra -Werror -Wno-unknown-warning-option -Wno-unused-parameter -Wno-null-dereference -Wno-unused-local-typedefs -DGTEST_USE_OWN_TR1_TUPLE=1")
if(APPLE)
    # Ignore deprecated warnings due to OpenSSL
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated")
endif()

if (GOOGLE_STATIC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

    ExternalProject_add(
            gflags
            PREFIX ${kinetic_cpp_client_BINARY_DIR}/vendor
            URL "${kinetic_cpp_client_SOURCE_DIR}/tarballs/gflags-2.0-no-svn-files.tar.gz"
            URL_MD5 "9084829124e02a7e6be0f0f824523423"
            CONFIGURE_COMMAND ../gflags/configure --prefix=${kinetic_cpp_client_BINARY_DIR}/vendor --enable-static --with-pic
    )
    set(GFLAGS_LIBRARIES ${CMAKE_BINARY_DIR}/vendor/lib/libgflags.a)
    set(GFLAGS_INCLUDE_DIR ${CMAKE_BINARY_DIR}/vendor/include)
    ExternalProject_add(
            glog
            PREFIX ${kinetic_cpp_client_BINARY_DIR}/vendor
            URL "${kinetic_cpp_client_SOURCE_DIR}/tarballs/glog-0.3.3.tar.gz"
            URL_MD5 "a6fd2c22f8996846e34c763422717c18"
            PATCH_COMMAND sh ${kinetic_cpp_client_SOURCE_DIR}/patches/apply-glog-patches.sh ${kinetic_cpp_client_SOURCE_DIR}
            CONFIGURE_COMMAND ../glog/configure --prefix=${kinetic_cpp_client_BINARY_DIR}/vendor --with-gflags=${kinetic_cpp_client_BINARY_DIR}/vendor --enable-static --with-pic
            DEPENDS gflags
    )
    set(GLOG_LIBRARIES ${CMAKE_BINARY_DIR}/vendor/lib/libglog.a)
    set(GLOG_INCLUDE_DIR ${CMAKE_BINARY_DIR}/vendor/include)
else ()
    find_library(GLOG_LIBRARIES glog)
    find_library(GFLAGS_LIBRARIES gflags)
    find_path(GFLAGS_INCLUDE_DIR google/gflags.h)
    find_path(GLOG_INCLUDE_DIR glog/logging.h)
endif ()

if (PTHREAD_LOCKS)
    add_definitions("-DUSE_PTHREAD_LOCKS")
endif ()

add_definitions(-DGOOGLE_STRIP_LOG=2)

set(GENERATED_SOURCES_PATH ${kinetic_cpp_client_SOURCE_DIR}/src/main/generated)

ExternalProject_add(kinetic-proto
        PREFIX proto
        GIT_REPOSITORY https://github.com/Seagate/kinetic-protocol
        GIT_TAG 3.0.0
        UPDATE_COMMAND mkdir -p ${GENERATED_SOURCES_PATH}
        CONFIGURE_COMMAND ""
        BUILD_IN_SOURCE 1
        BUILD_COMMAND sed s/com\\.seagate\\.kinetic\\.proto/com.seagate.kinetic.client.proto/ kinetic.proto > kinetic_client.proto
        INSTALL_COMMAND cp kinetic_client.proto ${GENERATED_SOURCES_PATH}/kinetic_client.proto
        )

add_custom_command(
        COMMENT "Compiling protobuf"
        OUTPUT ${GENERATED_SOURCES_PATH}/kinetic_client.pb.cc ${GENERATED_SOURCES_PATH}/kinetic_client.pb.h
        COMMAND protoc --proto_path=${GENERATED_SOURCES_PATH} --cpp_out=${GENERATED_SOURCES_PATH} ${GENERATED_SOURCES_PATH}/kinetic_client.proto
        COMMAND cp ${GENERATED_SOURCES_PATH}/kinetic_client.pb.h ${kinetic_cpp_client_SOURCE_DIR}/include/kinetic
        DEPENDS kinetic-proto
)
add_custom_target(proto-compile
        DEPENDS ${GENERATED_SOURCES_PATH}/kinetic_client.pb.cc ${GENERATED_SOURCES_PATH}/kinetic_client.pb.h
        )
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${GENERATED_SOURCES_PATH})
set_source_files_properties(${GENERATED_SOURCES_PATH}/kinetic_client.pb.cc PROPERTIES GENERATED TRUE)

# If libunwind is installed it is used by glog and thus has to linked.
# Otherwise glog uses the standard glibc unwinder and there is no dependency.
find_library(LIBUNWIND "unwind")
if (LIBUNWIND)
    set(GLOG_LIBRARIES
            ${GLOG_LIBRARIES}
            ${LIBUNWIND}
            )
endif (LIBUNWIND)

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
if (GOOGLE_STATIC)
    add_dependencies(kinetic_client_static gflags glog)
    add_dependencies(kinetic_client gflags glog)
endif ()

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

# Set lib or lib64 depending on architecture
get_property(LIB64 GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS)
set(LIBSUFFIX "")
if ("${LIB64}" STREQUAL "TRUE" AND "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
    set(LIBSUFFIX 64)
endif ()
message(STATUS "Library installation directory is set to ${CMAKE_INSTALL_PREFIX}/lib${LIBSUFFIX}")

install(TARGETS kinetic_client
        LIBRARY
        DESTINATION lib${LIBSUFFIX})
install(DIRECTORY ${kinetic_cpp_client_SOURCE_DIR}/include/
        DESTINATION include)

# Rule for generating docs
configure_file(${kinetic_cpp_client_SOURCE_DIR}/Doxyfile ${kinetic_cpp_client_BINARY_DIR}/Doxyfile)
add_custom_target(doc
        doxygen ${kinetic_cpp_client_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${kinetic_cpp_client_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
        )
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES docs)

if (BUILD_TEST)
    find_package(Threads REQUIRED)

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
            ${OPENSSL_LIBRARIES}
            ${PROTOBUF_LIBRARIES}
            ${GLOG_LIBRARIES}
            ${GFLAGS_LIBRARIES}
            ${CMAKE_BINARY_DIR}/vendor/src/gtest/libgtest.a
            ${CMAKE_BINARY_DIR}/vendor/src/gmock/libgmock.a
            ${CMAKE_THREAD_LIBS_INIT}
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
            ${OPENSSL_LIBRARIES}
            ${PROTOBUF_LIBRARIES}
            ${GLOG_LIBRARIES}
            ${GFLAGS_LIBRARIES}
            ${CMAKE_BINARY_DIR}/vendor/src/gtest/libgtest.a
            ${CMAKE_BINARY_DIR}/vendor/src/gmock/libgmock.a
            ${CMAKE_THREAD_LIBS_INIT}
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
if (GOOGLE_STATIC)
    set(CPACK_RPM_PACKAGE_REQUIRES "openssl, protobuf")
else ()
    set(CPACK_RPM_PACKAGE_REQUIRES "google-glog, gflags, openssl, protobuf")
endif ()

set(CPACK_GENERATOR "RPM")
include(CPack)
