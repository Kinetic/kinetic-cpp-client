# Try to find glog library
# Once done, this will define
#
# GLOG_FOUND        - system has glog library
# GLOG_INCLUDE_DIRS - the glog include directories
# GLOG_LIBRARIES    - glog libraries

if(GLOG_INCLUDE_DIRS AND GLOG_LIBRARIES)
    set(GLOG_FIND_QUIETLY TRUE)
endif(GLOG_INCLUDE_DIRS AND GLOG_LIBRARIES)

find_path(GLOG_INCLUDE_DIR glog/logging.h
        HINTS
        /usr/include/
        /usr/local/include/
        )

find_library(GLOG_LIBRARY glog
        PATHS /usr/ /usr/local/
        PATH_SUFFIXES lib lib64
        )

set(GLOG_INCLUDE_DIRS ${GLOG_INCLUDE_DIR})
set(GLOG_LIBRARIES ${GLOG_LIBRARY})

# handle the QUIETLY and REQUIRED arguments and set GLOG_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLOG DEFAULT_MSG GLOG_INCLUDE_DIRS GLOG_LIBRARIES)

mark_as_advanced(GLOG_INCLUDE_DIRS GLOG_LIBRARIES)