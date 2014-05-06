LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)


LOCAL_MODULE    := HLSPlayerSDK
LOCAL_SRC_FILES := HLSPlayerSDK.cpp HLSSegment.cpp HLSMediaSourceAdapter.cpp HLSPlayer.cpp
LOCAL_CFLAGS += -DHAVE_SYS_UIO_H
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../android-source/frameworks/av/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../android-source/frameworks/native/include/media/openmax
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../android-source/frameworks/native/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../android-source/system/core/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../android-source/hardware/libhardware/include
LOCAL_C_INCLUDES += c:/ADT/ndk/android-ndk-r9d/sources/cxx-stl/gnu-libstdc++/4.6/include

LOCAL_LDLIBS += -Landroid-libs -lz -lm -llog -lstagefright -lutils -lbinder -lmedia -landroid
LOCAL_LDLIBS += c:/ADT/ndk/android-ndk-r9d/sources/cxx-stl/gnu-libstdc++/4.6/libs/armeabi-v7a/libsupc++.a
LOCAL_LDLIBS += c:/ADT/ndk/android-ndk-r9d/sources/cxx-stl/gnu-libstdc++/4.6/libs/armeabi-v7a/libgnustl_static.a

include $(BUILD_SHARED_LIBRARY)
