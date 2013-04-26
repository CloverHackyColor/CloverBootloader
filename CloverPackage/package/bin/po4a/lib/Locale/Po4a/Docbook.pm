#!/usr/bin/perl
# aptitude: cmdsynopsis => missing removal of leading spaces

# Po4a::Docbook.pm
#
# extract and translate translatable strings from DocBook XML documents.
#
# This code extracts plain text from tags and attributes on DocBook XML
# documents.
#
# Copyright (c) 2004 by Jordi Vilalta  <jvprat@gmail.com>
# Copyright (c) 2007-2009 by Nicolas François <nicolas.francois@centraliens.net>
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

Locale::Po4a::Docbook - convert DocBook XML documents from/to PO files

=head1 DESCRIPTION

The po4a (PO for anything) project goal is to ease translations (and more
interestingly, the maintenance of translations) using gettext tools on
areas where they were not expected like documentation.

Locale::Po4a::Docbook is a module to help the translation of DocBook XML
documents into other [human] languages.

Please note that this module is still under heavy development, and not
distributed in official po4a release since we don't feel it to be mature
enough. If you insist on trying, check the CVS out.

=head1 STATUS OF THIS MODULE

This module is fully functional, as it relies in the L<Locale::Po4a::Xml>
module. This only defines the translatable tags and attributes.

The only known issue is that it doesn't handle entities yet, and this includes
the file inclusion entities, but you can translate most of those files alone
(except the typical entities files), and it's usually better to maintain them
separated.

=head1 SEE ALSO

L<Locale::Po4a::TransTractor(3pm)>, L<Locale::Po4a::Xml(3pm)>, L<po4a(7)|po4a.7>

=head1 AUTHORS

 Jordi Vilalta <jvprat@gmail.com>

=head1 COPYRIGHT AND LICENSE

 Copyright (c) 2004 by Jordi Vilalta  <jvprat@gmail.com>
 Copyright (c) 2007-2009 by Nicolas François <nicolas.francois@centraliens.net>

This program is free software; you may redistribute it and/or modify it
under the terms of GPL (see the COPYING file).

=cut

package Locale::Po4a::Docbook;

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
    $self->{options}{'wrap'}=1;
    $self->{options}{'doctype'}=$self->{options}{'doctype'} || 'docbook xml';

# AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA

    # abbrev; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <abbrev>";
    $self->{options}{'_default_inline'} .= " <abbrev>";

    # abstract; does not contain text; Formatted as a displayed block
    $self->{options}{'_default_untranslated'} .= " <abstract>";
    $self->{options}{'_default_break'} .= " <abstract>";

    # accel; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <accel>";
    $self->{options}{'_default_inline'} .= " <accel>";

    # ackno; does not contain text; Formatted as a displayed block
    # Replaced by acknowledgements in DocBook v5.0
    $self->{options}{'_default_untranslated'} .= " <ackno>";
    $self->{options}{'_default_break'} .= " <ackno>";
    # acknowledgements; does not contain text; Formatted as a displayed block
    $self->{options}{'_default_untranslated'} .= " <acknowledgements>";
    $self->{options}{'_default_break'} .= " <acknowledgements>";

    # acronym; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <acronym>";
    $self->{options}{'_default_inline'} .= " <acronym>";

    # action; contains text; Formatted inline; v4, not in v5
    $self->{options}{'_default_translated'} .= " <action>";
    $self->{options}{'_default_inline'} .= " <action>";

    # address; contains text; Formatted as a displayed block; verbatim
    $self->{options}{'_default_translated'} .= " W<address>";
    $self->{options}{'_default_placeholder'} .= " <address>";

    # affiliation; does not contain text; Formatted inline or as a
    # displayed block depending on context
    $self->{options}{'_default_untranslated'} .= " <affiliation>";
    $self->{options}{'_default_inline'} .= " <affiliation>";

    # alt; contains text; Formatted inline or as a
    # displayed block depending on context
    $self->{options}{'_default_translated'} .= " <alt>";
    $self->{options}{'_default_inline'} .= " <alt>";

    # anchor; does not contain text; Produces no output
    $self->{options}{'_default_untranslated'} .= " <anchor>";
    $self->{options}{'_default_inline'} .= " <anchor>";

    # annotation; does not contain text;
    $self->{options}{'_default_untranslated'} .= " <annotation>";
    $self->{options}{'_default_placeholder'} .= " <annotation>";

    # answer; does not contain text;
    $self->{options}{'_default_untranslated'} .= " <answer>";
    $self->{options}{'_default_break'} .= " <answer>";

    # appendix; does not contain text; Formatted as a displayed block
    $self->{options}{'_default_untranslated'} .= " <appendix>";
    $self->{options}{'_default_break'} .= " <appendix>";

    # appendixinfo; does not contain text; v4, not in v5
    $self->{options}{'_default_untranslated'} .= " <appendixinfo>";
    $self->{options}{'_default_placeholder'} .= " <appendixinfo>";

    # application; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <application>";
    $self->{options}{'_default_inline'} .= " <application>";

    # arc; does not contain text;
    $self->{options}{'_default_untranslated'} .= " <arc>";
    $self->{options}{'_default_inline'} .= " <arc>";

    # area; does not contain text;
    # NOTE: the area is not translatable as is, but the coords
    # attribute might be.
    $self->{options}{'_default_untranslated'} .= " <area>";
    $self->{options}{'_default_inline'} .= " <area>";

    # areaset; does not contain text;
    # NOTE: the areaset is not translatable as is. depending on the
    # language there might be more or less area tags inside.
    $self->{options}{'_default_untranslated'} .= " <areaset>";
    $self->{options}{'_default_inline'} .= " <areaset>";

    # areaspec; does not contain text;
    # NOTE: see area and areaset
    $self->{options}{'_default_translated'} .= " <areaspec>";
    $self->{options}{'_default_break'} .= " <areaspec>";

    # arg; contains text; Formatted inline or as a
    # displayed block depending on context
    $self->{options}{'_default_translated'} .= " <arg>";
    $self->{options}{'_default_inline'} .= " <arg>";

    # artheader; does not contain text; renamed to articleinfo in v4.0
    $self->{options}{'_default_untranslated'} .= " <artheader>";
    $self->{options}{'_default_placeholder'} .= " <artheader>";

    # article; does not contain text; Formatted as a displayed block
    $self->{options}{'_default_untranslated'} .= " <article>";
    $self->{options}{'_default_break'} .= " <article>";

    # articleinfo; does not contain text; v4 only
    $self->{options}{'_default_untranslated'} .= " <articleinfo>";
    $self->{options}{'_default_placeholder'} .= " <articleinfo>";

    # artpagenums; contains text; Formatted inline
    # NOTE: could be in the break class
    $self->{options}{'_default_translated'} .= " <artpagenums>";
    $self->{options}{'_default_inline'} .= " <artpagenums>";

    # attribution; contains text; Formatted inline or as a
    # displayed block depending on context
    $self->{options}{'_default_translated'} .= " <attribution>";
    $self->{options}{'_default_inline'} .= " <attribution>";

    # audiodata; does not contain text;
    # NOTE: the attributes might be translated
    $self->{options}{'_default_translated'} .= " <audiodata>";
    $self->{options}{'_default_placeholder'} .= " <audiodata>";
    $self->{options}{'_default_attributes'}.=' <audiodata>fileref';

    # audioobject; does not contain text;
    # NOTE: might be contained in a inlinemediaobject
    $self->{options}{'_default_translated'} .= " <audioobject>";
    $self->{options}{'_default_placeholder'} .= " <audioobject>";

    # author; does not contain text; Formatted inline or as a
    # displayed block depending on context
    $self->{options}{'_default_untranslated'} .= " <author>";
    $self->{options}{'_default_inline'} .= " <author>";

    # authorblurb; does not contain text; Formatted as a displayed block.
    # v4, not in v5
    $self->{options}{'_default_untranslated'} .= " <authorblurb>";
    $self->{options}{'_default_placeholder'} .= " <authorblurb>";

    # authorgroup; does not contain text; Formatted inline or as a
    # displayed block depending on context
    # NOTE: given the possible parents, it is probably very rarely
    #       inlined
    $self->{options}{'_default_untranslated'} .= " <authorgroup>";
    $self->{options}{'_default_break'} .= " <authorgroup>";

    # authorinitials; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <authorinitials>";
    $self->{options}{'_default_inline'} .= " <authorinitials>";

# BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB

    # beginpage; does not contain text; v4, not in v5
    $self->{options}{'_default_untranslated'} .= " <beginpage>";
    $self->{options}{'_default_break'} .= " <beginpage>";

    # bibliocoverage; contains text; Formatted inline
    # NOTE: could be in the break class
    $self->{options}{'_default_translated'} .= " <bibliocoverage>";
    $self->{options}{'_default_inline'} .= " <bibliocoverage>";

    # bibliodiv; does not contain text; Formatted as a displayed block
    $self->{options}{'_default_untranslated'} .= " <bibliodiv>";
    $self->{options}{'_default_break'} .= " <bibliodiv>";

    # biblioentry; does not contain text; Formatted as a displayed block
    $self->{options}{'_default_untranslated'} .= " <biblioentry>";
    $self->{options}{'_default_break'} .= " <biblioentry>";

    # bibliography; does not contain text; Formatted as a displayed block
    $self->{options}{'_default_untranslated'} .= " <bibliography>";
    $self->{options}{'_default_break'} .= " <bibliography>";

    # bibliographyinfo; does not contain text; v4, not in v5
    $self->{options}{'_default_untranslated'} .= " <bibliographyinfo>";
    $self->{options}{'_default_placeholder'} .= " <bibliographyinfo>";

    # biblioid; contains text; Formatted inline
    # NOTE: could be in the break class
    $self->{options}{'_default_translated'} .= " <biblioid>";
    $self->{options}{'_default_inline'} .= " <biblioid>";

    # bibliolist; does not contain text; Formatted as a displayed block
    $self->{options}{'_default_untranslated'} .= " <bibliolist>";
    $self->{options}{'_default_break'} .= " <bibliolist>";

    # bibliomisc; contains text; Formatted inline
    # NOTE: could be in the break class
    $self->{options}{'_default_translated'} .= " <bibliomisc>";
    $self->{options}{'_default_inline'} .= " <bibliomisc>";

    # bibliomixed; contains text; Formatted as a displayed block
    $self->{options}{'_default_translated'} .= " <bibliomixed>";
    $self->{options}{'_default_placeholder'} .= " <bibliomixed>";

    # bibliomset; contains text; Formatted as a displayed block
    # NOTE: content might need to be inlined, e.g. <bibliomset><title>
    $self->{options}{'_default_translated'} .= " <bibliomset>";
    $self->{options}{'_default_placeholder'} .= " <bibliomset>";

    # biblioref; does not contain text; Formatted inline
    $self->{options}{'_default_untranslated'} .= " <biblioref>";
    $self->{options}{'_default_inline'} .= " <biblioref>";

    # bibliorelation; does not contain text; Formatted inline
    $self->{options}{'_default_translated'} .= " <bibliorelation>";
    $self->{options}{'_default_inline'} .= " <bibliorelation>";

    # biblioset; does not contain text; Formatted as a displayed block
    $self->{options}{'_default_untranslated'} .= " <biblioset>";
    $self->{options}{'_default_break'} .= " <biblioset>";

    # bibliosource; contains text; Formatted inline
    # NOTE: could be in the break class
    $self->{options}{'_default_translated'} .= " <bibliosource>";
    $self->{options}{'_default_inline'} .= " <bibliosource>";

    # blockinfo; does not contain text; v4.2, not in v5
    $self->{options}{'_default_untranslated'} .= " <blockinfo>";
    $self->{options}{'_default_placeholder'} .= " <blockinfo>";

    # blockquote; does not contain text; Formatted as a displayed block
    $self->{options}{'_default_untranslated'} .= " <blockquote>";
    $self->{options}{'_default_break'} .= " <blockquote>";

    # book; does not contain text; Formatted as a displayed block
    $self->{options}{'_default_untranslated'} .= " <book>";
    $self->{options}{'_default_break'} .= " <book>";

    # bookbiblio; does not contain text; Formatted as a displayed block
    # Removed in v4.0
    $self->{options}{'_default_untranslated'} .= " <bookbiblio>";
    $self->{options}{'_default_break'} .= " <bookbiblio>";

    # bookinfo; does not contain text; v4, not in v5
    $self->{options}{'_default_untranslated'} .= " <bookinfo>";
    $self->{options}{'_default_placeholder'} .= " <bookinfo>";

    # bridgehead; contains text; Formatted as a displayed block
    $self->{options}{'_default_translated'} .= " <bridgehead>";
    $self->{options}{'_default_break'} .= " <bridgehead>";

# CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC

    # callout; does not contain text; Formatted as a displayed block
    $self->{options}{'_default_untranslated'} .= " <callout>";
    $self->{options}{'_default_break'} .= " <callout>";

    # calloutlist; does not contain text; Formatted as a displayed block
    $self->{options}{'_default_untranslated'} .= " <calloutlist>";
    $self->{options}{'_default_break'} .= " <calloutlist>";

    # caption; does not contain text; Formatted as a displayed block
    $self->{options}{'_default_untranslated'} .= " <caption>";
    $self->{options}{'_default_break'} .= " <caption>";

    # caption (db.html.caption); contains text; Formatted as a displayed block
    # TODO: Check if this works
    $self->{options}{'_default_translated'} .= " <table><caption>";
    $self->{options}{'_default_break'} .= " <table><caption>";

    # caution; does not contain text; Formatted as a displayed block
    $self->{options}{'_default_untranslated'} .= " <caution>";
    $self->{options}{'_default_break'} .= " <caution>";

    # chapter; does not contain text; Formatted as a displayed block
    $self->{options}{'_default_untranslated'} .= " <chapter>";
    $self->{options}{'_default_break'} .= " <chapter>";

    # chapterinfo; does not contain text; v4, not in v5
    $self->{options}{'_default_untranslated'} .= " <chapterinfo>";
    $self->{options}{'_default_placeholder'} .= " <chapterinfo>";

    # citation; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <citation>";
    $self->{options}{'_default_inline'} .= " <citation>";

    # citebiblioid; contains text; Formatted inline
    # NOTE: maybe untranslated?
    $self->{options}{'_default_translated'} .= " <citebiblioid>";
    $self->{options}{'_default_inline'} .= " <citebiblioid>";

    # citerefentry; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <citerefentry>";
    $self->{options}{'_default_inline'} .= " <citerefentry>";

    # citetitle; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <citetitle>";
    $self->{options}{'_default_inline'} .= " <citetitle>";

    # city; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <city>";
    $self->{options}{'_default_inline'} .= " <city>";

    # classname; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <classname>";
    $self->{options}{'_default_inline'} .= " <classname>";

    # classsynopsis; does not contain text; may be in a para
    # NOTE: It may contain a classsynopsisinfo, which should be
    #       verbatim
    # XXX: since it is in untranslated class, does the W flag takes
    #      effect?
    $self->{options}{'_default_untranslated'} .= " W<classsynopsis>";
    $self->{options}{'_default_placeholder'} .= " <classsynopsis>";

    # classsynopsisinfo; contains text;
    # NOTE: see above
    $self->{options}{'_default_translated'} .= " W<classsynopsisinfo>";
    $self->{options}{'_default_inline'} .= " <classsynopsisinfo>";

    # cmdsynopsis; does not contain text; may be in a para
    # NOTE: It may be clearer as a verbatim block
    # XXX: since it is in untranslated class, does the W flag takes
    #      effect? => not completely. Rewrap afterward?
    $self->{options}{'_default_untranslated'} .= " W<cmdsynopsis>";
    $self->{options}{'_default_placeholder'} .= " <cmdsynopsis>";

    # co; does not contain text; Formatted inline
    # XXX: tranlsated or not? (label attribute)
    $self->{options}{'_default_translated'} .= " <co>";
    $self->{options}{'_default_inline'} .= " <co>";

    # code; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <code>";
    $self->{options}{'_default_inline'} .= " <code>";

    # col; does not contain text;
    # NOTE: could be translated to change the layout in a translation
    #       To be done on colgroup in that case.
    $self->{options}{'_default_untranslated'} .= " <col>";
    $self->{options}{'_default_break'} .= " <col>";

    # colgroup; does not contain text;
    # NOTE: could be translated to change the layout in a translation
    $self->{options}{'_default_untranslated'} .= " <colgroup>";
    $self->{options}{'_default_break'} .= " <colgroup>";

    # collab; does not contain text; Formatted inline or as a
    # displayed block depending on context
    # NOTE: could be in the break class
    $self->{options}{'_default_untranslated'} .= " <collab>";
    $self->{options}{'_default_inline'} .= " <collab>";

    # collabname; contains text; Formatted inline or as a
    # displayed block depending on context; v4, not in v5
    $self->{options}{'_default_translated'} .= " <collabname>";
    $self->{options}{'_default_inline'} .= " <collabname>";

    # colophon; does not contain text; Formatted as a displayed block
    $self->{options}{'_default_untranslated'} .= " <colophon>";
    $self->{options}{'_default_break'} .= " <colophon>";

    # colspec; does not contain text;
    # NOTE: could be translated to change the layout in a translation
    $self->{options}{'_default_untranslated'} .= " <colspec>";
    $self->{options}{'_default_break'} .= " <colspec>";

    # command; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <command>";
    $self->{options}{'_default_inline'} .= " <command>";

    # comment; contains text; Formatted inline or as a displayed block
    # Renamed to remark in v4.0
    $self->{options}{'_default_translated'} .= " <comment>";
    $self->{options}{'_default_inline'} .= " <comment>";

    # computeroutput; contains text; Formatted inline
    # NOTE: "is not a verbatim environment, but an inline."
    $self->{options}{'_default_translated'} .= " <computeroutput>";
    $self->{options}{'_default_inline'} .= " <computeroutput>";

    # confdates; contains text; Formatted inline or as a
    # displayed block depending on context
    $self->{options}{'_default_translated'} .= " <confdates>";
    $self->{options}{'_default_inline'} .= " <confdates>";

    # confgroup; does not contain text; Formatted inline or as a
    # displayed block depending on context
    # NOTE: could be in the break class
    $self->{options}{'_default_untranslated'} .= " <confgroup>";
    $self->{options}{'_default_inline'} .= " <confgroup>";

    # confnum; contains text; Formatted inline or as a
    # displayed block depending on context
    $self->{options}{'_default_translated'} .= " <confnum>";
    $self->{options}{'_default_inline'} .= " <confnum>";

    # confsponsor; contains text; Formatted inline or as a
    # displayed block depending on context
    $self->{options}{'_default_translated'} .= " <confsponsor>";
    $self->{options}{'_default_inline'} .= " <confsponsor>";

    # conftitle; contains text; Formatted inline or as a
    # displayed block depending on context
    $self->{options}{'_default_translated'} .= " <conftitle>";
    $self->{options}{'_default_inline'} .= " <conftitle>";

    # constant; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <constant>";
    $self->{options}{'_default_inline'} .= " <constant>";

    # constraint; does not contain text;
    # NOTE: it might be better to have the production as verbatim
    #       Keeping the constrainst inline to have it close to the
    #       lhs or rhs.
    #       The attribute is translatable
    $self->{options}{'_default_untranslated'} .= " <constraint>";
    $self->{options}{'_default_break'} .= " <constraint>";

    # constraintdef; does not contain text; Formatted as a displayed block
    $self->{options}{'_default_untranslated'} .= " <constraintdef>";
    $self->{options}{'_default_break'} .= " <constraintdef>";

    # constructorsynopsis; does not contain text; may be in a para
    # NOTE: It may be clearer as a verbatim block
    # XXX: since it is in untranslated class, does the W flag takes
    #      effect?
    $self->{options}{'_default_untranslated'} .= " W<constructorsynopsis>";
    $self->{options}{'_default_placeholder'} .= " <constructorsynopsis>";

    # contractnum; contains text; Formatted inline or as a displayed block
    # NOTE: could be in the break class
    $self->{options}{'_default_translated'} .= " <contractnum>";
    $self->{options}{'_default_inline'} .= " <contractnum>";

    # contractsponsor; contains text; Formatted inline or as a displayed block
    # NOTE: could be in the break class
    $self->{options}{'_default_translated'} .= " <contractsponsor>";
    $self->{options}{'_default_inline'} .= " <contractsponsor>";

    # contrib; contains text; Formatted inline or as a displayed block
    $self->{options}{'_default_translated'} .= " <contrib>";
    $self->{options}{'_default_inline'} .= " <contrib>";

    # copyright; contains text; Formatted inline or as a displayed block
    # NOTE: could be in the break class
    $self->{options}{'_default_translated'} .= " <copyright>";
    $self->{options}{'_default_inline'} .= " <copyright>";

    # coref; does not contain text; Formatted inline
    # XXX: tranlsated or not? (label attribute)
    $self->{options}{'_default_translated'} .= " <coref>";
    $self->{options}{'_default_inline'} .= " <coref>";

    # corpauthor; contains text; Formatted inline or as a
    # displayed block depending on context; v4, not in v5
    $self->{options}{'_default_translated'} .= " <corpauthor>";
    $self->{options}{'_default_inline'} .= " <corpauthor>";

    # corpcredit; contains text; Formatted inline or as a
    # displayed block depending on context; v4, not in v5
    $self->{options}{'_default_translated'} .= " <corpcredit>";
    $self->{options}{'_default_inline'} .= " <corpcredit>";

    # corpname; contains text; Formatted inline or as a
    # displayed block depending on context; v4, not in v5
    $self->{options}{'_default_translated'} .= " <corpname>";
    $self->{options}{'_default_inline'} .= " <corpname>";

    # country; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <country>";
    $self->{options}{'_default_inline'} .= " <country>";

    # cover; does not contain text; Formatted as a displayed block
    $self->{options}{'_default_untranslated'} .= " <cover>";
    $self->{options}{'_default_break'} .= " <cover>";

# DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD

    # database; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <database>";
    $self->{options}{'_default_inline'} .= " <database>";

    # date; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <date>";
    $self->{options}{'_default_inline'} .= " <date>";

    # dedication; contains text; Formatted as a displayed block
    $self->{options}{'_default_translated'} .= " <dedication>";
    $self->{options}{'_default_break'} .= " <dedication>";

    # destructorsynopsis; does not contain text; may be in a para
    # NOTE: It may be clearer as a verbatim block
    # XXX: since it is in untranslated class, does the W flag takes
    #      effect?
    $self->{options}{'_default_untranslated'} .= " W<destructorsynopsis>";
    $self->{options}{'_default_placeholder'} .= " <destructorsynopsis>";

    # docinfo; does not contain text; removed in v4.0
    $self->{options}{'_default_untranslated'} .= " <docinfo>";
    $self->{options}{'_default_placeholder'} .= " <docinfo>";

# EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE

    # edition; contains text; Formatted inline or as a displayed block
    # NOTE: could be in the break class
    $self->{options}{'_default_translated'} .= " <edition>";
    $self->{options}{'_default_inline'} .= " <edition>";

    # editor; does not contain text; Formatted inline or as a
    # displayed block depending on context
    $self->{options}{'_default_untranslated'} .= " <editor>";
    $self->{options}{'_default_inline'} .= " <editor>";

    # email; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <email>";
    $self->{options}{'_default_inline'} .= " <email>";

    # emphasis; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <emphasis>";
    $self->{options}{'_default_inline'} .= " <emphasis>";

    # entry; contains text;
    $self->{options}{'_default_translated'} .= " <entry>";
    $self->{options}{'_default_break'} .= " <entry>";

    # entrytbl; does not contain text;
    $self->{options}{'_default_untranslated'} .= " <entrytbl>";
    $self->{options}{'_default_break'} .= " <entrytbl>";

    # envar; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <envar>";
    $self->{options}{'_default_inline'} .= " <envar>";

    # epigraph; contains text; Formatted as a displayed block.
    # NOTE: maybe contained in a para
    $self->{options}{'_default_translated'} .= " <epigraph>";
    $self->{options}{'_default_placeholder'} .= " <epigraph>";

    # equation; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <equation>";
    $self->{options}{'_default_break'} .= " <equation>";

    # errorcode; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <errorcode>";
    $self->{options}{'_default_inline'} .= " <errorcode>";

    # errorname; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <errorname>";
    $self->{options}{'_default_inline'} .= " <errorname>";

    # errortext; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <errortext>";
    $self->{options}{'_default_inline'} .= " <errortext>";

    # errortype; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <errortype>";
    $self->{options}{'_default_inline'} .= " <errortype>";

    # example; does not contain text; Formatted as a displayed block.
    # NOTE: maybe contained in a para
    $self->{options}{'_default_untranslated'} .= " <example>";
    $self->{options}{'_default_placeholder'} .= " <example>";

    # exceptionname; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <exceptionname>";
    $self->{options}{'_default_inline'} .= " <exceptionname>";

    # extendedlink; does not contain text;
    $self->{options}{'_default_untranslated'} .= " <extendedlink>";
    $self->{options}{'_default_inline'} .= " <extendedlink>";

# FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF

    # fax; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <fax>";
    $self->{options}{'_default_inline'} .= " <fax>";

    # fieldsynopsis; does not contain text; may be in a para
    $self->{options}{'_default_untranslated'} .= " <fieldsynopsis>";
    $self->{options}{'_default_inline'} .= " <fieldsynopsis>";

    # figure; does not contain text; Formatted as a displayed block.
    # NOTE: maybe contained in a para
    $self->{options}{'_default_untranslated'} .= " <figure>";
    $self->{options}{'_default_placeholder'} .= " <figure>";

    # filename; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <filename>";
    $self->{options}{'_default_inline'} .= " <filename>";

    # firstname; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <firstname>";
    $self->{options}{'_default_inline'} .= " <firstname>";

    # firstterm; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <firstterm>";
    $self->{options}{'_default_inline'} .= " <firstterm>";

    # footnote; contains text;
    $self->{options}{'_default_translated'} .= " <footnote>";
    $self->{options}{'_default_placeholder'} .= " <footnote>";

    # footnoteref; contains text;
    $self->{options}{'_default_translated'} .= " <footnoteref>";
    $self->{options}{'_default_inline'} .= " <footnoteref>";

    # foreignphrase; contains text;
    $self->{options}{'_default_translated'} .= " <foreignphrase>";
    $self->{options}{'_default_inline'} .= " <foreignphrase>";

    # formalpara; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <formalpara>";
    $self->{options}{'_default_break'} .= " <formalpara>";

    # funcdef; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <funcdef>";
    $self->{options}{'_default_inline'} .= " <funcdef>";

    # funcparams; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <funcparams>";
    $self->{options}{'_default_inline'} .= " <funcparams>";

    # funcprototype; does not contain text;
    # NOTE: maybe contained in a funcsynopsis, contained in a para
    $self->{options}{'_default_untranslated'} .= " <funcprototype>";
    $self->{options}{'_default_placeholder'} .= " <funcprototype>";

    # funcsynopsis; does not contain text;
    # NOTE: maybe contained in a para
    $self->{options}{'_default_untranslated'} .= " <funcsynopsis>";
    $self->{options}{'_default_placeholder'} .= " <funcsynopsis>";

    # funcsynopsisinfo; contains text; verbatim
    # NOTE: maybe contained in a funcsynopsis, contained in a para
    $self->{options}{'_default_translated'} .= " W<funcsynopsisinfo>";
    $self->{options}{'_default_placeholder'} .= " <funcsynopsisinfo>";

    # function; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <function>";
    $self->{options}{'_default_inline'} .= " <function>";

# GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG

    # glossary; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <glossary>";
    $self->{options}{'_default_break'} .= " <glossary>";

    # glossaryinfo; does not contain text; v4, not in v5
    $self->{options}{'_default_untranslated'} .= " <glossaryinfo>";
    $self->{options}{'_default_placeholder'} .= " <glossaryinfo>";

    # glossdef; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <glossdef>";
    $self->{options}{'_default_break'} .= " <glossdef>";

    # glossdiv; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <glossdiv>";
    $self->{options}{'_default_break'} .= " <glossdiv>";

    # glossentry; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <glossentry>";
    $self->{options}{'_default_break'} .= " <glossentry>";

    # glosslist; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <glosslist>";
    $self->{options}{'_default_break'} .= " <glosslist>";

    # glosssee; contains text; Formatted as a displayed block.
    $self->{options}{'_default_translated'} .= " <glosssee>";
    $self->{options}{'_default_break'} .= " <glosssee>";

    # glossseealso; contains text; Formatted as a displayed block.
    $self->{options}{'_default_translated'} .= " <glossseealso>";
    $self->{options}{'_default_break'} .= " <glossseealso>";

    # glossterm; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <glossterm>";
    $self->{options}{'_default_inline'} .= " <glossterm>";

    # graphic; does not contain text; Formatted as a displayed block
    # v4, not in v5
    $self->{options}{'_default_untranslated'} .= " <graphic>";
    $self->{options}{'_default_inline'} .= " <graphic>";
    $self->{options}{'_default_attributes'}.=' <graphic>fileref';

    # graphicco; does not contain text; Formatted as a displayed block.
    # v4, not in v5
    $self->{options}{'_default_untranslated'} .= " <graphicco>";
    $self->{options}{'_default_placeholder'} .= " <graphicco>";

    # group; does not contain text; Formatted inline
    $self->{options}{'_default_untranslated'} .= " W<group>";
    $self->{options}{'_default_inline'} .= " <group>";

    # guibutton; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <guibutton>";
    $self->{options}{'_default_inline'} .= " <guibutton>";

    # guiicon; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <guiicon>";
    $self->{options}{'_default_inline'} .= " <guiicon>";

    # guilabel; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <guilabel>";
    $self->{options}{'_default_inline'} .= " <guilabel>";

    # guimenu; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <guimenu>";
    $self->{options}{'_default_inline'} .= " <guimenu>";

    # guimenuitem; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <guimenuitem>";
    $self->{options}{'_default_inline'} .= " <guimenuitem>";

    # guisubmenu; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <guisubmenu>";
    $self->{options}{'_default_inline'} .= " <guisubmenu>";

# HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH

    # hardware; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <hardware>";
    $self->{options}{'_default_inline'} .= " <hardware>";

    # highlights; does not contain text; Formatted inline
    # v4, not in v5
    $self->{options}{'_default_untranslated'} .= " <highlights>";
    $self->{options}{'_default_break'} .= " <highlights>";

    # holder; contains text;
    # NOTE: may depend on the copyright container
    $self->{options}{'_default_translated'} .= " <holder>";
    $self->{options}{'_default_inline'} .= " <holder>";

    # honorific; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <honorific>";
    $self->{options}{'_default_inline'} .= " <honorific>";

    # html:button; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <html:button>";
    $self->{options}{'_default_inline'} .= " <html:button>";

    # html:fieldset; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <html:fieldset>";
    $self->{options}{'_default_inline'} .= " <html:fieldset>";

    # html:form; does not contain text;
    $self->{options}{'_default_translated'} .= " <html:form>";
    $self->{options}{'_default_inline'} .= " <html:form>";

    # html:input; does not contain text; Formatted inline
    # NOTE: attributes are translatable
    $self->{options}{'_default_translated'} .= " <html:input>";
    $self->{options}{'_default_inline'} .= " <html:input>";

    # html:label; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <html:label>";
    $self->{options}{'_default_inline'} .= " <html:label>";

    # html:legend; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <html:legend>";
    $self->{options}{'_default_inline'} .= " <html:legend>";

    # html:option; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <html:option>";
    $self->{options}{'_default_inline'} .= " <html:option>";

    # html:select; does not contain text; Formatted inline
    $self->{options}{'_default_translated'} .= " <html:select>";
    $self->{options}{'_default_inline'} .= " <html:select>";

    # html:textarea; contains text; Formatted as a displayed block.
    $self->{options}{'_default_translated'} .= " <html:textarea>";
    $self->{options}{'_default_placeholder'} .= " <html:textarea>";

    # imagedata; does not contain text; May be formatted inline or
    # as a displayed block, depending on context
    $self->{options}{'_default_translated'} .= " <imagedata>";
    $self->{options}{'_default_inline'} .= " <imagedata>";
    $self->{options}{'_default_attributes'}.=' <imagedata>fileref';

    # imageobject; does not contain text; May be formatted inline or
    # as a displayed block, depending on context
    $self->{options}{'_default_untranslated'} .= " <imageobject>";
    $self->{options}{'_default_inline'} .= " <imageobject>";

    # imageobjectco; does not contain text; Formatted as a displayed block
    # NOTE: may be in a inlinemediaobject
    # TODO: check if this works when the inlinemediaobject is defined
    # as inline
    $self->{options}{'_default_untranslated'} .= " <imageobjectco>";
    $self->{options}{'_default_break'} .= " <imageobjectco>";

    # important; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <important>";
    $self->{options}{'_default_break'} .= " <important>";

    # index; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <index>";
    $self->{options}{'_default_break'} .= " <index>";

    # indexdiv; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <indexdiv>";
    $self->{options}{'_default_break'} .= " <indexdiv>";

    # indexentry; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <indexentry>";
    $self->{options}{'_default_break'} .= " <indexentry>";

    # indexinfo; does not contain text; v4, not in v5
    $self->{options}{'_default_untranslated'} .= " <indexinfo>";
    $self->{options}{'_default_placeholder'} .= " <indexinfo>";

    # indexterm; does not contain text;
    $self->{options}{'_default_untranslated'} .= " <indexterm>";
    $self->{options}{'_default_placeholder'} .= " <indexterm>";

    # info; does not contain text;
    $self->{options}{'_default_untranslated'} .= " <info>";
    $self->{options}{'_default_placeholder'} .= " <info>";

    # informalequation; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <informalequation>";
    $self->{options}{'_default_placeholder'} .= " <informalequation>";

    # informalexample; does not contain text; Formatted as a displayed block.
    # NOTE: can be in a para
    $self->{options}{'_default_untranslated'} .= " <informalexample>";
    $self->{options}{'_default_break'} .= " <informalexample>";

    # informalfigure; does not contain text; Formatted as a displayed block.
    # NOTE: can be in a para
    $self->{options}{'_default_untranslated'} .= " <informalfigure>";
    $self->{options}{'_default_break'} .= " <informalfigure>";

    # informaltable; does not contain text; Formatted as a displayed block.
    # NOTE: can be in a para
    $self->{options}{'_default_untranslated'} .= " <informaltable>";
    $self->{options}{'_default_break'} .= " <informaltable>";

    # initializer; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <initializer>";
    $self->{options}{'_default_inline'} .= " <initializer>";

    # inlineequation; does not contain text; Formatted inline
    $self->{options}{'_default_translated'} .= " W<inlineequation>";
    $self->{options}{'_default_placeholder'} .= " <inlineequation>";

    # inlinegraphic; does not contain text; Formatted inline
    # empty; v4, not in v5
    $self->{options}{'_default_translated'} .= " W<inlinegraphic>";
    $self->{options}{'_default_inline'} .= " <inlinegraphic>";

    # inlinemediaobject; does not contain text; Formatted inline
    $self->{options}{'_default_translated'} .= " <inlinemediaobject>";
    $self->{options}{'_default_placeholder'} .= " <inlinemediaobject>";

    # interface; contains text; Formatted inline; v4, not in v5
    $self->{options}{'_default_translated'} .= " <interface>";
    $self->{options}{'_default_inline'} .= " <interface>";

    # interfacedefinition; contains text; Formatted inline
    # Removed in v4.0
    $self->{options}{'_default_translated'} .= " <interfacedefinition>";
    $self->{options}{'_default_inline'} .= " <interfacedefinition>";

    # interfacename; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <interfacename>";
    $self->{options}{'_default_inline'} .= " <interfacename>";

    # invpartnumber; contains text; Formatted inline; v4, not in v5
    $self->{options}{'_default_translated'} .= " <invpartnumber>";
    $self->{options}{'_default_inline'} .= " <invpartnumber>";

    # isbn; contains text; Formatted inline; v4, not in v5
    $self->{options}{'_default_translated'} .= " <isbn>";
    $self->{options}{'_default_inline'} .= " <isbn>";

    # issn; contains text; Formatted inline; v4, not in v5
    $self->{options}{'_default_translated'} .= " <issn>";
    $self->{options}{'_default_inline'} .= " <issn>";

    # issuenum; contains text; Formatted inline or as a displayed block
    # NOTE: could be in the break class
    $self->{options}{'_default_translated'} .= " <issuenum>";
    $self->{options}{'_default_inline'} .= " <issuenum>";

    # itemizedlist; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <itemizedlist>";
    $self->{options}{'_default_break'} .= " <itemizedlist>";

    # itermset; does not contain text;
    # FIXME
    $self->{options}{'_default_untranslated'} .= " <itermset>";
    $self->{options}{'_default_inline'} .= " <itermset>";

# JJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJ

    # jobtitle; contains text; Formatted inline or as a displayed block
    # NOTE: can be in a para
    $self->{options}{'_default_translated'} .= " <jobtitle>";
    $self->{options}{'_default_inline'} .= " <jobtitle>";

# KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK

    # keycap; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <keycap>";
    $self->{options}{'_default_inline'} .= " <keycap>";

    # keycode; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <keycode>";
    $self->{options}{'_default_inline'} .= " <keycode>";

    # keycombo; does not contain text; Formatted inline
    $self->{options}{'_default_translated'} .= " <keycombo>";
    $self->{options}{'_default_inline'} .= " <keycombo>";

    # keysym; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <keysym>";
    $self->{options}{'_default_inline'} .= " <keysym>";

    # keyword; contains text;
    # NOTE: could be inline
    $self->{options}{'_default_translated'} .= " <keyword>";
    $self->{options}{'_default_break'} .= " <keyword>";

    # keywordset; contains text; Formatted inline or as a displayed block
    # NOTE: could be placeholder/break
    $self->{options}{'_default_translated'} .= " <keywordset>";
    $self->{options}{'_default_break'} .= " <keywordset>";

# LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL

    # label; contains text; Formatted as a displayed block
    $self->{options}{'_default_translated'} .= " <label>";
    $self->{options}{'_default_break'} .= " <label>";

    # legalnotice; contains text; Formatted as a displayed block
    $self->{options}{'_default_translated'} .= " <legalnotice>";
    $self->{options}{'_default_break'} .= " <legalnotice>";

    # lhs; contains text; Formatted as a displayed block.
    # NOTE: it might be better to have the production as verbatim
    #       Keeping the constrainst inline to have it close to the
    #       lhs or rhs.
    $self->{options}{'_default_translated'} .= " <lhs>";
    $self->{options}{'_default_break'} .= " <lhs>";

    # lineage; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <lineage>";
    $self->{options}{'_default_inline'} .= " <lineage>";

    # lineannotation; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <lineannotation>";
    $self->{options}{'_default_inline'} .= " <lineannotation>";

    # link; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <link>";
    $self->{options}{'_default_inline'} .= " <link>";

    # listitem; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <listitem>";
    $self->{options}{'_default_break'} .= " <listitem>";

    # literal; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <literal>";
    $self->{options}{'_default_inline'} .= " <literal>";

    # literallayout; contains text; verbatim
    $self->{options}{'_default_translated'} .= " W<literallayout>";
    $self->{options}{'_default_placeholder'} .= " <literallayout>";

    # locator; does not contain text;
    $self->{options}{'_default_untranslated'} .= " <locator>";
    $self->{options}{'_default_inline'} .= " <locator>";

    # lot; does not contain text; Formatted as a displayed block.
    # v4, not in v5
    $self->{options}{'_default_untranslated'} .= " <lot>";
    $self->{options}{'_default_break'} .= " <lot>";

    # lotentry; contains text; Formatted as a displayed block.
    # v4, not in v5
    $self->{options}{'_default_translated'} .= " <lotentry>";
    $self->{options}{'_default_break'} .= " <lotentry>";

# MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

    # manvolnum; contains text;
    $self->{options}{'_default_translated'} .= " <manvolnum>";
    $self->{options}{'_default_inline'} .= " <manvolnum>";

    # markup; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <markup>";
    $self->{options}{'_default_inline'} .= " <markup>";

    # mathphrase; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <mathphrase>";
    $self->{options}{'_default_inline'} .= " <mathphrase>";

    # medialabel; contains text; Formatted inline
    # v4, not in v5
    $self->{options}{'_default_translated'} .= " <medialabel>";
    $self->{options}{'_default_inline'} .= " <medialabel>";

    # mediaobject; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <mediaobject>";
    $self->{options}{'_default_placeholder'} .= " <mediaobject>";

    # mediaobjectco; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <mediaobjectco>";
    $self->{options}{'_default_placeholder'} .= " <mediaobjectco>";

    # member; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <member>";
    $self->{options}{'_default_inline'} .= " <member>";

    # menuchoice; does not contain text; Formatted inline
    $self->{options}{'_default_translated'} .= " <menuchoice>";
    $self->{options}{'_default_inline'} .= " <menuchoice>";

    # methodname; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <methodname>";
    $self->{options}{'_default_inline'} .= " <methodname>";

    # methodparam; does not contain text; Formatted inline
    $self->{options}{'_default_translated'} .= " <methodparam>";
    $self->{options}{'_default_inline'} .= " <methodparam>";

    # methodsynopsis; does not contain text; Formatted inline
    $self->{options}{'_default_translated'} .= " <methodsynopsis>";
    $self->{options}{'_default_inline'} .= " <methodsynopsis>";

    # modifier; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <modifier>";
    $self->{options}{'_default_inline'} .= " <modifier>";

    # mousebutton; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <mousebutton>";
    $self->{options}{'_default_inline'} .= " <mousebutton>";

    # msg; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <msg>";
    $self->{options}{'_default_break'} .= " <msg>";

    # msgaud; contains text; Formatted as a displayed block.
    $self->{options}{'_default_translated'} .= " <msgaud>";
    $self->{options}{'_default_break'} .= " <msgaud>";

    # msgentry; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <msgentry>";
    $self->{options}{'_default_break'} .= " <msgentry>";

    # msgexplan; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <msgexplan>";
    $self->{options}{'_default_break'} .= " <msgexplan>";

    # msginfo; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <msginfo>";
    $self->{options}{'_default_break'} .= " <msginfo>";

    # msglevel; contains text; Formatted as a displayed block.
    $self->{options}{'_default_translated'} .= " <msglevel>";
    $self->{options}{'_default_break'} .= " <msglevel>";

    # msgmain; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <msgmain>";
    $self->{options}{'_default_break'} .= " <msgmain>";

    # msgorig; contains text; Formatted as a displayed block.
    $self->{options}{'_default_translated'} .= " <msgorig>";
    $self->{options}{'_default_break'} .= " <msgorig>";

    # msgrel; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <msgrel>";
    $self->{options}{'_default_break'} .= " <msgrel>";

    # msgset; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <msgset>";
    $self->{options}{'_default_placeholder'} .= " <msgset>";

    # msgsub; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <msgsub>";
    $self->{options}{'_default_break'} .= " <msgsub>";

    # msgtext; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <msgtext>";
    $self->{options}{'_default_break'} .= " <msgtext>";

# NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN

    # nonterminal; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <nonterminal>";
    $self->{options}{'_default_inline'} .= " <nonterminal>";

    # note; does not contain text; Formatted inline
    # NOTE: can be in a para
    $self->{options}{'_default_untranslated'} .= " <note>";
    $self->{options}{'_default_inline'} .= " <note>";

# OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO

    # objectinfo; does not contain text; v3.1 -> v4, not in v5
    $self->{options}{'_default_untranslated'} .= " <objectinfo>";
    $self->{options}{'_default_placeholder'} .= " <objectinfo>";

    # olink; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <olink>";
    $self->{options}{'_default_inline'} .= " <olink>";

    # ooclass; does not contain text; Formatted inline
    $self->{options}{'_default_translated'} .= " <ooclass>";
    $self->{options}{'_default_inline'} .= " <ooclass>";

    # ooexception; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <ooexception>";
    $self->{options}{'_default_inline'} .= " <ooexception>";

    # oointerface; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <oointerface>";
    $self->{options}{'_default_inline'} .= " <oointerface>";

    # option; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <option>";
    $self->{options}{'_default_inline'} .= " <option>";

    # optional; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <optional>";
    $self->{options}{'_default_inline'} .= " <optional>";

    # orderedlist; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <orderedlist>";
    $self->{options}{'_default_placeholder'} .= " <orderedlist>";

    # org; does not contain text; Formatted inline or as a
    # displayed block depending on context
    $self->{options}{'_default_untranslated'} .= " <org>";
    $self->{options}{'_default_inline'} .= " <org>";

    # orgdiv; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <orgdiv>";
    $self->{options}{'_default_inline'} .= " <orgdiv>";

    # orgname; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <orgname>";
    $self->{options}{'_default_inline'} .= " <orgname>";

    # otheraddr; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <otheraddr>";
    $self->{options}{'_default_inline'} .= " <otheraddr>";

    # othercredit; does not contain text; Formatted inline or as a
    # displayed block depending on context
    $self->{options}{'_default_untranslated'} .= " <othercredit>";
    $self->{options}{'_default_inline'} .= " <othercredit>";

    # othername; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <othername>";
    $self->{options}{'_default_inline'} .= " <othername>";

# PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP

    # package; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <package>";
    $self->{options}{'_default_inline'} .= " <package>";

    # pagenums; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <pagenums>";
    $self->{options}{'_default_inline'} .= " <pagenums>";

    # para; contains text; Formatted as a displayed block
    $self->{options}{'_default_translated'} .= " <para>";
    $self->{options}{'_default_break'} .= " <para>";

    # paramdef; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <paramdef>";
    $self->{options}{'_default_inline'} .= " <paramdef>";

    # parameter; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <parameter>";
    $self->{options}{'_default_inline'} .= " <parameter>";

    # part; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <part>";
    $self->{options}{'_default_break'} .= " <part>";

    # partinfo; does not contain text; v4, not in v5
    $self->{options}{'_default_untranslated'} .= " <partinfo>";
    $self->{options}{'_default_placeholder'} .= " <partinfo>";

    # partintro; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <partintro>";
    $self->{options}{'_default_break'} .= " <partintro>";

    # person; does not contain text; Formatted inline or as a
    # displayed block depending on context
    $self->{options}{'_default_untranslated'} .= " <person>";
    $self->{options}{'_default_inline'} .= " <person>";

    # personblurb; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <personblurb>";
    $self->{options}{'_default_placeholder'} .= " <personblurb>";

    # personname; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <personname>";
    $self->{options}{'_default_inline'} .= " <personname>";

    # phone; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <phone>";
    $self->{options}{'_default_inline'} .= " <phone>";

    # phrase; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <phrase>";
    $self->{options}{'_default_inline'} .= " <phrase>";

    # pob; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <pob>";
    $self->{options}{'_default_inline'} .= " <pob>";

    # postcode; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <postcode>";
    $self->{options}{'_default_inline'} .= " <postcode>";

    # preface; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <preface>";
    $self->{options}{'_default_break'} .= " <preface>";

    # prefaceinfo; does not contain text; v4, not in v5
    $self->{options}{'_default_untranslated'} .= " <prefaceinfo>";
    $self->{options}{'_default_placeholder'} .= " <prefaceinfo>";

    # primary; contains text;
    $self->{options}{'_default_translated'} .= " <primary>";
    $self->{options}{'_default_break'} .= " <primary>";

    # primaryie; contains text; Formatted as a displayed block.
    $self->{options}{'_default_translated'} .= " <primaryie>";
    $self->{options}{'_default_break'} .= " <primaryie>";

    # printhistory; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <printhistory>";
    $self->{options}{'_default_break'} .= " <printhistory>";

    # procedure; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <procedure>";
    $self->{options}{'_default_placeholder'} .= " <procedure>";

    # production; doesnot contain text;
    # NOTE: it might be better to have the production as verbatim
    #       Keeping the constrainst inline to have it close to the
    #       lhs or rhs.
    $self->{options}{'_default_untranslated'} .= " <production>";
    $self->{options}{'_default_break'} .= " <production>";

    # productionrecap; does not contain text; like production
    $self->{options}{'_default_untranslated'} .= " <productionrecap>";
    $self->{options}{'_default_break'} .= " <productionrecap>";

    # productionset; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <productionset>";
    $self->{options}{'_default_placeholder'} .= " <productionset>";

    # productname; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <productname>";
    $self->{options}{'_default_inline'} .= " <productname>";

    # productnumber; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <productnumber>";
    $self->{options}{'_default_inline'} .= " <productnumber>";

    # programlisting; contains text; Formatted as a displayed block.
    $self->{options}{'_default_translated'} .= " W<programlisting>";
    $self->{options}{'_default_placeholder'} .= " <programlisting>";

    # programlistingco; contains text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <programlistingco>";
    $self->{options}{'_default_placeholder'} .= " <programlistingco>";

    # prompt; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <prompt>";
    $self->{options}{'_default_inline'} .= " <prompt>";

    # property; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <property>";
    $self->{options}{'_default_inline'} .= " <property>";

    # pubdate; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <pubdate>";
    $self->{options}{'_default_inline'} .= " <pubdate>";

    # publisher; does not contain text; Formatted inline or as a displayed block
    # NOTE: could be in the break class
    $self->{options}{'_default_translated'} .= " <publisher>";
    $self->{options}{'_default_inline'} .= " <publisher>";

    # publishername; contains text; Formatted inline or as a displayed block
    $self->{options}{'_default_translated'} .= " <publishername>";
    $self->{options}{'_default_inline'} .= " <publishername>";

# QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ

    # qandadiv; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <qandadiv>";
    $self->{options}{'_default_break'} .= " <qandadiv>";

    # qandaentry; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <qandaentry>";
    $self->{options}{'_default_break'} .= " <qandaentry>";

    # qandaset; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <qandaset>";
    $self->{options}{'_default_break'} .= " <qandaset>";

    # question; does not contain text;
    $self->{options}{'_default_untranslated'} .= " <question>";
    $self->{options}{'_default_break'} .= " <question>";

    # quote; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <quote>";
    $self->{options}{'_default_inline'} .= " <quote>";

# RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR

    # refclass; contains text; Formatted inline or as a displayed block
    # NOTE: could be in the inline class
    $self->{options}{'_default_translated'} .= " <refclass>";
    $self->{options}{'_default_break'} .= " <refclass>";

    # refdescriptor; contains text; Formatted inline or as a displayed block
    # NOTE: could be in the inline class
    $self->{options}{'_default_translated'} .= " <refdescriptor>";
    $self->{options}{'_default_break'} .= " <refdescriptor>";

    # refentry; does not contain text; Formatted as a displayed block
    $self->{options}{'_default_untranslated'} .= " <refentry>";
    $self->{options}{'_default_break'} .= " <refentry>";

    # refentryinfo; does not contain text; v4, not in v5
    $self->{options}{'_default_untranslated'} .= " <refentryinfo>";
    $self->{options}{'_default_placeholder'} .= " <refentryinfo>";

    # refentrytitle; contains text; Formatted as a displayed block
# FIXME: do not seems to be a block
    $self->{options}{'_default_translated'} .= " <refentrytitle>";
    $self->{options}{'_default_inline'} .= " <refentrytitle>";

    # reference; does not contain text; Formatted as a displayed block
    $self->{options}{'_default_untranslated'} .= " <reference>";
    $self->{options}{'_default_break'} .= " <reference>";

    # referenceinfo; does not contain text; v4, not in v5
    $self->{options}{'_default_untranslated'} .= " <referenceinfo>";
    $self->{options}{'_default_placeholder'} .= " <referenceinfo>";

    # refmeta; does not contains text;
    # NOTE: could be in the inline class
    $self->{options}{'_default_untranslated'} .= " <refmeta>";
    $self->{options}{'_default_break'} .= " <refmeta>";

    # refmiscinfo; contains text; Formatted inline or as a displayed block
    # NOTE: could be in the inline class
    $self->{options}{'_default_translated'} .= " <refmiscinfo>";
    $self->{options}{'_default_break'} .= " <refmiscinfo>";

    # refname; contains text; Formatted inline or as a displayed block
    # NOTE: could be in the inline class
    $self->{options}{'_default_translated'} .= " <refname>";
    $self->{options}{'_default_break'} .= " <refname>";

    # refnamediv; does not contain text; Formatted as a displayed block
    $self->{options}{'_default_untranslated'} .= " <refnamediv>";
    $self->{options}{'_default_break'} .= " <refnamediv>";

    # refpurpose; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <refpurpose>";
    $self->{options}{'_default_inline'} .= " <refpurpose>";

    # refsect1; does not contain text; Formatted as a displayed block
    $self->{options}{'_default_untranslated'} .= " <refsect1>";
    $self->{options}{'_default_break'} .= " <refsect1>";

    # refsect1info; does not contain text; v4, not in v5
    $self->{options}{'_default_untranslated'} .= " <refsect1info>";
    $self->{options}{'_default_placeholder'} .= " <refsect1info>";

    # refsect2; does not contain text; Formatted as a displayed block
    $self->{options}{'_default_untranslated'} .= " <refsect2>";
    $self->{options}{'_default_break'} .= " <refsect2>";

    # refsect2info; does not contain text; v4, not in v5
    $self->{options}{'_default_untranslated'} .= " <refsect2info>";
    $self->{options}{'_default_placeholder'} .= " <refsect2info>";

    # refsect3; does not contain text; Formatted as a displayed block
    $self->{options}{'_default_untranslated'} .= " <refsect3>";
    $self->{options}{'_default_break'} .= " <refsect3>";

    # refsect3info; does not contain text; v4, not in v5
    $self->{options}{'_default_untranslated'} .= " <refsect3info>";
    $self->{options}{'_default_placeholder'} .= " <refsect3info>";

    # refsection; does not contain text; Formatted as a displayed block
    $self->{options}{'_default_untranslated'} .= " <refsection>";
    $self->{options}{'_default_break'} .= " <refsection>";

    # refsectioninfo; does not contain text; v4, not in v5
    $self->{options}{'_default_untranslated'} .= " <refsectioninfo>";
    $self->{options}{'_default_placeholder'} .= " <refsectioninfo>";

    # refsynopsisdiv; does not contain text; Formatted as a displayed block
    $self->{options}{'_default_untranslated'} .= " <refsynopsisdiv>";
    $self->{options}{'_default_break'} .= " <refsynopsisdiv>";

    # refsynopsisdivinfo; does not contain text; v4, not in v5
    $self->{options}{'_default_untranslated'} .= " <refsynopsisdivinfo>";
    $self->{options}{'_default_placeholder'} .= " <refsynopsisdivinfo>";

    # releaseinfo; contains text; Formatted inline or as a displayed block
    # NOTE: could be in the inline class
    $self->{options}{'_default_translated'} .= " <releaseinfo>";
    $self->{options}{'_default_break'} .= " <releaseinfo>";

    # remark; contains text; Formatted inline or as a displayed block
    $self->{options}{'_default_translated'} .= " <remark>";
    $self->{options}{'_default_inline'} .= " <remark>";

    # replaceable; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <replaceable>";
    $self->{options}{'_default_inline'} .= " <replaceable>";

    # returnvalue; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <returnvalue>";
    $self->{options}{'_default_inline'} .= " <returnvalue>";

    # revdescription; contains text; Formatted inline or as a displayed block
    $self->{options}{'_default_translated'} .= " <revdescription>";
    $self->{options}{'_default_break'} .= " <revdescription>";

    # revhistory; does not contain text; Formatted as a displayed block
    $self->{options}{'_default_untranslated'} .= " <revhistory>";
    $self->{options}{'_default_break'} .= " <revhistory>";

    # revision; does not contain text;
    $self->{options}{'_default_untranslated'} .= " <revision>";
    $self->{options}{'_default_break'} .= " <revision>";

    # revnumber; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <revnumber>";
    $self->{options}{'_default_inline'} .= " <revnumber>";

    # revremark; contains text; Formatted inline or as a displayed block
    $self->{options}{'_default_translated'} .= " <revremark>";
    $self->{options}{'_default_break'} .= " <revremark>";

    # rhs; contains text; Formatted as a displayed block.
    # NOTE: it might be better to have the production as verbatim
    #       Keeping the constrainst inline to have it close to the
    #       lhs or rhs.
    $self->{options}{'_default_translated'} .= " <rhs>";
    $self->{options}{'_default_break'} .= " <rhs>";

    # row; does not contain text;
    $self->{options}{'_default_untranslated'} .= " <row>";
    $self->{options}{'_default_break'} .= " <row>";

# SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS

    # sbr; does not contain text; line break
    $self->{options}{'_default_untranslated'} .= " <sbr>";
    $self->{options}{'_default_break'} .= " <sbr>";

    # screen; contains text; verbatim
    $self->{options}{'_default_translated'} .= " W<screen>";
    $self->{options}{'_default_placeholder'} .= " <screen>";

    # screenco; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <screenco>";
    $self->{options}{'_default_placeholder'} .= " <screenco>";

    # screeninfo; does not contain text; v4, not in v5
    $self->{options}{'_default_untranslated'} .= " <screeninfo>";
    $self->{options}{'_default_placeholder'} .= " <screeninfo>";

    # screenshot; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <screenshot>";
    $self->{options}{'_default_placeholder'} .= " <screenshot>";

    # secondary; contains text;
    $self->{options}{'_default_translated'} .= " <secondary>";
    $self->{options}{'_default_break'} .= " <secondary>";

    # secondaryie; contains text; Formatted as a displayed block.
    $self->{options}{'_default_translated'} .= " <secondaryie>";
    $self->{options}{'_default_break'} .= " <secondaryie>";

    # sect1; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <sect1>";
    $self->{options}{'_default_break'} .= " <sect1>";

    # sect1info; does not contain text; v4, not in v5
    $self->{options}{'_default_untranslated'} .= " <sect1info>";
    $self->{options}{'_default_placeholder'} .= " <sect1info>";

    # sect2; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <sect2>";
    $self->{options}{'_default_break'} .= " <sect2>";

    # sect2info; does not contain text; v4, not in v5
    $self->{options}{'_default_untranslated'} .= " <sect2info>";
    $self->{options}{'_default_placeholder'} .= " <sect2info>";

    # sect3; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <sect3>";
    $self->{options}{'_default_break'} .= " <sect3>";

    # sect3info; does not contain text; v4, not in v5
    $self->{options}{'_default_untranslated'} .= " <sect3info>";
    $self->{options}{'_default_placeholder'} .= " <sect3info>";

    # sect4; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <sect4>";
    $self->{options}{'_default_break'} .= " <sect4>";

    # sect4info; does not contain text; v4, not in v5
    $self->{options}{'_default_untranslated'} .= " <sect4info>";
    $self->{options}{'_default_placeholder'} .= " <sect4info>";

    # sect5; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <sect5>";
    $self->{options}{'_default_break'} .= " <sect5>";

    # sect5info; does not contain text; v4, not in v5
    $self->{options}{'_default_untranslated'} .= " <sect5info>";
    $self->{options}{'_default_placeholder'} .= " <sect5info>";

    # section; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <section>";
    $self->{options}{'_default_break'} .= " <section>";

    # sectioninfo; does not contain text; v3.1 -> v4, not in v5
    $self->{options}{'_default_untranslated'} .= " <sectioninfo>";
    $self->{options}{'_default_placeholder'} .= " <sectioninfo>";

    # see; contains text;
    $self->{options}{'_default_translated'} .= " <see>";
    $self->{options}{'_default_break'} .= " <see>";

    # seealso; contains text;
    $self->{options}{'_default_translated'} .= " <seealso>";
    $self->{options}{'_default_break'} .= " <seealso>";

    # seealsoie; contains text; Formatted as a displayed block.
    $self->{options}{'_default_translated'} .= " <seealsoie>";
    $self->{options}{'_default_break'} .= " <seealsoie>";

    # seeie; contains text; Formatted as a displayed block.
    $self->{options}{'_default_translated'} .= " <seeie>";
    $self->{options}{'_default_break'} .= " <seeie>";

    # seg; contains text;
    $self->{options}{'_default_translated'} .= " <seg>";
    $self->{options}{'_default_break'} .= " <seg>";

    # seglistitem; does not contain text;
    $self->{options}{'_default_untranslated'} .= " <seglistitem>";
    $self->{options}{'_default_break'} .= " <seglistitem>";

    # segmentedlist; does not contain text;
    $self->{options}{'_default_untranslated'} .= " <segmentedlist>";
    $self->{options}{'_default_break'} .= " <segmentedlist>";

    # segtitle; contains text;
    $self->{options}{'_default_translated'} .= " <segtitle>";
    $self->{options}{'_default_break'} .= " <segtitle>";

    # seriesinfo; does not contain text;
    # Removed in v4.0
    $self->{options}{'_default_untranslated'} .= " <seriesinfo>";
    $self->{options}{'_default_placeholder'} .= " <seriesinfo>";

    # seriesvolnums; contains text; Formatted inline
    # NOTE: could be in the break class
    $self->{options}{'_default_translated'} .= " <seriesvolnums>";
    $self->{options}{'_default_inline'} .= " <seriesvolnums>";

    # set; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <set>";
    $self->{options}{'_default_break'} .= " <set>";

    # setindex; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <setindex>";
    $self->{options}{'_default_break'} .= " <setindex>";

    # setindexinfo; does not contain text; v4, not in v5
    $self->{options}{'_default_untranslated'} .= " <setindexinfo>";
    $self->{options}{'_default_placeholder'} .= " <setindexinfo>";

    # setinfo; does not contain text; v4, not in v5
    $self->{options}{'_default_untranslated'} .= " <setinfo>";
    $self->{options}{'_default_placeholder'} .= " <setinfo>";

    # sgmltag; contains text; Formatted inline; v4, not in v5
    $self->{options}{'_default_translated'} .= " <sgmltag>";
    $self->{options}{'_default_inline'} .= " <sgmltag>";

    # shortaffil; contains text; Formatted inline or as a
    # displayed block depending on context
    $self->{options}{'_default_translated'} .= " <shortaffil>";
    $self->{options}{'_default_inline'} .= " <shortaffil>";

    # shortcut; does not contain text; Formatted inline
    $self->{options}{'_default_untranslated'} .= " <shortcut>";
    $self->{options}{'_default_inline'} .= " <shortcut>";

    # sidebar; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <sidebar>";
    $self->{options}{'_default_break'} .= " <sidebar>";

    # sidebarinfo; does not contain text; v4, not in v5
    $self->{options}{'_default_untranslated'} .= " <sidebarinfo>";
    $self->{options}{'_default_placeholder'} .= " <sidebarinfo>";

    # simpara; contains text; Formatted as a displayed block.
    $self->{options}{'_default_translated'} .= " <simpara>";
    $self->{options}{'_default_break'} .= " <simpara>";

    # simplelist; does not contain text;
    $self->{options}{'_default_untranslated'} .= " <simplelist>";
    $self->{options}{'_default_inline'} .= " <simplelist>";

    # simplemsgentry; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <simplemsgentry>";
    $self->{options}{'_default_break'} .= " <simplemsgentry>";

    # simplesect; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <simplesect>";
    $self->{options}{'_default_break'} .= " <simplesect>";

    # spanspec; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <spanspec>";
    $self->{options}{'_default_break'} .= " <spanspec>";

    # state; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <state>";
    $self->{options}{'_default_inline'} .= " <state>";

    # step; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <step>";
    $self->{options}{'_default_break'} .= " <step>";

    # stepalternatives; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <stepalternatives>";
    $self->{options}{'_default_break'} .= " <stepalternatives>";

    # street; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <street>";
    $self->{options}{'_default_inline'} .= " <street>";

    # structfield; contains text; Formatted inline; v4, not in v5
    $self->{options}{'_default_translated'} .= " <structfield>";
    $self->{options}{'_default_inline'} .= " <structfield>";

    # structname; contains text; Formatted inline; v4, not in v5
    $self->{options}{'_default_translated'} .= " <structname>";
    $self->{options}{'_default_inline'} .= " <structname>";

    # subject; does not contain text; Formatted inline or as a displayed block
    # NOTE: could be in the inline class
    $self->{options}{'_default_untranslated'} .= " <subject>";
    $self->{options}{'_default_break'} .= " <subject>";

    # subjectset; does not contain text; Formatted inline or as a displayed block
    # NOTE: could be in the inline class
    $self->{options}{'_default_untranslated'} .= " <subjectset>";
    $self->{options}{'_default_break'} .= " <subjectset>";

    # subjectterm; contains text; Formatted inline or as a displayed block
    # NOTE: could be in the inline class
    $self->{options}{'_default_translated'} .= " <subjectterm>";
    $self->{options}{'_default_break'} .= " <subjectterm>";

    # subscript; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <subscript>";
    $self->{options}{'_default_inline'} .= " <subscript>";

    # substeps; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <substeps>";
    $self->{options}{'_default_break'} .= " <substeps>";

    # subtitle; contains text; Formatted as a displayed block.
    $self->{options}{'_default_translated'} .= " <subtitle>";
    $self->{options}{'_default_break'} .= " <subtitle>";

    # superscript; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <superscript>";
    $self->{options}{'_default_inline'} .= " <superscript>";

    # surname; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <surname>";
    $self->{options}{'_default_inline'} .= " <surname>";

#svg:svg

    # symbol; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <symbol>";
    $self->{options}{'_default_inline'} .= " <symbol>";

    # synopfragment; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <synopfragment>";
    $self->{options}{'_default_placeholder'} .= " <synopfragment>";

    # synopfragmentref; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <synopfragmentref>";
    $self->{options}{'_default_inline'} .= " <synopfragmentref>";

    # synopsis; contains text; verbatim
    $self->{options}{'_default_translated'} .= " W<synopsis>";
    $self->{options}{'_default_placeholder'} .= " <synopsis>";

    # systemitem; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <systemitem>";
    $self->{options}{'_default_inline'} .= " <systemitem>";

# TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT

    # table; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <table>";
    $self->{options}{'_default_placeholder'} .= " <table>";

    # tag; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <tag>";
    $self->{options}{'_default_inline'} .= " <tag>";

    # task; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <task>";
    $self->{options}{'_default_placeholder'} .= " <task>";

    # taskprerequisites; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <taskprerequisites>";
    $self->{options}{'_default_break'} .= " <taskprerequisites>";

    # taskrelated; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <taskrelated>";
    $self->{options}{'_default_break'} .= " <taskrelated>";

    # tasksummary; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <tasksummary>";
    $self->{options}{'_default_break'} .= " <tasksummary>";

    # tbody; does not contain text;
    $self->{options}{'_default_untranslated'} .= " <tbody>";
    $self->{options}{'_default_break'} .= " <tbody>";

    # td; contains text;
    $self->{options}{'_default_translated'} .= " <td>";
    $self->{options}{'_default_break'} .= " <td>";

    # term; contains text; Formatted as a displayed block.
    $self->{options}{'_default_translated'} .= " <term>";
    $self->{options}{'_default_break'} .= " <term>";

    # termdef; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <termdef>";
    $self->{options}{'_default_inline'} .= " <termdef>";

    # tertiary; contains text; Suppressed
    $self->{options}{'_default_translated'} .= " <tertiary>";
    $self->{options}{'_default_placeholder'} .= " <tertiary>";

    # tertiaryie; contains text; Formatted as a displayed block.
    $self->{options}{'_default_translated'} .= " <tertiaryie>";
    $self->{options}{'_default_break'} .= " <tertiaryie>";

    # textdata; does not contain text; Formatted inline or as a displayed block
    # NOTE: could be in the inline class
    $self->{options}{'_default_untranslated'} .= " <textdata>";
    $self->{options}{'_default_break'} .= " <textdata>";
    $self->{options}{'_default_attributes'}.=' <textdata>fileref';

    # textobject; does not contain text; Formatted inline or as a displayed block
    # NOTE: could be in the inline class
    $self->{options}{'_default_untranslated'} .= " <textobject>";
    $self->{options}{'_default_break'} .= " <textobject>";

    # tfoot; does not contain text;
    $self->{options}{'_default_untranslated'} .= " <tfoot>";
    $self->{options}{'_default_break'} .= " <tfoot>";

    # tgroup; does not contain text;
    $self->{options}{'_default_untranslated'} .= " <tgroup>";
    $self->{options}{'_default_break'} .= " <tgroup>";

    # th; contains text;
    $self->{options}{'_default_translated'} .= " <th>";
    $self->{options}{'_default_break'} .= " <th>";

    # thead; does not contain text;
    $self->{options}{'_default_untranslated'} .= " <thead>";
    $self->{options}{'_default_break'} .= " <thead>";

    # tip; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <tip>";
    $self->{options}{'_default_break'} .= " <tip>";

    # title; contains text; Formatted as a displayed block.
    $self->{options}{'_default_translated'} .= " <title>";
    $self->{options}{'_default_break'} .= " <title>";

    # titleabbrev; contains text; Formatted inline or as a displayed block
    # NOTE: could be in the inline class
    $self->{options}{'_default_translated'} .= " <titleabbrev>";
    $self->{options}{'_default_break'} .= " <titleabbrev>";

    # toc; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <toc>";
    $self->{options}{'_default_break'} .= " <toc>";

    # tocback; contains text; Formatted as a displayed block.
    $self->{options}{'_default_translated'} .= " <tocback>";
    $self->{options}{'_default_break'} .= " <tocback>";

    # tocchap; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_translated'} .= " <tocchap>";
    $self->{options}{'_default_break'} .= " <tocchap>";

    # tocdiv; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <tocdiv>";
    $self->{options}{'_default_break'} .= " <tocdiv>";

    # tocentry; contains text; Formatted as a displayed block.
    $self->{options}{'_default_translated'} .= " <tocentry>";
    $self->{options}{'_default_break'} .= " <tocentry>";

    # tocfront; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_translated'} .= " <tocfront>";
    $self->{options}{'_default_break'} .= " <tocfront>";

    # toclevel1; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <toclevel1>";
    $self->{options}{'_default_break'} .= " <toclevel1>";

    # toclevel2; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <toclevel2>";
    $self->{options}{'_default_break'} .= " <toclevel2>";

    # toclevel3; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <toclevel3>";
    $self->{options}{'_default_break'} .= " <toclevel3>";

    # toclevel4; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <toclevel4>";
    $self->{options}{'_default_break'} .= " <toclevel4>";

    # toclevel5; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <toclevel5>";
    $self->{options}{'_default_break'} .= " <toclevel5>";

    # tocpart; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <tocpart>";
    $self->{options}{'_default_break'} .= " <tocpart>";

    # token; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <token>";
    $self->{options}{'_default_inline'} .= " <token>";

    # tr; does not contain text;
    $self->{options}{'_default_untranslated'} .= " <tr>";
    $self->{options}{'_default_break'} .= " <tr>";

    # trademark; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <trademark>";
    $self->{options}{'_default_inline'} .= " <trademark>";

    # type; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <type>";
    $self->{options}{'_default_inline'} .= " <type>";

# UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU

    # ulink; contains text; Formatted inline; v4, not in v5
    $self->{options}{'_default_translated'} .= " <ulink>";
    $self->{options}{'_default_inline'} .= " <ulink>";

    # uri; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <uri>";
    $self->{options}{'_default_inline'} .= " <uri>";

    # userinput; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <userinput>";
    $self->{options}{'_default_inline'} .= " <userinput>";

# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

    # varargs; empty element;
    $self->{options}{'_default_untranslated'} .= " <varargs>";
    $self->{options}{'_default_inline'} .= " <varargs>";

    # variablelist; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <variablelist>";
    $self->{options}{'_default_placeholder'} .= " <variablelist>";

    # varlistentry; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <varlistentry>";
    $self->{options}{'_default_break'} .= " <varlistentry>";

    # varname; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <varname>";
    $self->{options}{'_default_inline'} .= " <varname>";

    # videodata; contains text; Formatted inline or as a displayed block
    $self->{options}{'_default_untranslated'} .= " <videodata>";
    $self->{options}{'_default_break'} .= " <videodata>";
    $self->{options}{'_default_attributes'}.=' <videodata>fileref';

    # videoobject; contains text; Formatted inline or as a displayed block
    $self->{options}{'_default_untranslated'} .= " <videoobject>";
    $self->{options}{'_default_break'} .= " <videoobject>";

    # void; empty element;
    $self->{options}{'_default_untranslated'} .= " <void>";
    $self->{options}{'_default_inline'} .= " <void>";

    # volumenum; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <volumenum>";
    $self->{options}{'_default_inline'} .= " <volumenum>";

# WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW

    # warning; does not contain text; Formatted as a displayed block.
    $self->{options}{'_default_untranslated'} .= " <warning>";
    $self->{options}{'_default_break'} .= " <warning>";

    # wordasword; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <wordasword>";
    $self->{options}{'_default_inline'} .= " <wordasword>";

# XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

    # xref; empty element;
    $self->{options}{'_default_untranslated'} .= " <xref>";
    $self->{options}{'_default_inline'} .= " <xref>";

# YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY

    # year; contains text; Formatted inline
    $self->{options}{'_default_translated'} .= " <year>";
    $self->{options}{'_default_inline'} .= " <year>";

# ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ

    $self->{options}{'_default_attributes'}.='
        lang
        xml:lang';

    $self->treat_options;
}
