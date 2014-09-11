#!/usr/bin/perl -w

=encoding UTF-8

=head1 NAME

Locale::Po4a::AsciiDoc - convert AsciiDoc documents from/to PO files

=head1 DESCRIPTION

The po4a (PO for anything) project goal is to ease translations (and more
interestingly, the maintenance of translations) using gettext tools on
areas where they were not expected like documentation.

Locale::Po4a::AsciiDoc is a module to help the translation of documentation in
the AsciiDoc format.

=cut

package Locale::Po4a::AsciiDoc;

use 5.010;
use strict;
use warnings;

require Exporter;
use vars qw(@ISA @EXPORT);
@ISA = qw(Locale::Po4a::TransTractor);
@EXPORT = qw();

use Locale::Po4a::TransTractor;
use Locale::Po4a::Common;

=head1 OPTIONS ACCEPTED BY THIS MODULE

These are this module's particular options:

=over

=item B<definitions>

The name of a file containing definitions for po4a, as defined in the
B<INLINE CUSTOMIZATION> section.
You can use this option if it is not possible to put the definitions in
the document being translated.

In a definitions file, lines must not start by two slashes, but directly
by B<po4a:>.

=item B<entry>

Space-separated list of attribute entries you want to translate.  By default,
no attribute entries are translatable.

=item B<macro>

Space-separated list of macro definitions.

=item B<style>

Space-separated list of style definitions.

=back

=head1 INLINE CUSTOMIZATION

The AsciiDoc module can be customized with lines starting by B<//po4a:>.
These lines are interpreted as commands to the parser.
The following commands are recognized:

=over 4

=item B<//po4a: macro >I<name>B<[>I<attribute list>B<]>

This permits to describe in detail the parameters of a B<macro>;
I<name> must be a valid macro name, and it ends with an underscore
if the target must be translated.

The I<attribute list> argument is a comma separated list which
contains informations about translatable arguments.  This list contains
either numbers, to define positional parameters, or named attributes.

If a plus sign (B<+>) is prepended to I<name>, then the macro and its
arguments are translated as a whole.  There is no need to define
attribute list in this case, but brackets must be present.

=item B<//po4a: style >B<[>I<attribute list>B<]>

This permits to describe in detail which attributes of a style must
be translated.

The I<attribute list> argument is a comma separated list which
contains informations about translatable arguments.  This list contains
either numbers, to define positional parameters, or named attributes.
The first attribute is the style name, it will not be translated.

If a plus sign (B<+>) is prepended to the style name, then the
attribute list is translated as a whole.  There is no need to define
translatable attributes.

If a minus sign (B<->) is prepended to the style name, then this
attribute is not translated.

=item B<//po4a: entry >I<name>

This declares an attribute entry as being translatable.  By default,
they are not translated.

=back

=cut

my @comments = ();

my %debug=('split_attributelist' => 0,
           'join_attributelist'  => 0
           );

sub initialize {
    my $self = shift;
    my %options = @_;

    $self->{options}{'nobullets'} = 1;
    $self->{options}{'debug'}='';
    $self->{options}{'verbose'} = 1;
    $self->{options}{'entry'}='';
    $self->{options}{'macro'}='';
    $self->{options}{'style'}='';
    $self->{options}{'definitions'}='';

    foreach my $opt (keys %options) {
        die wrap_mod("po4a::asciidoc",
                     dgettext("po4a", "Unknown option: %s"), $opt)
            unless exists $self->{options}{$opt};
        $self->{options}{$opt} = $options{$opt};
    }

    if ($options{'debug'}) {
        foreach ($options{'debug'}) {
            $debug{$_} = 1;
        }
    }

    $self->{translate} = {
        macro => {},
        style => {},
        entry => {}
    };

    $self->register_attributelist('[verse,2,3,attribution,citetitle]');
    $self->register_attributelist('[quote,2,3,attribution,citetitle]');
    $self->register_attributelist('[icon]');
    $self->register_attributelist('[caption]');
    $self->register_attributelist('[-icons,caption]');
    $self->register_macro('image_[1,alt,title,link]');

    if ($self->{options}{'definitions'}) {
        $self->parse_definition_file($self->{options}{'definitions'})
    }
    $self->{options}{entry} =~ s/^\s*//;
    foreach my $attr (split(/\s+/, $self->{options}{entry})) {
        $self->{translate}->{entry}->{$attr} = 1;
    }
    $self->{options}{macro} =~ s/^\s*//;
    foreach my $attr (split(/\s+/, $self->{options}{macro})) {
        $self->register_macro($attr);
    }
    $self->{options}{style} =~ s/^\s*//;
    foreach my $attr (split(/\s+/, $self->{options}{style})) {
        $self->register_attributelist($attr);
    }

}

sub register_attributelist {
    my $self = shift;
    my $list = shift;
    my $type = shift || 'style';
    $list =~ s/^\[//;
    $list =~ s/\]$//;
    $list =~ s/\s+//;
    $list = ",".$list.",";
    $list =~ m/^,([-+]?)([^,]*)/;
    my $command = $2;
    $self->{translate}->{$type}->{$command} = $list;
    print STDERR "Definition: $type $command: $list\n" if $debug{definitions};
}

sub register_macro {
    my $self = shift;
    my $text = shift;
    die wrap_mod("po4a::asciidoc",
                 dgettext("po4a", "Unable to parse macro definition: %s"), $text)
            unless $text =~ m/^(\+?)([\w\d][\w\d-]*?)(_?)\[(.*)\]$/;
    my $macroplus = $1;
    my $macroname = $2;
    my $macrotarget = $3;
    my $macroparam = $macroname.",".$4;
    $self->register_attributelist($macroparam, 'macro');
    if ($macrotarget eq '_') {
        $self->{translate}->{macro}->{$macroname} .= '_';
    }
    if ($macroplus eq '+') {
        $self->{translate}->{macro}->{$macroname} =~ s/^,/,+/;
    }
}

sub is_translated_target {
    my $self = shift;
    my $macroname = shift;
    return defined($self->{translate}->{macro}->{$macroname}) &&
           $self->{translate}->{macro}->{$macroname} =~ m/_$/;
}

sub is_unsplitted_attributelist {
    my $self = shift;
    my $name = shift;
    my $type = shift;
    return defined($self->{translate}->{$type}->{$name}) &&
               $self->{translate}->{$type}->{$name} =~ m/^,\+/;
}

sub process_definition {
    my $self = shift;
    my $command = shift;
    if ($command =~ m/^po4a: macro\s+(.*\[.*\])\s*$/) {
        $self->register_macro($1);
    } elsif ($command =~ m/^po4a: style\s*(\[.*\])\s*$/) {
        $self->register_attributelist($1);
    } elsif ($command =~ m/^po4a: entry\s+(.+?)\s*$/) {
        $self->{translate}->{entry}->{$1} = 1;
    }
}

sub parse_definition_file {
    my $self = shift;
    my $filename = shift;
    if (! open (IN,"<", $filename)) {
        die wrap_mod("po4a::asciidoc",
            dgettext("po4a", "Can't open %s: %s"), $filename, $!);
    }
    while (<IN>) {
        chomp;
        process_definition($self, $_);
    }
    close IN;
}

my $RE_SECTION_TEMPLATES = "sect1|sect2|sect3|sect4|preface|colophon|dedication|synopsis|index";
my $RE_STYLE_ADMONITION = "TIP|NOTE|IMPORTANT|WARNING|CAUTION";
my $RE_STYLE_PARAGRAPH = "normal|literal|verse|quote|listing|abstract|partintro|comment|example|sidebar|source|music|latex|graphviz";
my $RE_STYLE_NUMBERING = "arabic|loweralpha|upperalpha|lowerroman|upperroman";
my $RE_STYLE_LIST = "appendix|horizontal|qanda|glossary|bibliography";
my $RE_STYLES = "$RE_SECTION_TEMPLATES|$RE_STYLE_ADMONITION|$RE_STYLE_PARAGRAPH|$RE_STYLE_NUMBERING|$RE_STYLE_LIST|float";

BEGIN {
    my $UnicodeGCString_available = 0;
    $UnicodeGCString_available = 1 if (eval { require Unicode::GCString });
    eval {
        sub columns($$$) {
            my $text = shift;
            my $encoder = shift;
            $text = $encoder->decode($text) if (defined($encoder) && $encoder->name ne "ascii");
            if ($UnicodeGCString_available) {
                return Unicode::GCString->new($text)->columns();
            } else {
                $text =~ s/\n$//s;
                return length($text) if !(defined($encoder) && $encoder->name ne "ascii");
                die wrap_mod("po4a::asciidoc",
                    dgettext("po4a", "Detection of two line titles failed at %s\nInstall the Unicode::GCString module!"), shift)
            }
        }
    };
}

sub parse {
    my $self = shift;
    my ($line,$ref);
    my $paragraph="";
    my $wrapped_mode = 1;
    ($line,$ref)=$self->shiftline();
    my $file = $ref;
    $file =~ s/:[0-9]+$// if defined($line);
    while (defined($line)) {
        $ref =~ m/^(.*):[0-9]+$/;
        if ($1 ne $file) {
            $file = $1;
            do_paragraph($self,$paragraph,$wrapped_mode);
            $paragraph="";
            $wrapped_mode = 1;
        }

        chomp($line);
        $self->{ref}="$ref";
        if ((defined $self->{verbatim}) and ($self->{verbatim} == 3)) {
            # Untranslated blocks
            $self->pushline($line."\n");
            if ($line =~ m/^~{4,}$/) {
                undef $self->{verbatim};
                undef $self->{type};
                $wrapped_mode = 1;
            }
        } elsif ((defined $self->{verbatim}) and ($self->{verbatim} == 2)) {
            # CommentBlock
            if ($line =~ m/^\/{4,}$/) {
                undef $self->{verbatim};
                undef $self->{type};
                $wrapped_mode = 1;
            } else {
                push @comments, $line;
            }
        } elsif ((not defined($self->{verbatim})) and ($line =~ m/^(\+|--)$/)) {
            # List Item Continuation or List Block
            do_paragraph($self,$paragraph,$wrapped_mode);
            $paragraph="";
            $self->pushline($line."\n");
        } elsif ((not defined($self->{verbatim})) and
                 ($line =~ m/^(={2,}|-{2,}|~{2,}|\^{2,}|\+{2,})$/) and
                 (defined($paragraph) )and
                 ($paragraph =~ m/^[^\n]*\n$/s) and
                 (columns($paragraph, $self->{TT}{po_in}{encoder}, $ref) == (length($line)))) {
            # Found title
            $wrapped_mode = 0;
            my $level = $line;
            $level =~ s/^(.).*$/$1/;
            $paragraph =~ s/\n$//s;
            my $t = $self->translate($paragraph,
                                     $self->{ref},
                                     "Title $level",
                                     "comment" => join("\n", @comments),
                                     "wrap" => 0);
            $self->pushline($t."\n");
            $paragraph="";
            @comments=();
            $wrapped_mode = 1;
            $self->pushline(($level x (columns($t, $self->{TT}{po_in}{encoder}, $ref)))."\n");
        } elsif ($line =~ m/^(={1,5})( +)(.*?)( +\1)?$/) {
            my $titlelevel1 = $1;
            my $titlespaces = $2;
            my $title = $3;
            my $titlelevel2 = $4||"";
            # Found one line title
            do_paragraph($self,$paragraph,$wrapped_mode);
            $wrapped_mode = 0;
            $paragraph="";
            my $t = $self->translate($title,
                                     $self->{ref},
                                     "Title $titlelevel1",
                                     "comment" => join("\n", @comments),
                                     "wrap" => 0);
            $self->pushline($titlelevel1.$titlespaces.$t.$titlelevel2."\n");
            @comments=();
            $wrapped_mode = 1;
        } elsif ($line =~ m/^(\/{4,}|\+{4,}|-{4,}|\.{4,}|\*{4,}|_{4,}|={4,}|~{4,}|\|={4,})$/) {
            # Found one delimited block
            my $t = $line;
            $t =~ s/^(.).*$/$1/;
            my $type = "delimited block $t";
            if (defined $self->{verbatim} and ($self->{type} ne $type)) {
                $paragraph .= "$line\n";
            } else {
                do_paragraph($self,$paragraph,$wrapped_mode);
                if (    (defined $self->{type})
                    and ($self->{type} eq $type)) {
                    undef $self->{type};
                    undef $self->{verbatim};
                    $wrapped_mode = 1;
                } else {
                    if ($t eq "\/") {
                        # CommentBlock, should not be treated
                        $self->{verbatim} = 2;
                    } elsif ($t eq "+") {
                        # PassthroughBlock
                        $wrapped_mode = 0;
                        $self->{verbatim} = 1;
                    } elsif ($t eq "-" or $t eq "|") {
                        # ListingBlock
                        $wrapped_mode = 0;
                        $self->{verbatim} = 1;
                    } elsif ($t eq ".") {
                        # LiteralBlock
                        $wrapped_mode = 0;
                        $self->{verbatim} = 1;
                    } elsif ($t eq "*") {
                        # SidebarBlock
                        $wrapped_mode = 1;
                    } elsif ($t eq "_") {
                        # QuoteBlock
                        if (    (defined $self->{type})
                            and ($self->{type} eq "verse")) {
                            $wrapped_mode = 0;
                            $self->{verbatim} = 1;
                        } else {
                            $wrapped_mode = 1;
                        }
                    } elsif ($t eq "=") {
                        # ExampleBlock
                        $wrapped_mode = 1;
                    } elsif ($t eq "~") {
                        # Filter blocks, TBC: not translated
                        $wrapped_mode = 0;
                        $self->{verbatim} = 3;
                    }
                    $self->{type} = $type;
                }
                $paragraph="";
                $self->pushline($line."\n") unless defined($self->{verbatim}) && $self->{verbatim} == 2;
            }
        } elsif ((not defined($self->{verbatim})) and ($line =~ m/^\/\/(.*)/)) {
            my $comment = $1;
            if ($comment =~ m/^po4a: /) {
                # Po4a command line
                $self->process_definition($comment);
            } else {
                # Comment line
                push @comments, $comment;
            }
        } elsif (not defined $self->{verbatim} and
                 ($line =~ m/^\[\[([^\]]*)\]\]$/)) {
            # Found BlockId
            do_paragraph($self,$paragraph,$wrapped_mode);
            $paragraph="";
            $wrapped_mode = 1;
            $self->pushline($line."\n");
            undef $self->{bullet};
            undef $self->{indent};
        } elsif (not defined $self->{verbatim} and
                 ($paragraph eq "") and
                 ($line =~ m/^((?:$RE_STYLE_ADMONITION):\s+)(.*)$/)) {
            my $type = $1;
            my $text = $2;
            do_paragraph($self,$paragraph,$wrapped_mode);
            $paragraph=$text."\n";
            $wrapped_mode = 1;
            $self->pushline($type);
            undef $self->{bullet};
            undef $self->{indent};
        } elsif (not defined $self->{verbatim} and
                 ($line =~ m/^\[($RE_STYLES)\]$/)) {
            my $type = $1;
            do_paragraph($self,$paragraph,$wrapped_mode);
            $paragraph="";
            $wrapped_mode = 1;
            $self->pushline($line."\n");
            if ($type  eq "verse") {
                $wrapped_mode = 0;
            }
            undef $self->{bullet};
            undef $self->{indent};
        } elsif (not defined $self->{verbatim} and
                 ($line =~ m/^\[.*\]$/)) {
            do_paragraph($self,$paragraph,$wrapped_mode);
            $paragraph="";
            my $t = $self->parse_style($line);
            $self->pushline("$t\n");
            @comments=();
            $wrapped_mode = 1;
            if ($line =~ m/^\[(['"]?)(verse|quote)\1,/) {
                $wrapped_mode = 0 if $2 eq 'verse';
                $self->{type} = $2;
            }
            undef $self->{bullet};
            undef $self->{indent};
        } elsif (not defined $self->{verbatim} and
                 ($line =~ m/^(\s*)([*_+`'#[:alnum:]].*)((?:::|;;|\?\?|:-)(?: *\\)?)$/)) {
            my $indent = $1;
            my $label = $2;
            my $labelend = $3;
            # Found labeled list
            do_paragraph($self,$paragraph,$wrapped_mode);
            $paragraph="";
            $wrapped_mode = 1;
            $self->{bullet} = "";
            $self->{indent} = $indent;
            my $t = $self->translate($label,
                                     $self->{ref},
                                     "Labeled list",
                                     "comment" => join("\n", @comments),
                                     "wrap" => 0);
            $self->pushline("$indent$t$labelend\n");
            @comments=();
        } elsif (not defined $self->{verbatim} and
                 ($line =~ m/^(\s*)(\S.*)((?:::|;;)\s+)(.*)$/)) {
            my $indent = $1;
            my $label = $2;
            my $labelend = $3;
            my $labeltext = $4;
            # Found Horizontal Labeled Lists
            do_paragraph($self,$paragraph,$wrapped_mode);
            $paragraph=$labeltext."\n";
            $wrapped_mode = 1;
            $self->{bullet} = "";
            $self->{indent} = $indent;
            my $t = $self->translate($label,
                                     $self->{ref},
                                     "Labeled list",
                                     "comment" => join("\n", @comments),
                                     "wrap" => 0);
            $self->pushline("$indent$t$labelend");
            @comments=();
        } elsif (not defined $self->{verbatim} and
                 ($line =~ m/^\:(\S.*?)(:\s*)(.*)$/)) {
            my $attrname = $1;
            my $attrsep = $2;
            my $attrvalue = $3;
            while ($attrvalue =~ s/ \+$//s) {
                ($line,$ref)=$self->shiftline();
                $ref =~ m/^(.*):[0-9]+$/;
                $line =~ s/^\s+//;
                $attrvalue .= $line;
            }
            # Found an Attribute entry
            do_paragraph($self,$paragraph,$wrapped_mode);
            $paragraph="";
            $wrapped_mode = 1;
            undef $self->{bullet};
            undef $self->{indent};
            if (defined($self->{translate}->{entry}->{$attrname})) {
                my $t = $self->translate($attrvalue,
                                     $self->{ref},
                                     "Attribute :$attrname:",
                                     "comment" => join("\n", @comments),
                                     "wrap" => 0);
                $self->pushline(":$attrname$attrsep$t\n");
            } else {
                $self->pushline(":$attrname$attrsep$attrvalue\n");
            }
            @comments=();
        } elsif (not defined $self->{verbatim} and
                 ($line =~ m/^([\w\d][\w\d-]*)(::)(\S+)\[(.*)\]$/)) {
            my $macroname = $1;
            my $macrotype = $2;
            my $macrotarget = $3;
            my $macroparam = $4;
            # Found a macro
            if ($macrotype eq '::') {
                do_paragraph($self,$paragraph,$wrapped_mode);
                $paragraph="";
                $wrapped_mode = 1;
                undef $self->{bullet};
                undef $self->{indent};
            }
            my $t = $self->parse_macro($macroname, $macrotype, $macrotarget, $macroparam);
            $self->pushline("$t\n");
            @comments=();
        } elsif (not defined $self->{verbatim} and
                 ($line !~ m/^\.\./) and ($line =~ m/^\.(\S.*)$/)) {
            my $title = $1;
            # Found block title
            do_paragraph($self,$paragraph,$wrapped_mode);
            $paragraph="";
            $wrapped_mode = 1;
            undef $self->{bullet};
            undef $self->{indent};
            my $t = $self->translate($title,
                                     $self->{ref},
                                     "Block title",
                                     "comment" => join("\n", @comments),
                                     "wrap" => 0);
            $self->pushline(".$t\n");
            @comments=();
        } elsif (not defined $self->{verbatim} and
                 ($line =~ m/^(\s*)((?:[-*o+]|(?:[0-9]+[.\)])|(?:[a-z][.\)])|\([0-9]+\)|\.|\.\.)\s+)(.*)$/)) {
            my $indent = $1||"";
            my $bullet = $2;
            my $text = $3;
            do_paragraph($self,$paragraph,$wrapped_mode);
            $paragraph = $text."\n";
            $self->{indent} = $indent;
            $self->{bullet} = $bullet;
        } elsif (not defined $self->{verbatim} and
                 ($line =~ m/^((?:<?[0-9]+)?> +)(.*)$/)) {
            my $bullet = $1;
            my $text = $2;
            do_paragraph($self,$paragraph,$wrapped_mode);
            $paragraph = $text."\n";
            $self->{indent} = "";
            $self->{bullet} = $bullet;
        } elsif (not defined $self->{verbatim} and
                 (defined $self->{bullet} and $line =~ m/^(\s+)(.*)$/)) {
            my $indent = $1;
            my $text = $2;
            if (not defined $self->{indent}) {
                $paragraph .= $text."\n";
                $self->{indent} = $indent;
            } elsif (length($paragraph) and (length($self->{bullet}) + length($self->{indent}) == length($indent))) {
                $paragraph .= $text."\n";
            } else {
                do_paragraph($self,$paragraph,$wrapped_mode);
                $paragraph = $text."\n";
                $self->{indent} = $indent;
                $self->{bullet} = "";
            }
        } elsif ($line =~ /^\s*$/) {
            # Break paragraphs on lines containing only spaces
            do_paragraph($self,$paragraph,$wrapped_mode);
            $paragraph="";
            $wrapped_mode = 1 unless defined($self->{verbatim});
            $self->pushline($line."\n");
            undef $self->{controlkey};
        } elsif ($line =~ /^-- $/) {
            # Break paragraphs on email signature hint
            do_paragraph($self,$paragraph,$wrapped_mode);
            $paragraph="";
            $wrapped_mode = 1;
            $self->pushline($line."\n");
        } elsif (   $line =~ /^=+$/
                 or $line =~ /^_+$/
                 or $line =~ /^-+$/) {
            $wrapped_mode = 0;
            $paragraph .= $line."\n";
            do_paragraph($self,$paragraph,$wrapped_mode);
            $paragraph="";
            $wrapped_mode = 1;
        } else {
            if ($line =~ /^\s/) {
                # A line starting by a space indicates a non-wrap
                # paragraph
                $wrapped_mode = 0;
            }
            undef $self->{bullet};
            undef $self->{indent};
    # TODO: comments
            $paragraph .= $line."\n";
        }
        # paragraphs starting by a bullet, or numbered
        # or paragraphs with a line containing many consecutive spaces
        # (more than 3)
        # are considered as verbatim paragraphs
        $wrapped_mode = 0 if (   $paragraph =~ m/^(\*|[0-9]+[.)] )/s
                          or $paragraph =~ m/[ \t][ \t][ \t]/s);
        ($line,$ref)=$self->shiftline();
    }
    if (length $paragraph) {
        do_paragraph($self,$paragraph,$wrapped_mode);
    }
}

sub do_paragraph {
    my ($self, $paragraph, $wrap) = (shift, shift, shift);
    my $type = shift || $self->{type} || "Plain text";
    return if ($paragraph eq "");

# DEBUG
#    my $b;
#    if (defined $self->{bullet}) {
#            $b = $self->{bullet};
#    } else {
#            $b = "UNDEF";
#    }
#    $type .= " verbatim: '".($self->{verbatim}||"NONE")."' bullet: '$b' indent: '".($self->{indent}||"NONE")."' type: '".($self->{type}||"NONE")."'";

    if (not $wrap and not defined $self->{verbatim}) {
        # Detect bullets
        # |        * blah blah
        # |<spaces>  blah
        # |          ^-- aligned
        # <empty line>
        #
        # Other bullets supported:
        # - blah         o blah         + blah
        # 1. blah       1) blah       (1) blah
TEST_BULLET:
        if ($paragraph =~ m/^(\s*)((?:[-*o+]|([0-9]+[.\)])|\([0-9]+\))\s+)([^\n]*\n)(.*)$/s) {
            my $para = $5;
            my $bullet = $2;
            my $indent1 = $1;
            my $indent2 = "$1".(' ' x length $bullet);
            my $text = $4;
            while ($para !~ m/$indent2(?:[-*o+]|([0-9]+[.\)])|\([0-9]+\))\s+/
                   and $para =~ s/^$indent2(\S[^\n]*\n)//s) {
                $text .= $1;
            }
            # TODO: detect if a line starts with the same bullet
            if ($text !~ m/\S[ \t][ \t][ \t]+\S/s) {
                my $bullet_regex = quotemeta($indent1.$bullet);
                $bullet_regex =~ s/[0-9]+/\\d\+/;
                if ($para eq '' or $para =~ m/^$bullet_regex\S/s) {
                    my $trans = $self->translate($text,
                                                 $self->{ref},
                                                 "Bullet: '$indent1$bullet'",
                                                 "wrap" => 1,
                                                 "wrapcol" => - (length $indent2));
                    $trans =~ s/^/$indent1$bullet/s;
                    $trans =~ s/\n(.)/\n$indent2$1/sg;
                    $self->pushline( $trans."\n" );
                    if ($para eq '') {
                        return;
                    } else {
                        # Another bullet
                        $paragraph = $para;
                        goto TEST_BULLET;
                    }
                }
            }
        }
    }

    my $end = "";
    if ($wrap) {
        $paragraph =~ s/^(.*?)(\n*)$/$1/s;
        $end = $2 || "";
    }
    my $t = $self->translate($paragraph,
                             $self->{ref},
                             $type,
                             "comment" => join("\n", @comments),
                             "wrap" => $wrap);
    @comments = ();
    if (defined $self->{bullet}) {
        my $bullet = $self->{bullet};
        my $indent1 = $self->{indent};
        my $indent2 = $indent1.(' ' x length($bullet));
        $t =~ s/^/$indent1$bullet/s;
        $t =~ s/\n(.)/\n$indent2$1/sg;
    }
    $self->pushline( $t.$end );
}

sub parse_style {
    my ($self, $text) = (shift, shift);
    $text =~ s/^\[//;
    $text =~ s/\]$//;
    $text =~ m/^([^=,]+)/;
    if (defined($1) && $self->is_unsplitted_attributelist($1, 'style')) {
       my $t = $self->translate($text,
                        $self->{ref},
                        "Unsplitted AttributeList",
                        "comment" => join("\n", @comments),
                        "wrap" => 0);
       return "[$t]";
    }
    my @attributes = $self->split_attributelist($text);
    return "[".join(", ", $self->join_attributelist("style", @attributes))."]";
}

sub parse_macro {
    my ($self, $macroname, $macrotype, $macrotarget, $macroparam) = (shift, shift, shift, shift, shift);
    if ($self->is_unsplitted_attributelist($macroname, 'macro')) {
        my $t = $self->translate("$macroname$macrotype$macrotarget\[$macroparam\]",
                         $self->{ref},
                         "Unsplitted macro call",
                         "comment" => join("\n", @comments),
                         "wrap" => 0);
        return $t;
    }
    my @attributes = $self->split_attributelist($macroparam);
    unshift @attributes, $macroname;
    my @translated_attributes = $self->join_attributelist("macro", @attributes);
    shift @translated_attributes;
    if ($self->is_translated_target($macroname)) {
        my $target = unquote_space($macrotarget);
        my $t = $self->translate($target,
                         $self->{ref},
                         "Target for macro $macroname",
                         "comment" => join("\n", @comments),
                         "wrap" => 0);
        $macrotarget = quote_space($t);
    }
    return "$macroname$macrotype$macrotarget\[".join(", ", @translated_attributes)."]";
}

sub split_attributelist {
    my ($self, $text) = (shift, shift);

    print STDERR "Splitting attributes in: $text\n" if $debug{split_attributelist};
    my @attributes = ();
    while ($text =~ m/\G(
         [^\W\d][-\w]*="(?:[^"\\]++|\\.)*+" # named attribute
       | [^\W\d][-\w]*=None                 # undefined named attribute
       | [^\W\d][-\w]*=\S+                  # invalid, but accept it anyway
       | "(?:[^"\\]++|\\.)*+"               # quoted attribute
       |  (?:[^,\\]++|\\.)++                # unquoted attribute
         )(?:,\s*+)?/gx) {
        print STDERR "  -> $1\n" if $debug{split_attributelist};
        push @attributes, $1;
    }
    die wrap_mod("po4a::asciidoc",
                 dgettext("po4a", "Unable to parse attribute list: [%s]"), $text)
            unless scalar(@attributes);
    return @attributes;
}

sub join_attributelist {
    my ($self, $type) = (shift, shift);
    my @attributes = @_;
    my $command = shift(@attributes);
    my $position = 1;
    my @text = ($command);
    if ($command =~ m/=/) {
        my $attr = $command;
        $command =~ s/=.*//;
        @text = ();
        push @text, $self->translate_attributelist($type, $command, $position, $attr);
    }
    foreach my $attr (@attributes) {
        $position++;
        push @text, $self->translate_attributelist($type, $command, $position, $attr);
    }
    print STDERR "Joined attributes: ".join(", ", @text)."\n" if $debug{join_attributelist};
    return @text;
}

sub translate_attributelist {
    my ($self, $type, $command, $count, $attr) = (shift, shift, shift, shift, shift);
    return $attr unless defined $self->{translate}->{$type}->{$command};
    if ($attr =~ m/^([^\W\d][-\w]*)=(.*)/) {
        my $attrname = $1;
        my $attrvalue = $2;
        if ($self->{translate}->{$type}->{$command} =~ m/,$attrname,/) {
            my $value = unquote($attrvalue);
            my $t = $self->translate($value,
                             $self->{ref},
                             "Named '$attrname' AttributeList argument for $type '$command'",
                             "comment" => join("\n", @comments),
                             "wrap" => 0);
            if ($attrvalue eq 'None' && $t eq 'None') {
                $attr = $attrname."=None";
            } else {
                $attr = $attrname."=".quote($t);
            }
        }
    } else {
        if ($self->{translate}->{$type}->{$command} =~ m/,$count,/) {
            my $attrvalue = unquote($attr);
            my $t = $self->translate($attrvalue,
                             $self->{ref},
                             "Positional (\$$count) AttributeList argument for $type '$command'",
                             "comment" => join("\n", @comments),
                             "wrap" => 0);
            $attr = quote($t);
        }
    }
    return $attr;
}

sub unquote {
    my ($text) = shift;
    return $text unless $text =~ s/^"(.*)"$/$1/;
    $text =~ s/\\"/"/g;
    return $text;
}

sub quote {
    my $text = shift;
    $text =~ s/"/\\"/g;
    return '"'.$text.'"';
}

sub quote_space {
    my $text = shift;
    $text =~ s/ /%20/g;
    return $text;
}

sub unquote_space {
    my $text = shift;
    $text =~ s/%20/ /g;
    return $text;
}

1;

=head1 STATUS OF THIS MODULE

Tested successfully on simple AsciiDoc files.

=head1 AUTHORS

 Nicolas François <nicolas.francois@centraliens.net>
 Denis Barbier <barbier@linuxfr.org>

=head1 COPYRIGHT AND LICENSE

 Copyright 2005-2008 by Nicolas FRANÇOIS <nicolas.francois@centraliens.net>.
 Copyright 2012 by Denis BARBIER <barbier@linuxfr.org>.

This program is free software; you may redistribute it and/or modify it
under the terms of GPL (see the COPYING file).
