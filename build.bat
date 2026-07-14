@echo off
setlocal

set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
if not exist "%VCVARS%" (
    echo Visual Studio 2022 vcvars64.bat not found at:
    echo   %VCVARS%
    echo Install the "Desktop development with C++" workload, or edit build.bat.
    exit /b 1
)

call "%VCVARS%" >nul
if errorlevel 1 exit /b 1

nmake %*
exit /b %ERRORLEVEL%
