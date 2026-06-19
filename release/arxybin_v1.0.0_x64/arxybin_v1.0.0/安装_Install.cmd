@echo off
chcp 65001 >nul
title arxybin. v1.0.0 — Install

echo.
echo   arxybin.  v1.0.0
echo   Granular + Glitch Particle Effect
echo   by arxybin.
echo.

net session >nul 2>&1
if %errorlevel% neq 0 (
    echo [WARN] Admin rights required. Restarting as admin...
    powershell -Command "Start-Process '%~f0' -Verb RunAs"
    exit /b
)

set VST3_DIR=C:\Program Files\Common Files\VST3
set PRESET_DIR=%USERPROFILE%\Documents\arxybin\Presets

echo [1/3] Installing VST3...
if exist "%VST3_DIR%\arxybin..vst3" rmdir /s /q "%VST3_DIR%\arxybin..vst3" 2>nul
xcopy /e /i /q "%~dp0arxybin..vst3" "%VST3_DIR%\arxybin..vst3" >nul
echo   -> %VST3_DIR%\arxybin..vst3

echo [2/3] Installing default preset...
if not exist "%PRESET_DIR%" mkdir "%PRESET_DIR%"
copy /y "%~dp0default.arxybin" "%PRESET_DIR%\default.arxybin" >nul
echo   -> %PRESET_DIR%\default.arxybin

echo [3/3] Standalone ready.
echo   Run: "%~dp0arxybin..exe"

echo.
echo   Install complete! Open your DAW and scan for new plugins.
echo.
pause
