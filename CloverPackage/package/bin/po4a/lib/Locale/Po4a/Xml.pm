#!/usr/bin/perl

# Po4a::Xml.pm
#
# extract and translate translatable strings from XML documents.
#
# This code extracts plain text from tags and attributes from generic
# XML documents, and it can be used as a base to build modules for
# XML-based documents.
#
# Copyright (c) 2004 by Jordi Vilalta  <jvprat@gmail.com>
# Copyright (c) 2008-2009 by Nicolas François  <nicolas.francois@centraliens.net>
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

Locale::Po4a::Xml - convert XML documents and derivates from/to PO files

=head1 DESCRIPTION

The po4a (PO for anything) project goal is to ease translations (and more
interestingly, the maintenance of translations) using gettext tools on
areas where they were not expected like documentation.

Locale::Po4a::Xml is a module to help the translation of XML documents into
other [human] languages. It can also be used as a base to build modules for
XML-based documents.

=cut

package Locale::Po4a::Xml;

use 5.006;
use strict;
use warnings;

require Exporter;
use vars qw(@ISA @EXPORT);
@ISA = qw(Locale::Po4a::TransTractor);
@EXPORT = qw(new initialize @tag_types);

use Locale::Po4a::TransTractor;
use Locale::Po4a::Common;
use Carp qw(croak);
use File::Basename;
use File::Spec;

#It will mantain the path from the root tag to the current one
my @path;

#It will contain a list of external entities and their attached paths
my %entities;

my @comments;
my %translate_options_cache;

my $_shiftline_in_comment = 0;
sub shiftline {
    my $self = shift;
    # call Transtractor's shiftline
    my ($line,$ref) = $self->SUPER::shiftline();
    return ($line,$ref) if (not defined $line);

    if ($self->{options}{'includeexternal'}) {
        my $tmp;

    for my $k (keys %entities) {
        if ($line =~ m/^(.*?)&$k;(.*)$/s) {
            my ($before, $after) = ($1, $2);
            my $linenum=0;
            my @textentries;

            $tmp = $before;
            my $tmp_in_comment = 0;
            if ($_shiftline_in_comment) {
                if ($before =~ m/^.*?-->(.*)$/s) {
                    $tmp = $1;
                    $tmp_in_comment = 0;
                } else {
                    $tmp_in_comment = 1;
                }
            }
            if ($tmp_in_comment == 0) {
                while ($tmp =~ m/^.*?<!--.*?-->(.*)$/s) {
                    $tmp = $1;
                }
                if ($tmp =~ m/<!--/s) {
                    $tmp_in_comment = 1;
                }
            }
            next if ($tmp_in_comment);

            open (my $in, $entities{$k})
                or croak wrap_mod("po4a::xml",
                                  dgettext("po4a", "Can't read from %s: %s"),
                                  $entities{$k}, $!);
            while (defined (my $textline = <$in>)) {
                $linenum++;
                my $textref=$entities{$k}.":$linenum";
                push @textentries, ($textline,$textref);
            }
            close $in
                or croak wrap_mod("po4a::xml",
                          dgettext("po4a", "Can't close %s after reading: %s"),
                                  $entities{$k}, $!);

            push @textentries, ($after, $ref);
            $line = $before.(shift @textentries);
            $ref .= " ".(shift @textentries);
            $self->unshiftline(@textentries);
        }
    }

        $tmp = $line;
        if ($_shiftline_in_comment) {
            if ($line =~ m/^.*?-->(.*)$/s) {
                $tmp = $1;
                $_shiftline_in_comment = 0;
            } else {
                $_shiftline_in_comment = 1;
            }
        }
        if ($_shiftline_in_comment == 0) {
            while ($tmp =~ m/^.*?<!--.*?-->(.*)$/s) {
                $tmp = $1;
            }
            if ($tmp =~ m/<!--/s) {
                $_shiftline_in_comment = 1;
            }
        }
    }

    return ($line,$ref);
}

sub read {
    my ($self,$filename)=@_;
    push @{$self->{DOCPOD}{infile}}, $filename;
    $self->Locale::Po4a::TransTractor::read($filename);
}

sub parse {
    my $self=shift;
    map {$self->parse_file($_)} @{$self->{DOCPOD}{infile}};
}

# @save_holders is a stack of references to ('paragraph', 'translation',
# 'sub_translations', 'open', 'close', 'folded_attributes') hashes, where:
# paragraph         is a reference to an array (see paragraph in the
#                   treat_content() subroutine) of strings followed by
#                   references.  It contains the @paragraph array as it was
#                   before the processing was interrupted by a tag instroducing
#                   a placeholder.
# translation       is the translation of this level up to now
# sub_translations  is a reference to an array of strings containing the
#                   translations which must replace the placeholders.
# open              is the tag which opened the placeholder.
# close             is the tag which closed the placeholder.
# folded_attributes is an hash of tags with their attributes (<tag attrs=...>
#                   strings), referenced by the folded tag id, which should
#                   replace the <tag po4a-id=id> strings in the current
#                   translation.
#
# If @save_holders only has 1 holder, then we are not processing the
# content of an holder, we are translating the document.
my @save_holders;


# If we are at the bottom of the stack and there is no <placeholder ...> in
# the current translation, we can push the translation in the translated
# document.
# Otherwise, we keep the translation in the current holder.
sub pushline {
    my ($self, $line) = (shift, shift);

    my $holder = $save_holders[$#save_holders];
    my $translation = $holder->{'translation'};
    $translation .= $line;

    while (    %{$holder->{folded_attributes}}
           and $translation =~ m/^(.*)<([^>]+?)\s+po4a-id=([0-9]+)>(.*)$/s) {
        my $begin = $1;
        my $tag = $2;
        my $id = $3;
        my $end = $4;
        if (defined $holder->{folded_attributes}->{$id}) {
            # TODO: check if the tag is the same
            $translation = $begin.$holder->{folded_attributes}->{$id}.$end;
            delete $holder->{folded_attributes}->{$id};
        } else {
            # TODO: It will be hard to identify the location.
            #       => find a way to retrieve the reference.
            die wrap_mod("po4a::xml", dgettext("po4a", "'po4a-id=%d' in the translation does not exist in the original string (or 'po4a-id=%d' used twice in the translation)."), $id, $id);
        }
    }
# TODO: check that %folded_attributes is empty at some time
# => in translate_paragraph?

    if (   ($#save_holders > 0)
        or ($translation =~ m/<placeholder\s+type="[^"]+"\s+id="(\d+)"\s*\/>/s)) {
        $holder->{'translation'} = $translation;
    } else {
        $self->SUPER::pushline($translation);
        $holder->{'translation'} = '';
    }
}

=head1 TRANSLATING WITH PO4A::XML

This module can be used directly to handle generic XML documents.  This will
extract all tag's content, and no attributes, since it's where the text is
written in most XML based documents.

There are some options (described in the next section) that can customize
this behavior.  If this doesn't fit to your document format you're encouraged
to write your own module derived from this, to describe your format's details.
See the section B<WRITING DERIVATE MODULES> below, for the process description.

=cut

#
# Parse file and translate it
#
sub parse_file {
    my ($self,$filename) = @_;
    my $eof = 0;

    while (!$eof) {
        # We get all the text until the next breaking tag (not
        # inline) and translate it
        $eof = $self->treat_content;
        if (!$eof) {
            # And then we treat the following breaking tag
            $eof = $self->treat_tag;
        }
    }
}

=head1 OPTIONS ACCEPTED BY THIS MODULE

The global debug option causes this module to show the excluded strings, in
order to see if it skips something important.

These are this module's particular options:

=over 4

=item B<nostrip>

Prevents it to strip the spaces around the extracted strings.

=item B<wrap>

Canonizes the string to translate, considering that whitespaces are not
important, and wraps the translated document. This option can be overridden
by custom tag options. See the "tags" option below.

=item B<caseinsensitive>

It makes the tags and attributes searching to work in a case insensitive
way.  If it's defined, it will treat E<lt>BooKE<gt>laNG and E<lt>BOOKE<gt>Lang as E<lt>bookE<gt>lang.

=item B<includeexternal>

When defined, external entities are included in the generated (translated)
document, and for the extraction of strings.  If it's not defined, you
will have to translate external entities separately as independent
documents.

=item B<ontagerror>

This option defines the behavior of the module when it encounter a invalid
XML syntax (a closing tag which does not match the last opening tag, or a
tag's attribute without value).
It can take the following values:

=over

=item I<fail>

This is the default value.
The module will exit with an error.

=item I<warn>

The module will continue, and will issue a warning.

=item I<silent>

The module will continue without any warnings.

=back

Be careful when using this option.
It is generally recommended to fix the input file.

=item B<tagsonly>

Extracts only the specified tags in the "tags" option.  Otherwise, it
will extract all the tags except the ones specified.

Note: This option is deprecated.

=item B<doctype>

String that will try to match with the first line of the document's doctype
(if defined). If it doesn't, a warning will indicate that the document
might be of a bad type.

=item B<addlang>

String indicating the path (e.g. E<lt>bbbE<gt>E<lt>aaaE<gt>) of a tag
where a lang="..." attribute shall be added. The language will be defined
as the basename of the PO file without any .po extension.

=item B<tags>

Space-separated list of tags you want to translate or skip.  By default,
the specified tags will be excluded, but if you use the "tagsonly" option,
the specified tags will be the only ones included.  The tags must be in the
form E<lt>aaaE<gt>, but you can join some (E<lt>bbbE<gt>E<lt>aaaE<gt>) to say that the content of
the tag E<lt>aaaE<gt> will only be translated when it's into a E<lt>bbbE<gt> tag.

You can also specify some tag options by putting some characters in front of
the tag hierarchy. For example, you can put 'w' (wrap) or 'W' (don't wrap)
to override the default behavior specified by the global "wrap" option.

Example: WE<lt>chapterE<gt>E<lt>titleE<gt>

Note: This option is deprecated.
You should use the B<translated> and B<untranslated> options instead.

=item B<attributes>

Space-separated list of tag's attributes you want to translate.  You can
specify the attributes by their name (for example, "lang"), but you can
prefix it with a tag hierarchy, to specify that this attribute will only be
translated when it's into the specified tag. For example: E<lt>bbbE<gt>E<lt>aaaE<gt>lang
specifies that the lang attribute will only be translated if it's into an
E<lt>aaaE<gt> tag, and it's into a E<lt>bbbE<gt> tag.

=item B<foldattributes>

Do not translate attributes in inline tags.
Instead, replace all attributes of a tag by po4a-id=<id>.

This is useful when attributes shall not be translated, as this simplifies the
strings for translators, and avoids typos.

=item B<customtag>

Space-separated list of tags which should not be treated as tags.
These tags are treated as inline, and do not need to be closed.

=item B<break>

Space-separated list of tags which should break the sequence.
By default, all tags break the sequence.

The tags must be in the form <aaa>, but you can join some
(<bbb><aaa>), if a tag (<aaa>) should only be considered
when it's into another tag (<bbb>).

=item B<inline>

Space-separated list of tags which should be treated as inline.
By default, all tags break the sequence.

The tags must be in the form <aaa>, but you can join some
(<bbb><aaa>), if a tag (<aaa>) should only be considered
when it's into another tag (<bbb>).

=item B<placeholder>

Space-separated list of tags which should be treated as placeholders.
Placeholders do not break the sequence, but the content of placeholders is
translated separately.

The location of the placeholder in its block will be marked with a string
similar to:

  <placeholder type=\"footnote\" id=\"0\"/>

The tags must be in the form <aaa>, but you can join some
(<bbb><aaa>), if a tag (<aaa>) should only be considered
when it's into another tag (<bbb>).

=item B<nodefault>

Space separated list of tags that the module should not try to set by
default in any category.

=item B<cpp>

Support C preprocessor directives.
When this option is set, po4a will consider preprocessor directives as
paragraph separators.
This is important if the XML file must be preprocessed because otherwise
the directives may be inserted in the middle of lines if po4a consider it
belong to the current paragraph, and they won't be recognized by the
preprocessor.
Note: the preprocessor directives must only appear between tags
(they must not break a tag).

=item B<translated>

Space-separated list of tags you want to translate.

The tags must be in the form <aaa>, but you can join some
(<bbb><aaa>), if a tag (<aaa>) should only be considered
when it's into another tag (<bbb>).

You can also specify some tag options by putting some characters in front of
the tag hierarchy. For example, you can put 'w' (wrap) or 'W' (don't wrap)
to override the default behavior specified by the global "wrap" option.

Example: WE<lt>chapterE<gt>E<lt>titleE<gt>

=item B<untranslated>

Space-separated list of tags you do not want to translate.

The tags must be in the form <aaa>, but you can join some
(<bbb><aaa>), if a tag (<aaa>) should only be considered
when it's into another tag (<bbb>).

=item B<defaulttranslateoption>

The default categories for tags that are not in any of the translated,
untranslated, break, inline, or placeholder.

This is a set of letters:

=over

=item I<w>

Tags should be translated and content can be re-wrapped.

=item I<W>

Tags should be translated and content should not be re-wrapped.

=item I<i>

Tags should be translated inline.

=item I<p>

Tags should be translated as placeholders.

=back

=back

=cut
# TODO: defaulttranslateoption
# w => indicate that it is only valid for translatable tags and do not
#      care about inline/break/placeholder?
# ...

sub initialize {
    my $self = shift;
    my %options = @_;

    # Reset the path
    @path = ();

    # Initialize the stack of holders
    my @paragraph = ();
    my @sub_translations = ();
    my %folded_attributes;
    my %holder = ('paragraph' => \@paragraph,
                  'translation' => "",
                  'sub_translations' => \@sub_translations,
                  'folded_attributes' => \%folded_attributes);
    @save_holders = (\%holder);

    $self->{options}{'addlang'}=0;
    $self->{options}{'nostrip'}=0;
    $self->{options}{'wrap'}=0;
    $self->{options}{'caseinsensitive'}=0;
    $self->{options}{'tagsonly'}=0;
    $self->{options}{'tags'}='';
    $self->{options}{'break'}='';
    $self->{options}{'translated'}='';
    $self->{options}{'untranslated'}='';
    $self->{options}{'defaulttranslateoption'}='';
    $self->{options}{'attributes'}='';
    $self->{options}{'foldattributes'}=0;
    $self->{options}{'inline'}='';
    $self->{options}{'placeholder'}='';
    $self->{options}{'customtag'}='';
    $self->{options}{'doctype'}='';
    $self->{options}{'nodefault'}='';
    $self->{options}{'includeexternal'}=0;
    $self->{options}{'ontagerror'}="fail";
    $self->{options}{'cpp'}=0;

    $self->{options}{'verbose'}='';
    $self->{options}{'debug'}='';

    foreach my $opt (keys %options) {
        if ($options{$opt}) {
            die wrap_mod("po4a::xml",
                dgettext("po4a", "Unknown option: %s"), $opt)
                unless exists $self->{options}{$opt};
            $self->{options}{$opt} = $options{$opt};
        }
    }
    # Default options set by modules. Forbidden for users.
    $self->{options}{'_default_translated'}='';
    $self->{options}{'_default_untranslated'}='';
    $self->{options}{'_default_break'}='';
    $self->{options}{'_default_inline'}='';
    $self->{options}{'_default_placeholder'}='';
    $self->{options}{'_default_attributes'}='';
    $self->{options}{'_default_customtag'}='';

    #It will maintain the list of the translatable tags
    $self->{tags}=();
    $self->{translated}=();
    $self->{untranslated}=();
    #It will maintain the list of the translatable attributes
    $self->{attributes}=();
    #It will maintain the list of the breaking tags
    $self->{break}=();
    #It will maintain the list of the inline tags
    $self->{inline}=();
    #It will maintain the list of the placeholder tags
    $self->{placeholder}=();
    #It will maintain the list of the customtag tags
    $self->{customtag}=();
    #list of the tags that must not be set in the tags or inline category
    #by this module or sub-module (unless specified in an option)
    $self->{nodefault}=();

    $self->treat_options;

    #  Clear cache
    %translate_options_cache=();
}

=head1 WRITING DERIVATE MODULES

=head2 DEFINE WHAT TAGS AND ATTRIBUTES TO TRANSLATE

The simplest customization is to define which tags and attributes you want
the parser to translate.  This should be done in the initialize function.
First you should call the main initialize, to get the command-line options,
and then, append your custom definitions to the options hash.  If you want
to treat some new options from command line, you should define them before
calling the main initialize:

  $self->{options}{'new_option'}='';
  $self->SUPER::initialize(%options);
  $self->{options}{'_default_translated'}.=' <p> <head><title>';
  $self->{options}{'attributes'}.=' <p>lang id';
  $self->{options}{'_default_inline'}.=' <br>';
  $self->treat_options;

You should use the B<_default_inline>, B<_default_break>,
B<_default_placeholder>, B<_default_translated>, B<_default_untranslated>,
and B<_default_attributes> options in derivated modules. This allow users
to override the default behavior defined in your module with command line
options.

=head2 OVERRIDING THE found_string FUNCTION

Another simple step is to override the function "found_string", which
receives the extracted strings from the parser, in order to translate them.
There you can control which strings you want to translate, and perform
transformations to them before or after the translation itself.

It receives the extracted text, the reference on where it was, and a hash
that contains extra information to control what strings to translate, how
to translate them and to generate the comment.

The content of these options depends on the kind of string it is (specified in an
entry of this hash):

=over

=item type="tag"

The found string is the content of a translatable tag. The entry "tag_options"
contains the option characters in front of the tag hierarchy in the module
"tags" option.

=item type="attribute"

Means that the found string is the value of a translatable attribute. The
entry "attribute" has the name of the attribute.

=back

It must return the text that will replace the original in the translated
document. Here's a basic example of this function:

  sub found_string {
    my ($self,$text,$ref,$options)=@_;
    $text = $self->translate($text,$ref,"type ".$options->{'type'},
      'wrap'=>$self->{options}{'wrap'});
    return $text;
  }

There's another simple example in the new Dia module, which only filters
some strings.

=cut

sub found_string {
    my ($self,$text,$ref,$options)=@_;

    if ($text =~ m/^\s*$/s) {
        return $text;
    }

    my $comment;
    my $wrap = $self->{options}{'wrap'};

    if ($options->{'type'} eq "tag") {
        $comment = "Content of: ".$self->get_path;

        if($options->{'tag_options'} =~ /w/) {
            $wrap = 1;
        }
        if($options->{'tag_options'} =~ /W/) {
            $wrap = 0;
        }
    } elsif ($options->{'type'} eq "attribute") {
        $comment = "Attribute '".$options->{'attribute'}."' of: ".$self->get_path;
    } elsif ($options->{'type'} eq "CDATA") {
        $comment = "CDATA";
        $wrap = 0;
    } else {
        die wrap_ref_mod($ref, "po4a::xml", dgettext("po4a", "Internal error: unknown type identifier '%s'."), $options->{'type'});
    }
    $text = $self->translate($text,$ref,$comment,'wrap'=>$wrap, comment => $options->{'comments'});
    return $text;
}

=head2 MODIFYING TAG TYPES (TODO)

This is a more complex one, but it enables a (almost) total customization.
It's based in a list of hashes, each one defining a tag type's behavior. The
list should be sorted so that the most general tags are after the most
concrete ones (sorted first by the beginning and then by the end keys). To
define a tag type you'll have to make a hash with the following keys:

=over 4

=item B<beginning>

Specifies the beginning of the tag, after the "E<lt>".

=item B<end>

Specifies the end of the tag, before the "E<gt>".

=item B<breaking>

It says if this is a breaking tag class.  A non-breaking (inline) tag is one
that can be taken as part of the content of another tag.  It can take the
values false (0), true (1) or undefined.  If you leave this undefined, you'll
have to define the f_breaking function that will say whether a concrete tag of
this class is a breaking tag or not.

=item B<f_breaking>

It's a function that will tell if the next tag is a breaking one or not.  It
should be defined if the B<breaking> option is not.

=item B<f_extract>

If you leave this key undefined, the generic extraction function will have to
extract the tag itself.  It's useful for tags that can have other tags or
special structures in them, so that the main parser doesn't get mad.  This
function receives a boolean that says if the tag should be removed from the
input stream or not.

=item B<f_translate>

This function receives the tag (in the get_string_until() format) and returns
the translated tag (translated attributes or all needed transformations) as a
single string.

=back

=cut

##### Generic XML tag types #####'

our @tag_types = (
    {    beginning    => "!--#",
        end        => "--",
        breaking    => 0,
        f_extract    => \&tag_extract_comment,
        f_translate    => \&tag_trans_comment},
    {    beginning    => "!--",
        end        => "--",
        breaking    => 0,
        f_extract    => \&tag_extract_comment,
        f_translate    => \&tag_trans_comment},
    {    beginning    => "?xml",
        end        => "?",
        breaking    => 1,
        f_translate    => \&tag_trans_xmlhead},
    {    beginning    => "?",
        end        => "?",
        breaking    => 1,
        f_translate    => \&tag_trans_procins},
    {    beginning    => "!DOCTYPE",
        end        => "",
        breaking    => 1,
        f_extract    => \&tag_extract_doctype,
        f_translate    => \&tag_trans_doctype},
    {    beginning    => "![CDATA[",
        end        => "]]",
        breaking    => 1,
        f_extract    => \&CDATA_extract,
        f_translate    => \&CDATA_trans},
    {    beginning    => "/",
        end        => "",
        f_breaking    => \&tag_break_close,
        f_translate    => \&tag_trans_close},
    {    beginning    => "",
        end        => "/",
        f_breaking    => \&tag_break_alone,
        f_translate    => \&tag_trans_alone},
    {    beginning    => "",
        end        => "",
        f_breaking    => \&tag_break_open,
        f_translate    => \&tag_trans_open}
);

sub tag_extract_comment {
    my ($self,$remove)=(shift,shift);
    my ($eof,@tag)=$self->get_string_until('-->',{include=>1,remove=>$remove});
    return ($eof,@tag);
}

sub tag_trans_comment {
    my ($self,@tag)=@_;
    return $self->join_lines(@tag);
}

sub tag_trans_xmlhead {
    my ($self,@tag)=@_;

    # We don't have to translate anything from here: throw away references
    my $tag = $self->join_lines(@tag);
    $tag =~ /encoding=(("|')|)(.*?)(\s|\2)/s;
    my $in_charset=$3;
    $self->detected_charset($in_charset);
    my $out_charset=$self->get_out_charset;

    if (defined $in_charset) {
        $tag =~ s/$in_charset/$out_charset/;
    } else {
        if ($tag =~ m/standalone/) {
            $tag =~ s/(standalone)/encoding="$out_charset" $1/;
        } else {
            $tag.= " encoding=\"$out_charset\"";
        }
    }

    return $tag;
}

sub tag_trans_procins {
    my ($self,@tag)=@_;
    return $self->join_lines(@tag);
}

sub tag_extract_doctype {
    my ($self,$remove)=(shift,shift);

    # Check if there is an internal subset (between []).
    my ($eof,@tag)=$self->get_string_until('>',{include=>1,unquoted=>1});
    my $parity = 0;
    my $paragraph = "";
    map { $parity = 1 - $parity; $paragraph.= $parity?$_:""; } @tag;
    my $found = 0;
    if ($paragraph =~ m/<.*\[.*</s) {
        $found = 1
    }

    if (not $found) {
        ($eof,@tag)=$self->get_string_until('>',{include=>1,remove=>$remove,unquoted=>1});
    } else {
        ($eof,@tag)=$self->get_string_until(']\s*>',{include=>1,remove=>$remove,unquoted=>1,regex=>1});
    }
    return ($eof,@tag);
}

sub tag_trans_doctype {
# This check is not really reliable.  There are system and public
# identifiers.  Only the public one could be checked reliably.
    my ($self,@tag)=@_;
    if (defined $self->{options}{'doctype'} ) {
        my $doctype = $self->{options}{'doctype'};
        if ( $tag[0] !~ /\Q$doctype\E/i ) {
            warn wrap_ref_mod($tag[1], "po4a::xml", dgettext("po4a", "Bad document type. '%s' expected. You can fix this warning with a -o doctype option, or ignore this check with -o doctype=\"\"."), $doctype);
        }
    }
    my $i = 0;
    my $basedir = $tag[1];
    $basedir =~ s/:[0-9]+$//;
    $basedir = dirname($basedir);

    while ( $i < $#tag ) {
        my $t = $tag[$i];
        my $ref = $tag[$i+1];
        if ( $t =~ /^(\s*<!ENTITY\s+)(.*)$/is ) {
            my $part1 = $1;
            my $part2 = $2;
            my $includenow = 0;
            my $file = 0;
            my $name = "";
            if ($part2 =~ /^(%\s+)(.*)$/s ) {
                $part1.= $1;
                $part2 = $2;
                $includenow = 1;
            }
            $part2 =~ /^(\S+)(\s+)(.*)$/s;
            $name = $1;
            $part1.= $1.$2;
            $part2 = $3;
            if ( $part2 =~ /^(SYSTEM\s+)(.*)$/is ) {
                $part1.= $1;
                $part2 = $2;
                $file = 1;
                if ($self->{options}{'includeexternal'}) {
                    $entities{$name} = $part2;
                    $entities{$name} =~ s/^"?(.*?)".*$/$1/s;
                    $entities{$name} = File::Spec->catfile($basedir, $entities{$name});
                }
            }
            if ((not $file) and (not $includenow)) {
                if ($part2 =~ m/^\s*(["'])(.*)\1(\s*>.*)$/s) {
                my $comment = "Content of the $name entity";
                my $quote = $1;
                my $text = $2;
                $part2 = $3;
                $text = $self->translate($text,
                                         $ref,
                                         $comment,
                                         'wrap'=>1);
                $t = $part1."$quote$text$quote$part2";
                }
            }
#            print $part1."\n";
#            print $name."\n";
#            print $part2."\n";
        }
        $tag[$i] = $t;
        $i += 2;
    }
    return $self->join_lines(@tag);
}

sub tag_break_close {
    my ($self,@tag)=@_;
    my $struct = $self->get_path;
    my $options = $self->get_translate_options($struct);
    if ($options =~ m/[ip]/) {
        return 0;
    } else {
        return 1;
    }
}

sub tag_trans_close {
    my ($self,@tag)=@_;
    my $name = $self->get_tag_name(@tag);

    my $test = pop @path;
    if (!defined($test) || $test ne $name ) {
        my $ontagerror = $self->{options}{'ontagerror'};
        if ($ontagerror eq "warn") {
            warn wrap_ref_mod($tag[1], "po4a::xml", dgettext("po4a", "Unexpected closing tag </%s> found. The main document may be wrong.  Continuing..."), $name);
        } elsif ($ontagerror ne "silent") {
            die wrap_ref_mod($tag[1], "po4a::xml", dgettext("po4a", "Unexpected closing tag </%s> found. The main document may be wrong."), $name);
        }
    }
    return $self->join_lines(@tag);
}

sub CDATA_extract {
    my ($self,$remove)=(shift,shift);
        my ($eof, @tag) = $self->get_string_until(']]>',{include=>1,unquoted=>0,remove=>$remove});

    return ($eof, @tag);
}

sub CDATA_trans {
    my ($self,@tag)=@_;
    return $self->found_string($self->join_lines(@tag),
                               $tag[1],
                               {'type' => "CDATA"});
}

sub tag_break_alone {
    my ($self,@tag)=@_;
    my $struct = $self->get_path($self->get_tag_name(@tag));
    if ($self->get_translate_options($struct) =~ m/i/) {
        return 0;
    } else {
        return 1;
    }
}

sub tag_trans_alone {
    my ($self,@tag)=@_;
    my $name = $self->get_tag_name(@tag);
    push @path, $name;

    $name = $self->treat_attributes(@tag);

    pop @path;
    return $name;
}

sub tag_break_open {
    my ($self,@tag)=@_;
    my $struct = $self->get_path($self->get_tag_name(@tag));
    my $options = $self->get_translate_options($struct);
    if ($options =~ m/[ip]/) {
        return 0;
    } else {
        return 1;
    }
}

sub tag_trans_open {
    my ($self,@tag)=@_;
    my $name = $self->get_tag_name(@tag);
    push @path, $name;

    $name = $self->treat_attributes(@tag);

    if (defined $self->{options}{'addlang'}) {
        my $struct = $self->get_path();
        if ($struct eq $self->{options}{'addlang'}) {
            $name .= ' lang="'.$self->{TT}{po_in}->{lang}.'"';
        }
    }

    return $name;
}

##### END of Generic XML tag types #####

=head1 INTERNAL FUNCTIONS used to write derivated parsers

=head2 WORKING WITH TAGS

=over 4

=item get_path()

This function returns the path to the current tag from the document's root,
in the form E<lt>htmlE<gt>E<lt>bodyE<gt>E<lt>pE<gt>.

An additional array of tags (without brackets) can be passed as argument.
These path elements are added to the end of the current path.

=cut

sub get_path {
    my $self = shift;
    my @add = @_;
    if ( @path > 0 or @add > 0 ) {
        return "<".join("><",@path,@add).">";
    } else {
        return "outside any tag (error?)";
    }
}

=item tag_type()

This function returns the index from the tag_types list that fits to the next
tag in the input stream, or -1 if it's at the end of the input file.

=cut

sub tag_type {
    my $self = shift;
    my ($line,$ref) = $self->shiftline();
    my ($match1,$match2);
    my $found = 0;
    my $i = 0;

    if (!defined($line)) { return -1; }

    $self->unshiftline($line,$ref);
    my ($eof,@lines) = $self->get_string_until(">",{include=>1,unquoted=>1});
    my $line2 = $self->join_lines(@lines);
    while (!$found && $i < @tag_types) {
        ($match1,$match2) = ($tag_types[$i]->{beginning},$tag_types[$i]->{end});
        if ($line =~ /^<\Q$match1\E/) {
            if (!defined($tag_types[$i]->{f_extract})) {
#print substr($line2,length($line2)-1-length($match2),1+length($match2))."\n";
                if (defined($line2) and $line2 =~ /\Q$match2\E>$/) {
                    $found = 1;
#print "YES: <".$match1." ".$match2.">\n";
                } else {
#print "NO: <".$match1." ".$match2.">\n";
                    $i++;
                }
            } else {
                $found = 1;
            }
        } else {
            $i++;
        }
    }
    if (!$found) {
        #It should never enter here, unless you undefine the most
        #general tags (as <...>)
        chomp $line;
        die $ref.": Unknown tag type: ".$line."\n";
    } else {
        return $i;
    }
}

=item extract_tag($$)

This function returns the next tag from the input stream without the beginning
and end, in an array form, to maintain the references from the input file.  It
has two parameters: the type of the tag (as returned by tag_type) and a
boolean, that indicates if it should be removed from the input stream.

=cut

sub extract_tag {
    my ($self,$type,$remove) = (shift,shift,shift);
    my ($match1,$match2) = ($tag_types[$type]->{beginning},$tag_types[$type]->{end});
    my ($eof,@tag);
    if (defined($tag_types[$type]->{f_extract})) {
        ($eof,@tag) = &{$tag_types[$type]->{f_extract}}($self,$remove);
    } else {
        ($eof,@tag) = $self->get_string_until($match2.">",{include=>1,remove=>$remove,unquoted=>1});
    }
    $tag[0] =~ /^<\Q$match1\E(.*)$/s;
    $tag[0] = $1;
    $tag[$#tag-1] =~ /^(.*)\Q$match2\E>$/s;
    $tag[$#tag-1] = $1;
    return ($eof,@tag);
}

=item get_tag_name(@)

This function returns the name of the tag passed as an argument, in the array
form returned by extract_tag.

=cut

sub get_tag_name {
    my ($self,@tag)=@_;
    $tag[0] =~ /^(\S*)/;
    return $1;
}

=item breaking_tag()

This function returns a boolean that says if the next tag in the input stream
is a breaking tag or not (inline tag).  It leaves the input stream intact.

=cut

sub breaking_tag {
    my $self = shift;
    my $break;

    my $type = $self->tag_type;
    if ($type == -1) { return 0; }

#print "TAG TYPE = ".$type."\n";
    $break = $tag_types[$type]->{breaking};
    if (!defined($break)) {
        # This tag's breaking depends on its content
        my ($eof,@lines) = $self->extract_tag($type,0);
        $break = &{$tag_types[$type]->{f_breaking}}($self,@lines);
    }
#print "break = ".$break."\n";
    return $break;
}

=item treat_tag()

This function translates the next tag from the input stream.  Using each
tag type's custom translation functions.

=cut

sub treat_tag {
    my $self = shift;
    my $type = $self->tag_type;

    my ($match1,$match2) = ($tag_types[$type]->{beginning},$tag_types[$type]->{end});
    my ($eof,@lines) = $self->extract_tag($type,1);

    $lines[0] =~ /^(\s*)(.*)$/s;
    my $space1 = $1;
    $lines[0] = $2;
    $lines[$#lines-1] =~ /^(.*?)(\s*)$/s;
    my $space2 = $2;
    $lines[$#lines-1] = $1;

    # Calling this tag type's specific handling (translation of
    # attributes...)
    my $line = &{$tag_types[$type]->{f_translate}}($self,@lines);
    $self->pushline("<".$match1.$space1.$line.$space2.$match2.">");
    return $eof;
}

=item tag_in_list($@)

This function returns a string value that says if the first argument (a tag
hierarchy) matches any of the tags from the second argument (a list of tags
or tag hierarchies). If it doesn't match, it returns 0. Else, it returns the
matched tag's options (the characters in front of the tag) or 1 (if that tag
doesn't have options).

=back

=cut
sub tag_in_list ($$$) {
    my ($self,$path,$list) = @_;
    if ($self->{options}{'caseinsensitive'}) {
        $path = lc $path;
    }

    while (1) {
        if (defined $list->{$path}) {
            if (length $list->{$path}) {
                return $list->{$path};
            } else {
                return 1;
            }
        }
        last unless ($path =~ m/</);
        $path =~ s/^<.*?>//;
    }

    return 0;
}

=head2 WORKING WITH ATTRIBUTES

=over 4

=item treat_attributes(@)

This function handles the translation of the tags' attributes. It receives the tag
without the beginning / end marks, and then it finds the attributes, and it
translates the translatable ones (specified by the module option "attributes").
This returns a plain string with the translated tag.

=back

=cut

sub treat_attributes {
    my ($self,@tag)=@_;

    $tag[0] =~ /^(\S*)(.*)/s;
    my $text = $1;
    $tag[0] = $2;

    while (@tag) {
        my $complete = 1;

        $text .= $self->skip_spaces(\@tag);
        if (@tag) {
            # Get the attribute's name
            $complete = 0;

            $tag[0] =~ /^([^\s=]+)(.*)/s;
            my $name = $1;
            my $ref = $tag[1];
            $tag[0] = $2;
            $text .= $name;
            $text .= $self->skip_spaces(\@tag);
            if (@tag) {
                # Get the '='
                if ($tag[0] =~ /^=(.*)/s) {
                    $tag[0] = $1;
                    $text .= "=";
                    $text .= $self->skip_spaces(\@tag);
                    if (@tag) {
                        # Get the value
                        my $value="";
                        $ref=$tag[1];
                        my $quot=substr($tag[0],0,1);
                        if ($quot ne "\"" and $quot ne "'") {
                            # Unquoted value
                            $quot="";
                            $tag[0] =~ /^(\S+)(.*)/s;
                            $value = $1;
                            $tag[0] = $2;
                        } else {
                            # Quoted value
                            $text .= $quot;
                            $tag[0] =~ /^\Q$quot\E(.*)/s;
                            $tag[0] = $1;
                            while ($tag[0] !~ /\Q$quot\E/) {
                                $value .= $tag[0];
                                shift @tag;
                                shift @tag;
                            }
                            $tag[0] =~ /^(.*?)\Q$quot\E(.*)/s;
                            $value .= $1;
                            $tag[0] = $2;
                        }
                        $complete = 1;
                        if ($self->tag_in_list($self->get_path.$name,$self->{attributes})) {
                            $text .= $self->found_string($value, $ref, { type=>"attribute", attribute=>$name });
                        } else {
                            print wrap_ref_mod($ref, "po4a::xml", dgettext("po4a", "Content of attribute %s excluded: %s"), $self->get_path.$name, $value)
                                   if $self->debug();
                            $text .= $self->recode_skipped_text($value);
                        }
                        $text .= $quot;
                    }
                }
            }

            unless ($complete) {
                my $ontagerror = $self->{options}{'ontagerror'};
                if ($ontagerror eq "warn") {
                    warn wrap_ref_mod($ref, "po4a::xml", dgettext ("po4a", "Bad attribute syntax.  Continuing..."));
                } elsif ($ontagerror ne "silent") {
                    die wrap_ref_mod($ref, "po4a::xml", dgettext ("po4a", "Bad attribute syntax"));
                }
            }
        }
    }
    return $text;
}

# Returns an empty string if the content in the $path should not be
# translated.
#
# Otherwise, returns the set of options for translation:
#   w: the content shall be re-wrapped
#   W: the content shall not be re-wrapped
#   i: the tag shall be inlined
#   p: a placeholder shall replace the tag (and its content)
#   n: a custom tag
#
# A translatable inline tag in an untranslated tag is treated as a translatable breaking tag.
sub get_translate_options {
    my $self = shift;
    my $path = shift;

    if (defined $translate_options_cache{$path}) {
        return $translate_options_cache{$path};
    }

    my $options = "";
    my $translate = 0;
    my $usedefault = 1;

    my $inlist = 0;
    my $tag = $self->get_tag_from_list($path, $self->{tags});
    if (defined $tag) {
        $inlist = 1;
    }
    if ($self->{options}{'tagsonly'} eq $inlist) {
        $usedefault = 0;
        if (defined $tag) {
            $options = $tag;
            $options =~ s/<.*$//;
        } else {
            if ($self->{options}{'wrap'}) {
                $options = "w";
            } else {
                $options = "W";
            }
        }
        $translate = 1;
    }

# TODO: a less precise set of tags should not override a more precise one
    # The tags and tagsonly options are deprecated.
    # The translated and untranslated options have an higher priority.
    $tag = $self->get_tag_from_list($path, $self->{translated});
    if (defined $tag) {
        $usedefault = 0;
        $options = $tag;
        $options =~ s/<.*$//;
        $translate = 1;
    }

    if ($translate and $options !~ m/w/i) {
        $options .= ($self->{options}{'wrap'})?"w":"W";
    }

    if (not defined $tag) {
        $tag = $self->get_tag_from_list($path, $self->{untranslated});
        if (defined $tag) {
            $usedefault = 0;
            $options = "";
            $translate = 0;
        }
    }

    $tag = $self->get_tag_from_list($path, $self->{inline});
    if (defined $tag) {
        $usedefault = 0;
        $options .= "i";
    } else {
        $tag = $self->get_tag_from_list($path, $self->{placeholder});
        if (defined $tag) {
            $usedefault = 0;
            $options .= "p";
        }
    }

    $tag = $self->get_tag_from_list($path, $self->{customtag});
    if (defined $tag) {
        $usedefault = 0;
        $options = "in"; # This erase any other setting
    }

    if ($usedefault) {
        $options = $self->{options}{'defaulttranslateoption'};
    }

    # A translatable inline tag in an untranslated tag is treated as a
    # translatable breaking tag.
    if ($options =~ m/i/) {
        my $ppath = $path;
        $ppath =~ s/<[^>]*>$//;
        my $poptions = $self->get_translate_options ($ppath);
        if ($poptions eq "") {
            $options =~ s/i//;
        }
    }

    if ($options =~ m/i/ and $self->{options}{'foldattributes'}) {
        $options .= "f";
    }

    $translate_options_cache{$path} = $options;
    return $options;
}


# Return the tag (or biggest set of tags) of a list which matches with the
# given path.
#
# The tag (or set of tags) is returned with its options.
#
# If no tags could match the path, undef is returned.
sub get_tag_from_list ($$$) {
    my ($self,$path,$list) = @_;
    if ($self->{options}{'caseinsensitive'}) {
        $path = lc $path;
    }

    while (1) {
        if (defined $list->{$path}) {
            return $list->{$path}.$path;
        }
        last unless ($path =~ m/</);
        $path =~ s/^<.*?>//;
    }

    return undef;
}



sub treat_content {
    my $self = shift;
    my $blank="";
    # Indicates if the paragraph will have to be translated
    my $translate = "";

    my ($eof,@paragraph)=$self->get_string_until('<',{remove=>1});

    while (!$eof and !$self->breaking_tag) {
    NEXT_TAG:
        my @text;
        my $type = $self->tag_type;
        my $f_extract = $tag_types[$type]->{'f_extract'};
        if (    defined($f_extract)
            and $f_extract eq \&tag_extract_comment) {
            # Remove the content of the comments
            ($eof, @text) = $self->extract_tag($type,1);
            $text[$#text-1] .= "\0";
            if ($tag_types[$type]->{'beginning'} eq "!--#") {
                $text[0] = "#".$text[0];
            }
            push @comments, @text;
        } else {
            my ($tmpeof, @tag) = $self->extract_tag($type,0);
            # Append the found inline tag
            ($eof,@text)=$self->get_string_until('>',
                                                 {include=>1,
                                                  remove=>1,
                                                  unquoted=>1});
            # Append or remove the opening/closing tag from
            # the tag path
            if ($tag_types[$type]->{'end'} eq "") {
                if ($tag_types[$type]->{'beginning'} eq "") {
                    # Opening inline tag
                    my $cur_tag_name = $self->get_tag_name(@tag);
                    my $t_opts = $self->get_translate_options($self->get_path($cur_tag_name));
                    if ($t_opts =~ m/p/) {
                        # We enter a new holder.
                        # Append a <placeholder ...> tag to the current
                        # paragraph, and save the @paragraph in the
                        # current holder.
                        my $last_holder = $save_holders[$#save_holders];
                        my $placeholder_str = "<placeholder type=\"".$cur_tag_name."\" id=\"".($#{$last_holder->{'sub_translations'}}+1)."\"/>";
                        push @paragraph, ($placeholder_str, $text[1]);
                        my @saved_paragraph = @paragraph;

                        $last_holder->{'paragraph'} = \@saved_paragraph;

                        # Then we must push a new holder
                        my @new_paragraph = ();
                        my @sub_translations = ();
                        my %folded_attributes;
                        my %new_holder = ('paragraph' => \@new_paragraph,
                                          'open' => $self->join_lines(@text),
                                          'translation' => "",
                                          'close' => undef,
                                          'sub_translations' => \@sub_translations,
                                          'folded_attributes' => \%folded_attributes);
                        push @save_holders, \%new_holder;
                        @text = ();

                        # The current @paragraph
                        # (for the current holder)
                        # is empty.
                        @paragraph = ();
                    } elsif ($t_opts =~ m/f/) {
                        my $tag_full = $self->join_lines(@text);
                        my $tag_ref = $text[1];
                        if ($tag_full =~ m/^<\s*\S+\s+\S.*>$/s) {
                            my $holder = $save_holders[$#save_holders];
                            my $id = 0;
                            foreach (keys %{$holder->{folded_attributes}}) {
                                $id = $_ + 1 if ($_ >= $id);
                            }
                            $holder->{folded_attributes}->{$id} = $tag_full;

                            @text = ("<$cur_tag_name po4a-id=$id>", $tag_ref);
                        }
                    }
                    unless ($t_opts =~ m/n/) {
                        push @path, $cur_tag_name;
                    }
                } elsif ($tag_types[$type]->{'beginning'} eq "/") {
                    # Closing inline tag

                    # Check if this is closing the
                    # last opening tag we detected.
                    my $test = pop @path;
                    my $name = $self->get_tag_name(@tag);
                    if (!defined($test) ||
                        $test ne $name ) {
                        my $ontagerror = $self->{options}{'ontagerror'};
                        if ($ontagerror eq "warn") {
                            warn wrap_ref_mod($tag[1], "po4a::xml", dgettext("po4a", "Unexpected closing tag </%s> found. The main document may be wrong.  Continuing..."), $name);
                        } elsif ($ontagerror ne "silent") {
                            die wrap_ref_mod($tag[1], "po4a::xml", dgettext("po4a", "Unexpected closing tag </%s> found. The main document may be wrong."), $name);
                        }
                    }

                    if ($self->get_translate_options($self->get_path($self->get_tag_name(@tag))) =~ m/p/) {
                        # This closes the current holder.

                        push @path, $self->get_tag_name(@tag);
                        # Now translate this paragraph if needed.
                        # This will call pushline and append the
                        # translation to the current holder's translation.
                        $self->translate_paragraph(@paragraph);
                        pop @path;

                        # Now that this holder is closed, we can remove
                        # the holder from the stack.
                        my $holder = pop @save_holders;
                        # We need to keep the translation of this holder
                        my $translation = $holder->{'open'}.$holder->{'translation'};
                        $translation .= $self->join_lines(@text);

                        @text = ();

                        # Then we store the translation in the previous
                        # holder's sub_translations array
                        my $previous_holder = $save_holders[$#save_holders];
                        push @{$previous_holder->{'sub_translations'}}, $translation;
                        # We also need to restore the @paragraph array, as
                        # it was before we encountered the holder.
                        @paragraph = @{$previous_holder->{'paragraph'}};
                    }
                }
            }
            push @paragraph, @text;
        }

        # Next tag
        ($eof,@text)=$self->get_string_until('<',{remove=>1});
        if ($#text > 0) {
            # Check if text (extracted after the inline tag)
            # has to be translated
            push @paragraph, @text;
        }
    }

    # This strips the extracted strings
    # (only if you don't specify the 'nostrip' option, and if the
    # paragraph can be re-wrapped)
    $translate = $self->get_translate_options($self->get_path);
    if (!$self->{options}{'nostrip'} and $translate !~ m/W/) {
        my $clean = 0;
        # Clean the beginning
        while (!$clean and $#paragraph > 0) {
            $paragraph[0] =~ /^(\s*)(.*)/s;
            my $match = $1;
            if ($paragraph[0] eq $match) {
                if ($match ne "") {
                    $self->pushline($match);
                }
                shift @paragraph;
                shift @paragraph;
            } else {
                $paragraph[0] = $2;
                if ($match ne "") {
                    $self->pushline($match);
                }
                $clean = 1;
            }
        }
        $clean = 0;
        # Clean the end
        while (!$clean and $#paragraph > 0) {
            $paragraph[$#paragraph-1] =~ /^(.*?)(\s*)$/s;
            my $match = $2;
            if ($paragraph[$#paragraph-1] eq $match) {
                if ($match ne "") {
                    $blank = $match.$blank;
                }
                pop @paragraph;
                pop @paragraph;
            } else {
                $paragraph[$#paragraph-1] = $1;
                if ($match ne "") {
                    $blank = $match.$blank;
                }
                $clean = 1;
            }
        }
    }

    # Translate the string when needed
    # This will either push the translation in the translated document or
    # in the current holder translation.
    $self->translate_paragraph(@paragraph);

    # Push the trailing blanks
    if ($blank ne "") {
        $self->pushline($blank);
    }
    return $eof;
}

# Translate a @paragraph array of (string, reference).
# The $translate argument indicates if the strings must be translated or
# just pushed
sub translate_paragraph {
    my $self = shift;
    my @paragraph = @_;
    my $translate = $self->get_translate_options($self->get_path);

    while (    (scalar @paragraph)
           and ($paragraph[0] =~ m/^\s*\n/s)) {
        $self->pushline($paragraph[0]);
        shift @paragraph;
        shift @paragraph;
    }

    my $comments;
    while (@comments) {
        my ($comment,$eoc);
        do {
            my ($t,$l) = (shift @comments, shift @comments);
            $t =~ s/\n?(\0)?$//;
            $eoc = $1;
            $comment .= "\n" if defined $comment;
            $comment .= $t;
        } until ($eoc);
        $comments .= "\n" if defined $comments;
        $comments .= $comment;
        $self->pushline("<!--".$comment."-->\n") if defined $comment;
    }
    @comments = ();

    if ($self->{options}{'cpp'}) {
        my @tmp = @paragraph;
        @paragraph = ();
        while (@tmp) {
            my ($t,$l) = (shift @tmp, shift @tmp);
            # #include can be followed by a filename between
            # <> brackets. In that case, the argument won't be
            # handled in the same call to translate_paragraph.
            # Thus do not try to match "include ".
            if ($t =~ m/^#[ \t]*(if |endif|undef |include|else|ifdef |ifndef |define )/si) {
                if (@paragraph) {
                    $self->translate_paragraph(@paragraph);
                    @paragraph = ();
                    $self->pushline("\n");
                }
                $self->pushline($t);
            } else {
                push @paragraph, ($t,$l);
            }
        }
    }

    my $para = $self->join_lines(@paragraph);
    if ( length($para) > 0 ) {
        if ($translate ne "") {
            # This tag should be translated
            $self->pushline($self->found_string(
                $para,
                $paragraph[1], {
                    type=>"tag",
                    tag_options=>$translate,
                    comments=>$comments
                }));
        } else {
            # Inform that this tag isn't translated in debug mode
            print wrap_ref_mod($paragraph[1], "po4a::xml", dgettext ("po4a", "Content of tag %s excluded: %s"), $self->get_path, $para)
                   if $self->debug();
            $self->pushline($self->recode_skipped_text($para));
        }
    }
    # Now the paragraph is fully translated.
    # If we have all the holders' translation, we can replace the
    # placeholders by their translations.
    # We must wait to have all the translations because the holders are
    # numbered.
    {
        my $holder = $save_holders[$#save_holders];
        my $translation = $holder->{'translation'};

        # Count the number of <placeholder ...> in $translation
        my $count = 0;
        my $str = $translation;
        while (    (defined $str)
               and ($str =~ m/^.*?<placeholder\s+type="[^"]+"\s+id="(\d+)"\s*\/>(.*)$/s)) {
            $count += 1;
            $str = $2;
            if ($holder->{'sub_translations'}->[$1] =~ m/<placeholder\s+type="[^"]+"\s+id="(\d+)"\s*\/>/s) {
                $count = -1;
                last;
            }
        }

        if (    (defined $translation)
            and (scalar(@{$holder->{'sub_translations'}}) == $count)) {
            # OK, all the holders of the current paragraph are
            # closed (and translated).
            # Replace them by their translation.
            while ($translation =~ m/^(.*?)<placeholder\s+type="[^"]+"\s+id="(\d+)"\s*\/>(.*)$/s) {
                # FIXME: we could also check that
                #          * the holder exists
                #          * all the holders are used
                $translation = $1.$holder->{'sub_translations'}->[$2].$3;
            }
            # We have our translation
            $holder->{'translation'} = $translation;
            # And there is no need for any holder in it.
            my @sub_translations = ();
            $holder->{'sub_translations'} = \@sub_translations;
        }
    }

}



=head2 WORKING WITH THE MODULE OPTIONS

=over 4

=item treat_options()

This function fills the internal structures that contain the tags, attributes
and inline data with the options of the module (specified in the command-line
or in the initialize function).

=back

=cut

sub treat_options {
    my $self = shift;

    if ($self->{options}{'caseinsensitive'}) {
        $self->{options}{'nodefault'}             = lc $self->{options}{'nodefault'};
        $self->{options}{'tags'}                  = lc $self->{options}{'tags'};
        $self->{options}{'break'}                 = lc $self->{options}{'break'};
        $self->{options}{'_default_break'}        = lc $self->{options}{'_default_break'};
        $self->{options}{'translated'}            = lc $self->{options}{'translated'};
        $self->{options}{'_default_translated'}   = lc $self->{options}{'_default_translated'};
        $self->{options}{'untranslated'}          = lc $self->{options}{'untranslated'};
        $self->{options}{'_default_untranslated'} = lc $self->{options}{'_default_untranslated'};
        $self->{options}{'attributes'}            = lc $self->{options}{'attributes'};
        $self->{options}{'_default_attributes'}   = lc $self->{options}{'_default_attributes'};
        $self->{options}{'inline'}                = lc $self->{options}{'inline'};
        $self->{options}{'_default_inline'}       = lc $self->{options}{'_default_inline'};
        $self->{options}{'placeholder'}           = lc $self->{options}{'placeholder'};
        $self->{options}{'_default_placeholder'}  = lc $self->{options}{'_default_placeholder'};
        $self->{options}{'customtag'}             = lc $self->{options}{'customtag'};
        $self->{options}{'_default_customtag'}    = lc $self->{options}{'_default_customtag'};
    }

    $self->{options}{'nodefault'} =~ /^\s*(.*)\s*$/s;
    my %list_nodefault;
    foreach (split(/\s+/s,$1)) {
        $list_nodefault{$_} = 1;
    }
    $self->{nodefault} = \%list_nodefault;

    $self->{options}{'tags'} =~ /^\s*(.*)\s*$/s;
    if (length $self->{options}{'tags'}) {
        warn wrap_mod("po4a::xml",
                     dgettext("po4a",
                              "The '%s' option is deprecated. Please use the translated/untranslated and/or break/inline/placeholder categories."), "tags");
    }
    foreach (split(/\s+/s,$1)) {
        $_ =~ m/^(.*?)(<.*)$/;
        $self->{tags}->{$2} = $1 || "";
    }

    if ($self->{options}{'tagsonly'}) {
        warn wrap_mod("po4a::xml",
                     dgettext("po4a",
                              "The '%s' option is deprecated. Please use the translated/untranslated and/or break/inline/placeholder categories."), "tagsonly");
    }

    $self->{options}{'break'} =~ /^\s*(.*)\s*$/s;
    foreach my $tag (split(/\s+/s,$1)) {
        $tag =~ m/^(.*?)(<.*)$/;
        $self->{break}->{$2} = $1 || "";
    }
    $self->{options}{'_default_break'} =~ /^\s*(.*)\s*$/s;
    foreach my $tag (split(/\s+/s,$1)) {
        $tag =~ m/^(.*?)(<.*)$/;
        $self->{break}->{$2} = $1 || ""
            unless    $list_nodefault{$2}
                   or defined $self->{break}->{$2};
    }

    $self->{options}{'translated'} =~ /^\s*(.*)\s*$/s;
    foreach my $tag (split(/\s+/s,$1)) {
        $tag =~ m/^(.*?)(<.*)$/;
        $self->{translated}->{$2} = $1 || "";
    }
    $self->{options}{'_default_translated'} =~ /^\s*(.*)\s*$/s;
    foreach my $tag (split(/\s+/s,$1)) {
        $tag =~ m/^(.*?)(<.*)$/;
        $self->{translated}->{$2} = $1 || ""
            unless    $list_nodefault{$2}
                   or defined $self->{translated}->{$2};
    }

    $self->{options}{'untranslated'} =~ /^\s*(.*)\s*$/s;
    foreach my $tag (split(/\s+/s,$1)) {
        $tag =~ m/^(.*?)(<.*)$/;
        $self->{untranslated}->{$2} = $1 || "";
    }
    $self->{options}{'_default_untranslated'} =~ /^\s*(.*)\s*$/s;
    foreach my $tag (split(/\s+/s,$1)) {
        $tag =~ m/^(.*?)(<.*)$/;
        $self->{untranslated}->{$2} = $1 || ""
            unless    $list_nodefault{$2}
                   or defined $self->{untranslated}->{$2};
    }

    $self->{options}{'attributes'} =~ /^\s*(.*)\s*$/s;
    foreach my $tag (split(/\s+/s,$1)) {
        if ($tag =~ m/^(.*?)(<.*)$/) {
            $self->{attributes}->{$2} = $1 || "";
        } else {
            $self->{attributes}->{$tag} = "";
        }
    }
    $self->{options}{'_default_attributes'} =~ /^\s*(.*)\s*$/s;
    foreach my $tag (split(/\s+/s,$1)) {
        if ($tag =~ m/^(.*?)(<.*)$/) {
            $self->{attributes}->{$2} = $1 || ""
                unless    $list_nodefault{$2}
                       or defined $self->{attributes}->{$2};
        } else {
            $self->{attributes}->{$tag} = ""
                unless    $list_nodefault{$tag}
                       or defined $self->{attributes}->{$tag};
        }
    }

    $self->{options}{'inline'} =~ /^\s*(.*)\s*$/s;
    foreach my $tag (split(/\s+/s,$1)) {
        $tag =~ m/^(.*?)(<.*)$/;
        $self->{inline}->{$2} = $1 || "";
    }
    $self->{options}{'_default_inline'} =~ /^\s*(.*)\s*$/s;
    foreach my $tag (split(/\s+/s,$1)) {
        $tag =~ m/^(.*?)(<.*)$/;
        $self->{inline}->{$2} = $1 || ""
            unless    $list_nodefault{$2}
                   or defined $self->{inline}->{$2};
    }

    $self->{options}{'placeholder'} =~ /^\s*(.*)\s*$/s;
    foreach my $tag (split(/\s+/s,$1)) {
        $tag =~ m/^(.*?)(<.*)$/;
        $self->{placeholder}->{$2} = $1 || "";
    }
    $self->{options}{'_default_placeholder'} =~ /^\s*(.*)\s*$/s;
    foreach my $tag (split(/\s+/s,$1)) {
        $tag =~ m/^(.*?)(<.*)$/;
        $self->{placeholder}->{$2} = $1 || ""
            unless    $list_nodefault{$2}
                   or defined $self->{placeholder}->{$2};
    }

    $self->{options}{'customtag'} =~ /^\s*(.*)\s*$/s;
    foreach my $tag (split(/\s+/s,$1)) {
        $tag =~ m/^(.*?)(<.*)$/;
        $self->{customtag}->{$2} = $1 || "";
    }
    $self->{options}{'_default_customtag'} =~ /^\s*(.*)\s*$/s;
    foreach my $tag (split(/\s+/s,$1)) {
        $tag =~ m/^(.*?)(<.*)$/;
        $self->{customtag}->{$2} = $1 || ""
            unless    $list_nodefault{$2}
                   or defined $self->{customtag}->{$2};
    }

    # There should be no translated and untranslated tags
    foreach my $tag (keys %{$self->{translated}}) {
        die wrap_mod("po4a::xml",
                     dgettext("po4a",
                              "Tag '%s' both in the %s and %s categories."), $tag, "translated", "untranslated")
            if defined $self->{untranslated}->{$tag};
    }
    # There should be no inline, break, placeholder, and customtag tags
    foreach my $tag (keys %{$self->{inline}}) {
        die wrap_mod("po4a::xml",
                     dgettext("po4a",
                              "Tag '%s' both in the %s and %s categories."), $tag, "inline", "break")
            if defined $self->{break}->{$tag};
        die wrap_mod("po4a::xml",
                     dgettext("po4a",
                              "Tag '%s' both in the %s and %s categories."), $tag, "inline", "placeholder")
            if defined $self->{placeholder}->{$tag};
        die wrap_mod("po4a::xml",
                     dgettext("po4a",
                              "Tag '%s' both in the %s and %s categories."), $tag, "inline", "customtag")
            if defined $self->{customtag}->{$tag};
    }
    foreach my $tag (keys %{$self->{break}}) {
        die wrap_mod("po4a::xml",
                     dgettext("po4a",
                              "Tag '%s' both in the %s and %s categories."), $tag, "break", "placeholder")
            if defined $self->{placeholder}->{$tag};
        die wrap_mod("po4a::xml",
                     dgettext("po4a",
                              "Tag '%s' both in the %s and %s categories."), $tag, "break", "customtag")
            if defined $self->{customtag}->{$tag};
    }
    foreach my $tag (keys %{$self->{placeholder}}) {
        die wrap_mod("po4a::xml",
                     dgettext("po4a",
                              "Tag '%s' both in the %s and %s categories."), $tag, "placeholder", "customtag")
            if defined $self->{customtag}->{$tag};
    }
}

=head2 GETTING TEXT FROM THE INPUT DOCUMENT

=over

=item get_string_until($%)

This function returns an array with the lines (and references) from the input
document until it finds the first argument.  The second argument is an options
hash. Value 0 means disabled (the default) and 1, enabled.

The valid options are:

=over 4

=item B<include>

This makes the returned array to contain the searched text

=item B<remove>

This removes the returned stream from the input

=item B<unquoted>

This ensures that the searched text is outside any quotes

=back

=cut

sub get_string_until {
    my ($self,$search) = (shift,shift);
    my $options = shift;
    my ($include,$remove,$unquoted, $regex) = (0,0,0,0);

    if (defined($options->{include})) { $include = $options->{include}; }
    if (defined($options->{remove})) { $remove = $options->{remove}; }
    if (defined($options->{unquoted})) { $unquoted = $options->{unquoted}; }
    if (defined($options->{regex})) { $regex = $options->{regex}; }

    my ($line,$ref) = $self->shiftline();
    my (@text,$paragraph);
    my ($eof,$found) = (0,0);

    $search = "\Q$search\E" unless $regex;
    while (defined($line) and !$found) {
        push @text, ($line,$ref);
        $paragraph .= $line;
        if ($unquoted) {
            if ( $paragraph =~ /^((\".*?\")|(\'.*?\')|[^\"\'])*$search/s ) {
                $found = 1;
            }
        } else {
            if ( $paragraph =~ /$search/s ) {
                $found = 1;
            }
        }
        if (!$found) {
            ($line,$ref)=$self->shiftline();
        }
    }

    if (!defined($line)) { $eof = 1; }

    if ( $found ) {
        $line = "";
        if($unquoted) {
            $paragraph =~ /^(?:(?:\".*?\")|(?:\'.*?\')|[^\"\'])*?$search(.*)$/s;
            $line = $1;
            $text[$#text-1] =~ s/\Q$line\E$//s;
        } else {
            $paragraph =~ /$search(.*)$/s;
            $line = $1;
            $text[$#text-1] =~ s/\Q$line\E$//s;
        }
        if(!$include) {
            $text[$#text-1] =~ /^(.*)($search.*)$/s;
            $text[$#text-1] = $1;
            $line = $2.$line;
        }
        if (defined($line) and ($line ne "")) {
            $self->unshiftline ($line,$text[$#text]);
        }
    }
    if (!$remove) {
        $self->unshiftline (@text);
    }

    #If we get to the end of the file, we return the whole paragraph
    return ($eof,@text);
}

=item skip_spaces(\@)

This function receives as argument the reference to a paragraph (in the format
returned by get_string_until), skips his heading spaces and returns them as
a simple string.

=cut

sub skip_spaces {
    my ($self,$pstring)=@_;
    my $space="";

    while (@$pstring and (@$pstring[0] =~ /^(\s+)(.*)$/s or @$pstring[0] eq "")) {
        if (@$pstring[0] ne "") {
            $space .= $1;
            @$pstring[0] = $2;
        }

        if (@$pstring[0] eq "") {
            shift @$pstring;
            shift @$pstring;
        }
    }
    return $space;
}

=item join_lines(@)

This function returns a simple string with the text from the argument array
(discarding the references).

=cut

sub join_lines {
    my ($self,@lines)=@_;
    my ($line,$ref);
    my $text = "";
    while ($#lines > 0) {
        ($line,$ref) = (shift @lines,shift @lines);
        $text .= $line;
    }
    return $text;
}

=back

=head1 STATUS OF THIS MODULE

This module can translate tags and attributes.

=head1 TODO LIST

DOCTYPE (ENTITIES)

There is a minimal support for the translation of entities. They are
translated as a whole, and tags are not taken into account. Multilines
entities are not supported and entities are always rewrapped during the
translation.

MODIFY TAG TYPES FROM INHERITED MODULES
(move the tag_types structure inside the $self hash?)

=head1 SEE ALSO

L<Locale::Po4a::TransTractor(3pm)|Locale::Po4a::TransTractor>,
L<po4a(7)|po4a.7>

=head1 AUTHORS

 Jordi Vilalta <jvprat@gmail.com>
 Nicolas François <nicolas.francois@centraliens.net>

=head1 COPYRIGHT AND LICENSE

 Copyright (c) 2004 by Jordi Vilalta  <jvprat@gmail.com>
 Copyright (c) 2008-2009 by Nicolas François <nicolas.francois@centraliens.net>

This program is free software; you may redistribute it and/or modify it
under the terms of GPL (see the COPYING file).

=cut

1;
