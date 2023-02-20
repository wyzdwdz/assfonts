cmake_minimum_required (VERSION 3.16)

set(THIRD_PARTY_LIBS CLI11::CLI11
                     spdlog::spdlog
                     fmt::fmt)

set(TARGET_NAME assfonts)
						  
add_executable(${TARGET_NAME} assfonts.cc)

target_link_libraries(${TARGET_NAME} PRIVATE ${THIRD_PARTY_LIBS} libassfonts)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_options(${TARGET_NAME} PRIVATE -s)
endif()

target_include_directories(${TARGET_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include
                                                  ${PROJECT_BINARY_DIR}/include
                                                  ${THIRD_PARTY_INCLUDE_DIRS})

target_compile_options(${TARGET_NAME} PRIVATE ${COMPILE_OPTIONS})

target_compile_features(${TARGET_NAME} PRIVATE ${CXX_STD_VER})

install(TARGETS ${TARGET_NAME} RUNTIME 
        DESTINATION ${CMAKE_INSTALL_BINDIR} 
        CONFIGURATIONS Release
        COMPONENT ${TARGET_NAME})
