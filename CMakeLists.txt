cmake_minimum_required (VERSION 3.16)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")

project (assfonts CXX C)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 6)
set(VERSION_PATCH 3)

add_subdirectory(3rdparty)

find_package(harfbuzz CONFIG REQUIRED)
find_package(Freetype REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(Iconv REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(CLI11 CONFIG REQUIRED)
find_package(ghc_filesystem CONFIG REQUIRED)
find_package(pcre2 CONFIG REQUIRED)
find_package(string-view-lite CONFIG REQUIRED)
find_package(Qt6 CONFIG REQUIRED COMPONENTS Core Gui Widgets Concurrent)
find_package(httplib CONFIG REQUIRED)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        set(COMPILE_OPTS /Od /W4 /MDd)
    elseif(CMAKE_BUILD_TYPE MATCHES "Release")
        set(COMPILE_OPTS /O2 /GL /LTCG /W4 /WX /MT)
    endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        set(COMPILE_OPTS -O0 -Wall -fPIC)
    elseif(CMAKE_BUILD_TYPE MATCHES "Release")
        set(COMPILE_OPTS -O3 -Wall -Werror -fPIC)
    endif()
endif()

set(CXX_STD_VER cxx_std_17)

include(GNUInstallDirs)

install(FILES LICENSE NEWS NOTICE
        DESTINATION ${CMAKE_INSTALL_DOCDIR}
        CONFIGURATIONS Release)

if(UNIX)

install(CODE "
    execute_process(COMMAND gzip -kf doc/man/assfonts.1
                    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
")

install(FILES doc/man/assfonts.1.gz
        DESTINATION ${CMAKE_INSTALL_MANDIR}/man1
        CONFIGURATIONS Release)

endif()

add_subdirectory(lib)
add_subdirectory(src)
# add_subdirectory(test)
