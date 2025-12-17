CXXFLAGS = $(PCFLAGS) $(INCLUDES)
LDFLAGS = $(PLDFLAGS)

CXX_Debug     = -Wall -g -DSF_BUILD_DEBUG
CXX_DebugOpt  = $(CXX_opt) -Wall -g -DSF_BUILD_DEBUG -DSF_BUILD_DEBUGOPT
CXX_Release   = $(CXX_opt) -Wall -g
CXX_Shipping  = $(CXX_opt) -Wall -g -DSF_BUILD_SHIPPING
CXX_Profiling = $(CXX_opt) -g -DSF_BUILD_PROFILING

CXX_NoRTTI         = -fno-exceptions -fno-rtti
CXX_Lite           = -DSF_BUILD_LITE
CXX_Watermark      = -DSF_SHOW_WATERMARK
CXX_Profiler       = -DSF_USE_GPU_PROFILER

CXX_opt       = $(CXX_opt_speed)
CXX_opt_size  = -Os
CXX_opt_speed = -O2

LD_Map        = -Wl,-Map,$$@.map
LD_Strip      = -s

LDFLAGS_Debug        =
LDFLAGS_DebugOpt   =
LDFLAGS_Release      = $(LD_Strip)
LDFLAGS_Shipping     = $(LD_Strip)
LDFLAGS_Profiling      =

INCLUDES = -IInclude

LDMAP         := 1

-include Makeconfig

HAS_HEADER = $(shell mkdir -p Obj; touch Obj/empty.h; for i in `$(PP)$(CXX) $(PCFLAGS) -Wp,-v -E Obj/empty.h 2>&1 >/dev/null |sed -ne '/^ /{s/^ //;p;}'`; do [ -f $$i/$(1) ] && echo 1; done; rm -f Obj/empty.h)

ifneq ($(strip $(PLATFORMS)),)
P:=
endif
ifneq ($(strip $(CONFIGS)),)
C:=
endif

.PHONY: clean clean_all rebuild dummy

ifeq ($(strip $(P)),)

ifeq ($(strip $(PLATFORMS)),)
PLATFORMS := $(shell (for i in `ls Projects/*/Makefile 2>/dev/null |sed -e 's/^.*\/\(.*\)\/Makefile$$/\1/'`; do (which `(grep '^BUILD_REQ ' Projects/$$i/Makefile || echo $$i-gcc) | sed -e 's/^.*:= *//'` 2>/dev/null | grep -v '^no ' >/dev/null 2>&1 && echo $$i ); done; cat Makeconfig 2>/dev/null | sed -ne '/^ifeq  *(\$$(P.*),.*)/{s/^ifeq  *(\$$(P.*),\(.*\))/\1/;p;}') | sort | uniq)

default:

BINDIR := Bin/_
LIBDIR := Lib/_
-include Projects/Common/*.mk
PLATFORMS += $(if $(strip $(filter Bin/_/%,$(TARGETS))),local,)
endif

default: $(PLATFORMS)

$(PLATFORMS):
	$(MAKE) P=$@

list:
	@echo $(PLATFORMS)

rebuild: clean $(PLATFORMS)

clean:
	@for i in $(PLATFORMS); do $(MAKE) P=$$i clean; done

else

ifeq ($(strip $(C)),)

CONFIGS := Release+NoRTTI Debug+NoRTTI DebugOpt+NoRTTI Shipping+NoRTTI

default: $(CONFIGS)

$(CONFIGS):
	$(MAKE) P=$(P) C=$@

%.a: dummy
	$(MAKE) P=$(P) C=$(shell echo $@ | sed -e 's/^[^/]*\/[^/]*\/[^/]*\.a/Release/; s/^[^/]*\/[^/]*\/\([^/]*\)\/[^/]*\.a$$/\1/;y/_/+/; s/^$$/default/') $@

%.elf: dummy
	$(MAKE) P=$(P) C=$(shell echo $@ | sed -e 's/^\(.*\/[^+_]*\)\.elf/Release/; s/^.*-\([^-]*\)\.elf$$/\1/;y/_/+/; s/^$$/default/') $@
%.self: dummy
	$(MAKE) P=$(P) C=$(shell echo $@ | sed -e 's/^\(.*\/[^+_]*\)\.self/Release/; s/^.*-\([^-]*\)\.self$$/\1/;y/_/+/; s/^$$/default/') $@


rebuild: clean $(CONFIGS)

clean:
	@for i in $(CONFIGS); do $(MAKE) P=$(P) C=$$i clean_all; done

install:
	@for i in $(CONFIGS); do $(MAKE) P=$(P) C=$$i install; done

list:
	@for i in $(CONFIGS); do $(MAKE) P=$(P) C=$$i list; done

dummy:

else

ifeq ($(findstring +,$(P)),)
PX  := $(word 2, $(subst /, ,$(P)))
P1  := $(firstword $(subst /, ,$(P)))
else
PXT := $(word 2, $(subst +, ,$(P)))
PX  := $(word 2, $(subst /, ,$(PXT)))
PT  := $(firstword $(subst /, ,$(PXT)))
P1  := $(firstword $(subst +, ,$(P)))
endif

P1M := $(P1)
P1A := $(strip $(shell echo $(P1) | sed -e 's/-Win\(CE\|32\)$$//'))
ifneq ($(P1A),$(P1))
P1 := $(strip $(shell echo $(P1) | sed -e 's/.*-Win/Win/'))
PA := $(P1A)
endif

SCU := 1

-include Makeconfig

ifeq ($(strip $(P1)),local)
P1 := $(shell gcc -v 2>&1 | sed -ne '/^Target:/{s/^Target: *//;p;q;};/\/.*-.*\/.*specs/{s/^.*\/\([^\/]*-[^\/]*\)\/.*$$/\1/;p;q;};')
P1 := $(if $(P1),$(P1),unknown)
PP :=
else
PP := $(P1)-
endif

P1 := $(shell echo $(P1) | sed -e 's/-[^-]*-linux/-linux/;s/-linux-gnu/-linux/;s/i[3456]86/i386/;')
PD := $(P1)
CD := $(subst +,_,$(C))

ifneq ($(strip $(C)),Release+NoRTTI)
CSX := _$(subst +,_,$(C))
endif

OBJDIRP := Obj/$(if $(strip $(PA)),$(PA)-)$(PD)$(if $(strip $(PX)),-$(PX))$(if $(strip $(PT)),-$(PT))
OBJDIR  := $(OBJDIRP)/$(CD)
LIBDIRP := Lib/$(if $(strip $(PA)),$(PA)-)$(PD)$(if $(strip $(PX)),-$(PX))$(if $(strip $(PT)),/$(PT))
LIBDIR   = $(LIBDIRP)/$(CD)
BINDIR  := Bin/$(if $(strip $(PA)),$(PA)-)$(PD)$(if $(strip $(PX)),-$(PX))$(if $(strip $(PT)),/$(PT))

-include $(OBJDIRP)/all/config

DEPS = $(patsubst %.cpp,$(OBJDIR)/%.d,$(patsubst %.mm,$(OBJDIR)/%.d,$(patsubst %.c,$(OBJDIR)/%.d,$(SRCS))))
TARGETS :=

P_CC      = $(PP)gcc
P_CXX     = $(PP)g++
P_AR      = $(PP)ar rc
P_RANLIB  = $(PP)ranlib
P_CXXLD  = $(PP)$(CXX)

CMD := $(if $(VERBOSE),,@)
CMDREDIR := $(if $(VERBOSE),,>/dev/null 2>&1)

GFXLIBS = -ljpeg -lpng -lz -lexpat -lpcre
GFXLIBS += -L$(LIBDIRP) -L$(LIBDIR)
GFXDEPS = $(LIBDIR)/libgfxexpat$(LIBSUFFIX) $(LIBDIR)/libgfx$(LIBSUFFIX) $(LIBDIR)/libgfx_as3$(LIBSUFFIX) $(LIBDIR)/libgfx_air$(LIBSUFFIX) $(LIBDIR)/libgfx_as2$(LIBSUFFIX) $(LIBDIR)/libgfxplatform$(LIBSUFFIX) $(LIBDIR)/libgfx$(LIBSUFFIX)

default: Makefile

MDCPPOPTS := -Wp,-MMD,$$(@:.o=.d),-MT,$$@ -o $$@ -c $$<

-include Projects/$(PD)/*.mk0
-include Projects/Common/*.mk0

define COMPILE_CPP_RULE
$(OBJDIR)/%$(1).o : %.cpp
	@mkdir -p $$(dir $$@)
	@echo "  cc     $$<"
	$$(CMD)$$(P_CXX) $$(CXXFLAGS) $$(CXXCFLAGS) $(2) $(MDCPPOPTS)
$(OBJDIR)/%$(1).o : %.c
	@mkdir -p $$(dir $$@)
	@echo "  cc     $$<"
	$$(CMD)$$(P_CC) $$(CXXFLAGS) $$(CXXCFLAGS) $(2) $(MDCPPOPTS)
$(OBJDIR)/%$(1).o : $(OBJDIRP)/all/config

$(OBJDIRP)/%$(1).x.o : %.c
	@mkdir -p $$(dir $$@)
	@echo "  cc     $$<"
	$$(CMD)$$(PP)$$(CXX) $$(CXXFLAGS) $$(CXXCFLAGS) $(2) -Wp,-MMD,$$(@:.o=.d),-MT,$$@ -o $$@ -c $$<
endef



ifeq ($(shell echo $(PT) | tr A-Z a-z),clang)
P_CC := clang
P_CXX := clang++

MDCPPOPTS := -Wp,-dependency-file,$$(@:.o=.d),-MT,$$@ -o $$@ -c $$<

define COMPILE_CPP_RULE
$(OBJDIR)/%$(1).o : %.cpp
	@mkdir -p $$(dir $$@)
	@echo "  cc     $$<"
	$$(CMD)$$(P_CXX) $$(CXXFLAGS) $$(CXXCFLAGS) $(2) $(MDCPPOPTS)
$(OBJDIR)/%$(1).o : %.c
	@mkdir -p $$(dir $$@)
	@echo "  cc     $$<"
	$$(CMD)$$(P_CC) $$(CXXFLAGS) $$(CXXCFLAGS) $(2) $(MDCPPOPTS)
$(OBJDIR)/%$(1).o : $(OBJDIRP)/all/config

$(OBJDIRP)/%$(1).x.o : %.c
	@mkdir -p $$(dir $$@)
	@echo "  cc     $$<"
	$$(CMD)$$(P_CC) $$(CXXFLAGS) $$(CXXCFLAGS) $(2) -Wp,-dependency-file,$$(@:.o=.d),-MT,$$@ -o $$@ -c $$<
endef

endif

$(eval $(call COMPILE_CPP_RULE))

FMODSDK ?= 3rdParty/fmod/$(P1)
ifeq ($(strip $(shell [ -f $(FMODSDK)/inc/fmod.hpp ] && echo 1)),1)
USEFMOD := 1
INCLUDES += -I$(FMODSDK)/inc
FMODLIB := libfmodex.a
FMODLIBOPTS = $(FMODSDK)/lib/$(FMODLIB)
GFXLIBS += $(FMODLIBOPTS)
GFXPLDEPS += $(LIBDIR)/libgfxsound_fmod$(LIBSUFFIX)
FMODCFLAGS += -DGFX_SOUND_FMOD
CXXFLAGS += -DGFX_ENABLE_SOUND
endif

# add ZLIB include path
INCLUDES += -I3rdParty/zlib-1.2.7

ifneq ($(strip $(shell [ -e Projects/$(PD)/Makefile ] && echo 1)),1)

$(OBJDIRP)/all/config:
	@mkdir -p $(OBJDIRP)/all
	@echo > $@
	@echo > $@.cpp
	@echo "#include \"Kernel/SF_Atomic.h\"" > $@.cpp
	@echo "#include <pthread.h>" >> $@.cpp
	@echo "using namespace Scaleform; AtomicInt<SInt32> x; int main() { x++; x.CompareAndSet_Sync(0,1); x.Exchange_Sync(1); return 0; }" >> $@.cpp
	@$(PP)$(CXX) $(CXXFLAGS) -O2 $(PCFLAGS) -o $@.o $@.cpp >/dev/null 2>&1 || echo "CXXFLAGS += -USF_ENABLE_THREADS" >> $@
	@rm -f $@.cpp $@.o

ifeq ($(findstring SF_ENABLE_THREADS,$(CXXFLAGS)),)
GFXLIBS += -lpthread
endif

ifeq ($(USEFMOD),1)
ifeq ($(strip $(shell [ -f $(FMODSDK)/lib/$(FMODLIB) ] || ([ -f $(FMODSDK)/lib/libfmodex.so ] && echo 1))),1)
FMODLIB := libfmodex.so
FMODLIBOPTS = -lfmodex -L$(FMODSDK)/lib -Wl,-R,'$$ORIGIN/../../$(FMODSDK)/lib'
endif
ifeq ($(strip $(shell [ -f $(FMODSDK)/lib/$(FMODLIB) ] || ([ -f $(FMODSDK)/lib/libfmodex64.so ] && echo 1))),1)
FMODLIB := libfmodex64.so
FMODLIBOPTS = -lfmodex64 -L$(FMODSDK)/lib -Wl,-R,'$$ORIGIN/../../$(FMODSDK)/lib'
endif
endif

endif

Bin/Data/AS3/FxPlayer/fxplayermini.swf.h: Bin/Data/AS3/FxPlayer/fxplayermini.swf
	@rm -f $@
	@mkdir -p $(dir $@)
	@echo "const unsigned char fxplayermini_swf[] = {" > $@
	@od -v -t x1 $^ | sed -e 's/^[0-7][0-7]*//;s/[0-9a-f][0-9a-f]/0x&,/g' >> $@
	@echo " 0};" >> $@

Bin/Data/AS2/FxPlayer/fxvideoplayer.swf.h: Bin/Data/AS2/FxPlayer/fxvideoplayer.swf
	@rm -f $@
	@mkdir -p $(dir $@)
	@echo "const unsigned char fxvideoplayer_swf[] = {" > $@
	@od -v -t x1 $^ | sed -e 's/^[0-7][0-7]*//;s/[0-9a-f][0-9a-f]/0x&,/g' >> $@
	@echo " 0};" >> $@


define BUILD_GFX_APP1
SRCS += $(2)
TARGETS += $(BINDIR)/$(1)$(CSX)$$(FINALEXESUFFIX)
LIBDEPTARGETS += $(BINDIR)/$(1)$(CSX)$$(EXESUFFIX)
$(BINDIR)/$(1)$(CSX)$(EXESUFFIX): $(GFXDEPS) $(GFXDEPS_APP) $(patsubst %.cpp,$(OBJDIR)/%.o,$(2)) $(4)
	@mkdir -p $$(dir $$@)
	@echo "  ld     $$@"
	$$(CMD)$$(P_CXXLD) $$(LDFLAGS) $$(GFXLDFLAGS_APP) $(5) -o $$@ $$^ $$(GFXDEPS) $$(APPLIBS) $$(GFXLIBS) $(3)
endef

define BUILD_GFX_APP
$(eval $(call BUILD_GFX_APP1,$(1),$(2),$(3),$(4),$(5)))
endef

define BUILD_GFX_REN_APP
TARGETS += $(BINDIR)/$(1)$(6)$(CSX)$$(FINALEXESUFFIX)
LIBDEPTARGETS += $(BINDIR)/$(1)$(6)$(CSX)$$(EXESUFFIX)
SRCS += $(2) $(patsubst %.cpp,%.$(3).cpp,$(5))
ifneq ($(5),)
$(5): $(GFXSRCDEPS_$(3))
$(call COMPILE_CPP_RULE,.$(3),$$(GFXCCFLAGS_$(3)))
endif
$(BINDIR)/$(1)$(6)$(CSX)$(EXESUFFIX): $(GFXDEPS) $(GFXDEPS_$(3)) $(patsubst %.cpp,$(OBJDIR)/%.o,$(2)) $(patsubst %.cpp,$(OBJDIR)/%.$(3).o,$(5)) $(GFXPLDEPS)
	@mkdir -p $$(dir $$@)
	@echo "  ld     $$@"
	$(CMD)$(P_CXXLD) $(LDFLAGS) $(GFXLDFLAGS_$(3)) -o $$@ $$^ $(GFXDEPS) $(APPLIBS) $$(GFXLIBS) $(GFXPLDEPS) $(GFXLIBS_$(3)) $(4)
endef

define BUILD_GFX_REN_APPS
$(if $(strip $(GFX_RENDERERS)), $(if $(findstring $(words $(GFX_RENDERERS)),1),\
$(eval $(call BUILD_GFX_REN_APP,$(1),$(2),$(firstword $(GFX_RENDERERS)),$(3),$(4))),\
$(foreach v,$(GFX_RENDERERS),$(eval $(call BUILD_GFX_REN_APP,$(1),$(2),$(v),$(3),$(4),$(v))))),)
endef

$(OBJDIRP)/%.txt: Projects/%.txt
	@mkdir -p $(dir $@)
	@awk '/^\[.*\]/ {x=1} /^\[([^-].*,)?$(P1)(,.*)?\]/ {x=0} /^\[-.*\]/ {x=0} /^\[-(.*,)?$(P1)(,.*)?\]/ {x=1} /^[^[].*\.(([^hH])|(...*))$$/ {if (x==0) print "  " $$1 " \\" }' $< >> $@

$(OBJDIRP)/%.mk : Projects/%.txt
	@mkdir -p $(dir $@)
	@echo $< | sed -e 's/Projects\/\(.*\)\.txt/GFX\1SRCS_ALL := \\/' > $@
	@awk '/^\[.*\]/ {x=1} /^\[([^-].*,)?$(P1)(,.*)?\]/ {x=0} /^\[-.*\]/ {x=0} /^\[-(.*,)?$(P1)(,.*)?\]/ {x=1} /^[^[].*\.(([^hH])|(...*))$$/ {if (x==0) print "  " $$1 " \\" }' $< >> $@
	@echo >> $@
	@echo >> $@
	@echo $< | sed -e 's/Projects\/\(.*\)\.txt/GFX\1SRCS += $$(GFX\1SRCS_ALL)/' >> $@
	@awk '/^\[.*\]/ {x=1} /^\[([^-].*,)?$(P1)(,.*)?\]/ {x=0} /^\[-.*\]/ {x=0} /^\[-(.*,)?$(P1)(,.*)?\]/ {x=1} /^[^[].*\.(([^hH])|(...*))$$/ {if (x==0 && $$2) { for (i=2; i<=NF; i=i+1) { print $$1 " " $$i } }}' $< \
         | grep = | sed -e 's/ \([a-z][a-z]*\)=\([a-z][a-z]*\)\( \|$$\)/ CXX_\1 := $$(CXX_\1_\2) /;s/\.cpp/\.o/;s/^\([^ ]*\)  *\(.*\)$$/$$(OBJDIR)\/\1: \2/;'  >> $@
	@echo >> $@
	@echo $< | sed -e 's/Projects\/\(.*\)\.txt/GFX\1SRCS_1 := \\/' >> $@
	@awk '/^\[.*\]/ {x=1} /^\[([^-].*,)?$(P1)(,.*)?\]/ {x=0} /^\[-.*\]/ {x=0} /^\[-(.*,)?$(P1)(,.*)?\]/ {x=1} /^[^[].*\.(([^hH])|(...*))$$/ {if (x==0 && $$2) { for (i=2; i<=NF; i=i+1) { print $$1 " " $$i } }}' $< \
         | sed -e 's/^\(.*\) *noscu/ \1 \\/'  >> $@
	@echo >> $@

define BUILD_GFX_LIB1
$(LIBDIR)/$(1)$(LIBSUFFIX): $(OBJDIRP)/$(1).mk
$(eval -include $(OBJDIRP)/$(1).mk)
endef

define BUILD_GFX_LIB2
TARGETS += $(LIBDIR)/$(1)$(LIBSUFFIX)
SRCS += $$(GFX$(1)SRCS)
$(LIBDIR)/$(1)$(LIBSUFFIX): $$(patsubst %.cpp,$(OBJDIR)/%.o,$$(patsubst %.c,$(OBJDIR)/%.o,$$(GFX$(1)SRCS))) $$(GFX$(1)ARDEPS)
	@mkdir -p $$(dir $$@)
	@rm -f $$@
	@echo "  ar     $$@"
	$$(CMD)$$(P_AR) $$(P_AR_OUTOPT)$$@ $$(patsubst %.cpp,$(OBJDIR)/%.o,$$(patsubst %.c,$(OBJDIR)/%.o,$$(GFX$(1)SRCS))) $$(GFX$(1)ARDEPS)
	$$(CMD)$$(P_RANLIB) $$@
endef

define BUILD_GFX_LIB2_CFLAGS
TARGETS += $(LIBDIR)/$(1)$(LIBSUFFIX)
SRCS += $(patsubst %.cpp,%.$(1).cpp,$$(GFX$(1)SRCS))
$(call COMPILE_CPP_RULE,.$(1),$(3))
$(LIBDIR)/$(1)$(LIBSUFFIX): $$(patsubst %.cpp,$(OBJDIR)/%.$(1).o,$$(GFX$(1)SRCS)) $$(GFX$(1)ARDEPS)
	@mkdir -p $$(dir $$@)
	@rm -f $$@
	@echo "  ar     $$@"
	$$(CMD)$$(P_AR) $$(P_AR_OUTOPT)$$@ $$(patsubst %.cpp,$(OBJDIR)/%.$(1).o,$$(GFX$(1)SRCS)) $$(GFX$(1)ARDEPS)
	$$(CMD)$$(P_RANLIB) $$@
endef

define BUILD_SCU_SRC
$(2): Projects/$(1).txt
	@ls -1 $(3) | grep -E '^$(shell echo $(2) | sed -e "s/$(1)_all.cpp//")[^/]*$$$$' | sed -e 's/^Src\/\(.*\)/#include "\1"/' >$$@
endef

define BUILD_GFX_LIB2_SCU
TARGETS += $(LIBDIR)/$(1)$(LIBSUFFIX)
GFX$(1)SRCS_U = $$(filter-out $$(GFX$(1)SRCS_1),$$(GFX$(1)SRCS))
SCU_SRCS_$(1) := $$(shell echo $$(GFX$(1)SRCS_U) | perl -ne '$$$$in=$$$$_; foreach (split(/\s+/,$$$$in)) { s@^(.*/)[^/]*$$$$@\1$(1)_all.cpp@; $$$$f{$$$$_."\n"}=1;} END {print keys %f}')
SRCS += $$(SCU_SRCS_$(1))
$$(foreach v,$$(SCU_SRCS_$(1)),$$(eval $$(call BUILD_SCU_SRC,$(1),$$(v),$$(GFX$(1)SRCS_U))))
$(LIBDIR)/$(1)$(LIBSUFFIX): $$(patsubst %.cpp,$(OBJDIR)/%.o,$$(patsubst %.c,$(OBJDIR)/%.o,$$(GFX$(1)SRCS_1) $$(SCU_SRCS_$(1)))) $$(GFX$(1)ARDEPS)
	@mkdir -p $$(dir $$@)
	@rm -f $$@
	@echo "  ar     $$@"
	$$(CMD)$$(P_AR) $$(P_AR_OUTOPT)$$@ $$(patsubst %.cpp,$(OBJDIR)/%.o,$$(patsubst %.c,$(OBJDIR)/%.o,$$(GFX$(1)SRCS_1) $$(SCU_SRCS_$(1)))) $$(GFX$(1)ARDEPS)
	$$(CMD)$$(P_RANLIB) $$@
endef

define BUILD_GFX_LIB3
GFX$(1)SRCS = $(2)

endef

define BUILD_GFX_LIB
$(eval $(call BUILD_GFX_LIB1,$(1),$(2)) $(call BUILD_GFX_LIB2,$(1),$(2)))
endef

define BUILD_GFX_LIB_CFLAGS
$(eval $(call BUILD_GFX_LIB1,$(1),$(2)) $(call BUILD_GFX_LIB2_CFLAGS,$(1),$(2),$(3)))
endef

ifeq ($(SCU),1)
define BUILD_GFX_LIB_SCU
$(eval $(call BUILD_GFX_LIB1,$(1),$(2)) $(call BUILD_GFX_LIB2_SCU,$(1),$(3)))
endef
else
define BUILD_GFX_LIB_SCU
$(eval $(call BUILD_GFX_LIB1,$(1),$(2)) $(call BUILD_GFX_LIB2,$(1)))
endef
endif

define BUILD_GFX_REN_LIB
$(eval $(call BUILD_GFX_LIB3,libgfxrender_$(1),$(2)) $(call BUILD_GFX_LIB2,libgfxrender_$(1)))
endef
define BUILD_GFX_SOUND_LIB
$(eval $(call BUILD_GFX_LIB3,libgfxsound_$(1),Src/Sound/Sound_SoundRenderer.cpp Src/Sound/Sound_Sound.cpp $(2)) $(call BUILD_GFX_LIB2,libgfxsound_$(1)))
endef


LIBSUFFIX := .a
FINALEXESUFFIX = $(EXESUFFIX)

APPS := desktop

-include Projects/$(PD)/*.mk1
-include Projects/Common/*.mk1

LDFLAGS += $(foreach v,$(subst +, ,$(C)),$(LDFLAGS_$(v)))
ifeq ($(LDMAP),1)
LDFLAGS += $(LD_Map)
endif

ifneq ($(BUILD_VIDEO),no)
ifneq ($(strip $(shell [ -f $(LIBDIR)/libgfxvideo$(LIBSUFFIX) ] && echo 1)$(BUILD_VIDEO)),)
GFXDEPS += $(LIBDIR)/libgfxvideo$(LIBSUFFIX) $(LIBDIR)/libgfx$(LIBSUFFIX)
CXXFLAGS += -DGFX_USE_VIDEO
endif
endif

-include Projects/$(PD)/Makefile
ifeq ($(strip $(SPECIAL_BUILD)),)
ifeq ($(strip $(shell [ -f $(FMODSDK)/inc/fmod.hpp ] && echo 1)),1)
$(call BUILD_GFX_SOUND_LIB,fmod,Src/Sound/Sound_SoundRendererFMOD.cpp)
endif
$(call BUILD_GFX_SOUND_LIB,null,)

-include Projects/Makefile
-include Projects/Common/*.mk
endif
-include Projects/$(PD)/*.mk

CXXCFLAGS = $(foreach v,$(subst +, ,$(C)),$(CXX_$(v)))
EXTCFLAGS = $(PCFLAGS) $(INCLUDES) $(CXX_Release)

CONFIGWARN := $(if $(CXXCFLAGS),,$(warning Unknown configuration(s) $(C)))

default: $(TARGETS)

clean:
	rm -rf $(OBJDIR) $(TARGETS)

clean_all: clean
	rm -rf $(OBJDIRP)

rebuild: clean default

list:
	@echo $(TARGETS)

ifneq ($(strip $(DEPS)),)
-include $(DEPS)

.DEFAULT:
	@echo $@ | grep '.h$$' >/dev/null && (touch $@; chmod a-w $@) || true
endif

endif
endif
