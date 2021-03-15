# set headers
set(HeaderPath "${HSLM_CSIO_SOURCE_DIR}/include/hslm_csio")
file(GLOB_RECURSE HeaderFiles 
    ${HSLM_CSIO_SOURCE_DIR}/include/*.h
)
file(GLOB_RECURSE SourceFiles 
    ${HSLM_CSIO_SOURCE_DIR}/src/*.cpp
)

# configure Config.h.in
configure_file("Config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/ConfigImpl.h")

# define library target

add_library(hslm_csio ${HeaderFiles} ${SourceFiles})

target_include_directories(hslm_csio PUBLIC $<BUILD_INTERFACE:${HSLM_CSIO_SOURCE_DIR}/include/> # for client in subdirectory
                                            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}> # for generated files in build mode
                                            $<INSTALL_INTERFACE:${HSLM_CSIO_INCLUDE_INSTALL_DIR}> # for client in install mode
                                            PRIVATE ${HeaderPath}) # for source files

target_compile_definitions(hslm_csio PUBLIC
                           HSLM_CSIO=1
                           HSLM_CSIO_VERSION_MAJOR=${HSLM_CSIO_VERSION_MAJOR}
                           HSLM_CSIO_VERSION_MINOR=${HSLM_CSIO_VERSION_MINOR}
                           HSLM_CSIO_VERSION_PATCH=${HSLM_CSIO_VERSION_PATCH})

set_target_properties(hslm_csio PROPERTIES
                           OUTPUT_NAME "hslm_csio-${FOONATHAN_MEMORY_VERSION}"
                           POSITION_INDEPENDENT_CODE ON)

target_link_libraries(hslm_csio asio)

install(TARGETS hslm_csio asio EXPORT hslm_csioTargets 
        RUNTIME   DESTINATION ${HSLM_CSIO_RUNTIME_INSTALL_DIR}
        LIBRARY   DESTINATION ${HSLM_CSIO_LIBRARY_INSTALL_DIR}
        ARCHIVE   DESTINATION ${HSLM_CSIO_ARCHIVE_INSTALL_DIR}
        FRAMEWORK DESTINATION ${HSLM_CSIO_FRAMEWORK_INSTALL_DIR})

# Write/install version file
include(CMakePackageConfigHelpers)
set(VersionFile "${CMAKE_CURRENT_BINARY_DIR}/cmake/hslm_csio-config-version.cmake")
write_basic_package_version_file(${VersionFile}
                                 VERSION ${HSLM_CSIO_VERSION}
                                 COMPATIBILITY AnyNewerVersion)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ConfigImpl.hpp DESTINATION ${HSLM_CSIO_INCLUDE_INSTALL_DIR})
install(FILES ${HeaderFiles}                             DESTINATION ${HSLM_CSIO_INCLUDE_INSTALL_DIR}/hslm_csio/)
install(FILES ${VersionFile}                             DESTINATION ${HSLM_CSIO_CMAKE_CONFIG_INSTALL_DIR})
