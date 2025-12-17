/**************************************************************************

Filename    :   GFx_Render.h
Content     :   Convenience header collection for  
Created     :   July 2010 
Authors     :   Automatically generated 

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
 
#ifndef INC_GFx_Render_H 
#define INC_GFx_Render_H 
 
#include "../Src/Render/Renderer2D.h" 		
#include "../Src/Render/Renderer2DImpl.h" 		
#include "../Src/Render/Render_Bundle.h" 		
#include "../Src/Render/Render_CacheEffect.h" 		
#include "../Src/Render/Render_Color.h" 		
#include "../Src/Render/Render_Constants.h" 		
#include "../Src/Render/Render_Containers.h" 		
#include "../Src/Render/Render_Context.h" 		
#include "../Src/Render/Render_CxForm.h" 		
#include "../Src/Render/Render_Font.h" 		
#include "../Src/Render/Render_GlyphCache.h" 		
#include "../Src/Render/Render_GlyphCacheConfig.h" 		
#include "../Src/Render/Render_GlyphFitter.h" 		
#include "../Src/Render/Render_GlyphParam.h" 		
#include "../Src/Render/Render_GlyphQueue.h" 		
#include "../Src/Render/Render_Gradients.h" 		
#include "../Src/Render/Render_Hairliner.h" 		
#include "../Src/Render/Render_HAL.h" 		
#include "../Src/Render/Render_Image.h" 		
#include "../Src/Render/Render_ImageFiles.h" 		
#include "../Src/Render/Render_Math2D.h" 		
#include "../Src/Render/Render_Matrix2x4.h" 		
#include "../Src/Render/Render_Matrix3x4.h" 		
#include "../Src/Render/Render_Matrix4x4.h" 		
#include "../Src/Render/Render_MatrixPool.h" 		
#include "../Src/Render/Render_MemoryManager.h" 		
#include "../Src/Render/Render_MeshCache.h" 		
#include "../Src/Render/Render_MeshCacheConfig.h" 		
#include "../Src/Render/Render_MeshKey.h" 		
#include "../Src/Render/Render_PathDataPacker.h" 		
#include "../Src/Render/Render_Point3.h" 		
#include "../Src/Render/Render_Primitive.h" 		
#include "../Src/Render/Render_PrimitiveBundle.h" 		
#include "../Src/Render/Render_Queue.h" 		
#include "../Src/Render/Render_Rasterizer.h" 		
#include "../Src/Render/Render_ResizeImage.h" 		
#include "../Src/Render/Render_Scale9Grid.h" 		
#include "../Src/Render/Render_ScreenToWorld.h" 		
#include "../Src/Render/Render_Shader.h" 		
#include "../Src/Render/Render_ShapeDataDefs.h" 		
#include "../Src/Render/Render_ShapeDataFloat.h" 		
#include "../Src/Render/Render_ShapeDataFloatMP.h" 		
#include "../Src/Render/Render_ShapeDataPacked.h" 		
#include "../Src/Render/Render_ShapeMeshProvider.h" 		
#include "../Src/Render/Render_SimpleMeshCache.h" 		
#include "../Src/Render/Render_StateBag.h" 		
#include "../Src/Render/Render_States.h" 		
#include "../Src/Render/Render_Stats.h" 		
#include "../Src/Render/Render_Stroker.h" 		
#include "../Src/Render/Render_StrokerAA.h" 		
#include "../Src/Render/Render_TessDefs.h" 		
#include "../Src/Render/Render_Tessellator.h" 		
#include "../Src/Render/Render_TextLayout.h" 		
#include "../Src/Render/Render_TextMeshProvider.h" 		
#include "../Src/Render/Render_TextureUtil.h" 		
#include "../Src/Render/Render_ThreadCommandQueue.h" 		
#include "../Src/Render/Render_ToleranceParams.h" 		
#include "../Src/Render/Render_TreeCacheNode.h" 		
#include "../Src/Render/Render_TreeLog.h" 		
#include "../Src/Render/Render_TreeNode.h" 		
#include "../Src/Render/Render_TreeShape.h" 		
#include "../Src/Render/Render_TreeText.h" 		
#include "../Src/Render/Render_Twips.h" 		
#include "../Src/Render/Render_Types2D.h" 		
#include "../Src/Render/Render_Vertex.h" 		
#include "../Src/Render/Render_Viewport.h" 		
#include "../Src/Render/FontProvider/Render_FontProviderHUD.h" 		
#if defined(SF_OS_WIN32) && !defined(SF_OS_WINMETRO)
#include "../Src/Render/FontProvider/Render_FontProviderWinAPI.h" 	
#endif		
#include "../Src/Render/ImageFiles/DDS_ImageFile.h" 		
#include "../Src/Render/ImageFiles/JPEG_ImageFile.h" 		
#include "../Src/Render/ImageFiles/PNG_ImageFile.h" 		
#include "../Src/Render/ImageFiles/Render_ImageFile.h" 		
#include "../Src/Render/ImageFiles/Render_ImageFileUtil.h" 		
#include "../Src/Render/ImageFiles/TGA_ImageFile.h" 		
#include "../Src/Render/Text/Text_Core.h" 		
#include "../Src/Render/Text/Text_DocView.h" 		
#include "../Src/Render/Text/Text_FilterDesc.h" 		
#include "../Src/Render/Text/Text_FontManager.h" 		
#include "../Src/Render/Text/Text_Highlight.h" 		
#include "../Src/Render/Text/Text_LineBuffer.h" 		
#include "../Src/Render/Text/Text_SGMLParser.h" 		
#include "../Src/Render/Text/Text_StyledText.h" 		
 
#endif     // INC_GFx_Render_H 
