# Locale::Po4a::Common -- Common parts of the po4a scripts and utils
#
# Copyright 2005 by Jordi Vilalta <jvprat@gmail.com>
#
# This program is free software; you may redistribute it and/or modify it
# under the terms of GPL (see COPYING).
#
# This module has common utilities for the various scripts of po4a

=encoding UTF-8

=head1 NAME

Locale::Po4a::Common - common parts of the po4a scripts and utils

=head1 DESCRIPTION

Locale::Po4a::Common contains common parts of the po4a scripts and some useful
functions used along the other modules.

In order to use Locale::Po4a programatically, one may want to disable
the use of Text::WrapI18N, by writing e.g.

    use Locale::Po4a::Common qw(nowrapi18n);
    use Locale::Po4a::Text;

instead of:

    use Locale::Po4a::Text;

Ordering is important here: as most Locale::Po4a modules themselves
load Locale::Po4a::Common, the first time this module is loaded
determines whether Text::WrapI18N is used.

=cut

package Locale::Po4a::Common;

require Exporter;
use vars qw(@ISA @EXPORT);
@ISA = qw(Exporter);
@EXPORT = qw(wrap_msg wrap_mod wrap_ref_mod textdomain gettext dgettext);

use 5.006;
use strict;
use warnings;

sub import {
    my $class=shift;

    my $wrapi18n=1;
    if (exists $_[0] && defined $_[0] && $_[0] eq 'nowrapi18n') {
        shift;
        $wrapi18n=0;
    }
    $class->export_to_level(1, $class, @_);

    return if defined &wrapi18n;

    if ($wrapi18n && -t STDERR && -t STDOUT && eval { require Text::WrapI18N }) {

        # Don't bother determining the wrap column if we cannot wrap.
        my $col=$ENV{COLUMNS};
        if (!defined $col) {
            my @term=eval "use Term::ReadKey; Term::ReadKey::GetTerminalSize()";
            $col=$term[0] if (!$@);
            # If GetTerminalSize() failed we will fallback to a safe default.
            # This can happen if Term::ReadKey is not available
            # or this is a terminal-less build or such strange condition.
        }
        $col=76 if (!defined $col);

        eval ' use Text::WrapI18N qw($columns);
               $columns = $col;
             ';

        eval ' sub wrapi18n($$$) { Text::WrapI18N::wrap($_[0],$_[1],$_[2]) } '
    } else {
        # If we cannot wrap, well, that's too bad. Survive anyway.
        eval ' sub wrapi18n($$$) { $_[0].$_[2] } '
    }
}

sub min($$) {
    return $_[0] < $_[1] ? $_[0] : $_[1];
}

=head1 FUNCTIONS

=head2 Showing output messages

=over

=item

show_version($)

Shows the current version of the script, and a short copyright message. It
takes the name of the script as an argument.

=cut

sub show_version {
    my $name = shift;

    print sprintf(gettext(
        "%s version %s.\n".
        "written by Martin Quinson and Denis Barbier.\n\n".
        "Copyright (C) 2002, 2003, 2004 Software in the Public Interest, Inc.\n".
        "This is free software; see source code for copying\n".
        "conditions. There is NO warranty; not even for\n".
        "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."
        ), $name, $Locale::Po4a::TransTractor::VERSION)."\n";
}

=item

wrap_msg($@)

This function displays a message the same way than sprintf() does, but wraps
the result so that they look nice on the terminal.

=cut

sub wrap_msg($@) {
    my $msg = shift;
    my @args = @_;

    return wrapi18n("", "", sprintf($msg, @args))."\n";
}

=item

wrap_mod($$@)

This function works like wrap_msg(), but it takes a module name as the first
argument, and leaves a space at the left of the message.

=cut

sub wrap_mod($$@) {
    my ($mod, $msg) = (shift, shift);
    my @args = @_;

    $mod .= ": ";
    my $spaces = " " x min(length($mod), 15);
    return wrapi18n($mod, $spaces, sprintf($msg, @args))."\n";
}

=item

wrap_ref_mod($$$@)

This function works like wrap_msg(), but it takes a file:line reference as the
first argument, a module name as the second one, and leaves a space at the left
of the message.

=back

=cut

sub wrap_ref_mod($$$@) {
    my ($ref, $mod, $msg) = (shift, shift, shift);
    my @args = @_;

    if (!$mod) {
        # If we don't get a module name, show the message like wrap_mod does
        return wrap_mod($ref, $msg, @args);
    } else {
        $ref .= ": ";
        my $spaces = " " x min(length($ref), 15);
        $msg = "$ref($mod)\n$msg";
        return wrapi18n("", $spaces, sprintf($msg, @args))."\n";
    }
}

=head2 Wrappers for other modules

=over

=item

Locale::Gettext

When the Locale::Gettext module cannot be loaded, this module provide dummy
(empty) implementation of the following functions. In that case, po4a
messages won't get translated but the program will continue to work.

If Locale::gettext is present, this wrapper also calls
setlocale(LC_MESSAGES, "") so callers don't depend on the POSIX module
either.

=over

=item

bindtextdomain($$)

=item

textdomain($)

=item

gettext($)

=item

dgettext($$)

=back

=back

=cut

BEGIN {
    if (eval { require Locale::gettext }) {
       import Locale::gettext;
       require POSIX;
       POSIX::setlocale(&POSIX::LC_MESSAGES, '');
    } else {
       eval '
           sub bindtextdomain($$) { }
           sub textdomain($) { }
           sub gettext($) { shift }
           sub dgettext($$) { return $_[1] }
       '
    }
}

1;
__END__

=head1 AUTHORS

 Jordi Vilalta <jvprat@gmail.com>

=head1 COPYRIGHT AND LICENSE

Copyright 2005 by SPI, inc.

This program is free software; you may redistribute it and/or modify it
under the terms of GPL (see the COPYING file).

=cut
