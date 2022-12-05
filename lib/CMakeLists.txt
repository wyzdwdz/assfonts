cmake_minimum_required (VERSION 3.21)

set(THIRD_PARTY_LIBS PRIVATE harfbuzz-subset
                             harfbuzz
                             freetype 
                             Boost::boost 
                             Boost::filesystem
                             Boost::locale
                             Boost::serialization
                             Boost::system
                             spdlog::spdlog
                             fmt::fmt)

set(TARGET_SOURCES PRIVATE font_parser.cc
                           ass_parser.cc
                           font_subsetter.cc
                           ass_font_embedder.cc)
						  
add_library(libassfonts STATIC)

target_link_libraries(libassfonts PRIVATE ${THIRD_PARTY_LIBS})

target_include_directories(libassfonts PRIVATE ${PROJECT_SOURCE_DIR}/include
                                               ${VCPKG_INCLUDE_DIR})

target_sources(libassfonts PRIVATE ${TARGET_SOURCES})

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        target_compile_options(libassfonts PRIVATE /W4 /WX /MDd /Od /D_WIN32_WINNT=0x0601)
    elseif(CMAKE_BUILD_TYPE MATCHES "Release")
        target_compile_options(libassfonts PRIVATE /W4 /WX /MT /O2 /D_WIN32_WINNT=0x0601)
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        target_compile_options(libassfonts PRIVATE -Wall -O0 -pthread)
    elseif(CMAKE_BUILD_TYPE MATCHES "Release")
        target_compile_options(libassfonts PRIVATE -Wall -O2 -static -pthread)
    endif()
endif()
