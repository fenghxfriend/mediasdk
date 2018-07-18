LOCAL_PATH:= $(call my-dir)


include $(CLEAR_VARS)
LOCAL_MODULE := avfilter
LOCAL_SRC_FILES := ../easyffmpeg/lib/libavfilter.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../easyffmpeg/include
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := avutil
LOCAL_SRC_FILES := ../easyffmpeg/lib/libavutil.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../easyffmpeg/include
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := avcodec
LOCAL_SRC_FILES := ../easyffmpeg/lib/libavcodec.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../easyffmpeg/include
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := avforamt
LOCAL_SRC_FILES := ../easyffmpeg/lib/libavformat.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../easyffmpeg/include
LOCAL_STATIC_LIBRARIES := avutil
include $(PREBUILT_STATIC_LIBRARY)

#include $(CLEAR_VARS)
#LOCAL_MODULE := avresample
#LOCAL_SRC_FILES := ../easyffmpeg/lib/libavresample.a
#LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../easyffmpeg/include
#include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := swscale
LOCAL_SRC_FILES := ../easyffmpeg/lib/libswscale.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../easyffmpeg/include
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := swresample
LOCAL_SRC_FILES := ../easyffmpeg/lib/libswresample.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../easyffmpeg/include
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE:= mediasdk
MEDIASDK_SRC_DIR    := source module/source processor/source controller/source codec/source player/source io/source
MEDIASDK_INC_DIR    := include module/include processor/include controller/include codec/include player/include io/include
COMM_DIR    		 := ../common
RENDER_DIR    	     := ../render/include
AUDIOTRACK_DIR    	 := ../audiotrack/include ../audiotrack/filter/include
CODEC_DIR    	     := ../codec/include
MP4V2_DIR    	 	 := ../mp4v2/include
EASYFFMPEG_DIR    	 := ../easyffmpeg/include
WEBP_DIR    	     := ../webp/src
YUV_DIR    	         := ../libyuv/include
CORE_DIR    	     := ../core/include

INC_PATH    	 := $(MEDIASDK_INC_DIR) \
            $(CORE_DIR) \
            $(YUV_DIR) \
            $(WEBP_DIR) \
            $(RENDER_DIR) \
            $(AUDIOTRACK_DIR) \
			$(MP4V2_DIR) \
			$(CODEC_DIR) \
			$(COMM_DIR) \
			$(EASYFFMPEG_DIR)

MEDIASDK_SRC_CFILES		:= $(foreach i, $(MEDIASDK_SRC_DIR),$(wildcard $(LOCAL_PATH)/$(i)/*.c)) 
# $(warning  $(MEDIASDK_SRC_CFILES))
MEDIASDK_SRC_CPPFILES		:= $(foreach i, $(MEDIASDK_SRC_DIR),$(wildcard $(LOCAL_PATH)/$(i)/*.cpp)) 
# $(warning  $(MEDIASDK_SRC_CPPFILES))
MEDIASDK_SRC_CCFILES		:= $(foreach i, $(MEDIASDK_SRC_DIR),$(wildcard $(LOCAL_PATH)/$(i)/*.cc)) 
# $(warning  $(MEDIASDK_SRC_CCFILES))
# MEDIASDK_SRC_CPPFILES	:= $(wildcard $(LOCAL_PATH)/$(MEDIASDK_SRC_DIR)/*.cpp)
# MEDIASDK_SRC_CCFILES	:= $(wildcard $(LOCAL_PATH)/$(MEDIASDK_SRC_DIR)/*.cc)
		
LOCAL_C_INCLUDES:=$(addprefix $(LOCAL_PATH)/,$(INC_PATH))

LOCAL_SRC_FILES := $(subst $(LOCAL_PATH)/,,$(MEDIASDK_SRC_CFILES))\
		$(subst $(LOCAL_PATH)/,,$(MEDIASDK_SRC_CPPFILES))\
		$(subst $(LOCAL_PATH)/,,$(MEDIASDK_SRC_CCFILES))
		
LOCAL_CPPFLAGS += -DHAVE_PTHREADS=1 -DLOG_NDEBUG=0 -D__ANDROID__ -DHW_MEDIACODEC_ENABLE=0

LOCAL_CFLAGS += -fpic -fno-exceptions

ifeq ($(PWLIB_SUPPORT),1)
   LOCAL_C_INCLUDES += $(PWLIBDIR)/include/ptlib/unix $(PWLIBDIR)/include
endif

LOCAL_STATIC_LIBRARIES := avutil avforamt avcodec avfilter swscale swresample webpdecoder_static webpdemux imageio_util audiotrack
LOCAL_SHARED_LIBRARIES := soundtouch render mp4v2 yuv core

LOCAL_LDLIBS += -lc -lm -lz -ldl -llog -lGLESv2 -lEGL -lOpenSLES -lOpenMAXAL

include $(BUILD_SHARED_LIBRARY)