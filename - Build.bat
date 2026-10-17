@echo off
setlocal EnableExtensions
if /i "%~1"=="--run-build" goto :run_build
set "RESULTS_LOG=%~dp0Results.log"
set "BUILD_SCRIPT=%~f0"
powershell.exe -NoProfile -ExecutionPolicy Bypass -Command ^
  "& { & $env:BUILD_SCRIPT '--run-build' 2>&1 | Tee-Object -FilePath $env:RESULTS_LOG; exit $LASTEXITCODE }"
set "BUILD_EXIT_CODE=%ERRORLEVEL%"
echo.
echo Results saved: %RESULTS_LOG%
echo.
pause
exit /b %BUILD_EXIT_CODE%

:run_build
title Afterglow - Build
set "PROJECT_ROOT=%~dp0"
set "SOURCE_DIR=%PROJECT_ROOT%source"
set "BUILD_DIR=%PROJECT_ROOT%build"
set "DIST_DIR=%PROJECT_ROOT%dist"
set "CMAKE_EXE=%PROJECT_ROOT%..\_Tools\cmake\_4.4.2\bin\cmake.exe"
set "JUCE_CMAKE=%PROJECT_ROOT%..\_Tools\JUCE\_9.0.4\CMakeLists.txt"
set "JUCE_CACHE_MARKER=%BUILD_DIR%\.juce-9.0.4"
set "BUNDLE_BINARY=%BUILD_DIR%\Afterglow_artefacts\Release\VST3\Afterglow.vst3\Contents\x86_64-win\Afterglow.vst3"
set "FINAL_PLUGIN=%DIST_DIR%\Afterglow.vst3"

echo.
echo Afterglow 64-bit VST3 - v0.7.0 Build
echo ======================================
echo.

echo Closing PolyHostInterface.exe if it is running...
taskkill.exe /F /IM "PolyHostInterface.exe" >nul 2>&1
if errorlevel 1 (
    echo       No running instance found
) else (
    echo       CLOSED
)
echo.

echo [1/4] Checking requirements...
if not exist "%CMAKE_EXE%" goto :missing_cmake
if not exist "%JUCE_CMAKE%" goto :missing_juce
for %%F in (CMakeLists.txt PluginProcessor.cpp PluginProcessor.h PluginEditor.cpp PluginEditor.h) do (
    if not exist "%SOURCE_DIR%\%%F" goto :missing_source
)
echo       PASS
echo.
echo [2/4] Configuring Visual Studio Community 2026 x64...
if exist "%BUILD_DIR%\CMakeCache.txt" if not exist "%JUCE_CACHE_MARKER%" (
    rmdir /s /q "%BUILD_DIR%"
    if exist "%BUILD_DIR%" goto :configure_failed
)
"%CMAKE_EXE%" -S "%SOURCE_DIR%" -B "%BUILD_DIR%" -G "Visual Studio 18 2026" -A x64
if errorlevel 1 goto :configure_failed
> "%JUCE_CACHE_MARKER%" echo 9.0.4
echo       PASS
echo.
echo [3/4] Compiling Release VST3...
"%CMAKE_EXE%" --build "%BUILD_DIR%" --config Release --target Afterglow_VST3 --clean-first --parallel
if errorlevel 1 goto :build_failed
if not exist "%BUNDLE_BINARY%" goto :binary_not_found
echo       PASS
echo.
echo [4/4] Creating and checking portable x64 output...
if not exist "%DIST_DIR%" mkdir "%DIST_DIR%"
if exist "%FINAL_PLUGIN%\" rmdir /s /q "%FINAL_PLUGIN%"
if exist "%FINAL_PLUGIN%" del /f /q "%FINAL_PLUGIN%"
copy /y "%BUNDLE_BINARY%" "%FINAL_PLUGIN%" >nul
if errorlevel 1 goto :copy_failed
powershell.exe -NoProfile -Command ^
  "$bytes = [IO.File]::ReadAllBytes('%FINAL_PLUGIN%'); if ($bytes.Length -lt 64) { exit 1 }; $pe = [BitConverter]::ToInt32($bytes,60); if ($pe -lt 0 -or ($pe+6) -gt $bytes.Length) { exit 1 }; if ([BitConverter]::ToUInt32($bytes,$pe) -ne 0x00004550) { exit 1 }; if ([BitConverter]::ToUInt16($bytes,$pe+4) -ne 0x8664) { exit 1 }"
if errorlevel 1 goto :wrong_architecture
echo       PASS
echo.
echo BUILD SUMMARY
echo   - PASS: Required files
 echo   - PASS: CMake configuration
 echo   - PASS: Release VST3 compiled
 echo   - PASS: Portable x64 binary validated
 echo.
echo Output: %FINAL_PLUGIN%
echo No plugin installed automatically.
exit /b 0
:missing_cmake
echo FAIL: CMake missing: %CMAKE_EXE%
goto :failed
:missing_juce
echo FAIL: JUCE missing: %JUCE_CMAKE%
goto :failed
:missing_source
echo FAIL: Required source file missing from %SOURCE_DIR%
goto :failed
:configure_failed
echo FAIL: Configuration failed.
goto :failed
:build_failed
echo FAIL: Compilation failed.
goto :failed
:binary_not_found
echo FAIL: Built VST3 binary not found: %BUNDLE_BINARY%
goto :failed
:copy_failed
echo FAIL: Output copy failed.
goto :failed
:wrong_architecture
echo FAIL: Output is not Windows x64.
goto :failed
:failed
echo.
echo Please send Results.log.
exit /b 1
