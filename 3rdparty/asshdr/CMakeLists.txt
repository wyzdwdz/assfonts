cmake_minimum_required (VERSION 3.16)

project (asshdr CXX)

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/install)

add_subdirectory(${PROJECT_SOURCE_DIR}/3rdparty)

configure_file(${PROJECT_SOURCE_DIR}/src/ver.h.in ${PROJECT_BINARY_DIR}/src/ver.h)

find_package(pcre2 CONFIG REQUIRED)
find_package(CLI11 CONFIG REQUIRED)

set(THIRD_PARTY_INCLUDE_DIRS ${PCRE2_INCLUDE_DIR}
                             ${CLI11_INCLUDE_DIR})

set(CXX_STD_VER cxx_std_17)

add_subdirectory(${PROJECT_SOURCE_DIR}/lib)
add_subdirectory(${PROJECT_SOURCE_DIR}/src)