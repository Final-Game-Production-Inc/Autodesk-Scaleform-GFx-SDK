#!/usr/bin/perl

use strict;

my %extfns = ();
my %decls = ();
my $ver = '';
my $glexth = '/usr/include/GL/glext.h';
my $usep4 = 1;

# Command line parameters.
for (my $i = 0; $i <= $#ARGV; ) 
{
    if ($ARGV[$i] eq '-v')
    {
        $ver = $ARGV[$i+1] ;
        $i += 2;
    }
    elsif ($ARGV[$i] eq '-h') 
    {
        $glexth = $ARGV[$i+1];
        $i += 2;
    }
    elsif ($ARGV[$i] eq '-nop4' )
    {
        $usep4 = 0;
        $i++;
    }
    else
    {
        die "Unknown argument: " . $ARGV[$i]. "\n" .
            "Usage: Extensions.pl [-v <VERSION>] [-h <GLEXT_HEADER>] [-nop4]\n\n" .
            "<VERSION>      can be one of: ES10, ES20. Defaults to empty (OpenGL).\n" .
            "<GLEXT_HEADER> Points to the glext.h extension header (defaults to $glexth).\n" .
            "-nop4          Will disable check out the modified files from perforce before writing.\n";
    }
}

# Scan the extensions file.
my $exttxt = "Extensions$ver.txt";
open(EXTS, "<$exttxt") or die "Couldn't open the extensions list at: $exttxt: $!\n";
while (<EXTS>) 
{
    # Skip comment or empty lines.
    if (/^\s*(#|;)/ || /^\s*$/ )
    {
        next;
    }
    
    # Read the definitions.
    if (/(\?)?\s*(!)?\s*(gl\w+)(?:\s+(\w+\s*)*)/) 
    {
        my ($optional, $required, $function, $defines) = ($1, $2, $3, $4);
        $extfns{$3}->{'defined'} = 1;
        if ($1 eq "?")
        {
            if ($4 ne undef)
            {
                my @defines = split(/\s+/, $4);
                $extfns{$3}->{'extensionDefine'} = \@defines;
            }
            $extfns{$3}->{'optional'} = 1;
        }
        if ($2 eq "!")
        {
            $extfns{$3}->{'canfail'} = 1;
        }
    }
    else
    {
        print "line did not match extension definition: $_\n";
    }
}
close(EXTS);

# Make sure glext.h exists.
my $glexthOriginal = $glexth;
unless (-f $glexth) 
{
    $glexth = "../../../3rdParty/glext/glext.h";
}
-f $glexth or die "glext.h could not be located (tried $glexthOriginal and $glexth).\n";

# Scan glext.h
open(GLEXT, "<$glexth");
while (<GLEXT>) 
{   
    if (/(?:GLAPI|GL_APICALL\s+)?(.*?)\s*(?:(?:GL_)?APIENTRY\s+)?(gl\w+) *\((.*)\)/) 
    {
        if ($extfns{$2}) 
        {
            my ($ret,$name,$proto) = ($1,$2,$3);
			print "Found extension: $name\n";
            print "\tReturn type = $ret\n";
            print "\tPrototype   = $proto\n";
            
            # Split the prototype into individual parameters, and strip off the identifier, if it exists.
            my @protoparams = split(/\s*,\s*/, $proto);
            foreach my $param (@protoparams)
            {
                $param =~ s/((const\s*)?(\w+)(\s*\*(const\s+)?){0,2}).*$/\1/;
            }
            $decls{$name} = [$ret,@protoparams];
        }
    }
}
close(GLEXT);

# Open the extension files for writing.
my ($exth, $extmacroh, $extcpp ) = ("GL${ver}_Extensions.h", "GL${ver}_ExtensionMacros.h", "GL${ver}_Extensions.cpp");

if ( $usep4 )
{
    foreach my $f (@_)
    {
        `p4 edit $f`;
    }
}

open(OUTH, ">$exth") or die "Couldn't open $exth: $!\n";
open(OUTC, ">$extcpp") or die "Couldn't open $extcpp: $!\n";
open(OUTM, ">$extmacroh") or die "Couldn't open $extmacroh: $!\n";

print OUTH <<"EOF";
/**********************************************************************

Filename    :   $exth
Content     :   GL $ver extension header.
Created     :   Automatically generated.
Authors     :   Extensions.pl

Copyright   :   (c) 2001-2011 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

EOF
print OUTH "#ifndef INC_SF_Render_GL$ver". "_Extensions_H\n";
print OUTH "#define INC_SF_Render_GL$ver". "_Extensions_H\n";

print OUTH "\nstatic unsigned SF_GL_VERBOSE_EXTENSION_PRINT = 0;   // Set this to non-zero to have usage of all extension functions print in the debug output.\n";

print OUTH <<"EOF";

#include "Kernel/SF_Debug.h"
#include "Render/GL/GL_Common.h"

namespace Scaleform { namespace Render { namespace GL {

class Extensions
{
EOF

print OUTC <<"EOF";
/**********************************************************************

Filename    :   $extcpp
Content     :   GL $ver extension implementation.
Created     :   Automatically generated.
Authors     :   Extensions.pl

Copyright   :   (c) 2001-2011 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#include "Render/GL/GL_Common.h"

#ifdef SF_GL_RUNTIME_LINK

namespace Scaleform { namespace Render { namespace GL {

bool Extensions::Init()
{
    bool result = 1;
EOF

print OUTM <<"EOF";
/**********************************************************************

Filename    :   $extmacroh
Content     :   GL $ver extension macros header.
Created     :   Automatically generated.
Authors     :   Extensions.pl

Copyright   :   (c) 2001-2011 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

EOF
print OUTM "#ifndef INC_SF_Render_GL$ver". "_ExtensionMacros_H\n";
print OUTM "#define INC_SF_Render_GL$ver". "_ExtensionMacros_H\n";

if ( $ver eq 'ES11' )
{
    # GLES 1.0 has a different HAL than other GL versions.
    print OUTM "#include \"Render/GL/GLES11_HAL.h\"\n\n#ifdef SF_GL_RUNTIME_LINK\n\n";
}
else
{
    print OUTM "#include \"Render/GL/GL_HAL.h\"\n\n#ifdef SF_GL_RUNTIME_LINK\n\n";
}

my $maxlen = 0;
foreach my $fn (keys %extfns) 
{
    $maxlen = length($fn) if (length($fn) > $maxlen);
}
$maxlen += 7;

my @calls = ();
my $OptionalMacros = "";

foreach my $fn (keys %extfns) 
{
  die "function $fn not defined" unless $decls{$fn};
  my @args = (@{$decls{$fn}});
  my $ret = shift @args;
  my @pargs = ();
  my @params = ();

  if ("@args" ne 'void') {
    for (my $i = 0; $i <= $#args; $i++) {
      push @pargs, $args[$i] . " a$i";
      push @params, "a$i";
    }
  }

    my $pfn = uc("PFN${fn}PROC");

    # Setup the preprocessor define that determines whether the function prototype/function pointer typedef will exist.
    my $extIfDef = undef;
    if ($extfns{$fn}->{'extensionDefine'} ne undef) 
    { 
        $extIfDef = "#if " . (join " && ", map { "defined(" . $_ . ")" } @{$extfns{$fn}->{'extensionDefine'}});
    }

    if ($extIfDef ne undef)
    {
        print OUTH "    $extIfDef\n";
    }
    printf OUTH "    %-${maxlen}s p_$fn;\n", $pfn;
    if ($extIfDef ne undef)
    {
        print OUTH "    #endif\n";
    }
    
    
    if ($extIfDef ne undef)
    {
        print OUTM "    $extIfDef\n";
    }
    print OUTM "    #define $fn GetHAL()->$fn\n";
    if ($extIfDef ne undef)
    {
        print OUTM "    #endif\n";
    }

    my $c = sprintf "    %s %s(%s)\n    {\n", $ret, $fn, join(', ', @pargs);
    if ( $extfns{$fn}->{'canfail'} == undef )
    {
        $c .= "        ScopedGLErrorCheck check(__FUNCTION__);\n";
    }
    
    my %argTypePrintfFormat = (
        GLenum  => "%d",
        GLint   => "%d",
        GLsizei => "%d",
        GLuint  => "%x",
        GLfloat => "%.2f",
        GLboolean => "%d",
        GLchar    => "%d",
		GLbitfield => "0x%08x",
        GLptr     => "0x%p"
    );
    
    if ($#pargs >= 0)
    {
        $c .= "        SF_DEBUG_MESSAGE" . ($#pargs+1) . "(SF_GL_VERBOSE_EXTENSION_PRINT, \"$fn(";
    }
    else
    {
        $c .= "        SF_DEBUG_MESSAGE (SF_GL_VERBOSE_EXTENSION_PRINT, \"$fn(";
    }
    
    my @pargsTemp = @pargs;
    my $argListString = "";
    for (my $i = 0; $i < $#pargs+1; $i++)
    {
        my $argType = $pargs[$i];
        $argType =~ s/\s+([^\s]+)$//;
        my $argName = $1;
        if ($argType =~ /\*/)
        {
            $argType = "GLptr"; # special case.
        }
        if ($argTypePrintfFormat{$argType} eq undef)
        {
            # Just print it as a ptr if we can't guess the type, it's probably right.
            $argType = "GLptr"; # special case.                        
        }
        if ($i != 0)
        {
            $c .= ", ";
        }
        $c .= $argTypePrintfFormat{$argType};
        if ($argType eq "GLptr")
        {
            $argListString .= ", reinterpret_cast<const void*>($argName)";
        }
        else
        {
            $argListString .= ", $argName";
        }
    }
    if ($#pargs >= 0)
    {
        $c .= ")\\n\" $argListString );\n";
    }
    else
    {
        $c .= "\\n\");\n";        
    }
            

    if ($extIfDef ne undef)
    {
        $c .= "        $extIfDef\n";
    }    
    if ($ret ne 'void')
    {
        $c .= "        return p_$fn(" . join(', ', @params). ");\n";
    }
    else
    {
        $c .= "        p_$fn(".join(', ', @params).");\n";
    }
    if ($extIfDef ne undef)
    {
        $c .= "        #else\n";
        $c .= "        SF_UNUSED" . ($#params + 1) . "(" . join(', ', @params) . ");\n";
        $c .= "        SF_DEBUG_ASSERT(1, \"glext.h did not contain required preprocessor defines to \"\n" .
              "                           \"use the $fn extension function (" . $extIfDef .")\");\n";
        if ($ret ne 'void')
        {
        $c .= "        return ($ret)(0);\n";
        }
        $c .= "        #endif\n";
    }
    $c .="    }\n\n";
    
    push @calls, $c;

    my $reqValue = "true";
    if ( $extfns{$fn}->{'optional'} != undef )
    {
        $reqValue = "false";
    }
    
    if ($extIfDef ne undef)
    {
        print OUTC "    $extIfDef\n";
    }
    print OUTC "    p_$fn = ($pfn) SF_GL_RUNTIME_LINK(\"$fn\");\n";
    if (!($fn =~ /(?:EXT|ARB|OES)$/)) 
    {
        print OUTC <<"EOF";
        if (!p_$fn)
        {
             p_$fn = ($pfn) SF_GL_RUNTIME_LINK(\"${fn}ARB\");
             if (!p_$fn)
                  p_$fn = ($pfn) SF_GL_RUNTIME_LINK(\"${fn}EXT\");
        }
EOF
    }
print OUTC <<"EOF";    
    if (!p_$fn && $reqValue)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: $fn\\n");
        result = 0;
    }
EOF
    
      
    
      
    if ($extIfDef ne undef)
    {
        print OUTC "    #endif\n";
    }    
    print OUTC "\n";
}

print OUTH << "EOF";
public:
    bool Init();
    
    class ScopedGLErrorCheck
    {
    public:
        ScopedGLErrorCheck(const char* func) : FuncName(func)
        {
            performErrorCheck("before");
        }
        ~ScopedGLErrorCheck()
        {
            performErrorCheck("after");
        }
    private:
        void performErrorCheck(const char* timing)
        {
            SF_UNUSED(timing);
            #ifdef SF_BUILD_DEBUG
                GLenum error = glGetError(); SF_UNUSED(error);
                static const char* errorMessages[] = {
                    "GL_INVALID_ENUM",
                    "GL_INVALID_VALUE",
                    "GL_INVALID_OPERATION",
                    "GL_STACK_OVERFLOW",
                    "GL_STACK_UNDERFLOW",
                    "GL_OUT_OF_MEMORY" };
                if (error >= GL_INVALID_ENUM && error <= GL_OUT_OF_MEMORY)
                {
                    SF_DEBUG_ASSERT3(!error, "GL error %s function %s (error code = %s)\\n", timing, FuncName, errorMessages[error-GL_INVALID_ENUM] );
                }
                else
                {
                    SF_DEBUG_ASSERT3(!error, "GL error %s function %s (non-standard error code = %d)\\n", timing, FuncName, error );
                }
            #endif
        }
        const char * FuncName;
    };
EOF

foreach (@calls) {
  print OUTH;
}

print OUTH "};\n\n}}}\n#endif\n";
print OUTC "    return result;\n}\n\n}}}\n\n#endif\n";
print OUTM "\n#else\n$OptionalMacros\n#endif\n#endif\n";
close(OUTH);
close(OUTC);
close(OUTM);
