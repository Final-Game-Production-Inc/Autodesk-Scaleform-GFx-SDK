project(libgfxexpat)

set(SRC_FILES
    "../../../../Src/XML/XML_Expat.cpp"
)

include(../BuildOptions.cmake)

target_link_libraries(${PROJECT_NAME} PRIVATE
    expat
)
