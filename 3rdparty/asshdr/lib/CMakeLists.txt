cmake_minimum_required (VERSION 3.16)

set(THIRD_PARTY_LIBS pcre2::pcre2)

set(TARGET_SOURCES ass_recolorize.cc)

set(TARGET_NAME libasshdr)
						  
add_library(${TARGET_NAME} STATIC)

target_sources(${TARGET_NAME} PRIVATE ${TARGET_SOURCES})

target_link_libraries(${TARGET_NAME} PRIVATE ${THIRD_PARTY_LIBS})

target_include_directories(${TARGET_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_compile_options(${TARGET_NAME} PRIVATE ${COMPILE_OPTIONS})

target_compile_features(${TARGET_NAME} PRIVATE ${CXX_STD_VER})

set_target_properties(${TARGET_NAME} PROPERTIES PREFIX "")