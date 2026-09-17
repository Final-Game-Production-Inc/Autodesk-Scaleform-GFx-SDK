package com.adobe.fre;

import java.nio.ByteBuffer;

public class FREByteArray extends FREObject 
{
	private long m_dataPointer;

	protected FREByteArray() throws FRETypeMismatchException, FREInvalidObjectException, FREASErrorException, FRENoSuchNameException, FREWrongThreadException, IllegalStateException 
	{
		super("flash.utils.ByteArray", null);
	}

	public static FREByteArray newByteArray() throws FREASErrorException, FREWrongThreadException 
	{
		try 
		{
			return new FREByteArray();
		
		} catch (FRETypeMismatchException e) {
		} catch (FREInvalidObjectException e) {
		} catch (FRENoSuchNameException e) {
		} catch (IllegalStateException e) {
		}
		
		return null;
	}

	public native long getLength() throws FREInvalidObjectException, FREWrongThreadException, IllegalStateException;
	public native ByteBuffer getBytes() throws FREInvalidObjectException, FREWrongThreadException, IllegalStateException;
	public native void acquire() throws FREInvalidObjectException, FREWrongThreadException, IllegalStateException;
	public native void release() throws FREInvalidObjectException, FREWrongThreadException, IllegalStateException;
}
