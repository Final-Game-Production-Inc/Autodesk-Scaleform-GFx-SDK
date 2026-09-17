#!/usr/bin/perl

#C++/Makefile
#    - Unzip *.ane user extensions.
#    - Parse extension.xml for each user Extension: 
#         - get the id, ios/android lib paths & initializer/finalizer
#
#=> Android
#      - generate InitExtensions() of MainActivity.java
#      - copy FRERuntimeExtensions.jar and user jars in libs
#
#=> iOS
#     - generate XXXXXExtension.h for each user extension
#     - generate Get() in AllExtensions.h
#     - copy user libs *.a NativeExtensions/libs
#     - Parse platform.xml in iPhone-ARM folder
#     => extract linkerOptions: min iOS version, used iOS frameworks/libs

use strict;
use Getopt::Long;
use Cwd;
use XML::Simple;

use File::Spec;
use File::Find;
use File::Copy qw(copy);

use File::Basename;
my $scriptdir = dirname(__FILE__);

sub uniq {
    return keys %{{ map { $_ => 1 } @_ }};
}

# Get arguments and check them
my ($ane_path, $ane_list, $android_app_path, $ios_all_extensions_path, $ios_libraries_path);
GetOptions( "p|path:s"   => \$ane_path, "a|ane:s" => \$ane_list, "d|droid:s" => \$android_app_path, "i|ios:s" => \$ios_all_extensions_path, "l|iosLibs:s" => \$ios_libraries_path);

die "Error: Missing input parameter (-p): path to ANE directory\n" if not defined ($ane_path);
die "Error: Missing input parameter (-a): ANE extension list\n" if not defined ($ane_list);

if (not defined ($android_app_path) and (not defined ($ios_all_extensions_path) or not defined ($ios_libraries_path)))
{
	die "Error: Missing input parameter (-d or -i): paths to android or/and iOs applications\n";
}

if (! -d $ane_path)
{
    die "$ane_path does not exist\n";
}

my $current_dir = getcwd;
chdir $ane_path;

#list of extensions that exist
my @extensions = ();

#Arrays of valid extensions
my @android_extensions = ();
my @ios_extensions = ();

my @arg_extensions = split('\+', $ane_list);
foreach my $arg_extension (@arg_extensions) 
{
	if (! -e $arg_extension)
	{
		print("Warning: Extension $arg_extension not found.\n");
	}
	else
	{
		push(@extensions, $arg_extension);
	}
}

if (scalar(@extensions) == 0) 
{
	die "Error: No Native Extensions in folder \"$ane_path\", matching names \"@arg_extensions\" have been found.\n";
} 
 
foreach my $extension (@extensions) 
{
	my $out_folder = $extension;
    $out_folder =~ s/\.ane$//;
	`unzip -n $extension -d $out_folder`; #need unzip or proper zip extractor, tar or gzip won't work
	
	my $extension_folder = File::Spec->catfile($out_folder, "META-INF", "ANE");
	my $extension_xml_file = File::Spec->catfile($extension_folder, "extension.xml");
	#print("$extension_xml_file\n");

	if (-e $extension_xml_file)
	{
		my $parser = XML::Simple->new( KeepRoot => 1 );
		my $doc = $parser->XMLin($extension_xml_file);
		
		my ($extension_id, $android_lib_path, $ios_lib_path, $android_initalizer, $ios_initalizer, $ios_finalizer, $ios_basename, $ios_formatedname);
		
		$extension_id = $doc->{extension}->{id};
		$android_lib_path = $doc->{extension}->{platforms}->{platform}->{'Android-ARM'}->{applicationDeployment}->{nativeLibrary};
		$ios_lib_path = $doc->{extension}->{platforms}->{platform}->{'iPhone-ARM'}->{applicationDeployment}->{nativeLibrary};
		
		if ($android_lib_path eq '' and $ios_lib_path eq '')
		{
			#only one extension (android or ios)
			if ( $doc->{extension}->{platforms}->{platform}->{name} eq 'Android-ARM')
			{
				$android_lib_path = $doc->{extension}->{platforms}->{platform}->{applicationDeployment}->{nativeLibrary};
				$android_initalizer = $doc->{extension}->{platforms}->{platform}->{applicationDeployment}->{initializer};
			}
			elsif ($doc->{extension}->{platforms}->{platform}->{name} eq 'iPhone-ARM')
			{
				$ios_lib_path = $doc->{extension}->{platforms}->{platform}->{applicationDeployment}->{nativeLibrary};
				$ios_initalizer = $doc->{extension}->{platforms}->{platform}->{applicationDeployment}->{initializer};
				$ios_finalizer = $doc->{extension}->{platforms}->{platform}->{applicationDeployment}->{finalizer};
				($ios_basename = $ios_lib_path) =~ s/\.[^.]+$//;
                ($ios_formatedname = $ios_basename) =~ s/\W*//g;
			}
			else
			{
				die "Error: extension.xml doesn't contain any valid platform."
			}
		}
		else 
		{
			if (!$android_lib_path eq '')
			{
				$android_initalizer = $doc->{extension}->{platforms}->{platform}->{'Android-ARM'}->{applicationDeployment}->{initializer};
			}
			
			if (!$ios_lib_path eq '')
			{
				$ios_initalizer = $doc->{extension}->{platforms}->{platform}->{'iPhone-ARM'}->{applicationDeployment}->{initializer};
				$ios_finalizer = $doc->{extension}->{platforms}->{platform}->{'iPhone-ARM'}->{applicationDeployment}->{finalizer};
				($ios_basename = $ios_lib_path) =~ s/\.[^.]+$//;
                ($ios_formatedname = $ios_basename) =~ s/\W*//g;
			}
		}
		
		# print details #debug
		print ("Extension: $extension_id\n");
		if (!$android_lib_path eq '') {	print ("platform: Android-ARM, lib: $android_lib_path, initializer: $android_initalizer\n"); }	
		if (!$ios_lib_path eq '') { print ("platform: iPhone-ARM, lib: $ios_lib_path, initializer: $ios_initalizer, finalizer: $ios_finalizer\n"); }
		print ("\n");
		
		# add in the array
		if (!$android_lib_path eq '') 
		{
			my $android_extension_folder = File::Spec->catfile($extension_folder, "Android-ARM", $android_lib_path);
			push(@android_extensions, [$android_extension_folder, $extension_id, $android_initalizer] );
		}
		if (!$ios_lib_path eq '') 
		{
			my $ios_extension_folder = File::Spec->catfile($extension_folder, "iPhone-ARM", $ios_lib_path);
			push(@ios_extensions, [$ios_extension_folder, $extension_id, $ios_initalizer, $ios_finalizer, $ios_basename, "$ane_path/$out_folder", $ios_formatedname] );
		}
	}
}

chdir $current_dir;
 
#Android - generate InitExtensions() of MainActivity.java #? add this in the paramaters 
#        - copy FRERuntimeExtensions.jar and user jars in libs	

if (defined ($android_app_path))
{
	if (-d $android_app_path)
	{
		my $main_activity_path = "";
		File::Find::find({ wanted => \&find_file_callback, no_chdir => 1 }, $android_app_path);
		sub find_file_callback { $main_activity_path = $_ if -f and /[\\\/]MainActivity.java$/; }

		if (-e $main_activity_path)
		{
			print "Generating InitExtensions() in $main_activity_path\n";		
			copy "$main_activity_path", "$main_activity_path.bak";
			
			if ((open my $main_activity_out, '>', "$main_activity_path") and (open my $main_activity_file, '<', "$main_activity_path.bak"))
			{
				my $begin_ane = 0;
				while( my $line = <$main_activity_file>)  
				{   
					if ($begin_ane != 1)
					{
						print $main_activity_out $line;
					}
					
					if ($line =~ /begin ANDROID_NATIVE_EXTENSION/)
					{
						$begin_ane = 1;
						
						foreach my $android_extension (@android_extensions) 
						{
							print $main_activity_out "NativeOnLibraryInit(\"$android_extension->[1]\", \"$android_extension->[2]\");\n";
							
							#copy library to app/libs
							copy "$ane_path/$android_extension->[0]", "$android_app_path/libs";
						}
					}
					elsif ($line =~ /end ANDROID_NATIVE_EXTENSION/)
					{
						$begin_ane = 0;
						print $main_activity_out $line;
					}	
				}
				
				#close files
				close $main_activity_out;
				close $main_activity_file;
			}
			else
			{
				print "Warning: Unable to open files $main_activity_path.tmp and $main_activity_path. Skipping Android Native Extensions generation.\n";
			}
		}		
		else
		{
			print "Warning: MainActivity not found in $android_app_path. Skipping Android Native Extensions generation.\n";
		}
	}
	else
	{
		print "Warning: Path $android_app_path not found. Skipping Android Native Extensions generation.\n";

	}
}
			
	
#=> iOS
#     - generate XXXXXExtension.h for each user extension
#     - generate Get() in AllExtensions.h
#     - copy user libs *.a NativeExtensions/libs
#     - Parse platform.xml in iPhone-ARM folder
#     => extract linkerOptions: min iOS version, used iOS frameworks/libs


if (defined ($ios_all_extensions_path) and defined ($ios_libraries_path))
{
	if (-d $ios_libraries_path and -e $ios_all_extensions_path)
	{
		print "Generating $ios_all_extensions_path\n";		
		copy $ios_all_extensions_path, "$ios_all_extensions_path.bak";
			
		if ((open my $all_extensions_out, '>', "$ios_all_extensions_path") and (open my $all_extensions_file, '<', "$ios_all_extensions_path.bak"))
		{
			my $begin_include = 0;
			my $begin_ine = 0;
			
			while( my $line = <$all_extensions_file>)  
			{   
				if ($begin_include != 1 and $begin_ine != 1)
				{
					print $all_extensions_out $line;
				}
				
				if ($line =~ /begin IOS_NATIVE_EXTENSION_INCLUDE/)
				{
					$begin_include = 1;
					
					foreach my $ios_extension (@ios_extensions) 
					{
						print $all_extensions_out "#include \"$ios_libraries_path/$ios_extension->[4].h\"\n";
						
						#copy library to app/libs
						copy "$ane_path/$ios_extension->[0]", $ios_libraries_path;
					}
				}
				elsif ($line =~ /end IOS_NATIVE_EXTENSION_INCLUDE/)
				{
					$begin_include = 0;
					print $all_extensions_out $line;
				}
				elsif ($line =~ /begin IOS_NATIVE_EXTENSION/)
				{
					$begin_ine = 1;
					
					foreach my $ios_extension (@ios_extensions) 
					{
						print $all_extensions_out "freExtensionArray->PushBack( SF_NEW $ios_extension->[6]() );\n";
					}
				}
				elsif ($line =~ /end IOS_NATIVE_EXTENSION/)
				{
					$begin_ine = 0;
					print $all_extensions_out $line;
				}
			}
				
			#close files
			close $all_extensions_out;
			close $all_extensions_file;
			
			# linker options
			my @linker_options_tab = ();
		
			foreach my $ios_extension (@ios_extensions) 
			{
				#Generate extension header files
				my $header_content = "#include \"Platform/iPhone/FRERuntimeExtension.h\"\n#include \"Platform/Platform_IPhone_GL_Impl.h\"\n#include \"Platform/Platform.h\"\n\nextern \"C\" void TAG_EXTENSION_INITIALIZER (void** extDataToSet, FREContextInitializer* ctxInitializerToSet, FREContextFinalizer* ctxFinalizerToSet);\nextern \"C\" void TAG_EXTENSION_FINALIZER (FREContext ctx);\n\nnamespace Scaleform { namespace Platform {\n\nclass TAG_EXTENSION_CLASS_NAME : public IOSExtension	{\npublic:	TAG_EXTENSION_CLASS_NAME () : IOSExtension(\"TAG_EXTENSION_ID\", \"\")\n{\nFctInitializer = TAG_EXTENSION_INITIALIZER;\nFctFinalizer = TAG_EXTENSION_FINALIZER;\n}\n};\n\n}}";
				
				$header_content =~ s/\QTAG_EXTENSION_ID\E/$ios_extension->[1]/g;
				$header_content =~ s/\QTAG_EXTENSION_INITIALIZER\E/$ios_extension->[2]/g;
				$header_content =~ s/\QTAG_EXTENSION_FINALIZER\E/$ios_extension->[3]/g;
				$header_content =~ s/\QTAG_EXTENSION_CLASS_NAME\E/$ios_extension->[6]/g;

				open my $extension_header, '>', "$ios_libraries_path/$ios_extension->[4].h";
				print $extension_header $header_content;
				close $extension_header;
			
				#Generate linker options file
				my $platform_xml = File::Spec->catfile($ios_extension->[5], "META-INF", "ANE", "iPhone-ARM", "platform.xml");
				print "$platform_xml\n";
				if (-e $platform_xml)
				{
					my $parser = XML::Simple->new( KeepRoot => 1 );
					my $doc = $parser->XMLin($platform_xml);

					my $linker_options_node = $doc->{platform}->{linkerOptions};
					
					if(ref($linker_options_node->{option})) 
					{
						push(@linker_options_tab, @{$linker_options_node->{option}} );
					}
					else 
					{
						push(@linker_options_tab, $linker_options_node->{option});
					}
				}
			}
			
			open my $linker_options_file, '>', "$ios_libraries_path/linkerOptions.cfg";
			foreach ( uniq(@linker_options_tab) )
			{
				print $linker_options_file "$_\n";
			}
			close $linker_options_file;
		}
		else
		{
			print "Error: cannot open $ios_all_extensions_path for writing.";
		}
	}
	else
	{
		print "Warning: Path $android_app_path not found. Skipping Android Native Extensions generation.\n";
	}
}

chdir $current_dir		
			
			 