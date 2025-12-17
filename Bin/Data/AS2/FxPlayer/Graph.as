class Graph extends MovieClip
{
	var valv8, valv6, valv4, valv2; //textfield
	var NumPoints = 30;
	var ColorList:Array = new Array(0x0fc813, 0x82eaf7, 0x10eac8, 0xdf73fa, 0xffde1f, 0xff0601);
	//Xlist is onedimentional, since  time is the same for all graphs
	var Data:Array;// = new Array(new Array); //= new Array(20, 10, 15, 40);
	var PlotXList:Array = new Array; 
	var PlotYList:Array;// = new Array(new Array);
	var X_inc:Number = 24;
	var MaxVal:Number = 0;
	var Scale:Number = 0;
	
	
	
	public function Graph ()
	{
		for (var ii=0; ii<NumPoints; ii++)
		{
			PlotXList[ii] =  ii*X_inc;			
		}
	}
	
	public function AddPoint(pointList:Array)
	{
		// To make things faster first value must be the highest
		// i.e. first plot must be parent peak value
	    var cpoint:Number = pointList[0];
		//trace(Data);
		if (Data[0].length >= NumPoints)
		{
				Data[0].shift();
				PlotYList[0].shift();
		}
//		if(MaxVal < cpoint) 
//		{
//			SetScale(cpoint);
//		}
//		else
		{
			Data[0].push(cpoint);
			PlotYList[0].push(-cpoint * Scale);
		}
		
		for (var pp = 1; pp < pointList.length; pp++)
		{
			cpoint = pointList[pp];
			if (Data[pp].length >= NumPoints)
			{
					Data[pp].shift();
					PlotYList[pp].shift();
			}
			Data[pp].push(cpoint);
			PlotYList[pp].push(-cpoint * Scale);
			//trace("PlotYList.length:" + PlotYList.length);
		}
		
	}
	
	public function Plot()
	{
		//trace("PlotXList:" + PlotXList);
		//trace("PlotYList:" + PlotYList);
		clear();
		lineStyle(1.5, 0xFF0000, 150);
		moveTo(PlotXList[0], PlotYList[0][0]);
		for(var ii=1; ii < PlotYList[0].length; ii++)
		{
			lineTo(PlotXList[ii], PlotYList[0][ii]);	
		}
		for (var jj = 1; jj < Data.length -1; jj++)
		{
			lineStyle(1.5, ColorList[(jj - 1) % ColorList.length], 150);
			//trace("Color: "+ColorList[jj % ColorList.length]);
			moveTo(PlotXList[0], PlotYList[jj][0]);
			beginFill(ColorList[(jj - 1) % ColorList.length], 50);
			//trace("PlotYList[jj]:" + PlotYList[jj]);
			for(var ii=1; ii < PlotYList[jj].length; ii++)
			{
				lineTo(PlotXList[ii], PlotYList[jj][ii]);	
			}
			for (var ii = PlotYList[jj + 1].length - 1; ii >=0; ii--)
			{
				lineTo(PlotXList[ii], PlotYList[jj+1][ii]);
			}
			endFill();
		}
		var jj = Data.length - 1;
		lineStyle(1.5, ColorList[(jj - 1) % ColorList.length], 150);
		beginFill(ColorList[(jj - 1) % ColorList.length], 50);
		moveTo(PlotXList[0], PlotYList[jj][0]);
		//trace("PlotYList[jj]:" + PlotYList[jj]);
		for(var ii=1; ii < PlotYList[jj].length; ii++)
		{
			lineTo(PlotXList[ii], PlotYList[jj][ii]);	
		}
		lineTo(PlotXList[Data[jj].length-1], -1);
		lineTo(1, -1);
		endFill();		
	}
	
	public function SetupGraphs(ngraphs:Number)
	{
		Data = new Array(ngraphs);
		PlotYList = new Array(ngraphs);
		for (var jj = 0; jj < ngraphs; jj++)
		{
			Data[jj] = new Array;
			PlotYList[jj] = new Array;
		}
		MaxVal = 0;
		Scale = 0;
	}
	
	public function SetScale (maxVal : Number)
	{
		//trace ("SetScale: maxVal = "+maxVal);
		MaxVal = maxVal;
		//Data[0].push(maxVal);
		Scale = 192/MaxVal; //// _height/MaxVal;
		for (var jj = 0; jj < Data.length; jj++)
		{
			for(var ii=0; ii < Data[jj].length;ii++)
			{
				PlotYList[jj][ii] =  - Data[jj][ii] * Scale; 				
			}
		}
		_root.graphValv8.text = SetLabel (MaxVal);
		_root.graphValv6.text = SetLabel (MaxVal/4*3);
		_root.graphValv4.text = SetLabel(MaxVal/2);
		_root.graphValv2.text = SetLabel (MaxVal/4);
	}
	
	public function SetLabel (n: Number) : String
	{
		var str:String;
		var m:Number = 1000000;
		var k:Number = 1000;
		//trace(_root.tabRow.tabMemory.bSelected);
		if (_root.tabRow.tabMemory.bSelected) // for memory K=1024
		{
			k = 1024;
			m = 1024*1024; 
		}
		if (n > m)
		{
			str = Math.floor(n * 10 / m) / 10  + "M";
		}
		else if (n > k)
		{
			str = Math.floor(n * 10/ k) / 10 + "K";
		}
		else
		{
			str = n.toString();
		}
		
		return str;
				
	}
	
	public function Clear()
	{
		clear();
		for (var jj = 0; jj < Data.length; jj++)
		{
			Data[jj].length = 0;
			PlotYList[jj].length = 0;
		}
		MaxVal = 0;
		Scale = 0; 
	}
	
}