#!/usr/bin/perl -w

# Copyright (c) 2004-2008 by Nicolas FRANÇOIS <nicolas.francois@centraliens.net>
#
# This file is part of po4a.
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
# along with po4a; if not, write to the Free Software
# Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#
########################################################################

=encoding UTF-8

=head1 NAME

Locale::Po4a::Halibut - convert Halibut documents and derivates from/to PO files

=head1 DESCRIPTION

The po4a (PO for anything) project goal is to ease translations (and more
interestingly, the maintenance of translations) using gettext tools on
areas where they were not expected like documentation.

Locale::Po4a::Halibut is a module to help the translation of Halibut documents into
other [human] languages.

This module contains the definitions of common Halibut commands and
environments.

=head1 STATUS OF THIS MODULE

This module is still beta.
Please send feedback and feature requests.

=head1 CAVEAT

Some constructs are badly supported. The known ones are documented below.

=head2 Verbatim blocks

  \c foo
  \c bar

The verbatim block is not considered as a whole. Each line will be
translated separately.

=head1 SEE ALSO

L<Locale::Po4a::TeX(3pm)|Locale::Po4a::TeX>,
L<Locale::Po4a::TransTractor(3pm)|Locale::Po4a::TransTractor>,
L<po4a(7)|po4a.7>

=head1 AUTHORS

 Nicolas François <nicolas.francois@centraliens.net>

=head1 COPYRIGHT AND LICENSE

Copyright 2004-2008 by Nicolas FRANÇOIS <nicolas.francois@centraliens.net>.

This program is free software; you may redistribute it and/or modify it
under the terms of GPL (see COPYING file).

=cut

package Locale::Po4a::Halibut;

use 5.006;
use strict;
use warnings;

require Exporter;
use vars qw($VERSION @ISA @EXPORT);
$VERSION= $Locale::Po4a::TeX::VERSION;
@ISA= qw(Locale::Po4a::TeX);
@EXPORT= qw();

use Locale::Po4a::Common;
use Locale::Po4a::TeX;
use subs qw(&parse_definition_file
            &register_generic_command &is_closed &translate_buffer
            &register_verbatim_environment
            &generic_command
            &in_verbatim
            &get_leading_command);
*parse_definition_file         = \&Locale::Po4a::TeX::parse_definition_file;
*get_leading_command           = \&Locale::Po4a::TeX::get_leading_command;
*register_generic_command      = \&Locale::Po4a::TeX::register_generic_command;
*register_verbatim_environment = \&Locale::Po4a::TeX::register_verbatim_environment;
*generic_command               = \&Locale::Po4a::TeX::generic_command;
*is_closed                     = \&Locale::Po4a::TeX::is_closed;
*in_verbatim                   = \&Locale::Po4a::TeX::in_verbatim;
*translate_buffer              = \&Locale::Po4a::TeX::translate_buffer;
use vars qw($RE_ESCAPE            $ESCAPE
            $RE_VERBATIM
            $RE_COMMENT           $RE_PRE_COMMENT
            $no_wrap_environments $separated_commands
            %commands             %environments
            %command_categories   %separated
            %env_separators       %debug
            %translate_buffer_env
            @exclude_include      @comments);
*RE_ESCAPE             = \$Locale::Po4a::TeX::RE_ESCAPE;
*ESCAPE                = \$Locale::Po4a::TeX::ESCAPE;
*RE_VERBATIM           = \$Locale::Po4a::TeX::RE_VERBATIM;
*RE_COMMENT            = \$Locale::Po4a::TeX::RE_COMMENT;
*RE_PRE_COMMENT        = \$Locale::Po4a::TeX::RE_PRE_COMMENT;
*no_wrap_environments  = \$Locale::Po4a::TeX::no_wrap_environments;
*separated_commands    = \$Locale::Po4a::TeX::separated_commands;
*commands              = \%Locale::Po4a::TeX::commands;
*environments          = \%Locale::Po4a::TeX::environments;
*command_categories    = \%Locale::Po4a::TeX::command_categories;
*separated             = \%Locale::Po4a::TeX::separated;
*env_separators        = \%Locale::Po4a::TeX::env_separators;
*debug                 = \%Locale::Po4a::TeX::debug;
*translate_buffer_env  = \%Locale::Po4a::TeX::translate_buffer_env;
*exclude_include       = \@Locale::Po4a::TeX::exclude_include;
*comments              = \@Locale::Po4a::TeX::comments;

#$ESCAPE = "\\";
#$RE_ESCAPE = "\\\\";
#$RE_VERBATIM = "\@example";
$RE_VERBATIM = "PO4A_FAKE_VERBATIM";
#$RE_COMMENT = "\\\@(?:c|comment)\\b";
$RE_COMMENT = "PO4A_FAKE_COMMENT";

sub docheader {
    return "\\# This file was generated with po4a. Translate the source file.\n".
           "\n";
}

my %break_line = ();

# translate_line_command indicate if the arguments to the command handled
# by line_command() should be translated:
# undefined: arguments are not translated
# 0:         there should be no arguments
# 1:         arguments should be translated
my %translate_line_command = ();

sub parse {
    my $self = shift;
    my ($line,$ref);
    my $paragraph = ""; # Buffer where we put the paragraph while building
    my @env = (); # environment stack
    my $t = "";
#    $docheader_pushed = 0;

  LINE:
    undef $self->{type};
    ($line,$ref)=$self->shiftline();

    while (defined($line)) {
        chomp($line);
        $self->{ref}="$ref";

        if ($line =~ /^\s*\\\s*po4a\s*:/) {
            parse_definition_line($self, $line);
            goto LINE;
        }

        my $t;
        ($paragraph, $t, @env) = parse_line($self, $line, $paragraph, \@env);
        $self->pushline($t);


        # Reinit the loop
        ($line,$ref)=$self->shiftline();
        undef $self->{type};
    }

    if (length($paragraph)) {
        ($t, @env) = translate_buffer($self,$paragraph,undef,@env);
        $self->pushline($t);
        $paragraph="";
    }
} # end of parse

sub parse_line {
    my $self = shift;
    my $line = shift;
    my $paragraph = shift;
    my $env = shift;
    my @e = @$env;
    my $translated = "";

    my $closed = 1;
    if (!in_verbatim(@e)) {
        $closed = is_closed($paragraph);
    }
#        if (not $closed) {
#            print "not closed. line: '$line'\n            para: '$paragraph'\n";
#        }

#warn "closed'$closed'$line'$paragraph'\n";
    if ($closed and $line =~ /^\s*$/) {
        # An empty line. This indicates the end of the current
        # paragraph.
        $paragraph .= $line."\n";
        if (length($paragraph)) {
            ($translated, @e) = translate_buffer($self,$paragraph,undef,@e);
            $paragraph="";
        }
    } elsif ($line =~ m/^\\input /) {
        if (length($paragraph)) {
            ($translated, @e) = translate_buffer($self,$paragraph,undef,@e);
            $paragraph="";
        }
        $translated .= $line."\n";
    } elsif ($line =~ m/^$RE_COMMENT/) {
        $translated = $line."\n";
    } elsif (    $closed
             and (is_closed($line) or $line =~ /^\\[ce] /)
             and ($line =~ /^\\([^ ]*?)( +.*)?$/)) {
        my ($command,$variant,$args,$buffer);
        if ($break_line{$1}) {
            my @a = ();
            $variant = "";
            $args = \@a;
            $command = $1;
            $buffer = $2||"";
        } else {
            ($command,$variant,$args,$buffer) = get_leading_command($self, $line);
        }
        if (    $break_line{$command}
            and not (    ($command eq "c" or $command eq "e")
                     and defined $args->[0])) {
# NOTE: This is just a workaround: "\c " is a verbatim line
#       and \c{...} is just a verbatim block
            my $t;
            if (length($paragraph)) {
                ($t, @e) = translate_buffer($self,$paragraph,undef,@e);
                $translated .= $t;
                $paragraph="";
            }
            ($t, @e) = generic_command($self, $command, $variant, $args, \@e);
            $translated .= $t;

            my $arg = $buffer;
            my @args = ();
            if (defined $arg and length $arg) {
                # FIXME: keep the spaces ?
                $arg =~ s/\s*$//s;
                @args= (" ", $arg);
            }
            ($t, @e) = line_command($self, $command, "", \@args, \@e, 1);
            $translated .= $t."\n";
        } else {
            # continue the same paragraph
            $paragraph .= $line."\n";
        }
    } else {
        # continue the same paragraph
        $paragraph .= $line."\n";
    }

    return ($paragraph, $translated, @e);
}

sub line_command {
    my $self = shift;
    my ($command,$variant,$args,$env) = (shift,shift,shift,shift);
    my $no_wrap = shift;
    print "line_command($command,$variant,@$args,@$env,$no_wrap)="
        if ($debug{'commands'});

    my $translated = ""; # $ESCAPE.$command;
    my $line = $args->[1];
#warn "line_command: '$line'\n";
    if (defined $line and length $line) {
        if (    defined $translate_line_command{$command}
            and $translate_line_command{$command}) {
            # $no_wrap could be forced to 1, but it should already be set
            $no_wrap = 1;
            $line =~ s/^(\s*)//;
            my $spaces = $1 || "";
            my ($t,$e) = $self->translate_buffer($line,$no_wrap,@$env,$command);
#warn "line_command: '$t'\n";
            $translated .= $spaces.$t;
        } else {
            $translated .= $line;
        }
    }
    print "($translated,@$env)\n"
        if ($debug{'commands'});
    return ($translated,@$env);
}


# 3.2 Simple inline formatting commands
# 3.2.1 `\e': Emphasising text
# inline. extract only if alone
register_generic_command("-e,{_}");
$translate_line_command{e} = 1;
$break_line{e} = 1;
# 3.2.2 `\c' and `\cw': Displaying computer code inline
# inline. extract only if alone
# NOTE: \c and \c{...} differs.
#       \c is marked as a break_line command, but this is reversed in
#       parse_line when the \c{...} form is used.
register_generic_command("-c,{_}");
$translate_line_command{c} = 1;
$break_line{c} = 1;
register_generic_command("-cw,{_}");
# 3.2.3 `\q': Quotation marks
# inline. extract only if alone
register_generic_command("-q,{_}");
# 3.2.4 `\-' and `\_': Non-breaking hyphens and spaces
# inline.

# 3.2.5 `\date': Automatic date generation
# inline.

# 3.2.6 `\W': WWW hyperlinks
# inline. extract only if alone
register_generic_command("-W,{_}");
# 3.2.7 `\u': Specifying arbitrary Unicode characters
# inline.

# 3.2.8 `\k' and `\K': Cross-references to other sections
# inline. They should not be translated. extract only if alone
# FIXME: it will expand to "Section ..." or "section ..."
#        Section and section should be translated.
register_generic_command("-k,{}");
register_generic_command("-K,{}");
# 3.2.9 `\#': Inline comments
# inline. But can be removed from the head or tail.
register_generic_command("-#,{}");
$translate_line_command{"#"} = 0;
$break_line{"#"} = 1;
# 3.3 Paragraph-level commands
# 3.3.1 `\c': Displaying whole paragraphs of computer code
# see above
# 3.3.2 `\b', `\n', `\dt', `\dd', `\lcont': Lists
register_generic_command("*b,");
register_generic_command("*n,"); # FIXME: \n{this-one} not supported?
register_generic_command("*dd,");
register_generic_command("*dt,");
# 3.3.2.4 Continuing list items into further paragraphs
register_generic_command("*lcont,{_}"); # registered, but redefined
$commands{lcont} = sub {
    my $self = shift;
    my ($command,$variant,$args,$env) = (shift,shift,shift,shift);
    my $no_wrap = shift;
    my ($t,@e)=("",@$env);
    my $translated = $ESCAPE.$command.$variant."{";
    my $text = $args->[1];
    my $paragraph = "";
    while (   $text =~ s/^(.*?)\n(.*)$/$2/s
           or $text =~ s/^([^\n]+)$//s) {
        ($paragraph, $t, @e) = parse_line($self, $1, $paragraph, \@e);
        $translated .= $t;
    }
    ($t, @e) = translate_buffer ($self, $paragraph, $no_wrap, @e);
    $translated .= $t;
    $translated .= "}";

    return ($translated, @$env);
};
# 3.3.3 `\rule': Horizontal rules
register_generic_command("rule,"); # TODO: TBC does it break paragraphs
# 3.3.4 `\quote': Indenting multiple paragraphs as a long quotation
register_generic_command("*quote,{_}"); # TODO: TBC
# 3.3.5 `\C', `\H', `\S', `\A', `\U': Chapter and section headings
# FIXME: What happens if the the line is rewrapped?
# NOTE: The name of the section is not translated.
register_generic_command("*C,{}");
register_generic_command("*S0,{}"); # Synonym for \H
register_generic_command("*H,{}");
register_generic_command("*S,{}");
register_generic_command("*S1,{}"); # Synonym for \S
register_generic_command("*S2,{}");
register_generic_command("*S3,{}"); # FIXME: and so on
# FIXME: \S{question-about-fish}{Question}
register_generic_command("*A,{}");
register_generic_command("*U,{}");
# 3.3.6 `\copyright', `\title', `\versionid': Miscellaneous blurb commands
register_generic_command("*title,");
register_generic_command("*copyright,");
register_generic_command("*versionid,");
# 3.4 Creating a bibliography
# nocite
register_generic_command("*nocite,{}");
# B
register_generic_command("*B,{}");
# BR
register_generic_command("*BR,{}"); # FIXME: \BR{freds-book} [Fred1993]
# 3.5 Creating an index
# 3.5.1 Simple indexing
# \i: inline \i{index} or \i\x{grep}
# \ii
register_generic_command("-ii,{_}");
# \IM: inline. Variable number of arguments
register_generic_command("*IM,{_}");
$translate_line_command{IM} = 1;
$break_line{IM} = 1;
# 3.6 Configuring Halibut
# \cfg
register_generic_command("+cfg,{}{_}"); # NOTE: the new command is not registered
# 3.7 Defining macros
register_generic_command("*define,{}"); # FIXME: line
$translate_line_command{define} = 1;
$break_line{define} = 1;

1;
