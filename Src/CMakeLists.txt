# Scaleform SDK libraries -- built from source.
# Source file lists match the original .vcxproj files.
# Files marked 'not in vcxproj' were added to resolve link errors.

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 17)

# ---------------------------------------------------------------------------
# libgfx -- core SDK library (Kernel + GFx + Render)
# ---------------------------------------------------------------------------
add_library(gfx STATIC
    ${CMAKE_SOURCE_DIR}/Src/Kernel/HeapPT/HeapPT_SysAllocMalloc.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/HeapPT/HeapPT_AllocBitSet1.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/HeapPT/HeapPT_AllocBitSet2.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/HeapPT/HeapPT_AllocEngine.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/HeapPT/HeapPT_AllocLite.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/HeapPT/HeapPT_BitSet1.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/HeapPT/HeapPT_Bookkeeper.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/HeapPT/HeapPT_DebugInfo.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/HeapPT/HeapPT_FreeBin.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/HeapPT/HeapPT_Granulator.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/HeapPT/HeapPT_MemoryHeap.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/HeapPT/HeapPT_PageTable.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/HeapPT/HeapPT_Root.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/HeapPT/HeapPT_Starter.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/HeapPT/HeapPT_SysAllocMapper.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/HeapPT/HeapPT_SysAllocStatic.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/HeapPT/HeapPT_SysAllocWinAPI.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/HeapMH/HeapMH_AllocBitSet2.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/HeapMH/HeapMH_AllocEngine.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/HeapMH/HeapMH_DebugInfo.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/HeapMH/HeapMH_FreeBin.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/HeapMH/HeapMH_MemoryHeap.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/HeapMH/HeapMH_Root.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/SF_FileFILE.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/SF_Alg.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/SF_AllocAddr.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/SF_AmpInterface.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/SF_Atomic.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/SF_Debug.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/SF_File.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/SF_Locale.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/SF_Log.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/SF_MemItem.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/SF_MemoryHeap.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/SF_MemReport.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/SF_MsgFormat.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/SF_Random.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/SF_RefCount.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/SF_Stats.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/SF_Std.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/SF_String.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/SF_String_PathUtil.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/SF_SysFile.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/SF_System.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/SF_Threads.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/SF_Timer.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/SF_UTF8Util.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/SF_WString.cpp
    ${CMAKE_SOURCE_DIR}/Src/Kernel/SF_ThreadsWinAPI.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AMP/Amp_Message.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AMP/Amp_MessageRegistry.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AMP/Amp_ObjectsLog.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AMP/Amp_ProfileFrame.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AMP/Amp_Server.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AMP/Amp_ServerCallbacks.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AMP/Amp_Socket.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AMP/Amp_Stream.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AMP/Amp_ThreadMgr.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AMP/Amp_ViewStats.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AMP/Amp_Socket_Win32.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/Audio/GFx_Sound.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/Audio/GFx_SoundResource.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/Audio/GFx_SoundTagsReader.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/Audio/GFx_Audio.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/Text/Text_EditorKit.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/Text/Text_StyleSheet.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/XML/XML_Document.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/XML/XML_DOM.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/XML/XML_Object.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/XML/XML_Support.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/IME/GFx_IMEManager.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/IME/GFxKoreanIME.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_ASString.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_ASUtils.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_Button.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_ButtonDef.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_CharacterDef.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_DisplayList.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_DisplayObjContainer.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_DisplayObject.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_DrawText.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_DrawingContext.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_Event.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_FileOpener.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_FilterDesc.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_FontCompactor.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_FontGlyphPacker.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_FontLib.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_FontProviderHUD.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_FontManager.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_FontMap.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_FontResource.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_ImageCreator.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_ImagePacker.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_ImageResource.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_ImageSupport.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_Input.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_InteractiveObject.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_LoadProcess.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_Loader.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_LoaderUtil.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_LoaderImpl.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_Log.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_MorphCharacter.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_MovieDef.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_PlayerImpl.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_PlayerTasks.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_Resource.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_ResourceHandle.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_Shape.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_ShapeSwf.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_Sprite.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_SpriteDef.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_StaticText.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_Stream.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_String.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_TagLoaders.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_TaskManager.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_TextField.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_TextFieldDef.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_TextureFont.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_WWHelper.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_ZLibFile.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_ZlibSupport.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_FontProviderWin32.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/ImageFiles/DDS_ImageReader.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/ImageFiles/JPEG_ImageCommon.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/ImageFiles/JPEG_ImageReader.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/ImageFiles/JPEG_ImageWriter.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/ImageFiles/PNG_ImageReader.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/ImageFiles/PVR_ImageReader.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/ImageFiles/Render_ImageFile.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/ImageFiles/Render_ImageFileUtil.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/ImageFiles/TGA_ImageReader.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/ImageFiles/TGA_ImageWriter.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/ImageFiles/SIF_ImageReader.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/ImageFiles/SIF_ImageWriter.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Text/Text_Core.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Text/Text_DocView.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Text/Text_FilterDesc.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Text/Text_Highlight.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Text/Text_HTMLParser.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Text/Text_LineBuffer.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Text/Text_StyledText.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Text/Text_WWHelper.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/FontProvider/Render_FontProviderHUD.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/FontProvider/Render_FontProviderWinAPI.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_BufferGeneric.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_Bundle.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_CacheEffect.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_Color.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_Context.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_CxForm.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_DrawableImage.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_DrawableImage_HW.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_DrawableImage_Queue.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_DrawableImage_SW.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_Filters.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_Font.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_FontCacheHandle.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_GlyphCache.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_GlyphFitter.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_GlyphQueue.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_Gradients.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_HAL.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_Hairliner.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_HitTest.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_Image.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_Matrix2x4.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_Matrix3x4.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_Matrix4x4.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_MatrixPool.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_MeshCache.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_MeshKey.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_PathDataPacker.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_Primitive.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_PrimitiveBundle.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_PrimitiveProcessor.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_Queue.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_Rasterizer.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_RectPacker.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_ResizeImage.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_Scale9Grid.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_ScreenToWorld.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_ShapeDataFloat.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_ShapeDataFloatMP.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_ShapeMeshProvider.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_SimpleMeshCache.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_StateBag.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_States.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_Stats.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_Stroker.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_StrokerAA.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_Sync.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_TessCurves.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_Tessellator.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_TextLayout.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_TextMeshProvider.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_TextPrimitiveBundle.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_TextureCacheGeneric.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_TextureUtil.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_TreeCacheNode.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_TreeCacheText.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_TreeLog.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_TreeMesh.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_TreeNode.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_TreeShape.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_TreeText.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_Vertex.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_VertexPath.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Render_Profiler.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Renderer2D.cpp
    ${CMAKE_SOURCE_DIR}/Src/Render/Renderer2DImpl.cpp
    # Net/Socket (4.3): platform socket implementation
    ${CMAKE_SOURCE_DIR}/Src/GFx/Net/GFx_Socket.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/Net/GFx_Socket_Win32.cpp
)
sf_apply_compile_flags(gfx)
target_link_libraries(gfx PUBLIC expat zlib libpng libjpeg pcre)
set_target_properties(gfx PROPERTIES OUTPUT_NAME "libgfx")

# ---------------------------------------------------------------------------
# libgfx_as2 -- ActionScript 2 runtime
# ---------------------------------------------------------------------------
if(SF_USE_SCU)
    # SCU mode: AS2_All.cpp includes all individual AS2 sources
    add_library(gfx_as2 STATIC
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/Audio/AS2_SoundObject.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/XML/AS2_Xml.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/XML/AS2_XmlNode.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/IME/GASIme.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/IME/AS2_IMEManager.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_All.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_AS2Support.cpp
    )
else()
    add_library(gfx_as2 STATIC
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/Audio/AS2_SoundObject.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/XML/AS2_Xml.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/XML/AS2_XmlNode.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/IME/GASIme.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/IME/AS2_IMEManager.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_Action.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_AmpMarker.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_ArrayObject.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_AsBroadcaster.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_AvmButton.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_AvmCharacter.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_AvmSprite.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_AvmTextField.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_BevelFilter.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_BitmapData.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_BitmapFilter.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_BlurFilter.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_BooleanObject.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_Capabilities.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_Color.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_ColorMatrixFilter.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_ColorTransform.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_Date.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_DropShadowFilter.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_ExternalInterface.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_Function.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_GFxValueImpl.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_GlowFilter.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_KeyObject.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_LoadVars.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_Math.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_MatrixObject.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_Mouse.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_MovieClipLoader.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_MovieRoot.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_NumberObject.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_Object.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_PointObject.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_RectangleObject.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_Selection.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_SharedObject.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_Stage.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_StringManager.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_StringObject.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_StyleSheet.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_TextFormat.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_TextSnapshot.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_Timers.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_TransformObject.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS2/AS2_Value.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_AS2Support.cpp
    )
endif()
sf_apply_compile_flags(gfx_as2)
set_target_properties(gfx_as2 PROPERTIES OUTPUT_NAME "libgfx_as2")

# ---------------------------------------------------------------------------
# libgfx_as3 -- ActionScript 3 runtime
# ---------------------------------------------------------------------------
# Non-SCU sources that are always compiled individually (not in _All.cpp)
set(SF_AS3_NON_SCU_SOURCES
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Abc/AS3_Abc_ConstPool.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Abc/AS3_Abc.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Abc/AS3_Abc_Read.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Audio/AS3_SoundObject.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/IME/AS3_IMEManager.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_AS3Support.cpp
    # Below: not in vcxproj, added to resolve link errors
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_SocketThreadMgr.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_SocketBuffer.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_AOTC2.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_AOTC2Execute.cpp
)

if(SF_USE_SCU)
    # SCU mode: _All.cpp aggregators replace individual Obj/ sources
    add_library(gfx_as3 STATIC
        ${SF_AS3_NON_SCU_SOURCES}
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Accessibility/AS3_Obj_Accessibility_All.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_All.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Errors/AS3_Obj_Errors_All.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_All.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/External/AS3_Obj_External_All.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Filters/AS3_Obj_Filters_All.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Geom/AS3_Obj_Geom_All.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Gfx/AS3_Obj_GFx_All.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Media/AS3_Obj_Media_All.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Net/AS3_Obj_Net_All.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Security/AS3_Obj_Security_All.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/System/AS3_Obj_System_All.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Text/AS3_Obj_Text_All.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Ui/AS3_Obj_Ui_All.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Utils/AS3_Obj_Utils_All.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Vec/AS3_Obj_Vec_All.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Xml/AS3_Obj_Xml_All.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Sensors/AS3_Obj_Sensors_All.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/AS3_Obj_All.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_All.cpp
    )
else()
add_library(gfx_as3 STATIC
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Abc/AS3_Abc_ConstPool.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Abc/AS3_Abc.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Abc/AS3_Abc_Read.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Audio/AS3_SoundObject.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/IME/AS3_IMEManager.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Vec/AS3_Obj_Vec_VectorBase.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Vec/AS3_Obj_Vec_Vector.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Vec/AS3_Obj_Vec_Vector_double.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Vec/AS3_Obj_Vec_Vector_int.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Vec/AS3_Obj_Vec_Vector_object.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Vec/AS3_Obj_Vec_Vector_uint.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Vec/AS3_Obj_Vec_Vector_String.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_ActionScriptVersion.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_AVM1Movie.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_Bitmap.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_BitmapDataChannel.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_BitmapData.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_BlendMode.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_CapsStyle.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_DisplayObjectContainer.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_DisplayObject.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_FrameLabel.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_GradientType.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_Graphics.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_IBitmapDrawable.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_InteractiveObject.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_InterpolationMethod.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_JointStyle.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_LineScaleMode.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_Loader.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_LoaderInfo.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_MorphShape.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_MovieClip.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_PixelSnapping.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_Scene.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_Shape.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_SimpleButton.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_SpreadMethod.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_Sprite.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_StageAlign.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_Stage.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_StageDisplayState.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_StageQuality.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_StageScaleMode.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_SWFVersion.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_Shader.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_ShaderData.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_GraphicsBitmapFill.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_GraphicsEndFill.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_GraphicsGradientFill.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_GraphicsPath.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_GraphicsPathCommand.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_GraphicsPathWinding.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_GraphicsSolidFill.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_GraphicsStroke.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_IGraphicsData.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_IGraphicsFill.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_IGraphicsPath.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Errors/AS3_Obj_Errors_EOFError.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Errors/AS3_Obj_Errors_IllegalOperationError.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Errors/AS3_Obj_Errors_InvalidSWFError.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Errors/AS3_Obj_Errors_IOError.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Errors/AS3_Obj_Errors_MemoryError.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Errors/AS3_Obj_Errors_ScriptTimeoutError.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Errors/AS3_Obj_Errors_StackOverflowError.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_ActivityEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_AppLifecycleEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_AsyncErrorEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_ContextMenuEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_DataEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_ErrorEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_Event.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_EventDispatcher.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_EventPhase.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_FocusEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_FullScreenEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_GestureEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_GesturePhase.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_HTTPStatusEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_IEventDispatcher.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_IMEEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_IOErrorEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_KeyboardEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_MouseEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_NetStatusEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_PressAndTapGestureEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_ProgressEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_SecurityErrorEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_StatusEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_SyncEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_TextEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_TimerEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_TouchEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_TransformGestureEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_StageOrientationEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_StageOrientation.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_AccelerometerEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_GeolocationEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Filters/AS3_Obj_Filters_BevelFilter.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Filters/AS3_Obj_Filters_BitmapFilter.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Filters/AS3_Obj_Filters_BitmapFilterQuality.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Filters/AS3_Obj_Filters_BitmapFilterType.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Filters/AS3_Obj_Filters_BlurFilter.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Filters/AS3_Obj_Filters_ColorMatrixFilter.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Filters/AS3_Obj_Filters_ConvolutionFilter.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Filters/AS3_Obj_Filters_DisplacementMapFilter.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Filters/AS3_Obj_Filters_DisplacementMapFilterMode.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Filters/AS3_Obj_Filters_DropShadowFilter.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Filters/AS3_Obj_Filters_GlowFilter.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Filters/AS3_Obj_Filters_GradientBevelFilter.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Filters/AS3_Obj_Filters_GradientGlowFilter.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Filters/AS3_Obj_Filters_ShaderFilter.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Geom/AS3_Obj_Geom_ColorTransform.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Geom/AS3_Obj_Geom_Matrix.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Geom/AS3_Obj_Geom_Matrix3D.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Geom/AS3_Obj_Geom_PerspectiveProjection.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Geom/AS3_Obj_Geom_Point.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Geom/AS3_Obj_Geom_Rectangle.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Geom/AS3_Obj_Geom_Transform.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Geom/AS3_Obj_Geom_Vector3D.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Media/AS3_Obj_Media_Camera.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Media/AS3_Obj_Media_ID3Info.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Media/AS3_Obj_Media_Microphone.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Media/AS3_Obj_Media_SoundChannel.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Media/AS3_Obj_Media_Sound.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Media/AS3_Obj_Media_SoundLoaderContext.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Media/AS3_Obj_Media_SoundMixer.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Media/AS3_Obj_Media_SoundTransform.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Media/AS3_Obj_Media_Video.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Net/AS3_Obj_Net_FileFilter.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Net/AS3_Obj_Net_FileReference.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Net/AS3_Obj_Net_FileReferenceList.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Net/AS3_Obj_Net_IDynamicPropertyOutput.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Net/AS3_Obj_Net_IDynamicPropertyWriter.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Net/AS3_Obj_Net_LocalConnection.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Net/AS3_Obj_Net_NetConnection.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Net/AS3_Obj_Net_NetStream.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Net/AS3_Obj_Net_ObjectEncoding.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Net/AS3_Obj_Net_Responder.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Net/AS3_Obj_Net_SharedObject.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Net/AS3_Obj_Net_SharedObjectFlushStatus.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Net/AS3_Obj_Net_Socket.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Net/AS3_Obj_Net_URLLoader.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Net/AS3_Obj_Net_URLLoaderDataFormat.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Net/AS3_Obj_Net_URLRequest.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Net/AS3_Obj_Net_URLRequestHeader.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Net/AS3_Obj_Net_URLRequestMethod.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Net/AS3_Obj_Net_URLStream.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Net/AS3_Obj_Net_URLVariables.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Net/AS3_Obj_Net_XMLSocket.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/System/AS3_Obj_System_ApplicationDomain.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/System/AS3_Obj_System_Capabilities.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/System/AS3_Obj_System_IMEConversionMode.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/System/AS3_Obj_System_IME.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/System/AS3_Obj_System_LoaderContext.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/System/AS3_Obj_System_Security.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/System/AS3_Obj_System_SecurityDomain.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/System/AS3_Obj_System_SecurityPanel.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/System/AS3_Obj_System_System.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Text/AS3_Obj_Text_AntiAliasType.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Text/AS3_Obj_Text_CSMSettings.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Text/AS3_Obj_Text_Font.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Text/AS3_Obj_Text_FontStyle.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Text/AS3_Obj_Text_FontType.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Text/AS3_Obj_Text_GridFitType.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Text/AS3_Obj_Text_StaticText.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Text/AS3_Obj_Text_StyleSheet.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Text/AS3_Obj_Text_TextColorType.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Text/AS3_Obj_Text_TextDisplayMode.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Text/AS3_Obj_Text_TextFieldAutoSize.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Text/AS3_Obj_Text_TextField.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Text/AS3_Obj_Text_TextFieldType.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Text/AS3_Obj_Text_TextFormatAlign.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Text/AS3_Obj_Text_TextFormat.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Text/AS3_Obj_Text_TextLineMetrics.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Text/AS3_Obj_Text_TextRenderer.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Text/AS3_Obj_Text_TextSnapshot.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Ui/AS3_Obj_Ui_ContextMenuItem.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Ui/AS3_Obj_Ui_ContextMenu.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Ui/AS3_Obj_Ui_ContextMenuBuiltInItems.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Ui/AS3_Obj_Ui_ContextMenuClipboardItems.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Ui/AS3_Obj_Ui_Keyboard.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Ui/AS3_Obj_Ui_KeyLocation.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Ui/AS3_Obj_Ui_Mouse.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Ui/AS3_Obj_Ui_MouseCursor.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Ui/AS3_Obj_Ui_Multitouch.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Ui/AS3_Obj_Ui_MultitouchInputMode.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Utils/AS3_Obj_Utils_ByteArray.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Utils/AS3_Obj_Utils_Dictionary.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Utils/AS3_Obj_Utils_Endian.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Utils/AS3_Obj_Utils_IDataInput.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Utils/AS3_Obj_Utils_IDataOutput.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Utils/AS3_Obj_Utils_IExternalizable.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Utils/AS3_Obj_Utils_Proxy.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Utils/AS3_Obj_Utils_Timer.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/External/AS3_Obj_External_ExternalInterface.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Gfx/AS3_Obj_Gfx_DisplayObjectEx.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Gfx/AS3_Obj_Gfx_SystemEx.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Gfx/AS3_Obj_Gfx_FocusManager.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Gfx/AS3_Obj_Gfx_Extensions.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Gfx/AS3_Obj_Gfx_FocusEventEx.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Gfx/AS3_Obj_Gfx_GamePad.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Gfx/AS3_Obj_Gfx_GamePadAnalogEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Gfx/AS3_Obj_Gfx_InteractiveObjectEx.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Gfx/AS3_Obj_Gfx_KeyboardEventEx.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Gfx/AS3_Obj_Gfx_MouseEventEx.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Gfx/AS3_Obj_Gfx_MouseCursorEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Gfx/AS3_Obj_Gfx_TextEventEx.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Gfx/AS3_Obj_Gfx_IMEEventEx.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Gfx/AS3_Obj_Gfx_IMEEx.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Gfx/AS3_Obj_Gfx_IMECandidateListStyle.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Gfx/AS3_Obj_Gfx_TextFieldEx.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Accessibility/AS3_Obj_Accessibility_Accessibility.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Accessibility/AS3_Obj_Accessibility_AccessibilityImplementation.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Accessibility/AS3_Obj_Accessibility_AccessibilityProperties.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Accessibility/AS3_Obj_Accessibility_ISearchableText.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Accessibility/AS3_Obj_Accessibility_ISimpleTextSelection.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Security/AS3_Obj_Security_XMLSignatureValidator.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Xml/AS3_Obj_Xml_XMLDocument.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Xml/AS3_Obj_Xml_XMLNode.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Xml/AS3_Obj_Xml_XMLNodeType.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Sensors/AS3_Obj_Sensors_Accelerometer.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Sensors/AS3_Obj_Sensors_Geolocation.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/AS3_Obj_Activation.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/AS3_Obj_ArgumentError.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/AS3_Obj_Array.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/AS3_Obj_Boolean.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/AS3_Obj_Catch.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/AS3_Obj_ClassClass.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/AS3_Obj_Date.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/AS3_Obj_DefinitionError.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/AS3_Obj_Error.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/AS3_Obj_EvalError.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/AS3_Obj_Function.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/AS3_Obj_Global.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/AS3_Obj_int.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/AS3_Obj_Math.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/AS3_Obj_Namespace.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/AS3_Obj_Number.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/AS3_Obj_Object.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/AS3_Obj_QName.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/AS3_Obj_RangeError.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/AS3_Obj_ReferenceError.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/AS3_Obj_RegExp.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/AS3_Obj_SecurityError.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/AS3_Obj_String.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/AS3_Obj_SyntaxError.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/AS3_Obj_TypeError.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/AS3_Obj_uint.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/AS3_Obj_UninitializedError.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/AS3_Obj_URIError.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/AS3_Obj_UserDefined.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/AS3_Obj_VerifyError.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/AS3_Obj_XML.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/AS3_Obj_XMLList.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_AbcDataBuffer.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_AOTC.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_AsString.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_AvmBitmap.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_AvmButton.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_AvmDisplayObjContainer.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_AvmDisplayObj.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_AvmInteractiveObj.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_AvmLoader.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_AvmSprite.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_AvmStage.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_AvmStaticText.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_AvmTextField.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_CodeExecute.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_GC.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_GFxValueImpl.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_Index.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_Instance.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_IntervalTimer.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_MovieRoot.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_Object.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_ObjCollector.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_Slot.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_StringManager.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_Tracer.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_Traits.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_Value.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_VM.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_VMRead.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_VTable.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_XMLSupport.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_UDBase.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_ValueStack.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/GFx_AS3Support.cpp
    # Below: not in vcxproj, added to resolve link errors
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/AS3_Obj_Domain.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_SocketThreadMgr.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_SocketBuffer.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_AOTC2.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/AS3_AOTC2Execute.cpp
)
endif()
sf_apply_compile_flags(gfx_as3)
set_target_properties(gfx_as3 PROPERTIES OUTPUT_NAME "libgfx_as3")

# ---------------------------------------------------------------------------
# libgfx_air -- Adobe AIR runtime extension
# ---------------------------------------------------------------------------
if(SF_USE_SCU)
    # SCU mode: _AIR_All.cpp aggregators replace individual sources
    add_library(gfx_air STATIC
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Desktop/AS3_Obj_Desktop_AIR_All.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_AIR_All.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_AIR_All.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Filesystem/AS3_Obj_Filesystem_AIR_All.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Html/AS3_Obj_Html_AIR_All.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Net/AS3_Obj_Net_AIR_All.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Printing/AS3_Obj_Printing_AIR_All.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Sampler/AS3_Obj_Sampler_AIR_All.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Security/AS3_Obj_Security_AIR_All.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Sensors/AS3_Obj_Sensors_AIR_All.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Utils/AS3_Obj_Utils_AIR_All.cpp
    )
else()
add_library(gfx_air STATIC
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Desktop/AS3_Obj_Desktop_Clipboard.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Desktop/AS3_Obj_Desktop_ClipboardFormats.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Desktop/AS3_Obj_Desktop_ClipboardTransferMode.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Desktop/AS3_Obj_Desktop_Icon.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Desktop/AS3_Obj_Desktop_InteractiveIcon.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Desktop/AS3_Obj_Desktop_DockIcon.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Desktop/AS3_Obj_Desktop_NativeApplication.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Desktop/AS3_Obj_Desktop_NativeDragActions.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Desktop/AS3_Obj_Desktop_NativeDragManager.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Desktop/AS3_Obj_Desktop_NativeDragOptions.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Desktop/AS3_Obj_Desktop_NotificationType.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Desktop/AS3_Obj_Desktop_SystemTrayIcon.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Desktop/AS3_Obj_Desktop_Updater.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_FocusDirection.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_NativeMenu.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_NativeMenuItem.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_NativeWindow.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_NativeWindowDisplayState.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_NativeWindowInitOptions.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_NativeWindowResize.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_NativeWindowSystemChrome.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_NativeWindowType.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Display/AS3_Obj_Display_Screen.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_BrowserInvokeEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_DRMAuthenticateEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_DRMErrorEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_DRMStatusEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_FileListEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_HTMLUncaughtScriptExceptionEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_InvokeEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_NativeDragEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_NativeWindowBoundsEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_NativeWindowDisplayStateEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_OutputProgressEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Events/AS3_Obj_Events_ScreenMouseEvent.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Filesystem/AS3_Obj_Filesystem_File.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Filesystem/AS3_Obj_Filesystem_FileMode.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Filesystem/AS3_Obj_Filesystem_FileStream.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Html/AS3_Obj_Html_HTMLHistoryItem.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Html/AS3_Obj_Html_HTMLHost.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Html/AS3_Obj_Html_HTMLLoader.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Html/AS3_Obj_Html_HTMLPDFCapability.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Html/AS3_Obj_Html_HTMLWindowCreateOptions.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Net/AS3_Obj_Net_URLRequestDefaults.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Printing/AS3_Obj_Printing_PrintJob.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Printing/AS3_Obj_Printing_PrintJobOptions.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Printing/AS3_Obj_Printing_PrintJobOrientation.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Sampler/AS3_Obj_Sampler_DeleteObjectSample.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Sampler/AS3_Obj_Sampler_NewObjectSample.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Sampler/AS3_Obj_Sampler_Sample.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Sampler/AS3_Obj_Sampler_StackFrame.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Security/AS3_Obj_Security_IURIDereferencer.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Security/AS3_Obj_Security_ReferencesValidationSetting.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Security/AS3_Obj_Security_RevocationCheckSettings.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Security/AS3_Obj_Security_SignatureStatus.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Security/AS3_Obj_Security_SignerTrustSettings.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Utils/AS3_Obj_Utils_CompressionAlgorithm.cpp
    ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Sensors/AS3_Obj_Sensors_Geolocation.cpp
)
endif()
sf_apply_compile_flags(gfx_air)
set_target_properties(gfx_air PROPERTIES OUTPUT_NAME "libgfx_air")

# ---------------------------------------------------------------------------
# libgfxexpat -- XML parser wrapper
# ---------------------------------------------------------------------------
add_library(gfxexpat STATIC ${CMAKE_SOURCE_DIR}/Src/XML/XML_Expat.cpp)
sf_apply_compile_flags(gfxexpat)
target_link_libraries(gfxexpat PRIVATE expat)
set_target_properties(gfxexpat PROPERTIES OUTPUT_NAME "libgfxexpat")

# ---------------------------------------------------------------------------
# libgfxsound_fmod -- FMOD sound renderer
# ---------------------------------------------------------------------------
add_library(gfxsound_fmod STATIC
    ${CMAKE_SOURCE_DIR}/Src/Sound/Sound_Sound.cpp
    ${CMAKE_SOURCE_DIR}/Src/Sound/Sound_SoundRenderer.cpp
    ${CMAKE_SOURCE_DIR}/Src/Sound/Sound_SoundRendererFMOD.cpp
)
sf_apply_compile_flags(gfxsound_fmod)
target_link_libraries(gfxsound_fmod PRIVATE fmod)
set_target_properties(gfxsound_fmod PROPERTIES OUTPUT_NAME "libgfxsound_fmod")

# ---------------------------------------------------------------------------
# libgfxvideo -- CRI Mana (Sofdec2) video playback
# ---------------------------------------------------------------------------
if(SF_ENABLE_VIDEO)
    # Locate CRI Mana 2019 SDK (headers + prebuilt libs)
    set(SF_CRI_SDK_DIR "${CMAKE_SOURCE_DIR}/3rdParty/cri2019/pc" CACHE PATH
        "Path to CRI Mana SDK (must contain include/ and libs/)")
    if(NOT EXISTS "${SF_CRI_SDK_DIR}/include/cri_mana.h")
        message(FATAL_ERROR
            "SF_ENABLE_VIDEO=ON but CRI Mana SDK not found at ${SF_CRI_SDK_DIR}\n"
            "Set SF_CRI_SDK_DIR to the directory containing include/cri_mana.h")
    endif()

    # CRI imported libraries (Mana + dependencies)
    add_library(cri_mana       STATIC IMPORTED)
    add_library(cri_base       STATIC IMPORTED)
    add_library(cri_file_system STATIC IMPORTED)
    add_library(cri_atom       STATIC IMPORTED)
    add_library(cri_framework  STATIC IMPORTED)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set_target_properties(cri_mana       PROPERTIES IMPORTED_LOCATION
            "${SF_CRI_SDK_DIR}/libs/x64/cri_mana_pcx64.lib")
        set_target_properties(cri_base       PROPERTIES IMPORTED_LOCATION
            "${SF_CRI_SDK_DIR}/libs/x64/cri_base_pcx64.lib")
        set_target_properties(cri_file_system PROPERTIES IMPORTED_LOCATION
            "${SF_CRI_SDK_DIR}/libs/x64/cri_file_system_pcx64.lib")
        set_target_properties(cri_atom       PROPERTIES IMPORTED_LOCATION
            "${SF_CRI_SDK_DIR}/libs/x64/cri_atom_pcx64.lib")
        set_target_properties(cri_framework  PROPERTIES IMPORTED_LOCATION
            "${SF_CRI_SDK_DIR}/libs/x64/cri_framework_pcx64.lib")
    else()
        set_target_properties(cri_mana       PROPERTIES IMPORTED_LOCATION
            "${SF_CRI_SDK_DIR}/libs/x86/cri_mana_pcx86.lib")
        set_target_properties(cri_base       PROPERTIES IMPORTED_LOCATION
            "${SF_CRI_SDK_DIR}/libs/x86/cri_base_pcx86.lib")
        set_target_properties(cri_file_system PROPERTIES IMPORTED_LOCATION
            "${SF_CRI_SDK_DIR}/libs/x86/cri_file_system_pcx86.lib")
        set_target_properties(cri_atom       PROPERTIES IMPORTED_LOCATION
            "${SF_CRI_SDK_DIR}/libs/x86/cri_atom_pcx86.lib")
        set_target_properties(cri_framework  PROPERTIES IMPORTED_LOCATION
            "${SF_CRI_SDK_DIR}/libs/x86/cri_framework_pcx86.lib")
    endif()

    add_library(gfxvideo STATIC
        ${CMAKE_SOURCE_DIR}/Src/Video/Video_Video.cpp
        ${CMAKE_SOURCE_DIR}/Src/Video/Video_VideoPC.cpp
        ${CMAKE_SOURCE_DIR}/Src/Video/Video_VideoPlayerImpl.cpp
        ${CMAKE_SOURCE_DIR}/Src/Video/Video_VideoCharacter.cpp
        ${CMAKE_SOURCE_DIR}/Src/Video/Video_VideoImage.cpp
        ${CMAKE_SOURCE_DIR}/Src/Video/Video_VideoAS2.cpp
        ${CMAKE_SOURCE_DIR}/Src/Video/Video_VideoAS3.cpp
        ${CMAKE_SOURCE_DIR}/Src/Video/Video_CriHeap.cpp
        ${CMAKE_SOURCE_DIR}/Src/Video/Video_CriMvFileReader.cpp
        ${CMAKE_SOURCE_DIR}/Src/Video/Video_CriMvSoundGateway.cpp
        ${CMAKE_SOURCE_DIR}/Src/Video/Video_CriMvSystemTimer.cpp
        ${CMAKE_SOURCE_DIR}/Src/Video/Video_SystemSoundInterface.cpp
        ${CMAKE_SOURCE_DIR}/Src/Video/Video_VideoSoundSystemDX8.cpp
        ${CMAKE_SOURCE_DIR}/Src/Video/AS2/AS2_NetConnection.cpp
        ${CMAKE_SOURCE_DIR}/Src/Video/AS2/AS2_NetStream.cpp
        ${CMAKE_SOURCE_DIR}/Src/Video/AS2/AS2_VideoObject.cpp
        ${CMAKE_SOURCE_DIR}/Src/Video/AS3/AS3_VideoCharacter.cpp
        ${CMAKE_SOURCE_DIR}/Src/Video/AS3/AS3_VideoProviderNetStream.cpp
        # AS3 runtime objects needed by video (Video, NetStream class info)
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Media/AS3_Obj_Media_Video.cpp
        ${CMAKE_SOURCE_DIR}/Src/GFx/AS3/Obj/Net/AS3_Obj_Net_NetStream.cpp
    )
    sf_apply_compile_flags(gfxvideo)
    target_include_directories(gfxvideo PRIVATE "${SF_CRI_SDK_DIR}/include")
    target_link_libraries(gfxvideo PUBLIC cri_mana cri_atom cri_file_system cri_base cri_framework)
    target_link_libraries(gfxvideo INTERFACE dsound)
    set_target_properties(gfxvideo PROPERTIES OUTPUT_NAME "libgfxvideo")
endif()

# ---------------------------------------------------------------------------
# libgfxrender_vulkan -- Vulkan renderer
# ---------------------------------------------------------------------------
if(SF_RENDERER_VULKAN)
    find_package(Vulkan REQUIRED)

    add_library(gfxrender_vulkan STATIC
        ${CMAKE_SOURCE_DIR}/Src/Render/Vulkan/Vulkan_HAL.cpp
        ${CMAKE_SOURCE_DIR}/Src/Render/Vulkan/Vulkan_MeshCache.cpp
        ${CMAKE_SOURCE_DIR}/Src/Render/Vulkan/Vulkan_Shader.cpp
        ${CMAKE_SOURCE_DIR}/Src/Render/Vulkan/Vulkan_ShaderDescs.cpp
        ${CMAKE_SOURCE_DIR}/Src/Render/Vulkan/Vulkan_ShaderBinary.cpp
        ${CMAKE_SOURCE_DIR}/Src/Render/Vulkan/Vulkan_Texture.cpp
        ${CMAKE_SOURCE_DIR}/Src/Render/Vulkan/Vulkan_Events.cpp
    )
    sf_apply_compile_flags(gfxrender_vulkan)
    target_link_libraries(gfxrender_vulkan PUBLIC Vulkan::Vulkan)
    target_compile_definitions(gfxrender_vulkan PUBLIC FXPLAYER_RENDER_VULKAN)
    set_target_properties(gfxrender_vulkan PROPERTIES OUTPUT_NAME "libgfxrender_vulkan")
endif()

# ---------------------------------------------------------------------------
# libgfxrender_d3d12 -- Direct3D 12 renderer
# ---------------------------------------------------------------------------
if(SF_RENDERER_D3D12)
    add_library(gfxrender_d3d12 STATIC
        ${CMAKE_SOURCE_DIR}/Src/Render/D3D12/D3D12_HAL.cpp
        ${CMAKE_SOURCE_DIR}/Src/Render/D3D12/D3D12_MeshCache.cpp
        ${CMAKE_SOURCE_DIR}/Src/Render/D3D12/D3D12_Shader.cpp
        ${CMAKE_SOURCE_DIR}/Src/Render/D3D12/D3D12_ShaderDescs.cpp
        ${CMAKE_SOURCE_DIR}/Src/Render/D3D12/D3D12_ShaderBinary.cpp
        ${CMAKE_SOURCE_DIR}/Src/Render/D3D12/D3D12_Texture.cpp
    )
    sf_apply_compile_flags(gfxrender_d3d12)
    target_compile_definitions(gfxrender_d3d12 PUBLIC SF_D3D_VERSION=12 FXPLAYER_RENDER_DIRECT3D)
    target_link_libraries(gfxrender_d3d12 PUBLIC d3d12 dxgi d3dcompiler dxguid)
    set_target_properties(gfxrender_d3d12 PROPERTIES OUTPUT_NAME "libgfxrender_d3d12")
endif()

# ---------------------------------------------------------------------------
# libgfxrender_d3d1x -- Direct3D 10/11 renderer
# ---------------------------------------------------------------------------
if(SF_RENDERER_D3D11)
    add_library(gfxrender_d3d1x STATIC
        ${CMAKE_SOURCE_DIR}/Src/Render/D3D1x/D3D1x_HAL.cpp
        ${CMAKE_SOURCE_DIR}/Src/Render/D3D1x/D3D1x_MeshCache.cpp
        ${CMAKE_SOURCE_DIR}/Src/Render/D3D1x/D3D1x_Shader.cpp
        ${CMAKE_SOURCE_DIR}/Src/Render/D3D1x/D3D1x_ShaderDescs.cpp
        ${CMAKE_SOURCE_DIR}/Src/Render/D3D1x/D3D1x_ShaderBinary.cpp
        ${CMAKE_SOURCE_DIR}/Src/Render/D3D1x/D3D1x_Texture.cpp
    )
    sf_apply_compile_flags(gfxrender_d3d1x)
    target_compile_definitions(gfxrender_d3d1x PUBLIC SF_D3D_VERSION=11 FXPLAYER_RENDER_DIRECT3D)
    target_link_libraries(gfxrender_d3d1x PUBLIC d3d11 dxgi d3dcompiler dxguid)
    set_target_properties(gfxrender_d3d1x PROPERTIES OUTPUT_NAME "libgfxrender_d3d1x")
endif()

# ---------------------------------------------------------------------------
# libgfxrender_d3d9 -- Direct3D 9 renderer
# ---------------------------------------------------------------------------
if(SF_RENDERER_D3D9)
    add_library(gfxrender_d3d9 STATIC
        ${CMAKE_SOURCE_DIR}/Src/Render/D3D9/D3D9_HAL.cpp
        ${CMAKE_SOURCE_DIR}/Src/Render/D3D9/D3D9_MeshCache.cpp
        ${CMAKE_SOURCE_DIR}/Src/Render/D3D9/D3D9_Shader.cpp
        ${CMAKE_SOURCE_DIR}/Src/Render/D3D9/D3D9_ShaderBinary.cpp
        ${CMAKE_SOURCE_DIR}/Src/Render/D3D9/D3D9_ShaderDescs.cpp
        ${CMAKE_SOURCE_DIR}/Src/Render/D3D9/D3D9_Texture.cpp
    )
    sf_apply_compile_flags(gfxrender_d3d9)
    target_compile_definitions(gfxrender_d3d9 PUBLIC SF_D3D_VERSION=9 FXPLAYER_RENDER_DIRECT3D)
    target_link_libraries(gfxrender_d3d9 PUBLIC d3d9 d3dx9)
    set_target_properties(gfxrender_d3d9 PROPERTIES OUTPUT_NAME "libgfxrender_d3d9")
endif()

# ---------------------------------------------------------------------------
# libgfxrender_d3d1x -- Direct3D 10 renderer (SF_D3D_VERSION=10)
# ---------------------------------------------------------------------------
if(SF_RENDERER_D3D10)
    add_library(gfxrender_d3d10 STATIC
        ${CMAKE_SOURCE_DIR}/Src/Render/D3D1x/D3D1x_HAL.cpp
        ${CMAKE_SOURCE_DIR}/Src/Render/D3D1x/D3D1x_MeshCache.cpp
        ${CMAKE_SOURCE_DIR}/Src/Render/D3D1x/D3D1x_Shader.cpp
        ${CMAKE_SOURCE_DIR}/Src/Render/D3D1x/D3D1x_ShaderDescs.cpp
        ${CMAKE_SOURCE_DIR}/Src/Render/D3D1x/D3D1x_ShaderBinary.cpp
        ${CMAKE_SOURCE_DIR}/Src/Render/D3D1x/D3D1x_Texture.cpp
    )
    sf_apply_compile_flags(gfxrender_d3d10)
    target_compile_definitions(gfxrender_d3d10 PUBLIC SF_D3D_VERSION=10 FXPLAYER_RENDER_DIRECT3D)
    target_link_libraries(gfxrender_d3d10 PUBLIC d3d10_1 d3d10 dxgi d3dcompiler dxguid)
    set_target_properties(gfxrender_d3d10 PROPERTIES OUTPUT_NAME "libgfxrender_d3d10")
endif()

# ---------------------------------------------------------------------------
# libgfxrender_gl -- OpenGL renderer
# ---------------------------------------------------------------------------
if(SF_RENDERER_GL)
    add_library(gfxrender_gl STATIC
        ${CMAKE_SOURCE_DIR}/Src/Render/GL/GL_Extensions.cpp
        ${CMAKE_SOURCE_DIR}/Src/Render/GL/GL_HAL.cpp
        ${CMAKE_SOURCE_DIR}/Src/Render/GL/GL_MeshCache.cpp
        ${CMAKE_SOURCE_DIR}/Src/Render/GL/GL_Shader.cpp
        ${CMAKE_SOURCE_DIR}/Src/Render/GL/GL_ShaderDescs.cpp
        ${CMAKE_SOURCE_DIR}/Src/Render/GL/GL_ShaderSource.cpp
        ${CMAKE_SOURCE_DIR}/Src/Render/GL/GL_Texture.cpp
    )
    sf_apply_compile_flags(gfxrender_gl)
    target_compile_definitions(gfxrender_gl PUBLIC FXPLAYER_RENDER_OPENGL)
    target_link_libraries(gfxrender_gl PUBLIC opengl32 glu32)
    set_target_properties(gfxrender_gl PROPERTIES OUTPUT_NAME "libgfxrender_gl")
endif()

# ---------------------------------------------------------------------------
# libgfxplatform -- platform integration (renderer-specific source selection)
# ---------------------------------------------------------------------------
set(SF_PLATFORM_COMMON_SOURCES
    ${CMAKE_SOURCE_DIR}/Src/Platform/Platform.cpp
    ${CMAKE_SOURCE_DIR}/Src/Platform/Platform_CommandLine.cpp
    ${CMAKE_SOURCE_DIR}/Src/Platform/Platform_CoreTest.cpp
    ${CMAKE_SOURCE_DIR}/Src/Platform/Platform_RenderThread.cpp
    ${CMAKE_SOURCE_DIR}/Src/Platform/Platform_RTCommandQueue.cpp
    ${CMAKE_SOURCE_DIR}/Src/Platform/Platform_SystemCursorManager.cpp
    ${CMAKE_SOURCE_DIR}/Src/Platform/Platform_RenderHALThread.cpp
    ${CMAKE_SOURCE_DIR}/Src/Platform/Platform_WinAPI.cpp
    ${CMAKE_SOURCE_DIR}/Src/Platform/Platform_XInput.cpp
)

# D3D12 platform
if(SF_RENDERER_D3D12)
    set(_d3d12_platform_sources ${SF_PLATFORM_COMMON_SOURCES}
        ${CMAKE_SOURCE_DIR}/Src/Platform/Platform_D3D1x.cpp
    )
    # Platform_WinAPI_GL.cpp instantiates GL::HAL — only include when GL renderer is available
    if(SF_RENDERER_GL)
        list(APPEND _d3d12_platform_sources ${CMAKE_SOURCE_DIR}/Src/Platform/Platform_WinAPI_GL.cpp)
    endif()
    add_library(gfxplatform_d3d12 STATIC ${_d3d12_platform_sources})
    sf_apply_compile_flags(gfxplatform_d3d12)
    target_compile_definitions(gfxplatform_d3d12 PUBLIC SF_D3D_VERSION=12)
    if(SF_RENDERER_GL)
        target_link_libraries(gfxplatform_d3d12 INTERFACE opengl32 gfxrender_gl)
    endif()
    set_target_properties(gfxplatform_d3d12 PROPERTIES OUTPUT_NAME "libgfxplatform_d3d12")
endif()

# D3D11 platform
if(SF_RENDERER_D3D11)
    set(_d3d11_platform_sources ${SF_PLATFORM_COMMON_SOURCES}
        ${CMAKE_SOURCE_DIR}/Src/Platform/Platform_D3D1x.cpp
    )
    # Platform_WinAPI_GL.cpp instantiates GL::HAL — only include when GL renderer is available
    if(SF_RENDERER_GL)
        list(APPEND _d3d11_platform_sources ${CMAKE_SOURCE_DIR}/Src/Platform/Platform_WinAPI_GL.cpp)
    endif()
    add_library(gfxplatform_d3d11 STATIC ${_d3d11_platform_sources})
    sf_apply_compile_flags(gfxplatform_d3d11)
    target_compile_definitions(gfxplatform_d3d11 PUBLIC SF_D3D_VERSION=11)
    if(SF_RENDERER_GL)
        target_link_libraries(gfxplatform_d3d11 INTERFACE opengl32 gfxrender_gl)
    endif()
    set_target_properties(gfxplatform_d3d11 PROPERTIES OUTPUT_NAME "libgfxplatform_d3d11")
endif()

# D3D10 platform
if(SF_RENDERER_D3D10)
    set(_d3d10_platform_sources ${SF_PLATFORM_COMMON_SOURCES}
        ${CMAKE_SOURCE_DIR}/Src/Platform/Platform_D3D1x.cpp
    )
    if(SF_RENDERER_GL)
        list(APPEND _d3d10_platform_sources ${CMAKE_SOURCE_DIR}/Src/Platform/Platform_WinAPI_GL.cpp)
    endif()
    add_library(gfxplatform_d3d10 STATIC ${_d3d10_platform_sources})
    sf_apply_compile_flags(gfxplatform_d3d10)
    target_compile_definitions(gfxplatform_d3d10 PUBLIC SF_D3D_VERSION=10)
    if(SF_RENDERER_GL)
        target_link_libraries(gfxplatform_d3d10 INTERFACE opengl32 gfxrender_gl)
    endif()
    set_target_properties(gfxplatform_d3d10 PROPERTIES OUTPUT_NAME "libgfxplatform_d3d10")
endif()

# D3D9 platform
if(SF_RENDERER_D3D9)
    add_library(gfxplatform_d3d9 STATIC
        ${SF_PLATFORM_COMMON_SOURCES}
        ${CMAKE_SOURCE_DIR}/Src/Platform/Platform_D3D9.cpp
    )
    sf_apply_compile_flags(gfxplatform_d3d9)
    target_compile_definitions(gfxplatform_d3d9 PUBLIC SF_D3D_VERSION=9)
    set_target_properties(gfxplatform_d3d9 PROPERTIES OUTPUT_NAME "libgfxplatform_d3d9")
endif()

# GL platform
if(SF_RENDERER_GL)
    add_library(gfxplatform_gl STATIC
        ${SF_PLATFORM_COMMON_SOURCES}
        ${CMAKE_SOURCE_DIR}/Src/Platform/Platform_D3D1x.cpp
        ${CMAKE_SOURCE_DIR}/Src/Platform/Platform_WinAPI_GL.cpp
    )
    sf_apply_compile_flags(gfxplatform_gl)
    set_target_properties(gfxplatform_gl PROPERTIES OUTPUT_NAME "libgfxplatform_gl")
endif()

# ---------------------------------------------------------------------------
# sf_core -- convenience target linking all core SDK libraries
# ---------------------------------------------------------------------------
add_library(sf_core INTERFACE)
target_link_libraries(sf_core INTERFACE gfx gfx_as2 gfx_as3 gfx_air gfxexpat)

