@echo off
SETLOCAL

set VERSION=0.2.0

set INSTALL_DIR=C:\dev\c\lib-common
set LIB_DIR=justengine
set LIB_INSTALL_DIR=%LIB_DIR%-%VERSION%

echo A | xcopy /E /I %LIB_DIR% %INSTALL_DIR%\%LIB_INSTALL_DIR%
"vendor/openssl-3.5.0/bin/openssl.exe" fipsinstall -module "%INSTALL_DIR%/%LIB_INSTALL_DIR%/bin/fips.dll" -out "%INSTALL_DIR%/%LIB_INSTALL_DIR%/conf/openssl/fipsmodule.cnf" -provider_name fips