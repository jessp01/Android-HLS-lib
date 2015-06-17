#!/bin/sh -x
set -e
export NDK_PROJECT_PATH=`pwd`/HLSPlayerSDK/
#sed -i "s@\${aapt}@${ANDROID_HOME}/tools/aapt@g" `pwd`/android-sdk-linux/tools/ant/build.xml
sed -i s#@NDK_BUILD_PATH@#`pwd`/android-ndk-$NDK_VER/ndk-build#g build.xml
#ln -sf $ANDROID_HOME/build-tools/23.0.0_rc2/aapt ${ANDROID_HOME}/tools
#ls -al ${ANDROID_HOME}/tools/aapt $ANDROID_HOME/build-tools/23.0.0_rc2/aapt
#ldd ${ANDROID_HOME}/tools/tools/aapt
cd HLSPlayerSDK && ndk-build && ant release
