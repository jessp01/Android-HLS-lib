/*
 * HLSSegment.cpp
 *
 *  Created on: Apr 29, 2014
 *      Author: Mark
 */
#include <android/native_window.h>
#include <android/window.h>
#include <../android-source/frameworks/av/include/media/stagefright/MediaDefs.h>


#include "constants.h"
#include "debug.h"
#include "HLSSegment.h"

using namespace android;


#define CLASS_NAME APP_NAME"::HLSSegment"


#define METHOD CLASS_NAME"::HLSSegment()"
HLSSegment::HLSSegment() : mWidth(0), mHeight(0), mBitrate(0), mExtractorFlags(0), mActiveAudioTrackIndex(0)
{

}

#define METHOD CLASS_NAME"::~HLSSegment()"
HLSSegment::~HLSSegment()
{

}

#define METHOD CLASS_NAME"::SetDataSource()"
bool HLSSegment::SetDataSource(android::sp<android::DataSource> dataSource)
{
	status_t err = dataSource->initCheck();
	if (err != OK)
	{
		LOGERROR(METHOD, "DataSource is invalid: %s", strerror(-err));
		return false;
	}

	mFileSource = dataSource;
	mExtractor = MediaExtractor::Create(mFileSource);
	if (mExtractor == NULL)
	{
		LOGERROR(METHOD, "Could not create MediaExtractor from DataSource %0x", dataSource.get());
		return false;
	}

	if (mExtractor->getDrmFlag())
	{
		LOGERROR(METHOD, "This datasource has DRM - not implemented!!!");
		return false;
	}

	int64_t totalBitRate = 0;
	for (size_t i = 0; i < mExtractor->countTracks(); ++i)
	{

		sp<MetaData> meta = mExtractor->getTrackMetaData(i);

		int32_t bitrate = 0;
		if (!meta->findInt32(kKeyBitRate, &bitrate))
		{
			const char* mime = "[unknown]";
			meta->findCString(kKeyMIMEType, &mime);

			LOGINFO(METHOD, "Track #%d of type '%s' does not publish bitrate", i, mime );
			continue;
		}
		LOGINFO(METHOD, "bitrate for track %d = %d bits/sec", i , bitrate);
		totalBitRate += bitrate;
	}

	mBitrate = totalBitRate;



	LOGINFO(METHOD, "mBitrate = %lld bits/sec", mBitrate);

	bool haveAudio = false;
	bool haveVideo = false;

	for (size_t i = 0; i < mExtractor->countTracks(); ++i)
	{
		sp<MetaData> meta = mExtractor->getTrackMetaData(i);
		meta->dumpToLog();

		const char* cmime;
		if (meta->findCString(kKeyMIMEType, &cmime))
		{
			String8 mime = String8(cmime);

			if (!haveVideo && !strncasecmp(mime.string(), "video/", 6))
			{
				SetVideoTrack(mExtractor->getTrack(i));
				haveVideo = true;

				// Set the presentation/display size
				int32_t width, height;
				bool res = meta->findInt32(kKeyWidth, &width);
				if (res)
				{
					res = meta->findInt32(kKeyHeight, &height);
				}
				if (res)
				{
					mWidth = width;
					mHeight = height;
					LOGINFO(METHOD, "Video Track Width = %d, Height = %d, %d", width, height, __LINE__);
				}
			}
			else if (!haveAudio && !strncasecmp(mime.string(), "audio/", 6))
			{
				SetAudioTrack(mExtractor->getTrack(i));
				haveAudio = true;

				mActiveAudioTrackIndex = i;

			}
			else if (!strcasecmp(mime.string(), MEDIA_MIMETYPE_TEXT_3GPP))
			{
				//addTextSource_l(i, mExtractor->getTrack(i));
			}
		}
	}

	if (!haveAudio && !haveVideo)
	{
		return UNKNOWN_ERROR;
	}



	mExtractorFlags = mExtractor->flags();

	return true;
}

//#define METHOD CLASS_NAME"::InitAudioDecoder()"
//bool HLSSegment::InitAudioDecoder()
//{
//	LOGINFO(METHOD, "Entered");
//	if (mAudioTrack == NULL) return false;
//
//	sp<MetaData> meta = mAudioTrack->getFormat();
//
//	LOGINFO(METHOD, "mAudioTrack MetaData");
//	RUNDEBUG(meta->dumpToLog());
//
//	const char* mime = NULL;
//
//	if (!meta->findCString(kKeyMIMEType, &mime))
//	{
//		LOGINFO(METHOD, "Couldn't find mime type for audio track");
//		return false;
//	}
//
//	LOGINFO(METHOD, "Audio Track Mime Type = '%s' Line: %d", mime, __LINE__);
//
//	audio_stream_type_t streamType = AUDIO_STREAM_MUSIC;
//
//	LOGINFO(METHOD, "mAudioSink == %0x", mAudioSink.get());
//	if (mAudioSink != NULL)
//	{
//		streamType = mAudioSink->getAudioStreamType();
//	}
//
//	mOffloadAudio = canOffloadStream(meta, (mVideoSource != NULL), false /*streaming http */, streamType);
//	LOGINFO(METHOD, "mOffloadAudio == %s", mOffloadAudio ? "true" : "false");
//
//
//	if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_RAW))
//	{
//		LOGINFO(METHOD, "bybass OMX (raw) Line: %d", __LINE__);
//		mAudioSource = mAudioTrack;
//	}
//	else
//	{
//		mOmxSource = OMXCodec::Create(mClient.interface(), mAudioTrack->getFormat(), false, mAudioTrack);
//		if (mOffloadAudio)
//		{
//			LOGINFO(METHOD, "Bypass OMX (offload) Line: %d", __LINE__);
//			mAudioSource = mAudioTrack;
//		}
//		else
//		{
//			LOGINFO(METHOD, "Not Bypassing OMX Line: %d", __LINE__);
//			mAudioSource = mOmxSource;
//		}
//	}
//
//	if (mAudioSource != NULL)
//	{
//		int64_t durationUs = 0;
//		if (mAudioTrack->getFormat()->findInt64(kKeyDuration, &durationUs))
//		{
//			if (mDurationUs < 0 || durationUs > mDurationUs)
//			{
//				mDurationUs = durationUs;
//			}
//		}
//
//		status_t err = mAudioSource->start();
//
//		if (err != OK)
//		{
//			LOGINFO(METHOD, "Audio couldn't start - err: %d Line: %d", err, __LINE__);
//			mAudioSource.clear();
//			mOmxSource.clear();
//			return false;
//		}
//	}
//
//	return true;
//}

#define METHOD CLASS_NAME"::SetAudioTrack()"
void HLSSegment::SetAudioTrack(sp<MediaSource> source)
{
	mAudioTrack = source;
}

#define METHOD CLASS_NAME"::SetVideoTrack()"
void HLSSegment::SetVideoTrack(sp<MediaSource> source)
{
	mVideoTrack = source;
}

#define METHOD CLASS_NAME"::GetAudioTrack()"
sp<MediaSource> HLSSegment::GetAudioTrack()
{
	return mAudioTrack;
}

#define METHOD CLASS_NAME"::GetVideoTrack()"
sp<MediaSource> HLSSegment::GetVideoTrack()
{
	return mVideoTrack;
}

#define METHOD CLASS_NAME"::GetWidth()"
int32_t HLSSegment::GetWidth()
{
	return mWidth;
}

#define METHOD CLASS_NAME"::GetWidth()"
int32_t HLSSegment::GetHeight()
{
	return mHeight;
}

#define METHOD CLASS_NAME"::GetBitrate()"
int64_t HLSSegment::GetBitrate()
{
	return mBitrate;
}
