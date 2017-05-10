file(GLOB selection_src
    "*.cpp"
)

add_compile_options(
    -std=c++0x		
	-O3
	#-g
	-msse2
    # All warnings
    -Wall 
    # except unused typedefs, which occur in code from boost library
    -Wno-unused-local-typedefs
	#except unused variables in boost system
	-Wno-unused-variable
    -Wno-strict-overflow
    -Wno-error=maybe-uninitialized
)

link_libraries(boost_program_options boost_system boost_filesystem pthread hdf5 hdf5_cpp)

set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
set (CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined -static ${CMAKE_STATIC_LINKER_FLAGS}")

add_executable(selection ${selection_src})
#link_libraries(selection ${selection_src} ${HDF5_LIBS})
link_libraries(selection ${selection_src})