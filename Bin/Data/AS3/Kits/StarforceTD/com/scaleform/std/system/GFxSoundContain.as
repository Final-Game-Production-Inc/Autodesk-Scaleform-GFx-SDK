package com.scaleform.std.system
{
    import flash.media.SoundChannel;
    import flash.media.SoundTransform;
	/**
     * ...
     * @author ...
     */
    public class GFxSoundContain 
    {
        
        public var channel: SoundChannel;
        public var transform: SoundTransform;
        
        public function GFxSoundContain(channel: SoundChannel, transform: SoundTransform) 
        {
            this.channel = channel;
            this.transform = transform;
        }
        
        public function get volume():Number
        {
            return transform.volume;
        }
        
        public function set volume(val: Number):void
        {
            transform.volume = val;
            channel.soundTransform = transform;
        }
    }

}