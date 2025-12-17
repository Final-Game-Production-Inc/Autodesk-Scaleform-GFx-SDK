
package
{
import flash.display.*
import flash.events.MouseEvent;

 public class baseradio extends MovieClip
 {
	public var Btn:MovieClip;
	function pressHandler(event:MouseEvent):void
	{
		if ( this.Btn.currentFrame < MovieClip(root).RadioOn)
		{
			this.Btn.gotoAndPlay("offpress");
		}
		stage.focus = this;
	}
	
	function releaseHandler(event:MouseEvent):void
	{
		if ( this.Btn.currentFrame < MovieClip(root).RadioOn)
		{
			this.Btn.gotoAndPlay("onover");
		}
		if ( !MovieClip(root).IsSelected(this))
		{
			 MovieClip(root).ToggleOption(this);
		}
	}
	
	function rolloverHandler(event:MouseEvent):void
	{
		if ( this.Btn.currentFrame < MovieClip(root).RadioOn)
		{
			this.Btn.gotoAndPlay("offover");
		}
		else
		{
			this.Btn.gotoAndPlay("onover");
		}
	}
	
	function rolloutHandler(event:MouseEvent):void
	{
		if ( this.Btn.currentFrame < MovieClip(root).RadioOn)
		{
			this.Btn.gotoAndPlay("off");
		}
		else
		{
			this.Btn.gotoAndPlay("on");
		}
	}
	
	public function baseradio()
	{
		// create event handlers
		addEventListener(MouseEvent.MOUSE_DOWN, pressHandler);
		addEventListener(MouseEvent.MOUSE_UP, releaseHandler);
		addEventListener(MouseEvent.ROLL_OVER, rolloverHandler);
		addEventListener(MouseEvent.ROLL_OUT, rolloutHandler);
	}
 }
}
