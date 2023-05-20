cmake_minimum_required (VERSION 3.16)

set(THIRD_PARTY_LIBS harfbuzz::harfbuzz
                     Freetype::Freetype
                     nlohmann_json::nlohmann_json
                     Iconv::Iconv
                     fmt::fmt
                     threadpool::threadpool
                     ced
                     libasshdr)

set(TARGET_SOURCES font_parser.cc
                   ass_parser.cc
                   font_subsetter.cc
                   ass_font_embedder.cc
                   ass_string.cc
                   ass_freetype.cc
                   assfonts.cc)

set(TARGET_NAME assfonts)

configure_file(${PROJECT_SOURCE_DIR}/include/assfonts.h.in ${PROJECT_BINARY_DIR}/include/assfonts.h)
						  
add_library(${TARGET_NAME} OBJECT EXCLUDE_FROM_ALL)

target_sources(${TARGET_NAME} PRIVATE ${TARGET_SOURCES})

target_link_libraries(${TARGET_NAME} PRIVATE ${THIRD_PARTY_LIBS})

target_include_directories(${TARGET_NAME} PUBLIC ${PROJECT_BINARY_DIR}/include)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_options(${TARGET_NAME} PRIVATE -pthread)
endif()

target_compile_options(${TARGET_NAME} PRIVATE ${COMPILE_OPTIONS})

target_compile_features(${TARGET_NAME} PRIVATE ${CXX_STD_VER})

add_library(${TARGET_NAME}_shared SHARED $<TARGET_OBJECTS:${TARGET_NAME}>)
target_link_libraries(${TARGET_NAME}_shared PRIVATE ${THIRD_PARTY_LIBS})
target_sources(${TARGET_NAME}_shared PUBLIC FILE_SET HEADERS
               BASE_DIRS ${PROJECT_BINARY_DIR}/include
               FILES ${PROJECT_BINARY_DIR}/include/assfonts.h)
set_target_properties(${TARGET_NAME}_shared PROPERTIES OUTPUT_NAME ${TARGET_NAME})

if(LINUX)
install(TARGETS ${TARGET_NAME}_shared
        CONFIGURATIONS Release
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
elseif(WIN32)
install(TARGETS ${TARGET_NAME}_shared
        CONFIGURATIONS Release
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()