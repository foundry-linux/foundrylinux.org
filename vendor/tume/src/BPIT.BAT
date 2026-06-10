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

del localdir.h
@echo /* >localdir.h
@echo  * The contents of this file are subject to the Mozilla Public >>localdir.h
@echo  * License Version 1.1 (the "License"); you may not use this file >>localdir.h
@echo  * except in compliance with the License. You may obtain a copy of >>localdir.h
@echo  * the License at http://www.mozilla.org/MPL/ >>localdir.h
@echo  * >>localdir.h
@echo  * Software distributed under the License is distributed on an "AS >>localdir.h
@echo  * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or >>localdir.h
@echo  * implied. See the License for the specific language governing >>localdir.h
@echo  * rights and limitations under the License. >>localdir.h
@echo  * >>localdir.h
@echo  * The Original Code is tUME: the Universal Map Editor, released >>localdir.h
@echo  * June 18, 2000. >>localdir.h
@echo  * >>localdir.h
@echo  * The Initial Developer of the Original Code is Echidna. >>localdir.h
@echo  * Portions created by Echidna are Copyright (c) 1989-2000 Echidna. >>localdir.h
@echo  * All Rights Reserved. >>localdir.h
@echo  * >>localdir.h
@echo  * Contributor(s): >>localdir.h
@echo  */>>localdir.h
@echo #define EUI_SYMBOLS_DIR "%ELIB%/ECHIDNA/EUILIB/SYMBOLS" >>localdir.h
@echo #define EUI_POINTERS_DIR "%ELIB%/ECHIDNA/EUILIB/POINTERS" >>localdir.h
@echo #define EUI_SYSTEM_DIR "%ELIB%/ECHIDNA/EUILIB/SYSTEM" >>localdir.h
@echo #define EUI_SMALL_DIR "%ELIB%/ECHIDNA/EUILIB/SMALL" >>localdir.h
brushpak euibpi.bps
brushpak tumebpi.bps

