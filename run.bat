@echo off
SETLOCAL

set ENTRY=%1

set LIB_DIR=justengine

set PATH=%~dp0%LIB_DIR%\bin;%PATH%
%ENTRY%.exe