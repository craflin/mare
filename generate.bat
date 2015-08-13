@echo off

if not exist build\Debug\mare\mare.exe call compile.bat
if not "%1"=="" build\Debug\mare\mare.exe %*
