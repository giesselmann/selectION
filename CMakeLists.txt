cmake_minimum_required(VERSION 2.8)

project(selection)

# required for unit testing using CMake's ctest command
include(CTest)
enable_testing()

# required modules
include(CheckIncludeFile)
include(CheckIncludeFileCXX)
include(CheckIncludeFiles)
include(ExternalProject)

find_package(Boost
    1.46
    REQUIRED
    COMPONENTS
        filesystem
        program_options
        system
)
find_package(Threads 
	REQUIRED
)

# thirdparty modules
include(thirdparty/hdf5.cmake)

# add the files in the include directory to be included
# while compiling all the source code
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/build/include)

# link against all installed thirdparty libs
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/build/lib)

# add the sub-directories
add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(thirdparty)

# install
install (
  FILES ${CMAKE_CURRENT_SOURCE_DIR}/build/src/selection
  DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
  PERMISSIONS OWNER_READ 
			  OWNER_WRITE 
			  OWNER_EXECUTE 
			  GROUP_READ 
			  GROUP_EXECUTE 
			  WORLD_READ 
			  WORLD_EXECUTE
)


