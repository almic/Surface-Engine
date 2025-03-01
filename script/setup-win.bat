@echo off

pushd ..
vendor\premake\premake5.exe --file=premake.lua vs2022
popd
pause
