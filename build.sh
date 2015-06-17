#!/bin/sh -x
set -e
# newest Android NDK
NDK_VER=r9c
wget http://dl.google.com/android/ndk/android-ndk-$NDK_VER-linux-x86_64.tar.bz2
tar jxf android-ndk-$NDK_VER-linux-x86_64.tar.bz2
export ANDROID_NDK_HOME=`pwd`/android-ndk-$NDK_VER
export PATH=${PATH}:${ANDROID_HOME}/tools:${ANDROID_HOME}/platform-tools:${ANDROID_NDK_HOME}
  # manually set sdk.dir variable, according to local paths
echo "sdk.dir=$ANDROID_HOME" > local.properties
export NDK_PROJECT_PATH=`pwd`/HLSPlayerSDK/
#sed -i "s@\${aapt}@${ANDROID_HOME}/tools/aapt@g" `pwd`/android-sdk-linux/tools/ant/build.xml
sed -i s#@NDK_BUILD_PATH@#`pwd`/android-ndk-$NDK_VER/ndk-build#g build.xml
#ln -sf $ANDROID_HOME/build-tools/23.0.0_rc2/aapt ${ANDROID_HOME}/tools
#ls -al ${ANDROID_HOME}/tools/aapt $ANDROID_HOME/build-tools/23.0.0_rc2/aapt
#ldd ${ANDROID_HOME}/tools/tools/aapt
cd HLSPlayerSDK && ndk-build && ant release
