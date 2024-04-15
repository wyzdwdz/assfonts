#!/bin/sh

qdbusxml2cpp-qt5 -N -p fcitxqtinputcontextproxyimpl -c FcitxQtInputContextProxyImpl interfaces/org.fcitx.Fcitx.InputContext1.xml -i fcitxqtdbustypes.h -i fcitx5qt5dbusaddons_export.h
qdbusxml2cpp-qt5 -N -p fcitxqtinputmethodproxy -c FcitxQtInputMethodProxy interfaces/org.fcitx.Fcitx.InputMethod1.xml -i fcitxqtdbustypes.h -i fcitx5qt5dbusaddons_export.h
qdbusxml2cpp-qt5 -N -p fcitxqtcontrollerproxy -c FcitxQtControllerProxy interfaces/org.fcitx.Fcitx.Controller1.xml -i fcitxqtdbustypes.h -i fcitx5qt5dbusaddons_export.h
