set(FCITX5_QT_EXTRA_PLUGIN_NAME "")
if (WITH_FCITX_PLUGIN_NAME)
    set(FCITX5_QT_EXTRA_PLUGIN_NAME "\"fcitx\",")
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/fcitx5.json.in" "${CMAKE_CURRENT_BINARY_DIR}/fcitx5.json")

set(plugin_SRCS
    fcitx4watcher.cpp
    fcitx4inputcontextproxy.cpp
    fcitx4inputcontextproxyimpl.cpp
    fcitx4inputmethodproxy.cpp
    hybridinputcontext.cpp
    qfcitxplatforminputcontext.cpp
    fcitxcandidatewindow.cpp
    fcitxtheme.cpp
    font.cpp
    qtkey.cpp
    main.cpp
)

if (BUILD_ONLY_PLUGIN AND BUILD_STATIC_PLUGIN)
set(plugin_SRCS ${plugin_SRCS} $<TARGET_OBJECTS:Fcitx5Qt5DBusAddons>)
endif()

add_library(fcitx5platforminputcontextplugin ${PLUGIN_LIBRARY_TYPE} ${plugin_SRCS})
set_target_properties(fcitx5platforminputcontextplugin PROPERTIES
                         AUTOMOC TRUE
                        )
target_include_directories(fcitx5platforminputcontextplugin PRIVATE ${Qt5Gui_PRIVATE_INCLUDE_DIRS} "${PROJECT_SOURCE_DIR}/common")
if (BUILD_ONLY_PLUGIN AND BUILD_STATIC_PLUGIN)
target_compile_definitions(fcitx5platforminputcontextplugin PRIVATE "-DQT_STATICPLUGIN")
endif()

target_compile_definitions(fcitx5platforminputcontextplugin PRIVATE "-DFCITX_PLUGIN_DATA_FILE_PATH=\"${CMAKE_CURRENT_BINARY_DIR}/fcitx5.json\"")
if (WITH_FCITX_PLUGIN_NAME)
    # This is not really necessary, but can trigger a cmake rebuild.
    target_compile_definitions(fcitx5platforminputcontextplugin PRIVATE "-DFCITX5_QT_WITH_FCITX_NAME")
endif()


target_link_libraries(fcitx5platforminputcontextplugin
                          Qt5::Core
                          Qt5::Gui
                          Qt5::DBus
                          Qt5::Widgets
                          Fcitx5Qt5::DBusAddons
                          XKBCommon::XKBCommon
                         )
if (ENABLE_X11)
    target_link_libraries(fcitx5platforminputcontextplugin XCB::XCB)
    target_compile_definitions(fcitx5platforminputcontextplugin PRIVATE "-DENABLE_X11")
endif()
include(ECMQueryQmake)
query_qmake(_QT5PLUGINDIR QT_INSTALL_PLUGINS)
set(CMAKE_INSTALL_QT5PLUGINDIR ${_QT5PLUGINDIR} CACHE PATH "Qt5 plugin dir")
install(TARGETS fcitx5platforminputcontextplugin DESTINATION ${CMAKE_INSTALL_QT5PLUGINDIR}/platforminputcontexts)
