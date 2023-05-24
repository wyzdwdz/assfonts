cmake_minimum_required (VERSION 3.16)

set(THIRD_PARTY_LIBS imgui)

set(TARGET_SOURCES ImGuiFileDialog.cpp)

set(TARGET_NAME ImGuiFileDialog)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        set(COMPILE_OPTIONS /Od /W4 /MDd)
    elseif(CMAKE_BUILD_TYPE MATCHES "Release")
        set(COMPILE_OPTIONS /O2 /GL /LTCG /MT)
    endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        set(COMPILE_OPTIONS -O0 -Wall -fPIC)
    elseif(CMAKE_BUILD_TYPE MATCHES "Release")
        set(COMPILE_OPTIONS -O3 -Wall -fPIC)
    endif()
endif()

set(CXX_STD_VER cxx_std_17)
						  
add_library(${TARGET_NAME} STATIC)

target_sources(${TARGET_NAME} PRIVATE ${TARGET_SOURCES})

target_link_libraries(${TARGET_NAME} PRIVATE ${THIRD_PARTY_LIBS})

target_include_directories(${TARGET_NAME} PUBLIC . )

target_compile_options(${TARGET_NAME} PRIVATE ${COMPILE_OPTIONS})

target_compile_features(${TARGET_NAME} PRIVATE ${CXX_STD_VER})