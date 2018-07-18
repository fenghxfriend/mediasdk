APP_ABI := armeabi-v7a

NDK_TOOLCHAIN_VERSION := 4.9

APP_PLATFORM := android-19

APP_STL := gnustl_static

APP_CPPFLAGS := -frtti -std=c++11 -fpermissive -DLOG_ENABLE=1

APP_MODULES += core
APP_MODULES += mp4v2
#APP_MODULES += jpeg
APP_MODULES += soundtouch
APP_MODULES += yuv
APP_MODULES += webpdecoder_static
APP_MODULES += webpdemux
APP_MODULES += imageio_util
APP_MODULES += audiotrack
APP_MODULES += render
#APP_MODULES += codec
APP_MODULES += mediasdk
APP_MODULES += interface
