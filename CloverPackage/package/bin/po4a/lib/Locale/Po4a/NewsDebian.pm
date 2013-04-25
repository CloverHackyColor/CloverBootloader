#!/usr/bin/perl -w

# Po4a::NewsDebian.pm
#
# extract and translate translatable strings from a NEWS.Debian documents
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
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
########################################################################

=encoding UTF-8

=head1 NAME

Locale::Po4a::NewsDebian - convert NEWS.Debian documents from/to PO files

=head1 DESCRIPTION

The po4a (PO for anything) project goal is to ease translations (and more
interestingly, the maintenance of translations) using gettext tools on
areas where they were not expected like documentation.

Locale::Po4a::NewsDebian is a module to help the translation of the
NEWS.Debian files into other [human] languages. Those files are where
maintainer are supposed to write the important news about their package.

=head1 OPTIONS ACCEPTED BY THIS MODULE

NONE.

=head1 STATUS OF THIS MODULE

Not tested.

A finer split of the entries may be preferable (search for /^ */, for
example), but this version is more robust and NEWS.Debian entries are not
supposed to change that often.

=cut

package Locale::Po4a::NewsDebian;

use 5.006;
use strict;
use warnings;

require Exporter;
use vars qw(@ISA @EXPORT);
@ISA = qw(Locale::Po4a::TransTractor);
@EXPORT = qw();

use Locale::Po4a::TransTractor;
use Locale::Po4a::Common;


sub initialize {}

sub parse {
    my $self = shift;

    my ($blanklines)=(""); # We want to preserve the blank lines inside the entry, and strip the extrem ones

    my ($body)=""; # the accumulated paragraph
    my ($bodyref)="";
    my ($bodytype)="";

    my ($line,$lref);

    # main loop
    ($line,$lref)=$self->shiftline();
    print "seen >>$line<<\n" if $self->debug();
    while (defined($line)) {

	# Begining of an entry
	if ($line =~ m/^(\w[-+0-9a-z.]*) \(([^\(\) \t]+)\)((\s+[-0-9a-z]+)+)\;/i) {

	    die wrap_ref_mod($lref, "po4a::newsdebian", dgettext("po4a", "Begin of a new entry before the end of previous one"))
	      if (length ($body));

	    $self->pushline($line."\n");

	    # Signature of this entry
	    $bodyref = $lref;
	    $bodytype = $line;

	    # eat all leading empty lines
	    ($line,$lref)=$self->shiftline();
	    while (defined($line) && $line =~ m/^\s*$/) {
		print "Eat >>$line<<\n" if $self->debug();
		($line,$lref)=$self->shiftline();
	    }
	    # ups, ate one line too much. Put it back.
	    $self->unshiftline($line,$lref);


	    # get ready to read the entry (cleanups)
	    $blanklines = "";

	# End of current entry
	} elsif ($line =~ m/^ \-\- (.*) <(.*)>  .*$/) { #((\w+\,\s*)?\d{1,2}\s+\w+\s+\d{4}\s+\d{1,2}:\d\d:\d\d\s+[-+]\d{4}(\s+\([^\\\(\)]\))?) *$/) {

	    $self->translate($body, $bodyref, $bodytype,
		             wrap=>0);
	    $body="";

	# non-specific line
	} else {

	    if ($line =~ /^\s*$/) {
		$blanklines .= "$line";
	    } else {
		$body .= $blanklines.$line;
		$blanklines = "";
	    }
	}

	($line,$lref)=$self->shiftline();
	print "seen >>".($line || '')."<<\n" if $self->debug();
    }
}

1;

=head1 AUTHORS

This module is loosely inspired from /usr/lib/dpkg/parsechangelog/debian, which is:

 Copyright (C) 1996 Ian Jackson.  This is free software; see the GNU
 General Public Licence version 2 or later for copying conditions.  There
 is NO warranty.

The adaptation for po4a was done by:

 Martin Quinson (mquinson#debian.org)

=head1 COPYRIGHT AND LICENSE

 Copyright (c) 1996 by Ian Jackson.
 Copyright 2005 by SPI, inc.

This program is free software; you may redistribute it and/or modify it
under the terms of GPL (see the COPYING file).
