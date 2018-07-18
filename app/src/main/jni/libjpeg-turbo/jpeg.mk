# Makefile for libjpeg-turbo
 
ifneq ($(TARGET_SIMULATOR),true)
 
##################################################
###                simd                        ###
##################################################
LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)

ifeq ($(ARCH_ARM_HAVE_NEON),true)
LOCAL_CFLAGS += -D__ARM_HAVE_NEON
endif

# From autoconf-generated Makefile
EXTRA_DIST = simd/nasm_lt.sh simd/jcclrmmx.asm simd/jcclrss2.asm simd/jdclrmmx.asm simd/jdclrss2.asm \
	simd/jdmrgmmx.asm simd/jdmrgss2.asm simd/jcclrss2-64.asm simd/jdclrss2-64.asm \
	simd/jdmrgss2-64.asm simd/CMakeLists.txt
 
libsimd_SOURCES_DIST = simd/jsimd_arm_neon.S \
                       simd/jsimd_arm.c 

LOCAL_SRC_FILES := $(libsimd_SOURCES_DIST)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/simd
 
AM_CFLAGS := -march=armv7-a -mfpu=neon
AM_CCASFLAGS := -march=armv7-a -mfpu=neon
 
LOCAL_MODULE_TAGS := debug
 
LOCAL_MODULE := simd
 
include $(BUILD_STATIC_LIBRARY)
 
######################################################
###           libjpeg.so                       ##
######################################################
 
include $(CLEAR_VARS)

# From autoconf-generated Makefile
libjpeg_SOURCES_DIST =  jcapimin.c jcapistd.c jccoefct.c jccolor.c \
        jcdctmgr.c jchuff.c jcinit.c jcmainct.c jcmarker.c jcmaster.c \
        jcomapi.c jcparam.c jcphuff.c jcprepct.c jcsample.c jctrans.c \
        jdapimin.c jdapistd.c jdatadst.c jdatasrc.c jdcoefct.c jdcolor.c \
        jddctmgr.c jdhuff.c jdinput.c jdmainct.c jdmarker.c jdmaster.c \
        jdmerge.c jdphuff.c jdpostct.c jdsample.c jdtrans.c jerror.c \
        jfdctflt.c jfdctfst.c jfdctint.c jidctflt.c jidctfst.c jidctint.c \
        jidctred.c jquant1.c jquant2.c jutils.c jmemmgr.c jmemnobs.c \
	jaricom.c jcarith.c jdarith.c \
	turbojpeg.c transupp.c jdatadst-tj.c jdatasrc-tj.c \
	turbojpeg-mapfile

LOCAL_SRC_FILES:= $(libjpeg_SOURCES_DIST)

LOCAL_STATIC_LIBRARIES := simd
 
LOCAL_C_INCLUDES := $(LOCAL_PATH) 
 
LOCAL_CFLAGS := -DAVOID_TABLES  -O3 -fstrict-aliasing -fprefetch-loop-arrays  -DANDROID \
        -DANDROID_TILE_BASED_DECODE -DENABLE_ANDROID_NULL_CONVERT

LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_STATIC_LIBRARY)
 
LOCAL_MODULE_TAGS := debug
 
LOCAL_MODULE := jpeg

include $(BUILD_SHARED_LIBRARY)

endif  # TARGET_SIMULATOR != true
