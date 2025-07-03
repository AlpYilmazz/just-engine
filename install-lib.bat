@echo off
SETLOCAL

set VERSION=0.1.1

set INSTALL_DIR=C:\dev\c\lib-common
set LIB_DIR=justengine
set LIB_INSTALL_DIR=%LIB_DIR%-%VERSION%

echo A | xcopy /E /I %LIB_DIR% %INSTALL_DIR%\%LIB_INSTALL_DIR%