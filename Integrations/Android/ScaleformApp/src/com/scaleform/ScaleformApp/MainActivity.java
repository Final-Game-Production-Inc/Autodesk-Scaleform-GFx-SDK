package com.scaleform.${APP_NAME};

import java.util.Date;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Intent;
import android.content.Context;
import android.content.res.Configuration;
import android.net.Uri;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.location.LocationProvider;
import android.os.Bundle;
import android.os.Looper;
import android.util.Log;
import android.util.Pair;
import android.util.SparseArray;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;

/// begin GFX_SOUND_FMOD
import org.fmod.FMODAudioDevice;
/// end GFX_SOUND_FMOD

@SuppressLint("ViewConstructor")
public class MainActivity extends Activity implements SensorEventListener
{
    GLView View;

    private static native void NativeAppInit();
    private static native void NativeOnOpenFile(String path);
    private native void NativeOnCreate();
    private static native void NativeOnPause();
    private static native void NativeOnResume();
    private static native void NativeOnWindowFocusChanged(boolean hasFocus); 
    private static native void NativeOnOrientation(int orient);
    private static native void NativeOnTouch(int pointerId, int event, float x, float y);
    private static native void NativeOnTouchMouse(int event, float x, float y);
    private static native void NativeOnGesture(int event, int mode, float x, float y, float panx, float pany, float scale);
    private static native void NativeOnGeolocationUpdate(int idGeo, double latitude, double longitude, double altitude, double hAccuarcy, double vAccuarcy, double speed, double heading, double timestamp);
    private static native void NativeOnAccelerometerUpdate(int idAcc, double timestamp, double accelerationX, double accelerationY, double accelerationZ);
    private static native void NativeOnStatus(String  code, String level);

    private static native void NativeOnKey(boolean down, int key);
    private static native void NativeOnChar(int code);
    
    private native void NativeCacheObject();
    private native void NativeClearObject();
    
    private native void NativeOnLibraryInit(String extensionName, String extensionInitializer);

    public static final int GEST_EVENT_END = 0; 
    public static final int GEST_EVENT_BEGIN = 1;
    public static final int GEST_EVENT_GESTURE = 2;
    public static final int GEST_EVENT_SIMPLE = 3;
    
    public static final int GEST_EVENT_NONE = 0;
    public static final int GEST_EVENT_PAN = 1;
    public static final int GEST_EVENT_ZOOM = 2;
    public static final int GEST_EVENT_SWIPE = 3;
    
    public static final int ACTION_POINTER_2_MOVE = 263;
    
    public static final float VEL_DELTA = 2.0f;
    
    protected float x1 = 0.0f, 
                    x2 = 0.0f, 
                    y1 = 0.0f, 
                    y2 = 0.0f, 
                    x1_pre = 0.0f,
                    y1_pre = 0.0f,
                    x_scale = 1.0f,
                    y_scale = 1.0f,
                    dist_curr = -1.0f, 
                    dist_pre = -1.0f,
                    dist_delta = 0.0f,
                    velocity = 0;
    
    private long time_delta = 0,
                 time_curr = 0,
                 time_pre = 0;
        
    private String mDebugMessage = "";
    
    private static long beginAppTimestamp = new Date().getTime();

    // Used to remember the state of OnWindowFocusChanged calls. This may be required to start sounds again
    // onResume, because there may not be a subsequent OnWindowFocusChanged call, if there is no lock screen. 
    private boolean WindowHasFocus = false;
    
/// begin GFX_SOUND_FMOD
    private FMODAudioDevice Sound = new FMODAudioDevice();
/// end GFX_SOUND_FMOD

    static
    {
/// begin GFX_SOUND_FMOD
        System.loadLibrary("fmodex");
/// end GFX_SOUND_FMOD
        System.loadLibrary("FxPlayerMobile_Release_NoRTTI");
        // This block can be used to impart a delay to allow a native debugger to attach at the very beginning.
        /*int count = 0;
        while (count < 500000000)
        {
            count++;
            if ((count % 50000000) == 1)
            {
                Log.d("SparkPlayer", "Count = " + count/50000000);
            }
        }*/
        NativeAppInit();
    }
    
    @Override
    public void onStart()
    {
        super.onStart();
        NativeCacheObject();
        InitExtensions();
    }
    
    @Override
    public void onStop()
    { 
        NativeClearObject();
        super.onStop();
    }

    @Override
    protected void onPause()
    {
/// begin GFX_SOUND_FMOD
        Sound.stop();
/// end GFX_SOUND_FMOD
        super.onPause();
        NativeOnPause();
        View.onPause();
    }
    @Override
    protected void onResume()
    {
/// begin GFX_SOUND_FMOD
      // If we are getting an onResume, and onWindowFocusChanged(FALSE) has not been called, start sounds
      // again, as this means that this window still has focus, and thus should become fully interactive.
      if (WindowHasFocus)
        Sound.start();
/// end GFX_SOUND_FMOD
        super.onResume();
        NativeOnResume();
        View.onResume();
    }
    
    private void InitExtensions() 
    {   
        /// begin ANDROID_NATIVE_EXTENSION
        /// end ANDROID_NATIVE_EXTENSION
    }
    
  @Override
  public void onWindowFocusChanged(boolean hasFocus) 
  {
    super.onWindowFocusChanged(hasFocus);
    
    // When the window loses gains/loses focus, start/stop the sound. 
/// begin GFX_SOUND_FMOD
    if (hasFocus)
      Sound.start();
    else
      Sound.stop();
/// end GFX_SOUND_FMOD
    
    // Record the focus state.
    WindowHasFocus = hasFocus;
    
    NativeOnWindowFocusChanged(hasFocus);
  }
  
    public void OpenVirtualKeyboard(boolean multiline)
    {
        if (BuildConfig.DEBUG){ Log.d("GFxPlayer", "OpenVirtualKeyboard called"); }
        
        InputMethodManager imm = (InputMethodManager) this.getSystemService(Context.INPUT_METHOD_SERVICE);
        if (imm == null) 
        {
            if (BuildConfig.DEBUG){ Log.d("GFxPlayer", "OpenVirtualKeyboard Input Method Service not found!"); }
            return;
        }
        Configuration config = this.getResources().getConfiguration();
        if (config.hardKeyboardHidden == Configuration.HARDKEYBOARDHIDDEN_YES)
        {
            if (BuildConfig.DEBUG){ Log.d("GFxPlayer", "OpenVirtualKeyboard Showing soft input! multiline = " + multiline); }
            View.setMultilineTextfieldMode(multiline);
            imm.restartInput(View);
            imm.showSoftInput(View, 0);
        }
        else
        {
            if (BuildConfig.DEBUG){ Log.d("GFxPlayer", "HARDKEYBOARDHIDDEN_YES is false!"); }
        }
    }
    public void CloseVirtualKeyboard()
    {
        if (BuildConfig.DEBUG){ Log.d("GFxPlayer", "CloseVirtualKeyboard called"); }
        
        InputMethodManager imm = (InputMethodManager) this.getSystemService(Context.INPUT_METHOD_SERVICE);
        if (imm == null) 
        {
            if (BuildConfig.DEBUG){ Log.d("GFxPlayer", "CloseVirtualKeyboard Input Method Service not found!"); }
            return;
        }
        Configuration config = this.getResources().getConfiguration();
        if (config.hardKeyboardHidden == Configuration.HARDKEYBOARDHIDDEN_YES)
        {
            if (BuildConfig.DEBUG){ Log.d("GFxPlayer", "CloseVirtualKeyboard Hiding soft input!"); }
            imm.hideSoftInputFromWindow(View.getWindowToken(), 0);
        }
        else
        {
            if (BuildConfig.DEBUG){ Log.d("GFxPlayer", "HARDKEYBOARDHIDDEN_YES is false!"); }
        }
    }
    
    public void OpenURL(String url)
    {
        if (!url.startsWith("https://") && !url.startsWith("http://"))
            url = "http://" + url;

        Uri uriUrl = Uri.parse(url);
        
        Intent intent = new Intent(Intent.ACTION_VIEW, uriUrl);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(intent);
    }

    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        NativeOnCreate();

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);

        onNewIntent(getIntent());

        View = new GLView(getApplication(), this);
        setContentView(View);
    }

    protected void onNewIntent(Intent in)
    {
        if (Intent.ACTION_VIEW.equals(in.getAction()) && getIntent().getData().getScheme().equals("file"))
        {
            String path = in.getData().getPath();
            String[] s = path.split("\\.");

            if (s.length >= 2 && s[s.length-1].equals("swf"))
            {
                NativeOnOpenFile(path);
            }
        }
    }
    
    public void onConfigurationChanged(Configuration config) {
        super.onConfigurationChanged(config);

        DeviceRotation = getWindowManager().getDefaultDisplay().getRotation();
        NativeOnOrientation(DeviceRotation);
        if (BuildConfig.DEBUG){ Log.d("GFxPlayer", "Orientation = " + DeviceRotation); }
    }

    public boolean onKeyDown (int key, KeyEvent event)
    {
      switch (key)
      {
      case KeyEvent.KEYCODE_MENU:
        NativeOnKey (true, 19);
        return true;
      case KeyEvent.KEYCODE_VOLUME_UP:
      case KeyEvent.KEYCODE_VOLUME_DOWN:
      case KeyEvent.KEYCODE_BACK:
        // We let the system handle volume up, down and back
        return super.onKeyDown (key, event);
      }
      //return super.onKeyDown(key, event);
      NativeOnKey (true, key);
      return true;
    }

    public boolean onKeyUp(int key, KeyEvent event)
    {
        switch (key)
        {
        case KeyEvent.KEYCODE_MENU:
            NativeOnKey(false, 19);
            return true;
        }
        //return super.onKeyUp(key, event);
        NativeOnKey(false, key);
        return true;
    }

    public void onChar(int code)
    {
        NativeOnChar(code);
    }

    public boolean onTouchEvent(MotionEvent event)
    {
        // If the renderer hasn't rendered anything yet, 'swallow' all of the touch events.
        if(!View.RendererInitComplete())
            return true;
        
        int p = event.getActionIndex();
        switch(event.getActionMasked()){
            case MotionEvent.ACTION_DOWN:
            case MotionEvent.ACTION_POINTER_DOWN:
                if (BuildConfig.DEBUG) { Log.d("GFxPlayer", "TOUCH DOWN--> id: " + event.getPointerId(p) + " x: " + event.getX(p) + " y: " + event.getY(p)); }
                NativeOnTouch(event.getPointerId(p), 0, event.getX(p), event.getY(p));        

                if(event.getPointerId(p)==0)
                {
                    if (BuildConfig.DEBUG) { Log.d("GFxPlayer", "MOUSE DOWN--> x: " + event.getX(p) + " y: " + event.getY(p)); }
                    NativeOnTouchMouse(0, event.getX(p), event.getY(p));
                }
            break;              
            case MotionEvent.ACTION_POINTER_UP:                 
            case MotionEvent.ACTION_UP:
                if (BuildConfig.DEBUG) { Log.d("GFxPlayer", "TOUCH UP--> id: " + event.getPointerId(p) + " x: " + event.getX(p) + " y: " + event.getY(p)); }
                NativeOnTouch(event.getPointerId(p), 1, event.getX(p), event.getY(p));    
                
                if(event.getPointerId(p)==0)
                {
                    if (BuildConfig.DEBUG) { Log.d("GFxPlayer", "MOUSE UP--> x: " + event.getX(p) + " y: " + event.getY(p)); }
                    NativeOnTouchMouse(1, event.getX(p), event.getY(p));
                }
                
                break;
            case MotionEvent.ACTION_MOVE:
                final int historySize = event.getHistorySize();
                final int pointerCount = event.getPointerCount();
                for (int h = 0; h < historySize; h++) 
                {
                     for (int p1 = 0; p1 < pointerCount; p1++) 
                     {
                        if (BuildConfig.DEBUG) { Log.d("GFxPlayer", "TOUCH MOVE--> id: " + event.getPointerId(p1) + " x: " + (int)event.getHistoricalX(p1, h) + " y: " + (int)event.getHistoricalY(p1, h)); }
                        NativeOnTouch(event.getPointerId(p1), 2, (int)event.getHistoricalX(p1, h), (int)event.getHistoricalY(p1, h));    
                     }
                }
                for (int p1 = 0; p1 < event.getPointerCount(); p1++) 
                {
                    if (BuildConfig.DEBUG) { Log.d("GFxPlayer", "TOUCH MOVE--> id: " + event.getPointerId(p1) + " x: " + event.getX(p1) + " y: " + event.getY(p1)); }
                    NativeOnTouch(event.getPointerId(p1), 2, event.getX(p1), event.getY(p1));
                }
                
                if(event.getPointerId(p)==0)
                {
                    if (BuildConfig.DEBUG) { Log.d("GFxPlayer", "MOUSE MOVE--> x: " + event.getX(p) + " y: " + event.getY(p)); }
                    NativeOnTouchMouse(2, event.getX(p), event.getY(p));
                }
            break;
        }

        return true;
    }
    
    
  //-------------------------------------------------------------------------------------------------
    
    private final static long DefaultAccelerometerInterval = 200000000;  //0.2 second = SENSOR_DELAY_NORMAL
    private static long minimumtAccelerometerInterval = DefaultAccelerometerInterval;  //0.2 second = SENSOR_DELAY_NORMAL
    private SensorManager sensorManager = null;
    private Sensor accelerometer = null;    
    private SparseArray<Pair<Long, Long>> id2IntervalTimestampMap;
    private int DeviceRotation = Surface.ROTATION_0;

    private SensorManager getSensorManager() 
    {
        if (sensorManager == null)
            sensorManager = (SensorManager) getApplicationContext().getSystemService(Context.SENSOR_SERVICE);

        return sensorManager;
    }

    public boolean registerAccelerometer(int accelerometerId) 
    {
        /* Init On demand... */
        if (id2IntervalTimestampMap == null)
            id2IntervalTimestampMap = new SparseArray<Pair<Long, Long>>();
        
        id2IntervalTimestampMap.put(accelerometerId, new Pair<Long, Long>(DefaultAccelerometerInterval, (long)0));      
        
        this.updateSmallestIntervalAndStartAccelerometer();
        return true;
    }

    private void updateSmallestIntervalAndStartAccelerometer() 
    {
        minimumtAccelerometerInterval = Integer.MAX_VALUE;
 
        for(int i = 0; i < id2IntervalTimestampMap.size(); i++)
        {
            long interval = id2IntervalTimestampMap.valueAt(i).first;
            if (interval < minimumtAccelerometerInterval)
                minimumtAccelerometerInterval = interval;
        }
        
        if (accelerometer == null)
        {
            accelerometer = getSensorManager().getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
            if (accelerometer == null)
                return;         
        }
        
        getSensorManager().registerListener(this, accelerometer, (int) (minimumtAccelerometerInterval/1000));
        
    }
    public boolean unregisterAccelerometer(int accelerometerId) 
    {   
        id2IntervalTimestampMap.remove(accelerometerId);
        
        if (sensorManager == null || accelerometer == null)
            return true;
        
        if (id2IntervalTimestampMap.size() == 0)
        {
            sensorManager.unregisterListener(this, accelerometer);
            accelerometer = null;
        }
        else
            updateSmallestIntervalAndStartAccelerometer();
        
        return true;
    }

    private void pauseAccelerometer()
    {
        if (accelerometer != null)
        {
            sensorManager.unregisterListener(this, accelerometer);
            accelerometer = null;
        }
    }

    private void resumeAccelerometer() 
    {
        if (id2IntervalTimestampMap != null && id2IntervalTimestampMap.size()> 0)
        {
            accelerometer = getSensorManager().getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
            if (accelerometer == null)
                return;         
            getSensorManager().registerListener(this, accelerometer, (int) (minimumtAccelerometerInterval/1000));
        }
    }

    public void setAccelerometerInterval(int accelerometerId, int interval) 
    {   
        if (interval <= 0)
            return;
            
        id2IntervalTimestampMap.put(accelerometerId, new Pair<Long, Long>((long) (interval*1000000), (long)0));  // milli sec -> nano sec
        
        this.updateSmallestIntervalAndStartAccelerometer();
    }

    public boolean isAccelerometerSupported() 
    {
        return getSensorManager().getSensorList(Sensor.TYPE_ACCELEROMETER).size() > 0;
    }

    public boolean isAccelerometerMuted() 
    {
        // Can't be muted.
        return false;
    } 
    
    public void onSensorChanged(SensorEvent event) 
    {
        switch (event.sensor.getType()) {
            case Sensor.TYPE_ACCELEROMETER:
            {
            	Configuration config = this.getResources().getConfiguration();
                double timestamp = event.timestamp - beginAppTimestamp;
                boolean bAccelerationValuesProcessed = false;
                double aX = 0, aY = 0, aZ = 0;
                //int tmpIdArray[] = new int[id2IntervalTimestampMap.size()];
                //int countIds = 0;
                for(int i = 0; i < id2IntervalTimestampMap.size(); i++)
                {
                    Pair<Long, Long> value = id2IntervalTimestampMap.valueAt(i);
                    
                    if ((event.timestamp - value.second) > value.first || value.second == 0)
                    {
                        if (!bAccelerationValuesProcessed)
                        {
                            bAccelerationValuesProcessed = true;
                            
                            if (config.orientation == Configuration.ORIENTATION_PORTRAIT)
                            {
                            aX = event.values[0] / SensorManager.GRAVITY_EARTH;
                            aY = event.values[1] / SensorManager.GRAVITY_EARTH;
                            aZ = event.values[2] / SensorManager.GRAVITY_EARTH; 
                        }
                            else if (config.orientation == Configuration.ORIENTATION_LANDSCAPE)
                            {
                            	aX = event.values[1] / SensorManager.GRAVITY_EARTH;
                            	aY = event.values[0] / SensorManager.GRAVITY_EARTH;
                            	if (DeviceRotation == Surface.ROTATION_270)
                            		aY = -aY;
                            	else
                            		aX = -aX;
                               	aZ = event.values[2] / SensorManager.GRAVITY_EARTH;                            	
                            }
                        }
                        
                        id2IntervalTimestampMap.setValueAt(i, new Pair<Long, Long>(value.first, event.timestamp));
                        NativeOnAccelerometerUpdate(id2IntervalTimestampMap.keyAt(i), timestamp, aX, aY, aZ);
                        //tmpIdArray[countIds++] = entry.getKey();
                    }                   
                }
                
                //int idArray[] = new int[countIds]; System.arraycopy(tmpIdArray, 0, idArray, 0, countIds);
                break;
            }

            default:
                break;
        }
    }
    
    public void onAccuracyChanged(Sensor sensor, int accuracy) 
    {   
    }
    
  //-------------------------------------------------------------------------------------------------
    private static LocationManager locationManager = null;
    private static SparseArray<Pair<Integer, Long>> geoId2IntervalTimestampMap;
    private final static int DefaultLocationInterval = 5000; // 5 seconds
    private static int SmallestLocationInterval = DefaultLocationInterval;
    private static boolean bGPSRegistered = false, bNetworkRegistered = false;
    
    private final LocationManager getLocationManager()
    {
        if (locationManager == null)
            locationManager = (LocationManager) getSystemService( Context.LOCATION_SERVICE );
        return locationManager;
    }
    
    private final static LocationListener locationListener = new LocationListener() 
    {
        public void onLocationChanged(Location location) 
        {
            if (geoId2IntervalTimestampMap == null)
                return;
            long timestamp = location.getTime();
            
            for(int i = 0; i < geoId2IntervalTimestampMap.size(); i++)
            {
                Pair<Integer, Long> value = geoId2IntervalTimestampMap.valueAt(i);
                
                if ((timestamp - value.second) > value.first || value.second == 0)
                {   
                    geoId2IntervalTimestampMap.setValueAt(i, new Pair<Integer, Long>(value.first, timestamp));
                    NativeOnGeolocationUpdate(geoId2IntervalTimestampMap.keyAt(i), location.getLatitude(), location.getLongitude(), location.getAltitude(), location.getAccuracy(),
                            location.getAccuracy(), location.getSpeed(), 0, timestamp - beginAppTimestamp);
                }                   
            }
        }
    
        public void onStatusChanged(String provider, int status, Bundle extras) 
        {
        	if (geoId2IntervalTimestampMap == null || geoId2IntervalTimestampMap.size() == 0 || locationManager == null)
                return;
        	
        	if (LocationManager.GPS_PROVIDER.equals(provider)  || LocationManager.NETWORK_PROVIDER.equals(provider))
             { 
            switch (status) 
            {
                case LocationProvider.OUT_OF_SERVICE:
	                	NativeOnStatus(provider, "outOfService");
	                	break;
                case LocationProvider.TEMPORARILY_UNAVAILABLE:
	                	NativeOnStatus(provider, "temporarilyUnavailable");
                    break;
	                    
	                case LocationProvider.AVAILABLE:
	                	NativeOnStatus(provider, "available");
	                	break;
	                default: ;               
	            }
            }
        }
    
        public void onProviderEnabled(String provider) 
        {
            if (geoId2IntervalTimestampMap == null || geoId2IntervalTimestampMap.size() == 0 || locationManager == null)
                return;
            
            // Request again location update, interval may have changed
            if (LocationManager.GPS_PROVIDER.equals(provider)  || LocationManager.NETWORK_PROVIDER.equals(provider))
            {
                locationManager.requestLocationUpdates(provider, SmallestLocationInterval, 0.0f, locationListener, Looper.getMainLooper());
                NativeOnStatus(provider, "enabled");
            }
        }
        public void onProviderDisabled(String provider) 
        {
        	if (geoId2IntervalTimestampMap == null || geoId2IntervalTimestampMap.size() == 0 || locationManager == null)
                return;
        	
        	 if (LocationManager.GPS_PROVIDER.equals(provider)  || LocationManager.NETWORK_PROVIDER.equals(provider))
             {
        		 NativeOnStatus(provider, "disabled");
             }
        }
    };
    
    public boolean registerGeolocation(int geolocationId) 
    {
        computeSmallestLocationInterval();
        
        if (isGPSEnabled())
            requestGPSLocationUpdates();
        if (isNetworkEnabled())
            requestNetworkLocationUpdates();
        
        if (geoId2IntervalTimestampMap == null)
            geoId2IntervalTimestampMap = new SparseArray<Pair<Integer,Long>>();
        
        geoId2IntervalTimestampMap.put(geolocationId, new Pair<Integer, Long>(DefaultLocationInterval, (long)0));       
        return true;
    }

    private boolean computeSmallestLocationInterval() 
    {
        if (geoId2IntervalTimestampMap == null || geoId2IntervalTimestampMap.size() == 0)
            return false;
        
        int oldSmallestInterval = SmallestLocationInterval;
        SmallestLocationInterval = 3600000; //1 hour
        for(int i = 0; i < geoId2IntervalTimestampMap.size(); i++)
        {
            int value = geoId2IntervalTimestampMap.valueAt(i).first;
            if (value < SmallestLocationInterval)
                SmallestLocationInterval = value;
        }
        
        return oldSmallestInterval != SmallestLocationInterval;
    }
    
    public boolean unregisterGeolocation(int geolocationId) 
    {       
        geoId2IntervalTimestampMap.remove(geolocationId);
        this.computeSmallestLocationInterval();
        
        if (geoId2IntervalTimestampMap.size() == 0)
            getLocationManager().removeUpdates(locationListener);
        return true;
    }

    private void pauseGeolocation()
    {
        getLocationManager().removeUpdates(locationListener);
    }

    private void resumeGeolocation() 
    {
        if (geoId2IntervalTimestampMap != null && geoId2IntervalTimestampMap.size()> 0)
        {
            if (isGPSEnabled())
                requestGPSLocationUpdates();
            if (isNetworkEnabled())
                requestNetworkLocationUpdates();
        }
    }

    public void setGeolocationInterval(int geoId, int interval) 
    {   
        if (interval <= 0)
            return;
        
        geoId2IntervalTimestampMap.put(geoId, new Pair<Integer, Long>(interval, (long)0));
        
        if (!this.computeSmallestLocationInterval())
            return;
        
        if (isGPSEnabled()) 
            requestGPSLocationUpdates();
        if (isNetworkEnabled())
            requestNetworkLocationUpdates();
    }

    public boolean isGeolocationSupported() 
    {
        return true;
    }

    public boolean isGeolocationMuted() 
    {
        return !isGPSEnabled() && !isNetworkEnabled(); 
    }
    
    private boolean isGPSEnabled() 
    {
    	try {
        return getLocationManager().isProviderEnabled(LocationManager.GPS_PROVIDER); 
    	} catch(Exception e) {
    		return false;
    	}
    }
    
    private boolean isNetworkEnabled() 
    {
    	try {
        return getLocationManager().isProviderEnabled(LocationManager.NETWORK_PROVIDER); 
	    } catch(Exception e) {
			return false;
		}
    }
    
    private void requestGPSLocationUpdates() 
    {
        getLocationManager().requestLocationUpdates(LocationManager.GPS_PROVIDER, SmallestLocationInterval, 0, locationListener, Looper.getMainLooper()); 
    }
    
    private void requestNetworkLocationUpdates() 
    {
        getLocationManager().requestLocationUpdates(LocationManager.NETWORK_PROVIDER, SmallestLocationInterval, 0, locationListener, Looper.getMainLooper()); 
    }
}
