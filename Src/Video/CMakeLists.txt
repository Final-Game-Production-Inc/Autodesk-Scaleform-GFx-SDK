# Source files
file(GLOB_RECURSE VideoSources *.cpp)
file(GLOB_RECURSE VideoHeaders *.h)

# Randomly include files in another directory for AS3 support.
list(APPEND VideoSources "../GFx/AS3/Obj/Media/AS3_Obj_Media_Video.cpp")
list(APPEND VideoHeaders "../GFx/AS3/Obj/Media/AS3_Obj_Media_Video.h")
list(APPEND VideoSources "../GFx/AS3/Obj/Net/AS3_Obj_Net_NetStream.cpp")
list(APPEND VideoHeaders "../GFx/AS3/Obj/Net/AS3_Obj_Net_NetStream.h")

set(AllVideoFiles ${VideoHeaders} ${VideoSources})

ScaleformSetupLibrary(Video "${AllVideoFiles}")
