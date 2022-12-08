cmake_minimum_required (VERSION 3.21)

set(THIRD_PARTY_LIBS harfbuzz-subset
                     harfbuzz
                     freetype 
                     Boost::boost 
                     Boost::filesystem
                     Boost::locale
                     Boost::serialization
                     Boost::system
                     spdlog::spdlog
                     fmt::fmt)

set(TARGET_SOURCES font_parser.cc
                   ass_parser.cc
                   font_subsetter.cc
                   ass_font_embedder.cc
                   ass_string.cc
                   ass_freetype.cc)

set(TARGET_NAME libassfonts)
						  
add_library(${TARGET_NAME} STATIC)

target_link_libraries(${TARGET_NAME} PRIVATE ${THIRD_PARTY_LIBS})

target_include_directories(${TARGET_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include
                                                  ${VCPKG_INCLUDE_DIR})

target_sources(${TARGET_NAME} PRIVATE ${TARGET_SOURCES})

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        target_compile_options(${TARGET_NAME} PRIVATE /Od /W4 /WX /MDd /D_WIN32_WINNT=0x0601)
    elseif(CMAKE_BUILD_TYPE MATCHES "Release")
        target_compile_options(${TARGET_NAME} PRIVATE /O2 /W4 /WX /MT /D_WIN32_WINNT=0x0601)
    endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_options(${TARGET_NAME} PRIVATE -no-pie)
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        target_compile_options(${TARGET_NAME} PRIVATE -O0 -Wall -Werror)
    elseif(CMAKE_BUILD_TYPE MATCHES "Release")
        target_compile_options(${TARGET_NAME} PRIVATE -O2 -Wall -Werror)
    endif()
endif()
