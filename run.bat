@echo off
@echo on
SETLOCAL

set ENTRY=%1
for /f "tokens=1,* delims= " %%a in ("%*") do set ARGS=%%b

set LIB_DIR=justengine

set PATH=%~dp0%LIB_DIR%\bin;%PATH%
%ENTRY% %ARGS%
@REM START /D "%cd%" "" "%ENTRY%" %ARGS%
@REM start /b /D %cd% "" "%ENTRY%" %ARGS%
@REM start /b "" "%ENTRY%" %ARGS%