cmake_minimum_required (VERSION 3.21)

set(TARGET_NAME assfonts_gui)

set(THIRD_PARTY_LIBS PRIVATE wx::core
                             wx::base
                             Boost::boost
                             Boost::filesystem
                             Boost::thread
                             spdlog::spdlog)

set(TARGET_SOURCE PRIVATE gui_frame.cc
                          run.cc)

set(RESOURCE_DIR PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/resource)

set(INCLUDE_DIR PRIVATE ${RESOURCE_DIR})

if(WIN32)						  
    add_executable(${TARGET_NAME} WIN32 assfonts_gui.cc)
else()
    add_executable(${TARGET_NAME} assfonts_gui.cc)
endif()

target_sources(${TARGET_NAME} PRIVATE ${TARGET_SOURCE})

target_link_libraries(${TARGET_NAME} PRIVATE ${THIRD_PARTY_LIBS} libassfonts)

target_include_directories(${TARGET_NAME} PRIVATE ${INCLUDE_DIR}
                                                  ${PROJECT_SOURCE_DIR}/include
                                                  ${THIRD_PARTY_INCLUDE_DIRS})

if(CMAKE_BUILD_TYPE MATCHES "Release")
    install(TARGETS ${TARGET_NAME})
endif()