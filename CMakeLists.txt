cmake_minimum_required (VERSION 3.16)

project (assfonts CXX C)

add_subdirectory(3rdparty)

find_package(harfbuzz REQUIRED)
find_package(freetype REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Iconv REQUIRED)
find_package(fmt REQUIRED)
find_package(CLI11 REQUIRED)
find_package(threadpool REQUIRED)
find_package(ghc_filesystem REQUIRED)
find_package(PCRE2 REQUIRED)
find_package(Qt5 REQUIRED)
find_package(httplib REQUIRED)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        set(COMPILE_OPTIONS /Od /W4 /MDd)
    elseif(CMAKE_BUILD_TYPE MATCHES "Release")
        set(COMPILE_OPTIONS /O2 /GL /LTCG /W4 /WX /MT)
    endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        set(COMPILE_OPTIONS -O0 -Wall -fPIC)
    elseif(CMAKE_BUILD_TYPE MATCHES "Release")
        set(COMPILE_OPTIONS -O3 -Wall -Werror -fPIC)
    endif()
endif()

set(CXX_STD_VER cxx_std_11)

install(FILES LICENSE NEWS NOTICE
        DESTINATION ${CMAKE_INSTALL_DOCDIR}
        CONFIGURATIONS Release)

add_subdirectory(lib)
add_subdirectory(src)
# add_subdirectory(test)
