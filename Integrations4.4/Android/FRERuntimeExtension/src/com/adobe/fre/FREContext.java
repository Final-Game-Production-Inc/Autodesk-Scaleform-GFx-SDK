package com.adobe.fre;
 
import java.util.Map;

import android.app.Activity;
import android.content.res.Resources;

public abstract class FREContext 
{
	public abstract Map<String, FREFunction> getFunctions();
	public abstract void dispose();

	public native void dispatchStatusEventAsync(String code, String level) throws IllegalArgumentException, IllegalStateException;	
	public native Activity getActivity() throws IllegalStateException;
	
	public native FREObject getActionScriptData();
	public native void setActionScriptData(FREObject objASData);

	public /*native*/ int getResourceId(String paramString) throws IllegalArgumentException, Resources.NotFoundException, IllegalStateException
	{
		if (paramString == null || paramString.length() == 0)
			throw new IllegalArgumentException("Argument method is not valid.");
		
		Activity act = getActivity();
		int resourceId = act.getResources().getIdentifier(paramString, "id", act.getPackageName());
		
		if (resourceId == 0)
			throw new Resources.NotFoundException("Resource id couldn't be found.");
		
		return resourceId;
	}
}
