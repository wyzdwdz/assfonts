cmake_minimum_required (VERSION 3.16)

set(THIRD_PARTY_LIBS nowide::nowide
                     CLI11::CLI11)

set(TARGET_NAME asshdr)
						  
add_executable(${TARGET_NAME} asshdr.cc)

target_link_libraries(${TARGET_NAME} PRIVATE ${THIRD_PARTY_LIBS} libasshdr)

target_include_directories(${TARGET_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include
                                                  ${PROJECT_BINARY_DIR}/src
                                                  ${THIRD_PARTY_INCLUDE_DIRS})

target_compile_options(${TARGET_NAME} PRIVATE ${COMPILE_OPTIONS})

target_compile_features(${TARGET_NAME} PRIVATE ${CXX_STD_VER})

install(TARGETS ${TARGET_NAME} RUNTIME 
        DESTINATION ${CMAKE_INSTALL_BINDIR} 
        CONFIGURATIONS Release
        COMPONENT ${TARGET_NAME})