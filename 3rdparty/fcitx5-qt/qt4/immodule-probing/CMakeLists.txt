add_executable(fcitx5-qt4-immodule-probing main.cpp)
target_link_libraries(fcitx5-qt4-immodule-probing Qt4::QtGui)

install(TARGETS fcitx5-qt4-immodule-probing DESTINATION "${CMAKE_INSTALL_BINDIR}")