#!/usr/bin/perl

# Po4a::Xhtml.pm
#
# extract and translate translatable strings from XHTML documents.
#
# This code extracts plain text from tags and attributes from strict XHTML
# documents.
#
# Copyright (c) 2005 by Yves Rütschlé <po4a@rutschle.net>
# Copyright (c) 2007-2008 by Nicolas François <nicolas.francois@centraliens.net>
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

Locale::Po4a::Xhtml - convert XHTML documents from/to PO files

=head1 DESCRIPTION

The po4a (PO for anything) project goal is to ease translations (and more
interestingly, the maintenance of translations) using gettext tools on
areas where they were not expected like documentation.

Locale::Po4a::Xhtml is a module to help the translation of XHTML documents into
other [human] languages.

=head1 OPTIONS ACCEPTED BY THIS MODULE

These are this module's particular options:

=over 4

=item B<includessi>[B<=>I<rootpath>]

Include files specified by an include SSI (Server Side Includes) element
(e.g. <!--#include virtual="/foo/bar.html" -->).

B<Note:> You should use it only for static files.

An additional I<rootpath> parameter can be specified. It specifies the root
path to find files included by a B<virtual> attribute.

=back

=head1 STATUS OF THIS MODULE

This module is fully functional, as it relies in the L<Locale::Po4a::Xml>
module. This only defines the translatable tags and attributes.

"It works for me", which means I use it successfully on my personal Web site.
However, YMMV: please let me know if something doesn't work for you. In
particular, tables are getting no testing whatsoever, as we don't use them.

=head1 SEE ALSO

L<Locale::Po4a::TransTractor(3pm)>, L<Locale::Po4a::Xml(3pm)>, L<po4a(7)|po4a.7>

=head1 AUTHORS

 Yves Rütschlé <po4a@rutschle.net>
 Nicolas François <nicolas.francois@centraliens.net>

=head1 COPYRIGHT AND LICENSE

 Copyright (c) 2004 by Yves Rütschlé <po4a@rutschle.net>
 Copyright (c) 2007-2008 by Nicolas François <nicolas.francois@centraliens.net>

This program is free software; you may redistribute it and/or modify it
under the terms of GPL (see the COPYING file).

=cut

package Locale::Po4a::Xhtml;

use 5.006;
use strict;
use warnings;

use Locale::Po4a::Xml;
use vars qw(@tag_types);
*tag_types = \@Locale::Po4a::Xml::tag_types;

use Locale::Po4a::Common;
use Carp qw(croak);

use vars qw(@ISA);
@ISA = qw(Locale::Po4a::Xml);

sub tag_extract_SSI {
        my ($self,$remove)=(shift,shift);
        my ($eof,@tag)=$self->get_string_until("-->",
                                               {include=>1,
                                                remove=>$remove,
                                                unquoted=>1});
        my ($t,$r) = @tag;
        if ($t =~ m/<!--#include (file|virtual)="(.*?)"\s-->/s) {
                my $includefile;
                if ($1 eq "file") {
                        $includefile = ".";
                } else {
                        $includefile = $self->{options}{'includessi'};
                }
                $includefile .= $2;
                if (!$remove) {
                        $self->get_string_until("-->",
                                                {include=>1,
                                                 remove=>1,
                                                 unquoted=>1});
                }
                my $linenum=0;
                my @include;

                open (my $in, $includefile)
                    or croak wrap_mod("po4a::xml",
                                     dgettext("po4a", "Can't read from %s: %s"),
                                      $includefile, $!);
                while (defined (my $includeline = <$in>)) {
                        $linenum++;
                        my $includeref=$includefile.":$linenum";
                        push @include, ($includeline,$includeref);
                }
                close $in
                    or croak wrap_mod("po4a::xml",
                           dgettext("po4a", "Can't close %s after reading: %s"),
                                      $includefile, $!);

                while (@include) {
                        my ($ir, $il) = (pop @include, pop @include);
                        $self->unshiftline($il,$ir);
                }
                $t =~ s/<!--#include/<!-- SSI included by po4a: /;
                $self->unshiftline($t, $r);
        }
        return ($eof,@tag);
}

sub initialize {
        my $self = shift;
        my %options = @_;

        $self->{options}{'includessi'}='';

        $self->SUPER::initialize(%options);

        $self->{options}{'wrap'}=1;
        $self->{options}{'doctype'}=$self->{options}{'doctype'} || 'html';

        # Default tags are translated (text rewrapped), and introduce a
        # break.
        # The following list indicates the list of tags which should be
        # translated without rewrapping.
        $self->{options}{'_default_translated'}.='
                W<pre>
        ';

        # The following list indicates the list of tags which should be
        # translated inside the current block, whithout introducing a
        # break.
        $self->{options}{'_default_inline'}.='
                <a>
                <abbr>
                <acronym>
                <b>
                <big>
                <bdo>
                <button>
                <cite>
                <code>
                <del>
                <dfn>
                <em>
                <i>
                <ins>
                <input>
                <kbd>
                <label>
                <object>
                <q>
                <samp>
                <select>
                <small>
                <span>
                <strong>
                <sub>
                <sup>
                <textarea>
                <tt>
                <u>
                <var>
        ';

        # Ignored tags: <img>
        # Technically, <img> is an inline tag, but setting it as such is
        # annoying, and not usually useful, unless you use images to
        # write text (in which case you have bigger problems than this
        # program not inlining img: you now have to translate all your
        # images. That'll teach you).
        # If you choose to translate images, you may also want to set
        # <map> as placeholder and <area> as inline.

        $self->{options}{'_default_attributes'}.='
                alt
                lang
                title
                ';
        $self->treat_options;

        if (    defined $self->{options}{'includessi'}
            and length $self->{options}{'includessi'}) {
                foreach (@tag_types) {
                        if ($_->{beginning} eq "!--#") {
                                $_->{f_extract} = \&tag_extract_SSI;
                        }
                }
                # FIXME: the directory may be named "1" ;(
                if ($self->{options}{'includessi'} eq "1") {
                        $self->{options}{'includessi'} = ".";
                }
        }
}
