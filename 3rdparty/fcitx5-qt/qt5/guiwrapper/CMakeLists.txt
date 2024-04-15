set(QT_WRAPPER_SRCS
  main.cpp
  wrapperapp.cpp
  mainwindow.cpp
  )

add_executable(fcitx5-qt5-gui-wrapper ${QT_WRAPPER_SRCS})

set_target_properties(fcitx5-qt5-gui-wrapper
  PROPERTIES AUTOMOC TRUE AUTOUIC TRUE AUTOUIC_OPTIONS "-tr=fcitx::tr2fcitx;--include=fcitxqti18nhelper.h")

target_link_libraries(fcitx5-qt5-gui-wrapper
      Fcitx5::Utils
      Qt5::Core
      Qt5::Gui
      Qt5::Widgets
      Fcitx5Qt5::DBusAddons
      Fcitx5Qt5::WidgetsAddons
)

install(TARGETS fcitx5-qt5-gui-wrapper DESTINATION "${CMAKE_INSTALL_LIBEXECDIR}")
configure_file(org.fcitx.fcitx5-qt5-gui-wrapper.desktop.in.in ${CMAKE_CURRENT_BINARY_DIR}/org.fcitx.fcitx5-qt5-gui-wrapper.desktop.in @ONLY)
fcitx5_translate_desktop_file(${CMAKE_CURRENT_BINARY_DIR}/org.fcitx.fcitx5-qt5-gui-wrapper.desktop.in
  org.fcitx.fcitx5-qt5-gui-wrapper.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.fcitx.fcitx5-qt5-gui-wrapper.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
