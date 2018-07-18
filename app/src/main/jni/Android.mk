LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

subdirs += $(LOCAL_PATH)/core/core.mk

subdirs += $(LOCAL_PATH)/mp4v2/mp4v2.mk

#subdirs += $(LOCAL_PATH)/libjpeg-turbo/jpeg.mk

subdirs += $(LOCAL_PATH)/soundtouch/soundtouch.mk

subdirs += $(LOCAL_PATH)/libyuv/yuv.mk

subdirs += $(LOCAL_PATH)/webp/Android.mk

subdirs += $(LOCAL_PATH)/audiotrack/audiotrack.mk

subdirs += $(LOCAL_PATH)/render/render.mk

#subdirs += $(LOCAL_PATH)/codec/codec.mk

subdirs += $(LOCAL_PATH)/mediasdk/mediasdk.mk

subdirs += $(LOCAL_PATH)/interface/interface.mk

include $(subdirs)
