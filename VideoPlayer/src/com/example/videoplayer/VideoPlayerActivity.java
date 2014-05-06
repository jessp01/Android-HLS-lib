package com.example.videoplayer;

import android.support.v7.app.ActionBarActivity;
import android.support.v7.app.ActionBar;
import android.support.v4.app.Fragment;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.VideoView;
import android.os.Build;

import com.kaltura.hlsplayersdk.PlayerViewController;

public class VideoPlayerActivity extends ActionBarActivity {

	PlayerViewController playerView = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_video_player);
        
      
        Uri uri=Uri.parse("http://www.ebookfrenzy.com/android_book/movie.mp4");
        
        
        try
        {
        	
        	playerView = (PlayerViewController)findViewById(R.id.custom_player);
        	playerView.addComponents("/storage/emulated/0/Movies/segment1_0_av.ts", this);

        }
        catch (Exception e)
        {
        	Log.e("KalturaTestApp", e.getMessage());
        }
        

    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.video_player, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();
        if (id == R.id.action_settings) {
        	playerView.play();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }



}
