package com.adobe.fre;

public class FREArray extends FREObject 
{	
	protected FREArray(String base, FREObject[] constructorArgs) throws FRETypeMismatchException, FREInvalidObjectException,
			FREASErrorException, FRENoSuchNameException, FREWrongThreadException 
	{
		super("Vector.<" + base + ">", constructorArgs);
	}

	protected FREArray(FREObject[] constructorArgs) throws FRETypeMismatchException, FREInvalidObjectException,
			FREASErrorException, FRENoSuchNameException, FREWrongThreadException 
	{
		super("Array", constructorArgs);
	}

	public static FREArray newArray(String classname, int numElements, 	boolean fixed) throws FREASErrorException, FRENoSuchNameException,	FREWrongThreadException 
	{
		FREObject[] args = new FREObject[2];
		args[0] = new FREObject(numElements);
		args[1] = new FREObject(fixed);
		try 
		{
			return new FREArray(classname, args);
		} catch (FRETypeMismatchException e) {
		} catch (FREInvalidObjectException e) {
		} catch (IllegalStateException e) {
		}
		
		return null;
	}

	public static FREArray newArray(int numElements) throws FREASErrorException, FREWrongThreadException 
	{
		FREObject[] args = new FREObject[1];
		args[0] = new FREObject(numElements);
		try 
		{
			return new FREArray(args);
		} catch (FRETypeMismatchException e) {
		} catch (FREInvalidObjectException e) {
		} catch (FRENoSuchNameException e) {
		} catch (IllegalStateException e) {
		}
		
		return null;
	}

	public native long getLength() throws FREInvalidObjectException, FREWrongThreadException;
	public native void setLength(long paramLong) throws FREInvalidObjectException, IllegalArgumentException, FREReadOnlyException, FREWrongThreadException;
	public native FREObject getObjectAt(long paramLong) throws FREInvalidObjectException, IllegalArgumentException, FREWrongThreadException;
	public native void setObjectAt(long paramLong, FREObject paramFREObject) throws FREInvalidObjectException, FRETypeMismatchException, FREWrongThreadException;
}