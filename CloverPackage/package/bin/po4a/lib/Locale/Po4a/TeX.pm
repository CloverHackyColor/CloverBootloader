#!/usr/bin/perl -w

# Copyright (c) 2004, 2005 by Nicolas FRANÇOIS <nicolas.francois@centraliens.net>
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

Locale::Po4a::TeX - convert TeX documents and derivates from/to PO files

=head1 DESCRIPTION

The po4a (PO for anything) project goal is to ease translations (and more
interestingly, the maintenance of translations) using gettext tools on
areas where they were not expected like documentation.

Locale::Po4a::TeX is a module to help the translation of TeX documents into
other [human] languages. It can also be used as a base to build modules for
TeX-based documents.

Users should probably use the LaTeX module, which inherite from the TeX module
and contains the definitions of common LaTeX commands.

=head1 TRANSLATING WITH PO4A::TEX

This module can be used directly to handle generic TeX documents.
This will split your document in smaller blocks (paragraphs, verbatim
blocks, or even smaller like titles or indexes).

There are some options (described in the next section) that can customize
this behavior.  If this doesn't fit to your document format you're encouraged
to write your own module derived from this, to describe your format's details.
See the section B<WRITING DERIVATE MODULES> below, for the process description.

This module can also be customized by lines starting with "% po4a:" in the
TeX file.
These customizations are described in the B<INLINE CUSTOMIZATION> section.

=head1 OPTIONS ACCEPTED BY THIS MODULE

These are this module's particular options:

=over 4

=cut

package Locale::Po4a::TeX;

use 5.006;
use strict;
use warnings;

require Exporter;
use vars qw(@ISA @EXPORT);
@ISA = qw(Locale::Po4a::TransTractor);
@EXPORT = qw(%commands %environments
             $RE_ESCAPE $ESCAPE $RE_VERBATIM
             $no_wrap_environments
             $verbatim_environments
             %separated_command
             %separated_environment
             %translate_buffer_env
             &generic_command
             &register_generic_command
             &register_generic_environment);

use Locale::Po4a::TransTractor;
use Locale::Po4a::Common;
use File::Basename qw(dirname);
use Carp qw(croak);

use Encode;
use Encode::Guess;

# hash of known commands and environments, with parsing sub.
# See end of this file
use vars qw(%commands %environments);
# hash to describe the number of parameters and which one have to be
# translated. Used by generic commands
our %command_parameters = ();
our %environment_parameters = ();
# hash to describe the separators of environments.
our %env_separators =();

# The escape character used to introduce commands.
our $RE_ESCAPE = "\\\\";
our $ESCAPE    = "\\";
# match the beginning of a verbatim block
our $RE_VERBATIM = "\\\\begin\\{(?:verbatim)\\*?\\}";
# match the beginning of a comment.
# NOTE: It must contain a group, with chars preceding the comment
our $RE_PRE_COMMENT= "(?<!\\\\)(?:\\\\\\\\)*";
our $RE_COMMENT= "\\\%";

# Space separated list of environments that should not be re-wrapped.
our $no_wrap_environments = "verbatim";
our $verbatim_environments = "verbatim";
# hash with the commands that have to be separated (or have to be joined).
# 3 modes are currently used:
#  '*' The command is separated if it appear at an extremity of a
#      paragraph
#  '+' The command is separated, but its arguments are joined together
#      with the command name for the translation
#  '-' The command is not separated, unless it appear alone on a paragraph
#      (e.g. \strong)
our %separated_command = ();
our %separated_environment = ();

=item B<debug>

Activate debugging for some internal mechanisms of this module.
Use the source to see which parts can be debugged.

=item B<no_wrap>

Comma-separated list of environments which should not be re-wrapped.

Note that there is a difference between verbatim and no_wrap environments.
There is no command and comments analysis in verbatim blocks.

If this environment was not already registered, po4a will consider that
this environment does not take any parameters.

=item B<exclude_include>

Colon-separated list of files that should not be included by \input and
\include.

=item B<definitions>

The name of a file containing definitions for po4a, as defined in the
B<INLINE CUSTOMIZATION> section.
You can use this option if it is not possible to put the definitions in
the document being translated.

=item B<verbatim>

Comma-separated list of environments which should be taken as verbatim.

If this environment was not already registered, po4a will consider that
this environment does not take any parameters.

=back

Using these options permits to override the behaviour of the commands defined
in the default lists.

=head1 INLINE CUSTOMIZATION

The TeX module can be customized with lines starting by B<% po4a:>.
These lines are interpreted as commands to the parser.
The following commands are recognized:

=over 4

=item B<% po4a: command> I<command1> B<alias> I<command2>

Indicates that the arguments of the I<command1> command should be
treated as the arguments of the I<command2> command.

=item B<% po4a: command> I<command1> I<parameters>

This permit to describe in detail the parameters of the I<command1>
command.
This information will be used to check the number of arguments and their
types.

You can precede the I<command1> command by

=over 4

=item an asterisk (B<*>)

po4a will extract this command from paragraphs (if it is located at
the beginning or the end of a paragraph).
The translators will then have to translate the parameters that are marked
as translatable.

=item a plus (B<+>)

As for an asterisk, the command will be extracted if it appear at an
extremity of a block, but the parameters won't be translated separately.
The translator will have to translate the command concatenated to all its
parameters.
This permits to keep more context, and is useful for commands with small
words in parameter, which can have multiple meanings (and translations).

Note: In this case you don't have to specify which parameters are
translatable, but po4a must know the type and number of parameters.

=item a minus (B<->)

In this case, the command won't be extracted from any block.
But if it appears alone on a block, then only the parameters marked as
translatable will be presented to the translator.
This is useful for font commands.  These commands should generally not be
separated from their paragraph (to keep the context), but there is no
reason to annoy the translator with them if a whole string is enclosed in
such a command.

=back

The I<parameters> argument is a set of [] (to indicate an optional
argument) or {} (to indicate a mandatory argument).
You can place an underscore (_) between these brackets to indicate that
the parameter must be translated. For example:
 % po4a: command *chapter [_]{_}

This indicates that the chapter command has two parameters: an optional
(short title) and a mandatory one, which must both be translated.
If you want to specify that the href command has two mandatory parameters,
that you don't want to translate the URL (first parameter), and that you
don't want this command to be separated from its paragraph (which allow
the translator to move the link in the sentence), you can use:
 % po4a: command -href {}{_}

In this case, the information indicating which arguments must be
translated is only used if a paragraph is only composed of this href
command.

=item B<% po4a: environment> I<env> I<parameters>

This permits to define the parameters accepted by the I<env> environment.
This information is latter used to check the number of arguments of the
\begin command, and permit to specify which one must be translated.
The syntax of the I<parameters> argument is the same as described for the
commands.
The first parameter of the \begin command is the name of the environment.
This parameter must not be specified in the list of parameters. Here are
some examples:
 % po4a: environment multicols {}
 % po4a: environment equation

As for the commands, I<env> can be preceded by a plus (+) to indicate
that the \begin command must be translated with all its arguments.

=item B<% po4a: separator> I<env> B<">I<regex>B<">

Indicates that an environment should be split according to the given
regular expression.

The regular expression is delimited by quotes.
It should not create any backreference.
You should use (?:) if you need a group.
It may also need some escapes.

For example, the LaTeX module uses the "(?:&|\\\\)" regular expression to
translate separately each cell of a table (lines are separated by '\\' and
cells by '&').

The notion of environment is expended to the type displayed in the PO file.
This can be used to split on "\\\\" in the first mandatory argument of the
title command.  In this case, the environment is title{#1}.

=item B<% po4a: verbatim environment> I<env>

Indicate that I<env> is a verbatim environment.
Comments and commands will be ignored in this environment.

If this environment was not already registered, po4a will consider that
this environment does not take any parameters.

=back

=cut

# Directory name of the main file.
# It is the directory where included files will be searched.
# See read_file.
my $my_dirname;

# Array of files that should not be included by read_file.
# See read_file.
our @exclude_include;

my %type_end=('{'=>'}', '['=>']', ' '=>'');

#########################
#### DEBUGGING STUFF ####
#########################
my %debug=('pretrans'         => 0, # see pre-conditioning of translation
           'postrans'         => 0, # see post-conditioning of translation
           'translate'        => 0, # see translation
           'extract_commands' => 0, # see commands extraction
           'commands'         => 0, # see command subroutines
           'environments'     => 0, # see environment subroutines
           'translate_buffer' => 0  # see buffer translation
           );

=head1 WRITING DERIVATE MODULES

=over 4

=item B<pre_trans>

=cut

sub pre_trans {
    my ($self,$str,$ref,$type)=@_;
    # Preformatting, so that translators don't see
    # strange chars
    my $origstr=$str;
    print STDERR "pre_trans($str)="
        if ($debug{'pretrans'});

    # Accentuated characters
    # FIXME: only do this if the encoding is UTF-8?
#    $str =~ s/${RE_ESCAPE}`a/à/g;
##    $str =~ s/${RE_ESCAPE}c{c}/ç/g; # not in texinfo: @,{c}
#    $str =~ s/${RE_ESCAPE}^e/ê/g;
#    $str =~ s/${RE_ESCAPE}'e/é/g;
#    $str =~ s/${RE_ESCAPE}`e/è/g;
#    $str =~ s/${RE_ESCAPE}`u/ù/g;
#    $str =~ s/${RE_ESCAPE}"i/ï/g;
#    # Non breaking space. FIXME: should we change $\sim$ to ~
#    $str =~ s/~/\xA0/g; # FIXME: not in texinfo: @w{ }

    print STDERR "$str\n" if ($debug{'pretrans'});
    return $str;
}

=item B<post_trans>

=cut

sub post_trans {
    my ($self,$str,$ref,$type)=@_;
    my $transstr=$str;

    print STDERR "post_trans($str)="
        if ($debug{'postrans'});

    # Accentuated characters
#    $str =~ s/à/${ESCAPE}`a/g;
##    $str =~ s/ç/$ESCAPEc{c}/g; # FIXME: not in texinfo
#    $str =~ s/ê/${ESCAPE}^e/g;
#    $str =~ s/é/${ESCAPE}'e/g;
#    $str =~ s/è/${ESCAPE}`e/g;
#    $str =~ s/ù/${ESCAPE}`u/g;
#    $str =~ s/ï/${ESCAPE}"i/g;
#    # Non breaking space. FIXME: should we change ~ to $\sim$
#    $str =~ s/\xA0/~/g; # FIXME: not in texinfo

    print STDERR "$str\n" if ($debug{'postrans'});
    return $str;
}

# Comments are extracted in the parse function.
# They are stored in the @comments array, and then displayed as a PO
# comment with the first translated string of the paragraph.
my @comments = ();

=item B<translate>

Wrapper around Transtractor's translate, with pre- and post-processing
filters.

Comments of a paragraph are inserted as a PO comment for the first
translated string of this paragraph.

=cut

sub translate {
    my ($self,$str,$ref,$type) = @_;
    my (%options)=@_;
    my $origstr=$str;
    print STDERR "translate($str)="
        if ($debug{'translate'});

    return $str unless (defined $str) && length($str);
    return $str if ($str eq "\n");

    $str=pre_trans($self,$str,$ref||$self->{ref},$type);

    # add comments (if any and not already added to the PO)
    if (@comments) {
        $options{'comment'} .= join('\n', @comments);

        @comments = ();
    }

# FIXME: translate may append a newline, keep the trailing spaces so we can
# recover them.
    my $spaces = "";
    if ($options{'wrap'} and $str =~ m/^(.*?)(\s+)$/s) {
        $str    = $1;
        $spaces = $2;
    }

    # Translate this
    $str = $self->SUPER::translate($str,
                                   $ref||$self->{ref},
                                   $type || $self->{type},
                                   %options);

# FIXME: translate may append a newline, see above
    if ($options{'wrap'}) {
        chomp $str;
        $str .= $spaces;
    }

    $str=post_trans($self,$str,$ref||$self->{ref},$type);

    print STDERR "'$str'\n" if ($debug{'translate'});
    return $str;
}

###########################
### COMMANDS SEPARATION ###
###########################

=item B<get_leading_command>($buffer)

This function returns:

=over 4

=item A command name

If no command is found at the beginning of the given buffer, this string
will be empty.  Only commands that can be separated are considered.
The %separated_command hash contains the list of these commands.

=item A variant

This indicates if a variant is used.  For example, an asterisk (*) can
be added at the end of sections command to specify that they should
not be numbered.  In this case, this field will contain "*".  If there
is no variant, the field is an empty string.

=item An array of tuples (type of argument, argument)

The type of argument can be either '{' (for mandatory arguments) or '['
(for optional arguments).

=item The remaining buffer

The rest of the buffer after the removal of this leading command and
its arguments.  If no command is found, the original buffer is not
touched and returned in this field.

=back

=cut

sub get_leading_command {
    my ($self, $buffer) = (shift,shift);
    my $command = ""; # the command name
    my $variant = ""; # a varriant for the command (e.g. an asterisk)
    my @args; # array of arguments
    print STDERR "get_leading_command($buffer)="
        if ($debug{'extract_commands'});

    if ($buffer =~ m/^$RE_ESCAPE([[:alnum:]]+)(\*?)(.*)$/s
        && defined $separated_command{$1}) {
        # The buffer begin by a comand (possibly preceded by some
        # whitespaces).
        $command = $1;
        $variant = $2;
        $buffer  = $3;
        # read the arguments (if any)
        while ($buffer =~ m/^\s*$RE_PRE_COMMENT([\[\{])(.*)$/s) {
            my $type = $1;
            my $arg = "";
            my $count = 1;
            $buffer = $2;
            # stop reading the buffer when the number of ] (or }) matches the
            # the number of [ (or {).
            while ($count > 0) {
                if ($buffer =~ m/^(.*?$RE_PRE_COMMENT)([\[\]\{\}])(.*)$/s) {
                    $arg .= $1;
                    $buffer = $3;
                    if ($2 eq $type) {
                        $count++;
                    } elsif ($2 eq $type_end{$type}) {
                        $count--;
                    }
                    if ($count > 0) {
                        $arg .= $2
                    }
                } else {
                    die wrap_ref_mod($self->{ref},
                                     "po4a::tex",
                                     dgettext("po4a", "un-balanced %s in '%s'"),
                                     $type,
                                     $buffer);
                }
            }
            push @args, ($type,$arg);
        }
    }
    if (defined $command and length $command) {
        # verify the number of arguments
        my($check,$reason,$remainder) = check_arg_count($self,$command,\@args);
        if (not $check) {
            die wrap_ref_mod($self->{ref}, "po4a::tex",
                             dgettext("po4a",
                                 "Error while checking the number of ".
                                 "arguments of the '%s' command: %s")."\n",
                             $command, $reason);
        }

        if (@$remainder) {
            # FIXME: we should also keep the spaces to be idempotent
            my ($temp,$type,$arg);
            while (@$remainder) {
                $type = shift @$remainder;
                $arg  = shift @$remainder;
                $temp .= $type.$arg.$type_end{$type};
                # And remove the same number of arguments from @args
                pop @args;
                pop @args;
            }
            $buffer = $temp.$buffer;
        }
    }

    print STDERR "($command,$variant,@args,$buffer)\n"
        if ($debug{'extract_commands'});
    return ($command,$variant,\@args,$buffer);
}

=item B<get_trailing_command>($buffer)

The same as B<get_leading_command>, but for commands at the end of a buffer.

=cut

sub get_trailing_command {
    my ($self, $buffer) = (shift,shift);
    my $orig_buffer = $buffer;
    print STDERR "get_trailing_command($buffer)="
        if ($debug{'extract_commands'});

    my @args;
    my $command = "";
    my $variant = "";

    # While the buffer ends by }, consider it is a mandatory argument
    # and extract this argument.
    while (   $buffer =~ m/^(.*$RE_PRE_COMMENT(\{).*)$RE_PRE_COMMENT\}$/s
           or $buffer =~ m/^(.*$RE_PRE_COMMENT(\[).*)$RE_PRE_COMMENT\]$/s) {
        my $arg = "";
        my $count = 1;
        $buffer = $1;
        my $type = $2;
        # stop reading the buffer when the number of } (or ]) matches the
        # the number of { (or [).
        while ($count > 0) {
            if ($buffer =~ m/^(.*$RE_PRE_COMMENT)([\{\}\[\]])(.*)$/s) {
                 $arg = $3.$arg;
                 $buffer = $1;
                 if ($2 eq $type) {
                     $count--;
                 } elsif ($2 eq $type_end{$type}) {
                     $count++;
                 }
                 if ($count > 0) {
                     $arg = $2.$arg;
                 }
            } else {
                die wrap_ref_mod($self->{ref},
                                 "po4a::tex",
                                 dgettext("po4a", "un-balanced %s in '%s'"),
                                 $type_end{$type},
                                 $buffer);
            }
        }
        unshift @args, ($type,$arg);
    }

    # There should now be a command, maybe followed by an asterisk.
    if ($buffer =~ m/^(.*$RE_PRE_COMMENT)$RE_ESCAPE([[:alnum:]]+)(\*?)\s*$/s
        && defined $separated_command{$2}) {
        $buffer = $1;
        $command = $2;
        $variant = $3;
        my($check,$reason,$remainder) = check_arg_count($self,$command,\@args);
        if (not $check) {
            die wrap_ref_mod($self->{ref}, "po4a::tex",
                             dgettext("po4a",
                                 "Error while checking the number of ".
                                 "arguments of the '%s' command: %s")."\n",
                             $command, $reason);
        }
        if (@$remainder) {
            # There are some arguments after the command.
            # We can't extract this comand.
            $command = "";
        }
    }

    # sanitize return values if no command was found.
    if (!length($command)) {
        $command = "";
        $variant = "";
        undef @args;
        $buffer = $orig_buffer;
    }
# verify the number of arguments

    print STDERR "($command,$variant,@args,$buffer)\n"
        if ($debug{'extract_commands'});
    return ($command,$variant,\@args,$buffer);
}

=item B<translate_buffer>

Recursively translate a buffer by separating leading and trailing
commands (those which should be translated separately) from the
buffer.

If a function is defined in %translate_buffer_env for the current
environment, this function will be used to translate the buffer instead of
translate_buffer().

=cut

our %translate_buffer_env = ();
sub translate_buffer {
    my ($self,$buffer,$no_wrap,@env) = (shift,shift,shift,@_);

    if (@env and defined $translate_buffer_env{$env[-1]}) {
        return &{$translate_buffer_env{$env[-1]}}($self,$buffer,$no_wrap,@env);
    }

    print STDERR "translate_buffer($buffer,$no_wrap,@env)="
        if ($debug{'translate_buffer'});

    my ($command,$variant) = ("","");
    my $args;
    my $translated_buffer = "";
    my $orig_buffer = $buffer;
    my $t = ""; # a temporary string

    if ($buffer =~ /^\s*$/s) {
        print STDERR "($buffer,@env)\n"
            if ($debug{'translate_buffer'});
        return ($buffer, @env);
    }
    # verbatim blocks.
    # Buffers starting by \end{verbatim} are handled after.
    if (in_verbatim(@env) and $buffer !~ m/^\n?\Q$ESCAPE\Eend\{$env[-1]\*?\}/) {
        if($buffer =~ m/^(.*?)(\n?\Q$ESCAPE\Eend\{$env[-1]\*?\}.*)$/s) {
            # end of a verbatim block
            my ($begin, $end) = ($1?$1:"", $2);
            my ($t1, $t2) = ("", "");
            if (defined $begin) {
                $t1 = $self->translate($begin,$self->{ref},
                                       $env[-1],
                                       "wrap" => 0);
            }
            ($t2, @env) = translate_buffer($self, $end, $no_wrap, @env);
            print STDERR "($t1$t2,@env)\n"
                if ($debug{'translate_buffer'});
            return ($t1.$t2, @env);
        } else {
            $translated_buffer = $self->translate($buffer,$self->{ref},
                                                  $env[-1],
                                                  "wrap" => 0);
            print STDERR "($translated_buffer,@env)\n"
                if ($debug{'translate_buffer'});
            return ($translated_buffer, @env);
        }
    }
    # early detection of verbatim environment
    if ($buffer =~ /^($RE_VERBATIM\n?)(.*)$/s and length $2) {
        my ($begin, $end) = ($1, $2);
        my ($t1, $t2) = ("", "");
        ($t1, @env) = translate_buffer($self, $begin, $no_wrap, @env);
        ($t2, @env) = translate_buffer($self, $end,   $no_wrap, @env);

        print STDERR "($t1$t2,@env)\n"
            if ($debug{'translate_buffer'});
        return ($t1.$t2, @env);
    }
    # detect \begin and \end (if they are not commented)
    if ($buffer =~ /^((?:.*?\n)?                # $1 is
                      (?:[^%]                   # either not a %
                        |                       # or
                         (?<!\\)(?:\\\\)*\\%)*? # a % preceded by an odd nb of \
                     )                          # $2 is a \begin{ with the end of the line
                      (${RE_ESCAPE}(?:begin|end)\{.*)$/sx
        and length $1) {
        my ($begin, $end) = ($1, $2);
        my ($t1, $t2) = ("", "");
        if (is_closed($begin)) {
            ($t1, @env) = translate_buffer($self, $begin, $no_wrap, @env);
            ($t2, @env) = translate_buffer($self, $end,   $no_wrap, @env);

            print STDERR "($t1$t2,@env)\n"
                if ($debug{'translate_buffer'});
            return ($t1.$t2, @env);
        }
    }

    # remove comments from the buffer.
    # Comments are stored in an array and shown as comments in the PO.
    while ($buffer =~ m/($RE_PRE_COMMENT)$RE_COMMENT([^\n]*)(\n[ \t]*)(.*)$/s) {
        my $comment = $2;
        my $end = "";
        if ($4 =~ m/^\n/s and $buffer !~ m/^$RE_COMMENT/s) {
            # a line with comments, followed by an empty line.
            # Keep the empty line, but remove the comment.
            # This is an empirical heuristic, but seems to work;)
            $end = "\n";
        }
        if (defined $comment and $comment !~ /^\s*$/s) {
            push @comments, $comment;
        }
        $buffer =~ s/($RE_PRE_COMMENT)$RE_COMMENT([^\n]*)(\n[ \t]*)/$1$end/s;
    }

    # translate leading commands.
    do {
        # keep the leading space to put them back after the translation of
        # the command.
        my $spaces = "";
        if ($buffer =~ /^(\s+)(.*?)$/s) {
            $spaces = $1;
#            $buffer = $2; # FIXME: this also remove trailing spaces!!
            $buffer =~ s/^\s*//s;
        }
        my $buffer_save = $buffer;
        ($command, $variant, $args, $buffer) =
            get_leading_command($self,$buffer);
        if (    (length $command)
            and (defined $separated_command{$command})
            and ($separated_command{$command} eq '-')
            and (   (not (defined($buffer)))
                 or ($buffer !~ m/^\s*$/s)  )) {
            # This command can be separated only if alone on a buffer.
            # We need to remove the trailing commands first, and see if it
            # will be alone on this buffer.
            $buffer = $buffer_save;
            $command = "";
        }
        if (length($command)) {
            # call the command subroutine.
            # These command subroutines will probably call translate_buffer
            # with the content of each argument that need a translation.
            if (defined ($commands{$command})) {
                ($t,@env) = &{$commands{$command}}($self,$command,$variant,
                                                   $args,\@env,$no_wrap);
                $translated_buffer .= $spaces.$t;
                # Handle spaces after a command.
                $spaces = "";
                if ($buffer =~ /^(\s+)(.*?)$/s) {
                    $spaces = $1;
#                    $buffer = $2;  # FIXME: this also remove trailing spaces!!
                    $buffer =~ s/^\s*//s;
                }
                $translated_buffer .= $spaces;
            } else {
                die wrap_ref_mod($self->{ref},
                                 "po4a::tex",
                                 dgettext("po4a", "Unknown command: '%s'"),
                                 $command);
            }
        } else {
            $buffer = $spaces.$buffer;
        }
    } while (length($command));

    # array of trailing commands, which will be translated later.
    my @trailing_commands = ();
    do {
        my $spaces = "";
        if ($buffer =~ /^(.*?)(\s+)$/s) {
            $buffer = $1;
            $spaces = $2;
        }
        my $buffer_save = $buffer;
        ($command, $variant, $args, $buffer) =
            get_trailing_command($self,$buffer);
        if (    (length $command)
            and (defined $separated_command{$command})
            and ($separated_command{$command} eq '-')
            and (   (not defined $buffer)
                 or ($buffer !~ m/^\s*$/s))) {
            # We can extract this command.
            $command = "";
            $buffer = $buffer_save;
        }
        if (length($command)) {
            unshift @trailing_commands, ($command, $variant, $args, $spaces);
        } else {
            $buffer .= $spaces;
        }
    } while (length($command));

    # Now, $buffer is just a block that can be translated.

    # environment specific treatment
    if (@env and defined $env_separators{$env[-1]}) {
        my $re_separator = $env_separators{$env[-1]};
        my $buf_begin = "";
# FIXME: the separator may have to be translated.
        while ($buffer =~ m/^(.*?)(\s*$re_separator\s*)(.*)$/s) {
            my ($begin, $sep, $end) = ($1, $2, $3);
            $buf_begin .= $begin;
            if (is_closed($buf_begin)) {
                my $t = "";
                ($t, @env) = translate_buffer($self,$buf_begin,$no_wrap,@env);
                $translated_buffer .= $t.$sep;
                $buf_begin = "";
            } else {
                # the command is in a command argument
                $buf_begin .= $sep;
            }
            $buffer = $end;
        }
        $buffer = $buf_begin . $buffer;
    }

    # finally, translate
    if (length($buffer)) {
        my $wrap = 1;
        my ($e1, $e2);
        NO_WRAP_LOOP: foreach $e1 (@env) {
            foreach $e2 (split(' ', $no_wrap_environments)) {
                if ($e1 eq $e2) {
                    $wrap = 0;
                    last NO_WRAP_LOOP;
                }
            }
        }
        $wrap = 0 if (defined $no_wrap and $no_wrap == 1);
        # Keep spaces at the end of the buffer.
        my $spaces_end = "";
        if ($buffer =~ /^(.*?)(\s+)$/s) {
            $spaces_end = $2;
            $buffer = $1;
        }
        if ($wrap and $buffer =~ s/^(\s+)//s) {
                $translated_buffer .= $1;
        }
        $translated_buffer .= $self->translate($buffer,$self->{ref},
                                               @env?$env[-1]:"Plain text",
                                               "wrap" => $wrap);
        # Restore spaces at the end of the buffer.
        $translated_buffer .= $spaces_end;
    }

    # append the translation of the trailing commands
    while (@trailing_commands) {
        my $command = shift @trailing_commands;
        my $variant = shift @trailing_commands;
        my $args    = shift @trailing_commands;
        my $spaces  = shift @trailing_commands;
        if (defined ($commands{$command})) {
            ($t,@env) = &{$commands{$command}}($self,$command,$variant,
                                               $args,\@env,$no_wrap);
            $translated_buffer .= $t.$spaces;
        } else {
            die wrap_ref_mod($self->{ref},
                             "po4a::tex",
                             dgettext("po4a", "Unknown command: '%s'"),
                             $command);
        }
    }

    print STDERR "($translated_buffer,@env)\n"
        if ($debug{'translate_buffer'});
    return ($translated_buffer,@env);
}

################################
#### EXTERNAL CUSTOMIZATION ####
################################

=item B<read>

Overload Transtractor's read

=cut

sub read {
    my $self=shift;
    my $filename=shift;

    # keep the directory name of the main file.
    $my_dirname = dirname($filename);

    push @{$self->{TT}{doc_in}}, read_file($self, $filename);
}

=item B<read_file>

Recursively read a file, appending included files which are not listed in the
@exclude_include array.  Included files are searched using the B<kpsewhich>
command from the Kpathsea library.

Except from the file inclusion part, it is a cut and paste from
Transtractor's read.

=cut

# TODO: fix DOS end of lines
sub read_file {
    my $self=shift;
    my $filename=shift
        or croak wrap_mod("po4a::tex",
            dgettext("po4a", "Can't read from file without having a filename"));
    my $linenum=0;
    my @entries=();

    open (my $in, $filename)
        or croak wrap_mod("po4a::tex",
            dgettext("po4a", "Can't read from %s: %s"), $filename, $!);
    while (defined (my $textline = <$in>)) {
        $linenum++;
        my $ref="$filename:$linenum";
        # TODO: add support for includeonly
        # The next regular expression matches \input or \includes that are
        # not commented (but can be preceded by a \%.
        while ($textline =~ /^((?:[^%]|(?<!\\)(?:\\\\)*\\%)*)
                              \\(include|input)
                              \{([^\{]*)\}(.*)$/x) {
            my ($begin,$newfilename,$end) = ($1,$3,$4);
            my $tag = $2;
            my $include = 1;
            foreach my $f (@exclude_include) {
                if ($f eq $newfilename) {
                    $include = 0;
                    $begin .= "\\$tag"."{$newfilename}";
                    $textline = $end;
                    last;
                }
            }
            if ($include and ($tag eq "include")) {
                $begin .= "\\clearpage";
            }
            if ($begin !~ /^\s*$/) {
                push @entries, ($begin,$ref);
            }
            if ($include) {
                # search the file
                open (KPSEA, "kpsewhich " . $newfilename . " |");
                my $newfilepath = <KPSEA>;

                if ($newfilename ne "" and $newfilepath eq "") {
                    die wrap_mod("po4a::tex",
                                 dgettext("po4a",
                                          "Can't find %s with kpsewhich"),
                                 $filename);
                }

                push @entries, read_file($self,
                                         $newfilepath);
                if ($tag eq "include") {
                    $textline = "\\clearpage".$end;
                } else {
                    $textline = $end;
                }
            }
        }
        if (length($textline)) {
            my @entry=($textline,$ref);
            push @entries, @entry;

            # Detect if this file has non-ascii characters
            if($self->{TT}{ascii_input}) {

                my $decoder = guess_encoding($textline);
                if (!ref($decoder) or $decoder !~ /Encode::XS=/) {
                    # We have detected a non-ascii line
                    $self->{TT}{ascii_input} = 0;
                    # Save the reference for future error message
                    $self->{TT}{non_ascii_ref} ||= $ref;
                }
            }
        }
    }
    close $in
        or croak wrap_mod("po4a::tex",
            dgettext("po4a", "Can't close %s after reading: %s"), $filename, $!);

    return @entries;
}

=back


=over 4

=item B<parse_definition_file>

Subroutine for parsing a file with po4a directives (definitions for
new commands).

=cut

sub parse_definition_file {
    my ($self,$filename,$only_try)=@_;
    my $filename_org = $filename;

    open (KPSEA, "kpsewhich " . $filename . " |");
    $filename = <KPSEA>;

    if (not defined $filename) {
        warn wrap_mod("po4a::tex",
            dgettext("po4a", "kpsewhich cannot find %s"), $filename_org);
        if (defined $only_try && $only_try) {
            return;
        } else {
            exit 1;
        }
    }

    if (! open (IN,"<$filename")) {
        warn wrap_mod("po4a::tex",
            dgettext("po4a", "Can't open %s: %s"), $filename, $!);
        if (defined $only_try && $only_try) {
            return;
        } else {
            exit 1;
        }
    }
    while (<IN>) {
        if (/^\s*%\s*po4a\s*:/) {
            parse_definition_line($self, $_);
        }
    }
}

=item B<parse_definition_line>

Parse a definition line of the form "% po4a: ".

See the B<INLINE CUSTOMIZATION> section for more details.

=cut

sub parse_definition_line {
    my ($self,$line)=@_;
    $line =~ s/^\s*%\s*po4a\s*:\s*//;

    if ($line =~ /^command\s+([-*+]?)(\w+)\s+(.*)$/) {
        my $command = $2;
        $line = $3;
        if ($1) {
            $separated_command{$command} = $1;
        }
        if ($line =~ /^alias\s+(\w+)\s*$/) {
            if (defined ($commands{$1})) {
                $commands{$command} = $commands{$1};
                $command_parameters{$command} = $command_parameters{$1};
            } else {
                die wrap_mod("po4a::tex",
                             dgettext("po4a", "Cannot use an alias to the unknown command '%s'"),
                             $2);
            }
        } elsif ($line =~ /^(-1|\d+),(-1|\d+),(-1|[ 0-9]*),(-1|[ 0-9]*?)\s*$/) {
            die wrap_ref_mod($self->{ref},
                             "po4a::tex",
                             dgettext("po4a", "You are using the old ".
                                      "definitions format (%s).  ".
                                      "Please update this definition line."),
                             $_[1])
        } elsif ($line =~ m/^((?:\{_?\}|\[_?\])*)\s*$/) {
            register_generic_command("$command,$1");
        }
    } elsif ($line =~ /^environment\s+([+]?\w+\*?)(.*)$/) {
        my $env = $1;
        $line = $2;
        if ($line =~ m/^\s*((?:\{_?\}|\[_?\])*)\s*$/) {
            register_generic_environment("$env,$1");
        }
    } elsif ($line =~ /^separator\s+(\w+(?:\[#[0-9]+\])?)\s+\"(.*)\"\s*$/) {
        my $env = $1; # This is not necessarily an environment.
                      # It can also be smth like 'title[#1]'.
        $env_separators{$env} = $2;
    } elsif ($line =~ /^verbatim\s+environment\s+(\w+)\s+$/) {
        register_verbatim_environment($1);
    }
}

=item B<is_closed>

=cut

sub is_closed {
    my $paragraph = shift;
# FIXME: [ and ] are more difficult to handle, because it is not easy to detect if it introduce an optional argument
    my $tmp = $paragraph;
    my $closing = 0;
    my $opening = 0;
    # FIXME: { and } should not be counted in verbatim blocks
    # Remove comments
    $tmp =~ s/$RE_PRE_COMMENT$RE_COMMENT.*//mg;
    while ($tmp =~ /^.*?$RE_PRE_COMMENT\{(.*)$/s) {
        $opening += 1;
        $tmp = $1;
    }
    $tmp = $paragraph;
    # Remove comments
    $tmp =~ s/$RE_PRE_COMMENT$RE_COMMENT.*//mg;
    while ($tmp =~ /^.*?$RE_PRE_COMMENT\}(.*)$/s) {
        $closing += 1;
        $tmp = $1;
    }
    return $opening eq $closing;
}

sub in_verbatim {
    foreach my $e1 (@_) {
        foreach my $e2 (split(' ', $verbatim_environments)) {
            if ($e1 eq $e2) {
                return 1;
            }
        }
    }

    return 0;
}

#############################
#### MAIN PARSE FUNCTION ####
#############################
=item B<parse>

=cut

sub parse {
    my $self = shift;
    my ($line,$ref);
    my $paragraph = ""; # Buffer where we put the paragraph while building
    my @env = (); # environment stack
    my $t = "";

  LINE:
    undef $self->{type};
    ($line,$ref)=$self->shiftline();

    while (defined($line)) {
        chomp($line);
        $self->{ref}="$ref";

        if ($line =~ /^\s*%\s*po4a\s*:/) {
            parse_definition_line($self, $line);
            goto LINE;
        }

        my $closed = is_closed($paragraph);

#FIXME: what happens if a \begin{verbatim} or \end{verbatim} is in the
#       middle of a line. (This is only an issue if the verbatim
#       environment contains an un-closed bracket)
        if (   ($closed and ($line =~ /^\s*$/ or
                             $line =~ /^\s*$RE_VERBATIM\s*$/))
            or (in_verbatim(@env) and $line =~ /^\s*\Q$ESCAPE\Eend{$env[-1]}\s*$/)
           ) {
            # An empty line. This indicates the end of the current
            # paragraph.
            $paragraph .= $line."\n";
            if (length($paragraph)) {
                ($t, @env) = translate_buffer($self,$paragraph,undef,@env);
                $self->pushline($t);
                $paragraph="";
                @comments = ();
            }
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

=item B<docheader>

=back

=cut

sub docheader {
    return "% This file was generated with po4a. Translate the source file.\n".
           "%\n";
}


####################################
#### DEFINITION OF THE COMMANDS ####
####################################

=head1 INTERNAL FUNCTIONS used to write derivated parsers

Command and environment functions take the following arguments (in
addition to the $self object):

=over

=item A command name

=item A variant

=item An array of (type, argument) tuples

=item The current environment

=back

The first 3 arguments are extracted by get_leading_command or
get_trailing_command.

Command and environment functions return the translation of the command
with its arguments and a new environment.

Environment functions are called when a \begin command is found. They are
called with the \begin command and its arguments.

The TeX module only proposes one command function and one environment
function: generic_command and generic_environment.

generic_command uses the information specified by
register_generic_command or by adding definition to the TeX file:
 % po4a: command I<command1> I<parameters>

generic_environment uses the information specified by
register_generic_environment or by adding definition to the TeX file:
 % po4a: environment I<env> I<parameters>

Both functions will only translate the parameters that were specified as
translatable (with a '_').
generic_environment will append the name of the environment to the
environment stack and generic_command will append the name of the command
followed by an identifier of the parameter (like {#7} or [#2]).

=cut

# definition of environment related commands

$commands{'begin'}= sub {
    my $self = shift;
    my ($command,$variant,$args,$env) = (shift,shift,shift,shift);
    my $no_wrap = shift;
    print "begin($command,$variant,@$args,@$env,$no_wrap)="
        if ($debug{'commands'} || $debug{'environments'});
    my ($t,@e) = ("",());

    my $envir = $args->[1];
    if (defined($envir) and $envir =~ /^(.*)\*$/) {
        $envir = $1;
    }

    if (defined($envir) && defined($environments{$envir})) {
        ($t, @e) = &{$environments{$envir}}($self,$command,$variant,
                                            $args,$env,$no_wrap);
    } else {
        die wrap_ref_mod($self->{ref}, "po4a::tex",
                     dgettext("po4a", "unknown environment: '%s'"),
                     $args->[1]);
    }

    print "($t, @e)\n"
        if ($debug{'commands'} || $debug{'environments'});
    return ($t, @e);
};
# Use register_generic to set the type of arguments. The function is then
# overwritten:
register_generic_command("*end,{}");
$commands{'end'}= sub {
    my $self = shift;
    my ($command,$variant,$args,$env) = (shift,shift,shift,shift);
    my $no_wrap = shift;
    print "end($command,$variant,@$args,@$env,$no_wrap)="
        if ($debug{'commands'} || $debug{'environments'});

    # verify that this environment was the last pushed environment.
    if (!@$env || @$env[-1] ne $args->[1]) {
        # a begin may have been hidden in the middle of a translated
        # buffer. FIXME: Just warn for now.
        warn wrap_ref_mod($self->{'ref'}, "po4a::tex",
                          dgettext("po4a", "unmatched end of environment '%s'"),
                          $args->[1]);
    } else {
        pop @$env;
    }

    my ($t,@e) = generic_command($self,$command,$variant,$args,$env,$no_wrap);

    print "($t, @$env)\n"
        if ($debug{'commands'} || $debug{'environments'});
    return ($t, @$env);
};
$separated_command{'begin'} = '*';

sub generic_command {
    my $self = shift;
    my ($command,$variant,$args,$env) = (shift,shift,shift,shift);
    my $no_wrap = shift;
    print "generic_command($command,$variant,@$args,@$env,$no_wrap)="
        if ($debug{'commands'} || $debug{'environments'});

    my ($t,@e)=("",());
    my $translated = "";

    # the number of arguments is checked during the extraction of the
    # arguments

    if (   (not (defined $separated_command{$command}))
        or $separated_command{$command} ne '+') {
        # Use the information from %command_parameters to only translate
        # the needed parameters
        $translated = "$ESCAPE$command$variant";
        # handle arguments
        my @arg_types = @{$command_parameters{$command}{'types'}};
        my @arg_translated = @{$command_parameters{$command}{'translated'}};
        my ($type, $opt);
        my @targs = @$args;
        my $count = 0;
        while (@targs) {
            $type = shift @targs;
            $opt  = shift @targs;
            my $have_to_be_translated = 0;
TEST_TYPE:
            if ($count >= scalar @arg_types) {
                # The number of arguments does not match,
                # and a variable number of arguments was not specified
                die wrap_ref_mod($self->{ref}, "po4a::tex",
                                 dgettext("po4a",
                                          "Wrong number of arguments for ".
                                          "the '%s' command.")."\n",
                                 $command);
            } elsif ($type eq $arg_types[$count]) {
                $have_to_be_translated = $arg_translated[$count];
                $count ++;
            } elsif ($type eq '{' and $arg_types[$count] eq '[') {
                # an optionnal argument was not provided,
                # try with the next argument.
                $count++;
                goto TEST_TYPE;
            } else {
                my $reason = dgettext("po4a",
                                      "An optional argument ".
                                      "was provided, but a mandatory one ".
                                      "is expected.");
                die wrap_ref_mod($self->{ref}, "po4a::tex",
                                 dgettext("po4a", "Command '%s': %s")."\n",
                                 $command, $reason);
            }
            if ($have_to_be_translated) {
                ($t, @e) = translate_buffer($self,$opt,$no_wrap,(@$env,$command.$type."#".$count.$type_end{$type}));
            } else {
                $t = $opt;
            }
            $translated .= $type.$t.$type_end{$type};
        }
    } else {
        # Translate the command with all its arguments joined
        my $tmp = "$ESCAPE$command$variant";
        my ($type, $opt);
        while (@$args) {
            $type = shift @$args;
            $opt  = shift @$args;
            $tmp .= $type.$opt.$type_end{$type};
        }
        @e = @$env;
        my $wrap = 1;
        $wrap = 0 if (defined $no_wrap and $no_wrap == 1);
        $translated = $self->translate($tmp,$self->{ref},
                                       @e?$e[-1]:"Plain text",
                                       "wrap" => $wrap);
    }

    print "($translated, @$env)\n"
        if ($debug{'commands'} || $debug{'environments'});
    return ($translated, @$env);
}

sub register_generic_command {
    if ($_[0] =~ m/^(.*),((\{_?\}|\[_?\]| _? )*)$/) {
        my $command = $1;
        my $arg_types = $2;
        if ($command =~ /^([-*+])(.*)$/) {
            $command = $2;
            $separated_command{$command}=$1;
        }
        my @types = ();
        my @translated = ();
        while (    defined $arg_types
               and length $arg_types
               and $arg_types =~ m/^(?:([\{\[ ])(_?)[\}\] ])(.*)$/) {
            push @types, $1;
            push @translated, ($2 eq "_")?1:0;
            $arg_types = $3;
        }
        $command_parameters{$command}{'types'} = \@types;
        $command_parameters{$command}{'translated'} = \@translated;
        $command_parameters{$command}{'nb_args'} = "";
        $commands{$command} = \&generic_command;
    } else {
        die wrap_mod("po4a::tex",
                     dgettext("po4a",
                              "register_generic_command: unsupported ".
                              "format: '%s'.")."\n",
                     $_[0]);
    }
}

########################################
#### DEFINITION OF THE ENVIRONMENTS ####
########################################
sub generic_environment {
    my $self = shift;
    my ($command,$variant,$args,$env) = (shift,shift,shift,shift);
    my $no_wrap = shift;
    print "generic_environment($command,$variant,$args,$env,$no_wrap)="
        if ($debug{'environments'});
    my ($t,@e)=("",());
    my $translated = "";

    # The first argument (the name of the environment is never translated)
    # For the others, @types and @translated are used.
    $translated = "$ESCAPE$command$variant";
    my @targs = @$args;
    my $type = shift @targs;
    my $opt  = shift @targs;
    my $new_env = $opt;
    $translated .= $type.$new_env.$type_end{$type};
    if (   (not (defined $separated_environment{$new_env}))
        or $separated_environment{$new_env} ne '+') {
        # Use the information from %command_parameters to only translate
        # the needed parameters
        my @arg_types = @{$environment_parameters{$new_env}{'types'}};
        my @arg_translated = @{$environment_parameters{$new_env}{'translated'}};

        my $count = 0;
        while (@targs) {
            $type = shift @targs;
            $opt  = shift @targs;
            my $have_to_be_translated = 0;
TEST_TYPE:
            if ($count >= scalar @arg_types) {
                die wrap_ref_mod($self->{ref}, "po4a::tex",
                                 dgettext("po4a",
                                          "Wrong number of arguments for ".
                                          "the '%s' command.")."\n",
                                 $command);
            } elsif ($type eq $arg_types[$count]) {
                $have_to_be_translated = $arg_translated[$count];
                $count ++;
            } elsif ($type eq '{' and $arg_types[$count] eq '[') {
                # an optionnal argument was not provided,
                # try with the next argument.
                $count++;
                goto TEST_TYPE;
            } else {
                my $reason = dgettext("po4a",
                                      "An optional argument ".
                                      "was provided, but a mandatory one ".
                                      "is expected.");
                die wrap_ref_mod($self->{ref}, "po4a::tex",
                                 dgettext("po4a", "Command '%s': %s")."\n",
                                 $command, $reason);
            }

            if ($have_to_be_translated) {
                ($t, @e) = translate_buffer($self,$opt,$no_wrap,(@$env,$new_env.$type."#".$count.$type_end{$type}));
            } else {
                $t = $opt;
            }
            $translated .= $type.$t.$type_end{$type};

        }
    } else {
        # Translate the \begin command with all its arguments joined
        my ($type, $opt);
        my $buf = $translated;
        while (@targs) {
            $type = shift @targs;
            $opt  = shift @targs;
            $buf .= $type.$opt.$type_end{$type};
        }
        @e = @$env;
        my $wrap = 1;
        $wrap = 0 if $no_wrap == 1;
        $translated = $self->translate($buf,$self->{ref},
                                       @e?$e[-1]:"Plain text",
                                       "wrap" => $wrap);
    }
    @e = (@$env, $new_env);

    print "($translated,@e)\n"
        if ($debug{'environments'});
    return ($translated,@e);
}


sub check_arg_count {
    my $self = shift;
    my $command = shift;
    my $args = shift;
    my @targs = @$args;
    my $check = 1;
    my @remainder = ();
    my $reason = "";
    my ($type, $arg);
    my @arg_types;

    if ($command eq 'begin') {
        $type = shift @targs;
        # The name of the environment is mandatory
        if (   (not defined $type)
            or ($type ne '{')) {
            $reason = dgettext("po4a",
                               "The first argument of \\begin is mandatory.");
            $check = 0;
        }
        my $env = shift @targs;
        if (not defined $environment_parameters{$env}) {
            die wrap_ref_mod($self->{ref},"po4a::tex",
                             dgettext("po4a", "unknown environment: '%s'"),
                             $env);
        }
        @arg_types = @{$environment_parameters{$env}{'types'}};
    } else {
        @arg_types = @{$command_parameters{$command}{'types'}};
    }

    my $count = 0;
    while ($check and @targs) {
        $type = shift @targs;
        $arg  = shift @targs;
TEST_TYPE:
        if ($count >= scalar @arg_types) {
            # Too many arguments some will remain
            @remainder = ($type, $arg, @targs);
            last;
        } elsif ($type eq $arg_types[$count]) {
            $count ++;
        } elsif ($type eq '{' and $arg_types[$count] eq '[') {
            # an optionnal argument was not provided,
            # try with the next argument.
            $count++;
            goto TEST_TYPE;
        } else {
            $check = 0;
            $reason = dgettext("po4a",
                               "An optional argument was ".
                               "provided, but a mandatory one is expected.");
        }
    }

    return ($check, $reason, \@remainder);
}

sub register_generic_environment {
    print "register_generic_environment($_[0])\n"
        if ($debug{'environments'});
    if ($_[0] =~ m/^(.*),((?:\{_?\}|\[_?\])*)$/) {
        my $env = $1;
        my $arg_types = $2;
        if ($env =~ /^([+])(.*)$/) {
            $separated_environment{$2} = $1;
            $env = $2;
        }
        my @types = ();
        my @translated = ();
        while (    defined $arg_types
               and length $arg_types
               and $arg_types =~ m/^(?:([\{\[])(_?)[\}\]])(.*)$/) {
            push @types, $1;
            push @translated, ($2 eq "_")?1:0;
            $arg_types = $3;
        }
        $environment_parameters{$env} = {
            'types'      => \@types,
            'translated' => \@translated
        };
        $environments{$env} = \&generic_environment;
    }
}

sub register_verbatim_environment {
    my $env = shift;
    $no_wrap_environments .= " $env";
    $verbatim_environments .= " $env";
    $RE_VERBATIM = "\\\\begin\\{(?:".
                   join("|", split(/ /, $verbatim_environments)).
                   ")\\*?\\}";
    register_generic_environment("$env,")
        unless (defined $environments{$env});
}

####################################
### INITIALIZATION OF THE PARSER ###
####################################
sub initialize {
    my $self = shift;
    my %options = @_;

    $self->{options}{'definitions'}='';
    $self->{options}{'exclude_include'}='';
    $self->{options}{'no_wrap'}='';
    $self->{options}{'verbatim'}='';
    $self->{options}{'debug'}='';
    $self->{options}{'verbose'}='';

    %debug = ();
    # FIXME: %commands and %separated_command should also be restored to their
    #        default values.

    foreach my $opt (keys %options) {
        if ($options{$opt}) {
            die wrap_mod("po4a::tex",
                         dgettext("po4a", "Unknown option: %s"), $opt)
                unless exists $self->{options}{$opt};
            $self->{options}{$opt} = $options{$opt};
        }
    }

    if ($options{'debug'}) {
        foreach ($options{'debug'}) {
            $debug{$_} = 1;
        }
    }

    if ($options{'exclude_include'}) {
        foreach (split(/:/, $options{'exclude_include'})) {
            push  @exclude_include, $_;
        }
    }

    if ($options{'no_wrap'}) {
        foreach (split(/,/, $options{'no_wrap'})) {
            $no_wrap_environments .= " $_";
            register_generic_environment("$_,")
                unless (defined $environments{$_});
        }
    }

    if ($options{'verbatim'}) {
        foreach (split(/,/, $options{'verbatim'})) {
            register_verbatim_environment($_);
        }
    }

    if ($options{'definitions'}) {
        $self->parse_definition_file($options{'definitions'})
    }
}

=head1 STATUS OF THIS MODULE

This module needs more tests.

It was tested on a book and with the Python documentation.

=head1 TODO LIST

=over 4

=item Automatic detection of new commands

The TeX module could parse the newcommand arguments and try to guess the
number of arguments, their type and whether or not they should be
translated.

=item Translation of the environment separator

When \item is used as an environment separator, the item argument is
attached to the following string.

=item Some commands should be added to the environment stack

These commands should be specified by couples.
This could allow to specify commands beginning or ending a verbatim
environment.

=item Others

Various other points are tagged TODO in the source.

=back

=head1 KNOWN BUGS

Various points are tagged FIXME in the source.

=head1 SEE ALSO

L<Locale::Po4a::LaTeX(3pm)|Locale::Po4a::LaTeX>,
L<Locale::Po4a::TransTractor(3pm)|Locale::Po4a::TransTractor>,
L<po4a(7)|po4a.7>

=head1 AUTHORS

 Nicolas François <nicolas.francois@centraliens.net>

=head1 COPYRIGHT AND LICENSE

Copyright 2004, 2005 by Nicolas FRANÇOIS <nicolas.francois@centraliens.net>.

This program is free software; you may redistribute it and/or modify it
under the terms of GPL (see the COPYING file).

=cut

1;
