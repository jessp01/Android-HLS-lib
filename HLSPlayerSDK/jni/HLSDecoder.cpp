/*
 * HLSDecoder.cpp
 *
 *  Created on: Apr 9, 2014
 *      Author: Mark
 */
#include <jni.h>
#include "HLSDecoder.h"
#include <android/log.h>

#include <android/native_window_jni.h>

#include <../android-source/frameworks/av/include/media/stagefright/MediaDefs.h>
#include <../android-source/frameworks/native/include/ui/GraphicBuffer.h>
#include <../android-source/frameworks/av/include/media/stagefright/ColorConverter.h>
#include <../android-source/frameworks/av/include/media/stagefright/Utils.h>

#include "HLSSegment.h"
#include "debug.h"
#include "constants.h"


#define CLASS_NAME APP_NAME"::HLSDecoder"




using namespace android;

#define METHOD CLASS_NAME"::HLSDecoder()"
HLSDecoder::HLSDecoder() :
		mCurSegment(NULL), mVideoDecoder(NULL), mExtractorFlags(0),
		mHeight(0), mWidth(0), mBitrate(0), mActiveAudioTrackIndex(-1),
		mVideoBuffer(NULL), mWindow(NULL), mSurface(NULL), mRenderedFrameCount(0),
		mAudioPlayer(NULL), mAudioSink(NULL), mTimeSource(NULL),
		mDurationUs(0), mOffloadAudio(false), mStatus(HLSDecoder::STOPPED),
		mAudioTrack(NULL), mVideoTrack(NULL)
{
	status_t status = mClient.connect();
	LOGINFO(METHOD, "OMXClient::Connect return %d", status);
	mAudioTrack = new HLSMediaSourceAdapter();
	mVideoTrack = new HLSMediaSourceAdapter();
}

#define METHOD CLASS_NAME"::~HLSDecoder()"
HLSDecoder::~HLSDecoder()
{

}

#define METHOD CLASS_NAME"::Close()"
void HLSDecoder::Close(JNIEnv* env)
{
	LOGINFO(METHOD, "Entered");
	if (mWindow)
	{
		ANativeWindow_release(mWindow);
		mWindow = NULL;
	}
	if (mSurface)
	{
		(*env).DeleteGlobalRef(mSurface);
		mSurface = NULL;
	}
	if (mAudioPlayer)
	{
		// do something!
	}
	if (mTimeSource)
	{
		// do something!
	}

}

#define METHOD CLASS_NAME"::SetSurface()"
void HLSDecoder::SetSurface(JNIEnv* env, jobject surface)
{
	LOGINFO(METHOD, "Entered");

	mSurface = (jobject)env->NewGlobalRef(surface);

	ANativeWindow* window = ANativeWindow_fromSurface(env, mSurface);

	LOGINFO(METHOD, "Java_com_kaltura_hlsplayersdk_PlayerView_SetSurface() - window = %0x", window);
	LOGINFO(METHOD, "window->flags = %0x", window->flags);
	LOGINFO(METHOD, "window->swapInterfal Min: %d Max: %d", window->minSwapInterval, window->maxSwapInterval);
	LOGINFO(METHOD, "window->dpi  x:%f y:%f", window->xdpi, window->ydpi);



	if (window)
	{
		SetNativeWindow(window);
	}
}

#define METHOD CLASS_NAME"::SetNativeWindow()"
void HLSDecoder::SetNativeWindow(ANativeWindow* window)
{
	LOGINFO(METHOD, "window = %0x", window);
	if (mWindow)
	{
		LOGINFO(METHOD, "::mWindow is already set to %0x", window);
		// Umm - resetting?
		ANativeWindow_release(mWindow);
	}
	mWindow = window;
}




#define METHOD CLASS_NAME"::Play()"
bool HLSDecoder::Play()
{
	LOGINFO(METHOD, "Entered");
	if (!mWindow) { LOGINFO(METHOD, "mWindow is NULL"); return false; }
	sp<ANativeWindow> nativeWindow = NULL;
	nativeWindow = mWindow;
	LOGINFO(METHOD, "%d", __LINE__);

	return PlayNextSegment();

//	mVideoDecoder = OMXCodec::Create(mClient.interface(), mVideoSource->getFormat(), false, mVideoSource, NULL, 0, NULL /*nativeWindow*/);
//	LOGINFO(METHOD, "%d", __LINE__);
//	status_t res = mVideoDecoder->start();
//	LOGINFO(METHOD, "::Play() - res = %s", strerror(-res));
//	if (InitAudioDecoder() && CreateAudioPlayer())
//	{
//		LOGINFO(METHOD, "Audio Player Created");
//		res = mAudioPlayer->start(true);
//		LOGINFO(METHOD, "mAudioPlayer->start() return %s", strerror(-res));
//	}
//	return res == OK;
}

#define METHOD CLASS_NAME"::PlayNextSegment()"
bool HLSDecoder::PlayNextSegment()
{
	LOGINFO(METHOD, "Entered");
	if (mCurSegment)
	{
		delete mCurSegment;
		mCurSegment = NULL;
	}
	if (!mWindow) { LOGINFO(METHOD, "mWindow is NULL"); return false; }
	if (mSegments.size() == 0)
	{
		LOGINFO(METHOD, "We're out of segments!");
		return false;
	}
	mCurSegment = mSegments.front();
	mSegments.pop_front();

	//Create the video and audio decoders

	LOGINFO(METHOD, "mCurSegment = %0x", mCurSegment);
	if (mVideoDecoder != NULL)
	{
		status_t stopRes =  mVideoDecoder->stop();
		LOGINFO(METHOD, "mVideoDecoder->stop returned: %s", strerror(-stopRes));
	}
	mVideoDecoder = OMXCodec::Create(mClient.interface(), mCurSegment->GetVideoTrack()->getFormat(), false, mCurSegment->GetVideoTrack(), NULL, 0, NULL /*nativeWindow*/);
	LOGINFO(METHOD, "OMXCodec::Create() returned %0x", mVideoDecoder.get());
	status_t res = mVideoDecoder->start();

	LOGINFO(METHOD, "mVideoDecoder->start() res = %s", strerror(-res));
	if (res == OK)
	{
		// Create the audio player (do we need a new one?) I don't have a clue
		// Can we just send it a new source?
		if (!InitAudioDecoder())
		{
			LOGINFO(METHOD, "Failed to init the Audio Decoder");
			return false;
		}
		if (!mAudioPlayer)
		{
			if (CreateAudioPlayer())
			{
				LOGINFO(METHOD, "Starting the Audio Player");
				mAudioPlayer->start(true);
			}
		}
		else
		{
			mAudioPlayer->pause(true);
			LOGINFO(METHOD, "mAudioSource == %0x", mAudioSource.get());
			mAudioPlayer->setSource(mAudioSource);
			mTimeSource = mAudioPlayer;
		}
		SetStatus(PLAYING);
		return true;
	}
	return false;
}

#define METHOD CLASS_NAME"::InitAudioDecoder()"
bool HLSDecoder::InitAudioDecoder()
{
	LOGINFO(METHOD, "Entered");
	if (mCurSegment == NULL || mCurSegment->GetAudioTrack() == NULL) return false;

	sp<MetaData> meta = mCurSegment->GetAudioTrack()->getFormat();

	LOGINFO(METHOD, "mAudioTrack MetaData");
	RUNDEBUG(meta->dumpToLog());

	const char* mime = NULL;

	if (!meta->findCString(kKeyMIMEType, &mime))
	{
		LOGINFO(METHOD, "Couldn't find mime type for audio track");
		return false;
	}

	LOGINFO(METHOD, "Audio Track Mime Type = '%s' Line: %d", mime, __LINE__);

	audio_stream_type_t streamType = AUDIO_STREAM_MUSIC;

	LOGINFO(METHOD, "mAudioSink == %0x", mAudioSink.get());
	if (mAudioSink != NULL)
	{
		streamType = mAudioSink->getAudioStreamType();
	}

	mOffloadAudio = canOffloadStream(meta, (mCurSegment->GetVideoTrack() != NULL), false /*streaming http */, streamType);
	LOGINFO(METHOD, "mOffloadAudio == %s", mOffloadAudio ? "true" : "false");


	if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_RAW))
	{
		LOGINFO(METHOD, "bybass OMX (raw) Line: %d", __LINE__);
		mAudioSource = mCurSegment->GetAudioTrack();
	}
	else
	{
		mOmxSource = OMXCodec::Create(mClient.interface(), mCurSegment->GetAudioTrack()->getFormat(), false, mCurSegment->GetAudioTrack());
		if (mOffloadAudio)
		{
			LOGINFO(METHOD, "Bypass OMX (offload) Line: %d", __LINE__);
			mAudioSource = mCurSegment->GetAudioTrack(); //mAudioTrack;
		}
		else
		{
			LOGINFO(METHOD, "Not Bypassing OMX Line: %d", __LINE__);
			mAudioSource = mOmxSource;
		}
	}

	if (mAudioSource != NULL)
	{
		int64_t durationUs = 0;
		if (mCurSegment->GetAudioTrack()->getFormat()->findInt64(kKeyDuration, &durationUs))
		{
			if (mDurationUs < 0 || durationUs > mDurationUs)
			{
				mDurationUs = durationUs;
			}
		}

		status_t err = mAudioSource->start();

		if (err != OK)
		{
			LOGINFO(METHOD, "Audio couldn't start - err: %d Line: %d", err, __LINE__);
			mAudioSource.clear();
			mOmxSource.clear();
			return false;
		}
	}

	return true;
}

#define METHOD CLASS_NAME"::CreateAudioPlayer()"
bool HLSDecoder::CreateAudioPlayer()
{
	LOGINFO(METHOD, "Entered");
	uint32_t flags = 0;
	int64_t chackedDurationUs = 0;
	bool eos;

	if (mOffloadAudio)
	{
		flags |= AudioPlayer::USE_OFFLOAD;
	}
	else if (mCurSegment == NULL || mCurSegment->GetVideoTrack() == NULL)
	{
		LOGINFO(METHOD, "mVideoSource == NULL");
		// bah... this probably doesn't apply to us
		return false;
	}

	mAudioPlayer = new AudioPlayer(mAudioSink, flags, NULL);
	LOGINFO(METHOD, "mAudioSource == %0x", mAudioSource.get());
	mAudioPlayer->setSource(mAudioSource);
	mTimeSource = mAudioPlayer;

	return true;
}



#define METHOD CLASS_NAME"::FeedSegment()"
status_t HLSDecoder::FeedSegment(const char* path)
{
	LOGINFO(METHOD, "path = '%s'", path);
	sp<DataSource> dataSource = new FileSource(path);

	status_t err = dataSource->initCheck();

	if (err != OK) return err;

	HLSSegment* s = new HLSSegment();
	if (s)
	{
		if (s->SetDataSource(dataSource))
		{
			mSegments.push_back(s);

			return OK;
		}
		else
		{
			delete s;
			s = NULL;
		}
		return UNKNOWN_ERROR;
	}
	return NO_MEMORY;




//	mFileSource = dataSource;
//
//	mExtractor = MediaExtractor::Create(mFileSource);
//
//	if (mExtractor == NULL) { return UNKNOWN_ERROR; }
//
//	if (mExtractor->getDrmFlag())
//	{
//		checkDrmStatus(mFileSource);
//	}
//
//	int64_t totalBitrate = 0;
//
//	for (size_t i = 0; i < mExtractor->countTracks(); ++i)
//	{
//
//		sp<MetaData> meta = mExtractor->getTrackMetaData(i);
//
//		int32_t bitrate = 0;
//		if (!meta->findInt32(kKeyBitRate, &bitrate))
//		{
//			const char* mime = "[unknown]";
//			meta->findCString(kKeyMIMEType, &mime);
//
//			LOGINFO(METHOD, "Track #%d of type '%s' does not publish bitrate", i, mime );
//			continue;
//		}
//		LOGINFO(METHOD, "bitrate for track %d = %d bits/sec", i , bitrate);
//		totalBitrate += bitrate;
//	}
//
//	mBitrate = totalBitrate;
//
//	LOGINFO(METHOD, "mBitrate = %lld bits/sec", mBitrate);
//
//	bool haveAudio = false;
//	bool haveVideo = false;
//
//	for (size_t i = 0; i < mExtractor->countTracks(); ++i)
//	{
//		sp<MetaData> meta = mExtractor->getTrackMetaData(i);
//		meta->dumpToLog();
//
//		const char* cmime;
//		if (meta->findCString(kKeyMIMEType, &cmime))
//		{
//			String8 mime = String8(cmime);
//
//			if (!haveVideo && !strncasecmp(mime.string(), "video/", 6))
//			{
//				SetVideoSource(mExtractor->getTrack(i));
//				haveVideo = true;
//
//				// Set the presentation/display size
//				int32_t width, height;
//				bool res = meta->findInt32(kKeyWidth, &width);
//				if (res)
//				{
//					res = meta->findInt32(kKeyHeight, &height);
//				}
//				if (res)
//				{
//					mWidth = width;
//					mHeight = height;
//					LOGINFO(METHOD, "Video Track Width = %d, Height = %d, %d", width, height, __LINE__);
//				}
//			}
//			else if (!haveAudio && !strncasecmp(mime.string(), "audio/", 6))
//			{
//				SetAudioSource(mExtractor->getTrack(i));
//				haveAudio = true;
//
//				mActiveAudioTrackIndex = i;
//
//			}
//			else if (!strcasecmp(mime.string(), MEDIA_MIMETYPE_TEXT_3GPP))
//			{
//				//addTextSource_l(i, mExtractor->getTrack(i));
//			}
//		}
//	}
//
//	if (!haveAudio && !haveVideo)
//	{
//		return UNKNOWN_ERROR;
//	}
//
//
//
//	mExtractorFlags = mExtractor->flags();

//	return OK;
}

#define METHOD CLASS_NAME"::Update()"
int HLSDecoder::Update()
{
	LOGINFO(METHOD, "Entered");

	if (mStatus != PLAYING) return -1;
	if (mVideoBuffer != NULL)
	{
		mVideoBuffer->release();
		mVideoBuffer = NULL;
	}
	MediaSource::ReadOptions options;
	bool rval = -1;
	for (;;)
	{
		LOGINFO(METHOD, "mVideoBuffer = %0x", mVideoBuffer);
		RUNDEBUG(mVideoDecoder->getFormat()->dumpToLog());
		status_t err = mVideoDecoder->read(&mVideoBuffer, &options);
		if (err != OK)
		{
			LOGINFO(METHOD, "err=%s,%0x  Line: %d", strerror(-err), -err, __LINE__);
			switch (err)
			{
			case INFO_FORMAT_CHANGED:
			case INFO_DISCONTINUITY:
			case INFO_OUTPUT_BUFFERS_CHANGED:
				// If it doesn't have a valid buffer, maybe it's informational?
				if (mVideoBuffer == NULL) return 0;
				break;
			case ERROR_END_OF_STREAM:
				PlayNextSegment();
				break;
			default:
				SetStatus(STOPPED);
				// deal with any errors
				// in the sample code, they're sending the video event, anyway
				return -1;
			}
		}

		if (mVideoBuffer->range_length() != 0)
		{
			// We appear to have a valid buffer?!
			if (RenderBuffer(mVideoBuffer))
			{
				++mRenderedFrameCount;
				rval = mRenderedFrameCount;
				LOGINFO(METHOD, "mRenderedFrameCount = %d", mRenderedFrameCount);
			}
			else
			{
				LOGINFO(METHOD, "Render Buffer returned false: STOPPING");
				SetStatus(STOPPED);
				rval=-1;
			}
			break;
		}

		LOGINFO(METHOD, "Found empty buffer%d", __LINE__);
		// Some decoders return spurious empty buffers we want to ignore
		mVideoBuffer->release();
		mVideoBuffer = NULL;

	}


//    int64_t nextTimeUs;
//    mVideoBuffer->meta_data()->findInt64(kKeyTime, &nextTimeUs);
//    int64_t delayUs = nextTimeUs - ts->getRealTimeUs() + mTimeSourceDeltaUs;
//    postVideoEvent_l(delayUs > 10000 ? 10000 : delayUs < 0 ? 0 : delayUs);
    return rval; // keep going!

}

#define METHOD CLASS_NAME"::RenderBuffer()"
bool HLSDecoder::RenderBuffer(MediaBuffer* buffer)
{
	LOGINFO(METHOD, "Entered");
	//LOGINFO(METHOD, "Rendering Buffer size=%d", buffer->size());
	if (!mWindow) { LOGINFO(METHOD, "mWindow is NULL"); return false; }
	if (!buffer) { LOGINFO(METHOD, "the MediaBuffer is NULL"); return false; }
	//if (!buffer->graphicBuffer().get()){ LOGINFO(CLASS_NAME, "the MediaBuffer->graphicBuffer is NULL"); return false; }
	RUNDEBUG(buffer->meta_data()->dumpToLog());
	int colf = 0;
	bool res = mVideoDecoder->getFormat()->findInt32(kKeyColorFormat, &colf);
	LOGINFO(METHOD, "Found Frame Color Format: %s", res ? "true" : "false" );
	RUNDEBUG(mVideoDecoder->getFormat()->dumpToLog());
	ColorConverter cc((OMX_COLOR_FORMATTYPE)colf, OMX_COLOR_Format16bitRGB565); // Should be getting these from the formats, probably
	LOGINFO(METHOD, "ColorConversion from %d is valid: %s", colf, cc.isValid() ? "true" : "false" );
    int64_t timeUs;
    if (buffer->meta_data()->findInt64(kKeyTime, &timeUs))
    {
    	LOGINFO(METHOD, "%d", __LINE__);
		native_window_set_buffers_timestamp(mWindow, timeUs * 1000);
		LOGINFO(METHOD, "%d", __LINE__);
		//status_t err = mWindow->queueBuffer(mWindow, buffer->graphicBuffer().get(), -1);

		ANativeWindow_Buffer windowBuffer;
		if (ANativeWindow_lock(mWindow, &windowBuffer, NULL) == 0)
		{
			LOGINFO(METHOD, "buffer locked (%d x %d stride=%d, format=%d)", windowBuffer.width, windowBuffer.height, windowBuffer.stride, windowBuffer.format);

			cc.convert(buffer->data(), 480, 256, 0, 0, 479, 255, windowBuffer.bits, 1280, 628, 0,0,479,255);
			//unsigned short *pixels = (unsigned short *)windowBuffer.bits;
			//for(int i=0; i<windowBuffer.width * windowBuffer.height; i++)
			//	pixels[i] = 0xF0F0 ^ (int)buffer;
			void *gbBits = NULL;

			//buffer->graphicBuffer().get()->lock(0, &gbBits);

			//memcpy(windowBuffer.bits, buffer->data(), buffer->size());
			//buffer->graphicBuffer().get()->unlock();

			ANativeWindow_unlockAndPost(mWindow);
		}


		LOGINFO(METHOD, "%d", __LINE__);
		//if (err != 0) {
		//	ALOGE("queueBuffer failed with error %s (%d)", strerror(-err),
		//			-err);
		//	return false;
		//}
		LOGINFO(METHOD, "%d", __LINE__);

		sp<MetaData> metaData = buffer->meta_data();
		LOGINFO(METHOD, "%d", __LINE__);
		metaData->setInt32(kKeyRendered, 1);
		LOGINFO(METHOD, "%d", __LINE__);
		return true;
    }
    return false;

}

#define METHOD CLASS_NAME"::checkDrmStatus()"
void HLSDecoder::checkDrmStatus(const sp<DataSource>& dataSource)
{

}

//void HLSDecoder::Update()
//{
//
////	android::sp<android::MediaSource> mVideoSource = new AVFormatSource(filepath);
//
//	for (;;)
//	{
//		android::MediaBuffer* mVideoBuffer;
//		android::MediaSource::ReadOptions options;
//		//status_t err = mVideoDecoder->Read(&mVideoBuffer, &options);
//	}
//
//}

//#define METHOD CLASS_NAME"::SetVideoSource()"
//void HLSDecoder::SetVideoSource(sp<MediaSource> source)
//{
//	mVideoSource = source;
////	mVideoSource->start(NULL);
////	MediaBuffer* buffer = NULL;
////	mVideoSource->read(&buffer);
////	LOGINFO(METHOD, "::SetVideoSource() buffer->Size() = %d", buffer->size());
////	buffer->release();
//
//}
//
//#define METHOD CLASS_NAME"::SetAudioSource()"
//void HLSDecoder::SetAudioSource(sp<MediaSource> source)
//{
//	mAudioTrack = source;
//}

#define METHOD CLASS_NAME"::SetStatus()"
void HLSDecoder::SetStatus(int status)
{
	mStatus = status;
	LOGINFO(METHOD, "status = %d", status);
}



