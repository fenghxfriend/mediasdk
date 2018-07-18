LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE:= core
CORE_SRC_DIR	:= source
CORE_INC_DIR	:= include
COMM_DIR    		:= ../common

INC_PATH            := $(CORE_INC_DIR)\
                $(COMM_DIR)

CORE_SRC_CFILES	:= $(foreach i, $(CORE_SRC_DIR),$(wildcard $(LOCAL_PATH)/$(i)/*.c))
CORE_SRC_CPPFILES	:= $(foreach i, $(CORE_SRC_DIR),$(wildcard $(LOCAL_PATH)/$(i)/*.cpp))
CORE_SRC_CCFILES	:= $(foreach i, $(CORE_SRC_DIR),$(wildcard $(LOCAL_PATH)/$(i)/*.cc))

LOCAL_C_INCLUDES:=$(addprefix $(LOCAL_PATH)/,$(INC_PATH))

ALL_SRC_FILES := $(subst $(LOCAL_PATH)/,,$(CORE_SRC_CFILES))\
		$(subst $(LOCAL_PATH)/,,$(CORE_SRC_CPPFILES))\
		$(subst $(LOCAL_PATH)/,,$(CORE_SRC_CCFILES))


LOCAL_SRC_FILES := $(ALL_SRC_FILES)

LOCAL_CFLAGS += -fpic -fno-exceptions

LOCAL_CPPFLAGS += -DHAVE_PTHREADS=1 -DLOG_NDEBUG=0 -D__ANDROID__

LOCAL_PRELINK_MODULE:= false
LOCAL_LDLIBS += -lz -llog -ldl
include $(BUILD_SHARED_LIBRARY)


