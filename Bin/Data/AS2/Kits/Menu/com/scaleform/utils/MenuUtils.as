/**
 * ...
 * @author 
 */

class com.scaleform.utils.MenuUtils
{
	public static function setDocumentClass(mc:MovieClip, cls:Object):Void {
		registerClass(mc, cls);
	}
	
	public static function registerClass(mc:MovieClip, cls:Object):Void {
		mc.__proto__ = cls.prototype;
		Function(cls).apply(mc, null);
	}
}