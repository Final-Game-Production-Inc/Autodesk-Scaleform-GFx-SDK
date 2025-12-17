project(libgfx_as2)

set(SRC_FILES
    "../../../../Src/GFx/AS2/Audio/AS2_SoundObject.cpp"
    "../../../../Src/GFx/AS2/XML/AS2_Xml.cpp"
    "../../../../Src/GFx/AS2/XML/AS2_XmlNode.cpp"
    "../../../../Src/GFx/AS2/IME/GASIme.cpp"
    "../../../../Src/GFx/AS2/IME/AS2_IMEManager.cpp"
    "../../../../Src/GFx/AS2/AS2_All.cpp"
    "../../../../Src/GFx/GFx_AS2Support.cpp"
)

include(../BuildOptions.cmake)

target_link_libraries(${PROJECT_NAME})
