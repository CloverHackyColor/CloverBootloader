#!/usr/bin/perl

# Po4a::Dia.pm
#
# extract and translate translatable strings from Dia diagrams.
#
# This code extracts plain text from string tags on uncompressed Dia
# diagrams.
#
# Copyright (c) 2004 by Jordi Vilalta  <jvprat@gmail.com>
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

Locale::Po4a::Dia - convert uncompressed Dia diagrams from/to PO files

=head1 DESCRIPTION

The po4a (PO for anything) project goal is to ease translations (and more
interestingly, the maintenance of translations) using gettext tools on
areas where they were not expected like documentation.

Locale::Po4a::Dia is a module to help the translation of diagrams in the
uncompressed Dia format into other [human] languages.

You can get Dia (the graphical editor for these diagrams) from:
  http://www.gnome.org/projects/dia/

=head1 TRANSLATING WITH PO4A::DIA

This module only translates uncompressed Dia diagrams.  You can save your
uncompressed diagrams with Dia itself, unchecking the "Compress diagram
files" at the "Save Diagram" dialog.

Another way is to uncompress the dia files from command line with:
  gunzip < original.dia > uncompressed.dia

=head1 STATUS OF THIS MODULE

This module is fully functional, as it relies in the L<Locale::Po4a::Xml>
module. This only defines the translatable tags (E<lt>dia:stringE<gt>), and
filters the internal strings (the content of the E<lt>dia:diagramdataE<gt>
tag), not interesting for translation.

=head1 SEE ALSO

L<Locale::Po4a::TransTractor(3pm)>, L<Locale::Po4a::Xml(3pm)>, L<po4a(7)|po4a.7>

=head1 AUTHORS

 Jordi Vilalta <jvprat@gmail.com>

=head1 COPYRIGHT AND LICENSE

Copyright (c) 2004 by Jordi Vilalta  <jvprat@gmail.com>

This program is free software; you may redistribute it and/or modify it
under the terms of GPL (see the COPYING file).

=cut

package Locale::Po4a::Dia;

use 5.006;
use strict;
use warnings;

use Locale::Po4a::Xml;

use vars qw(@ISA);
@ISA = qw(Locale::Po4a::Xml);

sub initialize {
    my $self = shift;
    my %options = @_;

    $self->SUPER::initialize(%options);
    $self->{options}{'nostrip'}=1;
    $self->{options}{'_default_translated'}.=' <dia:string>';
    $self->treat_options;
}

sub found_string {
    my ($self,$text,$ref,$options)=@_;
    return $text if $text =~ m/^\s*$/s;

    #We skip the paper type string
    if ( $self->get_path() !~ /<dia:diagramdata>/ ) {
        $text =~ /^#(.*)#$/s;
        $text = "#".$self->translate($1,$ref,"String",
            'wrap'=>$self->{options}{'wrap'})."#";
    }

    return $text;
}
