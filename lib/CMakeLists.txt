






# asio 
add_library(asio INTERFACE)
file(GLOB_RECURSE HeaderFiles 
    ${CMAKE_CURRENT_SOURCE_DIR}/asio/*.hpp
)
target_sources(asio INTERFACE HeaderFiles)
target_include_directories(asio INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/asio)




