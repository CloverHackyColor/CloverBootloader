#!/usr/bin/perl

# Po4a::Html.pm
#
# extract and translate translatable strings from a HTML document.
#
# This code extracts plain text between HTML tags and some "alt" or
# "title" attributes.
#
# Copyright (c) 2003 by Laurent Hausermann  <laurent@hausermann.org>
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

Locale::Po4a::Html - convert HTML documents from/to PO files

=head1 DESCRIPTION

The po4a (PO for anything) project goal is to ease translations (and more
interestingly, the maintenance of translations) using gettext tools on
areas where they were not expected like documentation.

Locale::Po4a::Html is a module to help the translation of documentation in
the HTML format into other [human] languages.

Please note that this module is not distributed with the main po4a archive
because we don't feel it mature enough for that. If you insist on using it
anyway, check it from the CVS out.

=cut

package Locale::Po4a::Html;
require Exporter;
use vars qw(@ISA @EXPORT);
@ISA = qw(Locale::Po4a::TransTractor);
@EXPORT = qw(new initialize);

use Locale::Po4a::TransTractor;

use strict;
use warnings;

use HTML::TokeParser;

use File::Temp;

sub initialize {}

sub read {
    my ($self,$filename)=@_;
    push @{$self->{DOCPOD}{infile}}, $filename;
    $self->Locale::Po4a::TransTractor::read($filename);
}

sub parse {
    my $self=shift;
    map {$self->parse_file($_)} @{$self->{DOCPOD}{infile}};
}

#
# Parse file and translate it
#
sub parse_file {
    my ($self,$filename)=@_;
    my $stream = HTML::TokeParser->new($filename)
        || die "Couldn't read HTML file $filename : $!";

    $stream->unbroken_text( [1] );

    my @type=();
    NEXT : while (my $token = $stream->get_token) {
        if($token->[0] eq 'T') {
            my $text = $token->[1];
            my ($pre_spaces) = ($text =~ /^(\s*)/);
            my ($post_spaces) = ($text =~ /(\s*)$/);
            $text = trim($text);
            if (notranslation($text) == 1) {
                $self->pushline( get_tag( $token ) );
                next NEXT;
            }
#  FIXME : it should be useful to encode characters
#  in UTF8 in the po, but converting them in HTML::Entities
#  in the doc_out, translate acts both way
#  so we cant do that.
#  use HTML::Entities ();
#  $encoded = HTML::Entities::encode($a);
#  $decoded = HTML::Entities::decode($a);
            #print STDERR $token->[0];
            $self->pushline( $pre_spaces . $self->translate($text,
                                                  "FIXME:0",
                                                  (scalar @type ? $type[scalar @type-1]: "NOTYPE")
                                                  ) . $post_spaces,
                             'wrap' => 1
                             );
            next NEXT;
        } elsif ($token->[0] eq 'S') {
            push @type,$token->[1];
            my $text =  get_tag( $token );
            my $tag = $token->[1];
# TODO: It would be nice to support an option to specify these
#       (e.g. a list of tag.attribute)
            my @trans_attr = (( $tag eq 'img' ) || ( $tag eq 'input' ) ||
                              ( $tag eq 'area' ) || ( $tag eq 'applet'))
                ? qw/title alt/ : qw/title/;
            my %attr = %{$token->[2]};
            for my $a (@trans_attr) {
                my $content = $attr{$a};
                if (defined $content) {
                    $content = trim($content);
                    my $translated = $self->translate(
                        $content,
                        "FIXME:0",
                        "${tag}_$a"
                        );
                    $attr{$a} = $translated;
                }
            }
            my ($closing) = ( $text =~ /(\s*\/?>)/ );
            # reconstruct the tag from scratch
            delete $attr{'/'}; # Parser thinks closing / in XHTML is an attribute
            $text = "<$tag";
            $text .= " $_=\"$attr{$_}\"" foreach keys %attr;
            $text .= $closing;
            $self->pushline( $text );
        } elsif ($token->[0] eq 'E') {
            pop @type;
            $self->pushline( get_tag( $token ) );
        } else {
            $self->pushline( get_tag( $token ) );
        }
    }
}

sub get_tag {
    my $token = shift;
    my $tag = "";

    if ($token->[0] eq 'S') {
        $tag = $token->[4];
    }
    if ( ($token->[0] eq 'C') ||
         ($token->[0] eq 'D') ||
         ($token->[0] eq 'T') ) {
        $tag =  $token->[1];
    }
    if ( ($token->[0] eq 'E') ||
         ($token->[0] eq 'PI') ) {
        $tag =  $token->[2];
    }

    return $tag;
}

sub trim {
    my $s=shift;
    $s =~ s/\n/ /g;  # remove \n in text
    $s =~ s/\r/ /g;  # remove \r in text
    $s =~ s/\t/ /g;  # remove tabulations
    $s =~ s/\s+/ /g; # remove multiple spaces
    $s =~ s/^\s*//g; # remove leading spaces
    $s =~ s/\s*$//g; # remove trailing spaces
    return $s;
}

#
# This method says if a string must be
# translated or not.
# To be improved with more test or regexp
# Maybe there is a way to do this in TransTractor
# for example ::^ should not be translated
sub notranslation {
    my $s=shift;
    return 1 if ( ($s cmp "")   == 0);
    return 1 if ( ($s cmp "-")  == 0);
    return 1 if ( ($s cmp "::") == 0);
    return 1 if ( ($s cmp ":")  == 0);
    return 1 if ( ($s cmp ".")  == 0);
    return 1 if ( ($s cmp "|")  == 0);
    return 1 if ( ($s cmp '"')  == 0);
    return 1 if ( ($s cmp "'")  == 0);
    # don't translate entries composed of one entity
    return 1 if ($s =~ /^&[^;]*;$/);

# don't translate entries with no letters
# (happens with e.g.  <b>Hello</b>, <i>world</i> )
#                                 ^^
#                    ", " doesn't need translation
    return 1 unless $s =~ /\w/;
    return 0;
}

=head1 AUTHORS

 Laurent Hausermann <laurent@hausermann.org>

=head1 COPYRIGHT AND LICENSE

Laurent Hausermann <laurent@hausermann.org>

This program is free software; you may redistribute it and/or modify it
under the terms of GPL (see the COPYING file).
