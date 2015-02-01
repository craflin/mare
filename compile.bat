@echo off
setlocal

set MARE_BUILD_DIR="build/Debug/mare"
set MARE_OUTPUT_DIR="build/Debug/mare"
set MARE_SOURCE_DIR="src"
set MARE_SOURCE_FILES=mare/Generator.cpp mare/CMake.cpp mare/CodeBlocks.cpp mare/CodeLite.cpp mare/Main.cpp mare/Make.cpp mare/Mare.cpp mare/NetBeans.cpp mare/Vcproj.cpp mare/Vcxproj.cpp mare/Tools/md5.cpp mare/Tools/Win32/getopt.cpp libmare/Engine.cpp libmare/Namespace.cpp libmare/Parser.cpp libmare/Statement.cpp libmare/Tools/Directory.cpp libmare/Tools/Error.cpp libmare/Tools/File.cpp libmare/Tools/Process.cpp libmare/Tools/Scope.cpp libmare/Tools/String.cpp libmare/Tools/Word.cpp

:main
goto get_args
:get_args_return
goto find_visual_studio
:find_visual_studio_return
goto build
:build_return
goto end


:get_args
if "%1" == "--buildDir" (
  set MARE_BUILD_DIR="%2"
  shift /1
  shift /1
  goto get_args
)
if "%1" == "--outputDir" (
  set MARE_OUTPUT_DIR="%2"
  shift /1
  shift /1
  goto get_args
)
if "%1" == "--sourceDir" (
  set MARE_SOURCE_DIR="%2"
  shift /1
  shift /1
  goto get_args
)
if "%1" == "" goto get_args_return
goto usage


:usage
echo Usage: %0 [^-^-buildDir=^<dir^>] [^-^-outputDir=^<dir^>] [^-^-sourceDir=^<dir^>]
goto end


:find_visual_studio
if not "%VCINSTALLDIR%"=="" goto find_visual_studio_return

if not "%VS120COMNTOOLS%"=="" (
   call "%VS120COMNTOOLS%vsvars32.bat"
   goto find_visual_studio_return
)

if not "%VS110COMNTOOLS%"=="" (
  call "%VS110COMNTOOLS%vsvars32.bat"
  goto find_visual_studio_return
)

if not "%VS100COMNTOOLS%"=="" (
  call "%VS100COMNTOOLS%vsvars32.bat"
  goto find_visual_studio_return
)

echo "error: Could not find Visual Studio installation."
goto end


:build
if not exist %MARE_SOURCE_DIR:"=%/mare/Main.cpp (
  echo "error: Could not find source files."
  goto end
)
setlocal enableextensions
if not exist %MARE_BUILD_DIR% mkdir %MARE_BUILD_DIR%
if not exist %MARE_OUTPUT_DIR% mkdir %MARE_OUTPUT_DIR%
endlocal
setlocal
set DEL_FILE=%MARE_OUTPUT_DIR:"=%/mare.exe
del %DEL_FILE:/=\% 2>NUL
endlocal
for %%f in (%MARE_SOURCE_FILES%) do (
  cl /I"%MARE_SOURCE_DIR:"=%/libmare" /Zi /nologo /W3 /WX- /Od /Oy- /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /Gm /EHsc /RTC1 /GS /fp:precise /Zc:wchar_t /Zc:forScope /Fd"%MARE_BUILD_DIR:"=%/" /Fo"%MARE_BUILD_DIR:"=%/" /Gd /analyze- /c "%MARE_SOURCE_DIR:"=%/%%f"
)
set MARE_OBJECTS=
for %%f in (%MARE_BUILD_DIR:"=%/*.obj) do (
  call set MARE_OBJECTS=%%MARE_OBJECTS%% %MARE_BUILD_DIR:"=%/%%f
)
echo ^-^> %MARE_BUILD_DIR:"=%/mare.exe
link /OUT:"%MARE_OUTPUT_DIR:"=%/mare.exe" /NOLOGO "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib" /ALLOWISOLATION /MANIFESTUAC:"level='asInvoker' uiAccess='false'" /DEBUG /SUBSYSTEM:CONSOLE /TLBID:1 /DYNAMICBASE /NXCOMPAT /MACHINE:X86 %MARE_OBJECTS%
rem if exist %MARE_OUTPUT_DIR:"=%/mare.exe (echo done) else echo failed
goto build_return


:end
endlocal
