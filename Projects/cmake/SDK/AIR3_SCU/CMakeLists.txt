project(libgfx_as3)

set(SRC_FILES
    "../../../../Src/GFx/AS3/Abc/AS3_Abc_ConstPool.cpp"
    "../../../../Src/GFx/AS3/Abc/AS3_Abc.cpp"
    "../../../../Src/GFx/AS3/Abc/AS3_Abc_Read.cpp"
    "../../../../Src/GFx/AS3/Audio/AS3_SoundObject.cpp"
    "../../../../Src/GFx/AS3/IME/AS3_IMEManager.cpp"
    "../../../../Src/GFx/AS3/Obj/Accessibility/AS3_Obj_Accessibility_All.cpp"
    "../../../../Src/GFx/AS3/Obj/Display/AS3_Obj_Display_All.cpp"
    "../../../../Src/GFx/AS3/Obj/Errors/AS3_Obj_Errors_All.cpp"
    "../../../../Src/GFx/AS3/Obj/Events/AS3_Obj_Events_All.cpp"
    "../../../../Src/GFx/AS3/Obj/External/AS3_Obj_External_All.cpp"
    "../../../../Src/GFx/AS3/Obj/Filters/AS3_Obj_Filters_All.cpp"
    "../../../../Src/GFx/AS3/Obj/Geom/AS3_Obj_Geom_All.cpp"
    "../../../../Src/GFx/AS3/Obj/GFx/AS3_Obj_GFx_All.cpp"
    "../../../../Src/GFx/AS3/Obj/Media/AS3_Obj_Media_All.cpp"
    "../../../../Src/GFx/AS3/Obj/Net/AS3_Obj_Net_All.cpp"
    "../../../../Src/GFx/AS3/Obj/Security/AS3_Obj_Security_All.cpp"
    "../../../../Src/GFx/AS3/Obj/Sensors/AS3_Obj_Sensors_All.cpp"
    "../../../../Src/GFx/AS3/Obj/System/AS3_Obj_System_All.cpp"
    "../../../../Src/GFx/AS3/Obj/Text/AS3_Obj_Text_All.cpp"
    "../../../../Src/GFx/AS3/Obj/Ui/AS3_Obj_Ui_All.cpp"
    "../../../../Src/GFx/AS3/Obj/Utils/AS3_Obj_Utils_All.cpp"
    "../../../../Src/GFx/AS3/Obj/Vec/AS3_Obj_Vec_All.cpp"
    "../../../../Src/GFx/AS3/Obj/Xml/AS3_Obj_Xml_All.cpp"
    "../../../../Src/GFx/AS3/Obj/AS3_Obj_All.cpp"
    "../../../../Src/GFx/AS3/AS3_All.cpp"
    "../../../../Src/GFx/GFx_AS3Support.cpp"
)

include(../BuildOptions.cmake)

target_link_libraries(${PROJECT_NAME} PRIVATE
    pcre
    zlib
)
