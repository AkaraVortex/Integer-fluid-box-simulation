@echo off
md "bin" >nul 2>&1
copy json bin >nul 2>&1
copy lodepng bin >nul 2>&1
copy zlib bin >nul 2>&1
copy base64 bin >nul 2>&1
copy main bin >nul >nul 2>&1

make

del "bin\*" /f /q /s >nul 2>&1
rmdir "bin" >nul 2>&1