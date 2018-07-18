LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE:= render
RENDER_SRC_DIR	:= source filter/source
RENDER_INC_DIR	:= include filter/include
COMM_DIR    		:= ../common

INC_PATH            := $(RENDER_INC_DIR)\
                $(LIBYUV_DIR)\
                $(COMM_DIR)

RENDER_SRC_CFILES	:= $(foreach i, $(RENDER_SRC_DIR),$(wildcard $(LOCAL_PATH)/$(i)/*.c))
RENDER_SRC_CPPFILES	:= $(foreach i, $(RENDER_SRC_DIR),$(wildcard $(LOCAL_PATH)/$(i)/*.cpp))
RENDER_SRC_CCFILES	:= $(foreach i, $(RENDER_SRC_DIR),$(wildcard $(LOCAL_PATH)/$(i)/*.cc))

LOCAL_C_INCLUDES:=$(addprefix $(LOCAL_PATH)/,$(INC_PATH))

ALL_SRC_FILES := $(subst $(LOCAL_PATH)/,,$(RENDER_SRC_CFILES))\
		$(subst $(LOCAL_PATH)/,,$(RENDER_SRC_CPPFILES))\
		$(subst $(LOCAL_PATH)/,,$(RENDER_SRC_CCFILES))


LOCAL_SRC_FILES := $(ALL_SRC_FILES)

LOCAL_CPPFLAGS += -DHAVE_PTHREADS=1 -DLOG_NDEBUG=0 -D__ANDROID__

LOCAL_CFLAGS += -fpic -fno-exceptions

LOCAL_PRELINK_MODULE:= false
LOCAL_LDLIBS += -lz -llog -ldl -lGLESv2 -lEGL
include $(BUILD_SHARED_LIBRARY)


