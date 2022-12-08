cmake_minimum_required (VERSION 3.21)

project ("assfonts")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(VCPKG_INCLUDE_DIR ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include)

find_package(harfbuzz CONFIG REQUIRED)
find_package(freetype CONFIG REQUIRED)
find_package(Boost REQUIRED filesystem locale serialization system thread)
find_package(spdlog CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(CLI11 CONFIG REQUIRED)
find_package(wxWidgets CONFIG REQUIRED)

add_library(harfbuzz-subset INTERFACE IMPORTED PRIVATE)
find_library(HARFBUZZ_SUBSET_LIBRARY_DEBUG NAMES harfbuzz-subset PATHS "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug" PATH_SUFFIXES lib NO_DEFAULT_PATH REQUIRED)
if(HARFBUZZ_SUBSET_LIBRARY_DEBUG)
    target_link_libraries(harfbuzz-subset INTERFACE $<$<CONFIG:DEBUG>:${HARFBUZZ_SUBSET_LIBRARY_DEBUG}>)
endif()
find_library(HARFBUZZ_SUBSET_LIBRARY_RELEASE NAMES harfbuzz-subset PATHS "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}" PATH_SUFFIXES lib NO_DEFAULT_PATH REQUIRED)
target_link_libraries(harfbuzz-subset INTERFACE $<$<NOT:$<CONFIG:DEBUG>>:${HARFBUZZ_SUBSET_LIBRARY_RELEASE}>)

add_subdirectory(${PROJECT_SOURCE_DIR}/lib)
add_subdirectory(${PROJECT_SOURCE_DIR}/src)
add_subdirectory(${PROJECT_SOURCE_DIR}/gui)