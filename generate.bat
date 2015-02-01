@echo off

if not exist Debug/mare.exe call compile.bat
if not "%1"=="" Debug\mare.exe %*
