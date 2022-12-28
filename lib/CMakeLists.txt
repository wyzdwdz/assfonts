cmake_minimum_required (VERSION 3.21)

set(THIRD_PARTY_LIBS harfbuzz::harfbuzz
                     freetype 
                     Boost::boost 
                     Boost::filesystem
                     Boost::serialization
                     Boost::thread
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

target_compile_options(${TARGET_NAME} PRIVATE ${COMPILE_OPTIONS})
