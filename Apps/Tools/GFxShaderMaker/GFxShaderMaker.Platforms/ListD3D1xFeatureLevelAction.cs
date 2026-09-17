namespace GFxShaderMaker.Platforms;

public class ListD3D1xFeatureLevelAction : ListEnumeration
{
	public ListD3D1xFeatureLevelAction()
		: base("D3D1x Feature Levels:\n (use comma separated list if multiple are desired, eg. -featurelevel D3D_FEATURE_LEVEL_9_1,D3D_FEATURE_LEVEL_10_0", typeof(Platform_D3D1x.FeatureLevels))
	{
	}
}
