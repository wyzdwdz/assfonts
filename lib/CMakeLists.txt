cmake_minimum_required (VERSION 3.16)

set(THIRD_PARTY_LIBS harfbuzz::harfbuzz-subset
                     Freetype::Freetype
                     nlohmann_json::nlohmann_json
                     Iconv::Iconv
                     fmt::fmt
                     ced
                     libasshdr
                     ghcFilesystem::ghc_filesystem
                     nonstd::string-view-lite)

set(TARGET_SOURCES font_parser.cc
                   ass_parser.cc
                   font_subsetter.cc
                   ass_font_embedder.cc
                   ass_string.cc
                   ass_utf8.cc
                   ass_freetype.cc
                   assfonts.cc)

set(TARGET_NAME libassfonts)

if(APPLE)
    list(APPEND TARGET_SOURCES get_home_dir_macos.mm)
endif()

configure_file(${PROJECT_SOURCE_DIR}/include/assfonts.h.in ${PROJECT_BINARY_DIR}/include/assfonts.h)
						  
add_library(${TARGET_NAME} STATIC EXCLUDE_FROM_ALL)

target_sources(${TARGET_NAME} PRIVATE ${TARGET_SOURCES})

target_link_libraries(${TARGET_NAME} PRIVATE ${THIRD_PARTY_LIBS})

if(APPLE)
    target_link_options(${TARGET_NAME} PRIVATE -framework Foundation)
endif()

target_include_directories(${TARGET_NAME} PUBLIC ${PROJECT_BINARY_DIR}/include)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_link_options(${TARGET_NAME} PRIVATE -pthread)
endif()

target_compile_features(${TARGET_NAME} PRIVATE ${CXX_STD_VER})

set_target_properties(${TARGET_NAME} PROPERTIES PREFIX "")