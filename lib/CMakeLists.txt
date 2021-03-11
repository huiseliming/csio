






# asio 
add_library(asio INTERFACE)
#target_sources(asio INTERFACE HeaderFiles)
target_include_directories(asio 
                           INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/asio/asio/include/> 
                                     $<INSTALL_INTERFACE:${${PROJECT_NAME}_INCLUDE_INSTALL_DIR}>)
                                      
file(GLOB_RECURSE HeaderFiles 
    ${CMAKE_CURRENT_SOURCE_DIR}/asio/asio/include/*.hpp
)

install(FILES ${HeaderFiles}
        DESTINATION ${${PROJECT_NAME}_INCLUDE_INSTALL_DIR}/asio)

target_compile_definitions(asio INTERFACE -DASIO_STANDALONE)

        