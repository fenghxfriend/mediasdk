LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE:= soundtouch
SOUNDTOUCH_SRC_DIR	:= ./ source
SOUNDTOUCH_INC_DIR	:= ./ include

INC_PATH            := $(SOUNDTOUCH_INC_DIR)\
                $(LIBYUV_DIR)\
                $(COMM_DIR)

SOUNDTOUCH_SRC_CFILES	:= $(foreach i, $(SOUNDTOUCH_SRC_DIR),$(wildcard $(LOCAL_PATH)/$(i)/*.c))
SOUNDTOUCH_SRC_CPPFILES	:= $(foreach i, $(SOUNDTOUCH_SRC_DIR),$(wildcard $(LOCAL_PATH)/$(i)/*.cpp))
SOUNDTOUCH_SRC_CCFILES	:= $(foreach i, $(SOUNDTOUCH_SRC_DIR),$(wildcard $(LOCAL_PATH)/$(i)/*.cc))

LOCAL_C_INCLUDES:=$(addprefix $(LOCAL_PATH)/,$(INC_PATH))

ALL_SRC_FILES := $(subst $(LOCAL_PATH)/,,$(SOUNDTOUCH_SRC_CFILES))\
		$(subst $(LOCAL_PATH)/,,$(SOUNDTOUCH_SRC_CPPFILES))\
		$(subst $(LOCAL_PATH)/,,$(SOUNDTOUCH_SRC_CCFILES))


LOCAL_SRC_FILES := $(ALL_SRC_FILES)

LOCAL_CPPFLAGS += \
	-DHAVE_PTHREADS=1 \
	-DLOG_NDEBUG=0

LOCAL_CFLAGS := -D__ANDROID__ -fpic -fexceptions -fvisibility=hidden -fdata-sections -ffunction-sections

LOCAL_PRELINK_MODULE:= false
LOCAL_LDLIBS += -llog
include $(BUILD_SHARED_LIBRARY)