@echo off

set OutputPath=%CD%
set ProjDirPath=%~dp0
set ProjName=%1
set BuildArtifactFileName=%2

arm-none-eabi-objcopy --remove-section .param -O ihex "%BuildArtifactFileName%" "%ProjName%.hex"
move "%ProjDirPath%\gitinfo.txt" "%OutputPath%\"
move "%ProjDirPath%\Core\Inc\gitdefs.h" "%OutputPath%\"