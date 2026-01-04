package com.adobe.fre;

public class FRENoSuchNameException extends Exception {
	
	public static final long serialVersionUID = 1L;
	
	public FRENoSuchNameException(String msg) {
		super(msg);
	}
}