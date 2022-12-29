cmake_minimum_required (VERSION 3.21)

project (assfonts CXX C)

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/install)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(${PROJECT_SOURCE_DIR}/3rdparty)

find_package(harfbuzz REQUIRED)
find_package(freetype REQUIRED)
find_package(Boost REQUIRED)
find_package(Iconv REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)

set(THIRD_PARTY_INCLUDE_DIRS ${harfbuzz_INCLUDE_DIR}
                             ${freetype_INCLUDE_DIR}
                             ${Boost_INCLUDE_DIR}
                             ${Iconv_INCLUDE_DIR}
                             ${spdlog_INCLUDE_DIR}
                             ${fmt_INCLUDE_DIR})

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        SET(COMPILE_OPTIONS /Od /W4 /MDd /D_WIN32_WINNT=0x0601)
    elseif(CMAKE_BUILD_TYPE MATCHES "Release")
        SET(COMPILE_OPTIONS /O2 /W4 /WX /MT /D_WIN32_WINNT=0x0601)
    endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        SET(COMPILE_OPTIONS -O0 -Wall -fPIC)
    elseif(CMAKE_BUILD_TYPE MATCHES "Release")
        SET(COMPILE_OPTIONS -O2 -Wall -Werror -fPIC)
    endif()
endif()

add_subdirectory(${PROJECT_SOURCE_DIR}/lib)
add_subdirectory(${PROJECT_SOURCE_DIR}/src)
add_subdirectory(${PROJECT_SOURCE_DIR}/gui)
# add_subdirectory(${PROJECT_SOURCE_DIR}/test)