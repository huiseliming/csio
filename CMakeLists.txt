cmake_minimum_required(VERSION 3.17)


project(csio)

# define library version (update: apparently you can also do it in project()!)
set(MESSAGE_SERVICE_VERSION_MAJOR 1 CACHE STRING "major version" FORCE)
set(MESSAGE_SERVICE_VERSION_MINOR 0 CACHE STRING "minor version" FORCE)
set(MESSAGE_SERVICE_VERSION ${MY_LIBRARY_VERSION_MAJOR}.${MY_LIBRARY_VERSION_MINOR} CACHE STRING "version" FORCE)

# some options
option(BUILD_TOOL "build tool?" ON)
option(BUILD_TEST "build test?" OFF)
option(BUILD_EXAMPLE "build example?" OFF)

# add subdiretories
add_subdirectory(lib)
add_subdirectory(src)

if(BUILD_TOOL)
	add_subdirectory(tool)
endif()

if(BUILD_TEST)
	add_subdirectory(test)
endif()

if(BUILD_EXAMPLE)
	add_subdirectory(example)
endif()