#!/bin/sh -x
set -e
# newest Android NDK
NDK_VER=r9c
wget http://dl.google.com/android/ndk/android-ndk-$NDK_VER-linux-x86_64.tar.bz2
tar jxvf android-ndk-$NDK_VER-linux-x86_64.tar.bz2
export ANDROID_NDK_HOME=`pwd`/android-ndk-$NDK_VER
export PATH=${PATH}:${ANDROID_NDK_HOME}
  # manually set sdk.dir variable, according to local paths
#echo "sdk.dir=$ANDROID_HOME" > local.properties
#echo "sdk.dir=$ANDROID_HOME" > `pwd`/HLSPlayerSDK/local.properties
export NDK_PROJECT_PATH=`pwd`/HLSPlayerSDK/
#sed -i "s@\${aapt}@${ANDROID_HOME}/tools/aapt@g" `pwd`/android-sdk-linux/tools/ant/build.xml
sed -i s#@NDK_BUILD_PATH@#`pwd`/android-ndk-$NDK_VER/ndk-build#g build.xml
cd HLSPlayerSDK &&  ndk-build && ant release
