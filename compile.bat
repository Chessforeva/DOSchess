@echo off
rem ......................................................
rem DOWNLOAD COMPILER v1.9 from www.openwatcom.com
rem AND UNZIP IN FOLDER C:\watcom19
rem THEN USE THIS compile.bat TO COMPILE TO .exe WHICH
rem WORKS TOGETHER WITH dos4gw.exe
rem UNDER DosBox/MS Win MSDOS prompt
rem ......................................................

rem SETS PATHS REQUIRED FOR WATCOM COMPILER
call c:\watcom19\owsetenv.bat

rem COMPILES & LINKS
wcl386.exe DOSchess.c

pause

rem EXECUTE TO TEST IT
DOSchess.exe

pause

