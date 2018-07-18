LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE:= interface
INTERFACE_SRC_DIR    := source
INTERFACE_INC_DIR    := include
COMM_DIR    		 := ../common
YUV_DIR    	         := ../libyuv/include
RENDER_DIR    	     := ../render/include
AUDIOTRACK_DIR    	 := ../audiotrack/include ../audiotrack/filter/include
WEBP_DIR    	     := ../webp/src
MEDIASDK_DIR    	 := ../mediasdk/include \
				        ../mediasdk/module/include \
				        ../mediasdk/controller/include \
				        ../mediasdk/processor/include \
				        ../mediasdk/codec/include \
				        ../mediasdk/player/include \
				        ../mediasdk/io/include

MP4V2_DIR    	 	 := ../mp4v2/include
INC_PATH    	 := $(INTERFACE_INC_DIR)\
                    $(YUV_DIR) \
                    $(WEBP_DIR) \
                    $(RENDER_DIR) \
                    $(AUDIOTRACK_DIR) \
                    $(MEDIASDK_DIR) \
                    $(MP4V2_DIR) \
                    $(COMM_DIR)

INTERFACE_SRC_CFILES		:= $(wildcard $(LOCAL_PATH)/$(INTERFACE_SRC_DIR)/*.c)
INTERFACE_SRC_CPPFILES	:= $(wildcard $(LOCAL_PATH)/$(INTERFACE_SRC_DIR)/*.cpp)
INTERFACE_SRC_CCFILES	:= $(wildcard $(LOCAL_PATH)/$(INTERFACE_SRC_DIR)/*.cc)
		
LOCAL_C_INCLUDES:=$(addprefix $(LOCAL_PATH)/,$(INC_PATH))

LOCAL_SRC_FILES := $(subst $(LOCAL_PATH)/,,$(INTERFACE_SRC_CFILES))\
		$(subst $(LOCAL_PATH)/,,$(INTERFACE_SRC_CPPFILES))\
		$(subst $(LOCAL_PATH)/,,$(INTERFACE_SRC_CCFILES))
		
LOCAL_CPPFLAGS += -DHAVE_PTHREADS=1 -DLOG_NDEBUG=0 -D__ANDROID__

LOCAL_CFLAGS += -fpic -fno-exceptions

ifeq ($(PWLIB_SUPPORT),1)
   LOCAL_C_INCLUDES += $(PWLIBDIR)/include/ptlib/unix $(PWLIBDIR)/include
endif

LOCAL_PRELINK_MODULE:= false
LOCAL_SHARED_LIBRARIES := yuv render mediasdk
LOCAL_LDLIBS += -lz -llog -ldl -lGLESv2 -lEGL -lOpenSLES -landroid -lOpenMAXAL -ljnigraphics

include $(BUILD_SHARED_LIBRARY)