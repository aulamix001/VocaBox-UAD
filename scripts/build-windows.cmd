@echo off
setlocal
where cmake >nul 2>&1
if errorlevel 1 (
 echo Instale CMake e Visual Studio 2022 com Desktop development with C++.
 pause
 exit /b 1
)
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0build-windows.ps1"
set "build_result=%ERRORLEVEL%"
if not "%build_result%"=="0" (
 echo A compilacao falhou. Consulte a mensagem acima.
) else (
 echo VST3 Windows gerado com sucesso. O caminho esta acima.
)
pause
exit /b %build_result%
