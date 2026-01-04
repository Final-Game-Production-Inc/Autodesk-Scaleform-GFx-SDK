package com.adobe.fre;

public class FREObject
{
	private long pObject;	
	private FREObject() { }
	
	public void finalize() throws Throwable
	{
		super.finalize();
	    releaseNative();
	}
	
	public static FREObject newObject(int value) throws FREWrongThreadException 	{ return new FREObject(value); }
	public static FREObject newObject(double value) throws FREWrongThreadException 	{ return new FREObject(value); }
	public static FREObject newObject(boolean value) throws FREWrongThreadException { return new FREObject(value); }
	public static FREObject newObject(String value) throws FREWrongThreadException 	{ return new FREObject(value); }
	
	protected FREObject(int value) throws FREWrongThreadException {
		//jObject = Integer.valueOf(value);
		FREObjectFromInt(value);
	}

	protected FREObject(double value) throws FREWrongThreadException {
		//jObject = Double.valueOf(value);
		FREObjectFromDouble(value);
	}

	protected FREObject(boolean value) throws FREWrongThreadException {
		//jObject = Boolean.valueOf(value);
		FREObjectFromBoolean(value);
	}

	protected FREObject(String value) throws FREWrongThreadException {
		//jObject = new String(value);
		FREObjectFromString(value);
	}
	

	/*****************************************************************************************************************/
	
	public native int getAsInt() throws FRETypeMismatchException, FREInvalidObjectException, FREWrongThreadException, IllegalStateException;
	public native double getAsDouble() throws FRETypeMismatchException, FREInvalidObjectException, FREWrongThreadException, IllegalStateException;
	public native boolean getAsBool() throws FRETypeMismatchException, FREInvalidObjectException, FREWrongThreadException, IllegalStateException;
	public native String getAsString() throws FRETypeMismatchException, FREInvalidObjectException, FREWrongThreadException, IllegalStateException;
	
	private native void FREObjectFromInt(int paramInt) throws FREWrongThreadException;
	private native void FREObjectFromDouble(double paramDouble) throws FREWrongThreadException;
	private native void FREObjectFromBoolean(boolean paramBoolean) throws FREWrongThreadException;
	private native void FREObjectFromString(String paramString) throws FREWrongThreadException;

	private native void FREObjectFromClass(String paramString, FREObject[] paramArrayOfFREObject) 
		throws FRETypeMismatchException, FREInvalidObjectException, FREASErrorException, FRENoSuchNameException, FREWrongThreadException;
	
	public static native FREObject newObject(String className, FREObject constructorArgs[]) 
		throws FRETypeMismatchException, FREInvalidObjectException, FREASErrorException, FRENoSuchNameException, FREWrongThreadException, IllegalStateException;
	
	public FREObject(String className, FREObject[] constructorArgs)
		throws FRETypeMismatchException, FREInvalidObjectException, FREASErrorException, FRENoSuchNameException, FREWrongThreadException, IllegalStateException
	{
		FREObjectFromClass(className, constructorArgs);
	}
	
	public native FREObject getProperty(String paramString)
		throws FRETypeMismatchException, FREInvalidObjectException, FREASErrorException, FRENoSuchNameException, FREWrongThreadException, IllegalStateException;
	
	public native void setProperty(String paramString, FREObject paramFREObject)
		throws FRETypeMismatchException, FREInvalidObjectException, FREASErrorException, FRENoSuchNameException, FREReadOnlyException, FREWrongThreadException, IllegalStateException;
	
	public native FREObject callMethod(String paramString, FREObject[] paramArrayOfFREObject) 
		throws FRETypeMismatchException, FREInvalidObjectException, FREASErrorException, FRENoSuchNameException, FREWrongThreadException, IllegalStateException;
	
	public native void releaseNative();

}