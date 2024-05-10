cmake_minimum_required (VERSION 3.16)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")

project (assfonts CXX C)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 7)
set(VERSION_PATCH 0)

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

if(LINUX)
    find_package(Qt6WaylandClient CONFIG REQUIRED)
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
