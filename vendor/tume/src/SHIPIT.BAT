@ECHO OFF
if "%1"=="" goto HELP

if "%1"=="NONE" goto ALL
if "%1"=="CLD" goto ALL
if "%1"=="PSYQ" goto ALL
if "%1"=="SEGADEV" goto ALL
if "%1"=="SNASM" goto ALL

@ECHO Unknown option %1.
goto HELP

:ALL
REM Stuff that everyone gets.

del %TMP%\tume.zip
tdstrip tume %TMP%\tume
pkzip -ex -ao %TMP%\tume %TMP%\tume.exe
del %TMP%\tume.exe
pkzip -ex -ao %TMP%\tume tume.ini
pkzip -ex -ao %TMP%\tume tume.fpf
pkzip -ex -ao %TMP%\tume tume.ico
pkzip -ex -ao %TMP%\tume tume.pif
pkzip -ex -ao %TMP%\tume newtume.doc

if "%1"=="NONE" goto NONE
if "%1"=="CLD" goto CLD
if "%1"=="PSYQ" goto PSYQ
if "%1"=="SEGADEV" goto SEGADEV
if "%1"=="SNASM" goto SNASM

:NONE
goto END

:CLD
pkzip -ex -ao %TMP%\tume cldtume.bat
pkzip -ex -ao %TMP%\tume c:\bin\dlnobcld.exe
pkzip -ex -ao %TMP%\tume cldscrol.nob
goto END

:PSYQ
pkzip -ex -ao %TMP%\tume pstume.bat
pkzip -ex -ao %TMP%\tume %EGGS%\tools\dlcpe.exe
pkzip -ex -ao %TMP%\tume psyqs68k.cpe
pkzip -ex -ao %TMP%\tume psyqssfx.cpe
goto END

:SEGADEV
pkzip -ex -ao %TMP%\tume sdtume.bat
pkzip -ex -ao %TMP%\tume scrolsd.bin
goto END

:SNASM
pkzip -ex -ao %TMP%\tume sntume.bat
pkzip -ex -ao %TMP%\tume %EGGS%\tools\dlcpe.exe
pkzip -ex -ao %TMP%\tume scrolsfx.cpe
pkzip -ex -ao %TMP%\tume scrol68k.cpe
goto END

:HELP
@ECHO Must specify NONE, CLD, PSYQ, SEGADEV or SNASM.
:END
