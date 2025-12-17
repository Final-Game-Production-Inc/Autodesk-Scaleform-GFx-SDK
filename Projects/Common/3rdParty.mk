ifeq ($(strip $(EXT_BUILD_ENVS)),)
EXT_BUILD_ENVS = CC="$(P_CC) $(PCFLAGS)" CXX="$(P_CXX) $(PCFLAGS)" LD="$(PP)ld" AR="$(PP)ar rc" RANLIB=$(P_RANLIB)
endif

define ADDLIBDEPS
$(foreach v,$(LIBDEPTARGETS), $(eval $(v): $(1) ))
endef

LIBSUFFIX := .a
define BUILD_EXT_LIB
TARGETS += $$(LIBDIR)/$(1)$(LIBSUFFIX)
$(1)_OBJS := $(patsubst %.c,$(OBJDIRP)/$(2)/%.$(1).x.o,$(3))
$(eval $(call COMPILE_CPP_RULE,.$(1),$(4) $$(CFLAGS_$(1))))

$(LIBDIRP)/$(1)$(LIBSUFFIX): $$($(1)_OBJS)
	@mkdir -p $$(dir $$@)
	$(CMD)$(P_AR) $(P_AR_OUTOPT)$$@ $$($(1)_OBJS)
	$(CMD)$(P_RANLIB) $$@
endef


ifeq ($(strip $(call HAS_HEADER,jpeglib.h)),)
BUILD_JPEG := 1
endif

ifneq ($(strip $(BUILD_JPEG)),)

ifneq ($(strip $(NONSTD_TOOLS)),) 

$(eval $(call BUILD_EXT_LIB,libjpeg,3rdParty/jpeg-8d, \
    jcapimin.c  jcmarker.c  jdapimin.c  jdinput.c   jdtrans.c   jdarith.c \
    jcapistd.c  jcmaster.c  jdapistd.c  jdmainct.c  jerror.c    jmemmgr.c \
    jccoefct.c  jcomapi.c   jdatadst.c  jdmarker.c  jfdctflt.c  jmemnobs.c \
    jccolor.c   jcparam.c   jdatasrc.c  jdmaster.c  jfdctfst.c  jquant1.c \
    jcdctmgr.c  jdcoefct.c  jdmerge.c   jfdctint.c  jquant2.c   jaricom.c \
    jchuff.c    jcprepct.c  jdcolor.c   jidctflt.c  jutils.c    jcarith.c \
    jcinit.c    jcsample.c  jddctmgr.c  jdpostct.c  jidctfst.c \
    jcmainct.c  jctrans.c   jdhuff.c    jdsample.c  jidctint.c \
))

INCLUDES += -I3rdParty/jpeg-8d

else
$(OBJDIRP)/3rdParty/jpeg-8d/libjpeg.a $(OBJDIRP)/3rdParty/jpeg-8d/jpeglib.h:
	@mkdir -p $(OBJDIRP)/3rdParty
	@echo "  make   $@"
	$(CMD)cp -rp 3rdParty/jpeg-8d $(OBJDIRP)/3rdParty | (cd $(OBJDIRP))
	$(CMD)(cd $(OBJDIRP)/3rdParty/jpeg-8d; chmod -R 755 *; sh ./configure $(EXT_BUILD_ENVS); cp -rp makefile.ansi Makefile; make $(EXT_BUILD_ENVS) libjpeg.a; $(P_RANLIB) libjpeg.a) $(CMDREDIR)

$(LIBDIRP)/libjpeg.a: $(OBJDIRP)/3rdParty/jpeg-8d/libjpeg.a
	@mkdir -p $(dir $@)
	@cp -p $< $@

TARGETS += $(LIBDIRP)/libjpeg.a
INCLUDES += -I$(OBJDIRP)/3rdParty/jpeg-8d
endif

$(OBJDIR)/Src/Render/ImageFiles/JPEG_ImageCommon.o: $(LIBDIRP)/libjpeg$(LIBSUFFIX)
$(call ADDLIBDEPS,$(LIBDIRP)/libjpeg$(LIBSUFFIX))

endif

ifeq ($(strip $(call HAS_HEADER,zlib.h)),)
BUILD_ZLIB := 1
endif
ifneq ($(strip $(BUILD_ZLIB)),)

ifneq ($(strip $(NONSTD_TOOLS)),)
$(eval $(call BUILD_EXT_LIB,libz,3rdParty/zlib-1.2.7, \
    adler32.c   crc32.c    inffast.c  inftrees.c  uncompr.c \
    compress.c  deflate.c  infback.c  inflate.c  trees.c     zutil.c \
))

INCLUDES += -I3rdParty/zlib-1.2.7

else
$(OBJDIRP)/3rdParty/zlib-1.2.7/libz.a $(OBJDIRP)/3rdParty/zlib-1.2.7/zconf.h:
	@mkdir -p $(OBJDIRP)/3rdParty
	@echo "  make   $@"
	$(CMD)cp -rp 3rdParty/zlib-1.2.7 $(OBJDIRP)/3rdParty | (cd $(OBJDIRP))
	$(CMD)(cd $(OBJDIRP)/3rdParty/zlib-1.2.7 && (chmod u+w -R .; make distclean; rm -f zconf.h Makefile)) $(CMDREDIR)
	$(CMD)(cd $(OBJDIRP)/3rdParty/zlib-1.2.7; $(EXT_BUILD_ENVS) sh ./configure -s; make $(EXT_BUILD_ENVS) libz.a; $(P_RANLIB) libz.a)  $(CMDREDIR)

$(LIBDIRP)/libz.a: $(OBJDIRP)/3rdParty/zlib-1.2.7/libz.a
	@mkdir -p $(dir $@)
	@cp -p $< $@

TARGETS += $(LIBDIRP)/libz.a
INCLUDES += -I$(OBJDIRP)/3rdParty/zlib-1.2.7

endif

$(OBJDIR)/Src/GFx/GFx_ZLibFile.o: $(LIBDIRP)/libz$(LIBSUFFIX)
$(OBJDIR)/Src/GFx/GFx_ZlibSupport.o: $(LIBDIRP)/libz$(LIBSUFFIX)
$(OBJDIR)/Src/GFx/AS3/Obj/Utils/AS3_Obj_Utils_ByteArray.o: $(LIBDIRP)/libz$(LIBSUFFIX)
$(OBJDIR)/Src/GFx/AS3/Obj/Utils/libgfx_as3_all.o: $(LIBDIRP)/libz$(LIBSUFFIX)
$(call ADDLIBDEPS, $(LIBDIRP)/libz$(LIBSUFFIX))

$(OBJDIRP)/3rdParty/libpng-1.5.13/libpng.a: $(LIBDIRP)/libz$(LIBSUFFIX)

endif

ifeq ($(strip $(call HAS_HEADER,png.h)),)
BUILD_PNG := 1
endif

ifneq ($(strip $(BUILD_PNG)),)

ifneq ($(strip $(NONSTD_TOOLS)),)
$(eval $(call BUILD_EXT_LIB,libpng,3rdParty/libpng-1.5.13, \
  png.c       pngmem.c    pngrio.c    pngset.c    pngwio.c    pngwutil.c \
  pngerror.c  pngpread.c  pngrtran.c  pngtrans.c  pngwrite.c \
  pngget.c    pngread.c   pngrutil.c  pngwtran.c \
))
INCLUDES += -I3rdParty/libpng-1.5.13

else
$(OBJDIRP)/3rdParty/libpng-1.5.13/libpng.a:
	@mkdir -p $(OBJDIRP)/3rdParty
	@echo "  make   $@"
	$(CMD)cp -rp 3rdParty/libpng-1.5.13 $(OBJDIRP)/3rdParty | (cd $(OBJDIRP))
	$(CMD)cp -f $(OBJDIRP)/3rdParty/libpng-1.5.13/scripts/makefile.std $(OBJDIRP)/3rdParty/libpng-1.5.13/Makefile
	$(CMD)(cd $(OBJDIRP)/3rdParty/libpng-1.5.13; make $(EXT_BUILD_ENVS) $(EXT_BUILD_PNG) ZLIBINC=../zlib-1.2.7 ZLIBLIB=../zlib-1.2.7 libpng.a; $(P_RANLIB) libpng.a) $(CMDREDIR)

$(LIBDIRP)/libpng.a: $(OBJDIRP)/3rdParty/libpng-1.5.13/libpng.a
	@mkdir -p $(dir $@)
	@cp -p $< $@

TARGETS += $(LIBDIRP)/libpng$(LIBSUFFIX)
INCLUDES += -I$(OBJDIRP)/3rdParty/libpng-1.5.13
endif

$(OBJDIR)/Src/Render/ImageFiles/PNG_ImageReader.o: $(LIBDIRP)/libpng$(LIBSUFFIX)
$(OBJDIR)/Src/GFx/GFx_PNGSupport.o: $(LIBDIRP)/libpng$(LIBSUFFIX)
$(call ADDLIBDEPS,$(LIBDIRP)/libpng$(LIBSUFFIX))

endif

ifeq ($(strip $(call HAS_HEADER,expat.h)),)
BUILD_EXPAT := 1
endif

ifneq ($(strip $(BUILD_EXPAT)),)

CCTARGET = $(shell $(P_CC) -v 2>&1 | sed -ne '/^Target:/{s/^Target: *//;p;q;};/\/.*-.*\/.*specs/{s/^.*\/\([^\/]*-[^\/]*\)\/.*$$/\1/;p;q;};')

ifeq ($(EXPAT_CONFIG),)
EXPAT_BASE := $(OBJDIRP)/3rdParty/expat-2.1.0
$(OBJDIRP)/3rdParty/expat-2.1.0/expat_config.h:
	@mkdir -p $(OBJDIRP)/3rdParty
	@echo "  make   expat"
	$(CMD)cp -rp 3rdParty/expat-2.1.0 $(OBJDIRP)/3rdParty | (cd $(OBJDIRP))
	$(CMD)(cd $(OBJDIRP)/3rdParty/expat-2.1.0; chmod -R u+w .; make distclean; rm -f zconf.h Makefile) $(CMDREDIR)
	$(CMD)(cd $(OBJDIRP)/3rdParty/expat-2.1.0; sh ./configure --target=$(CCTARGET) --disable-shared ) $(CMDREDIR)
else
EXPAT_BASE := 3rdParty/expat-2.1.0
$(OBJDIRP)/include/expat_config.h: $(EXPAT_CONFIG)
	@mkdir -p $(OBJDIRP)/include
	$(CMD)cp $(EXPAT_CONFIG) $@
endif

EXPAT_SRC := $(EXPAT_BASE)/lib
EXPAT_OBJDIR := $(OBJDIRP)/3rdParty/expat-2.1.0/lib
EXPAT_OBJS := $(patsubst %.o,$(EXPAT_OBJDIR)/%.expat.x.o,xmlparse.o xmlrole.o xmltok.o xmltok_impl.o xmltok_ns.o)
EXPAT_CFLAGS := -DHAVE_EXPAT_CONFIG_H -I$(OBJDIRP)/include -I$(EXPAT_BASE) -I$(EXPAT_SRC)
ifeq ($(PT),CW)
EXPAT_CFLAGS += -I- -I$(OBJDIRP)/include -I$(EXPAT_BASE) -I$(EXPAT_SRC)
endif

$(eval $(call COMPILE_CPP_RULE,.expat,$$(EXPAT_CFLAGS)))

$(LIBDIRP)/libexpat$(LIBSUFFIX): $(EXPAT_OBJS)
	@mkdir -p $(dir $@)
	$(CMD)$(P_AR) $(P_AR_OUTOPT)$@ $(EXPAT_OBJS)
	$(CMD)$(P_RANLIB) $@

ifeq ($(EXPAT_CONFIG),)
$(EXPAT_OBJS) : $(EXPAT_BASE)/expat_config.h
else
$(EXPAT_OBJS) : $(OBJDIRP)/include/expat_config.h
endif

TARGETS += $(LIBDIRP)/libexpat$(LIBSUFFIX)

INCLUDES += -I$(EXPAT_SRC)

$(OBJDIR)/Src/XML/XML_Expat.o: $(LIBDIRP)/libexpat$(LIBSUFFIX)
$(OBJDIR)/Src/GFx/AS3/AS3_CodeExecute.o: $(LIBDIRP)/libexpat$(LIBSUFFIX)
$(OBJDIR)/Src/GFx/AS3/AS3_Value.o: $(LIBDIRP)/libexpat$(LIBSUFFIX)
$(OBJDIR)/Src/GFx/AS3/AS3_XMLSupport.o: $(LIBDIRP)/libexpat$(LIBSUFFIX)
$(OBJDIR)/Src/GFx/AS3/Obj/AS3_Obj_All.o: $(LIBDIRP)/libexpat$(LIBSUFFIX)
$(OBJDIR)/Src/GFx/AS3/Obj/AS3_Obj_XML.o: $(LIBDIRP)/libexpat$(LIBSUFFIX)
$(OBJDIR)/Src/GFx/AS3/Obj/AS3_Obj_XMLList.o: $(LIBDIRP)/libexpat$(LIBSUFFIX)
$(OBJDIR)/Src/GFx/AS3/Obj/AS3_Obj_Function.o: $(LIBDIRP)/libexpat$(LIBSUFFIX)
$(OBJDIR)/Src/GFx/AS3/Obj/Net/AS3_Obj_Net_XMLSocket.o: $(LIBDIRP)/libexpat$(LIBSUFFIX)
$(OBJDIR)/Src/GFx/AS3/Obj/Xml/AS3_Obj_Xml_XMLDocument.o: $(LIBDIRP)/libexpat$(LIBSUFFIX)
$(OBJDIR)/Src/GFx/AS3/Obj/Xml/AS3_Obj_Xml_XMLNode.o: $(LIBDIRP)/libexpat$(LIBSUFFIX)
$(OBJDIR)/Src/GFx/AS3/Obj/Xml/AS3_Obj_Xml_XMLNodeType.o: $(LIBDIRP)/libexpat$(LIBSUFFIX)
$(OBJDIR)/Src/GFx/AS3/Obj/Security/AS3_Obj_Security_XMLSignatureValidator.o: $(LIBDIRP)/libexpat$(LIBSUFFIX)
$(OBJDIR)/Src/GFx/AS3/libgfx_as3_all.o: $(LIBDIRP)/libexpat$(LIBSUFFIX)
$(OBJDIR)/Src/GFx/AS3/Obj/libgfx_as3_all.o: $(LIBDIRP)/libexpat$(LIBSUFFIX)
$(OBJDIR)/Src/GFx/AS3/Obj/Net/libgfx_as3_all.o: $(LIBDIRP)/libexpat$(LIBSUFFIX)
$(OBJDIR)/Src/GFx/AS3/Obj/Xml/libgfx_as3_all.o: $(LIBDIRP)/libexpat$(LIBSUFFIX)
$(OBJDIR)/Src/GFx/AS3/Obj/Security/libgfx_as3_all.o: $(LIBDIRP)/libexpat$(LIBSUFFIX)

$(call ADDLIBDEPS,$(LIBDIRP)/libexpat$(LIBSUFFIX))

endif

ifeq ($(strip $(call HAS_HEADER,pcre.h)),)
BUILD_PCRE := 1
endif

ifneq ($(strip $(BUILD_PCRE)),)

$(OBJDIRP)/3rdParty/pcre/libpcre.a $(OBJDIRP)/3rdParty/pcre/pcre.h:
	@mkdir -p $(OBJDIRP)/3rdParty
	@echo "  make   pcre"
	$(CMD)rm -rf $(OBJDIRP)/3rdParty/pcre; cp -rp 3rdParty/pcre $(OBJDIRP)/3rdParty | (cd $(OBJDIRP))
	$(CMD)(cd $(OBJDIRP)/3rdParty/pcre; make $(EXT_BUILD_ENVS)) $(CMDREDIR)

$(LIBDIRP)/libpcre.a: $(OBJDIRP)/3rdParty/pcre/libpcre.a
	@mkdir -p $(dir $@)
	@cp -p $< $@

TARGETS += $(LIBDIRP)/libpcre.a
INCLUDES += -I$(OBJDIRP)/3rdParty/pcre

$(OBJDIR)/Src/GFx/AS3/Obj/AS3_Obj_RegExp.o: $(LIBDIRP)/libpcre$(LIBSUFFIX)
$(OBJDIR)/Src/GFx/AS3/Obj/libgfx_as3_all.o: $(LIBDIRP)/libpcre$(LIBSUFFIX)
$(call ADDLIBDEPS,$(LIBDIRP)/libpcre$(LIBSUFFIX))

endif
BUILD_NVTT := 1

ifneq ($(strip $(BUILD_NVTT)),)

NVTT_SRC := $(OBJDIRP)/3rdParty/nvtt/src

NVTT_OBJS := $(patsubst %.o,$(NVTT_SRC)/%.nvtt.o,nvcore/StrLib.o nvcore/Debug.o nvcore/Memory.o nvmath/Eigen.o nvmath/Fitting.o nvimage/BlockDXT.o nvimage/ColorBlock.o nvimage/DirectDrawSurface.o nvimage/Filter.o nvimage/FloatImage.o nvimage/Image.o nvimage/ImageIO.o nvimage/NormalMap.o nvimage/Quantize.o nvtt/nvtt.o nvtt/CompressRGB.o nvtt/InputOptions.o nvtt/OutputOptions.o nvtt/CompressionOptions.o nvtt/CompressDXT.o nvtt/OptimalCompressDXT.o nvtt/QuickCompressDXT.o nvtt/Compressor.o nvtt/cuda/CudaUtils.o nvtt/cuda/CudaCompressDXT.o nvtt/squish/colourblock.o nvtt/squish/colourfit.o nvtt/squish/colourset.o nvtt/squish/fastclusterfit.o nvtt/squish/maths.o nvtt/squish/weightedclusterfit.o)
$(NVTT_OBJS): $(OBJDIRP)/3rdParty/nvtt/src/nvconfig.h

%.nvtt.o : %.cpp
	@mkdir -p $(dir $@)
	$(CMD)$(PP)$(P_CXX) $(PCFLAGS) -I$(NVTT_SRC) -o $@ -c $<
%.nvtt.o : %.c
	@mkdir -p $(dir $@)
	$(CMD)$(PP)$(P_CXX) $(PCFLAGS) -I$(NVTT_SRC) -o $@ -c $<

$(NVTT_SRC)/%.cpp: $(OBJDIRP)/3rdParty/nvtt/src/nvconfig.h
	@touch $@
$(NVTT_SRC)/%.c: $(OBJDIRP)/3rdParty/nvtt/src/nvconfig.h
	@touch $@
.PRECIOUS: $(NVTT_SRC)/%.cpp $(NVTT_SRC)/%.c

$(OBJDIRP)/3rdParty/nvtt/libnvtt.a: $(OBJDIRP)/3rdParty/nvtt/src/nvconfig.h $(NVTT_OBJS)
	$(CMD)$(P_AR) $@ $(NVTT_OBJS)

$(OBJDIRP)/3rdParty/nvtt/src/nvconfig.h:
	@mkdir -p $(OBJDIRP)/3rdParty
	@echo "  make   nvtt"
	$(CMD)rm -rf $(OBJDIRP)/3rdParty/nvtt; cp -rp 3rdParty/nvtt $(OBJDIRP)/3rdParty | (cd $(OBJDIRP))
	$(CMD)(cd $(OBJDIRP)/3rdParty/nvtt/src; rm -f nvconfig.h; \
	  echo "#ifndef NV_CONFIG" > nvconfig.h; \
	  echo "#define NV_CONFIG" >> nvconfig.h; \
	  $(if $(call HAS_HEADER,unistd.h),echo "#define HAVE_UNISTD_H" >> nvconfig.h;,)\
	  $(if $(call HAS_HEADER,stdarg.h),echo "#define HAVE_STDARG_H" >> nvconfig.h;,)\
	  $(if $(call HAS_HEADER,signal.h),echo "#define HAVE_SIGNAL_H" >> nvconfig.h;,)\
	  $(if $(call HAS_HEADER,malloc.h),echo "#define HAVE_MALLOC_H" >> nvconfig.h;,)\
	  $(if $(call HAS_HEADER,execinfo.h),echo "#define HAVE_EXECINFO_H" >> nvconfig.h;,)\
	  echo "#endif" >> nvconfig.h)

$(OBJDIR)/Apps/Tools/GFxExport/ImageFormats/DXTHelper.o: $(OBJDIRP)/3rdParty/nvtt/libnvtt.a

endif


ifeq ($(strip $(call HAS_HEADER,lua.h)),)
BUILD_LUA := 1
endif
ifneq ($(strip $(BUILD_LUA)),)

#$(eval $(call BUILD_EXT_LIB,liblua,3rdParty/lua-5.1.4 \
#    lapi.c      lfunc.c   llex.c    lobject.c   lstate.c    ltablib.c  lundump.c \
#    lauxlib.c   ldebug.c  lgc.c     lopcodes.c  lstring.c   ltm.c      lvm.c \
#    lbaselib.c  ldo.c     linit.c   lmem.c      lstrlib.c   lua.c      lzio.c \
#    lcode.c     ldump.c   loadlib.c lparser.c   ltable.c    \
#))
INCLUDES += -I3rdParty/lua-5.1.4/src

endif

ifeq ($(P),local)
ifneq ($(strip $(shell which freetype-config 2>/dev/null)),)

CXXFLAGS += $(shell freetype-config --cflags) -DGFC_HAS_FREETYPE
GFXLIBS  += $(shell freetype-config --libs)

endif
endif
