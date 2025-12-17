@echo off

xcopy /Y /E /I /Q ..\..\..\..\jpeg-8d ..\..\..\..\..\Obj\PS3\3rdParty\jpeg-8d
del   /S ..\..\..\..\..\Obj\PS3\3rdParty\jpeg-8d\jconfig.h
copy  /Y ..\..\..\..\..\Obj\PS3\3rdParty\jpeg-8d\jconfigPS3.h ..\..\..\..\..\Obj\PS3\3rdParty\jpeg-8d\jconfig.h
copy  /Y ..\..\..\..\..\Obj\PS3\3rdParty\jpeg-8d\jconfigPS3.h ..\..\..\..\..\3rdParty\Include\PS3\jconfigPS3.h
copy  /Y ..\..\..\..\zlib-1.2.7\zlib.h ..\..\..\..\..\Obj\PS3\3rdParty
copy  /Y ..\..\..\..\zlib-1.2.7\zconf.h ..\..\..\..\..\Obj\PS3\3rdParty
copy  /Y ..\..\..\..\expat-2.1.0\gfx_projects\PS3\expat_config.h ..\..\..\..\..\Obj\PS3\3rdParty
