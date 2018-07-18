LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE:= codec
COMM_DIR    		:= ../common

INC_PATH            := $(COMM_DIR)

LOCAL_C_INCLUDES:=$(addprefix $(LOCAL_PATH)/,$(INC_PATH))

LOCAL_SRC_FILES := $(wildcard $(LOCAL_PATH)/*.c)

LOCAL_CPPFLAGS += -DHAVE_PTHREADS=1 -DLOG_NDEBUG=0 -D__ANDROID__

LOCAL_CFLAGS += -fpic -fno-exceptions

ifeq ($(PWLIB_SUPPORT),1)
   LOCAL_C_INCLUDES += $(PWLIBDIR)/include/ptlib/unix $(PWLIBDIR)/include
endif

LOCAL_LDLIBS += -lz -llog -ldl -lGLESv2 -lEGL -lOpenSLES

include $(BUILD_SHARED_LIBRARY)