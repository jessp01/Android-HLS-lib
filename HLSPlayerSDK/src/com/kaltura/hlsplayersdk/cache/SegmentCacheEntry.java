package com.kaltura.hlsplayersdk.cache;

import java.io.IOException;

import android.util.Log;

import com.squareup.okhttp.Callback;
import com.squareup.okhttp.Request;
import com.squareup.okhttp.Response;

class SegmentCacheEntry implements Callback {
	public String uri;
	public byte[] data;
	public boolean running;
	public long lastTouchedMillis;

	// If >= 0, ID of a crypto context on the native side.
	protected int cryptoHandle = -1;

	// All bytes < decryptHighWaterMark are descrypted; all >=  are still 
	// encrypted. This allows us to avoid duplicating every segment.
	protected long decryptHighWaterMark = 0;
	
	public void setCryptoHandle(int handle)
	{
		cryptoHandle = handle;
	}

	public void ensureDecryptedTo(int offset)
	{
		long delta = offset - decryptHighWaterMark;
		decryptHighWaterMark = HLSSegmentCache.decrypt(data, decryptHighWaterMark, delta);
	}

	@Override
	public void onFailure(Request arg0, IOException arg1) {
		Log.e("HLS Cache", "Failed to download '" + uri + "'! " + arg1.toString());
	}
	
	@Override
	public void onResponse(Response response) throws IOException {
		if (!response.isSuccessful()) throw new IOException("Unexpected code " + response);
		
		Log.i("HLS Cache", "Got " + uri);
		HLSSegmentCache.store(uri, response.body().bytes());
	}
}
