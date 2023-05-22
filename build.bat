@echo off

set BUILD_TYPE=Release

if "%1" EQU "-b" (set BUILD_TYPE=%~2)

set BUILD_TYPE_LOW=%BUILD_TYPE%
for %%i in (a b c d e f g h i j k l m n o p q r s t u v w x y z)  do call set  BUILD_TYPE_LOW=%%BUILD_TYPE_LOW:%%i=%%i%%

conan install . -b missing -s build_type=%BUILD_TYPE% -c tools.system.package_manager:mode=install
cd build
call conanbuild.bat
cmake .. --preset conan-default -DCMAKE_INSTALL_PREFIX=../install
cmake --build --preset conan-%BUILD_TYPE_LOW% --target install -j