Autodesk Scaleform 
Compilation Instructions for Unix-Like Systems (including Cygwin)

This package includes Scaleform libs and sample players on most platforms.
Refer to the platform-specific readme (if supplied) for more information.

libjpeg and zlib are built if not provided by the system.
Thread support is enabled by default (using pthreads) if the system supports it.

GNU make version 3.81 is required (On some systems, version 3.80 also works).


To build Scaleform using the system's normal compiler, using any GFx libs
included in the package:

   $ make P=local
   
This will place the executables in a subdirectory of Bin based on the platform.
For example, i386-linux on Linux for x86.

To run the sample player:
   $ Bin/<platform>/FxPlayer <swf file>
   
To build Scaleform using a cross compiler:
   $ make P=<target>
   This expects the toolchain to be installed as "<target>-gcc"
   and similar. Output is stored in Bin/<target> and Lib/<target>.
   This also builds the X11 player, if the cross-compile target provides X
   headers. Many releases also have platform-specific makefiles, in
   Projects/<target>. 


Sound support with FMOD
-----------------------
Scaleform now includes sound support based on FMOD, which will be automatically enabled if
FMOD is detected. Many Scaleform packages already include FMOD.

If installing FMOD yourself:
  Set FMODSDK in Makeconfig to the directory that contains "inc" and
  "lib" from the platform-specific FMOD SDK. If not specified, FMOD is searched for in
  "3rdParty/fmod/<platform>" within the Scaleform SDK. 


Evaluation Versions
-------------------
If this is an evaluation version, place the license key in "gfxlicense.txt"
in any of:
   The current directory
   The path from the GFXSDK environment variable
   The home directory

If this release included a platform-specific readme file, refer to it for the
location of the license file.


Addons
------
After installing an addon, it will automatically be enabled in our demo applications
after they are rebuilt with make. The prebuilt demo applications included in the main
release have the addons included, but they will no longer have the addons if rebuilt.


Build Configurations
--------------------
Libs and executables have the configuration names appended (except for Release alone).

   Debug        With debug info
   DebugOpt	Optimized with debug info
   Release      Optimized
   Shipping     Optimized shipping
   NoRTTI       Disable RTTI & exceptions

Specify the configuration to build with C=<config> on the make command line.
Separate multiple configuration names with "+". For example, optimized with no RTTI & exceptions
support is "make C=Release+NoRTTI".
The flags used for each configuration are near the top of the root Makefile, or at the
top of a platform-specific makefile in a subdirectory of Projects.


Renderer Libs
-------------
Unlike older releases, prebuilt renderer libs are supplied, named
Lib/<platform>/libgfxrender_<renderer>.a

These can usually be linked into an application and used without being rebuilt.


Autodesk Scaleform Support
http://www.scaleform.com/support