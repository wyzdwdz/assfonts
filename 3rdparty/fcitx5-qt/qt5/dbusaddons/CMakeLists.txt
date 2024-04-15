ecm_setup_version(PROJECT VARIABLE_PREFIX FCITX5QT5DBUSADDONS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/fcitx5qt5dbusaddons_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/Fcitx5Qt5DBusAddonsConfigVersion.cmake"
                        SOVERSION 1)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/Fcitx5Qt5DBusAddons")

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/Fcitx5Qt5DBusAddonsConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/Fcitx5Qt5DBusAddonsConfig.cmake"
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

add_library(Fcitx5Qt5DBusAddons ${LIBRARY_TYPE} ${dbusaddons_SOURCES})
generate_export_header(Fcitx5Qt5DBusAddons BASE_NAME Fcitx5Qt5DBusAddons)
add_library(Fcitx5Qt5::DBusAddons ALIAS Fcitx5Qt5DBusAddons)

target_include_directories(Fcitx5Qt5DBusAddons PUBLIC "$<BUILD_INTERFACE:${fcitxqtdbusaddons_INCLUDE_DIRS}>")
target_include_directories(Fcitx5Qt5DBusAddons INTERFACE "$<INSTALL_INTERFACE:${Fcitx5Qt5_INCLUDE_INSTALL_DIR}/Fcitx5Qt5DBusAddons>")

set_target_properties(Fcitx5Qt5DBusAddons
                      PROPERTIES VERSION ${FCITX5QT5DBUSADDONS_VERSION}
                      AUTOMOC TRUE
                      SOVERSION ${FCITX5QT5DBUSADDONS_SOVERSION}
                      POSITION_INDEPENDENT_CODE ON
                      EXPORT_NAME DBusAddons
)

target_link_libraries(
    Fcitx5Qt5DBusAddons
    PUBLIC
        Qt5::Core
        Qt5::DBus
)

if(BUILD_ONLY_PLUGIN)
  set_target_properties(Fcitx5Qt5DBusAddons PROPERTIES
                        COMPILE_DEFINITIONS "FCITX5QT5DBUSADDONS_STATIC_DEFINE")

else()
  install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/Fcitx5Qt5DBusAddonsConfig.cmake"
                "${CMAKE_CURRENT_BINARY_DIR}/Fcitx5Qt5DBusAddonsConfigVersion.cmake"
          DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
          COMPONENT Devel )

  install(EXPORT Fcitx5Qt5DBusAddonsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE Fcitx5Qt5DBusAddonsTargets.cmake NAMESPACE Fcitx5Qt5:: )

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fcitx5qt5dbusaddons_version.h
          DESTINATION ${Fcitx5Qt5_INCLUDE_INSTALL_DIR} COMPONENT Devel )

  install(TARGETS Fcitx5Qt5DBusAddons EXPORT Fcitx5Qt5DBusAddonsTargets LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")

  install(FILES ${dbusaddons_HEADERS}
                ${CMAKE_CURRENT_BINARY_DIR}/fcitx5qt5dbusaddons_export.h
                DESTINATION "${Fcitx5Qt5_INCLUDE_INSTALL_DIR}/Fcitx5Qt5DBusAddons")
endif()


