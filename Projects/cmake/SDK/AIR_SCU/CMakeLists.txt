project(libgfx_air)

set(SRC_FILES
    "../../../../Src/GFx/AS3/Obj/Desktop/AS3_Obj_Desktop_AIR_All.cpp"
    "../../../../Src/GFx/AS3/Obj/Display/AS3_Obj_Display_AIR_All.cpp"
    "../../../../Src/GFx/AS3/Obj/Events/AS3_Obj_Events_AIR_All.cpp"
    "../../../../Src/GFx/AS3/Obj/External/AS3_Obj_External_AIR_All.cpp"
    "../../../../Src/GFx/AS3/Obj/Filesystem/AS3_Obj_Filesystem_AIR_All.cpp"
    "../../../../Src/GFx/AS3/Obj/Html/AS3_Obj_Html_AIR_All.cpp"
    "../../../../Src/GFx/AS3/Obj/Net/AS3_Obj_Net_AIR_All.cpp"
    "../../../../Src/GFx/AS3/Obj/Printing/AS3_Obj_Printing_AIR_All.cpp"
    "../../../../Src/GFx/AS3/Obj/Sampler/AS3_Obj_Sampler_AIR_All.cpp"
    "../../../../Src/GFx/AS3/Obj/Security/AS3_Obj_Security_AIR_All.cpp"
    "../../../../Src/GFx/AS3/Obj/Sensors/AS3_Obj_Sensors_AIR_All.cpp"
    "../../../../Src/GFx/AS3/Obj/Utils/AS3_Obj_Utils_AIR_All.cpp"
)

include(../BuildOptions.cmake)

target_link_libraries(${PROJECT_NAME})
