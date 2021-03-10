# set headers
set(HeaderPath "${csio_SOURCE_DIR}/include/csio")
file(GLOB_RECURSE HeaderFiles 
    ${csio_SOURCE_DIR}/include/*.h
)
file(GLOB_RECURSE SourceFiles 
    ${csio_SOURCE_DIR}/src/*.cpp
)

#set(HeaderFiles ${HeaderPath}/IClient.h
#		   ${HeaderPath}/IServer.h
#		   #${HeaderPath}/config.h
#		   )

# set source files
#set(SourceFiles )
		
# configure Config.h.in
#configure_file("Config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/ConfigImpl.h")

# define library target

add_library(csio MODULE ${HeaderFiles} ${SourceFiles})

target_include_directories(csio PUBLIC ${csio_SOURCE_DIR}/include)
target_link_libraries(csio INTERFACE asio)

