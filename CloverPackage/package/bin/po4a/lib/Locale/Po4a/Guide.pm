#!/usr/bin/perl

# Po4a::Guide.pm
#
# extract and translate translatable strings from Guide XML documents.
#
# This code extracts plain text from tags and attributes on Guide XML
# documents.
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

Locale::Po4a::Guide - convert Guide XML documents from/to PO files

=head1 DESCRIPTION

The po4a (PO for anything) project goal is to ease translations (and more
interestingly, the maintenance of translations) using gettext tools on
areas where they were not expected like documentation.

Locale::Po4a::Guide is a module to help in the translation of the Gentoo
Linux documentation in the Guide XML format into other [human] languages.

This format is documented here: http://www.gentoo.org/doc/en/xml-guide.xml

=head1 STATUS OF THIS MODULE

This module is fully functional, as it relies in the L<Locale::Po4a::Xml>
module. This only defines the translatable tags and attributes.

The only known issue is that it doesn't include files with the <include
href="..."> tag, but you can translate all those files alone, and it's usually
better to have them separated.

=head1 SEE ALSO

L<Locale::Po4a::TransTractor(3pm)>, L<Locale::Po4a::Xml(3pm)>, L<po4a(7)|po4a.7>

=head1 AUTHORS

 Jordi Vilalta <jvprat@gmail.com>

=head1 COPYRIGHT AND LICENSE

Copyright (c) 2004 by Jordi Vilalta  <jvprat@gmail.com>

This program is free software; you may redistribute it and/or modify it
under the terms of GPL (see the COPYING file).

=cut

package Locale::Po4a::Guide;

use 5.006;
use strict;
use warnings;

use Locale::Po4a::Xml;

use vars qw(@ISA);
@ISA = qw(Locale::Po4a::Xml);

sub initialize {
    my $self = shift;
    my %options = @_;

#TODO: <include href="..."> includes a file
    $self->SUPER::initialize(%options);
    $self->{options}{'_default_translated'}.='
        w<abstract>
        <author>
        <b>
        <brite>
        <c>
        <codenote>
        <comment>
        <const>
        <date>
        w<dd>
        w<dt>
        <e>
        <i>
        <ident>
        w<impo>
        <keyword>
        w<li>
        <mail>
        w<note>
        w<p>
        <path>
        W<pre>
        <stmt>
        <sub>
        w<subtitle>
        w<summary>
        <sup>
        w<th>
        w<ti>
        w<title>
        <uri>
        <var>
        <version>
        w<warn>';
    $self->{options}{'_default_attributes'}.='
        <author>title
        <figure>caption
        <figure>link
        <figure>short
        <guide>lang
        <guide>link
        <p>by
        <pre>caption';
    $self->{options}{'_default_inline'}.='
        <b>
        <brite>
        <c>
        <const>
        <e>
        <i>
        <ident>
        <img>
        <keyword>
        <mail>
        <path>
        <stmt>
        <sub>
        <sup>
        <uri>
        <var>';
    $self->treat_options;
}
