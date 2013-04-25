#!/usr/bin/perl -w

# Po4a::Debconf.pm
#
# extract and translate translatable strings from debconf templates
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

Locale::Po4a::Debconf - convert debconf templates from/to PO files

=head1 DESCRIPTION

The po4a (PO for anything) project goal is to ease translations (and more
interestingly, the maintenance of translations) using gettext tools on
areas where they were not expected like documentation.

Locale::Po4a::Debconf is a module to help the translation of the debconf
templates into other [human] languages.

=head1 OPTIONS ACCEPTED BY THIS MODULE

NONE.

=head1 STATUS OF THIS MODULE

Not tested.


DO NOT USE THIS MODULE TO PRODUCE TEMPLATES. It's only good to extract data.


=cut

# Note that the following works. It may help to write a multi-translate

# sub toto {
#     do shift;
# }
# toto({print "ok"});


package Locale::Po4a::Debconf;

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

    my ($line,$lref);

    my ($field, $value, $extended,$ref,$type)=('', '', '', '','');
    my $verb = 0; # whether we are in verbatim mode

    my $escape = sub {
	my $str=shift;
	$str =~ s/"/\\"/g;
	return $str;
    };

    # function in charge of pushing the accumulated material to output
    my $handle_field = sub {
	my $field=shift;
	my $value=shift;
	my $extended=shift;
	my $ref = shift;
	my $type = shift;

	$field =~ s/^(_*)(.*)$/$2/;
	my $undercount = length($1) || 0; # number of _ leading the field name

	# Only one leading _: regular translated field
	if ($undercount == 1) {

	    # the untranslated field
	    $self->pushline("$field: $value");
	    map {$self->pushline(' '.($_||'.'))} split (/\n/,$extended);


	    my $eval='$self->pushline("'.$field.'[FIXME:LANGCODE.ENCODING]: "'; # what to multi-eval
	    $eval .= '.$self->translate("'.$escape->($value)."\",\"$ref\",\"$type/$field\",wrap=>1)".'."\n".'."\n";

	    my $count = 0;
	    foreach my $para (split(/\n\n/, $extended)) {
		my $wrap = 1;
		if ($para =~ /(^|\n)\s/m) {
		    $wrap = 0;
		}
		$eval .= ($count?'.':'');
		$count ++;
		$eval .= '$self->translate("'.$escape->($para)."\",\"$ref\",\"$type/$field\[$count\]\",wrap=>$wrap)"."\n";
	    }

	    $eval .= ")\n";
	    print STDERR $eval if $self->debug();
	    eval $eval;
	    print STDERR "XXXXXXXXXXXXXXXXX\n" if $self->debug();


	# two leading _: split on coma and multi-translate each part. No extended value.
	} elsif ($undercount == 2) {
	    $self->pushline("$field: $value"); # the untranslated field

	    my $eval='$self->pushline("'.$field.'FIXME[LANGCODE]: "'; # what to multi-eval

	    my $first = 1;
	    for my $part (split(/(?<!\\), */, $value, 0))
	    {
		$part =~ s/\\,/,/g;
		$eval .= ($first?'':'.", "').'.$self->translate("'.$escape->($part)."\",\"$ref\",\"$type/$field chunk\",wrap=>1)";
		$first = 0;
	    }
	    $eval .= ")\n";

	    print $eval if $self->debug();
	    eval $eval;

	# no leading _: don't touch it
	} else {
	    $self->pushline("$field: $value");
	    map {$self->pushline(' '.($_||'.'))} split (/\n/,$extended);
	}
    };

    # main loop
    ($line,$lref)=$self->shiftline();

    while (defined($line)) {
	# a new field (within a stanza)
	if ($line=~/^([-_.A-Za-z0-9]*):\s?(.*)/) {

	    $handle_field->($field, $value, $extended, $ref,$type); # deal with previously accumulated
	    ($field, $value, $extended,$verb)=('', '', '', 0);

	    $field=$1;
	    $value=$2;
	    $value=~s/\s*$//;
	    $extended='';
	    $ref=$lref;

	    $type = $value if $field eq 'Type';

	    die wrap_mod("po4a::debconf", dgettext("po4a", "Translated field in master document: %s"), $field)
	      if $field =~ m/-/;

	# paragraph separator within extended value
	} elsif ($line=~/^\s\.$/) {
	    $extended.="\n\n";

	# continuation of extended value
	} elsif ($line=~/^\s(.*)/) {

	    my $bit=$1;
	    $verb = 1 if ($bit =~ m/^\s/);

	    $bit=~s/\s*$//;

	    $extended .= ($verb ? "\n" : ' ') if length $extended && $extended !~ /[\n ]$/;
	    $extended .= $bit.($verb ? "\n" : "");

	# this may be an empty line closing the stanza, a comment or even a parse error (if file not DebConf-clean).
	} else {

	    $handle_field->($field, $value, $extended, $ref,$type);
	    ($field, $value, $extended,$verb)=('', '', '', 0);

	    $self->pushline($line);

	}

	($line,$lref)=$self->shiftline();
    }

    $handle_field->($field, $value, $extended, $ref,$type);
}

1;

=head1 AUTHORS

This module is loosely inspired from both po-debconf and debconf code. The
adaptation for po4a was done by:

 Martin Quinson (mquinson#debian.org)

=head1 COPYRIGHT AND LICENSE

 Copyright 2005 by SPI, inc.

This program is free software; you may redistribute it and/or modify it
under the terms of GPL (see the COPYING file).
