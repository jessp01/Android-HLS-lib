package com.kaltura.hlsplayersdk;


import android.media.MediaPlayer;

import com.kaltura.hlsplayersdk.events.*;

public interface VideoPlayerInterface {
    public String getVideoUrl();
    public void setVideoUrl(String url);

    public int getDuration();

    public boolean getIsPlaying();

    public void play();

    public void pause();

    public void stop();

    public void seek(int msec);
    
    public void close();

    // events
    public void registerPlayerStateChange(OnPlayerStateChangeListener listener);

    public void registerReadyToPlay(MediaPlayer.OnPreparedListener listener);

    public void registerError(MediaPlayer.OnErrorListener listener);

    public void registerPlayheadUpdate(OnPlayheadUpdateListener listener);

    public void registerProgressUpdate(OnProgressListener listener);
}
