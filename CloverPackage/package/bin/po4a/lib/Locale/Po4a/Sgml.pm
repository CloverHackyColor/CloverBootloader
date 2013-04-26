#!/usr/bin/perl -w

# Po4a::Sgml.pm
#
# extract and translate translatable strings from an sgml based document.
#
# This code is an adapted version of sgmlspl (SGML postprocessor for the
#   SGMLS and NSGMLS parsers) which was:
#
# Copyright (c) 1995 by David Megginson <dmeggins@aix1.uottawa.ca>
#
# The adaptation for po4a was done by Denis Barbier <barbier@linuxfr.org>,
# Martin Quinson (mquinson#debian.org) and others.
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

Locale::Po4a::Sgml - convert SGML documents from/to PO files

=head1 DESCRIPTION

The po4a (PO for anything) project goal is to ease translations (and more
interestingly, the maintenance of translations) using gettext tools on
areas where they were not expected like documentation.

Locale::Po4a::Sgml is a module to help the translation of documentation in
the SGML format into other [human] languages.

This module uses B<nsgmls> to parse the SGML files. Make sure it is
installed.
Also make sure that the DTD of the SGML files are installed in the system.

=head1 OPTIONS ACCEPTED BY THIS MODULE

=over 4

=item B<debug>

Space separated list of keywords indicating which part you want to debug. Possible values are: tag, generic, entities and refs.

=item B<verbose>

Give more information about what's going on.

=item B<translate>

Space separated list of extra tags (beside the DTD provided ones) whose
content should form an extra msgid.

=item B<section>

Space separated list of extra tags (beside the DTD provided ones)
containing other tags, some of them being of category B<translate>.

=item B<indent>

Space separated list of tags which increase the indentation level.

=item B<verbatim>

The layout within those tags should not be changed. The paragraph won't get
wrapped, and no extra indentation space or new line will be added for
cosmetic purpose.

=item B<empty>

Tags not needing to be closed.

=item B<ignore>

Tags ignored and considered as plain char data by po4a. That is to say that
they can be part of an msgid. For example, E<lt>bE<gt> is a good candidate
for this category since putting it in the translate section would create
msgids not being whole sentences, which is bad.

=item B<attributes>

A space separated list of attributes that need to be translated. You can
specify the attributes by their name (for example, "lang"), but you can also
prefix it with a tag hierarchy, to specify that this attribute will only be
translated when it is into the specified tag. For example:
E<lt>bbbE<gt>E<lt>aaaE<gt>lang specifies that the lang attribute will only be
translated if it is in an E<lt>aaaE<gt> tag, which is in a E<lt>bbbE<gt> tag.
The tag names are actually regular expressions so you can also write things
like E<lt>aaa|bbbbE<gt>lang to only translate lang attributes that are in
an E<lt>aaaE<gt> or a E<lt>bbbE<gt> tag.

=item B<qualify>

A space separated list of attributes for which the translation must be
qualified by the attribute name. Note that this setting automatically adds the
given attribute into the 'attributes' list too.

=item B<force>

Proceed even if the DTD is unknown or if nsgmls finds errors in the input
file.

=item B<include-all>

By default, msgids containing only one entity (like '&version;') are skipped
for the translator comfort. Activating this option prevents this
optimisation. It can be useful if the document contains a construction like
"<title>&Aacute;</title>", even if I doubt such things to ever happen...

=item B<ignore-inclusion>

Space separated list of entities that won't be inlined.
Use this option with caution: it may cause nsgmls (used internally) to add
tags and render the output document invalid.

=back

=head1 STATUS OF THIS MODULE

The result is perfect. I.e., the generated documents are exactly the
same. But there are still some problems:

=over 2

=item *

The error output of nsgmls is redirected to /dev/null, which is clearly
bad. I don't know how to avoid that.

The problem is that I have to "protect" the conditional inclusions (i.e. the
C<E<lt>! [ %foo [> and C<]]E<gt>> stuff) from nsgmls. Otherwise
nsgmls eats them, and I don't know how to restore them in the final
document. To prevent that, I rewrite them to C<{PO4A-beg-foo}> and
C<{PO4A-end}>.

The problem with this is that the C<{PO4A-end}> and such I add are valid in
the document (not in a E<lt>pE<gt> tag or so).

Everything works well with nsgmls's output redirected that way, but it will
prevent us from detecting that the document is badly formatted.

=item *

It does work only with the DebianDoc and DocBook DTD. Adding support for a
new DTD should be very easy. The mechanism is the same for every DTD, you just
have to give a list of the existing tags and some of their characteristics.

I agree, this needs some more documentation, but it is still considered as
beta, and I hate to document stuff which may/will change.

=item *

Warning, support for DTDs is quite experimental. I did not read any
reference manual to find the definition of every tag. I did add tag
definition to the module 'till it works for some documents I found on the
net. If your document use more tags than mine, it won't work. But as I said
above, fixing that should be quite easy.

I did test DocBook against the SAG (System Administrator Guide) only, but
this document is quite big, and should use most of the DocBook
specificities.

For DebianDoc, I tested some of the manuals from the DDP, but not all yet.

=item *

In case of file inclusion, string reference of messages in PO files
(i.e. lines like C<#: en/titletoc.sgml:9460>) will be wrong.

This is because I preprocess the file to protect the conditional inclusion
(i.e. the C<E<lt>! [ %foo [> and C<]]E<gt>> stuff) and some entities (like
&version;) from nsgmls because I want them verbatim to the generated
document. For that, I make a temp copy of the input file and do all the
changes I want to this before passing it to nsgmls for parsing.

So that it works, I replace the entities asking for a file inclusion by the
content of the given file (so that I can protect what needs to be in a subfile
also). But nothing is done so far to correct the references (i.e., filename
and line number) afterward. I'm not sure what the best thing to do is.

=back

=cut

package Locale::Po4a::Sgml;

use 5.006;
use strict;
use warnings;


require Exporter;
use vars qw(@ISA @EXPORT);
@ISA = qw(Locale::Po4a::TransTractor);
@EXPORT = qw();

use Locale::Po4a::TransTractor;
use Locale::Po4a::Common;

eval qq{use SGMLS};
if ($@) {
  die wrap_mod("po4a::sgml", dgettext("po4a","The needed module SGMLS.pm was not found and needs to be installed. It can be found on the CPAN, in package libsgmls-perl on debian, etc."));
}

use File::Temp;

my %debug=('tag'      => 0,
           'generic'  => 0,
           'entities' => 0,
           'refs'     => 0,
           'nsgmls'   => 0);

my $xmlprolog = undef; # the '<?xml ... ?>' line if existing

sub initialize {
    my $self = shift;
    my %options = @_;

    $self->{options}{'translate'}='';
    $self->{options}{'section'}='';
    $self->{options}{'indent'}='';
    $self->{options}{'empty'}='';
    $self->{options}{'verbatim'}='';
    $self->{options}{'ignore'}='';
    $self->{options}{'ignore-inclusion'}='';

    $self->{options}{'include-all'}='';

    $self->{options}{'force'}='';

    $self->{options}{'verbose'}='';
    $self->{options}{'debug'}='';

    foreach my $opt (keys %options) {
        if ($options{$opt}) {
            die wrap_mod("po4a::sgml", dgettext ("po4a", "Unknown option: %s"), $opt) unless exists $self->{options}{$opt};
            $self->{options}{$opt} = $options{$opt};
        }
    }
    if ($options{'debug'}) {
        foreach (split /\s+/, $options{'debug'}) {
            $debug{$_} = 1;
        }
    }
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

#
# Filter out some uninteresting strings for translation
#
sub translate {
    my ($self)=(shift);
    my ($string,$ref,$type)=(shift,shift,shift);
    my (%options)=@_;

    # don't translate entries composed of one entity
    if ( (($string =~ /^&[^;]*;$/) || ($options{'wrap'} && $string =~ /^\s*&[^;]*;\s*$/))
         && !($self->{options}{'include-all'}) ){
        warn wrap_mod("po4a::sgml", dgettext("po4a", "msgid skipped to help translators (contains only an entity)"), $string)
            unless $self->verbose() <= 0;
        return $string.($options{'wrap'}?"\n":"");
    }
    # don't translate entries composed of tags only
    if ( $string =~ /^(((<[^>]*>)|\s)*)$/
         && !($self->{options}{'include-all'}) ) {
        warn wrap_mod("po4a::sgml", dgettext("po4a", "msgid skipped to help translators (contains only tags)"), $string)
               unless $self->verbose() <= 0;
        return $string.($options{'wrap'}?"\n":"");
    }

    # don't translate entries composed of marked section tags only
    if (   ($string =~ /^(?:<!\s*\[\s*[^\[]+\s*\[|\]\s*]\s*>|\s)*$/)
        && !($self->{options}{'include-all'})) {
        warn wrap_mod("po4a::sgml", dgettext("po4a", "msgid skipped to ".
                      "help translators (contains only opening or closing ".
                      "tags of marked sections)"), $string)
               unless $self->verbose() <= 0;
        return $string.($options{'wrap'}?"\n":"");
    }

    $string = $self->SUPER::translate($string,$ref,$type,%options);

    $string = $self->post_trans($string,$ref,$type);

    return $string;
}

sub post_trans {
    my ($self,$str,$ref,$type)=@_;

    # Change ascii non-breaking space to an &nbsp;
    my $nbs_out = "\xA0";
    my $enc_length = Encode::from_to($nbs_out, "latin1",
                                               $self->get_out_charset);
    $str =~ s/\Q$nbs_out/&nbsp;/g if defined $enc_length;

    return $str;
}

#
# Make sure our cruft is removed from the file
#
sub pushline {
    my ($self,$line)=@_;
    $line =~ s/{PO4A-amp}/&/g;
    $self->SUPER::pushline($line);
}

sub set_tags_kind {
    my $self=shift;
    my (%kinds)=@_;

    foreach (qw(translate empty section verbatim ignore attributes qualify)) {
        $self->{SGML}->{k}{$_} = $self->{options}{$_} ? $self->{options}{$_}.' ' : '';
        # Remove the default behavior for the tags defined with the
        # options.
        foreach my $k (keys %kinds) {
            foreach my $t (split(" ", $self->{SGML}->{k}{$_})) {
                $kinds{$k} =~ s/\b$t\b//;
            }
        }
    }

    foreach (keys %kinds) {
        die "po4a::sgml: internal error: set_tags_kind called with unrecognized arg $_"
            if ($_ !~ /^(translate|empty|verbatim|ignore|indent|attributes|qualify)$/);

        $self->{SGML}->{k}{$_} .= $kinds{$_};
    }
}

#
# Do the actual work, using the SGMLS package and settings done elsewhere.
#
sub parse_file {
    my ($self,$mastername)=@_;
    my ($prolog);

    # Rewrite the file to:
    #   - protect optional inclusion marker (i.e. "<![ %str [" and "]]>")
    #   - protect entities from expansion (ie "&release;")
    my $origfile="";
    my $i=0;
    while (defined(@{$self->{TT}{doc_in}}) && $i < @{$self->{TT}{doc_in}}) {
        $origfile .= ${$self->{TT}{doc_in}}[$i];
        $i+=2;
    }

    unless ($self->{options}{'force'}) {
        # Detect if we can find the DTD
        my ($tmpfh,$tmpfile)=File::Temp::tempfile("po4a-XXXX",
                                                  SUFFIX => ".sgml",
                                                  DIR    => "/tmp",
                                                  UNLINK => 0);
        print $tmpfh $origfile;
        close $tmpfh
            or die wrap_mod("po4a::sgml",
                            dgettext("po4a", "Can't close tempfile: %s"), $!);
        if (system("nsgmls -p < $tmpfile")) {
            unlink ($tmpfile);
            die wrap_mod("po4a::sgml",
                         dgettext("po4a", "Error while running nsgmls -p.  ".
                                          "Please check if nsgmls and the ".
                                          "DTD are installed."));
        }
        unlink ($tmpfile);
    }
    # Detect the XML pre-prolog
    if ($origfile =~ s/^(\s*<\?xml[^?]*\?>)//) {
        warn wrap_mod("po4a::sgml", dgettext("po4a",
                "Trying to handle a XML document as a SGML one. ".
                "Feel lucky if it works, help us implementing a proper XML backend if it does not."), $mastername)
          unless $self->verbose() <= 0;
        $xmlprolog=$1;
    }
    # Get the prolog
    {
        $prolog=$origfile;
        my $lvl;    # number of '<' seen without matching '>'
        my $pos = 0;  # where in the document (in chars) while detecting prolog boundaries

        unless ($prolog =~ s/^(.*<!DOCTYPE).*$/$1/is) {
            die wrap_mod("po4a::sgml", dgettext("po4a",
                "This file is not a master SGML document (no DOCTYPE). ".
                "It may be a file to be included by another one, in which case it should not be passed to po4a directly. Text from included files is extracted/translated when handling the master file including them."));
        }
        $pos += length($prolog);
        $lvl=1;
        while ($lvl != 0) {
            # Eat comments in the prolog, since there may be some '>' or '<' in them.
            if ($origfile =~ m/^.{$pos}?(<!--.*?-->)/s) {
                print "Found a comment in the prolog: $1\n" if ($debug{'generic'});
                $pos += length($1);
                # take care of the line numbers
                my @a = split(/\n/,$1);
                shift @a; # nb line - 1
                while (defined(shift @a)) {
                    $prolog .= "\n";
                }
                next;
            }
            # Search the closing '>'
            my ($c)=substr($origfile,$pos,1);
            $lvl++ if ($c eq '<');
            $lvl-- if ($c eq '>');
            $prolog = "$prolog$c";
            $pos++;
        }
    }

    # Add the definition of new tags that will be used for the
    # conditionnal inclusions
    if ($origfile =~ /^.*<!DOCTYPE[^[>]*\[/is) {
        $origfile =~ s/^(.*<!DOCTYPE[^[>]*\[)/$1 <!ELEMENT PO4ABEG - o empty> <!ATTLIST PO4ABEG name CDATA #REQUIRED> <!ELEMENT PO4AEND - o empty>/is;
    }

    print STDERR "PROLOG=$prolog\n------------\n" if ($debug{'generic'});

    # Configure the tags for this dtd
    if ($prolog =~ /debiandoc/i) {
        $self->set_tags_kind("translate" => "author version abstract title".
                                            "date copyrightsummary heading p ".
                                            "example tag title",
                             "empty"     => "date ref manref url toc",
                             "verbatim"  => "example",
                             "ignore"    => "package prgn file tt em var ".
                                            "name email footnote po4aend po4abeg ".
                                            "strong ftpsite ftppath qref",
                             "indent"    => "appendix ".
                                            "book ".
                                            "chapt copyright ".
                                            "debiandoc ".
                                            "enumlist ".
                                            "item ".
                                            "list ".
                                            "sect sect1 sect2 sect3 sect4 ".
                                            "tag taglist titlepag toc");

    } elsif ($prolog =~ /docbook/i) {
        $self->set_tags_kind("translate" => "abbrev appendixinfo artheader attribution ".
                                            "biblioentry biblioset ".
                                            "chapterinfo collab collabname confdates confgroup conftitle ".
                                            "date ".
                                            "edition editor entry example ".
                                            "figure ".
                                            "glosssee glossseealso glossterm ".
                                            "holder ".
                                            "member msgaud msglevel msgorig ".
                                            "orgdiv orgname othername ".
                                            "pagenums para phrase pubdate publishername primary ".
                                            "refclass refdescriptor refentrytitle refmiscinfo refname refpurpose releaseinfo remark revnumber revremark ".
                                            "screeninfo seg secondary see seealso segtitle simpara substeps subtitle synopfragmentref synopsis ".
                                            "term tertiary title titleabbrev ".
                                            "contrib epigraph",
                             "empty"     => "audiodata colspec graphic imagedata textdata sbr spanspec videodata xref",
                             "indent"    => "abstract answer appendix article articleinfo audioobject author authorgroup ".
                                            "bibliodiv bibliography blockquote blockinfo book bookinfo bridgehead ".
                                            "callout calloutlist caption caution chapter copyright ".
                                            "dedication docinfo ".
                                            "entry ".
                                            "formalpara ".
                                            "glossary glossdef glossdiv glossentry glosslist group ".
                                            "imageobject important index indexterm informaltable itemizedlist ".
                                            "keyword keywordset ".
                                            "legalnotice listitem lot ".
                                            "mediaobject msg msgentry msginfo msgexplan msgmain msgrel msgsub msgtext ".
                                            "note ".
                                            "objectinfo orderedlist ".
                                            "part partintro preface procedure publisher ".
                                            "qandadiv qandaentry qandaset question ".
                                            "reference refentry refentryinfo refmeta refnamediv refsect1 refsect1info refsect2 refsect2info refsect3 refsect3info refsection refsectioninfo refsynopsisdiv refsynopsisdivinfo revision revdescription row ".
                                            "screenshot sect1 sect1info sect2 sect2info sect3 sect3info sect4 sect4info sect5 sect5info section sectioninfo seglistitem segmentedlist set setindex setinfo shortcut simplelist simplemsgentry simplesect step synopfragment ".
                                            "table tbody textobject tgroup thead tip toc ".
                                            "variablelist varlistentry videoobject ".
                                            "warning",
                             "verbatim"  => "address cmdsynopsis holder literallayout programlisting ".
                                            "refentrytitle refname refpurpose screen term title",
                             "ignore"    => "acronym action affiliation anchor application arg author authorinitials ".
                                            "city citation citerefentry citetitle classname co command computeroutput constant corpauthor country ".
                                            "database po4abeg po4aend ".
                                            "email emphasis envar errorcode errorname errortext errortype exceptionname ".
                                            "filename firstname firstterm footnote footnoteref foreignphrase function ".
                                            "glossterm guibutton guiicon guilabel guimenu guimenuitem guisubmenu ".
                                            "hardware ".
                                            "indexterm informalexample inlineequation inlinegraphic inlinemediaobject interface interfacename isbn ".
                                            "keycap keycode keycombo keysym ".
                                            "link lineannotation literal ".
                                            "manvolnum markup medialabel menuchoice methodname modespec mousebutton ".
                                            "nonterminal ".
                                            "olink ooclass ooexception oointerface option optional othercredit ".
                                            "parameter personname phrase productname productnumber prompt property pubsnumber ".
                                            "quote ".
                                            "remark replaceable returnvalue revhistory ".
                                            "sgmltag sidebar structfield structname subscript superscript surname symbol systemitem ".
                                            "token trademark type ".
                                            "ulink userinput ".
                                            "varname volumenum ".
                                            "wordasword ".
                                            "xref ".
                                            "year",
                             "attributes" =>"<(article|book)>lang");

    } else {
        if ($self->{options}{'force'}) {
            warn wrap_mod("po4a::sgml", dgettext("po4a", "DTD of this file is unknown, but proceeding as requested."));
            $self->set_tags_kind();
        } else {
            die wrap_mod("po4a::sgml", dgettext("po4a",
                "DTD of this file is unknown. (supported: DebianDoc, DocBook). The prolog follows:")."\n$prolog");
        }
    }

    # Hash of the file entities that won't be included
    my %ignored_inclusion = ();
    foreach (split / /,$self->{options}{'ignore-inclusion'}) {
        $ignored_inclusion{$_} = 1;
    }

    # Prepare the reference indirection stuff
    my @refs;
    my $length = ($origfile =~ tr/\n/\n/);
    print "XX Prepare reference indirection stuff\n" if $debug{'refs'};
    for (my $i=1; $i<=$length; $i++) {
        push @refs,"$mastername:$i";
        print "$mastername:$i\n" if $debug{'refs'};
    }

    # protect the conditional inclusions in the file
    $origfile =~ s/<!\[\s*IGNORE\s*\[/{PO4A-beg-IGNORE}/g; # cond. incl. starts
    $origfile =~ s/<!\[\s*CDATA\s*\[/{PO4A-beg-CDATA}/g; # cond. incl. starts
    $origfile =~ s/<!\[\s*RCDATA\s*\[/{PO4A-beg-RCDATA}/g; # cond. incl. starts
    $origfile =~ s/<!\[\s*([^\[\s]+)\s*\[/<po4abeg name="$1">/g; # cond. incl. starts
    $origfile =~ s/\]\]>/<po4aend>/g;                # cond. incl. end

    # Remove <![ IGNORE [ sections
    # FIXME: we don't support included PO4A-beg-
    my $tmp1 = $origfile;
    while ($tmp1 =~ m/^(.*?)({PO4A-beg-\s*IGNORE\s*}(?:.+?)<po4aend>)(.*)$/s)
    {
        my ($begin,$ignored,$end) = ($1, $2, $3);
        my @begin   = split(/\n/, $begin);
        my @ignored = split(/\n/, $ignored);
        my $pre = scalar @begin;
        my $len = (scalar @ignored) -1;
        $pre++ if ($begin =~ /\n$/s);
        $len++ if ($end   =~ /^\n/s);
        # remove the references of the ignored lines
        splice @refs, $pre+1, $len-1;
        # remove the lines
        $tmp1 = $begin.$end;
    }
    $origfile = $tmp1;
    # The <, >, and & in a CDATA must be escaped because they do not
    # correspond to tags or entities delimiters.
    $tmp1 = $origfile;
    $origfile = "";
    while ($tmp1 =~ m/^(.*?{PO4A-beg-\s*(?:CDATA|RCDATA)\s*})(.+?)(<po4aend>.*)$/s) {
        my ($begin, $tmp) = ($1, $2);
        $tmp1 = $3;
        $tmp =~ s/</{PO4A-lt}/gs;
        $tmp =~ s/>/{PO4A-gt}/gs;
        $tmp =~ s/&/{PO4A-amp}/gs;
        $origfile .= $begin.$tmp;
    }
    $origfile .= $tmp1;

    # Deal with the %entities; in the prolog. God damn it, this code is gross!
    # Try hard not to change the number of lines to not fuck up the references
    my %prologentincl;
    my $moretodo=1;
    PROLOGENTITY: while ($moretodo) { # non trivial loop to deal with recursive inclusion
        $moretodo = 0;
        # Unprotect not yet defined inclusions
        $prolog =~ s/{PO4A-percent}/%/sg;
        print STDERR "prolog=>>>>$prolog<<<<\n"
              if ($debug{'entities'});
        while ($prolog =~ /(.*?)<!ENTITY\s*%\s*(\S*)\s+SYSTEM\s*"([^>"]*)"\s*>(.*)$/is) {  #})"{ (Stupid editor)
            print STDERR "Seen the definition entity of prolog inclusion '$2' (=$3)\n"
              if ($debug{'entities'});
            # Preload the content of the entity.
            my $key = $2;
            my $filename=$3;
            my $origfilename = $filename;
            my ($begin, $end) = ($1, $4);
            if ($filename !~ m%^/% && $mastername =~ m%/%) {
                my $dir=$mastername;
                $dir =~ s%/[^/]*$%%;
                $filename="$dir/$filename";
                # origfile also needs to be fixed otherwise nsgmls won't
                # find the file.
                $origfile =~ s/(<!ENTITY\s*%\s*\Q$key\E\s+SYSTEM\s*")\Q$origfilename\E("\s*>)/$1$filename$2/gsi;
            }
            if (defined $ignored_inclusion{$key} or !-e $filename) {
                # We won't expand this entity.
                # And we avoid nsgmls to do so.
                $prolog = "$begin<!--{PO4A-ent-beg-$key}$filename".
                          "{PO4A-ent-end}-->$end";
            } else {
            $prolog = $begin.$end;
            (-e $filename && open IN,"<$filename")  ||
              die wrap_mod("po4a::sgml",
                           dgettext("po4a",
                               "Can't open %s (content of entity %s%s;): %s"),
                           $filename, '%', $key, $!);
            local $/ = undef;
            $prologentincl{$key} = <IN>;
            close IN;
            print STDERR "Content of \%$key; is $filename (".
                         ($prologentincl{$key} =~ tr/\n/\n/).
                         " lines long)\n"
              if ($debug{'entities'});
            print STDERR "content: ".$prologentincl{$key}."\n"
              if ($debug{'entities'});
            $moretodo = 1;
            next PROLOGENTITY;
            }
        }
        while ($prolog =~ /(.*?)<!ENTITY\s*%\s*(\S*)\s*"([^>"]*)"\s*>(.*)$/is) {  #})"{ (Stupid editor)
            print STDERR "Seen the definition entity of prolog definition '$2' (=$3)\n"
              if ($debug{'entities'});
            # Preload the content of the entity.
            my $key = $2;
            $prolog = $1.$4;
            $prologentincl{$key} = $3;
            print STDERR "content: ".$prologentincl{$key}."\n"
              if ($debug{'entities'});
            $moretodo = 1;
            next PROLOGENTITY;
        }
        while ($prolog =~ /^(.*?)%([^;\s]*);(.*)$/s) {
            my ($pre,$ent,$post) = ($1,$2,$3);
            # Yeah, right, the content of the entity can be defined in a not yet loaded entity
            # It's easy to build a weird case where all that shit collapses poorly. But why the
            # hell are you using those strange constructs in your document, damn it?
            print STDERR "Seen prolog inclusion $ent\n" if ($debug{'entities'});
            if (defined ($prologentincl{$ent})) {
                $prolog = $pre.$prologentincl{$ent}.$post;
                print STDERR "Change \%$ent; to its content in the prolog\n"
                  if $debug{'entities'};
                $moretodo = 1;
            } else {
                # AAAARGH stupid document using %bla; and having then defined in another inclusion!
                # Protect it for this pass, and unprotect it on next one
                print STDERR "entity $ent not defined yet ?!\n"
                  if $debug{'entities'};
                $prolog = "$pre".'{PO4A-percent}'."$ent;$post";
            }
        }
    }
    $prolog =~ s/<!--{PO4A-ent-beg-(.*?)}(.*?){PO4A-ent-end}-->/<!ENTITY % $1 SYSTEM "$2">/g;
    # Unprotect undefined inclusions, and die of them
    $prolog =~ s/{PO4A-percent}/%/sg;
    if ($prolog =~ /%([^;\s]*);/) {
        die wrap_mod("po4a::sgml",
                     dgettext("po4a",
                              "unrecognized prolog inclusion entity: %%%s;"),
                     $1)
           unless ($ignored_inclusion{$1});
    }
    # Protect &entities; (all but the ones asking for a file inclusion)
    #   search the file inclusion entities
    my %entincl;
    my $searchprolog=$prolog;
    while ($searchprolog =~ /(.*?)<!ENTITY\s+(\S*)\s+SYSTEM\s*"([^>"]*)"\s*>(.*)$/is) {  #})"{
        print STDERR "Seen the entity of inclusion $2 (=$3)\n"
          if ($debug{'entities'});
        my $key = $2;
        my $filename = $3;
        my $origfilename = $filename;
        $searchprolog = $4;
        if ($filename !~ m%^/% && $mastername =~ m%/%) {
            my $dir=$mastername;
            $dir =~ s%/[^/]*$%%;
            $filename="$dir/$filename";
            # origfile also needs to be fixed otherwise nsgmls won't find
            # the file.
            $origfile =~ s/(<!ENTITY\s+$key\s+SYSTEM\s*")\Q$origfilename\E("\s*>)/$1$filename$2/gsi;
        }
        if ((not defined $ignored_inclusion{$2}) and (-e $filename)) {
            $entincl{$key}{'filename'}=$filename;
            # Preload the content of the entity
            (-e $filename && open IN,"<$filename")  ||
                die wrap_mod("po4a::sgml",
                       dgettext("po4a",
                                "Can't open %s (content of entity %s%s;): %s"),
                                $filename, '&', $key, $!);
            local $/ = undef;
            $entincl{$key}{'content'} = <IN>;
            close IN;
            $entincl{$key}{'length'} = ($entincl{$key}{'content'} =~ tr/\n/\n/);
            print STDERR "read $filename (content of \&$key;, $entincl{$key}{'length'} lines long)\n"
                if ($debug{'entities'});
        }
    }

    #   Change the entities including files in the document
    my $dosubstitution = 1;
    while ($dosubstitution) {
        $dosubstitution = 0;
        foreach my $key (keys %entincl) {
            # The external entity can be referenced as &key; or &key
            # In the second case, we must differentiate &key and &key2
            while ($origfile =~/^(.*?)&$key(;.*$|[^-_:.A-Za-z0-9].*$|$)/s) {
                # Since we will include a new file, we
                # must do a new round of substitutions.
                $dosubstitution = 1;
                my ($begin,$end)=($1,$2);
                $end = "" unless (defined $end);
                $end =~ s/^;//s;

                if ($begin =~ m/.*<!--(.*?)$/s and $1 !~ m/-->/s) {
                    # This entity is commented. Just remove it.
                    $origfile = $begin.$end;
                    next;
                }

                # add the refs
                my $len  = $entincl{$key}{'length'}; # number added by the inclusion
                my $pre  = ($begin =~ tr/\n/\n/); # number of \n
                my $post = ($end =~ tr/\n/\n/);
                print "XX Add a ref. pre=$pre; len=$len; post=$post\n"
                    if $debug{'refs'};
                # Keep a reference of inclusion position in main file
                my $main = $refs[$pre];

                # Remove the references for the lines after the inclusion
                # point.
                my @endrefs = splice @refs, $pre+1;

                # Add the references of the added lines
                my $i;
                for ($i=0; $i<$len; $i++) {
                    $refs[$i+$pre] = "$main $entincl{$key}{'filename'}:".($i+1);
                }

                if ($begin !~ m/\n[ \t]*$/s) {
                    if ($entincl{$key}{'content'} =~ m/^[ \t]*\n/s) {
                        # There is nothing in the first line of the
                        # included file, and something on the line before
                        # the inclusion The line reference will be more
                        # informative like this:
                        $refs[$pre] = $main;
                    }
                }
                if ($end !~ s/^[ \t]*\n//s) {
                    if ($entincl{$key}{'content'} =~ m/\n[ \t]*$/s) {
                        # There is something on the line after the
                        # inclusion, and there is an end of line at the
                        # end of the included file. We must add the line
                        # reference of the remainder on the line:
                        push @refs, $main;
                    }
                }
                # Append the references removed earlier (lines after the
                # inclusion point).
                push @refs, @endrefs;

                # Do the substitution
                $origfile = "$begin".$entincl{$key}{'content'}."$end";
                print STDERR "substitute $key\n" if ($debug{'entities'});
            }
        }
    }
    $origfile=~s/\G(.*?)&([A-Za-z_:][-_:.A-Za-z0-9]*|#[0-9]+|#x[0-9a-fA-F]+)\b/$1\{PO4A-amp\}$2/gs;
    if (defined($xmlprolog) && length($xmlprolog)) {
        $origfile=~s/\/>/\{PO4A-close\}>/gs;
    }

    if ($debug{'refs'}) {
        print "XX Resulting shifts\n";
        for (my $i=0; $i<scalar @refs; $i++) {
            print "$mastername:".($i+1)." -> $refs[$i]\n";
        }
    }

    my ($tmpfh,$tmpfile)=File::Temp::tempfile("po4a-XXXX",
                                              SUFFIX => ".sgml",
                                              DIR    => "/tmp",
                                              UNLINK => 0);
    print $tmpfh $origfile;
    close $tmpfh or die wrap_mod("po4a::sgml", dgettext("po4a", "Can't close tempfile: %s"), $!);

    my $cmd="nsgmls -l -E 0 -wno-valid < $tmpfile".
            ($debug{'nsgmls'}?"":" 2>/dev/null")." |";
    print STDERR "CMD=$cmd\n" if ($debug{'generic'} or $debug{'nsgmls'});

    open (IN,$cmd) || die wrap_mod("po4a::sgml", dgettext("po4a", "Can't run nsgmls: %s"), $!);

    # The kind of tags
    my (%translate,%empty,%verbatim,%indent,%exist,%attribute,%qualify);
    foreach (split(/ /, ($self->{SGML}->{k}{'translate'}||'') )) {
        $translate{uc $_} = 1;
        $indent{uc $_} = 1;
        $exist{uc $_} = 1;
    }
    foreach (split(/ /, ($self->{SGML}->{k}{'empty'}||'') )) {
        $empty{uc $_} = 1;
        $exist{uc $_} = 1;
    }
    foreach (split(/ /, ($self->{SGML}->{k}{'verbatim'}||'') )) {
        $translate{uc $_} = 1;
        $verbatim{uc $_} = 1;
        $exist{uc $_} = 1;
    }
    foreach (split(/ /, ($self->{SGML}->{k}{'indent'}||'') )) {
        $translate{uc $_} = 1;
        $indent{uc $_} = 1;
        $exist{uc $_} = 1;
    }
    foreach (split(/ /, ($self->{SGML}->{k}{'ignore'}) || '')) {
        $exist{uc $_} = 1;
    }
    foreach (split(/ /, ($self->{SGML}->{k}{'attributes'} || ''))) {
        my ($attr, $tags);
        if (m/(^.*>)(\w+)/) {
            $attr=uc $2;
            $tags=$1;
        } else {
            $attr=uc $_;
            $tags=".*";
        }
        if (exists $attribute{$attr}) {
            $attribute{$attr}.="|$tags";
        } else {
            $attribute{$attr} = $tags;
        }
    }
    foreach (split(/ /, ($self->{SGML}->{k}{'qualify'}) || '')) {
        $qualify{uc $_} = 1;
        $attribute{uc $_} = '.*' unless exists $attribute{uc $_};
    }


    # What to do before parsing

    # push the XML prolog if existing
    $self->pushline($xmlprolog."\n") if (defined($xmlprolog) && length($xmlprolog));

    # Put the prolog into the file, allowing for entity definition translation
    #  <!ENTITY myentity "definition_of_my_entity">
    # and push("<!ENTITY myentity \"".$self->translate("definition_of_my_entity")
    if ($prolog =~ m/(.*?\[)(.*)(\]>)/s) {
        warn "Pre=~~$1~~;Post=~~$3~~\n" if ($debug{'entities'});
        $self->pushline($1."\n") if (length($1));
        $prolog=$2;
        my ($post) = $3;
        while ($prolog =~ m/^(.*?)<!ENTITY\s+(\S*)\s+"([^"]*)"\s*>(.*)$/is) { #" ){
           $self->pushline($1) if length($1);
           $self->pushline("<!ENTITY $2 \"".$self->translate($3,"","definition of entity \&$2;")."\">");
           warn "Seen text entity $2\n" if ($debug{'entities'});
           $prolog = $4;
        }
        $prolog .= $post;
        $self->pushline($prolog."\n") if (length($prolog));
    } else {
        warn "No entity declaration detected in ~~$prolog~~...\n" if ($debug{'entities'});
        $self->pushline($prolog) if length($prolog);
    }

    # The parse object.
    # Damn SGMLS. It makes me do crude things.
    no strict "subs";
    my $parse= new SGMLS(IN);
    use strict;

    # Some values for the parsing
    my @open=(); # opened translation container tags
    my $verb=0;  # can we wrap or not
    my $verb_last_ref;
    my $seenfootnote=0;
    my $indent=0; # indent level
    my $lastchar = ''; #
    my $buffer= ""; # what we will soon handle

    # Keep a reference to the last line indicated by nsgmls
    my $line=0;
    # Unfortunately, nsgmls do not mention all the line changes.  We have
    # to keep track of the number of lines seen in the "record ends".
    my $adds=0;
    # If the last line received contains only spaces, do not take it into
    # account for the line reference of the paragraph.
    my $empty_last_cdata=0;
    # run the appropriate handler for each event
    EVENT: while (my $event = $parse->next_event) {
        # get the line reference to build po entries
        if ($line != $parse->line) {
            # nsgmls informs us of that the line changed. Reset $adds and
            # $empty_last_cdata
            $adds = 0;
            $empty_last_cdata = 0;
            $line = $parse->line;
        }
        my $ref=$refs[$parse->line-1 + $adds - $empty_last_cdata];
        # In verbatim mode, keep the current line reference.
        if ($verb) {
            $ref=$refs[$parse->line-1];
        }
        my $type;

        if ($event->type eq 'start_element') {
            die wrap_ref_mod($ref, "po4a::sgml",
                             dgettext("po4a", "Unknown tag %s"),
                             $event->data->name)
                unless $exist{$event->data->name};

            $lastchar = ">";

            # Which tag did we see?
            my $tag='';
            $tag .= '<'.lc($event->data->name());
            while (my ($attr, $val) = each %{$event->data->attributes()}) {
                my $value = $val->value();
#                if ($val->type() eq 'IMPLIED') {
#                    $tag .= ' '.lc($attr).'="'.lc($attr).'"';
#                } els
                if ($val->type() eq 'CDATA' ||
                    $val->type() eq 'IMPLIED') {
                    if (defined $value && length($value)) {
                        my $lattr=lc $attr;
                        my $uattr=uc $attr;
                        if (exists $attribute{$uattr}) {
                            my $context="";
                            foreach my $o (@open) {
                                next if (!defined $o or $o =~ m%^</%);
                                $o =~ s/ .*/>/;
                                $context.=$o;
                            }
                            $context=join("", $context,
                                          "<", lc($event->data->name()), ">");
                            if ($context =~ /^($attribute{$uattr})$/) {
                                if ($qualify{$uattr}) {
                                    my $translated = $self->translate("$lattr=$value", $ref, "attribute $context$lattr");
                                    if ($translated =~ s/^$lattr=//) {
                                        $value=$translated;
                                    } else {
                                        die wrap_mod("po4a::sgml", dgettext("po4a", "bad translation '%s' for '%s' in '%s'"), $translated, $context.$lattr, $ref);
                                    }
                                } else {
                                    $value = $self->translate($value, $ref, "attribute $context$lattr");
                                }
                            }
                        }
                        if ($value =~ m/\"/) {
                            $value = "'".$value."'";
                        } else {
                            $value = '"'.$value.'"';
                        }
                        $tag .= " $lattr=$value";
                    }
                } elsif ($val->type() eq 'NOTATION') {
                } else {
                    $tag .= ' '.lc($attr).'="'.lc($value).'"'
                        if (defined $value && length($value));
                }
            }
            $tag .= '>';


            # debug
            print STDERR "Seen $tag, open level=".(scalar @open).", verb=$verb\n"
                if ($debug{'tag'});

            if ($event->data->name() eq 'FOOTNOTE') {
                # we want to put the <para> inside the <footnote> in the same msgid
                $seenfootnote = 1;
            }

            if ($seenfootnote) {
                $buffer .= $tag;
                next EVENT;
            }
            if ($translate{$event->data->name()}) {
                # Build the type
                if (scalar @open > 0) {
                    $type=$open[$#open] . $tag;
                } else {
                    $type=$tag;
                }

                # do the job
                if (@open > 0) {
                    $self->end_paragraph($buffer,$ref,$type,$verb,$indent,
                                         @open);
                } else {
                    $self->pushline($buffer) if $buffer;
                }
                $buffer="";
                push @open,$tag;
            } elsif ($indent{$event->data->name()}) {
                die wrap_ref_mod($ref, "po4a::sgml", dgettext("po4a",
                    "Closing tag for a translation container missing before %s"),$tag)
                    if (scalar @open);
            }

            if ($verbatim{$event->data->name()}) {
                $verb++;
                # Keep a reference to the line that openned the verbatim
                # section. This is needed to check if its data starts on
                # the same line.
                $verb_last_ref = $ref;
            }
            if ($verb) {
                # Tag in a verbatim section. Check if it appeared at
                # the same line than the previous data. If not, it
                # means that an end of line must be added to the
                # buffer.
                if ($ref ne $verb_last_ref) {
                    # FIXME: Does it work if $verb > 1
                    $buffer .= "\n";
                    $verb_last_ref = $ref;
                }
            }

            if ($indent{$event->data->name()}) {
                # push the indenting space only if not in verb before that tag
                # push trailing "\n" only if not in verbose afterward
                $self->pushline( ($verb>1?"": (" " x $indent)).$tag.($verb?"":"\n"));
                $indent ++ unless $empty{$event->data->name()} ;
            }  else {
                $tag =~ s/<po4abeg name="([^"]+)">/<![ $1 [/; #"; Stupid emacs
                $tag =~ s/<po4aend>/]]>/;
                $buffer .= $tag;
            }
        } # end of type eq 'start_element'

        elsif ($event->type eq 'end_element') {
            my $tag = ($empty{$event->data->name()}
                           ?
                       ''
                           :
                       '</'.lc($event->data->name()).'>');

            if ($verb) {
                # Tag in a verbatim section. Check if it appeared at
                # the same line than the previous data. If not, it
                # means that an end of line must be added to the
                # buffer.
                if ($ref ne $verb_last_ref) {
                    # FIXME: Does it work if $verb > 1
                    $buffer .= "\n";
                    $verb_last_ref = $ref;
                }
            }
            print STDERR "Seen $tag, level=".(scalar @open).", verb=$verb\n"
                if ($debug{'tag'});

            $lastchar = ">";

            if ($event->data->name() eq 'FOOTNOTE') {
                # we want to put the <para> inside the <footnote> in the same msgid
                $seenfootnote = 0;
            }

            if ($seenfootnote) {
                $buffer .= $tag;
                next EVENT;
            }
            if ($translate{$event->data->name()}) {
                $type = $open[$#open] . $tag;
                $self->end_paragraph($buffer,$ref,$type,$verb,$indent,@open);
                $buffer = "";
                pop @open;
                if (@open > 0) {
                    pop @open;
                    push @open,$tag;
                }
            } elsif ($indent{$event->data->name()}) {
                die wrap_ref_mod($ref, "po4a::sgml", dgettext("po4a",
           "Closing tag for a translation container missing before %s"), $tag)
                    if (scalar @open);
            }

            unless ($event->data->name() =~ m/^(PO4ABEG|PO4AEND)$/si) {
                if ($indent{$event->data->name()}) {
                    $indent -- ;
                    # add indenting space only when not in verbatim
                    # add the tailing \n only if out of verbatim after that tag
                    $self->pushline(($verb?"":(" " x $indent)).$tag.($verb>1?"":"\n"));
                }  else {
                    $buffer .= $tag;
                }
                $verb-- if $verbatim{$event->data->name()};
            }
        } # end of type eq 'end_element'

        elsif ($event->type eq 'cdata') {
            my $cdata = $event->data;
            $empty_last_cdata=($cdata =~ m/^\s*$/);
            $cdata =~ s/{PO4A-lt}/</g;
            $cdata =~ s/{PO4A-gt}/>/g;
            $cdata =~ s/{PO4A-amp}/&/g;
            $cdata =~ s/{PO4A-end}/\]\]>/g;
            $cdata =~ s/{PO4A-beg-([^\}]+)}/<!\[$1\[/g;
            if ($verb) {
                # Check if this line of data appear on the same line
                # than the previous tag. If not, append an end of line
                # to the buffer.
                if ($ref ne $verb_last_ref) {
                    $buffer .= "\n";
                    $verb_last_ref = $ref;
                }
            } else {
                $cdata =~ s/\\t/ /g;
                $cdata =~ s/\s+/ /g;
                $cdata =~ s/^\s//s if $lastchar eq ' ';
            }
            $lastchar = substr($cdata, -1, 1);
            $buffer .= $cdata;
            if (defined($xmlprolog) && length($xmlprolog)) {
                $buffer =~ s/>PO4A-close\}>/\/>/sg;
                $buffer =~ s/PO4A-close\}>//sg; # This should not be necessary
            }
        } # end of type eq 'cdata'

        elsif ($event->type eq 'sdata') {
            my $sdata = $event->data;
            $sdata =~ s/^\[//;
            $sdata =~ s/\s*\]$//;
            $lastchar = substr($sdata, -1, 1);
            $buffer .= '&'.$sdata.';';
        } # end of type eq 'sdata'

        elsif ($event->type eq 're') {
            # End of record, the line reference shall be incremented.
            $adds +=1;
            if ($verb) {
                # Check if this line of data appear on the same line
                # than the previous tag. If not, append an end of line
                # to the buffer.
                if ($ref ne $verb_last_ref) {
                    $buffer .= "\n";
                    $verb_last_ref = $ref;
                }
                $buffer .= "\n";
            } elsif ($lastchar ne ' ') {
                $buffer .= " ";
            }
            $lastchar = ' ';
        } #end of type eq 're'

        elsif ($event->type eq 'conforming') {

        }

        elsif ($event->type eq 'pi') {
            my $pi = $event->data;
            $buffer .= "<?$pi>";
        }

        else {
            die wrap_ref_mod($refs[$parse->line], "po4a::sgml",
                             dgettext("po4a","Unknown SGML event type: %s"),
                             $event->type);
        }
    }

    # What to do after parsing
    $self->pushline($buffer);
    close(IN);
    warn wrap_mod("po4a::sgml",
                  dgettext("po4a","Warning: nsgmls produced some errors.  ".
                  "This is usually caused by po4a, which modifies the input ".
                  "and restores it afterwards, causing the input of nsgmls ".
                  "to be invalid.  This is usually safe, but you may wish ".
                  "to verify the generated document with nsgmls -wno-valid.  ".
                  "Continuing..."))
        if ($? != 0 and $self->verbose() > 0);
    unlink ($tmpfile) unless ($debug{'refs'} or $debug{'nsgmls'});
}

sub end_paragraph {
    my ($self, $para,$ref, $type,$verb,$indent)=
        (shift,shift,shift,shift,shift,shift);
    my (@open)=@_;
    die "Internal error: no paragraph to end here!!"
        unless scalar @open;

    return unless defined($para) && length($para);

    if (($para =~ m/^\s*$/s) and (not $verb)) {
        # In non-verbatim environments, a paragraph with only spaces is
        # like an empty paragraph
        return;
    }

    # unprotect &entities;
    $para =~ s/{PO4A-amp}/&/g;
    # remove the name"\|\|" nsgmls added as attributes
    $para =~ s/ name=\"\\\|\\\|\"//g;
    $para =~ s/ moreinfo=\"none\"//g;

    # Extract the leading and trailing spaces. They will be restored only
    # in verbatim environments.
    my ($leading_spaces, $trailing_spaces) = ("", "");
    if ($verb) {
        # In the verbatim mode, we can ignore empty lines, but not the
        # leading spaces or tabulations. Otherwise, the PO will look
        # weird.
        if ($para =~ m/^(\s*\n)(.*?)(\s*)$/s) {
            $leading_spaces = $1;
            $para = $2;
            $trailing_spaces = $3;
        }
    } else {
        if ($para =~ m/^(\s*)(.*?)(\s*)$/s) {
            $leading_spaces = $1;
            $para = $2;
            $trailing_spaces = $3;
        }
    }

    $para = $self->translate($para,$ref,$type,
                             'wrap' => ! $verb,
                             'wrapcol' => (75 - $indent));

    if ($verb) {
        $para = $leading_spaces.$para.$trailing_spaces;
    } else {
        $para =~ s/^\s+//s;
        my $toadd=" " x ($indent+1);
        $para =~ s/^/$toadd/mg;
        $para .= "\n";
    }

    $self->pushline( $para );
}

1;

=head1 AUTHORS

This module is an adapted version of sgmlspl (SGML postprocessor for the
SGMLS and NSGMLS parsers) which was:

 Copyright (c) 1995 by David Megginson <dmeggins@aix1.uottawa.ca>

The adaptation for po4a was done by:

 Denis Barbier <barbier@linuxfr.org>
 Martin Quinson (mquinson#debian.org)

=head1 COPYRIGHT AND LICENSE

 Copyright (c) 1995 by David Megginson <dmeggins@aix1.uottawa.ca>
 Copyright 2002, 2003, 2004, 2005 by SPI, inc.

This program is free software; you may redistribute it and/or modify it
under the terms of GPL (see the COPYING file).
