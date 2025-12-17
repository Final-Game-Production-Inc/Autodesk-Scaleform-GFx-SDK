
DATADIR := $(WII_SDKDIR)/dvddata/FxPlayer

PP := 
CXX1 := "$(shell echo $(WII_CWDIR))/PowerPC_EABI_TOOLS/Command_Line_Tools/mwcceppc"
P_AR := "$(shell echo $(WII_CWDIR))/PowerPC_EABI_TOOLS/Command_Line_Tools/mwldeppc" -proc gekko -library -o
P_RANLIB := true
WIICFLAGS := -i- -I$(OBJDIRP)/3rdParty/zlib-1.2.5 -I$(OBJDIRP)/3rdParty/jpeg-6b
WIICFLAGS += "-I$(shell cygpath -m $(WII_SDKDIR)/include)" \
            "-I$(shell cygpath -m $(WII_SDKDIR)/include/revolution)" \
            -ir "$(shell cygpath -m $(WII_CWDIR)/PowerPC_EABI_Support/Msl/Msl_c)" \
            -ir "$(shell cygpath -m $(WII_CWDIR)/PowerPC_EABI_Support/Msl/Msl_c++)" \
            "-I$(shell cygpath -m $(WII_CWDIR)/PowerPC_EABI_Support/Runtime/Inc)" \
            "-I$(shell cygpath -am $(OBJDIRP)/include)"

CXX2     := $(shell cygpath -u $(OBJDIR)/ccw)
ARW      := $(shell cygpath -u $(OBJDIR)/arw)
CXX      := $(CXX2)

CXX_Release  := -opt space,level=2 -inline auto -DNDEBUG
CXX_DebugOpt := -opt space,level=2 -gdwarf-2 -inline off -D_DEBUG -DSF_BUILD_DEBUG
CXX_Debug    := -gdwarf-2 -inline off -opt off -D_DEBUG -DSF_BUILD_DEBUG
CXX_Shipping := -opt space,level=2 -inline auto -DNDEBUG -DSF_BUILD_SHIPPING
CXX_NoRTTI   := -RTTI off
CXX_Sdata0   := -sdata 0 -sdata2 0

CXX_opt_size  = -O4,p
CXX_opt_speed = -O4,s

LD_Map :=
LD_Strip :=

NONSTD_TOOLS := 1
EXPAT_CONFIG := Projects/wii/expat_config.h

CRILIBCONFIG_Sdata0 := _s0

LD := $(WII_CWDIR)/PowerPC_EABI_TOOLS/Command_Line_Tools/mwldeppc -proc gekko -stacksize 2048
P_CXXLD := $(LD)

EXT_BUILD_ENVS = CC='../../../../$(CXX2) $(CXXFLAGS)' AR='../../../../$(ARW)' RANLIB=$(P_RANLIB)

$(ARW): Makefile
	@mkdir -p $(OBJDIR)
	@echo -e '#!/bin/sh\nexec $(P_AR) "$$@"' > $@
	@chmod a+x $@

$(CXX2): Makefile
	@mkdir -p $(OBJDIR)
	@echo -e '#!/bin/sh\nexec $(CXX1) -nodefaults -cwd include -gccinc -proc gekko -DRVL -DRVL_SDK -align powerpc -enum int -fp hardware -Cpp_exceptions off -DEPPC -DGEKKO -DRVL_OS -DTRK_INTEGRATION -DIOP_REV=1 -DHOLLYWOOD_REV=1 -DBROADWAY_REV=1 -pragma "cats off" "$$@" $(WIICFLAGS)' > $@
	@chmod a+x $(CXX2)

$(OBJDIRP)/3rdParty/zlib-1.2.5/libz.a: $(CXX2) $(OBJDIRP)/include/sys/types.h $(ARW)
$(OBJDIRP)/3rdParty/jpeg-6b/libjpeg.a: $(CXX2) $(ARW)
$(OBJDIRP)/3rdParty/pcre/libpcre.a: $(CXX2) $(ARW)

$(OBJDIRP)/include/sys/types.h:
	@mkdir -p $(dir $@)
	@echo -e '#include <types.h>\n#include <sys/stat.h>' > $@

$(OBJSGFX): $(CXX2) $(OBJDIRP)/include/sys/types.h
$(OBJDIR)/Src/GFx/AMP/Amp_Message.o : $(CXX2) $(OBJDIRP)/include/sys/types.h

WSRCDIR_RE := $(shell cygpath -aw . | sed -e 's/\\/\\\\\\\\/g;s/ /\\\\\\ /g')

MDCPPOPTS := -gccdep -MD  -o $$@ -c $$<
-include Projects/wii/*.mk0

define COMPILE_CPP_RULE
$(OBJDIR)/%$(1).o : %.cpp
	@mkdir -p $$(dir $$@)
	@echo "  cc     $$<"
	$$(CMD)$$(CXX) $$(CXXFLAGS) $$(CXXCFLAGS) $(2) $(MDCPPOPTS)
	@grep -ivF PowerPC_EABI_Support $$(@:.o=.d) > $$(@:.o=.d1)
	@echo > $$(@:.o=.d)
	@for x in `sed -e 's/$(WSRCDIR_RE)\\\\//ig;s/: / : /g;s/\\\\/\\//g;s/\\/$$$$//g' < $$(@:.o=.d1)`; do echo -ne `cygpath $$$$x` " " >> $$(@:.o=.d); done
	@rm -f $$(@:.o=.d1)

$(OBJDIRP)/%$(1).x.o : %.c
	@mkdir -p $$(dir $$@)
	@echo "  cc     $$<"
	$$(CMD)$$(CXX) $$(EXTCFLAGS) $(2) -gccdep -MD  -o $$@ -c $$<
	@grep -ivF PowerPC_EABI_Support $$(@:.o=.d) > $$(@:.o=.d1)
	@echo > $$(@:.o=.d)
	@for x in `sed -e 's/$(WSRCDIR_RE)\\\\//ig;s/: / : /g;s/\\\\/\\//g;s/\\/$$$$//g' < $$(@:.o=.d1)`; do echo -ne `cygpath $$$$x` " " >> $$(@:.o=.d); done
	@rm -f $$(@:.o=.d1)

$(OBJDIR)/%$(1).o : %.c
	@mkdir -p $$(dir $$@)
	@echo "  cc     $$<"
	$$(CMD)$$(CXX) $$(CXXFLAGS) $$(CXXCFLAGS) $(2) $(MDCPPOPTS)
	@grep -ivF PowerPC_EABI_Support $$(@:.o=.d) > $$(@:.o=.d1)
	@echo > $$(@:.o=.d)
	@for x in `sed -e 's/$(WSRCDIR_RE)\\\\//ig;s/: / : /g;s/\\\\/\\//g;s/\\/$$$$//g' < $$(@:.o=.d1)`; do echo -ne `cygpath $$$$x` " " >> $$(@:.o=.d); done
	@rm -f $$(@:.o=.d1)
endef

$(eval $(call COMPILE_CPP_RULE))

WIILIBS1 := base os exi si db vi mtx gx dvd ai ax axfx mem cx mix sp axart syn seq dsp \
            hio2 nand sc wenc enc arc ipc fs pad wpad kpad euart usb bte
WIILIBS1 += $(shell [ -f $(WII_SDKDIR)/RVL/lib/esp.a ] && echo esp)
WIILIBS1 += $(shell [ -f $(WII_SDKDIR)/RVL/lib/card.a ] && echo card)
WIILIBS1 += $(shell [ -f $(WII_SDKDIR)/RVL/lib/wud.a ] && echo wud)
WIILIBS1 += $(shell [ -f $(WII_SDKDIR)/RVL/lib/pmic.a ] && echo pmic)
WIILIBS1 += $(shell [ -f $(WII_SDKDIR)/RVL/lib/nmic.a ] && echo nmic)
WIILIBS1 += $(shell [ -f $(WII_SDKDIR)/RVL/lib/ven.a ] && echo ven)

WIICSL   := $(if $(findstring Debug,$(C)),D,)
WIILIBS  := $(foreach x,$(WIILIBS1),$(shell cygpath -m $(WII_SDKDIR)/RVL/lib/$(x)$(WIICSL).a))
WIILDEXE := -nodefaults -gdwarf-2 "-L$(shell cygpath -m $(WII_SDKDIR)/RVL/lib)" -fp hardware \
            "$(shell cygpath -m $(WII_CWDIR)/PowerPC_EABI_Support/Runtime/Lib/Runtime.PPCEABI.H.a)" \
            "$(shell cygpath -m $(WII_CWDIR)/PowerPC_EABI_Support/Msl/Msl_C/PPC_EABI/Lib/MSL_C.PPCEABI.bare.H.a)" \
            "$(shell cygpath -m $(WII_CWDIR)/PowerPC_EABI_Support/Msl/Msl_c++/PPC_EABI/Lib/MSL_C++.PPCEABI.bare.H.a)" \
            "$(shell cygpath -m $(WII_CWDIR)/PowerPC_EABI_Support/MetroTRK/TRK_Hollywood_Revolution.a)" \
            "$(shell cygpath -m $(WII_NDEVDIR)/lib/NdevExi2A$(WIICSL).a)" -lodenotstub$(WIICSL).a -unused
ifneq ($(findstring debug,$(C)),)
WIILDEXE += -g
endif
WIILDEXE += -lcf "$(shell cygpath -m $(WII_SDKDIR)/include/revolution/eppc.RVL.lcf)"

EXESUFFIX := .elf

GFX_RENDERERS += WII
GFXDEPS_WII := $(LIBDIR)/libgfx.a $(LIBDIR)/libgfxrender_wii.a $(LIBDIR)/libgfxplatform.a $(LIBDIR)/libgfxsound_fmod.a \
               $(LIBDIRP)/libjpeg.a $(LIBDIRP)/libpng.a $(LIBDIRP)/libz.a $(LIBDIRP)/libexpat.a $(LIBDIRP)/libpcre.a

GFXDEPS_WII_TINY := $(LIBDIR)/libgfx.a $(LIBDIR)/libgfxrender_wii.a $(LIBDIR)/libgfxsound_fmod.a \
                    $(LIBDIRP)/libjpeg.a $(LIBDIRP)/libpng.a $(LIBDIRP)/libz.a $(LIBDIRP)/libexpat.a $(LIBDIRP)/libpcre.a

GFXLIBS += $(WIILIBS)
LDFLAGS += $(WIILDEXE)

SRCS += Apps/Samples/Common/FxSoundFMOD.cpp

$(call BUILD_GFX_REN_LIB,wii, \
    Src/Render/Wii/Wii_HAL.cpp Src/Render/Wii/Wii_Texture.cpp Src/Render/Wii/Wii_MeshCache.cpp \
	Src/Render/Wii/Wii_MemoryManager.cpp Src/Render/Wii/Wii_Shader.cpp)

FMODLIB := $(if $(findstring Debug,$(C)),fmodwiiL.a,fmodwii.a)

#$(call BUILD_GFX_APP,FxPlayerTiny, Apps/Samples/GFxPlayerTiny/GFxPlayerTinyWii.cpp, $(GFXDEPS_WII_TINY))

# Disabled for the time being.
#$(call BUILD_GFX_REN_APPS,FxPlayerSWFToTexture,Apps/Samples/Common/MathLib.cpp,,Apps/Samples/RenderTexture/FxPlayerSWFToTextureWii.cpp)
#$(call BUILD_GFX_REN_APPS,FxPlayerTextureInSWF,Apps/Samples/Common/MathLib.cpp,,Apps/Samples/RenderTexture/FxPlayerTextureInSWFWii.cpp)
#

default: $(CXX2) $(TARGETS)

install:
	@mkdir -p $(DATADIR)
	cp -f Bin/FxPlayer/fxplayer.swf $(DATADIR)
	cp -f Bin/Samples/Window.swf $(DATADIR)
	cp -f Bin/Samples/Mouse.swf $(DATADIR)
	cp -f Bin/RenderTexture/Window_texture.swf $(DATADIR)
