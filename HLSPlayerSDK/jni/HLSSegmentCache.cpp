#include <assert.h>
#include "HLSSegmentCache.h"

// Interface to the HLSSegmentCache Java subsystem.
JavaVM *HLSSegmentCache::mJVM = NULL;
jmethodID HLSSegmentCache::mPrecache = 0;
jmethodID HLSSegmentCache::mRead = 0;
jmethodID HLSSegmentCache::mGetSize = 0;
jclass HLSSegmentCache::mClass = 0;

void HLSSegmentCache::initialize(JavaVM *jvm)
{
	LOGI("Initializing...");

	// Keep reference to the JBM.
	mJVM = jvm;

	// Set up environment for this thread.
	JNIEnv *env = NULL;
	mJVM->AttachCurrentThread(&env, NULL);

	// Look up the class.
	jclass c = env->FindClass("com/kaltura/hlsplayersdk/cache/HLSSegmentCache");
	if ( env->ExceptionCheck() || c == NULL) {
		LOGE("Could not find class com/kaltura/cache/HLSSegmentCache" );
		mClass = NULL;
		return;
	}
	mClass = (jclass)env->NewGlobalRef((jobject)c);

	// Get the static methods.
	mPrecache = env->GetStaticMethodID(mClass, "precache", "(Ljava/lang/String;)V" );
	if (env->ExceptionCheck())
	{
		LOGE("Could not find method com/kaltura/hlsplayersdk/cache/HLSSegmentCache.precache" );
		return;
	}

	mRead = env->GetStaticMethodID(mClass, "read", "(Ljava/lang/String;JJLjava/nio/ByteBuffer;)J" );
	if (env->ExceptionCheck())
	{
		LOGE("Could not find method com/kaltura/hlsplayersdk/cache/HLSSegmentCache.read" );
		return;
	}

	mGetSize = env->GetStaticMethodID(mClass, "getSize", "(Ljava/lang/String;)J" );
	if (env->ExceptionCheck())
	{
		LOGE("Could not find method com/kaltura/hlsplayersdk/cache/HLSSegmentCache.getSize" );
		return;
	}

	LOGI("DONE");
}

void HLSSegmentCache::precache(const char *uri)
{
	assert(mJVM); // Didn't initialize.

	// Set up environment for this thread.
	JNIEnv *env = NULL;
	mJVM->AttachCurrentThread(&env, NULL);

	jstring juri = env->NewStringUTF(uri);
	env->CallStaticVoidMethod(mClass, mPrecache, juri);
}

int64_t HLSSegmentCache::read(const char *uri, int64_t offset, int64_t size, void *bytes)
{
	assert(mJVM); // Didn't initialize.
	assert(mClass);
	assert(mRead);

	// Set up environment for this thread.
	JNIEnv *env = NULL;
	mJVM->AttachCurrentThread(&env, NULL);

	jobject jbytes = env->NewDirectByteBuffer(bytes, size);
	jstring juri = env->NewStringUTF(uri);

	LOGV2("%s offset=%lld size=%lld bytes=%p", uri, offset, size, bytes);

	int64_t res = env->CallStaticLongMethod(mClass, mRead, juri, offset, size, jbytes);

	env->DeleteLocalRef(jbytes);
	env->DeleteLocalRef(juri);

	return res;
}

int64_t HLSSegmentCache::getSize(const char *uri)
{
	assert(mJVM); // Didn't initialize.

	// Set up environment for this thread.
	JNIEnv *env = NULL;
	mJVM->AttachCurrentThread(&env, NULL);

	jstring juri = env->NewStringUTF(uri);
	return env->CallStaticLongMethod(mClass, mGetSize, juri);
}