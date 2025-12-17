@echo off

xcopy /Y /E /I /Q ..\..\..\..\jpeg-8d ..\..\..\..\..\Obj\PS4\3rdParty\jpeg-8d
del   /S ..\..\..\..\..\Obj\PS4\3rdParty\jpeg-8d\jconfig.h
copy  /Y ..\..\..\..\..\Obj\PS4\3rdParty\jpeg-8d\jconfigPS4.h ..\..\..\..\..\Obj\PS4\3rdParty\jpeg-8d\jconfig.h
copy  /Y ..\..\..\..\..\Obj\PS4\3rdParty\jpeg-8d\jconfigPS4.h ..\..\..\..\..\3rdParty\Include\PS4\jconfigPS4.h
copy  /Y ..\..\..\..\zlib-1.2.7\zlib.h ..\..\..\..\..\Obj\PS4\3rdParty
copy  /Y ..\..\..\..\zlib-1.2.7\zconf.h ..\..\..\..\..\Obj\PS4\3rdParty
copy  /Y ..\..\..\..\expat-2.1.0\gfx_projects\PS4\expat_config.h ..\..\..\..\..\Obj\PS4\3rdParty
