# Try to find gflags library
# Once done, this will define
#
# GFLAGS_FOUND        - system has gflags library
# GFLAGS_INCLUDE_DIRS - the gflags include directories
# GFLAGS_LIBRARIES    - gflags libraries

if(GFLAGS_INCLUDE_DIRS AND GFLAGS_LIBRARIES)
    set(GFLAGS_FIND_QUIETLY TRUE)
endif(GFLAGS_INCLUDE_DIRS AND GFLAGS_LIBRARIES)

find_path(GFLAGS_INCLUDE_DIR gflags/gflags.h
        HINTS
        /usr/include/
        /usr/local/include/
        )

find_library(GFLAGS_LIBRARY gflags
        PATHS /usr/ /usr/local/
        PATH_SUFFIXES lib lib64
        )

set(GFLAGS_INCLUDE_DIRS ${GFLAGS_INCLUDE_DIR})
set(GFLAGS_LIBRARIES ${GFLAGS_LIBRARY})

# handle the QUIETLY and REQUIRED arguments and set GFLAGS_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GFLAGS DEFAULT_MSG GFLAGS_INCLUDE_DIRS GFLAGS_LIBRARIES)

mark_as_advanced(GFLAGS_INCLUDE_DIRS GFLAGS_LIBRARIES)