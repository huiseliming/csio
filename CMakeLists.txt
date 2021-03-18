cmake_minimum_required(VERSION 3.17)

# root CMakeLists.txt, specifies option and interface library
project(HSLM_CSIO)

set(HSLM_CSIO_VERSION_MAJOR 0 CACHE STRING "major version of csio" FORCE)
set(HSLM_CSIO_VERSION_MINOR 7 CACHE STRING "minor version of csio" FORCE)
set(HSLM_CSIO_VERSION_PATCH 0 CACHE STRING "patch version of csio" FORCE)
set(HSLM_CSIO_VERSION "${HSLM_CSIO_VERSION_MAJOR}.${HSLM_CSIO_VERSION_MINOR}.${HSLM_CSIO_VERSION_PATCH}" CACHE STRING "version of csio" FORCE)

include(cmake/tools.cmake)
# SET_WIN32_WINNT
#SET_WIN32_WINNT()
# Windows7 WINNT
add_definitions(-D _WIN32_WINNT=0x0601)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 可选项
option(HSLM_CSIO_BUILD_TOOLS "build tools?" ON)
option(HSLM_CSIO_BUILD_TESTS "build tests?" ON)
option(HSLM_CSIO_BUILD_EXAMPLES "build examples?" OFF)

# 配置路径
# 设置debug和release路径
# set(OUTPUT_DIRECTORY_DEBUG   ${CMAKE_BINARY_DIR})
# set(OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/release/)

#设置二进制文件输出路径
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY        ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY        ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY        ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)

#设置安装目标路径
if(UNIX AND APPLE)
    include(GNUInstallDirs)
    set(HSLM_CSIO_INCLUDE_INSTALL_DIR   "${CMAKE_INSTALL_INCLUDEDIR}/hslm_csio")
    set(HSLM_CSIO_RUNTIME_INSTALL_DIR   "${CMAKE_INSTALL_BINDIR}")
    set(HSLM_CSIO_LIBRARY_INSTALL_DIR   "${CMAKE_INSTALL_LIBDIR}")
    set(HSLM_CSIO_ARCHIVE_INSTALL_DIR   "${CMAKE_INSTALL_LIBDIR}")
    set(HSLM_CSIO_FRAMEWORK_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}")

    set(HSLM_CSIO_CMAKE_CONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/hslm_csio/cmake")
    set(HSLM_CSIO_ADDITIONAL_FILES_INSTALL_DIR "${CMAKE_INSTALL_DATADIR}/hslm_csio")
elseif(WIN32)
    set(HSLM_CSIO_INCLUDE_INSTALL_DIR   "include/hslm_csio")
    set(HSLM_CSIO_RUNTIME_INSTALL_DIR   "bin")
    set(HSLM_CSIO_LIBRARY_INSTALL_DIR   "bin")
    set(HSLM_CSIO_ARCHIVE_INSTALL_DIR   "lib")
    set(HSLM_CSIO_FRAMEWORK_INSTALL_DIR "bin")

    set(HSLM_CSIO_CMAKE_CONFIG_INSTALL_DIR "share/hslm_csio/cmake")
    set(HSLM_CSIO_ADDITIONAL_FILES_INSTALL_DIR "share/hslm_csio")
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
else()
    message(FATAL_ERROR "Could not set install folders for this platform!")
endif()

# Git Version Control
# configure git version control
set(PRE_CONFIGURE_FILE "src/CsioConfig.h.in")
set(POST_CONFIGURE_FILE "include/hslm_csio/CsioConfig.h")
include(cmake/git_watcher.cmake)

# add subdiretories
add_subdirectory(lib)
add_subdirectory(src)

if(HSLM_CSIO_BUILD_TOOLS)
    message("[HSLM_CSIO] Build tools")
	add_subdirectory(tool)
endif()

if(HSLM_CSIO_BUILD_TESTS)
    message("[HSLM_CSIO] Build tests")
    enable_testing()
	add_subdirectory(test)
endif()

if(HSLM_CSIO_BUILD_EXAMPLES)
    message("[HSLM_ASIO] Build examples")
	add_subdirectory(example)
endif()

# install readme and license
#install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE" "${CMAKE_CURRENT_SOURCE_DIR}/README.md" DESTINATION ${hslm_csio_ADDITIONAL_FILES_INSTALL_DIR})
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/README.md" 
        DESTINATION ${HSLM_CSIO_ADDITIONAL_FILES_INSTALL_DIR})

install(EXPORT hslm_csioTargets 
        DESTINATION ${HSLM_CSIO_CMAKE_CONFIG_INSTALL_DIR}
        FILE hslm_csio-config.cmake)
