add_executable(fcitx5-qt6-immodule-probing main.cpp)
target_include_directories(fcitx5-qt6-immodule-probing PRIVATE ${Qt6Gui_PRIVATE_INCLUDE_DIRS})
target_link_libraries(fcitx5-qt6-immodule-probing Qt6::Gui)

install(TARGETS fcitx5-qt6-immodule-probing DESTINATION "${CMAKE_INSTALL_BINDIR}")