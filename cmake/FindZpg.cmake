# This script locates the Zpg library
# ------------------------------------

# define the list of search paths for headers and libraries
set(FIND_ZPG_PATHS
	${ZPG_ROOT}
	$ENV{ZPG_ROOT}
    /usr/local
    /usr)

# find the LibZpg include directory
find_path(ZPG_INCLUDE_DIR Zpg/Zpg.hpp
          PATH_SUFFIXES include
          PATHS ${FIND_ZPG_PATHS})

set(ZPG_FOUND FALSE)
if(ZPG_INCLUDE_DIR)
    # release library
    find_library(ZPG_LIBRARIES
                 NAMES Zpg
                 PATH_SUFFIXES lib64 lib
                 PATHS ${FIND_ZPG_PATHS})
    if (ZPG_LIBRARIES)
        set(ZPG_FOUND TRUE)   
    endif()
endif()

# handle success
if(ZPG_FOUND)
    message(STATUS "Found LibZpg in ${ZPG_INCLUDE_DIR}")
endif()
