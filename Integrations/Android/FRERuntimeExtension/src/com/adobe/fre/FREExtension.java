package com.adobe.fre;

public abstract interface FREExtension 
{	
	public abstract void initialize();
	public abstract FREContext createContext(String paramString);
	public abstract void dispose();
}