@echo off
pushd %~dp0\..\
call vendor\bin\premake\premake5.exe --file=LinkVue.lua vs2022
popd
PAUSE