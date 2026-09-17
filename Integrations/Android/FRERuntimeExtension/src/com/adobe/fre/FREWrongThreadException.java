package com.adobe.fre;

public class FREWrongThreadException extends Exception {
	public static final long serialVersionUID = 1L;

	public FREWrongThreadException(String msg) {
		super(msg);
	}
}