/*
 * HLSDecoder.h
 *
 *  Created on: Apr 9, 2014
 *      Author: Mark
 */

#ifndef HLSDECODER_H_
#define HLSDECODER_H_

#include <jni.h>

#include <../android-source/frameworks/av/include/media/stagefright/OMXClient.h>
#include <../android-source/frameworks/av/include/media/stagefright/MediaBuffer.h>
#include <../android-source/frameworks/av/include/media/stagefright/MediaSource.h>
#include <../android-source/frameworks/av/include/media/stagefright/MediaExtractor.h>
#include <../android-source/frameworks/av/include/media/stagefright/MetaData.h>
#include <../android-source/frameworks/av/include/media/stagefright/DataSource.h>
#include <../android-source/frameworks/av/include/media/stagefright/FileSource.h>
#include <../android-source/frameworks/av/include/media/stagefright/OMXCodec.h>
#include <../android-source/frameworks/av/include/media/stagefright/TimeSource.h>

#include <../android-source/frameworks/av/include/media/stagefright/AudioPlayer.h>
#include <../android-source/frameworks/av/include/media/MediaPlayerInterface.h>



#include <android/native_window.h>
#include <android/window.h>
class HLSAudioPlayer;
class HLSSegment;
class HLSMediaSourceAdapter;

#include <list>

class HLSDecoder
{
public:
	enum
	{
		STOPPED,
		PAUSED,
		PLAYING,
		SEEKING
	};
	HLSDecoder();
	~HLSDecoder();
	void Close(JNIEnv* env);

	void SetSurface(JNIEnv* env, jobject surface);

	android::status_t FeedSegment(const char* path);
	int FeedSegment(const unsigned char* data);

	void checkDrmStatus(const android::sp<android::DataSource>& dataSource);

	int Update();

	bool Play();



private:
	void SetStatus(int status);
	int mStatus;
	bool PlayNextSegment();
	std::list<HLSSegment* > mSegments;
	HLSSegment* mCurSegment;
	void SetNativeWindow(ANativeWindow* window);
	bool InitAudioDecoder();
	jobject mSurface;

	bool CreateAudioPlayer();
	bool CreateVideoPlayer();
	bool RenderBuffer(android::MediaBuffer* buffer);

	int mRenderedFrameCount;

	ANativeWindow* mWindow;

//	void SetAudioSource(android::sp<android::MediaSource> source);
//	void SetVideoSource(android::sp<android::MediaSource> source);

	android::OMXClient mClient;
	android::sp<android::MediaSource> mVideoDecoder;
//	android::sp<android::MediaSource> mVideoSource;
	android::sp<android::MediaSource> mAudioSource;
//	android::sp<android::MediaSource> mAudioTrack;
	android::sp<android::MediaSource> mOmxSource;
	android::sp<android::DataSource> mFileSource;
	android::sp<android::MediaExtractor> mExtractor;

	HLSMediaSourceAdapter* mAudioTrack;
	HLSMediaSourceAdapter* mVideoTrack;

	android::AudioPlayer* mAudioPlayer;
	android::sp<android::MediaPlayerBase::AudioSink> mAudioSink;
	android::TimeSource* mTimeSource;
	bool mOffloadAudio;
	int64_t mDurationUs;

	android::MediaBuffer* mVideoBuffer;

	int64_t mBitrate;
	int32_t mWidth;
	int32_t mHeight;
	int32_t mActiveAudioTrackIndex;
	uint32_t mExtractorFlags;
};



#endif /* HLSDECODER_H_ */
