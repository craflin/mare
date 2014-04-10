@echo off

if not exist Debug/mare.exe call build.bat
if not "%1"=="" Debug\mare.exe %*
