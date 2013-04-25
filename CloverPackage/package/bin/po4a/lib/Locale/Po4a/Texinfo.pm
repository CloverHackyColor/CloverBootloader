#!/usr/bin/perl -w

# Copyright (c) 2004-2007 by Nicolas FRANÇOIS <nicolas.francois@centraliens.net>
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

Locale::Po4a::Texinfo - convert Texinfo documents and derivates from/to PO files

=head1 DESCRIPTION

The po4a (PO for anything) project goal is to ease translations (and more
interestingly, the maintenance of translations) using gettext tools on
areas where they were not expected like documentation.

Locale::Po4a::Texinfo is a module to help the translation of Texinfo documents into
other [human] languages.

This module contains the definitions of common Texinfo commands and
environments.

=head1 STATUS OF THIS MODULE

This module is still beta.
Please send feedback and feature requests.

=head1 SEE ALSO

L<Locale::Po4a::TeX(3pm)|Locale::Po4a::TeX>,
L<Locale::Po4a::TransTractor(3pm)|Locale::Po4a::TransTractor>,
L<po4a(7)|po4a.7>

=head1 AUTHORS

 Nicolas François <nicolas.francois@centraliens.net>

=head1 COPYRIGHT AND LICENSE

Copyright 2004-2007 by Nicolas FRANÇOIS <nicolas.francois@centraliens.net>.

This program is free software; you may redistribute it and/or modify it
under the terms of GPL (see COPYING file).

=cut

package Locale::Po4a::Texinfo;

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
use subs qw(
            &parse_definition_file
            &register_generic_command &is_closed &translate_buffer
            &register_verbatim_environment
            &generic_command
            &in_verbatim);
*parse_definition_file         = \&Locale::Po4a::TeX::parse_definition_file;
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

$ESCAPE = "\@";
$RE_ESCAPE = "\@";
$RE_VERBATIM = "\@example";
$RE_COMMENT = "\\\@(?:c|comment)\\b";
$RE_PRE_COMMENT = "(?<!\@)(?:\@\@)*";

my %break_line = ();

# translate_line_command indicate if the arguments to the command handled
# by line_command() should be translated:
# undefined: arguments are not translated
# 0:         there should be no arguments
# 1:         arguments should be translated
my %translate_line_command = ();

foreach (qw/example smallexample tex display smalldisplay verbatim format smallformat
            flushleft flushright lisp smalllisp ignore/) {
    register_verbatim_environment($_);
    $commands{$_} = \&environment_line_command;
    $translate_line_command{$_} = 0; # There should be no arguments
    $break_line{$_} = 1;
}

my $docheader_pushed = 0;
# The header shall not be written before the Texinfo header (which include
# the \input command that define the texinfo macros)
sub docheader {
    return "";
}

sub push_docheader {
    return if $docheader_pushed;
    my $self = shift;
    $self->pushline(<<END);
\@c ===========================================================================
\@c
\@c This file was generated with po4a. Translate the source file.
\@c
\@c ===========================================================================
END
    $docheader_pushed = 1;
}

sub parse {
    my $self = shift;
    my ($line,$ref);
    my $paragraph = ""; # Buffer where we put the paragraph while building
    my @env = (); # environment stack
    my $t = "";
    $docheader_pushed = 0;

  LINE:
    undef $self->{type};
    ($line,$ref)=$self->shiftline();

    while (defined($line)) {
        chomp($line);
        $self->{ref}="$ref";

        if ($line =~ /^\s*@\s*po4a\s*:/) {
            parse_definition_line($self, $line);
            goto LINE;
        }

        my $closed = 1;
        if (!in_verbatim(@env)) {
            $closed = is_closed($paragraph);
        }
#        if (not $closed) {
#            print "not closed. line: '$line'\n            para: '$paragraph'\n";
#        }

        if ($closed and $line =~ /^\s*$/) {
            # An empty line. This indicates the end of the current
            # paragraph.
            $paragraph .= $line."\n";
            if (length($paragraph)) {
                ($t, @env) = translate_buffer($self,$paragraph,undef,@env);
                $self->pushline($t);
                $paragraph="";
            }
        } elsif ($line =~ m/^\\input /) {
            if (length($paragraph)) {
                ($t, @env) = translate_buffer($self,$paragraph,undef,@env);
                $self->pushline($t);
                $paragraph="";
            }
            $self->pushline($line."\n");
            $self->push_docheader();
        } elsif ($line =~ m/^$RE_COMMENT/) {
            $self->push_docheader();
            $self->pushline($line."\n");
        } elsif (    $closed
                 and ($line =~ /^@([^ ]*?)(?: +(.*))?$/)
                 and (defined $commands{$1})
                 and ($break_line{$1})) {
            if (length($paragraph)) {
                ($t, @env) = translate_buffer($self,$paragraph,undef,@env);
                $self->pushline($t);
                $paragraph="";
            }
            my $arg = $2;
            my @args = ();
            if (defined $arg and length $arg) {
                # FIXME: keep the spaces ?
                $arg =~ s/\s*$//s;
                @args= (" ", $arg);
            }
            ($t, @env) = &{$commands{$1}}($self, $1, "", \@args, \@env, 1);
            $self->pushline($t."\n");
        } else {
            # continue the same paragraph
            $paragraph .= $line."\n";
        }

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

sub line_command {
    my $self = shift;
    my ($command,$variant,$args,$env) = (shift,shift,shift,shift);
    my $no_wrap = shift;
    print "line_command($command,$variant,@$args,@$env,$no_wrap)="
        if ($debug{'commands'});

    my $translated = $ESCAPE.$command;
    my $line = $args->[1];
    if (defined $line and length $line) {
        if (    defined $translate_line_command{$command}
            and $translate_line_command{$command}) {
            # $no_wrap could be forced to 1, but it should already be set
            my ($t,$e) = $self->translate_buffer($line,$no_wrap,@$env,$command);
            $translated .= " ".$t;
        } else {
            $translated .= " ".$line;
        }
    }
    print "($translated,@$env)\n"
        if ($debug{'commands'});
    return ($translated,@$env);
}

sub defindex_line_command {
    my $self = shift;
    my ($command,$variant,$args,$env) = (shift,shift,shift,shift);
    my $no_wrap = shift;
    print "line_command($command,$variant,@$args,@$env,$no_wrap)="
        if ($debug{'commands'});
    my $idx = $$args[1]."index";
    $commands{$idx} = \&line_command;
    $break_line{$idx} = 1;
    $translate_line_command{$idx} = 1;

    return line_command($self,$command,$variant,$args,$env,$no_wrap);
}

sub translate_buffer_menu {
    my ($self,$buffer,$no_wrap,@env) = (shift,shift,shift,@_);
    print STDERR "translate_buffer_menu($buffer,$no_wrap,@env)="
        if ($debug{'translate_buffer'});

    my $translated_buffer = "";
    my $spaces = "";
    if ($buffer =~ m/(\s*)$/s) {
        $spaces = $1;
    }


    while ($buffer =~ m/^(.*?)((?:\n|^)\* )(.*)$/s) {
        my $sep = $2;
        $buffer = $3;
        my($t, @e) = $self->translate_buffer_menuentry($1, $no_wrap,
                                                       @env, "menuentry");
        $translated_buffer .= $t.$sep;
    }
    my($t, @e) = $self->translate_buffer_menuentry($buffer, $no_wrap,
                                                   @env, "menuentry");
    $translated_buffer .= $t;

    $translated_buffer .= $spaces;

    print STDERR "($translated_buffer,@env)\n"
        if ($debug{'translate_buffer'});
    return ($translated_buffer,@env);
}
$translate_buffer_env{"menu"}       = \&translate_buffer_menu;
$translate_buffer_env{"detailmenu"} = \&translate_buffer_menu;
$translate_buffer_env{"direntry"}   = \&translate_buffer_menu;

my $menu_width = 78;
my $menu_sep_width = 30;
sub translate_buffer_menuentry {
    my ($self,$buffer,$no_wrap,@env) = (shift,shift,shift,@_);
    print STDERR "translate_buffer_menuentry($buffer,$no_wrap,@env)="
        if ($debug{'translate_buffer'});

    my $translated_buffer = "";

    if (   $buffer =~ m/^(.*?)(::)\s+(.*)$/s
        or $buffer =~ m/^(.*?: .*?)(\.)\s+(.*)$/s) {
        my ($name, $sep, $description) = ($1, $2, $3);
        my ($t, @e) = $self->translate_buffer($name, $no_wrap, @env);
        $translated_buffer = $t.$sep."  ";
        my $l = length($translated_buffer) + 2;
        if ($l < $menu_sep_width-1) {
            $translated_buffer .= ' 'x($menu_sep_width-1-$l);
            $l = $menu_sep_width-1;
        }
        ($t, @e) = $self->translate_buffer($description, $no_wrap, @env);
        $t =~ s/\n//sg;
        $t = Locale::Po4a::Po::wrap($t, $menu_width-$l-2);
        my $spaces = ' 'x($l+2);
        $t =~ s/\n/\n$spaces/sg;
        $translated_buffer .= $t;
    } else {
# FIXME: no-wrap if a line start by a space
        my ($t, @e) = $self->translate_buffer($buffer, $no_wrap, @env);
        $translated_buffer = $t;
    }

    print STDERR "($translated_buffer,@env)\n"
        if ($debug{'translate_buffer'});
    return ($translated_buffer,@env);
}

sub translate_buffer_ignore {
    my ($self,$buffer,$no_wrap,@env) = (shift,shift,shift,@_);
    print STDERR "translate_buffer_ignore($buffer,$no_wrap,@env);\n"
        if ($debug{'translate_buffer'});
    return ($buffer,@env);
}
$translate_buffer_env{"ignore"} = \&translate_buffer_ignore;

foreach (qw(appendix section cindex findex kindex opindex pindex vindex subsection
            dircategory subtitle include
            exdent center unnumberedsec
            heading unnumbered unnumberedsubsec
            unnumberedsubsubsec appendixsec appendixsubsec
            appendixsubsubsec majorheading chapheading subheading
            subsubheading shorttitlepage
            subsubsection top item itemx chapter settitle
            title author)) {
    $commands{$_} = \&line_command;
    $break_line{$_} = 1;
    $translate_line_command{$_} = 1;
}
foreach (qw(c comment clear set setfilename setchapternewpage vskip synindex
            syncodeindex need fonttextsize printindex headings finalout sp
            definfoenclose)) {
    $commands{$_} = \&line_command;
    $break_line{$_} = 1;
}
foreach (qw(defcodeindex defindex)) {
    $commands{$_} = \&defindex_line_command;
    $break_line{$_} = 1;
}
# definfoenclose: command definition => translate?
foreach (qw(insertcopying page bye summarycontents shortcontents contents
            noindent)) {
    $commands{$_} = \&line_command;
    $break_line{$_} = 1;
    $translate_line_command{$_} = 0;
}

foreach (qw(defcv deffn
            defivar defmac defmethod defop
            defopt defspec deftp deftypecv
            deftypefn deftypefun
            deftypeivar deftypemethod
            deftypeop deftypevar deftypevr
            defun defvar defvr)) {
    $commands{$_} = \&environment_line_command;
    $translate_line_command{$_} = 1;
    $break_line{$_} = 1;
}
foreach (qw(defcvx deffnx defivarx defmacx defmethodx defopx defoptx
            defspecx deftpx deftypecvx deftypefnx deftypefunx deftypeivarx
            deftypemethodx deftypeopx deftypevarx deftypevrx defunx
            defvarx defvrx)) {
    $commands{$_} = \&line_command;
    $translate_line_command{$_} = 1;
    $break_line{$_} = 1;
}

foreach (qw(titlefont w i r b sansserif sc slanted strong t cite email
            footnote indicateurl emph ref xref pxref inforef kbd key
            acronym),
# The following commands could cause problems since their arguments
# have a semantic and a translator could decide not to translate code but
# still translate theses short words if they appear in another context.
         qw(file command dfn dmn option math code samp var)) {
    register_generic_command("-$_,{_}");
}

register_generic_command("*anchor,{_}");
register_generic_command("*refill,");

$translate_line_command{'node'} = 1;
$no_wrap_environments .= " node";
$break_line{'node'} = 1;
# @node     Comments,  Minimum, Conventions, Overview
$commands{'node'} = sub {
    my $self = shift;
    my ($command,$variant,$args,$env) = (shift,shift,shift,shift);
    my $no_wrap = shift;
    print "node($command,$variant,@$args,@$env,$no_wrap)="
        if ($debug{'commands'});

    my $translated = $ESCAPE.$command;
    my $line = $args->[1];
    if (defined $line and length $line) {
        my @pointers = split (/, */, $line);
        my @t;
        foreach (@pointers) {
            push @t, $self->translate($_, $self->{ref}, $command, "wrap" => 0);
        }
        $translated .= " ".join(", ", @t);
    }

    print "($translated,@$env)\n"
        if ($debug{'commands'});
    return ($translated,@$env);
};

sub environment_command {
    my $self = shift;
    my ($command,$variant,$args,$env) = (shift,shift,shift,shift);
    my $no_wrap = shift;
    print "environment_command($command,$variant,@$args,@$env,$no_wrap)="
        if ($debug{'commands'});
    my ($t,@e)=("",());

    ($t, @e) = generic_command($self,$command,$variant,$args,$env,$no_wrap);
    @e = (@$env, $command);

    print "($t,@e)\n"
        if ($debug{'commands'});
    return ($t,@e);
}

sub environment_line_command {
    my $self = shift;
    my ($command,$variant,$args,$env) = (shift,shift,shift,shift);
    my $no_wrap = shift;
    print "environment_command_line($command,$variant,@$args,@$env,$no_wrap)="
        if ($debug{'commands'});
    my ($t,@e)=("",());

    ($t, @e) = line_command($self,$command,$variant,$args,$env,$no_wrap);
    @e = (@$env, $command);

    print "($t,@e)\n"
        if ($debug{'commands'});
    return ($t,@e);
}

## push the environment in the environment stack, and do not translate
## the command
#sub push_environment {
#    my $self = shift;
#    my ($command,$variant,$args,$env) = (shift,shift,shift,shift);
#    print "push_environment($command,$variant,@$args,@$env)="
#        if ($debug{'environments'});
#
#    my ($t,@e) = generic_command($self,$command,$variant,$args,$env);
#
#    print "($t,@e)\n"
#        if ($debug{'environments'});
#    return ($t,@e);
#}
#
foreach (qw(detailmenu menu titlepage group copying
            documentdescription cartouche
            direntry
            ifdocbook ifhtml ifinfo ifplaintext iftex ifxml
            ifnotdocbook ifnothtml ifnotinfo ifnotplaintext ifnottex ifnotxml)) {
    $commands{$_} = \&environment_line_command;
    $translate_line_command{$_} = 0;
    $break_line{$_} = 1;
}
foreach (qw(enumerate multitable ifclear ifset)) {
    $commands{$_} = \&environment_line_command;
    $break_line{$_} = 1;
}
foreach (qw(quotation)) {
    $commands{$_} = \&environment_line_command;
    $translate_line_command{$_} = 1;
    $break_line{$_} = 1;
}

$env_separators{'format'} = "(?:(?:^|\n)\\\*|END-INFO-DIR-ENTRY|START-INFO-DIR-ENTRY)";
$env_separators{'multitable'} = "(?:\@item|\@tab)";

my $end_command=$commands{'end'};
register_generic_command("*end,  ");
$commands{'end'} = $end_command;
$break_line{'end'} = 1;

register_generic_command("*macro,  ");
$commands{'macro'} = \&environment_command;
$break_line{'macro'} = 1;
register_generic_command("*itemize,  ");
$commands{'itemize'} = \&environment_command;
$break_line{'itemize'} = 1;
register_generic_command("*table,  ");
$commands{'table'} = \&environment_command;
$break_line{'table'} = 1;

# TODO: is_closed, use a regexp: \ does not escape the closing brace.
# TBC on LaTeX.
# In Texinfo, it appears with the "code" command. Maybe this command should
# be used as verbatim. (Expressions.texi)

# TODO: @include @ignore

# TBC: node Indices

1;
