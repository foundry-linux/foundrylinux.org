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

del %TMP%\tumetool.zip
pkzip -oa %TMP%\tumetool.zip c:\eggs\tools\cuttiles.exe
pkzip -oa %TMP%\tumetool.zip c:\src\cuttiles\composit.txt
pkzip -oa %TMP%\tumetool.zip c:\eggs\tools\dumptume.exe
pkzip -oa %TMP%\tumetool.zip c:\eggs\tools\makeroom.exe
pkzip -oa %TMP%\tumetool.zip c:\eggs\tools\map2pic.exe
pkzip -oa %TMP%\tumetool.zip c:\eggs\tools\tpbin.exe
pkzip -oa %TMP%\tumetool.zip c:\eggs\tools\tp16.exe
pkzip -oa %TMP%\tumetool.zip c:\bin\32rtm.exe
pkzip -oa %TMP%\tumetool.zip c:\bin\dpmi32vm.ovl
