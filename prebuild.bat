@rem #!/bin/bash
@echo off

:: Set working dir to this bat file
cd /d "%~dp0"
set path=%path%;C:\Program Files\Git\usr\bin

:: Normally gitdefs.h removed in postbuild
rm -f .\Core\Inc\gitdefs.h

git describe --abbrev=8 --dirty --always > gitinfo.txt
git describe --abbrev=8 --always >> gitinfo.txt
git rev-parse --abbrev-ref HEAD >> gitinfo.txt
git describe --always --tags --dirty=-d | gawk "NR==1{print $0}" >> gitinfo.txt
git rev-list HEAD | wc -l | tr -d ' ' >> gitinfo.txt
:: Git commit date
git log -1 --format=%%ci | tr "-" "\n" | tr " " "\n" | tr ":" "\n" >> gitinfo.txt

gawk -f gitinfo.awk < gitinfo.txt > .\Core\Inc\gitdefs.h

