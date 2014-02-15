@echo off

if "%1"=="" (goto usage)

if not exist Debug/mare.exe call build.bat
Debug\mare.exe %*
goto end

:usage
echo Usage: %0 [^-^-vcxproj=2010^|2012^|2013]
goto end


:end
