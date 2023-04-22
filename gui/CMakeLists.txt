cmake_minimum_required (VERSION 3.16)

set(TARGET_NAME assfonts_gui)

set(THIRD_PARTY_LIBS wx::core
                     wx::base
                     spdlog::spdlog)

set(TARGET_SOURCE gui_frame.cc)

set(RESOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resource)

if(WIN32)
    LIST(APPEND TARGET_SOURCE ${RESOURCE_DIR}/assfonts_gui.rc
                              ${RESOURCE_DIR}/assfonts_gui.manifest)
endif()

set(INCLUDE_DIR ${RESOURCE_DIR})

add_library(run STATIC EXCLUDE_FROM_ALL run.cc)

target_link_libraries(run PRIVATE libassfonts ${THIRD_PARTY_LIBS})

target_include_directories(run PRIVATE ${PROJECT_SOURCE_DIR}/include
                                       ${THIRD_PARTY_INCLUDE_DIRS})

target_compile_options(run PRIVATE ${COMPILE_OPTIONS})

target_compile_features(run PRIVATE ${CXX_STD_VER})

if(WIN32)						  
    add_executable(${TARGET_NAME} WIN32 assfonts_gui.cc)
else()
    add_executable(${TARGET_NAME} assfonts_gui.cc)
endif()

target_sources(${TARGET_NAME} PRIVATE ${TARGET_SOURCE})

target_link_libraries(${TARGET_NAME} PRIVATE ${THIRD_PARTY_LIBS} run)

target_include_directories(${TARGET_NAME} PRIVATE ${INCLUDE_DIR}
                                                  ${PROJECT_SOURCE_DIR}/include
                                                  ${PROJECT_BINARY_DIR}/include
                                                  ${THIRD_PARTY_INCLUDE_DIRS})
                                                  

target_compile_options(${TARGET_NAME} PRIVATE ${COMPILE_OPTIONS})

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        target_link_options(${TARGET_NAME} PRIVATE -no-pie -pthread)
    elseif(CMAKE_BUILD_TYPE MATCHES "Release")
        target_link_options(${TARGET_NAME} PRIVATE -no-pie -pthread -s)
    endif()
endif()

target_compile_features(${TARGET_NAME} PRIVATE ${CXX_STD_VER})

install(TARGETS ${TARGET_NAME} 
        CONFIGURATIONS Release 
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} 
        COMPONENT ${TARGET_NAME})