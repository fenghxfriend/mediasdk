LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE:= audiotrack
AUDIO_TRACK_SRC_DIR	:= source filter/source
AUDIO_TRACK_INC_DIR	:= include filter/include
COMM_DIR    		:= ../common
EASYFFMPEG_DIR    	:= ../easyffmpeg/include
SOUNDTOUCH_DIR      := ../soundtouch/include

INC_PATH            := $(AUDIO_TRACK_INC_DIR)\
                $(COMM_DIR) \
                $(EASYFFMPEG_DIR) \
                $(SOUNDTOUCH_DIR)

AUDIO_TRACK_SRC_CFILES	:= $(foreach i, $(AUDIO_TRACK_SRC_DIR),$(wildcard $(LOCAL_PATH)/$(i)/*.c))
AUDIO_TRACK_SRC_CPPFILES	:= $(foreach i, $(AUDIO_TRACK_SRC_DIR),$(wildcard $(LOCAL_PATH)/$(i)/*.cpp))
AUDIO_TRACK_SRC_CCFILES	:= $(foreach i, $(AUDIO_TRACK_SRC_DIR),$(wildcard $(LOCAL_PATH)/$(i)/*.cc))

LOCAL_C_INCLUDES:=$(addprefix $(LOCAL_PATH)/,$(INC_PATH))

ALL_SRC_FILES := $(subst $(LOCAL_PATH)/,,$(AUDIO_TRACK_SRC_CFILES))\
		$(subst $(LOCAL_PATH)/,,$(AUDIO_TRACK_SRC_CPPFILES))\
		$(subst $(LOCAL_PATH)/,,$(AUDIO_TRACK_SRC_CCFILES))


LOCAL_SRC_FILES := $(ALL_SRC_FILES)

LOCAL_CPPFLAGS += -DHAVE_PTHREADS=1 -DLOG_NDEBUG=0 -D__ANDROID__

LOCAL_CFLAGS += -fpic -fno-exceptions

LOCAL_PRELINK_MODULE:= false
LOCAL_LDLIBS += -lz -llog -ldl -lOpenSLES
include $(BUILD_STATIC_LIBRARY)


