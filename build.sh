#!/bin/bash

BUILD_TYPE=Release

while getopts "b:" arg
do
    case $arg in
        b)
            BUILD_TYPE=$OPTARG
            ;;
        ?)
    exit 1
    ;;
    esac
done
        
conan install . -b missing -s build_type=${BUILD_TYPE} -c tools.system.package_manager:mode=install
cd build
source conanbuild.sh
cmake .. --preset conan-${BUILD_TYPE,,}
cmake --build --preset conan-${BUILD_TYPE,,} --target install -j