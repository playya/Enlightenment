@echo off

rem Set external libraries directory.
set EXT_DIR=%cd%\..\..\..\extern

if not exist %EXT_DIR% (
	set EXT_DIR=
	echo ERROR: External libs dir is not set.
	pause
	goto END
)

if "%DXSDK_DIR%" == "" (
	echo ERROR: Microsoft DirectX SDK is not installed.
	pause
	goto END
)

rem Check for basic requirements for Visual Studio 2008
if "%VS90COMNTOOLS%" == "" (
	echo ERROR: Microsoft Visual Studio 2008 is not installed.
	echo ERROR: Try to launch Microsoft Visual Studio 2005.
	pause
	goto NOVS9
)

set PROJECT_TYPE=vs9
set VSCOMMONTOOLS=%VS90COMNTOOLS%vsvars32.bat

goto STARTVS

:NOVS9

rem Check for basic requirements for Visual Studio 2005
if "%VS80COMNTOOLS%" == "" (
	echo ERROR: Microsoft Visual Studio 2005 is not installed.
	pause
	goto END
)

set PROJECT_TYPE=vs8
set VSCOMMONTOOLS=%VS80COMNTOOLS%vsvars32.bat

rem Patching Eina
patch.exe --binary -p1 < %cd%\patch\eina.diff
echo INFO: Eina patched.

pause

:STARTVS

rem Setup common Win32 environment variables

rem Add DirectX includes and libraries dirs.
set INCLUDE=%DXSDK_DIR%Include;%INCLUDE%
set LIB=%DXSDK_DIR%Lib\x86;%LIB%

rem Add Evil lib path
set EvilInclude=%cd%\..\..\evil\src\lib
set EvilCommon=%cd%\..\..\evil\win32\common
set EvilOut=%cd%\..\..\evil\win32\%PROJECT_TYPE%\out

set INCLUDE=%EvilCommon%;%EvilInclude%;%EvilInclude%\dlfcn;%INCLUDE%
set LIB=%EvilOut%;%LIB%

rem Add Eina lib path
set EinaInclude=%cd%\..\..\eina\src\include
set EinaCommon=%cd%\..\..\eina\win32\common
set EinaOut=%cd%\..\..\eina\win32\%PROJECT_TYPE%\out

set INCLUDE=%EinaInclude%;%EinaCommon%;%INCLUDE%
set LIB=%EinaOut%;%LIB%

rem Add Eet lib path
set EetInclude=%cd%\..\..\eet\src\lib
set EetOut=%cd%\..\..\eet\win32\%PROJECT_TYPE%\out

set INCLUDE=%EetInclude%;%INCLUDE%
set LIB=%EetOut%;%LIB%

rem Add Evas lib path
set EvasInclude=%cd%\..\..\evas\src\lib;%cd%\..\..\evas\src\modules\engines\buffer;%cd%\..\..\evas\src\modules\engines\software_gdi;%cd%\..\..\evas\src\modules\engines\software_ddraw
set EvasOut=%cd%\..\..\evas\win32\%PROJECT_TYPE%\out

set INCLUDE=%EvasInclude%;%INCLUDE%
set LIB=%EvasOut%;%LIB%

rem Add installation directory pathes.
set INCLUDE=%EXT_DIR%\include;%INCLUDE%
set LIB=%EXT_DIR%\lib;%LIB%

set INCLUDE=%cd%\common;%cd%\..\src\lib\ecore;%cd%\..\src\lib\ecore_input;%cd%\..\src\lib\ecore_input_evas;%cd%\..\src\lib\ecore_win32;%INCLUDE%

set SolutionDirectory=%cd%\%PROJECT_TYPE%
set DebugOutputDirectory=%SolutionDirectory%\out
set ReleaseOutputDirectory=%SolutionDirectory%\out
set DebugLibraryDirectory=%SolutionDirectory%\out
set ReleaseLibraryDirectory=%SolutionDirectory%\out
set TemporaryDirectory=%SolutionDirectory%\temp

rem Setting environment for using Microsoft Visual Studio x86 tools.
call "%VSCOMMONTOOLS%"

%PROJECT_TYPE%\ecore.sln

:END
