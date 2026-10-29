@echo off
setlocal EnableExtensions DisableDelayedExpansion
title Afterglow - Convert External Presets

set "CONVERTER_DIR=%~dp0"
set "PYTHON_EXE=%CONVERTER_DIR%..\..\_Tools\python\_3.15.1\python.exe"
set "CONVERTER_SCRIPT=%CONVERTER_DIR%preset_converter.py"
set "SOURCE_DIR=%CONVERTER_DIR%Source Presets"
set "OUTPUT_ROOT=%CONVERTER_DIR%Converted Presets"
set "CONVERTED_COUNT=0"
set "FAILED=0"

echo.
echo Afterglow - External Preset Converter
echo ======================================
echo.

if not exist "%PYTHON_EXE%" goto :missing_python
if not exist "%CONVERTER_SCRIPT%" goto :missing_converter
if not exist "%SOURCE_DIR%" mkdir "%SOURCE_DIR%"
if not exist "%OUTPUT_ROOT%" mkdir "%OUTPUT_ROOT%"

if not "%~1"=="" goto :convert_arguments

for %%F in ("%SOURCE_DIR%\*.zip") do if exist "%%~fF" call :convert_one "%%~fF"
for /d %%D in ("%SOURCE_DIR%\*") do call :convert_one "%%~fD"
goto :finished

:convert_arguments
if "%~1"=="" goto :finished
if not exist "%~f1" (
    echo FAIL: File not found: %~f1
    set "FAILED=1"
) else if exist "%~f1\" (
    call :convert_one "%~f1"
) else if /i "%~x1"==".zip" (
    call :convert_one "%~f1"
) else if /i "%~x1"==".vpreset" (
    call :convert_one "%~f1"
) else if /i "%~x1"==".sspreset" (
    call :convert_one "%~f1"
) else if /i "%~x1"==".h2p" (
    call :convert_one "%~f1"
) else (
    echo SKIP: %~nx1 is not a supported preset, ZIP file or preset folder.
    set "FAILED=1"
)
shift
goto :convert_arguments

:convert_one
set "SOURCE_PRESETS=%~f1"
set "BANK_OUTPUT=%OUTPUT_ROOT%\%~n1"
echo Converting: %~nx1
"%PYTHON_EXE%" "%CONVERTER_SCRIPT%" "%SOURCE_PRESETS%" "%BANK_OUTPUT%"
if errorlevel 1 (
    echo       FAILED
    set "FAILED=1"
) else (
    echo       PASS: %BANK_OUTPUT%
    set /a CONVERTED_COUNT+=1
)
echo.
exit /b 0

:finished
if "%CONVERTED_COUNT%"=="0" goto :no_sources
echo Conversion complete: %CONVERTED_COUNT% bank(s).
echo.
echo Converted presets are in:
echo   %OUTPUT_ROOT%
echo.
if "%FAILED%"=="1" (
    echo One or more supplied files were skipped or failed.
    echo Successful banks are still available in the output folder.
    echo.
)
start "" "%OUTPUT_ROOT%"
pause
if "%FAILED%"=="1" exit /b 1
exit /b 0

:no_sources
echo No source preset ZIP files or unzipped folders were found.
echo.
echo Put every Valhalla, Deelay PE, ColourCopy or Uhbik Delay preset ZIP or folder into:
echo   %SOURCE_DIR%
echo.
echo Then double-click this batch file again.
pause
exit /b 1

:missing_python
echo FAIL: Portable Python was not found:
echo   %PYTHON_EXE%
echo.
echo Expected project layout:
echo   _Projects\Afterglow\Preset Converter
echo   _Projects\_Tools\python\_3.15.1\python.exe
pause
exit /b 1

:missing_converter
echo FAIL: Converter script was not found:
echo   %CONVERTER_SCRIPT%
pause
exit /b 1
