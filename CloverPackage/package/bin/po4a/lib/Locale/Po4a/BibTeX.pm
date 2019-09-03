#!/usr/bin/perl -w

# Po4a::BibTeX.pm
#
# extract and translate translatable strings from BibTeX documents
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

Locale::Po4a::BibTeX - convert BibTeX documents from/to PO files

=head1 DESCRIPTION

The po4a (PO for anything) project goal is to ease translations (and more
interestingly, the maintenance of translations) using gettext tools on
areas where they were not expected like documentation.

Locale::Po4a::BibTeX is a module to help the translation of
bibliographies in the BibTeX format into other [human] languages.

Fields values are extracted and proposed for translation.

=head1 OPTIONS ACCEPTED BY THIS MODULE

NONE.

=head1 STATUS OF THIS MODULE

It is a very simple module, but still young.

=cut

package Locale::Po4a::BibTeX;

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
    my ($line,$ref);
    my $paragraph="";
    my $field="";
    my $id="";
    my $wrapped_mode = 1;
    ($line,$ref)=$self->shiftline();
    while (defined($line)) {
        chomp($line);
#print "tutu: '$line'\n";
        $self->{ref}="$ref";
        if (    $id eq ""
            and $line =~ m/^\@.*?\s*\{\s*(.*),\s*$/) {
            $id = $1;
            $self->pushline( $line."\n" );
        } elsif (    $id ne ""
                 and $field eq ""
                 and $line =~ m/^((.*?)\s*=\s*)([^ "{].*?)(\s*,?\s*)$/) {
            my $end=(defined $4)?$4:"";
            $self->pushline( $1.$self->translate($3,
                                                 $self->{ref},
                                                 "$2 ($id)",
                                                 "wrap" => 1).$end."\n" );
            $field = "";
            $paragraph = "";
        } elsif (    $id ne ""
                 and $field eq ""
                 and $line =~ m/^((.*?)\s*=\s*)(.*)$/) {
            $field = $2;
            $paragraph = $3."\n";
            $self->pushline( $1 );
        } elsif ($field ne "") {
            $paragraph.="$line\n";
        } elsif ($line =~ m/^\s*(\%.*)?$/) {
            $self->pushline( $line."\n" );
        } elsif ($line =~ m/^\s*\}\s*$/) {
            $self->pushline( $line."\n" );
            $id="";
        } else {
            print "unsupported line: '$line'\n";
        }
        if (   $paragraph =~ m/^(\s*\{)(.*)(\}\s*,?\s*)$/s
            or $paragraph =~ m/^(\s*")(.*)("\s*,?\s*)$/s
            or $paragraph =~ m/^(\s*)([^ "{].*)(\s*,?\s*)$/s) {
            $self->pushline( $1.$self->translate($2,
                                                 $self->{ref},
                                                 "$field ($id)",
                                                 "wrap" => 1).$3);
            $field="";
            $paragraph="";
        }
        ($line,$ref)=$self->shiftline();
    }
        if (   $paragraph =~ m/^(\s*\{)(.*)(\}\s*,?\s*)$/s
            or $paragraph =~ m/^(\s*")(.*)("\s*,?\s*)$/s
            or $paragraph =~ m/^(\s*)(.*)(\s*,?\s*)$/s) {
            $self->pushline( $self->translate($1,
                                              $self->{ref},
                                              "$field ($id)",
                                              "wrap" => 1).$2);
            $field="";
            $paragraph="";
        }
}

sub do_paragraph {
    my ($self, $paragraph, $wrap) = (shift, shift, shift);
    $self->pushline( $self->translate($paragraph,
                                      $self->{ref},
                                      "Plain text",
                                      "wrap" => $wrap) );
}

1;

=head1 AUTHORS

 Nicolas François <nicolas.francois@centraliens.net>

=head1 COPYRIGHT AND LICENSE

 Copyright 2006 by Nicolas FRANÇOIS <nicolas.francois@centraliens.net>.

This program is free software; you may redistribute it and/or modify it
under the terms of GPL (see the COPYING file).
