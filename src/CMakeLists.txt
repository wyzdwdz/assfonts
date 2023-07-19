cmake_minimum_required (VERSION 3.16)

set(THIRD_PARTY_LIBS qt::qt)

set(TARGET_SOURCES main.cc
                   main_window.cc
                   task_runner.cc
                   log_highlighter.cc
                   check_window.cc)

set(TARGET_NAME assfonts_gui)

set(BINARY_NAME assfonts)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
						  
add_executable(${TARGET_NAME} ${TARGET_SOURCES})

qt5_add_resources(QT_RESOURCES resources/main.qrc)
target_sources(${TARGET_NAME} PRIVATE ${QT_RESOURCES})

target_link_libraries(${TARGET_NAME} PRIVATE ${THIRD_PARTY_LIBS} assfonts)

target_compile_options(${TARGET_NAME} PRIVATE ${COMPILE_OPTIONS})

target_compile_features(${TARGET_NAME} PRIVATE ${CXX_STD_VER})

set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME ${BINARY_NAME}
                                                WIN32_EXECUTABLE ON
                                                MACOSX_BUNDLE ON)

install(TARGETS ${TARGET_NAME} 
        CONFIGURATIONS Release 
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})