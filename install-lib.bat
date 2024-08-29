@echo off
SETLOCAL

set INSTALL_DIR=C:\dev\c\lib-common
set LIB_DIR=justengine

echo A | xcopy /E /I %LIB_DIR% %INSTALL_DIR%\%LIB_DIR%