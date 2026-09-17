package com.adobe.fre;

public class FREReadOnlyException extends Exception {
	public static final long serialVersionUID = 1L;

	public FREReadOnlyException(String msg) {
		super(msg);
	}
}
