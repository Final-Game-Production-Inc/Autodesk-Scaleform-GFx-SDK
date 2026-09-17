Autodesk Scaleform 4.0
PS3 Compilation Instructions

GNU make version 3.81 is required (unless using Visual Studio for building).
Visual Studio projects are also supplied, for use with ProDG VSI.

The renderer library libgfxrender_ps3.a is supplied prebuilt and contains the compiled
shaders. If you want to manually rebuild the renderer, or are using Visual Studio, then
the compiled shaders are provided as extra libs. If the renderer libs are built with
Visual Studio, they will not include the compiled shaders.

Compiled shaders for the libgcm renderer are stored in Lib/PS3/Shaders.a.
The compiled shaders can only be rebuilt with make.


Steps for using make:
---------------------

1) Create a Makeconfig file (from Makeconfig_example) in the top level GFx directory, containing:

   export CELL_SDK := <path to Cell SDK>
   EPATH := $(PATH)
   export PATH = $(CELL_SDK)/host-win32/ppu/bin:$(CELL_SDK)/host-win32/Cg/bin:$(CELL_SDK)/host-win32/bin:$(EPATH)

   The paths in Makeconfig must be unix-style format, with forward slashes and no drive
   letters with colons. Use /<drive letter>/... for MSYS and /cygdrive/<drive letter>/... for Cygwin.
   For example, if CELL_SDK is installed to c:\ps3\cell: "export CELL_SDK := /c/ps3/cell".

2) Build GFx libs and samples:
   $ make P=PS3
   
3) Set "app_home" directory to Bin/Samples/AS2 for FxPlayerTiny.
   Set "app_home" directory to Bin/Samples/AS2 (or Bin/Samples/AS3) and specify file name
   in command line for FxPlayer. Other .swf and .gfx files in that directory can be viewed
   in FxPlayer; use the L1 and R1 buttons.


Included PS3 Renderer support
-----------------------------

libgcm:
  Source files: $(GFXSDK)/Src/Render/PS3
  This renderer uses shaders linked into the executable; no runtime file loading.
  Link Lib/PS3/Shaders.a with your executable; to use our build system just for this:
  $ make P=PS3 Lib/PS3/Shaders.a


PRX Modules
-----------

  GFx requires resc, fs, and gcm:
    cellSysmoduleLoadModule(CELL_SYSMODULE_RESC);
    cellSysmoduleLoadModule(CELL_SYSMODULE_FS);
    cellSysmoduleLoadModule(CELL_SYSMODULE_GCM);

  ActionScript "Date" (and evaluation versions) require rtc also:
    cellSysmoduleLoadModule(CELL_SYSMODULE_RTC);


Scaleform demos
---------

  FxPlayer      		Basic SWF player
  FxPlayerTiny  		Tiny SWF player
  FxPlayerTinyStereo  	Tiny SWF player that demonstrates stereo display
    
  All samples using a common sample framework for video mode setup, buffer allocation, etc.
  Samples source files: $(GFXSDK)/Apps/Samples


Autodesk Scaleform Support
http://www.scaleform.com/support

