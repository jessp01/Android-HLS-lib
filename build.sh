#!/bin/sh -x
set -e
pwd
  # required libs for android build tools
sudo apt-get update
sudo apt-get install -qq --force-yes libgd2-xpm ia32-libs ia32-libs-multiarch
  # for gradle output style
export TERM=dumb
  # newer version of gradle
#wget http://services.gradle.org/distributions/gradle-1.10-bin.zip
#unzip -qq -o gradle-1.10-bin.zip
#export GRADLE_HOME=$PWD/gradle-1.10
#export PATH=$GRADLE_HOME/bin:$PATH
  #- chmod +x gradlew
  # newest Android SDK 22.3
wget http://dl.google.com/android/android-sdk_r24.2-linux.tgz
tar zxf android-sdk_r24.2-linux.tgz
NDK_VER=r9c
export ANDROID_HOME=`pwd`/android-sdk-linux
export PATH=${PATH}:${ANDROID_HOME}/tools:${ANDROID_HOME}/platform-tools
# newest Android NDK
wget http://dl.google.com/android/ndk/android-ndk-$NDK_VER-linux-x86_64.tar.bz2
tar jxf android-ndk-$NDK_VER-linux-x86_64.tar.bz2
export ANDROID_NDK_HOME=`pwd`/android-ndk-$NDK_VER
export PATH=${PATH}:${ANDROID_HOME}/tools:${ANDROID_HOME}/platform-tools:${ANDROID_NDK_HOME}
  # manually set sdk.dir variable, according to local paths
echo "sdk.dir=$ANDROID_HOME" > local.properties
(while :; do echo 'y'; sleep 2; done) | android update sdk -a -t tools,platform-tools,extra-android-support,extra-android-m2repository,android-20,build-tools-20.0.0,extra-google-google_play_services,extra-google-m2repository --force --no-ui
(while :; do echo 'y'; sleep 2; done) | android update sdk -u
export NDK_PROJECT_PATH=`pwd`/HLSPlayerSDK/
sed -i "s@\${aapt}@${ANDROID_HOME}/tools/aapt@g" `pwd`/android-sdk-linux/tools/ant/build.xml
sed -i s#@NDK_BUILD_PATH@#`pwd`/android-ndk-$NDK_VER/ndk-build#g build.xml
ln -sf $ANDROID_HOME/build-tools/23.0.0_rc2/aapt ${ANDROID_HOME}/tools
ls -al ${ANDROID_HOME}/tools/aapt $ANDROID_HOME/build-tools/23.0.0_rc2/aapt
ldd ${ANDROID_HOME}/tools/tools/aapt
cd HLSPlayerSDK && ndk-build && ant release
