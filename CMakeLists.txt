cmake_minimum_required (VERSION 3.16)

project (assfonts CXX C)

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/install)

add_subdirectory(${PROJECT_SOURCE_DIR}/3rdparty)

find_package(harfbuzz REQUIRED)
find_package(freetype REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Iconv REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(CLI11 REQUIRED)

# set(VERSION_MAJOR 0)
# set(VERSION_MINOR 0)
# set(VERSION_PATCH 0)

configure_file(${PROJECT_SOURCE_DIR}/include/ver.h.in ${PROJECT_BINARY_DIR}/include/ver.h)

set(THIRD_PARTY_INCLUDE_DIRS ${harfbuzz_INCLUDE_DIR}
                             ${freetype_INCLUDE_DIR}
                             ${nlohmann_json_INCLUDE_DIR}
                             ${Iconv_INCLUDE_DIR}
                             ${spdlog_INCLUDE_DIR}
                             ${fmt_INCLUDE_DIR}
                             ${CLI11_INCLUDE_DIR})

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        set(COMPILE_OPTIONS /Od /W4 /MDd /D_WIN32_WINNT=0x0601)
    elseif(CMAKE_BUILD_TYPE MATCHES "Release")
        set(COMPILE_OPTIONS /O2 /W4 /WX /MT /D_WIN32_WINNT=0x0601)
    endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        set(COMPILE_OPTIONS -O0 -Wall -fPIC)
    elseif(CMAKE_BUILD_TYPE MATCHES "Release")
        set(COMPILE_OPTIONS -O2 -Wall -Werror -fPIC)
    endif()
endif()

set(CXX_STD_VER cxx_std_17)

add_subdirectory(${PROJECT_SOURCE_DIR}/lib)
add_subdirectory(${PROJECT_SOURCE_DIR}/cli)
add_subdirectory(${PROJECT_SOURCE_DIR}/gui)
# add_subdirectory(${PROJECT_SOURCE_DIR}/test)