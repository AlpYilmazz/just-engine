@echo off
SETLOCAL

set COMMAND=%1

set SRC_DIR=src
set BUILD_DIR=target
set LIB_DIR=justengine

if "%COMMAND%" == "clean" (
    @echo on
    @rmdir /s /q %LIB_DIR% 2>nul || exit 0
    @mingw32-make -f Makefile-lib clean
    @echo off
)

@echo on
mingw32-make -f Makefile-lib
@echo off

mkdir %LIB_DIR%
mkdir %LIB_DIR%\include
mkdir %LIB_DIR%\lib
mkdir %LIB_DIR%\bin
mkdir %LIB_DIR%\conf

mkdir %LIB_DIR%\include\openssl
mkdir %LIB_DIR%\lib\openssl
mkdir %LIB_DIR%\conf\openssl
echo A | xcopy /s /e vendor\openssl-3.5.0\include %LIB_DIR%\include\openssl
echo A | xcopy /s /e vendor\openssl-3.5.0\lib64\dynamic %LIB_DIR%\lib\openssl
echo A | xcopy vendor\openssl-3.5.0\bin\*.dll %LIB_DIR%\bin\
"vendor/openssl-3.5.0/bin/openssl.exe" fipsinstall -module "./justengine/bin/fips.dll" -out "./justengine/conf/openssl/fipsmodule.cnf" -provider_name fips

mkdir %LIB_DIR%\include\curl
mkdir %LIB_DIR%\lib\curl
echo A | xcopy /s /e vendor\curl-8.16.0\include %LIB_DIR%\include\curl
echo A | xcopy /s /e vendor\curl-8.16.0\lib %LIB_DIR%\lib\curl
echo A | xcopy vendor\curl-8.16.0\bin\*.dll %LIB_DIR%\bin\

mkdir %LIB_DIR%\include\raylib
mkdir %LIB_DIR%\lib\raylib
echo A | xcopy /s /e vendor\raylib-5.0\include %LIB_DIR%\include\raylib
echo A | xcopy vendor\raylib-5.0\lib\libraylib.a %LIB_DIR%\lib\raylib\

mkdir %LIB_DIR%\include\clay
echo A | xcopy /s /e vendor\clay-0.14\include %LIB_DIR%\include\clay

echo A | xcopy %SRC_DIR%\justengine.h %LIB_DIR%\include\
echo A | xcopy %BUILD_DIR%\libjustengine.a %LIB_DIR%\lib\
