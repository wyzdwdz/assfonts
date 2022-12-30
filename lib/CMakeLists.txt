cmake_minimum_required (VERSION 3.16)

set(THIRD_PARTY_LIBS harfbuzz::harfbuzz
                     freetype 
                     Boost::serialization
                     Iconv::Iconv
                     spdlog::spdlog
                     fmt::fmt
                     ced)

set(TARGET_SOURCES font_parser.cc
                   ass_parser.cc
                   font_subsetter.cc
                   ass_font_embedder.cc
                   ass_string.cc
                   ass_freetype.cc
                   ass_threadpool.cc)

set(TARGET_NAME libassfonts)
						  
add_library(${TARGET_NAME} STATIC)

target_link_libraries(${TARGET_NAME} PRIVATE ${THIRD_PARTY_LIBS})

target_include_directories(${TARGET_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include
                                          PRIVATE ${THIRD_PARTY_INCLUDE_DIRS})

target_sources(${TARGET_NAME} PRIVATE ${TARGET_SOURCES})

if(UNIX)
    target_link_options(${TARGET_NAME} PRIVATE -pthread)
endif()

target_compile_options(${TARGET_NAME} PRIVATE ${COMPILE_OPTIONS})

target_compile_features(${TARGET_NAME} PRIVATE ${CXX_STD_VER})