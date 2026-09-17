# 3rdParty dependencies — built from source as static libraries.

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
# curl 8.22.0
# ---------------------------------------------------------------------------
set(CURL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/curl-8.22.0)

add_library(curl STATIC
    ${CURL_DIR}/libmak/amigaos.c
    ${CURL_DIR}/libmak/asyn-ares.c
    ${CURL_DIR}/libmak/asyn-thread.c
    ${CURL_DIR}/libmak/axtls.c
    ${CURL_DIR}/libmak/base64.c
    ${CURL_DIR}/libmak/bundles.c
    ${CURL_DIR}/libmak/conncache.c
    ${CURL_DIR}/libmak/connect.c
    ${CURL_DIR}/libmak/content_encoding.c
    ${CURL_DIR}/libmak/cookie.c
    ${CURL_DIR}/libmak/curl_addrinfo.c
    ${CURL_DIR}/libmak/curl_darwinssl.c
    ${CURL_DIR}/libmak/curl_fnmatch.c
    ${CURL_DIR}/libmak/curl_gethostname.c
    ${CURL_DIR}/libmak/curl_gssapi.c
    ${CURL_DIR}/libmak/curl_memrchr.c
    ${CURL_DIR}/libmak/curl_multibyte.c
    ${CURL_DIR}/libmak/curl_ntlm.c
    ${CURL_DIR}/libmak/curl_ntlm_core.c
    ${CURL_DIR}/libmak/curl_ntlm_msgs.c
    ${CURL_DIR}/libmak/curl_ntlm_wb.c
    ${CURL_DIR}/libmak/curl_rand.c
    ${CURL_DIR}/libmak/curl_rtmp.c
    ${CURL_DIR}/libmak/curl_sasl.c
    ${CURL_DIR}/libmak/curl_schannel.c
    ${CURL_DIR}/libmak/curl_sspi.c
    ${CURL_DIR}/libmak/curl_threads.c
    ${CURL_DIR}/libmak/cyassl.c
    ${CURL_DIR}/libmak/dict.c
    ${CURL_DIR}/libmak/easy.c
    ${CURL_DIR}/libmak/escape.c
    ${CURL_DIR}/libmak/file.c
    ${CURL_DIR}/libmak/fileinfo.c
    ${CURL_DIR}/libmak/formdata.c
    ${CURL_DIR}/libmak/ftp.c
    ${CURL_DIR}/libmak/ftplistparser.c
    ${CURL_DIR}/libmak/getenv.c
    ${CURL_DIR}/libmak/getinfo.c
    ${CURL_DIR}/libmak/gopher.c
    ${CURL_DIR}/libmak/gtls.c
    ${CURL_DIR}/libmak/hash.c
    ${CURL_DIR}/libmak/hmac.c
    ${CURL_DIR}/libmak/hostasyn.c
    ${CURL_DIR}/libmak/hostcheck.c
    ${CURL_DIR}/libmak/hostip.c
    ${CURL_DIR}/libmak/hostip4.c
    ${CURL_DIR}/libmak/hostip6.c
    ${CURL_DIR}/libmak/hostsyn.c
    ${CURL_DIR}/libmak/http.c
    ${CURL_DIR}/libmak/http_chunks.c
    ${CURL_DIR}/libmak/http_digest.c
    ${CURL_DIR}/libmak/http_negotiate.c
    ${CURL_DIR}/libmak/http_negotiate_sspi.c
    ${CURL_DIR}/libmak/http_proxy.c
    ${CURL_DIR}/libmak/idn_win32.c
    ${CURL_DIR}/libmak/if2ip.c
    ${CURL_DIR}/libmak/imap.c
    ${CURL_DIR}/libmak/inet_ntop.c
    ${CURL_DIR}/libmak/inet_pton.c
    ${CURL_DIR}/libmak/krb4.c
    ${CURL_DIR}/libmak/krb5.c
    ${CURL_DIR}/libmak/ldap.c
    ${CURL_DIR}/libmak/llist.c
    ${CURL_DIR}/libmak/md4.c
    ${CURL_DIR}/libmak/md5.c
    ${CURL_DIR}/libmak/memdebug.c
    ${CURL_DIR}/libmak/mprintf.c
    ${CURL_DIR}/libmak/multi.c
    ${CURL_DIR}/libmak/netrc.c
    ${CURL_DIR}/libmak/non-ascii.c
    ${CURL_DIR}/libmak/nonblock.c
    ${CURL_DIR}/libmak/nss.c
    ${CURL_DIR}/libmak/nwlib.c
    ${CURL_DIR}/libmak/nwos.c
    ${CURL_DIR}/libmak/openldap.c
    ${CURL_DIR}/libmak/parsedate.c
    ${CURL_DIR}/libmak/pingpong.c
    ${CURL_DIR}/libmak/polarssl.c
    ${CURL_DIR}/libmak/pop3.c
    ${CURL_DIR}/libmak/progress.c
    ${CURL_DIR}/libmak/qssl.c
    ${CURL_DIR}/libmak/rawstr.c
    ${CURL_DIR}/libmak/rtsp.c
    ${CURL_DIR}/libmak/security.c
    ${CURL_DIR}/libmak/select.c
    ${CURL_DIR}/libmak/sendf.c
    ${CURL_DIR}/libmak/share.c
    ${CURL_DIR}/libmak/slist.c
    ${CURL_DIR}/libmak/smtp.c
    ${CURL_DIR}/libmak/socks.c
    ${CURL_DIR}/libmak/socks_gssapi.c
    ${CURL_DIR}/libmak/socks_sspi.c
    ${CURL_DIR}/libmak/speedcheck.c
    ${CURL_DIR}/libmak/splay.c
    ${CURL_DIR}/libmak/ssh.c
    ${CURL_DIR}/libmak/sslgen.c
    ${CURL_DIR}/libmak/ssluse.c
    ${CURL_DIR}/libmak/strdup.c
    ${CURL_DIR}/libmak/strequal.c
    ${CURL_DIR}/libmak/strerror.c
    ${CURL_DIR}/libmak/strtok.c
    ${CURL_DIR}/libmak/strtoofft.c
    ${CURL_DIR}/libmak/telnet.c
    ${CURL_DIR}/libmak/tftp.c
    ${CURL_DIR}/libmak/timeval.c
    ${CURL_DIR}/libmak/transfer.c
    ${CURL_DIR}/libmak/url.c
    ${CURL_DIR}/libmak/version.c
    ${CURL_DIR}/libmak/warnless.c
    ${CURL_DIR}/libmak/wildcard.c
)

target_include_directories(curl PUBLIC ${CURL_DIR})
target_compile_definitions(curl PRIVATE WIN32 $<$<CONFIG:Debug>:_DEBUG> $<$<NOT:$<CONFIG:Debug>>:NDEBUG>)
set_target_properties(curl PROPERTIES OUTPUT_NAME "libcurl")

# ---------------------------------------------------------------------------
# zlib 1.3.2
# ---------------------------------------------------------------------------
set(ZLIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.3.2)

add_library(zlib STATIC
    ${ZLIB_DIR}/adler32.c
    ${ZLIB_DIR}/compress.c
    ${ZLIB_DIR}/crc32.c
    ${ZLIB_DIR}/deflate.c
    ${ZLIB_DIR}/infback.c
    ${ZLIB_DIR}/inffast.c
    ${ZLIB_DIR}/inflate.c
    ${ZLIB_DIR}/inftrees.c
    ${ZLIB_DIR}/trees.c
    ${ZLIB_DIR}/uncompr.c
    ${ZLIB_DIR}/zutil.c
)
target_include_directories(zlib PUBLIC ${ZLIB_DIR})
target_compile_definitions(zlib PRIVATE WIN32 $<$<CONFIG:Debug>:_DEBUG> $<$<NOT:$<CONFIG:Debug>>:NDEBUG>)
set_target_properties(zlib PROPERTIES OUTPUT_NAME "zlib")

# ---------------------------------------------------------------------------
# libpng 1.6.53
# ---------------------------------------------------------------------------
set(PNG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libpng-1.6.53)

add_library(libpng STATIC
    ${PNG_DIR}/png.c
    ${PNG_DIR}/pngerror.c
    ${PNG_DIR}/pngget.c
    ${PNG_DIR}/pngmem.c
    ${PNG_DIR}/pngpread.c
    ${PNG_DIR}/pngread.c
    ${PNG_DIR}/pngrio.c
    ${PNG_DIR}/pngrtran.c
    ${PNG_DIR}/pngrutil.c
    ${PNG_DIR}/pngset.c
    ${PNG_DIR}/pngtrans.c
    ${PNG_DIR}/pngwio.c
    ${PNG_DIR}/pngwrite.c
    ${PNG_DIR}/pngwtran.c
    ${PNG_DIR}/pngwutil.c
)
target_include_directories(libpng PUBLIC ${PNG_DIR})
target_link_libraries(libpng PRIVATE zlib)
target_compile_definitions(libpng PRIVATE WIN32
    $<$<CONFIG:Debug>:_DEBUG;DEBUG;PNG_DEBUG=1>
    $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
)
set_target_properties(libpng PROPERTIES OUTPUT_NAME "libpng")

# ---------------------------------------------------------------------------
# expat 2.7.5
# ---------------------------------------------------------------------------
set(EXPAT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/expat-2.7.5)

add_library(expat STATIC
    ${EXPAT_DIR}/lib/xmlparse.c
    ${EXPAT_DIR}/lib/xmlrole.c
    ${EXPAT_DIR}/lib/xmltok.c
    ${EXPAT_DIR}/lib/xmltok_impl.c
    ${EXPAT_DIR}/lib/xmltok_ns.c
)
target_include_directories(expat PUBLIC ${EXPAT_DIR}/lib)
target_compile_definitions(expat PRIVATE WIN32 COMPILED_FROM_DSP
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
)
set_target_properties(expat PROPERTIES OUTPUT_NAME "expat")

# ---------------------------------------------------------------------------
# libjpeg 10
# ---------------------------------------------------------------------------
set(JPEG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/jpeg-10)

add_library(libjpeg STATIC
    ${JPEG_DIR}/jaricom.c
    ${JPEG_DIR}/jcapimin.c
    ${JPEG_DIR}/jcapistd.c
    ${JPEG_DIR}/jcarith.c
    ${JPEG_DIR}/jccoefct.c
    ${JPEG_DIR}/jccolor.c
    ${JPEG_DIR}/jcdctmgr.c
    ${JPEG_DIR}/jcinit.c
    ${JPEG_DIR}/jcmainct.c
    ${JPEG_DIR}/jcmarker.c
    ${JPEG_DIR}/jcmaster.c
    ${JPEG_DIR}/jcomapi.c
    ${JPEG_DIR}/jcparam.c
    ${JPEG_DIR}/jcprepct.c
    ${JPEG_DIR}/jcsample.c
    ${JPEG_DIR}/jctrans.c
    ${JPEG_DIR}/jdapimin.c
    ${JPEG_DIR}/jdapistd.c
    ${JPEG_DIR}/jdarith.c
    ${JPEG_DIR}/jdatadst.c
    ${JPEG_DIR}/jdatasrc.c
    ${JPEG_DIR}/jdcoefct.c
    ${JPEG_DIR}/jdcolor.c
    ${JPEG_DIR}/jddctmgr.c
    ${JPEG_DIR}/jdhuff.c
    ${JPEG_DIR}/jdinput.c
    ${JPEG_DIR}/jdmainct.c
    ${JPEG_DIR}/jdmarker.c
    ${JPEG_DIR}/jdmaster.c
    ${JPEG_DIR}/jdmerge.c
    ${JPEG_DIR}/jdpostct.c
    ${JPEG_DIR}/jdsample.c
    ${JPEG_DIR}/jdtrans.c
    ${JPEG_DIR}/jerror.c
    ${JPEG_DIR}/jfdctflt.c
    ${JPEG_DIR}/jfdctfst.c
    ${JPEG_DIR}/jfdctint.c
    ${JPEG_DIR}/jidctflt.c
    ${JPEG_DIR}/jidctfst.c
    ${JPEG_DIR}/jidctint.c
    ${JPEG_DIR}/jmemmgr.c
    ${JPEG_DIR}/jmemnobs.c
    ${JPEG_DIR}/jquant1.c
    ${JPEG_DIR}/jquant2.c
    ${JPEG_DIR}/jutils.c
)
target_include_directories(libjpeg PUBLIC ${JPEG_DIR})
target_compile_definitions(libjpeg PRIVATE WIN32 _LIB _CRT_SECURE_NO_WARNINGS
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
)
set_target_properties(libjpeg PROPERTIES OUTPUT_NAME "libjpeg")

# ---------------------------------------------------------------------------
# pcre2-10.47
# ---------------------------------------------------------------------------
set(PCRE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/pcre2-10.47)

add_library(pcre STATIC
    ${PCRE_DIR}/src/pcre2.h
    ${PCRE_DIR}/src/pcre2_auto_possess.c
    ${PCRE_DIR}/src/pcre2_chartables.c
    ${PCRE_DIR}/src/pcre2_chkdint.c
    ${PCRE_DIR}/src/pcre2_compile.c
    ${PCRE_DIR}/src/pcre2_compile_cgroup.c
    ${PCRE_DIR}/src/pcre2_compile_class.c
    ${PCRE_DIR}/src/pcre2_config.c
    ${PCRE_DIR}/src/pcre2_context.c
    ${PCRE_DIR}/src/pcre2_convert.c
    ${PCRE_DIR}/src/pcre2_dfa_match.c
    ${PCRE_DIR}/src/pcre2_error.c
    ${PCRE_DIR}/src/pcre2_extuni.c
    ${PCRE_DIR}/src/pcre2_find_bracket.c
    ${PCRE_DIR}/src/pcre2_jit_compile.c
    ${PCRE_DIR}/src/pcre2_maketables.c
    ${PCRE_DIR}/src/pcre2_match.c
    ${PCRE_DIR}/src/pcre2_match_data.c
    ${PCRE_DIR}/src/pcre2_match_next.c
    ${PCRE_DIR}/src/pcre2_newline.c
    ${PCRE_DIR}/src/pcre2_ord2utf.c
    ${PCRE_DIR}/src/pcre2_pattern_info.c
    ${PCRE_DIR}/src/pcre2_script_run.c
    ${PCRE_DIR}/src/pcre2_serialize.c
    ${PCRE_DIR}/src/pcre2_string_utils.c
    ${PCRE_DIR}/src/pcre2_study.c
    ${PCRE_DIR}/src/pcre2_substitute.c
    ${PCRE_DIR}/src/pcre2_substring.c
    ${PCRE_DIR}/src/pcre2_tables.c
    ${PCRE_DIR}/src/pcre2_ucd.c
    ${PCRE_DIR}/src/pcre2_valid_utf.c
    ${PCRE_DIR}/src/pcre2_xclass.c
)
target_include_directories(pcre PUBLIC ${PCRE_DIR})
target_compile_definitions(pcre PRIVATE WIN32 HAVE_CONFIG_H _CRT_SECURE_NO_WARNINGS PCRE2_CODE_UNIT_WIDTH=8 PCRE2_STATIC
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
)
set_target_properties(pcre PROPERTIES OUTPUT_NAME "pcre")

# ---------------------------------------------------------------------------
# fmod (prebuilt vendor library — IMPORTED target)
# ---------------------------------------------------------------------------
set(FMOD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/fmod-2.03.13/core)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(FMOD_PLATFORM "x64")
    set(FMOD_LIB_NAME "fmod_vc")
else()
    set(FMOD_PLATFORM "Win32")
    set(FMOD_LIB_NAME "fmod_vc")
endif()

add_library(fmod STATIC IMPORTED GLOBAL)
set_target_properties(fmod PROPERTIES
    IMPORTED_LOCATION "${FMOD_DIR}/lib/${FMOD_PLATFORM}/${FMOD_LIB_NAME}.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${FMOD_DIR}/inc"
)

# ---------------------------------------------------------------------------
# nvtt-2.1.2 (NVIDIA Texture Tools — texture compression)
# ---------------------------------------------------------------------------
set(NVTT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/nvtt-2.1.2)

add_subdirectory(${NVTT_DIR})

#target_compile_definitions(nvtt PRIVATE
#    WIN32 NVTT_EXPORTS NVTT_SHARED __SSE2__ __SSE__ __MMX__
#    $<$<CONFIG:Debug>:_DEBUG>
#    $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
#)
set_target_properties(nvtt PROPERTIES OUTPUT_NAME "nvtt")

# ---------------------------------------------------------------------------
# Final Configue
# ---------------------------------------------------------------------------

if(MSVC)
    target_compile_options(curl PRIVATE "/W4")
    target_compile_options(zlib PRIVATE "/W4")
    target_compile_options(libpng PRIVATE "/W4")
    target_compile_options(expat PRIVATE "/W4")
    target_compile_options(libjpeg PRIVATE "/W4")
    target_compile_options(pcre PRIVATE "/W4")
    target_compile_options(nvtt PRIVATE "/W4")
endif()
