INCLUDES += -ISrc

ifneq ($(strip $(shell [ -e Projects/$(PD)/Makefile ] && echo 1)),1)
ifneq ($(strip $(call HAS_HEADER,pthread.h)),)
GFXlibgfxSRCS += Src/Kernel/SF_ThreadsPthread.cpp
endif

ifneq ($(strip $(call HAS_HEADER,GL/gl.h)),)
$(call BUILD_GFX_LIB,libgfxrender_gl)
$(GFXlibgfxrender_glSRCS_ALL): Src/Render/GL/GL_ShaderDescs.h
endif

ifeq ($(P),local)
# Disabled font provider temporarily
#ifneq ($(strip $(shell which freetype-config 2>/dev/null)),)
#GFXlibgfxSRCS += Src/GFx/GFx_FontProviderFT2.cpp
#endif
endif
endif

ifeq ($(shell [ -f Src/GFx/GFx_Loader.cpp ] && echo 1),1)

$(call BUILD_GFX_LIB,libgfx,Src/GFx/GFx_LoaderUtil.cpp)
$(call BUILD_GFX_LIB_SCU,libgfx_as2)
$(call BUILD_GFX_LIB_SCU,libgfx_as3)
$(call BUILD_GFX_LIB_SCU,libgfx_air)

endif

$(call BUILD_GFX_LIB,libgfxexpat)
$(call BUILD_GFX_LIB,libgfxplatform)
