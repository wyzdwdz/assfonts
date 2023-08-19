#!/bin/bash

BUILD_TYPE=Release

INSTALL_PREFIX=install

while getopts "b:o:" arg
do
    case $arg in
        b)
            BUILD_TYPE=$OPTARG
            ;;
        o)
            INSTALL_PREFIX=$OPTARG
            ;;
        ?)
    exit 1
    ;;
    esac
done

conan profile detect -vquiet
conan export --version 8.0.1 -nr recipes/harfbuzz_expt
conan export --version 5.15.10 -nr recipes/qt5_fix

if [ "$(uname)" = "Linux" ]; then
    conan export --version system -nr recipes/dbus_system
fi

if (( $EUID != 0 ))
then        
    conan install . -b missing -s build_type=${BUILD_TYPE} -c tools.system.package_manager:mode=install -c tools.system.package_manager:sudo=True
else
    conan install . -b missing -s build_type=${BUILD_TYPE} -c tools.system.package_manager:mode=install -c tools.system.package_manager:sudo=False
fi

source ./build/${BUILD_TYPE}/generators/conanbuild.sh
cmake --preset conan-$(tr '[:upper:]' '[:lower:]' <<< "$BUILD_TYPE")
cmake --build --preset conan-$(tr '[:upper:]' '[:lower:]' <<< "$BUILD_TYPE") -j
cmake --install build/${BUILD_TYPE} --prefix ${INSTALL_PREFIX} --strip
