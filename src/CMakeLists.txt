# set headers
set(HeaderPath "${csio_SOURCE_DIR}/include/my_library")
set(HeaderFiles ${HeaderPath}/IClient.h
		   ${HeaderPath}/IServer.h
		   ${HeaderPath}/config.h
		   )

# set source files
set(SourceFiles source-a.cpp
		source-b.cpp
		...)
		
# configure Config.h.in
configure_file("Config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/ConfigImpl.h")

# define library target
add_library(csio ${HeaderFiles} ${SourceFiles})
target_include_directories(csio PUBLIC ${csio_SOURCE_DIR}/include)
