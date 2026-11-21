using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;
using Microsoft.Win32;

namespace GFxShaderMaker.Platforms;

/// <summary>
/// Locates native Windows SDK / DirectX SDK tools (fxc.exe, dxc.exe).
///
/// The old implementation searched the whole "Windows Kits" tree and blindly took
/// the first match. On a machine that has several SDK versions installed the first
/// match is often the ARM64 (or ARM) build of the tool, which cannot be launched
/// from an x64 process -> "The application was unable to start correctly" /
/// "计算机类型不匹配" (ERROR_BAD_EXE_FORMAT, 216).
///
/// This locator therefore:
///   1. only considers the SDK layout that actually ships the tools
///        - Windows SDK 10 : &lt;KitsRoot&gt;\Bin\&lt;version&gt;\&lt;arch&gt;\fxc.exe
///        - Windows SDK 8.x: &lt;KitsRoot&gt;\Bin\&lt;arch&gt;\fxc.exe
///        - DirectX SDK   : &lt;DXSDK_DIR&gt;\Utilities\bin\&lt;arch&gt;\fxc.exe
///   2. prefers the host architecture (x64 on x64 Windows, ARM64 on ARM64 Windows)
///      and falls back to x86 (runs through WOW64 / emulation),
///   3. validates the PE machine type of every candidate before returning it,
///   4. prefers the newest SDK version,
///   5. honours an explicit override environment variable.
/// </summary>
internal static class ShaderCompilerLocator
{
	internal const string FxcOverrideEnvironmentVariable = "GFXSHADERMAKER_FXC";
	internal const string DxcOverrideEnvironmentVariable = "GFXSHADERMAKER_DXC";

	private const ushort MachineUnknown = 0;
	private const ushort MachineI386 = 0x014C;
	private const ushort MachineAmd64 = 0x8664;
	private const ushort MachineArm = 0x01C4;
	private const ushort MachineIa64 = 0x0200;
	private const ushort MachineArm64 = 0xAA64;

	/// <summary>Machine type of the operating system (not of the current process).</summary>
	internal static ushort HostMachineType
	{
		get
		{
			// On a 32-bit process running on a 64-bit OS, PROCESSOR_ARCHITEW6432 holds
			// the real OS architecture. (String based so the code also builds for net35.)
			string arch = Environment.GetEnvironmentVariable("PROCESSOR_ARCHITEW6432");
			if (string.IsNullOrEmpty(arch))
			{
				arch = Environment.GetEnvironmentVariable("PROCESSOR_ARCHITECTURE");
			}
			switch ((arch ?? string.Empty).Trim().ToUpperInvariant())
			{
			case "AMD64":
			case "X64":
				return MachineAmd64;
			case "ARM64":
				return MachineArm64;
			case "ARM":
				return MachineArm;
			case "IA64":
				return MachineIa64;
			default:
				return MachineI386;
			}
		}
	}

	/// <summary>Returns true when a PE image of the given machine type can be started here.</summary>
	internal static bool CanExecute(ushort machine)
	{
		if (machine == MachineUnknown)
		{
			// Not a readable PE image - do not hand it out as a compiler.
			return false;
		}
		ushort host = HostMachineType;
		if (machine == host)
		{
			return true;
		}
		// 32-bit images run on every 64-bit Windows through WOW64 / x86 emulation.
		if (machine == MachineI386)
		{
			return host == MachineAmd64 || host == MachineArm64;
		}
		return false;
	}

	internal static bool IsNative(ushort machine)
	{
		return machine != MachineUnknown && machine == HostMachineType;
	}

	internal static string GetMachineTypeName(ushort machine)
	{
		switch (machine)
		{
		case MachineUnknown:
			return "unknown";
		case MachineI386:
			return "x86";
		case MachineAmd64:
			return "x64";
		case MachineArm:
			return "ARM";
		case MachineIa64:
			return "IA64";
		case MachineArm64:
			return "ARM64";
		default:
			return "0x" + machine.ToString("X4");
		}
	}

	/// <summary>Reads the PE header machine type of a file. Returns 0 when it is not a PE image.</summary>
	internal static ushort ReadMachineType(string path)
	{
		try
		{
			using (FileStream stream = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.ReadWrite))
			{
				if (stream.Length < 0x40)
				{
					return MachineUnknown;
				}
				using (BinaryReader reader = new BinaryReader(stream))
				{
					if (reader.ReadUInt16() != 0x5A4D) // 'MZ'
					{
						return MachineUnknown;
					}
					stream.Position = 0x3C;
					int peOffset = reader.ReadInt32();
					if (peOffset <= 0 || (long)peOffset + 6L > stream.Length)
					{
						return MachineUnknown;
					}
					stream.Position = peOffset;
					if (reader.ReadUInt32() != 0x00004550) // 'PE\0\0'
					{
						return MachineUnknown;
					}
					return reader.ReadUInt16();
				}
			}
		}
		catch (Exception)
		{
			return MachineUnknown;
		}
	}

	/// <summary>
	/// Returns every Windows Kits root that actually exists, newest first.
	/// </summary>
	internal static List<string> GetWindowsKitsRoots()
	{
		List<string> roots = new List<string>();
		try
		{
			using (RegistryKey key = Registry.LocalMachine.OpenSubKey("SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots"))
			{
				if (key != null)
				{
					foreach (string valueName in key.GetValueNames())
					{
						if (!valueName.StartsWith("KitsRoot", StringComparison.OrdinalIgnoreCase))
						{
							continue;
						}
						string value = key.GetValue(valueName) as string;
						if (!string.IsNullOrEmpty(value))
						{
							roots.Add(value);
						}
					}
				}
			}
		}
		catch (Exception)
		{
		}

		// Registry may be missing / redirected (32-bit view); fall back to the default install locations.
		foreach (string programFiles in new string[] { Environment.GetFolderPath(Environment.SpecialFolder.ProgramFilesX86), Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles) })
		{
			if (string.IsNullOrEmpty(programFiles))
			{
				continue;
			}
			foreach (string version in new string[] { "10", "8.1", "8.0" })
			{
				string candidate = Path.Combine(programFiles, "Windows Kits", version);
				if (Directory.Exists(candidate))
				{
					roots.Add(candidate);
				}
			}
		}

		// De-duplicate and sort newest first.
		List<string> unique = new List<string>();
		HashSet<string> seen = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
		foreach (string root in roots)
		{
			if (string.IsNullOrEmpty(root))
			{
				continue;
			}
			string normalized = root.TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);
			if (seen.Add(normalized))
			{
				unique.Add(normalized);
			}
		}
		unique.Sort(delegate(string s0, string s1)
		{
			Version v0 = ExtractTrailingVersion(s0);
			Version v1 = ExtractTrailingVersion(s1);
			int result = v1.CompareTo(v0);
			return (result != 0) ? result : string.Compare(s0, s1, StringComparison.OrdinalIgnoreCase);
		});
		return unique;
	}

	/// <summary>Parses the trailing numeric component of a path ("...\Windows Kits\10" -> 10.0).</summary>
	private static Version ExtractTrailingVersion(string path)
	{
		try
		{
			string last = Path.GetFileName(path.TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar));
			if (last == null)
			{
				return new Version(0, 0);
			}
			if (!last.Contains("."))
			{
				double major;
				if (double.TryParse(last, out major))
				{
					return new Version((int)major, 0);
				}
				return new Version(0, 0);
			}
			Match match = Regex.Match(last, "\\d+(?:\\.\\d+)*$");
			if (match.Success)
			{
				return new Version(match.Value);
			}
		}
		catch (Exception)
		{
		}
		return new Version(0, 0);
	}

	/// <summary>
	/// Finds the tool inside the installed Windows Kits, newest SDK version and host architecture first.
	/// </summary>
	internal static string FindInWindowsKits(string executableName, int verbosity)
	{
		foreach (string root in GetWindowsKitsRoots())
		{
			if (verbosity > 1)
			{
				Console.WriteLine("Looking for {0} in Windows Kits root '{1}'.", executableName, root);
			}
			string found = FindInWindowsKitRoot(root, executableName, verbosity);
			if (found != null)
			{
				return found;
			}
		}
		return null;
	}

	private static string FindInWindowsKitRoot(string kitsRoot, string executableName, int verbosity)
	{
		string binRoot = Path.Combine(kitsRoot, "Bin");
		if (!Directory.Exists(binRoot))
		{
			return null;
		}

		// Windows SDK 8.x / 8.1 layout: <root>\Bin\x64\fxc.exe
		foreach (string archDirectory in GetArchitectureDirectories())
		{
			string candidate = Path.Combine(binRoot, archDirectory, executableName);
			if (IsUsable(candidate, verbosity))
			{
				return candidate;
			}
		}

		// Windows SDK 10 layout: <root>\Bin\<version>\<arch>\fxc.exe  (newest version first)
		foreach (string versionDirectory in GetVersionDirectories(binRoot))
		{
			foreach (string archDirectory in GetArchitectureDirectories())
			{
				string candidate = Path.Combine(versionDirectory, archDirectory, executableName);
				if (IsUsable(candidate, verbosity))
				{
					return candidate;
				}
			}
		}

		if (verbosity != 0)
		{
			Console.WriteLine("Warning: no usable {0} found in '{1}'. Trying next Windows Kit.", executableName, kitsRoot);
		}
		return null;
	}

	/// <summary>
	/// Finds the tool inside an arbitrary SDK root (DirectX SDK, XDK, DurangoXDK ...).
	/// Prefers native binaries, then x86, and reports anything that was rejected because of its architecture.
	/// </summary>
	internal static string FindInRoot(string rootDirectory, string executableName, int verbosity)
	{
		if (string.IsNullOrEmpty(rootDirectory) || !Directory.Exists(rootDirectory))
		{
			return null;
		}

		string[] files;
		try
		{
			files = Directory.GetFiles(rootDirectory, executableName, SearchOption.AllDirectories);
		}
		catch (Exception ex)
		{
			if (verbosity > 1)
			{
				Console.WriteLine("Warning: could not search '{0}': {1}", rootDirectory, ex.Message);
			}
			return null;
		}
		if (files.Length == 0)
		{
			return null;
		}

		List<string> candidates = new List<string>();
		foreach (string file in files)
		{
			ushort machine = ReadMachineType(file);
			if (!CanExecute(machine))
			{
				if (verbosity > 0)
				{
					Console.WriteLine("Ignoring '{0}' ({1} build cannot run on this machine).", file, GetMachineTypeName(machine));
				}
				continue;
			}
			candidates.Add(file);
		}
		candidates.Sort(delegate(string s0, string s1)
		{
			bool n0 = IsNative(ReadMachineType(s0));
			bool n1 = IsNative(ReadMachineType(s1));
			if (n0 != n1)
			{
				return n0 ? -1 : 1;
			}
			return string.Compare(s0, s1, StringComparison.OrdinalIgnoreCase);
		});

		return (candidates.Count > 0) ? candidates[0] : null;
	}

	private static IEnumerable<string> GetArchitectureDirectories()
	{
		ushort host = HostMachineType;
		// Native architecture first, 32-bit second (works everywhere through emulation).
		if (host == MachineArm64 || host == MachineArm)
		{
			return new string[] { "arm64", "arm", "x64", "x86" };
		}
		return new string[] { "x64", "x86" };
	}

	private static IEnumerable<string> GetVersionDirectories(string binRoot)
	{
		List<string> directories = new List<string>();
		try
		{
			foreach (string directory in Directory.GetDirectories(binRoot))
			{
				// Only "<major>.<minor>.<build>.<revision>" style version folders, never x64/x86/Win32.
				string name = Path.GetFileName(directory);
				if (Regex.IsMatch(name, "^\\d+(\\.\\d+){1,3}$"))
				{
					directories.Add(directory);
				}
			}
		}
		catch (Exception)
		{
		}
		directories.Sort(delegate(string s0, string s1)
		{
			Version v0 = ExtractTrailingVersion(s0);
			Version v1 = ExtractTrailingVersion(s1);
			int result = v1.CompareTo(v0);
			return (result != 0) ? result : string.Compare(s0, s1, StringComparison.OrdinalIgnoreCase);
		});
		return directories;
	}

	private static bool IsUsable(string candidatePath, int verbosity)
	{
		if (!File.Exists(candidatePath))
		{
			return false;
		}
		ushort machine = ReadMachineType(candidatePath);
		if (CanExecute(machine))
		{
			return true;
		}
		if (verbosity > 0)
		{
			Console.WriteLine("Ignoring '{0}' ({1} build cannot run on this machine).", candidatePath, GetMachineTypeName(machine));
		}
		return false;
	}
}
