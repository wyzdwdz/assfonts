cmake_minimum_required (VERSION 3.16)

project (asshdr CXX)

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/install)

add_subdirectory(${PROJECT_SOURCE_DIR}/3rdparty)

configure_file(${PROJECT_SOURCE_DIR}/src/ver.h.in ${PROJECT_BINARY_DIR}/src/ver.h)

find_package(PCRE2 REQUIRED)
find_package(CLI11 REQUIRED)

set(THIRD_PARTY_INCLUDE_DIRS ${PCRE2_INCLUDE_DIR}
                             ${CLI11_INCLUDE_DIR})

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        SET(COMPILE_OPTIONS /Od /W4 /MDd)
    elseif(CMAKE_BUILD_TYPE MATCHES "Release")
        SET(COMPILE_OPTIONS /O2 /W4 /WX /MT)
    endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        SET(COMPILE_OPTIONS -O0 -Wall -fPIC)
    elseif(CMAKE_BUILD_TYPE MATCHES "Release")
        SET(COMPILE_OPTIONS -O2 -Wall -Werror -fPIC)
    endif()
endif()

set(CXX_STD_VER cxx_std_17)

add_subdirectory(${PROJECT_SOURCE_DIR}/lib)
add_subdirectory(${PROJECT_SOURCE_DIR}/src)