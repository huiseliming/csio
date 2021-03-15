cmake_minimum_required(VERSION 3.17)

# root CMakeLists.txt, specifies option and interface library
project(HSLM_CSIO)

set(HSLM_CSIO_VERSION_MAJOR 0 CACHE STRING "major version of csio" FORCE)
set(HSLM_CSIO_VERSION_MINOR 7 CACHE STRING "minor version of csio" FORCE)
set(HSLM_CSIO_VERSION_PATCH 0 CACHE STRING "patch version of csio" FORCE)
set(HSLM_CSIO_VERSION "${HSLM_CSIO_VERSION_MAJOR}.${HSLM_CSIO_VERSION_MINOR}.${HSLM_CSIO_VERSION_PATCH}" CACHE STRING "version of csio" FORCE)

if(WIN32)
    # set _WIN32_WINNT
    if(True)
        add_definitions(-D _WIN32_WINNT=0x0601)
    elseif()
        if(${CMAKE_SYSTEM_VERSION} EQUAL 10) # Windows 10
            add_definitions(-D _WIN32_WINNT=0x0A00)
        elseif(${CMAKE_SYSTEM_VERSION} EQUAL 6.3) # Windows 8.1
            add_definitions(-D _WIN32_WINNT=0x0603)
        elseif(${CMAKE_SYSTEM_VERSION} EQUAL 6.2) # Windows 8
            add_definitions(-D _WIN32_WINNT=0x0602)
        elseif(${CMAKE_SYSTEM_VERSION} EQUAL 6.1) # Windows 7
            add_definitions(-D _WIN32_WINNT=0x0601)
        elseif(${CMAKE_SYSTEM_VERSION} EQUAL 6.0) # Windows Vista
            add_definitions(-D _WIN32_WINNT=0x0600)
        else() # Windows XP (5.1)
            add_definitions(-D _WIN32_WINNT=0x0501)
        endif()
    endif()
endif()

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 可选项
option(HSLM_CSIO_BUILD_TOOLS "build tool?" ON)
option(HSLM_CSIO_BUILD_TESTS "build test?" ON)
option(HSLM_CSIO_BUILD_EXAMPLES "build example?" OFF)

# 配置路径
# 设置debug和release路径
# set(OUTPUT_DIRECTORY_DEBUG   ${CMAKE_BINARY_DIR})
# set(OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/release/)

#设置二进制文件输出路径
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY        ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG   ${OUTPUT_DIRECTORY_DEBUG}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIRECTORY_RELEASE}/lib)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY        ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG   ${OUTPUT_DIRECTORY_DEBUG}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIRECTORY_RELEASE}/lib)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY        ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${OUTPUT_DIRECTORY_DEBUG}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIRECTORY_RELEASE}/bin)

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

# add subdiretories
add_subdirectory(lib)
add_subdirectory(src)


if(HSLM_CSIO_BUILD_TOOLS)
	add_subdirectory(tool)
endif()

if(HSLM_CSIO_BUILD_TESTS)
    message("Build Tests")
    enable_testing()
	add_subdirectory(test)
endif()

if(HSLM_CSIO_BUILD_EXAMPLES)
	add_subdirectory(example)
endif()

# install readme and license
#install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE" "${CMAKE_CURRENT_SOURCE_DIR}/README.md" DESTINATION ${hslm_csio_ADDITIONAL_FILES_INSTALL_DIR})
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/README.md" 
        DESTINATION ${HSLM_CSIO_ADDITIONAL_FILES_INSTALL_DIR})

install(EXPORT hslm_csioTargets 
        DESTINATION ${HSLM_CSIO_CMAKE_CONFIG_INSTALL_DIR}
        FILE hslm_csio-config.cmake)
