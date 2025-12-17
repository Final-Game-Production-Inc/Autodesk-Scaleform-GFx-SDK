@echo off
cls
echo.

rem ===========================================================================================
rem 
rem The path expected by this file should be Scaleform\GFx\4.0\Bin\Data\AS3\Kits\MMO
rem
rem ===========================================================================================

set resourceDir=.
if not "%1" == "" (set resourceDir=%1)
echo resourceDir("%resourceDir%")

rem ###########################################################################################
rem Setup the destination (delete it if it exists - STEALTH DELETE; MAKE SURE PATHS ARE CORRECT)
rem 
set resourceCompiledDir=%resourceDir%\data
echo Deleting folder "%resourceCompiledDir%"
if exist %resourceCompiledDir% (rd /s /q %resourceCompiledDir%)

rem ###########################################################################################
rem Setup the gfxexport command
rem 
set gfxexportApp=%resourceDir%\..\..\..\..\gfxexport.exe
echo GFxExport("%gfxexportApp%")

rem ###########################################################################################
rem Compile the main file.
rem 
echo.
echo Exporting main files...
%gfxexportApp% %resourceDir%\MMOKit.swf -d %resourceCompiledDir% -i DDS -pack -x
%gfxexportApp% %resourceDir%\widgets\*.swf -d %resourceCompiledDir%\widgets -i DDS -pack -x
echo.



