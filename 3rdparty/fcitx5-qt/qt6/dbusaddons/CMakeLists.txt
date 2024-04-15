ecm_setup_version(PROJECT VARIABLE_PREFIX FCITX5QT6DBUSADDONS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/fcitx5qt6dbusaddons_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/Fcitx5Qt6DBusAddonsConfigVersion.cmake"
                        SOVERSION 1)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/Fcitx5Qt6DBusAddons")

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/Fcitx5Qt6DBusAddonsConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/Fcitx5Qt6DBusAddonsConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              )

set(dbusaddons_SOURCES
    fcitxqtwatcher.cpp
    fcitxqtdbustypes.cpp
    fcitxqtinputcontextproxy.cpp
    fcitxqtinputcontextproxyimpl.cpp
    fcitxqtinputmethodproxy.cpp
    fcitxqtcontrollerproxy.cpp
    )

set(dbusaddons_HEADERS
    fcitxqtwatcher.h
    fcitxqtdbustypes.h
    fcitxqtinputcontextproxy.h
    fcitxqtinputmethodproxy.h
    fcitxqtcontrollerproxy.h
)

set(fcitxqtdbusaddons_INCLUDE_DIRS
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(Fcitx5Qt6DBusAddons ${LIBRARY_TYPE} ${dbusaddons_SOURCES})
generate_export_header(Fcitx5Qt6DBusAddons BASE_NAME Fcitx5Qt6DBusAddons)
add_library(Fcitx5Qt6::DBusAddons ALIAS Fcitx5Qt6DBusAddons)

target_include_directories(Fcitx5Qt6DBusAddons PUBLIC "$<BUILD_INTERFACE:${fcitxqtdbusaddons_INCLUDE_DIRS}>")
target_include_directories(Fcitx5Qt6DBusAddons INTERFACE "$<INSTALL_INTERFACE:${Fcitx5Qt6_INCLUDE_INSTALL_DIR}/Fcitx5Qt6DBusAddons>")

set_target_properties(Fcitx5Qt6DBusAddons
                      PROPERTIES VERSION ${FCITX5QT6DBUSADDONS_VERSION}
                      AUTOMOC TRUE
                      SOVERSION ${FCITX5QT6DBUSADDONS_SOVERSION}
                      POSITION_INDEPENDENT_CODE ON
                      EXPORT_NAME DBusAddons
)

target_link_libraries(
    Fcitx5Qt6DBusAddons
    PUBLIC
        Qt6::Core
        Qt6::DBus
)

if(BUILD_ONLY_PLUGIN)
  set_target_properties(Fcitx5Qt6DBusAddons PROPERTIES
                        COMPILE_DEFINITIONS "FCITX5QT6DBUSADDONS_STATIC_DEFINE")

else()
  install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/Fcitx5Qt6DBusAddonsConfig.cmake"
                "${CMAKE_CURRENT_BINARY_DIR}/Fcitx5Qt6DBusAddonsConfigVersion.cmake"
          DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
          COMPONENT Devel )

  install(EXPORT Fcitx5Qt6DBusAddonsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE Fcitx5Qt6DBusAddonsTargets.cmake NAMESPACE Fcitx5Qt6:: )

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fcitx5qt6dbusaddons_version.h
          DESTINATION ${Fcitx5Qt6_INCLUDE_INSTALL_DIR} COMPONENT Devel )

  install(TARGETS Fcitx5Qt6DBusAddons EXPORT Fcitx5Qt6DBusAddonsTargets LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")

  install(FILES ${dbusaddons_HEADERS}
                ${CMAKE_CURRENT_BINARY_DIR}/fcitx5qt6dbusaddons_export.h
                DESTINATION "${Fcitx5Qt6_INCLUDE_INSTALL_DIR}/Fcitx5Qt6DBusAddons")
endif()


