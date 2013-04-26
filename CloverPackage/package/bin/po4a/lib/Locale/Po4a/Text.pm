#!/usr/bin/perl -w

# Po4a::Text.pm
#
# extract and translate translatable strings from a text documents
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#
########################################################################

=encoding UTF-8

=head1 NAME

Locale::Po4a::Text - convert text documents from/to PO files

=head1 DESCRIPTION

The po4a (PO for anything) project goal is to ease translations (and more
interestingly, the maintenance of translations) using gettext tools on
areas where they were not expected like documentation.

Locale::Po4a::Text is a module to help the translation of text documents into
other [human] languages.

Paragraphs are split on empty lines (or lines containing only spaces or
tabulations).

If a paragraph contains a line starting by a space (or tabulation), this
paragraph won't be rewrapped.

=cut

package Locale::Po4a::Text;

use 5.006;
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

=item B<nobullets>

Deactivate detection of bullets.

By default, when a bullet is detected, the bullet paragraph is not considered
as a verbatim paragraph (with the no-wrap flag in the PO file), but the module
rewraps this paragraph in the generated PO file and in the translation.

=cut

my $bullets = 1;

=item B<tabs=>I<mode>

Specify how tabulations shall be handled. The I<mode> can be any of:

=over

=item B<split>

Lines with tabulations introduce breaks in the current paragraph.

=item B<verbatim>

Paragraph containing tabulations will not be re-wrapped.

=back

By default, tabulations are considered as spaces.

=cut

my $tabs = "";

=item B<breaks=>I<regex>

A regular expression matching lines which introduce breaks.
The regular expression will be anchored so that the whole line must match.

=cut

my $breaks;

=item B<debianchangelog>

Handle the header and footer of
released versions, which only contain non translatable informations.

=cut

my $debianchangelog = 0;

=item B<fortunes>

Handle the fortunes format, which separate fortunes with a line which
consists in '%' or '%%', and use '%%' as the beginning of a comment.

=cut

my $fortunes = 0;

=item B<markdown>

Handle some special markup in Markdown-formatted texts.

=cut

my $markdown = 0;

=item B<asciidoc>

Handle documents in the AsciiDoc format.

=cut

my $asciidoc = 0;

=item B<control>[B<=>I<taglist>]

Handle control files.
A comma-separated list of tags to be translated can be provided.

=cut

my %control = ();

my $parse_func = \&parse_fallback;

my @comments = ();

=back

=cut

sub initialize {
    my $self = shift;
    my %options = @_;

    $self->{options}{'control'} = "";
    $self->{options}{'asciidoc'} = 1;
    $self->{options}{'breaks'} = 1;
    $self->{options}{'debianchangelog'} = 1;
    $self->{options}{'debug'} = 1;
    $self->{options}{'fortunes'} = 1;
    $self->{options}{'markdown'} = 1;
    $self->{options}{'nobullets'} = 1;
    $self->{options}{'tabs'} = 1;
    $self->{options}{'verbose'} = 1;

    foreach my $opt (keys %options) {
        die wrap_mod("po4a::text",
                     dgettext("po4a", "Unknown option: %s"), $opt)
            unless exists $self->{options}{$opt};
        $self->{options}{$opt} = $options{$opt};
    }

    if (defined $options{'nobullets'}) {
        $bullets = 0;
    }

    if (defined $options{'tabs'}) {
        $tabs = $options{'tabs'};
    }

    if (defined $options{'breaks'}) {
        $breaks = $options{'breaks'};
    }

    if (defined $options{'debianchangelog'}) {
        $parse_func = \&parse_debianchangelog;
    }

    if (defined $options{'fortunes'}) {
        $parse_func = \&parse_fortunes;
    }

    if (defined $options{'markdown'}) {
        $parse_func = \&parse_markdown;
        $markdown=1;
    }

    if (defined $options{'asciidoc'}) {
        $parse_func = \&parse_asciidoc;
        $asciidoc=1;
    }

    if (defined $options{'control'}) {
        $parse_func = \&parse_control;
        if ($options{'control'} eq "1") {
            $control{''}=1;
        } else {
            foreach my $tag (split(',',$options{'control'})) {
                $control{$tag}=1;
            }
        }
    }
}

sub parse_fallback {
    my ($self,$line,$ref,$paragraph,$wrapped_mode,$expect_header,$end_of_paragraph) = @_;
    if (   ($line =~ /^\s*$/)
             or (    defined $breaks
                 and $line =~ m/^$breaks$/)) {
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
    } elsif ($tabs eq "split" and $line =~ m/\t/ and $paragraph !~ m/\t/s) {
        $wrapped_mode = 0;
        do_paragraph($self,$paragraph,$wrapped_mode);
        $paragraph = "$line\n";
        $wrapped_mode = 0;
    } elsif ($tabs eq "split" and $line !~ m/\t/ and $paragraph =~ m/\t/s) {
        do_paragraph($self,$paragraph,$wrapped_mode);
        $paragraph = "$line\n";
        $wrapped_mode = 1;
    } else {
        if ($line =~ /^\s/) {
            # A line starting by a space indicates a non-wrap
            # paragraph
            $wrapped_mode = 0;
        }
        if ($markdown and
                 (   $line =~ /\S  $/    # explicit newline
                  or $line =~ /"""$/)) { # """ textblock inside macro begin
            # Markdown markup needing separation _after_ this line
            $end_of_paragraph = 1;
        } else {
            undef $self->{bullet};
            undef $self->{indent};
        }
# TODO: comments
        $paragraph .= $line."\n";
    }
    return ($paragraph,$wrapped_mode,$expect_header,$end_of_paragraph);
}

sub parse_debianchangelog {
    my ($self,$line,$ref,$paragraph,$wrapped_mode,$expect_header,$end_of_paragraph) = @_;
    if ($expect_header and
        $line =~ /^(\w[-+0-9a-z.]*)\ \(([^\(\) \t]+)\) # src, version
                   \s+([-+0-9a-z.]+);                 # distribution
                   \s*urgency\s*\=\s*(.*\S)\s*$/ix) { #
        do_paragraph($self,$paragraph,$wrapped_mode);
        $paragraph="";
        $self->pushline("$line\n");
        $expect_header=0;
    } elsif ($line =~ m/^ \-\- (.*) <(.*)>  ((\w+\,\s*)?\d{1,2}\s+\w+\s+\d{4}\s+\d{1,2}:\d\d:\d\d\s+[-+]\d{4}(\s+\([^\\\(\)]\))?)$/) {
        # Found trailer
        do_paragraph($self,$paragraph,$wrapped_mode);
        $paragraph="";
        $self->pushline("$line\n");
        $expect_header=1;
    } else {
        return parse_fallback($self,$line,$ref,$paragraph,$wrapped_mode,$expect_header,$end_of_paragraph);
    }
    return ($paragraph,$wrapped_mode,$expect_header,$end_of_paragraph);
}

sub parse_fortunes {
    my ($self,$line,$ref,$paragraph,$wrapped_mode,$expect_header,$end_of_paragraph) = @_;
    if ($line =~ m/^%%?\s*$/) {
        # Found end of fortune
        do_paragraph($self,$paragraph,$wrapped_mode);
        $self->pushline("\n") unless (   $wrapped_mode == 0
                                  or $paragraph eq "");
        $paragraph="";
        $wrapped_mode = 1;
        $self->pushline("$line\n");
    } else {
        $line =~ s/%%(.*)$//;
    }
    return ($paragraph,$wrapped_mode,$expect_header,$end_of_paragraph);
}

sub parse_control {
    my ($self,$line,$ref,$paragraph,$wrapped_mode,$expect_header,$end_of_paragraph) = @_;
    if ($line =~ m/^([^ :]*): *(.*)$/) {
        warn "Unrecognized section: '$paragraph'\n"
            unless $paragraph eq "";
        my $tag = $1;
        my $val = $2;
        my $t;
        if ($control{''} or $control{$tag}) {
            $t = $self->translate($val,
                                  $self->{ref},
                                  $tag.(defined $self->{controlkey}?", ".$self->{controlkey}:""),
                                  "wrap" => 0);
        } else {
            $t = $val;
        }
        if (not defined $self->{controlkey}) {
            $self->{controlkey} = "$tag: $val";
        }
        $self->pushline("$tag: $t\n");
        $paragraph="";
        $wrapped_mode = 1;
        $self->{bullet} = "";
        $self->{indent} = " ";
    } elsif ($line eq " .") {
        do_paragraph($self,$paragraph,$wrapped_mode,
                     "Long Description".(defined $self->{controlkey}?", ".$self->{controlkey}:""));
        $paragraph="";
        $self->pushline($line."\n");
        $self->{bullet} = "";
        $self->{indent} = " ";
    } elsif ($line =~ m/^ Link: +(.*)$/) {
        do_paragraph($self,$paragraph,$wrapped_mode,
                     "Long Description".(defined $self->{controlkey}?", ".$self->{controlkey}:""));
        my $link=$1;
        my $t1 = $self->translate("Link: ",
                                  $self->{ref},
                                  "Link",
                                  "wrap" => 0);
        my $t2 = $self->translate($link,
                                  $self->{ref},
                                  "Link".(defined $self->{controlkey}?", ".$self->{controlkey}:""),
                                  "wrap" => 0);
        $self->pushline(" $t1$t2\n");
        $paragraph="";
    } elsif (defined $self->{indent} and
             $line =~ m/^$self->{indent}\S/) {
        $paragraph .= $line."\n";
        $self->{type} = "Long Description".(defined $self->{controlkey}?", ".$self->{controlkey}:"");
    } else {
        return parse_fallback($self,$line,$ref,$paragraph,$wrapped_mode,$expect_header,$end_of_paragraph);
    }
    return ($paragraph,$wrapped_mode,$expect_header,$end_of_paragraph);
}

my $asciidoc_RE_SECTION_TEMPLATES = "sect1|sect2|sect3|sect4|preface|colophon|dedication|synopsis|index";
my $asciidoc_RE_STYLE_ADMONITION = "TIP|NOTE|IMPORTANT|WARNING|CAUTION";
my $asciidoc_RE_STYLE_PARAGRAPH = "normal|literal|verse|quote|listing|abstract|partintro|comment|example|sidebar|source|music|latex|graphviz";
my $asciidoc_RE_STYLE_NUMBERING = "arabic|loweralpha|upperalpha|lowerroman|upperroman";
my $asciidoc_RE_STYLE_LIST = "appendix|horizontal|qanda|glossary|bibliography";
my $asciidoc_RE_STYLES = "$asciidoc_RE_SECTION_TEMPLATES|$asciidoc_RE_STYLE_ADMONITION|$asciidoc_RE_STYLE_PARAGRAPH|$asciidoc_RE_STYLE_NUMBERING|$asciidoc_RE_STYLE_LIST|float";

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
                die wrap_mod("po4a::text",
                    dgettext("po4a", "Detection of two line titles failed at %s\nInstall the Unicode::GCString module!"), shift)
            }
        }
    };
}

sub parse_asciidoc {
    my ($self,$line,$ref,$paragraph,$wrapped_mode,$expect_header,$end_of_paragraph) = @_;
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
        # Comment line
        push @comments, $1;
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
             ($line =~ m/^((?:$asciidoc_RE_STYLE_ADMONITION):\s+)(.*)$/)) {
        my $type = $1;
        my $text = $2;
        do_paragraph($self,$paragraph,$wrapped_mode);
        $paragraph=$text."\n";
        $wrapped_mode = 1;
        $self->pushline($type);
        undef $self->{bullet};
        undef $self->{indent};
    } elsif (not defined $self->{verbatim} and
             ($line =~ m/^\[($asciidoc_RE_STYLES)\]$/)) {
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
             ($line =~ m/^\[(['"]?)(verse|quote)\1, +(.*)\]$/)) {
        my $quote = $1 || '';
        my $type = $2;
        my $arg = $3;
        do_paragraph($self,$paragraph,$wrapped_mode);
        $paragraph="";
        my $t = $self->translate($arg,
                                 $self->{ref},
                                 "$type",
                                 "comment" => join("\n", @comments),
                                 "wrap" => 0);
        $self->pushline("[$quote$type$quote, $t]\n");
        @comments=();
        $wrapped_mode = 1;
        if ($type  eq "verse") {
            $wrapped_mode = 0;
        }
        $self->{type} = $type;
        undef $self->{bullet};
        undef $self->{indent};
    } elsif (not defined $self->{verbatim} and
             ($line =~ m/^\[icon="(.*)"\]$/)) {
        my $arg = $1;
        do_paragraph($self,$paragraph,$wrapped_mode);
        $paragraph="";
        my $t = $self->translate($arg,
                                 $self->{ref},
                                 "icon",
                                 "comment" => join("\n", @comments),
                                 "wrap" => 0);
        $self->pushline("[icon=\"$t\"]\n");
        @comments=();
        $wrapped_mode = 1;
        undef $self->{bullet};
        undef $self->{indent};
    } elsif (not defined $self->{verbatim} and
             ($line =~ m/^\[icons=None, +caption="(.*)"\]$/)) {
        my $arg = $1;
        do_paragraph($self,$paragraph,$wrapped_mode);
        $paragraph="";
        my $t = $self->translate($arg,
                                 $self->{ref},
                                 "caption",
                                 "comment" => join("\n", @comments),
                                 "wrap" => 0);
        $self->pushline("[icons=None, caption=\"$t\"]\n");
        @comments=();
        $wrapped_mode = 1;
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
        # Found a Attribute entry
        do_paragraph($self,$paragraph,$wrapped_mode);
        $paragraph="";
        $wrapped_mode = 1;
        undef $self->{bullet};
        undef $self->{indent};
        my $t = $self->translate($attrvalue,
                                 $self->{ref},
                                 "Attribute :$attrname:",
                                 "comment" => join("\n", @comments),
                                 "wrap" => 0);
        $self->pushline(":$attrname$attrsep$t\n");
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
    } else {
        return parse_fallback($self,$line,$ref,$paragraph,$wrapped_mode,$expect_header,$end_of_paragraph);
    }
    return ($paragraph,$wrapped_mode,$expect_header,$end_of_paragraph);
}

sub parse_markdown {
    my ($self,$line,$ref,$paragraph,$wrapped_mode,$expect_header,$end_of_paragraph) = @_;
    if (($line =~ m/^(={4,}|-{4,})$/) and
        (defined($paragraph) ) and
        ($paragraph =~ m/^[^\n]*\n$/s) and
        (length($paragraph) == (length($line)+1))) {
        # XXX: There can be any number of underlining according
        #      to the documentation. This detection, which avoid
        #      translating the formatting, is only supported if
        #      the underlining has the same size as the header text.
        # Found title
        $wrapped_mode = 0;
        my $level = $line;
        $level =~ s/^(.).*$/$1/;
        my $t = $self->translate($paragraph,
                                 $self->{ref},
                                 "Title $level",
                                 "wrap" => 0);
        $self->pushline($t);
        $paragraph="";
        $wrapped_mode = 1;
        $self->pushline(($level x (length($t)-1))."\n");
    } elsif ($line =~ m/^(#{1,6})( +)(.*?)( +\1)?$/) {
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
                                 "wrap" => 0);
        $self->pushline($titlelevel1.$titlespaces.$t.$titlelevel2."\n");
        $wrapped_mode = 1;
    } elsif (($paragraph eq "") and
             ($line =~ /^((\*\s*){3,}|(-\s*){3,}|(_\s*){3,})$/)) {
        # Horizontal rule
        $wrapped_mode = 1;
        $self->pushline($line."\n");
    } elsif (   $line =~ /^\s*\[\[\!\S+\s*$/     # macro begin
             or $line =~ /^\s*"""\s*\]\]\s*$/) { # """ textblock inside macro end
        # Avoid translating Markdown lines containing only markup
        do_paragraph($self,$paragraph,$wrapped_mode);
        $paragraph="";
        $wrapped_mode = 1;
        $self->pushline("$line\n");
    } elsif (    $line =~ /^#/            # headline
              or $line =~ /^\s*\[\[\!\S[^\]]*\]\]\s*$/) { # sole macro
        # Preserve some Markdown markup as a single line
        do_paragraph($self,$paragraph,$wrapped_mode);
        $paragraph="$line\n";
        $wrapped_mode = 0;
        $end_of_paragraph = 1;
    } elsif ($line =~ /^"""/) { # """ textblock inside macro end
        # Markdown markup needing separation _before_ this line
        do_paragraph($self,$paragraph,$wrapped_mode);
        $paragraph="$line\n";
        $wrapped_mode = 1;
    } else {
        return parse_fallback($self,$line,$ref,$paragraph,$wrapped_mode,$expect_header,$end_of_paragraph);
    }
    return ($paragraph,$wrapped_mode,$expect_header,$end_of_paragraph);
}

sub parse {
    my $self = shift;
    my ($line,$ref);
    my $paragraph="";
    my $wrapped_mode = 1;
    my $expect_header = 1;
    my $end_of_paragraph = 0;
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
            $expect_header = 1;
        }

        chomp($line);
        $self->{ref}="$ref";
        ($paragraph,$wrapped_mode,$expect_header,$end_of_paragraph) = &$parse_func($self,$line,$ref,$paragraph,$wrapped_mode,$expect_header,$end_of_paragraph);
        # paragraphs starting by a bullet, or numbered
        # or paragraphs with a line containing many consecutive spaces
        # (more than 3)
        # are considered as verbatim paragraphs
        $wrapped_mode = 0 if (   $paragraph =~ m/^(\*|[0-9]+[.)] )/s
                          or $paragraph =~ m/[ \t][ \t][ \t]/s);
        $wrapped_mode = 0 if (    $tabs eq "verbatim"
                              and $paragraph =~ m/\t/s);
        if ($markdown) {
            # Some Markdown markup can (or might) not survive wrapping
            $wrapped_mode = 0 if (
                   $paragraph =~ /^>/ms                  # blockquote
                or $paragraph =~ /^( {8}|\t)/ms          # monospaced
                or $paragraph =~ /^\$(\S+[{}]\S*\s*)+/ms # Xapian macro
                or $paragraph =~ /<(?![a-z]+[:@])/ms     # maybe html (tags but not wiki <URI>)
                or $paragraph =~ /^[^<]+>/ms             # maybe html (tag with vertical space)
                or $paragraph =~ /\S  $/ms               # explicit newline
                or $paragraph =~ /\[\[\!\S[^\]]+$/ms     # macro begin
            );
        }
        if ($end_of_paragraph) {
            do_paragraph($self,$paragraph,$wrapped_mode);
            $paragraph="";
            $wrapped_mode = 1;
            $end_of_paragraph = 0;
        }
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

    if ($bullets and not $wrap and not defined $self->{verbatim}) {
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

1;

=head1 STATUS OF THIS MODULE

Tested successfully on simple text files and NEWS.Debian files.

=head1 AUTHORS

 Nicolas François <nicolas.francois@centraliens.net>

=head1 COPYRIGHT AND LICENSE

 Copyright 2005-2008 by Nicolas FRANÇOIS <nicolas.francois@centraliens.net>.

This program is free software; you may redistribute it and/or modify it
under the terms of GPL (see the COPYING file).
