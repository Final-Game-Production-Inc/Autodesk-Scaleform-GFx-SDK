using System.Collections.Generic;
using System.Linq;
using System.Xml;

namespace GFxShaderMaker;

public class ShaderGroup : ShaderRestriction
{
	public string ID;

	public List<ShaderFeature> Features;

	public List<ShaderPermutation> Permutations;

	public Dictionary<string, uint> FeatureFlavorFlags;

	public override void ReadFromXml(XmlElement root)
	{
		Features = new List<ShaderFeature>();
		ID = root.Attributes.GetNamedItem("id").Value;
		foreach (XmlElement item in root.GetElementsByTagName("ShaderFeature"))
		{
			ShaderFeature shaderFeature = new ShaderFeature();
			shaderFeature.ReadFromXml(item);
			Features.Add(shaderFeature);
		}
		base.ReadFromXml(root);
	}

	private void DetermineFeatureFlags(Dictionary<string, uint> globalFeatureFlags)
	{
		FeatureFlavorFlags = new Dictionary<string, uint>();
		foreach (ShaderFeature feature in Features)
		{
			foreach (ShaderFeatureFlavor flavor in feature.Flavors)
			{
				if (flavor.ID != ShaderFeatureFlavor.EmptyID)
				{
					if (feature.Flavors.Count == 1 || !globalFeatureFlags.TryGetValue(flavor.ID, out var value))
					{
						value = 0u;
					}
					FeatureFlavorFlags.Add(flavor.ID, value);
				}
			}
		}
	}

	public void CreatePermutations(Dictionary<string, uint> globalFeatureFlags)
	{
		DetermineFeatureFlags(globalFeatureFlags);
		Permutations = new List<ShaderPermutation>();
		int[] array = new int[Features.Count];
		for (int i = 0; i < Features.Count; i++)
		{
			array[i] = 0;
		}
		int num = Features.Count - 1;
		while (num >= 0)
		{
			ShaderPermutation shaderPermutation = new ShaderPermutation();
			for (int j = 0; j < Features.Count; j++)
			{
				shaderPermutation.SpecificFeatures.Add(Features[j].Flavors[array[j]]);
			}
			if (shaderPermutation.IsValid())
			{
				shaderPermutation.ComputeShaderType(FeatureFlavorFlags);
				Permutations.Add(shaderPermutation);
			}
			bool flag = false;
			array[num]++;
			while (num >= 0 && array[num] >= Features[num].Flavors.Count())
			{
				array[num] = 0;
				num--;
				if (num < 0)
				{
					flag = false;
					break;
				}
				array[num]++;
				flag = true;
			}
			if (flag)
			{
				num = Features.Count - 1;
			}
		}
	}

	public List<ShaderLinkedSource> LinkShaderSources(List<ShaderSource> sources, ShaderVersion shaderVersion)
	{
		List<ShaderLinkedSource> list = new List<ShaderLinkedSource>();
		if (IsRestricted(shaderVersion))
		{
			return list;
		}
		foreach (ShaderPermutation permutation in Permutations)
		{
			if (permutation.IsValid(shaderVersion))
			{
				list.AddRange(permutation.LinkShaderSources(sources, shaderVersion));
			}
		}
		return list;
	}
}
