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
set(plugin_SRCS ${plugin_SRCS} $<TARGET_OBJECTS:Fcitx5Qt6DBusAddons>)
endif()

add_library(fcitx5platforminputcontextplugin-qt6 ${PLUGIN_LIBRARY_TYPE} ${plugin_SRCS})
set_target_properties(fcitx5platforminputcontextplugin-qt6 PROPERTIES
                         OUTPUT_NAME fcitx5platforminputcontextplugin
                         AUTOMOC TRUE
                        )
target_include_directories(fcitx5platforminputcontextplugin-qt6 PRIVATE "${PROJECT_SOURCE_DIR}/common")
if (BUILD_ONLY_PLUGIN AND BUILD_STATIC_PLUGIN)
target_compile_definitions(fcitx5platforminputcontextplugin-qt6 PRIVATE "-DQT_STATICPLUGIN")
endif()

target_compile_definitions(fcitx5platforminputcontextplugin-qt6 PRIVATE "-DFCITX_PLUGIN_DATA_FILE_PATH=\"${CMAKE_CURRENT_BINARY_DIR}/fcitx5.json\"")
if (WITH_FCITX_PLUGIN_NAME)
    # This is not really necessary, but can trigger a cmake rebuild.
    target_compile_definitions(fcitx5platforminputcontextplugin-qt6 PRIVATE "-DFCITX5_QT_WITH_FCITX_NAME")
endif()


target_link_libraries(fcitx5platforminputcontextplugin-qt6
                          Qt6::Core
                          Qt6::Gui
                          Qt6::GuiPrivate
                          Qt6::DBus
                          Qt6::Widgets
                          Fcitx5Qt6::DBusAddons
                          XKBCommon::XKBCommon
                         )
if (ENABLE_X11)
    target_link_libraries(fcitx5platforminputcontextplugin-qt6 XCB::XCB)
    target_compile_definitions(fcitx5platforminputcontextplugin-qt6 PRIVATE "-DENABLE_X11")
endif()

if (ENABLE_QT6_WAYLAND_WORKAROUND)
    target_compile_definitions(fcitx5platforminputcontextplugin-qt6 PRIVATE "-DFCITX_ENABLE_QT6_WAYLAND_WORKAROUND")
    target_link_libraries(fcitx5platforminputcontextplugin-qt6
        Qt6::GuiPrivate
        Qt6::WaylandGlobalPrivate
        Qt6::WaylandClient
        Qt6::WaylandClientPrivate)
endif()

get_target_property(_QT6_QMAKE_EXECUTABLE Qt6::qmake LOCATION)
execute_process(
    COMMAND ${_QT6_QMAKE_EXECUTABLE} -query "QT_INSTALL_PLUGINS"
    RESULT_VARIABLE return_code
    OUTPUT_VARIABLE _QT6PLUGINDIR
)
if(return_code EQUAL 0)
    string(STRIP "${_QT6PLUGINDIR}" _QT6PLUGINDIR)
else()
    message(FATAL_ERROR "QMake Qt6 call failed: ${return_code}")
endif()
set(CMAKE_INSTALL_QT6PLUGINDIR ${_QT6PLUGINDIR} CACHE PATH "Qt6 plugin dir")
install(TARGETS fcitx5platforminputcontextplugin-qt6 DESTINATION ${CMAKE_INSTALL_QT6PLUGINDIR}/platforminputcontexts)
