@echo off
SETLOCAL

set COMMAND=%1

set SRC_DIR=src
set BUILD_DIR=target
set LIB_DIR=justengine

if "%COMMAND%" == "clean" (
    @echo on
    @rmdir /s /q %LIB_DIR% 2>nul || exit 0
    mingw32-make -f Makefile-lib clean
    @echo off
)

@echo on
mingw32-make -f Makefile-lib
@echo off

mkdir %LIB_DIR%             >nul 2>&1
mkdir %LIB_DIR%\include     >nul 2>&1
mkdir %LIB_DIR%\lib         >nul 2>&1
mkdir %LIB_DIR%\bin         >nul 2>&1
mkdir %LIB_DIR%\conf        >nul 2>&1

mkdir %LIB_DIR%\include\openssl     >nul 2>&1
mkdir %LIB_DIR%\lib\openssl         >nul 2>&1
mkdir %LIB_DIR%\conf\openssl        >nul 2>&1
echo A | xcopy /s /e /q vendor\openssl-3.5.0\include %LIB_DIR%\include\openssl      >nul 2>&1
echo A | xcopy /s /e /q vendor\openssl-3.5.0\lib64\dynamic %LIB_DIR%\lib\openssl    >nul 2>&1
echo A | xcopy /q vendor\openssl-3.5.0\bin\*.dll %LIB_DIR%\bin\                     >nul 2>&1
"vendor/openssl-3.5.0/bin/openssl.exe" fipsinstall -module "./justengine/bin/fips.dll" -out "./justengine/conf/openssl/fipsmodule.cnf" -provider_name fips

mkdir %LIB_DIR%\include\curl    >nul 2>&1
mkdir %LIB_DIR%\lib\curl        >nul 2>&1
echo A | xcopy /s /e /q vendor\curl-8.16.0\include %LIB_DIR%\include\curl   >nul 2>&1
echo A | xcopy /s /e /q vendor\curl-8.16.0\lib %LIB_DIR%\lib\curl           >nul 2>&1
echo A | xcopy /q vendor\curl-8.16.0\bin\*.dll %LIB_DIR%\bin\               >nul 2>&1

mkdir %LIB_DIR%\include\raylib      >nul 2>&1
mkdir %LIB_DIR%\lib\raylib          >nul 2>&1
echo A | xcopy /s /e vendor\raylib-5.0\include %LIB_DIR%\include\raylib     >nul 2>&1
echo A | xcopy /q vendor\raylib-5.0\lib\libraylib.a %LIB_DIR%\lib\raylib\   >nul 2>&1

mkdir %LIB_DIR%\include\clay    >nul 2>&1
echo A | xcopy /s /e /q vendor\clay-0.14\include %LIB_DIR%\include\clay     >nul 2>&1

echo A | xcopy /q %SRC_DIR%\justengine.h %LIB_DIR%\include\     >nul 2>&1
echo A | xcopy /q %BUILD_DIR%\libjustengine.a %LIB_DIR%\lib\    >nul 2>&1
