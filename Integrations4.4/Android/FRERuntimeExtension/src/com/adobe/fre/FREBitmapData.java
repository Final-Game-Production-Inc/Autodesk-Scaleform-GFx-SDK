package com.adobe.fre;

import java.nio.ByteBuffer;

public class FREBitmapData extends FREObject 
{
	private long m_dataPointer;

	protected FREBitmapData(FREObject[] constructorArgs) throws FRETypeMismatchException, FREInvalidObjectException, FREASErrorException, FRENoSuchNameException, FREWrongThreadException, IllegalStateException 
	{
		super("flash.display.BitmapData", constructorArgs);
	}

	public static FREBitmapData newBitmapData(int width, int height, boolean transparent, Byte[] fillColor) 
			throws FREASErrorException, FREWrongThreadException, IllegalArgumentException 
	{
		if (fillColor.length != 4)
		{
			throw new IllegalArgumentException("fillColor has wrong length");
		}
	
		FREObject[] array = new FREObject[4];
		array[0] = new FREObject(width);
		array[1] = new FREObject(height);
		array[2] = new FREObject(transparent);
		int color = 0;
		
		for (int i = 0; i < 4; i++) 
		{
			int cShiftCount = 8 * (3 - i);
			color |= fillColor[i].byteValue() << cShiftCount;
		}
		array[3] = new FREObject(color);
		
		try 
		{
			return new FREBitmapData(array);
		} catch (FRETypeMismatchException e) {
		} catch (FREInvalidObjectException e) {
		} catch (FRENoSuchNameException e) {
		} catch (IllegalStateException e) {
		}
		
		return null;
	}

	public native int getWidth() throws FREInvalidObjectException, FREWrongThreadException, IllegalStateException;
	public native int getHeight() throws FREInvalidObjectException, FREWrongThreadException, IllegalStateException;
	public native boolean hasAlpha() throws FREInvalidObjectException, FREWrongThreadException, IllegalStateException;
	
	// Not implemented, use FREObject.callMethod() instead
	public native boolean isPremultiplied() throws FREInvalidObjectException, FREWrongThreadException, IllegalStateException;
	public native int getLineStride32() throws FREInvalidObjectException, FREWrongThreadException, IllegalStateException;
	public native boolean isInvertedY() throws FREInvalidObjectException, FREWrongThreadException, IllegalStateException;
	public native ByteBuffer getBits() throws FREInvalidObjectException, FREWrongThreadException, IllegalStateException;
	public native void acquire() throws FREInvalidObjectException, FREWrongThreadException, IllegalStateException;
	public native void invalidateRect(int paramInt1, int paramInt2, int paramInt3, int paramInt4) throws FREInvalidObjectException, FREWrongThreadException, IllegalStateException, IllegalArgumentException;
	public native void release() throws FREInvalidObjectException, FREWrongThreadException, IllegalStateException;
}
