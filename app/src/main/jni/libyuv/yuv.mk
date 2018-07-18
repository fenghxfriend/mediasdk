# This is the Android makefile for libyuv for NDK.
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CPP_EXTENSION := .cc

LOCAL_SRC_FILES := \
    source/compare.cc           \
    source/compare_common.cc    \
    source/convert.cc           \
    source/convert_argb.cc      \
    source/convert_from.cc      \
    source/convert_from_argb.cc \
    source/convert_to_argb.cc   \
    source/convert_to_i420.cc   \
    source/cpu_id.cc            \
    source/planar_functions.cc  \
    source/rotate.cc            \
    source/rotate_any.cc        \
    source/rotate_argb.cc       \
    source/rotate_common.cc     \
    source/row_any.cc           \
    source/row_common.cc        \
    source/scale.cc             \
    source/scale_any.cc         \
    source/scale_argb.cc        \
    source/scale_common.cc      \
    source/video_common.cc

#common_CFLAGS := -Wall -fexceptions
#ifneq ($(LIBYUV_DISABLE_JPEG), "yes")
#LOCAL_SRC_FILES += \
#    source/convert_jpeg.cc      \
#    source/mjpeg_decoder.cc     \
#    source/mjpeg_validate.cc
#common_CFLAGS += -DHAVE_JPEG
#LOCAL_SHARED_LIBRARIES := jpeg
#endif

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
    LOCAL_CFLAGS += -D__ARM_NEON__ -mfpu=neon
    LOCAL_SRC_FILES += \
        source/compare_neon.cc \
        source/rotate_neon.cc \
        source/row_neon.cc \
        source/scale_neon.cc

endif

ifeq ($(TARGET_ARCH_ABI),arm64-v8a)
    LOCAL_CFLAGS += -D__aarch64__ -mfpu=neon
    LOCAL_SRC_FILES += \
            source/compare_neon64.cc \
            source/rotate_neon64.cc \
            source/row_neon64.cc \
            source/scale_neon64.cc
endif

ifeq ($(TARGET_ARCH_ABI),$(filter $(TARGET_ARCH_ABI), x86 x86_64))
    LOCAL_SRC_FILES += \
            source/compare_gcc.cc \
            source/rotate_gcc.cc \
            source/row_gcc.cc \
            source/scale_gcc.cc
endif

ifeq ($(TARGET_ARCH_ABI),$(filter $(TARGET_ARCH_ABI), mips mips64))
    LOCAL_CFLAGS += -D__mips_msa
    LOCAL_SRC_FILES += \
            source/compare_msa.cc \
            source/rotate_msa.cc \
            source/row_msa.cc \
            source/scale_msa.cc
endif

LOCAL_CFLAGS += $(common_CFLAGS)
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include

LOCAL_MODULE := yuv_static
LOCAL_MODULE_TAGS := optional

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_WHOLE_STATIC_LIBRARIES := yuv_static
LOCAL_MODULE := yuv
#ifneq ($(LIBYUV_DISABLE_JPEG), "yes")
#LOCAL_SHARED_LIBRARIES := jpeg
#endif

include $(BUILD_SHARED_LIBRARY)
