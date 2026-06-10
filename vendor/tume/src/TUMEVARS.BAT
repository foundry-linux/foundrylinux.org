@ECHO OFF
REM 
REM The contents of this file are subject to the Mozilla Public 
REM License Version 1.1 (the "License"); you may not use this file 
REM except in compliance with the License. You may obtain a copy of 
REM the License at http://www.mozilla.org/MPL/ 
REM 
REM Software distributed under the License is distributed on an "AS 
REM IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or 
REM implied. See the License for the specific language governing 
REM rights and limitations under the License. 
REM 
REM The Original Code is tUME: the Universal Map Editor, released 
REM June 18, 2000. 
REM 
REM The Initial Developer of the Original Code is Echidna. 
REM Portions created by Echidna are Copyright (c) 1989-2000 Echidna. 
REM All Rights Reserved. 
REM 
REM Contributor(s): 
REM 

set CMN_ASM=tasm
set CMN_AFLAGS=/mx /zi /m
set CMN_CC=bcc
set CMN_CFLAGS=-c -d -v -w -wpro -wstv
set CMN_CSPEED=-Ox
set CMN_CSIZE=-O1
set CMN_LINK=tlink
set CLIB=D:\BC\LIB
set ELIB=D:\EGGS\LIB
