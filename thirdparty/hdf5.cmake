set(HDF5_PREFIX hdf5)
# download hdf5 lib
set(HDF5_URL https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.10/hdf5-1.10.1/src/hdf5-1.10.1.tar.gz )
# alternatively use manually downloaded source
#set(HDF5_URL ${CMAKE_SOURCE_DIR}/thirdparty/hdf5-1.8.14.tar.gz)
set(HDF5_URL_MD5 43a2f9466702fb1db31df98ae6677f15)

if (WIN32)
    set(HDF5_MAKE gmake)
else (WIN32)
    set(HDF5_MAKE make)
endif (WIN32)

# include hdf lib as external project
ExternalProject_Add(${HDF5_PREFIX}
	PREFIX ${HDF5_PREFIX}
	URL ${HDF5_URL}
	URL_MD5 ${HDF5_URL_MD5}
	CONFIGURE_COMMAND ./configure --prefix=${CMAKE_SOURCE_DIR}/build --enable-cxx
	INSTALL_DIR ${CMAKE_SOURCE_DIR}/build
	BUILD_IN_SOURCE 1
	LOG_DOWNLOAD 1
	LOG_BUILD 1
)

# get the unpacked source directory path
ExternalProject_Get_Property(${HDF5_PREFIX} SOURCE_DIR)
message(STATUS "Source directory of ${HDF5_PREFIX} ${SOURCE_DIR}")

# Set separate directories for building in Debug or Release mode
set(HDF5_DEBUG_DIR ${SOURCE_DIR}/build/${HDF5_PREFIX}_debug)
set(HDF5_RELEASE_DIR ${SOURCE_DIR}/build/${HDF5_PREFIX}_release)
message(STATUS "HDF5 Debug directory ${HDF5_DEBUG_DIR}")
message(STATUS "HDF5 Release directory ${HDF5_RELEASE_DIR}")

# set the include directory variable and include it
set(HDF5_INCLUDE_DIRS ${SOURCE_DIR}/include)
include_directories(${HDF5_INCLUDE_DIRS})

# link the correct HDF5 directory when the project is in Debug or Release mode
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	# in Debug mode
	link_directories(${HDF5_RELEASE_DIR})
	set(TBB_LIBS tbb_debug tbbmalloc_debug)
	set(TBB_LIBRARY_DIRS ${HDF5_DEBUG_DIR})
else (CMAKE_BUILD_TYPE STREQUAL "Debug")
	# in Release mode
	link_directories(${HDF5_RELEASE_DIR})
	set(TBB_LIBS tbb tbbmalloc)
	set(TBB_LIBRARY_DIRS ${HDF5_RELEASE_DIR})
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")

