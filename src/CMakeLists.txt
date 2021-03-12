# set headers
set(HeaderPath "${csio_SOURCE_DIR}/include/csio")
file(GLOB_RECURSE HeaderFiles 
    ${csio_SOURCE_DIR}/include/*.h
)
file(GLOB_RECURSE SourceFiles 
    ${csio_SOURCE_DIR}/src/*.cpp
)

# configure Config.h.in
configure_file("Config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/ConfigImpl.h")

# define library target

add_library(${PROJECT_NAME} MODULE ${HeaderFiles} ${SourceFiles})

target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include/> # for client in subdirectory
                                                   $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}> # for generated files in build mode
                                                   $<INSTALL_INTERFACE:${${PROJECT_NAME}_INCLUDE_INSTALL_DIR}> # for client in install mode
                                            PRIVATE ${HeaderPath}) # for source files

target_link_libraries(${PROJECT_NAME}  asio)

target_compile_definitions(${PROJECT_NAME} PUBLIC
                            ${PROJECT_NAME}=1
                            CSIO_VERSION_MAJOR=${${PROJECT_NAME}_VERSION_MAJOR}
                            CSIO_VERSION_MINOR=${${PROJECT_NAME}_VERSION_MINOR}
                            CSIO_VERSION_PATCH=${${PROJECT_NAME}_VERSION_PATCH})

# Write/install version file
include(CMakePackageConfigHelpers)

set(VersionFile "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}-config-version.cmake")

write_basic_package_version_file(${VersionFile}
                                 VERSION ${${PROJECT_NAME}_VERSION}
                                 COMPATIBILITY AnyNewerVersion)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ConfigImpl.hpp DESTINATION ${${PROJECT_NAME}_INCLUDE_INSTALL_DIR})
install(FILES ${HeaderFiles}                             DESTINATION ${${PROJECT_NAME}_INCLUDE_INSTALL_DIR}/${PROJECT_NAME}/)
install(FILES ${VersionFile}                             DESTINATION ${${PROJECT_NAME}_CMAKE_CONFIG_INSTALL_DIR})
