@echo off

set BUILD_TYPE=Release

if "%1" EQU "-b" (set BUILD_TYPE=%~2)

set BUILD_TYPE_LOW=%BUILD_TYPE%
for %%i in (a b c d e f g h i j k l m n o p q r s t u v w x y z)  do call set  BUILD_TYPE_LOW=%%BUILD_TYPE_LOW:%%i=%%i%%

conan profile detect -vquiet
conan export --version 8.0.1 -nr recipes/harfbuzz_expt
conan export --version 5.15.10 -nr recipes/qt5_fix
conan install . -b missing -s build_type=%BUILD_TYPE% -c tools.system.package_manager:mode=install
call .\build\generators\conanbuild.bat
cmake --preset conan-default
cmake --build --preset conan-%BUILD_TYPE_LOW% -j
cmake --install build --prefix install