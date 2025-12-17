
CAFE_ROOT_W := $(shell cygpath -am $(CAFE_ROOT))
TOOLSUFFIX := $(shell (uname -s | grep WOW64 >/dev/null) && echo 64 || echo 32)

PP := 
CXX := $(GHS_ROOT)/cxppc
P_CXX := $(CXX)
P_AR := $(GHS_ROOT)/ccppc -archive -o
P_RANLIB := true
P_CXXLD := $(P_CXX)

APPS := desktop mobile kits
MISC_PROGRAMS := no

BUILD_EXPAT := 1

CATBUILD := --g++ --link_once_templates --no_implicit_include -cpu=espresso -warn_dbo_not_found_max=0 -sda=none

CATCFLAGS := -DSF_OS_WIIU $(CATBUILD) --no_commons \
             -I$(CAFE_ROOT_W)/system/include -I$(OBJDIRP)/include \
             -DNDEV=1 -DCAFE=2 -DPLATFORM=CAFE -DEPPC -DGPU7_DISABLE_ENDIAN_WORKAROUND
			 
LDSCRIPT := $(shell cygpath -am $(CAFE_ROOT)/system/include/cafe/eppc.Cafe.ld)

CATLDFLAGS := -e _start -relprog_cafe -Mn -Mu -lnk=-nosegments_always_executable -nostartfile -G $(LDSCRIPT) $(CATBUILD)
LDFLAGS += $(CATLDFLAGS)

CXX_Release  := $(CXX_opt) -dual_debug
CXX_DebugOpt := $(CXX_opt) -dual_debug -D_DEBUG -DSF_BUILD_DEBUG
CXX_Debug    := -G -Onone -dual_debug -D_DEBUG -DSF_BUILD_DEBUG
CXX_NoRTTI   := --no_exceptions --no_rtti
CXX_Shipping := $(CXX_Release) -DSF_BUILD_SHIPPING

CXX_opt_size  = -Osize
CXX_opt_speed = -Ogeneral

LD_Map       := 
LD_Strip     := 

-include Projects/WiiU/*.mk0

PCFLAGS += $(CATCFLAGS)

$(OBJDIRP)/wiiuar: Makefile
	@mkdir -p $(OBJDIRP)
	@echo -e '#!/bin/sh\nexec $(GHS_ROOT)/ccppc -archive -o "$$@"' > $@
	@chmod a+x $(OBJDIRP)/wiiuar

$(OBJDIRP)/3rdParty/zlib-1.2.5/libz.a: $(OBJDIRP)/wiiuar $(OBJDIRP)/include/sys/types.h
$(OBJDIRP)/3rdParty/jpeg-6b/libjpeg.a: $(OBJDIRP)/wiiuar $(OBJDIRP)/include/sys/types.h
$(OBJDIRP)/3rdParty/expat-2.0.1/expat_config.h: $(OBJDIRP)/wiiuar $(OBJDIRP)/include/sys/types.h
$(OBJDIRP)/3rdParty/pcre/libpcre.a: $(OBJDIRP)/wiiuar

EXT_BUILD_ENVS = CC='$(P_CXX) $(CXXFLAGS) -I$(OBJDIRP)/include' CFLAGS='-Ogeneral' AR='$(GHS_ROOT)/ccppc -archive -o' RANLIB=$(P_RANLIB)
NONSTD_TOOLS := 1

define COMPILE_CPP_RULE
$(OBJDIR)/%$(1).o: %.cpp
	@mkdir -p $$(dir $$@)
	@echo "  cc     $$<"
	$$(CMD)$$(P_CXX) $$(CXXFLAGS) $$(CXXCFLAGS) $(2) -MD -o $$@ -c $$< || (rm -f $$(@:.o=.d); false)
	@mv $$(@:.o=.d) $$(@:.o=.d1)
	@sed -e '/.o:/s/\(^.*\)\.o:/\1.o \1.d:/; /\\../s/\\\(.\)/\/\1/g; /[A-Za-z]:[\/\\]/s/\([A-Za-z]\):\//\/cygdrive\/c\//g;' <$$(@:.o=.d1) > $$(@:.o=.d)
	@rm -f $$(@:.o=.d1)

$(OBJDIRP)/%$(1).x.o : %.c
	@mkdir -p $$(dir $$@)
	@echo "  cc     $$<"
	$$(CMD)$$(P_CXX) $$(EXTCFLAGS) $(2) -o $$@ -c $$<

$(OBJDIR)/%$(1).o : %.c
	@mkdir -p $$(dir $$@)
	@echo "  cc     $$<"
	$$(CMD)$$(P_CXX) $$(CXXFLAGS) $$(CXXCFLAGS) $(2) -o $$@ -c $$<
endef

$(eval $(call COMPILE_CPP_RULE))

$(OBJDIRP)/include/sys/types.h:
	@mkdir -p $(dir $@)
	@echo -e '#include <types.h>\n' > $@

$(OBJSGFX): $(OBJDIRP)/include/sys/types.h $(OBJDIRP)/wiiuar

PROFILERLIB := $(if $(findstring Profiler,$(C)),gx2spark nn_save,)

CATLIBS1 := coredyn padscore vpad vpadbase tcl avm gx2 tve dc snd_user snd_core wg nsysnet nsysccr nn_util $(PROFILERLIB)

CATLIBDIR := $(if $(findstring Debug,$(C)),DEBUG,NDEBUG)
CATLIBS  := $(foreach x,$(CATLIBS1),$(CAFE_ROOT_W)/system/lib/ghs/cafe/$(CATLIBDIR)/$(x).a)

LDFLAGS += -L $(CAFE_ROOT_W)/system/lib/ghs/cafe/$(CATLIBDIR)

GFXLIBS_WIIU := $(LIBDIRP)/libpng.a $(LIBDIRP)/libjpeg.a $(LIBDIRP)/libz.a
GFXCCFLAGS_WIIU := $(FMODCFLAGS)

GFX_RENDERERS += WIIU
GFXDEPS_WIIU := $(LIBDIR)/libgfx.a $(LIBDIR)/libgfxrender_wiiu.a $(LIBDIR)/libgfxplatform.a $(LIBDIR)/libgfxsound_fmod.a \
                $(LIBDIRP)/libjpeg.a $(LIBDIRP)/libpng.a $(LIBDIRP)/libz.a $(LIBDIRP)/libexpat.a $(LIBDIRP)/libpcre.a

FMODLIB := $(if $(findstring Debug,$(C)),libfmodexL.a,libfmodex.a)

GFXLIBS_NL := $(shell echo $(GFXLIBS) | sed -e 's/-[lL]\([^ ]*\)//g;')

define BUILD_GFX_APP1
SRCS += $(2)
TARGETS += $(BINDIR)/$(1)$(CSX)$$(FINALEXESUFFIX)
LIBDEPTARGETS += $(BINDIR)/$(1)$(CSX)$$(EXESUFFIX)
$(OBJDIR)/$(1)$(CSX).rpl_export.o: $(GFXDEPS) $(GFXDEPS_APP) $(patsubst %.cpp,$(OBJDIR)/%.o,$(2)) $(4)
	@mkdir -p $$(dir $$@)
	$(CMD)$(CAFE_ROOT)/system/bin/tool/preprpl$(TOOLSUFFIX) -e _start -o $$@ $$^ $(GFXDEPS) $(CATLIBS) $(APPLIBS) $$(GFXLIBS_NL) $(GFXPLDEPS) $(3)
$(BINDIR)/$(1)$(CSX)$(EXESUFFIX): $(OBJDIR)/$(1)$(CSX).rpl_export.o $(GFXDEPS) $(GFXDEPS_APP) $(patsubst %.cpp,$(OBJDIR)/%.o,$(2)) $(4)
	@mkdir -p $$(dir $$@)
	@echo "  ld     $$@"
	$$(CMD)$$(P_CXXLD) $$(LDFLAGS) -map=$$@.map $$(GFXLDFLAGS_APP) $(5) -o $$@ $$^ $$(GFXDEPS) $$(APPLIBS) $$(GFXLIBS) $(3)
endef

define BUILD_GFX_REN_APP
TARGETS += $(BINDIR)/$(1)$(6)$(CSX)$$(FINALEXESUFFIX)
LIBDEPTARGETS += $(BINDIR)/$(1)$(6)$(CSX)$$(EXESUFFIX)
SRCS += $(2) $(patsubst %.cpp,%.$(3).cpp,$(5))
ifneq ($(5),)
$(5): $(GFXSRCDEPS_$(3))
$(call COMPILE_CPP_RULE,.$(3),$$(GFXCCFLAGS_$(3)))
endif
$(OBJDIR)/$(1)$(6)$(CSX).rpl_export.o: $(GFXDEPS) $(GFXDEPS_$(3)) $(patsubst %.cpp,$(OBJDIR)/%.o,$(2)) $(patsubst %.cpp,$(OBJDIR)/%.$(3).o,$(5)) $(GFXPLDEPS)
	@mkdir -p $$(dir $$@)
	$(CMD)$(CAFE_ROOT)/system/bin/tool/preprpl$(TOOLSUFFIX) -e _start -o $$@ $$^ $(GFXDEPS) $(CATLIBS) $(APPLIBS) $$(GFXLIBS_NL) $(GFXPLDEPS) $(GFXLIBS_$(3)) $(4)
$(BINDIR)/$(1)$(6)$(CSX).elf: $(OBJDIR)/$(1)$(6)$(CSX).rpl_export.o $(GFXDEPS) $(GFXDEPS_$(3)) $(patsubst %.cpp,$(OBJDIR)/%.o,$(2)) $(patsubst %.cpp,$(OBJDIR)/%.$(3).o,$(5)) $(GFXPLDEPS)
	@mkdir -p $$(dir $$@)
	@echo "  ld     $$@"
	$(CMD)$(P_CXXLD) $$(LDFLAGS) -map=`echo $$@ | sed -e 's/.elf//g'`.map -o $$@ $$^ $(GFXDEPS) $(CATLIBS) $(APPLIBS) $$(GFXLIBS) $(GFXPLDEPS) $(GFXLIBS_$(3)) $(4)
endef

%.rpx: %.elf
	$(CMD)$(CAFE_ROOT)/system/bin/tool/makerpl$(TOOLSUFFIX) $< -f -nolib -l $(@:.rpx=.a)

# Build the shader desc sources, and shader library using GFxShaderMaker.
Src/Render/WiiU/WiiU_ShaderDescs.cpp Src/Render/WiiU/WiiU_ShaderDescs.h Src/Render/WiiU/WiiU_ShaderBinary.cpp: Src/Render/ShaderData.xml Bin/Tools/GFxShaderMaker/GFxShaderMaker.exe
	@echo "Building GFx Shaders..."
	Bin/Tools/GFxShaderMaker/GFxShaderMaker.exe -platform WiiU -xml Src/Render/ShaderData.xml -outdir Src/Render/WiiU -type Binary -config $(CD)

$(call BUILD_GFX_LIB,libgfxrender_wiiu)
#$(call BUILD_GFX_SOUND_LIB,null)

# Instead of generating dependencies and compiling separately, make all renderer lib sources depend on the ShaderMaker output.
Src/Platform/Platform_WiiU.cpp $(GFXlibgfxrender_wiiuSRCS_ALL): Src/Render/WiiU/WiiU_ShaderDescs.h

# This makes all REN_APPS sources dependent on this file.
GFXSRCDEPS_WIIU = Src/Render/GL/GL_ShaderDescs.h
    

EXESUFFIX := .elf
FINALEXESUFFIX := .rpx

default: $(OBJDIRP)/include/sys/types.h $(TARGETS)

DESTDIR := $(CAFE_ROOT)/data/disc/content/FxPlayer
CONTENT := Bin/Data/RenderModel/PlatformTexture Bin/Data/RenderModel/floor.tga

install: $(CONTENT)
	$(CMD)for i in $(CONTENT) do; (cd `dirname $i` && tar c `basename $i`) | (cd $(DESTDIR) && tar x)

$(call BUILD_GFX_REN_APPS,FxPlayerTiny,,,Apps/Samples/GFxPlayerTiny/GFxPlayerTinyWiiU.cpp)
