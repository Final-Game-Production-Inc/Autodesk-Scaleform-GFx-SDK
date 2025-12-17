Autodesk(R) Scaleform(R) 4.1.20

(c) 2011 Autodesk, Inc. All rights reserved.
============================================

This Software is subject to the terms and conditions of the Autodesk End User License Agreement.
The Software may be used or copied only in accordance with the terms of the Agreement. It is against
the law to copy the Software on any medium except as specifically allowed in the Agreement.

Autodesk and Scaleform are registered trademarks or trademarks of Autodesk, Inc. All other
brand names, product names or trademarks belong to their respective holders.

Platform-specific Notes
=======================
The libraries contained in this package are built with version 2.07.03 of the Nintendo libraries.
Cygwin and GNU make version 3.81 are required for building the samples as well as the latest version of the following Cygwin packages:

     binutils
     gcc-g++

Wii U Locked Cache
=======================
Scaleform does not use Wii U Locked Cache.


For more information
====================
For more information about this package, including a description of the directories contained within, 
see the Getting Started with Scaleform for WiiU document contained in the base Scaleform package.

1) Run cafe.bat in the root of the CAFE_SDK directory.  This will open a Cygwin window with the correct environment, 
from which you can build (make P=WiiU).  As an alternative, you can set the Cygwin environment yourself by creating a 
Makeconfig file (from Makeconfig_example) in the top level GFx directory, containing:

   If the GHS installer did not set GHS_ROOT correctly, you must set that (either in the environment
   or in Makeconfig). CAFE_ROOT must also be set.

   These paths must be in posix format, with forward slashes and no drive letters
   with colons. Use /cygdrive/<drive letter>/...
   For example, if CAFE_SDK is installed to c:/CAFE_SDK: "CAFE_ROOT := /cygdrive/c/CAFE_SDK".

2) Store Flash content to be displayed in the sample player(s) to a folder named FxPlayer in the content directory.  For example:

   "cafe_sdk\data\disc\content\FxPlayer"

   Store Flash content to be displayed in the Kits (MenuKit and HudKit) to a folder named Kits in the content directory.  For example:
   
   "cafe_sdk\data\disc\content\Kits"
   
   Store Flash content to be displayed in the Wii U Demo to a folder named WiiUDemo in the content directory.  For example:

   cafe_sdk\data\disc\content\WiiUDemo

   If this is an Evaluation release, place the sf_license.txt file in the FxPlayer folder as well.

3) Build the libs and samples:
   $ make P=WiiU

4) Run Bin/WiiU/FxPlayer.rpx (or one with a Debug suffix if desired).  For example:
 
   caferun FxPlayer.rpx /vol/content/FxPlayer/YOUR_SWF_FILE.swf
 
   Most samples display on the TV only, but the touch screen can be used for input. Press +, -, L, and R
   to switch between Flash files. 
   
   FxPlayerTiny displays two files if a DRC is connected. FxPlayerTiny takes two SWFs as commandline arguments.  For example:
   
   caferun FxPlayerTiny.rpx /vol/content/FxPlayer/YOUR_SWF_FILE1.swf /vol/content/FxPlayer/YOUR_SWF_FILE2.swf
 
   FxPlayerMobile also displays on the TV as well as the DRC.  Your SWF will display on the TV and the HUD for FxPlayerMobile will display on the DRC.
   To run FxPlayerMobilePlayer, use the following command:
   
   caferun Bin/WiiU/FxPlayerMobile.rpx
   
   You do not need to pass a SWF on the command line to FxPlayerMobile.  It defaults to using "flash.swf" (which is not provided in the SDK.
   We recommend renaming one of our existing files to "flash.swf" and using it to test the functionality of FxPlayerMobile.  
   Please place "flash.swf" in the root of your FxPlayer directory, for example:
   
   cafe_sdk\data\disc\content\FxPlayer\flash.swf
 
   To run the Kits, you can do following.
 
   caferun Bin/WiiU/HudDemo.rpx
   caferun Bin/WiiU/MenuDemo.rpx
   caferun Bin/WiiU/WiiUDemo.rpx


Included Renderer support
-------------------------

The renderer source is in Src/Render/WiiU. Since there is no special region of memory for graphics, the
default WiiU::MemoryManager can be used which just allocates from the default GFx heap.

The compiled shaders are built into libgfxrender_wiiu.a.


GTX Support
-------------------------

GTX support is provided in our Wii U integration.  To use GTX files, first generate DDS images with gfxexport.  
Then run the DDS image files through "TexConv2.exe"(an executable that resides in the CAFE SDK).  Please note that
the newly created GTX images must have a DDS extension for the Scaleform players to properly load them.


Autodesk Scaleform Support
http://www.scaleform.com/support
