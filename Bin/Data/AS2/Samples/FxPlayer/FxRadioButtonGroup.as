class FxRadioButtonGroup
{
	private var Radios:Array;
	private var CallbackHandler:Object;
	public var SelectedRadio:Object;
	private var Enabled:Boolean;
	
	public function FxRadioButtonGroup()
	{
		this.Radios = new Array();
		this.SelectedRadio = null;
		this.Enabled = true;
	}

	public function addRadio(radio:Object)
	{
		radio.Initialize(this);
		radio.RadioGroupIndex = Radios.length;
		Radios.push(radio);
	}
	
	public function setEnabledState(b:Boolean)
	{
		if (Enabled == b)
			return;
		Enabled = b;
		for (var i=0; i < Radios.length; i++)
			Radios[i].Enable(Enabled);
		this.onEnabled(b);
	}
	
	public function setSelectedIndex(idx:Number)
	{
		onSelect(Radios[idx]);
	}
	
	public function setSelectedRadio(radio:Object)
	{
		onSelect(radio);
	}
	
	public function onSelect(radio:Object)
	{
		if (this.SelectedRadio != null && this.SelectedRadio != radio)
		{
			this.SelectedRadio.Select(false);
		}
		if (radio.bSelectedRadio != radio)
		{
			radio.Select(true);
			this.SelectedRadio = radio;
			this.onSelected(radio);
		}
	}
	
	public function onSelected(radio:Object)
	{
		// Empty implementation
	}	
	
	public function onEnabled(radio:Object, b:Boolean)
	{
		// Empty implementation
	}
}