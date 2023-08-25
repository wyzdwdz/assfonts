cmake_minimum_required (VERSION 3.16)

set(THIRD_PARTY_LIBS nowide::nowide
                     rang
                     CLI11::CLI11
                     ghcFilesystem::ghc_filesystem)

set(TARGET_SOURCES main.cc)

set(TARGET_NAME assfonts)

set(OBJECT_NAME assfonts-obj)

if(APPLE)
    list(APPEND TARGET_SOURCES get_app_support_dir.mm)
endif()

add_library(${OBJECT_NAME} OBJECT ${TARGET_SOURCES})

target_link_libraries(${OBJECT_NAME} PRIVATE ${THIRD_PARTY_LIBS} libassfonts)

if(APPLE)
    target_link_options(${OBJECT_NAME} PRIVATE -framework Foundation)
endif()

add_executable(${TARGET_NAME} )

target_link_libraries(${TARGET_NAME} $<$<BOOL:UNIX>:mimalloc-obj> ${OBJECT_NAME})

target_compile_options(${TARGET_NAME} PRIVATE ${COMPILE_OPTIONS})

target_compile_features(${TARGET_NAME} PRIVATE ${CXX_STD_VER})

install(TARGETS ${TARGET_NAME} 
        CONFIGURATIONS Release 
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})