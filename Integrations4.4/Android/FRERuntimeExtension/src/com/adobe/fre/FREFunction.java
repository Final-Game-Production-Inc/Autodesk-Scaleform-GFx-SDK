package com.adobe.fre;

public abstract interface FREFunction
{
  public abstract FREObject call(FREContext paramFREContext, FREObject[] paramArrayOfFREObject);
}