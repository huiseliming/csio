

# set output directory
macro(SET_WIN32_WINNT)
    if(WIN32)
        # set _WIN32_WINNT
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
endmacro()



# set output directory
macro(SET_OUTPUT_DIRECTORY output_directory)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${output_directory}/lib)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${output_directory}/lib)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${output_directory}/bin)
endmacro()

# set default build type
macro(SET_DEFAULT_BUILD_TYPE)
    set(VALID_BUILD_TYPES "Release" "Debug" "MinSizeRel" "RelWithDebInfo")
    if(NOT CMAKE_CONFIGURATION_TYPES)
        # set default build type if not set 
        if("${CMAKE_BUILD_TYPE}" STREQUAL "")
            SET(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build configuration" FORCE)
        endif()

        # check build type 
        list(FIND VALID_BUILD_TYPES "${CMAKE_BUILD_TYPE}" INDEX)
        if(${INDEX} MATCHES -1)
            message(FATAL_ERROR "Invalid build type")
        endif()

        # set CMAKE_BUILD_TYPE as dropdown list for cmake gui
        if(DEFINED CMAKE_BUILD_TYPE)
            set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${VALID_BUILD_TYPES})
        endif()
    endif()
endmacro()

# set target postfix
macro(SET_TARGET_POSTFIX target_name)
    #rename static-library
    if(NOT BUILD_SHARED_LIBS)
        set(STATIC_POSTFIX "-static")
    endif()

    set_target_properties(${target_name} PROPERTIES
        RELEASE_POSTFIX "${STATIC_POSTFIX}"
        MINSIZEREL_POSTFIX "${STATIC_POSTFIX}-minsizerel"
        RELWITHDEBINFO_POSTFIX "${STATIC_POSTFIX}-relwithdebinfo"
        DEBUG_POSTFIX "${STATIC_POSTFIX}-debug"
        )

endmacro()