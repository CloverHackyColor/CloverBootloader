#!/usr/bin/perl -w

=encoding UTF-8

=head1 NAME

Locale::Po4a::Man - convert manual pages from/to PO files

=head1 DESCRIPTION

The po4a (PO for anything) project goal is to ease translations (and more
interestingly, the maintenance of translations) using gettext tools on
areas where they were not expected like documentation.

Locale::Po4a::Man is a module to help the translation of documentation in
the nroff format (the language of manual pages) into other [human]
languages.

=head1 TRANSLATING WITH PO4A::MAN

This module tries pretty hard to make translator's life easier. For that,
the text presented to translators isn't a verbatim copy of the text found
in the man page. Indeed, the cruder parts of the nroff format are hidden, so
that translators can't mess up with them.

=head2 Text wrapping

Unindented paragraphs are automatically rewrapped for the translator.  This
can lead to some minor difference in the generated output, since the
rewrapping rules used by groff aren't very clear. For example, two spaces
after a parenthesis are sometimes preserved.

Anyway, the difference will only be about the position of the extra spaces
in wrapped paragraph, and I think it's worth.

=head2 Font specification

The first change is about font change specifications.  In nroff, there are
several ways to specify if a given word should be written in small, bold or
italics. In the text to translate, there is only one way, borrowed from the
POD (Perl online documentation) format:

=over

=item IE<lt>textE<gt> -- italic text

equivalent to \fItext\fP or ".I text"

=item BE<lt>textE<gt> -- bold text

equivalent to \fBtext\fP or ".B text"

=item RE<lt>textE<gt> -- roman text

equivalent to \fRtext\fP

=item CWE<lt>textE<gt> -- constant width text

equivalent to \f(CWtext\fP or ".CW text"

=back

Remark: The CW face is not available for all groff devices. It is not
recommended to use it. It is provided for your convenience.

=head2 Automatic characters transliteration

Po4a automatically transliterate some characters to ease the translation
or the review of the translation.
Here is the list of the transliterations:

=over

=item hyphens

Hyphens (-) and minus signs (\-) in man pages are all transliterated
as simple dashes (-) in the PO file.  Then all dash are transliterated into
roff minus signs (\-) when the translation is inserted into the output
document.

Translators can force an hyphen by using the roff glyph '\[hy]' in their
translations.

=item non-breaking spaces

Translators can use non-breaking spaces in their translations.  These
non-breaking spaces (0xA0 in latin1) will be transliterated into a roff
non-breaking space ('\ ').

=item quotes transliterations

`` and '' are respectively tranliterated into \*(lq and \*(rq.

To avoid these transliterations, translators can insert a zero width roff
character (i.e., using `\&` or '\&' respectively).

=back

=head2 Putting 'E<lt>' and 'E<gt>' in translations

Since these chars are used to delimit parts under font modification, you
can't use them verbatim. Use EE<lt>ltE<gt> and EE<lt>gtE<gt> instead (as in
POD, one more time).

=head1 OPTIONS ACCEPTED BY THIS MODULE

These are this module's particular options:

=over

=item B<debug>

Activate debugging for some internal mechanisms of this module.
Use the source to see which parts can be debugged.

=item B<verbose>

Increase verbosity.

=item B<groff_code>

This option permits to change the behavior of the module when it encounter
a .de, .ie or .if section. It can take the following values:

=over

=item I<fail>

This is the default value.
The module will fail when a .de, .ie or .if section is encountered.

=item I<verbatim>

Indicates that the .de, .ie or .if sections must be copied as is
from the original to the translated document.

=item I<translate>

Indicates that the .de, .ie or .if sections will be proposed for the
translation.
You should only use this option if a translatable string is
contained in one of these section. Otherwise, I<verbatim>
should be preferred.

=back

=item B<generated>

This option specifies that the file was generated, and that po4a should not
try to detect if the man pages was generated from another format.
This permits to use po4a on generated man pages.
This option does not take any argument.

=item B<mdoc>

This option is only useful for mdoc pages.

It selects a stricter support of the mdoc format by telling po4a not to
translate the 'NAME' section.
mdoc pages whose 'NAME' section is translated won't generate any header or
footer.

According to the groff_mdoc page, the NAME, SYNOPSIS and DESCRIPTION
sections are mandatory.
There are no known issues with translated SYNOPSIS or DESCRIPTION section,
but you can also specify these sections this way:
 -o mdoc=NAME,SYNOPSIS,DESCRIPTION

This mdoc issue can also be solved with an addendum like this one:
 PO4A-HEADER:mode=before;position=^.Dd
 .TH DOCUMENT_TITLE 1 "Month day, year" OS "Section Name"


=back

The following options permit to specify the behavior of a new macro
(defined with a .de request), or of a macro not supported by po4a.
They take as argument a comma-separated list of macros.
For example:

 -o noarg=FO,OB,AR -o translate_joined=BA,ZQ,UX

Note: if a macro is not supported by po4a and if you consider that it is a
standard roff macro, you should submit it to the po4a development team.

=over

=item B<untranslated>

B<untranslated> indicates that this macro (at its arguments) don't have to
be translated.

=item B<noarg>

B<noarg> is like B<untranslated>, except that po4a will verify that no
argument is added to this macro.

=item B<translate_joined>

B<translate_joined> indicates that po4a must propose to translate the
arguments of the macro.

=item B<translate_each>

With B<translate_each>, the arguments will also be proposed for the
translation, except that each one will be translated separately.

=item B<no_wrap>

This option takes as argument a list of comma-separated couples
I<begin>:I<end>, where I<begin> and I<end> are commands that delimit
the begin and end of a section that should not be rewrapped.

Note: no test is done to ensure that an I<end> command matches its
I<begin> command; any ending command stop the no_wrap mode.
If you have a I<begin> (respectively I<end>) macro that has no I<end>
(respectively I<begin>), you can specify an existing I<end> (like fi) or
I<begin> (like nf) as a counterpart.
These macros (and their arguments) wont be translated.

=item B<inline>

This option specifies a list of comma-separated macros that must
not split the current paragraph. The string to translate will then contain
I<foo E<lt>.bar baz quxE<gt> quux>, where I<bar> is the command that
should be inlined, and I<baz qux> its arguments.

=item B<unknown_macros>

This option indicates how po4a should behave when an unknown macro is found.
By default, po4a fails with a warning.
It can take the following values: B<failed> (the default value),
B<untranslated>, B<noarg>, B<translate_joined>, or B<translate_each> (see above
for an explanation of these values).

=back

=head1 AUTHORING MAN PAGES COMPLIANT WITH PO4A::MAN

This module is still very limited, and will always be, because it's not a
real nroff interpreter. It would be possible to do a real nroff
interpreter, to allow authors to use all the existing macros, or even to
define new ones in their pages, but we didn't want to. It would be too
difficult, and we thought it wasn't necessary. We do think that if
manpages' authors want to see their productions translated, they may have to
adapt to ease the work of translators.

So, the man parser implemented in po4a have some known limitations we are
not really inclined to correct, and which will constitute some pitfalls
you'll have to avoid if you want to see translators taking care of your
documentation.

=head2 Don't program in nroff

nroff is a complete programming language, with macro definition,
conditionals and so on. Since this parser isn't a fully featured nroff
interpreter, it will fail on pages using these facilities (There are about
200 such pages on my box).

=head2 Use the plain macro set

There are still some macros which are not supported by po4a::man. This is
only because I failed to find any documentation about them. Here is the
list of unsupported macros used on my box. Note that this list isn't
exhaustive since the program fails on the first encountered unsupported
macro. If you have any information about some of these macros, I'll
happily add support for them. Because of these macros, about 250 pages on
my box are inaccessible to po4a::man.

 ..               ."              .AT             .b              .bank
 .BE              ..br            .Bu             .BUGS           .BY
 .ce              .dbmmanage      .do                             .En
 .EP              .EX             .Fi             .hw             .i
 .Id              .l              .LO             .mf
 .N               .na             .NF             .nh             .nl
 .Nm              .ns             .NXR            .OPTIONS        .PB
 .pp              .PR             .PRE            .PU             .REq
 .RH              .rn             .S<             .sh             .SI
 .splitfont       .Sx             .T              .TF             .The
 .TT              .UC             .ul             .Vb             .zZ

=head2 Hiding text from po4a

Sometimes, the author knows that some parts are not translatable, and
should not be extracted by po4a. For example, an option may accept an
I<other> argument, and I<other> may also appear as the last item of a
list. In the first case, I<other> should be not be translatable. And in
the second case, I<other> should be translated.

In such case, the author can avoid po4a to extract some strings, using
some special groff constructs:

 .if !'po4a'hide' .B other

(this will require the B<-o groff_code=verbatim> option)

A new macro can also be defined to automate this:
 .de IR_untranslated
 .    IR \\$@
 ..

 .IR_untranslated \-q ", " \-\-quiet

(this will require the options B<-o groff_code=verbatim> and
B<-o untranslated=IR_untranslated>; with this construct, the B<.if
!'po4a'hide'> conditional is not strictly needed since po4a will not parse
the internal of the macro definition)

or using an alias:
 .als IR_untranslated IR

 .IR_untranslated \-q ", " \-\-quiet

(this will require the B<-o untranslated=als,IR_untranslated> option)

=head2 Conclusion

To summarise this section, keep simple, and don't try to be clever while
authoring your man pages. A lot of things are possible in nroff, and not
supported by this parser. For example, don't try to mess with \c to
interrupt the text processing (like 40 pages on my box do). Or, be sure to
put the macro arguments on the same line that the macro itself. I know that
it's valid in nroff, but would complicate too much the parser to be
handled.

Of course, another possibility is to use another format, more translator
friendly (like POD using po4a::pod, or one of the XML familly like SGML),
but thanks to po4a::man it isn't needed anymore. That being said, if the
source format of your documentation is POD, or XML, it may be clever to
translate the source format and not this generated one. In most cases,
po4a::man will detect generated pages and issue a warning. It will even
refuse to process POD generated pages, because those pages are perfectly
handled by po4a::pod, and because their nroff counterpart defines a lot of
new macros I didn't want to write support for. On my box, 1432 of the 4323
pages are generated from POD and will be ignored by po4a::man.

In most cases, po4a::man will detect the problem and refuse to process the
page, issuing an adapted message. In some rare cases, the program will
complete without warning, but the output will be wrong. Such cases are
called "bugs" ;) If you encounter such case, be sure to report this, along
with a fix when possible...

=head1 STATUS OF THIS MODULE

This module can be used for most of the existing man pages.

Some tests are regularly run on Linux boxes:

=over 4

=item *

one third of the pages are refused because they were generated from
another format supported by po4a (e.g. POD or SGML).

=item *

10% of the remaining pages are rejected with an error (e.g. a
groff macro is not supported).

=item *

Then, less than 1% of the pages are accepted silently by po4a, but with
significant issues (i.e. missing words, or new words inserted)

=item *

The other pages are usually handled without differences more important
than spacing differences or line rewrapped (font issues in less than 10% of
the processed pages).

=back

=head1 SEE ALSO

L<Locale::Po4a::Pod(3pm)>,
L<Locale::Po4a::TransTractor(3pm)>,
L<po4a(7)|po4a.7>

=head1 AUTHORS

 Denis Barbier <barbier@linuxfr.org>
 Nicolas François <nicolas.francois@centraliens.net>
 Martin Quinson (mquinson#debian.org)

=head1 COPYRIGHT AND LICENSE

Copyright 2002-2008 by SPI, inc.

This program is free software; you may redistribute it and/or modify it
under the terms of GPL (see the COPYING file).

=cut

package Locale::Po4a::Man;
use DynaLoader;

use 5.006;
use strict;
use warnings;

require Exporter;
use vars qw(@ISA @EXPORT);
@ISA = qw(Locale::Po4a::TransTractor DynaLoader);
@EXPORT = qw();#  new initialize);

# Try to use a C extension if present.
eval('bootstrap Locale::Po4a::Man "0.30"');

use Locale::Po4a::TransTractor;
use Locale::Po4a::Common;

use File::Spec;
use Getopt::Std;

my %macro; # hash of known macro, with parsing sub. See end of this file
my %default_macro; # The default known macros, when no options are used.

# A font start by \f and is followed either by
# [.*] - a font name within brackets (e.g. [P], [A_USER_FONT])
# (..  - a parenthesis followed by two char (e.g. "(CW")
# .    - a single char (e.g. B, I, R, P, 1, 2, 3, 4, etc.)
my $FONT_RE = "\\\\f(?:\\[[^\\]]*\\]|\\(..|[^\\(\\[])";

# Variable used to identify non breaking spaces.
# These non breaking spaces are used to ease the parsing, and a
# translator can use them in her translation (and they will be translated
# into the groff non-breaking space.
my $nbs;

# Indicate if the page uses the mdoc macros
my $mdoc_mode = 0;

my $unknown_macros = undef;

#########################
#### DEBUGGING STUFF ####
#########################
my %debug;
# The following debug options can be set with '-o debug=...':
# * splitargs  see how macro args are separated
# * pretrans   see pre-conditioning of translation
# * postrans   see post-conditioning of translation
# * fonts      see font modifier handling


######## CONFIG #########
# This variable indicates the behavior of the module when a .de, .if or
# .ie is encountered.
my $groff_code;
# %no_wrap_begin and %no_wrap_end are lists of macros that respectively
# begins and ends a no_wrap paragraph.
# Any ending macro will end the no_wrap paragraph started by any beginning
# macro.
my %no_wrap_begin;
my %no_wrap_end;
# List of macros that should be inlined (with E<.xx ...>)
my %inline;
# The default list of inlined macros (when no options are used)
my %default_inline;
# This variable indicates whether po4a should try to detect the generated
# files.
my $allow_generated;
# This hash indicates section name that should not be translated in mdoc
# mode.
# The groff's mdoc processor requires the NAME section, otherwise headers
# and footers of the pages are not generated.
# The mdoc_groff man page indicates that NAME, SYNOPSIS and DESCRIPTION
# are mandatory.
my %mdoc;
sub initialize {
    my $self = shift;
    my %options = @_;

    $self->{options}{'debug'}='';
    $self->{options}{'verbose'}='';
    $self->{options}{'groff_code'}='';
    $self->{options}{'untranslated'}='';
    $self->{options}{'noarg'}='';
    $self->{options}{'translate_joined'}='';
    $self->{options}{'translate_each'}='';
    $self->{options}{'no_wrap'}='';
    $self->{options}{'inline'}='';
    $self->{options}{'generated'}='';
    $self->{options}{'mdoc'}='';
    $self->{options}{'unknown_macros'}='';

    foreach my $opt (keys %options) {
        if (defined $options{$opt}) {
            die wrap_mod("po4a::man",
                         dgettext("po4a", "Unknown option: %s"), $opt)
                unless exists $self->{options}{$opt};
            $self->{options}{$opt} = $options{$opt};
        }
    }

    %debug = ();
    if (defined $options{'debug'}) {
        foreach ($options{'debug'}) {
            $debug{$_} = 1;
        }
    }

    $groff_code = "fail";
    if (defined $options{'groff_code'}) {
        unless ($options{'groff_code'} =~ m/fail|verbatim|translate/) {
          die wrap_mod("po4a::man", dgettext("po4a",
               "Invalid 'groff_code' value. Must be one of 'fail', 'verbatim', 'translate'."));
        }
        $groff_code = $options{'groff_code'};
    }

    if (%default_macro) {
        %macro = %default_macro;
    } else {
        %default_macro = %macro
    }
    if (defined $options{'untranslated'}) {
        foreach (split(/,/, $options{'untranslated'})) {
            $macro{$_} = \&untranslated;
        }
    }
    if (defined $options{'noarg'}) {
        foreach (split(/,/, $options{'noarg'})) {
            $macro{$_} = \&noarg;
        }
    }
    if (defined $options{'translate_joined'}) {
        foreach (split(/,/, $options{'translate_joined'})) {
            $macro{$_} = \&translate_joined;
        }
    }
    if (defined $options{'translate_each'}) {
        foreach (split(/,/, $options{'translate_each'})) {
            $macro{$_} = \&translate_each;
        }
    }

    %no_wrap_begin = (
        'nf' => 1,
        'EX' => 1,
        'EQ' => 1
    );
    %no_wrap_end = (
        'fi' => 1,
        'EE' => 1,
        'EN' => 1
    );
    if (defined $options{'no_wrap'}) {
        foreach (split(/,/, $options{'no_wrap'})) {
            if ($_ =~ m/^(.*):(.*)$/) {
               $no_wrap_begin{$1} = 1;
               $no_wrap_end{$2} = 1;
            } else {
                die wrap_mod("po4a::man", dgettext("po4a","The no_wrap parameters must be a set of comma-separated begin:end couples.\n"));
            }
        }
    }

    if (%default_inline) {
        %inline = %default_inline;
    } else {
        %default_inline = %inline
    }
    if (defined $options{'inline'}) {
        foreach (split(/,/, $options{'inline'})) {
            $inline{$_} = 1;
        }
    }

    $allow_generated = 0;
    if (defined $options{'generated'}) {
        $allow_generated = 1;
    }

    %mdoc = ();
    if (defined $options{'mdoc'}) {
        if ($options{'mdoc'} eq 1) {
            $mdoc{"NAME"} = 1;
        } else {
            foreach (split(/,/, $options{'mdoc'})) {
                $mdoc{$_} = 1;
            }
        }
    }

    $unknown_macros = undef;
    if (defined $options{'unknown_macros'}) {
        if ($options{'unknown_macros'} eq "failed") {
            $unknown_macros = undef;
        } elsif ($options{'unknown_macros'} eq "untranslated") {
            $unknown_macros = \&untranslated;
        } elsif ($options{'unknown_macros'} eq "noarg") {
            $unknown_macros = \&noarg;
        } elsif ($options{'unknown_macros'} eq "translate_joined") {
            $unknown_macros = \&translate_joined;
        } elsif ($options{'unknown_macros'} eq "translate_each") {
            $unknown_macros = \&translate_each;
        } else {
            die wrap_mod("po4a::man", dgettext("po4a",
                "Invalid 'unknown_macros' value. Must be one of:\n").
                "failed untranslated noarg translate_joined translate_each\n");
        }
    }
}

my @comments = ();
my @next_comments = ();
# This function returns the next line of the document being parsed
# (and its reference).
# It overload the Transtractor shiftline to handle:
#   - font requests (.B, .I, .BR, .BI, ...)
#     because these requests can be present in a paragraph (handled
#     in the parse subroutine), or in argument (on the next line)
#     of some other request (for example .TP)
#   - font size requests (.SM,.SB) (not done yet)
#   - input escape (\ at the end of a line)
sub shiftline {
    my $self = shift;
    # call Transtractor's shiftline
NEW_LINE:
    my ($line,$ref) = $self->SUPER::shiftline();

    if (!defined $line) {
        # end of file
        return ($line,$ref);
    }

    # Do as few treatments as possible with the .de, .ie and .if sections
    if ($line =~ /^\.\s*(if|ie|de)/) {
        chomp $line;
        return ($line,$ref);
    }

    # Handle some escapes
    #   * reduce the number of \ in macros
    if ($line =~ /^\\?[.']/) {
        # The first backslash is consumed while the macro is read.
        $line =~ s/\\\\/\\/g;
    }
    #   * \\ is equivalent to \e, which is less error prone for the rest
    #     of the module (e.g. when searching for a font : \f, whe don't
    #     want to match \\f)
    $line =~ s/\\\\/\\e/g;
    #   * \. is just a dot (this can even be use to introduce a macro)
    $line =~ s/\\\././g;

    chomp $line;
    if ($line =~ m/^(.*?)(?:(?<!\\)\\(["#])(.*))$/) {
        my ($l, $t, $c) = ($1, $2, $3);
        $line = $l;
        unless ($allow_generated) {
                # Check for comments indicating that the file was generated.
                if ($c =~ /Pod::Man/) {
                    warn wrap_ref_mod($ref, "po4a::man", dgettext("po4a", "This file was generated with Pod::Man. Translate the POD file with the pod module of po4a."));
                    exit 254;
                } elsif ($c =~ /generated by help2man/)    {
                    warn wrap_ref_mod($ref, "po4a::man", dgettext("po4a", "This file was generated with help2man. Translate the source file with the regular gettext."));
                } elsif ($c =~ /with docbook-to-man/)      {
                    warn wrap_ref_mod($ref, "po4a::man", dgettext("po4a", "This file was generated with docbook-to-man. Translate the source file with the sgml module of po4a."));
                    exit 254;
                } elsif ($c =~ /generated by docbook2man/) {
                    warn wrap_ref_mod($ref, "po4a::man", dgettext("po4a", "This file was generated with docbook2man. Translate the source file with the sgml module of po4a."));
                    exit 254;
                } elsif ($c =~ /created with latex2man/)   {
                    warn wrap_ref_mod($ref, "po4a::man", dgettext("po4a",
                        "This file was generated with %s. ".
                        "You should translate the source file, but continuing anyway."
                        ),"latex2man");
                } elsif ($c =~ /Generated by db2man.xsl/)  {
                    warn wrap_ref_mod($ref, "po4a::man", dgettext("po4a","This file was generated with db2man.xsl. Translate the source file with the xml module of po4a."));
                    exit 254;
                } elsif ($c =~ /generated automatically by mtex2man/)  {
                    warn wrap_ref_mod($ref, "po4a::man", dgettext("po4a",
                        "This file was generated with %s. ".
                        "You should translate the source file, but continuing anyway."
                        ),"mtex2man");
                } elsif ($c =~ /THIS FILE HAS BEEN AUTOMATICALLY GENERATED.  DO NOT EDIT./ ||
                         $c =~ /DO NOT EDIT/i || $c =~ /generated/i) {
                    warn wrap_ref_mod($ref, "po4a::man", dgettext("po4a",
                        "This file contains the line '%s'. ".
                        "You should translate the source file, but continuing anyway."
                        ),$l."\\\"".$c);
                }
        }

        if ($line =~ m/^[.']*$/) {
            if ($c !~ m/^\s+$/) {
                # This commentted line may be comment for the next paragraph
                push @next_comments, $c;
            }
            if ($line =~ m/^[.']+$/) {
                # those lines are ignored
                # (empty lines are a little bit different)
                goto NEW_LINE;
            }
            if ($line =~ m/^\s*$/ and $t eq "#") {
                # Groff comments
                goto NEW_LINE;
            }
        } else {
            push @comments, $c;
        }
    } else {
        # finally, we did not reach the end of the paragraph.  The comments
        # belong to the current paragraph.
        push @comments, @next_comments;
        @next_comments = ();
    }

    # A .I or .B request change the current font
    # and on exit, switch the font to Roman
    # When one of these request doesn't have its argument on its line
    # (and when we support this usage), we must keep this font request to
    # insert it later.
    # It is a stack of fonts to be inserted (in case a .I is followed by
    # a .B and then followed bysome text; note that in this case,
    # only one \fR must be inserted at the end of the text)
    my $insert_font = "";
    while ($line =~ /\\$/ || $line =~ /^(\.[BI])\s*$/) {
        my ($l2,$r2)=$self->SUPER::shiftline();
        chomp($l2);
        if ($line =~ /^(\.[BI])\s*$/) {
            if ($l2 =~ /^[.'][\t ]*([BI]|BI|BR|IB|IR|RB|RI)(?:[\t ]|\s*$)/) {
                my $font = $line;
                $font =~ s/^\.([BI])\s*$/$1/;
                $insert_font = "\\f$font$insert_font";
                $line = $l2;
                $ref = $r2;
            } elsif ($l2 =~ /^[.'][\t ]*(SH|TP|TQ|P|PP|LP)(?:[\t ]|\s*$)/) {
                $line =~ s/^\.([BI])\s*$/$insert_font\\f$1/;
                $self->SUPER::unshiftline($l2,$r2);
            } elsif ($l2 =~ /^([.'][\t ]*(?:IP)[\t ]+"?)(.*)$/) {
                # Install the font modifier into the next line
                # after a possible quote (")
                my $macro = $1;
                my $arg   = $2;
                $line =~ /^\.([BI])\s*$/;
                $line = $macro."$insert_font\\f$1".$arg;
                $ref = $r2;
            } elsif ($l2 =~ /^[.']/) {
                warn wrap_ref_mod($ref, "po4a::man", dgettext("po4a",
                         "Font modifiers followed by a command may disturb ".
                         "po4a.  You should either remove the font modifier ".
                         "'%s', or integrate a \\f font modifier in the ".
                         "following command ('%s'), but continuing anyway."
                         ), $line, $l2);
                $line = "PO4A-INLINE:$line:PO4A-INLINE";
                $self->SUPER::unshiftline($l2,$r2);
            } else {
                # convert " to the groff's double quote glyph; it will be
                # converted back to " in pre_trans. It is needed because
                # otherwise, these quotes will be taken as arguments
                # delimiters.
                $l2 =~ s/"/\\(dq/g;
                # append this line to the macro, with surrounding quotes, so
                # that the line appear as an uniq argument.
                $line .= ' "'.$l2.'"';
            }
        } else {
            $line =~ s/\\$//;
            $line .= $l2;
        }
    }
    # Detect non-wrapped paragraphs
    # This must be done before handling the .B, .RI ... font requests
    $line =~ s/^($FONT_RE)(\s+)/$2$1/;

    $line .= "\n";

    # Handle font requests here
    if ($line =~ /^[.'][\t ]*([BI]|BI|BR|IB|IR|RB|RI)(?:(?: +|\t)(.*)|)$/) {
        my $macro = $1;
        my $arguments = $2;
        my @args = splitargs($ref,$arguments);
        if ($macro eq 'B' || $macro eq 'I') {
            # To keep the space(s), we must introduce some \&
            @args = map { $_ =~ s/^(\s*)$/\\&$1\\&/s; $_ } @args;
            my $arg=join(" ",@args);
            $arg =~ s/^ +//;
            this_macro_needs_args($macro,$ref,$arg);
            $line = "$insert_font\\f$macro".$arg."\\fR\n";
            $insert_font = "";
        }
        # .BI bold alternating with italic
        # .BR bold/roman
        # .IB italic/bold
        # .IR italic/roman
        # .RB roman/bold
        # .RI roman/italic
        if ($macro eq 'BI' || $macro eq 'BR' || $macro eq 'IB' ||
            $macro eq 'IR' || $macro eq 'RB' || $macro eq 'RI'   ) {
            # num of seen args, first letter of macro name, second one
            my ($i,$a,$b)=(0,substr($macro,0,1),substr($macro,1));
            $line = join("", map { $i++ % 2 ?
                                    "\\f$b$_" :
                                    "\\f$a$_"
                                 } @args)."\\fR\n";
            if ($i eq 0) {
                # If a .BI is used without argument, we must insert a
                # \fI\fR. The \fR was inserted previously.
                $line = "\\f$b$line";
            }
        }

        if (length $insert_font) {
            $line =~ s/\n$//;
            $line = "$insert_font$line\\fR\n";
        }

        if ($line =~ /^(.*)\\c(\\f.)?\s*\\fR\n/) {
            my $begin = $1;

            my ($l2,$r2)=$self->SUPER::shiftline();
            if ($l2 =~ /^[.']/) {
                $self->SUPER::unshiftline($l2,$r2);
            } else {
                $l2 =~ s/\s*$//s;
                $line = "$begin\\fR$l2\n";
            }
        }
    }

    return ($line,$ref);
}

# Overload Transtractor's pushline.
# This pushline first push comments (if there are comments for the
# current line, and the line is not empty), and then push the line.
sub pushline {
    my ($self, $line) = (shift, shift);
    if ($line !~ m/^\s*$/) {
        # add comments
        foreach my $c (@comments) {
            # comments are pushed (maybe at the wrong place).
            $self->SUPER::pushline($self->r(".\\\"$c\n"));
        }
        @comments = ();
    }

    $self->SUPER::pushline($line);
}

# The default unshiftline from Transtractor may fail because shiftline
# is overloaded
sub unshiftline {
    die wrap_mod("po4a::man", dgettext("po4a",
        "The unshiftline is not supported for the man module. ".
        "Please send a bug report with the groff page that generated ".
        "this error."));
}

###############################################
#### FUNCTION TO TRANSLATE OR NOT THE TEXT ####
###############################################
sub pushmacro {
    my $self=shift;
    if (scalar @_) {
        # Do quote the arguments containing spaces, as it should.

        #  but do not do so if they already contain quotes and escaped spaces
        # For example, cdrdao(1) uses:
        # .IP CATALOG\ "ddddddddddddd" (Here, the quote have to be displayed)
        # Adding extra quotes as in:
        # .IP "CATALOG\ "ddddddddddddd""
        # results in two args: 'CATALOG\ ' and 'ddddddddddddd""'
        $self->pushline(join(" ",map {
                # Replace double quotes by \(dq (double quotes could be
                # taken as an argument delimiter).
                # Only quotes not preceded by \ are taken into account
                # (\" introduces a comment).
                s/(?<!\\)"/\\\(dq/g if (defined $_);

                defined $_ ? (
                        length($_)?
                            (m/([^\\] |^ )/ ? "\"$_\"" : "$_")
                            # Quote arguments that contain a space.
                            # (not needed for non breaknig spaces, i.e.
                            # spaces preceded by '\')
                            :'""' # empty argument
                ) : '' # no argument
            } @_)."\n");
    } else {
        $self->pushline("\n");
    }
}
sub this_macro_needs_args {
    my ($macroname,$ref,$args)=@_;
    unless (length($args)) {
        die wrap_ref_mod($ref, "po4a::man", dgettext("po4a",
                "macro %s called without arguments. ".
                "Even if placing the macro arguments on the next line is authorized ".
                "by man(7), handling this would make the po4a parser too complicate. ".
                "Please simply put the macro args on the same line."
                ), $macroname);
    }
}

sub pre_trans {
    my ($self,$str,$ref,$type)=@_;
    # Preformatting, so that translators don't see
    # strange chars
    my $origstr=$str;
    print STDERR "pre_trans($str)="
        if ($debug{'pretrans'});

    # Do as few treatments as possible with the .de, .ie and .if sections
    if (defined $self->{type} && $self->{type} =~ m/^(ie|if|de)$/) {
        return $str;
    }

    # Note: if you want to implement \c support, the gdb man page is your playground
    if (    not defined $self->{type}) {
        $str =~ s/(\G|^(?:.*?)\n|^)        # Last position, or begin of a line
                   ([ \t]*[^.'][^\n]*(?<!\\)(?:\\\\)*) # the new line, which
                   \\c[ \t]*\n             # ends by \c and followed by a line
                   (?![ \t]*[.'])/$1$2/sgx;# not followed by a command (.')
    }
    die wrap_ref_mod($ref, "po4a::man", dgettext("po4a","Escape sequence \\c encountered. This is not completely handled yet."))
        if ($str =~ /\\c/);

    $str =~ s/>/E<gt>/sg;
    $str =~ s/</E<lt>/sg;
    $str =~ s/EE<lt>gt>/E<gt>/g; # could be done in a smarter way?

    while ($str =~ m/^(.*)PO4A-INLINE:(.*?):PO4A-INLINE(.*)$/s) {
        my ($t1,$t2, $t3) = ($1, $2, $3);
        $str = "$1E<$2>";
        if ($mdoc_mode) {
            # When a punctuation sign must be joined to an argument, mdoc
            # permits to use such a construct:
            # .Ar file1 , file2 , file3 ) .
            # Here, we move the punctuation out of the E<...> tag.
            # This is reverted in post_trans.
            # FIXME: To be checked with the French punctuation
            while ($str =~ m/(?<!\\) +([.,;:\)\]]) *>/s) {
                $str =~ s/(?<!\\) +([.,;:\)\]]) *>/>$1/s;
            }
        }
        if (defined $t3 and length $t3) {
            $t3 =~ s/^\n//s;
            $str .= "\n$t3";
        }
    }

    # simplify the fonts for the translators
    if (defined $self->{type} && $self->{type} =~ m/^(SH|SS)$/) {
        set_regular("B");
    }
    $str = do_fonts($str, $ref);
    if (defined $self->{type} && $self->{type} =~ m/^(SH|SS)$/) {
        set_regular("R");
    }

    # After the simplification, the first char can be a \n.
    # Simply push these newlines before the translation, but make sure the
    # resulting string is not empty (or an additional line will be
    # added).
    if ($str =~ /^(\n+)(.+)$/s) {
        $self->pushline($1);
        $str = $2;
    }

    unless ($mdoc_mode) {
    # Kill minus sign/hyphen difference.
    # Aestetic of printed man pages may suffer, but:
    #  * they are translator-unfriendly
    #  * they break when using utf8 (for obscure reasons)
    #  * they forbid the searches, since keybords don't have hyphen key
    #  * they forbid copy/paste, since options need minus sign, not hyphen
    $str =~ s|\\-|-|sg;

    # Groff bestiary
    $str =~ s/\\\*\(lq/``/sg;
    $str =~ s/\\\*\(rq/''/sg;
    $str =~ s/\\\(dq/"/sg;
    }

    # non-breaking spaces
    # some non-breaking spaces may have been added during the parsing
    $str =~ s/\Q$nbs/\\ /sg;

    print STDERR "$str\n" if ($debug{'pretrans'});
    return $str;
}

sub post_trans {
    my ($self,$str,$ref,$type,$wrap)=@_;
    my $transstr=$str;

    print STDERR "post_trans($str)="
        if ($debug{'postrans'});

    # Do as few treatments as possible with the .de, .ie and .if sections
    if (defined $self->{type} && $self->{type} =~ m/^(ie|if|de)$/) {
        return $str;
    }

    unless ($mdoc_mode) {
    # Post formatting, so that groff see the strange chars
    $str =~ s|\\-|-|sg; # in case the translator added some of them manually
    # change hyphens to minus signs
    # (this shouldn't be done for \s-<number> font size modifiers)
    # nor on .so/.mso args
    unless (defined $self->{type} && $self->{type} =~ m/^m?so$/) {
        my $tmp = "";
        while ($str =~ m/^(.*?)-(.*)$/s) {
            my $begin = $1;
            $str = $2;
            my $tmp2 = $tmp.$begin;
            if (   ($begin =~ m/(?<!\\)(\\\\)*\\s$/s)
                or ($begin =~ m/(?<!\\)(\\\\)*\\\((.|E<[gl]t>)?$/s)
                or ($tmp2 =~ m/(?<!\\)(\\\\)*\\[ZHhCv]'([^']|(?<!\\)(\\\\)*\\')*$/)
                or ($tmp2 =~ m/(?<!\\)(\\\\)*\\(\*)?\[([^\]]|(?<!\\)(\\\\)*\\\[)*$/)
                or ($tmp2 =~ m/(?<!\\)(\\\\)*\\\*\(.?$/)) {
                # Do not change - to \- for
                #  * \s-n (reduce font size)
                #  * \(.. (e.g. '<-', '-D')
                #  * inside a \h'...'
                #  * inside a \C'...'
                #  * inside a \[...]
                #  * inside a \*(..
                #  * inside a \*[...]
                #  * inside a \v'...'
                #  * inside a \H'...'
                #  * inside a \Z'...'
                $tmp = $tmp2."-";
            } else {
                $tmp = $tmp2."\\-";
            }
        }
        $str = $tmp.$str;
    }
    }

    # There must not be an end of line inside an inline macro
    $str =~ s/(E<\.[^>]*)\n([^>]*>)/$1 $2/gs;

    # No . or ' on first char, or nroff will think it's a macro
    # * at the beginning of a paragraph, add \& (zero width space) at
    #   the beginning of the line
    if (not defined $self->{type}) {
        # Only do it on regular text, because
        # his doesn't work after a TS (this macros shift
        # lines, which may contain macros)
        # or for the .ta arguments (e.g. .ta .5i 3i)
        $str =~ s/^((?:
                       (?:CW|[RBI])<
                      |$FONT_RE
                    )?
                    [.']
                   )/\\&$1/mgx;
    } elsif ($self->{type} =~ m/^(TP|TQ)$/) {
        # But it is also needed for some type (e.g. TP, if followed by a
        # font macro)
        # This regular expression is the same as above
        $str =~ s/^((?:(?:CW|[RBI])<|$FONT_RE)?[.'])/\\&$1/mg;
    }
    # * degraded mode, doesn't work for the first line of a paragraph
    $str =~ s/\n([.'])/ $1/mg;

    # Change ascii non-breaking space to groff one
    my $nbs_out = get_out_nbs($self->get_out_charset);
    $str =~ s/\Q$nbs_out/\\ /sg if defined $nbs_out;
    # No nbsp (said "\ " in groff on the last pos of the line, or groff adds
    # an extra space
    $str =~ s/\\ \n(?=.)/\\ /sg;

    # Make sure we compute internal sequences right.
    # think about: B<AZE E<lt> EZA E<gt>>
    while ($str =~ m/^(.*)(CW|[RBI])<(.*)$/s) {
        my ($done,$rest)=($1."\\f$2",$3);
        $done =~ s/CW$/\(CW/;
        my $lvl=1;
        while (length $rest && $lvl > 0) {
            my $first=substr($rest,0,1);
            if ($first eq '<') {
                $lvl++;
            } elsif ($first eq '>') {
                $lvl--;
            }
            $done .= $first  if ($lvl > 0);
            $rest=substr($rest,1);
        }
        die wrap_ref_mod($ref||$self->{ref}, "po4a::man", dgettext("po4a","Unbalanced '<' and '>' in font modifier. Faulty message: %s"),$str)
            if ($lvl > 0);
        # Return to the regular font
        $done .= "\\fP$rest";
        $str=$done;
    }

    while ($str =~ m/^(.*?)E<([.'][\t ]*.*?(?<!E<[gl]t))>(.*)$/s) {
        my ($t1, $t2, $t3) = ($1,$2,$3);
        $t1 =~ s/ +$//s;
        $t2 =~ s/\n/ /gs;
        if ($mdoc_mode) {
            # restore the punctuation inside the line (see pre_trans)
            if ($t3 =~ s/^([.,;:\)\]]+)//s) {
                my $punctuation = $1;
                $punctuation =~ s/([.,;:\)\]])/$1 /;
                $t2 .= " $punctuation";
            }
        }
        $t3 =~ s/^ +//s;
        if ($wrap) {
        # The no-wrap case should be checked
            $t1 =~ s/\n$//s;
        }
        $str = $t1;
        if (length $t1) {
            $t1 =~ s/\n$//s;
            $str = "$t1\n";
        }
        $str .= $t2;
        if (defined $t3 and length $t3) {
            $t3 =~ s/^\n//s;
            $str.= "\n$t3";
        }
    }
    my $str2 = $str;
    $str2 =~ s/E<[gl]t>//g;
    die wrap_ref_mod($ref||$self->{ref}, "po4a::man",
                     dgettext("po4a","Unknown '<' or '>' sequence. ".
                                     "Faulty message: %s"),$str)
        if $str2 =~ /[<>]/;
    $str =~ s/E<gt>/>/mg;
    $str =~ s/E<lt>/</mg;
    # Don't do that, because we'll go into trouble if previous line was .TP
    # $str =~ s/^\\f([BI])(.*?)\\f[RP]$/\.$1 $2/mg;

    unless ($mdoc_mode) {
        my $tmp = "";
        while ($str =~ m/^(.*?)(``|'')(.*)$/s) {
            $tmp .= $1;
            my $q = $2;
            $str = $3;
            # There are probably many more exceptions, here are those I could
            # detect in my manpages.
            # \*(.'  \*(.`
            # \*'    \*`
            # \N'xxx'
            if ($tmp =~ m/(?<!\\)(?:\\\\)*\\\*\($/s) {
                $tmp .= $q;
            } elsif (   $tmp =~ m/(?<!\\)(?:\\\\)*\\\*\(.$/s
                     or $tmp =~ m/(?<!\\)(?:\\\\)*\\\*$/s
                     or ($tmp =~ m/(?<!\\)(?:\\\\)*\\N'[0-9]*$/s
                         and $q eq "''")) {
                $q =~ m/(.)(.)/;
                $tmp .= $1;
                $str = $2.$str;
            } else {
                $q =~ s/``/\\\*\(lq/;
                $q =~ s/''/\\\*\(rq/;
                $tmp .= $q;
            }
        }
        $str = $tmp.$str;
    }
    if (not defined $self->{type}) {
        $str =~ s/(?<!\\) $//mg;
    }

    print STDERR "$str\n" if ($debug{'postrans'});
    return $str;
}
sub translate {
    my ($self,$str,$ref,$type) = (shift, shift, shift,shift);
    my (%options)=@_;
    my $origstr=$str;

    return $str unless (defined $str) && length($str);
    return $str if ($str eq "\n");
    # Do not translate the strings that only consist of fonts, spaces and
    # \&. This is useful because we introduced \& in shiftline.
    if ($str =~ m/^($FONT_RE|\s|\\&)*$/s) {
        do_fonts($str, $ref||$self->{ref});
        return $str;
    }

    # If a string is quoted, only translate the argument between the
    # quotes.
    if ($options{'wrap'} or $str !~ m/\n/s) {
        if ($str =~ m/^\"(.*)\"$/s and $1 !~ m/(?<!\\)\"/) {
            $str = '"'.$self->translate($1, $ref, $type, %options).'"';
            $str =~ s/\n"$/"\n/s;
            return $str;
        }
    }

    $str=pre_trans($self,$str,$ref||$self->{ref},$type);
    $options{'comment'} .= join('\n', @comments);
    # Translate this
    $str = $self->SUPER::translate($str,
                                   $ref||$self->{ref},
                                   $type || $self->{type},
                                   %options);
    if ($options{'wrap'}) {
        my (@paragraph);
        @paragraph=split (/\n/,$str);
        if (defined ($paragraph[0]) && $paragraph[0] eq '') {
            shift @paragraph;
        }
        $str = join("\n",@paragraph)."\n";
    }
    $str=post_trans($self,$str,$ref||$self->{ref},$type, $options{'wrap'});
    return $str;
}

# shortcut
sub t {
    return $_[0]->translate($_[1]);
}

# shortcut.
# As a rule of thumb, I do not recode macro names, unless they may be
# followed by other characters.
sub r {
    my $self = shift;
    my $str  = shift;

    # non-breaking spaces
    # some non-breaking spaces may have been added during the parsing
    $str =~ s/\Q$nbs/\\ /sg;

    return $self->recode_skipped_text($str);
}


sub do_paragraph {
    my ($self,$paragraph,$wrapped_mode) = (shift,shift,shift);

    # Following needed because of 'ft' (at least, see ft macro below)
    unless ($paragraph =~ m/\n$/s) {
        my @paragraph = split(/\n/,$paragraph);

        $paragraph .= "\n"
            unless scalar (@paragraph) == 1;
    }

    $self->pushline( $self->translate($paragraph,$self->{ref},"Plain text",
                                      "wrap" => ($wrapped_mode eq 'YES') ) );
}

#############################
#### MAIN PARSE FUNCTION ####
#############################
sub parse{
    my $self = shift;
    my ($line,$ref);
    my ($paragraph)=""; # Buffer where we put the paragraph while building
    my $wrapped_mode='YES'; # Should we wrap the paragraph? Three possible values:
                            # YES: do wrap
                            # NO: don't wrap because this paragraph contains indented lines
                            #     this status disapear after the end of the paragraph
                            # MACRONO: don't wrap because we saw the nf macro. It stays so
                            #          until the next fi macro.


    # We want to change the non-breaking space according to the input
    # document charset
    $nbs = get_in_nbs($self->{TT}{'file_in_charset'});

  LINE:
    undef $self->{type};
    ($line,$ref)=$self->shiftline();

    while (defined($line)) {
#        print STDERR "line=$line;ref=$ref";
        chomp($line);
        $self->{ref}="$ref";
#        print STDERR "LINE=$line<<\n";


        if ($line =~ /^[.']/) {
            die wrap_mod("po4a::man", dgettext("po4a", "Unparsable line: %s"), $line)
                unless ($line =~ /^([.']+\\*?)(\\["#])(.*)/ ||
                        $line =~ /^([.'])(\S*)(.*)/);
            my $arg1=$1;
            $arg1 .= $2;
            my $macro=$2;
            my $arguments=$3;

            if ($inline{$macro}) {
                $paragraph .= "PO4A-INLINE:".$line.":PO4A-INLINE\n";
                goto LINE;
            }

            # Split on spaces for arguments, but not spaces within double quotes
            my @args=();
            push @args,$arg1;
            if ($macro =~ /^(?:ta|TP|ie|if|de)$/) {
                # The number of spaces may be critical for the 'ta' macro,
                # and there is no need to split the arguments.
                push @args, $arguments;
            } else {
                push @args, splitargs($ref,$arguments);
            }


            if (length($paragraph)) {
                do_paragraph($self,$paragraph,$wrapped_mode);
                $paragraph="";
                $wrapped_mode = $wrapped_mode eq 'NO' ? 'YES' : $wrapped_mode;
            }

            # Special case: Don't change these lines
            #  .\"  => comments
            #  .\#  => comments
            #  ."   => comments
            #  .    => empty point on the line
            #  .tr abcd...
            #       => substitution like Perl's tr/ac/bd/ on output.
            if ($macro eq '\\"' || $macro eq '' || $macro eq 'tr' ||
                $macro eq '"'   || $macro eq '\\#') {
                $self->pushline($self->r($line)."\n");
                goto LINE;
            }
            # Special case:
            #  .nf => stop wrapped mode
            #  .fi => wrap again
            if ($no_wrap_begin{$macro} or $no_wrap_end{$macro}) {
                if ($no_wrap_end{$macro}) {
                    $wrapped_mode='YES';
                } else {
                    $wrapped_mode='MACRONO';
                }
                $self->pushline($self->r($line)."\n");
                goto LINE;
            }

            # SH resets the wrapping (in addition to starting a section)
            if ($macro eq 'SH') {
                $wrapped_mode='YES';
            }

            unshift @args,$self;
            # Apply macro
            $self->{type}=$macro;

            if (defined ($macro{$macro})) {
                &{$macro{$macro}}(@args);
            } else {
                if (defined $unknown_macros) {
                    &{$unknown_macros}(@args);
                } else {
                $self->pushline($self->r($line)."\n");
                die wrap_ref_mod($ref, "po4a::man", dgettext("po4a",
                    "Unknown macro '%s'. Remove it from the document, or refer to the Locale::Po4a::Man manpage to see how po4a can handle new macros."), $line);
                }
            }

        } elsif ($line =~ /^ +[^. ]/) {
            # (Lines containing only spaces are handled as empty lines)
            # Not a macro, but not a wrapped paragraph either
            $wrapped_mode = $wrapped_mode eq 'YES' ? 'NO' : $wrapped_mode;
            $paragraph .= $line."\n";
        } elsif ($line =~ /^[^.].*/ && $line !~ /^ *$/) {
            # (Lines containing only spaces are handled latter as empty lines)
            if ($line =~ /^\\"/) {
                # special case: the line is entirely a comment, keep the
                # comment.
                # NOTE: comment could also be found in the middle of a line.
                # From info groff:
                # Escape: \": Start a comment.  Everything to the end of the
                # input line is ignored.
                $self->pushline($self->r($line)."\n");
                goto LINE;
            } elsif ($line =~ /^\\#/) {
                # Special groff comment. Do not keep the new line
                goto LINE;
            } else {
                # Not a macro
                # * first, try to handle some "output line continuation" (\c)
                $paragraph =~ s/\\c *(($FONT_RE)?)\n?$/$1/s;
                # * append the line to the current paragraph
                $paragraph .= $line."\n";
            }
        } else { #empty line, or line containing only spaces
            if (length($paragraph)) {
                do_paragraph($self,$paragraph,$wrapped_mode);
                $paragraph="";
            }
            $wrapped_mode = $wrapped_mode eq 'NO' ? 'YES' : $wrapped_mode;
            $self->pushline($line."\n");
        }

        # finally, we did not reach the end of the paragraph.  The comments
        # belong to the current paragraph.
        push @comments, @next_comments;
        @next_comments = ();

        # Reinit the loop
        ($line,$ref)=$self->shiftline();
        undef $self->{type};
    }

    if (length($paragraph)) {
        do_paragraph($self,$paragraph,$wrapped_mode);
        $wrapped_mode = $wrapped_mode eq 'NO' ? 'YES' : $wrapped_mode;
        $paragraph="";
    }

    # flush the last comments
    push @comments, @next_comments;
    @next_comments = @comments;
    @comments = ();
    for my $c (@next_comments) {
        $self->pushline($self->r(".\\\"$c\n"));
    }

    # reinitialize the module
    @next_comments = ();
    set_regular("R");
    set_font("R");
    set_font("R");
    $mdoc_mode = 0;
} # end of main

# Cache the results of get_in_nbs and get_out_nbs
{
    my $last_in_charset;
    my $last_in_nbs;

# get_in_nbs(charset)
# Return the representation of a non breaking space in the input charset
# (given in argument).
# or PO4A:VERY_IMPROBABLE_STRING_USEDFOR_NON-BREAKING-SPACES if this
# character doesn't exist in this charset.
    sub get_in_nbs() {
        my $charset = shift;

        return $last_in_nbs
            if (    defined $charset
                and defined $last_in_charset
                and $charset eq $last_in_charset);

        my $nbs = "\xA0";
        my $length;
        if (defined $charset and length $charset)
        {
            eval ("\$length = Encode::from_to(\$nbs, \"latin-1\",
                                              \$charset,
                                              1)");
        }
        # fall back solution
        $nbs = "PO4A:VERY_IMPROBABLE_STRING_USEDFOR_NON-BREAKING-SPACES"
            unless defined $length;
        $last_in_charset = $charset;
        $last_in_nbs = $nbs;

        return $last_in_nbs;
    }

    my $last_out_charset;
    my $last_out_nbs;
# get_out_nbs(charset)
# Return the representation of a non breaking space in the output charset
# (given in argument).
# or undef if this character doesn't exist in this charset.
    sub get_out_nbs() {
        my $charset = shift;

        return $last_out_nbs
            if (    defined $charset
                and defined $last_out_charset
                and $charset eq $last_out_charset);

        my $nbs = "\xA0";
        my $length;
        if (defined $charset and length $charset)
        {
            eval ("\$length = Encode::from_to(\$nbs, \"latin-1\",
                                              \$charset,
                                              1)");
        }
        # fall back solution
        undef $nbs
            unless defined $length;
        $last_out_charset = $charset;
        $last_out_nbs = $nbs;

        return $last_out_nbs;
    }

}

# We can't push the header in the first line of the document, as in the
# other module, because the first line may contain indications on how the
# man page must be processed.
sub docheader {
    return "";
}

# The header is pushed just before the .TH macro (this macro is mandatory
# and must be specified at the begining (there may be macro definitions
# before).
sub push_docheader {
    my $self = shift;
    $self->pushline(
".\\\"*******************************************************************\n".
".\\\"\n".
".\\\" This file was generated with po4a. Translate the source file.\n".
".\\\"\n".
".\\\"*******************************************************************\n"
    );
}

# Split request's arguments.
# see:
#     info groff --index-search "Request Arguments"
sub splitargs {
    my ($ref,$arguments) = ($_[0],$_[1]);
    my @args=();
    my $buffer="";
    my $escaped=0;
    if (! defined $arguments) {
        return @args;
    }
    # change non-breaking space before to ensure that split does what we want
    # We change them back before pushing into the arguments. The one which
    # will be translated will have the same change again (in pre_trans and
    # post_trans), but the ones which won't get translated are not changed
    # anymore. Let's play safe.
    $arguments =~ s/\\ /$nbs/g;
    $arguments =~ s/^ +//;
    $arguments =~ s/\\&"/\\(dq/g;
    $arguments =~ s/^ *//;
    while (length $arguments) {
        if ($arguments =~ s/^"((?:[^"]|"")*)"(?!") *//) {
            my $a = $1;
            $a =~ s/""/"/g if defined $a;
            push @args,$a;
        } elsif ($arguments =~ s/^"((?:[^"]|"")*) *$//) {
            # Unterminated quote, but this seems to be handled by removing
            # the trailing spaces and closing the quotes.
            my $a = $1;
            $a =~ s/""/"/g if defined $a;
            push @args,$a;
        } elsif ($arguments =~ s/^([^ ]+) *//) {
            push @args,$1;
        } else {
            die wrap_ref_mod($ref, "po4a::man", dgettext("po4a",
                             "Cannot parse command arguments: %s"),
                             $arguments)
        }
    }
    if ($debug{'splitargs'}) {
        print STDERR "ARGS=";
        map { print STDERR "$_^"} @args;
        print STDERR "\n";
    }

    return @args;
}

{
    #static variables
    # font stack.
    #     Keep track of the current font (because a font modifier can
    #     stay open at the end of a paragraph), and the previous font (to
    #     handle \fP)
    my $current_font  = "R";
    my $previous_font = "R";
    # $regular_font describe the "Regular" font, which is the font used
    # when there is no font modifier.
    # For example, .SS use a Bold font, and thus in
    # .SS This is a \fRsubsection\fB header
    # the \fR and \fB font modifiers have to be kept.
    my $regular_font  = "R";

    # Set the regular font
    # It takes the regular font in argument (when no argument is provided,
    # it uses "R").
    sub set_regular {
        print STDERR "set_regular('@_')\n"
            if ($debug{'fonts'});
        set_font(@_);
        $regular_font = $current_font;
    }

    sub set_font {
        print STDERR "set_font('@_')\n"
            if ($debug{'fonts'});
        my $saved_previous = $previous_font;
        $previous_font = $current_font;

        if (! defined $_[0]) {
            $current_font = "R";
        } elsif ($_[0] =~ /^(P|\[\]|\[P\])/) {
            $current_font = $saved_previous;
        } elsif (length($_[0]) == 1) {
            $current_font = $_[0];
        } elsif (length($_[0]) == 2) {
            $current_font = "($_[0]";
        } else {
            $current_font = "[$_[0]]";
        }
        print STDERR "r:'$regular_font', p:'$previous_font', c:'$current_font'\n"
            if ($debug{'fonts'});
    }

    sub do_fonts {
        # one argument: a string
        my ($str, $ref) = (shift, shift);
        print STDERR "do_fonts('$str', '$ref')="
            if ($debug{'fonts'});

        # restore the font stack
        $str = "\\f$previous_font\\f$current_font".$str;
        # In order to be able to split on /\\f/, without problem with
        # \\foo, groff backslash (\\) are changed to the (equivalent)
        # form: \e (this should be done in shiftline).
        my @array1=split(/\\f/, $str);

        $str = shift @array1;  # The first element is always empty because
                               # the $current_font was put at the beginning
        # $last_font indicates the last font that was appended to the buffer.
        # It differ from $current_font because concecutive identical fonts
        # are not written in the buffer.
        my $last_font=$regular_font;

        foreach my $elem (@array1) {
            # Do not touch the fonts in the inline macros
            # These inline macros may have their argument in bold or italic,
            # we can't know.
            if ($str =~ m/E<\.([^>]|E<gt>|E<lt>)*$/s) {
                # We can't use \\f here, otherwise the font simplifier regexp
                # will use the fonts of the inline macros.
                $str .= "PO4A-FAKE-FONT".$elem;
                next;
            }

            # Replace \fP by the exact font (because some font modifiers will
            # be removed or added, which will break groff's font stack)
            $elem =~ s/^(P|\[\]|\[P\])/$previous_font/s;
                # change \f1 to \fR, etc.
                # Those fonts are defined in the DESC file, which
                # may depend on the groff device.
                # fonts 1 to 4 are usually mapped to R, I, B, BI
                # TODO: use an array for the font positions. This
                # array should be updated by .fp requests.
                $elem =~ s/^1/R/;
                $elem =~ s/^2/I/;
                $elem =~ s/^3/B/;
                $elem =~ s/^4/(BI/;

            if ($elem =~ /^([1-4]|B|I|R|\(..|\[[^]]*\]|L)(.*)$/s) {
                # Each element should now start by a recognized font modifier
                my $new_font = $1;
                my $arg = $2;
                # Update the font stack
                $previous_font = $current_font;
                $current_font = $new_font;

                if ($new_font eq $last_font) {
                    # continue with the same font.
                    $str.=$arg;
                } else {
                    # A new font is used, update $last_font
                    $last_font = $new_font;
                    $str .= "\\f".$elem;
                }
            } else {
                die wrap_ref_mod($ref,
                                 "po4a::man",
                                 dgettext("po4a",
                                          "Unsupported font in: '%s'."),
                                 "\\f".$elem);
            }
        }
        # Do some simplification (they don't change the font stack)
        # Remove empty font modifiers at the end
        $str =~ s/($FONT_RE)*$//s;

        # close any font modifier
        if ($str =~ /.*($FONT_RE)(.*?)$/s && $1 ne "\\f$regular_font") {
            $str =~ s/(\n?)$/\\f$regular_font$1/;
        }

        # remove fonts with empty argument
        while ($str =~ /($FONT_RE){2}/) {
            # while $str has two consecutive font modifiers
            # only keep the second one.
            $str =~ s/($FONT_RE)($FONT_RE)/$2/s;
        }

        # when there are two consecutive switches to the regular font,
        # remove the last one.
        while ($str =~ /^(.*)\\f$regular_font # anything followed by a
                                              # regular font
                        ((?:\\(?!f)|[^\\])*)  # the text concerned by
                                              # this font (i.e. without any
                                              # font modifier, i.e. it
                                              # contains no '\' followed by
                                              # an 'f')
                        \\f$regular_font      # another regular font
                        (.*)$/sx) {
            $str = "$1\\f$regular_font$2$3";
        }

        # the regular font modifier at the beginning of the string is not
        # needed (the do_fonts subroutine ensure that every paragraph ends with
        # the regular font.
        if ($str =~ /^(.*?)\\f$regular_font(.*)$/s && $1 !~ /$FONT_RE/) {
            $str = "$1$2";
        }

        # Use special markup for common fonts, so that translators don't see
        # groff's font modifiers
        my $PO_FONTS = "B|I|R|\\(CW";
        # remove the regular font from this list
        $PO_FONTS =~ s/^$regular_font\|//;
        $PO_FONTS =~ s/\|$regular_font\|/|/;
        $PO_FONTS =~ s/\|$regular_font$//;
        while ($str =~ /^(.*?)                  # $1: anything (non greedy: as
                                                # few as possible)
                         \\f($PO_FONTS)         # followed by a common font
                                                # modifier ($2)
                         ((?:\\[^f]|[^\\])*)    # $3: the text concerned by
                                                # this font (i.e. without any
                                                # font modifier, i.e. it
                                                # contains no '\' followed by
                                                # an 'f')
                         \\f                    # the next font modifier
                         (.*)$/sx) {            # $4: anything up to the end
            my ($begin, $font, $arg, $end) = ($1,$2,$3,$4);
            if ($end =~ /^$regular_font(.*)$/s) {
                # no need to add a switch to $regular_font
                $str = $begin."$font<$arg>$1";
            } else {
                $str = $begin."$font<$arg>\\f$end";
            }
        }
        $str =~ s/\(CW</CW</sg;
        $str =~ s/PO4A-FAKE-FONT/\\f/sg;

        print STDERR "'$str'\n" if ($debug{'fonts'});
        return $str;
    }
}

##########################################
#### DEFINITION OF THE MACROS WE KNOW ####
##########################################
# Each sub is passed self as first arg,
#   plus the args present on the roff line
#   ie, <<.TH LS "1" "October 2002" "ls (coreutils) 4.5.2" "User Commands">>
#   is passed (".TH","LS","1","October 2002","ls (coreutils) 4.5.2","User Commands")
#   Macro name is also passed, because .B (bold) will be encoded in pod format (and mangeled).
# They should return a list, which will be join'ed(' ',..)
#   or undef when they don't want to add anything

# Some well known macro handling

# For macro taking only one argument, but people may forget the quotes.
# Example: >>.SH Another Section<< which should be >>.SH "Another Section"<<
sub translate_joined {
    my ($self,$macroname,$macroarg)=(shift,shift,join(" ",@_));
    #section# .S[HS] name

    $self->pushmacro($macroname,
                     $self->t($macroarg));
}

# For macro taking several arguments, having to be translated separately
sub translate_each {
    my ($self,$first)= (shift,0);
    $self->pushmacro( map { $first++ ?$self->t($_):$_ } @_);
}

# For macro which shouldn't be given any arg
sub noarg {
    my $self = shift;
    warn "Macro $_[0] does not accept any argument\n"
        if (defined ($_[1]));
    $self->pushmacro(@_);
}

# For macro whose arguments shouldn't be translated
sub untranslated {
    my ($self,$first)= (shift,0);
    $self->pushmacro( map { $first++ ?$self->r($_):$_ } @_);
}

###
### man 7 man
###

$macro{'TH'}= sub {
    my $self=shift;
    my ($th,$title,$section,$date,$source,$manual)=@_;
    #Preamble#.TH      title     section   date     source   manual
#    print STDERR "TH=$th;titre=$title;sec=$section;date=$date;source=$source;manual=$manual\n";

    # Reset the memories
    $self->push_docheader();

    $self->pushmacro($th,
                     $self->t($title),
                     $section,
                     $self->t($date),
                     $self->t($source),
                     $self->t($manual));
};

# .SS t    Subheading t (like .SH, but used for a subsection inside a section).
$macro{'SS'}=$macro{'SH'}=sub {
    if (!defined $_[2]) {
        # The argument is on the next line.
        my ($self,$macroname) = (shift,shift);
        my ($l2,$ref2) = $self->shiftline();
        if ($l2 =~/^\./) {
            $self->SUPER::unshiftline($l2,$ref2);
        } else {
            chomp($l2);
            $self->pushmacro($macroname,
                             $self->t($l2));
        }
        return;
    } else {
        return translate_joined(@_);
    }
};

# Macro: .SM [text]
#     Set the text on the same line or the text on the next line in a
#     font that is one point size smaller than the default font.
# FIXME: Maybe we should find a better way to represent this (inline is
#        not really nice in the PO).
$inline{'SM'}=1;

# .SP n     Skip n lines (I think)
$macro{'SP'}=\&untranslated;

#Normal Paragraphs
#  .LP      Same as .PP (begin a new paragraph).
#  .P       Same as .PP (begin a new paragraph).
#  .PP      Begin a new paragraph and reset prevailing indent.
#Relative Margin Indent
#  .RS i    Start relative margin indent - moves the left margin i to the right
#           As a result,  all  following  paragraph(s) will be indented until
#           the corresponding .RE.
#  .RE      End  relative  margin indent.
$macro{'LP'}=$macro{'P'}=$macro{'PP'}=sub {
    noarg(@_);

    # From info groff:
    # The font size and shape are reset to the default value (10pt roman if no
    # `-rS' option is given on the command line).
    set_font("R");
};
$macro{'RE'}=\&noarg;
$macro{'RS'}=\&untranslated;

sub parse_tp_tq {
    my $self=shift;
    my ($line,$l2,$ref2);
    $line .= $_[0] if defined($_[0]);
    $line .= ' '.$_[1] if defined($_[1]);
    $self->pushline($self->r($line)."\n");

    ($l2,$ref2) = $self->shiftline();
    chomp($l2);
    while ($l2 =~ /^\.PD/) {
        $self->pushline($self->r($l2)."\n");
        ($l2,$ref2) = $self->shiftline();
        chomp($l2);
    }
    if ($l2 =~/^([.'][\t ]*([^\t ]*))(?:([\t ]+)(.*)$|$)/) {
        if ($inline{$2}) {
            my $tmp = "";
            if (defined $4 and length $4) {
                $tmp = $3.$self->t($4, "wrap" => 0);
            }
            $self->pushline($1.$tmp."\n");
        } else {
            # If the line after a .TP is a macro,
            # let the parser do it's job.
            # Note: use Transtractor unshiftline for now. This may require an
            #       implementation of the man module's own implementation.
            #       This may be a problem if, for example, the line resulted
            #       of a line continuation.
            $self->SUPER::unshiftline($l2,$ref2);
        }
    } else {
        $self->pushline($self->t($l2, "wrap" => 0)."\n");
    }
}

#Indented Paragraph Macros
#  .TP i    Begin  paragraph  with  hanging tag.  The tag is given on the next line,
#           but its results are like those of the .IP command.
$macro{'TP'}=sub {
    parse_tp_tq(@_);

    # From info groff:
    # Note that neither font shape nor font size of the label [i.e. argument
    # or first line] is set to a default value; on the other hand, the rest of
    # the text has default font settings.
    set_font("R");
};

# Indented Paragraph Macros
# .TQ  Indicates continuation of the .TP labels that precede the indented
#      paragraph.
$macro{'TQ'}=sub {
    warn "Macro $_[1] does not accept any argument\n"
        if (defined ($_[2]));

    parse_tp_tq(@_);
};

#   Indented Paragraph Macros
#       .HP i    Begin paragraph with a hanging indent (the first line of  the  paragraph
#                is  at  the  left margin of normal paragraphs, and the rest of the para-
#                graph's lines are indented).
#
$macro{'HP'}=sub {
    untranslated(@_);

    # From info groff:
    # Font size and face are reset to their default values.
    set_font("R");
};

# Indented Paragraph Macros
#       .IP [designator] [nnn]
#              Sets up an indented paragraph, using designator as a  tag  to  mark
#              its  beginning.   The indentation is set to nnn if that argument is
#              supplied (default unit is `n'), otherwise the  default  indentation
#              value  is  used.   Font size and face of the paragraph (but not the
#              designator) are reset to its default values.  To start an  indented
#              paragraph  with  a particular indentation but without a designator,
#              use `""' (two doublequotes) as the second argument.

# Note that the above is the groff_man(7) version, which of course differs radically
# from man(7). In one case, the designator is optional and the nnn is not, and the
# contrary in the other. This implies that when sticking to groff_man(7), we should
# mark an uniq argument as translatable.

$macro{'IP'}=sub {
    my $self=shift;
    if (defined $_[2]) {
        $self->pushmacro($_[0],$self->t($_[1]),$_[2]);
    } elsif (defined $_[1]) {
        $self->pushmacro($_[0],$self->t($_[1]));
    } else {
        $self->pushmacro(@_);
    }

    # From info groff:
    # Font size and face of the paragraph (but not the designator) are reset
    # to their default values.
    set_font("R");
};

# Hypertext Link Macros
#  .UR u  Begins a hypertext link to the URI (URL) u; it will end with
#         the corresponding UE command. When generating HTML this should
#         translate into the HTML command <A HREF="u">.
#         There is an exception: if u is the special value ":", then no
#         hypertext link of any kind will be generated until after the
#         closing UE (this permits disabling hypertext links in
#         phrases like LALR(1) when linking is not appropriate).
#  .UE    Ends the corresponding UR command; when generating HTML this
#         should translate into </A>.
#  .UN u  Creates a named hypertext location named u; do not include a
#         corresponding UE  command.
#         When generating HTML this should translate into the HTML command
#         <A  NAME="u" id="u">&nbsp;</A>
$inline{'UR'}=1;
$inline{'UE'}=1;
$macro{'UN'}=\&translate_joined;

# Miscellaneous Macros
#  .DT      Reset tabs to default tab values (every 0.5 inches); does not
#           cause a break.
#  .PD d    Set inter-paragraph vertical distance to d (if omitted, d=0.4v);
#            does not cause a break.
$macro{'DT'}=\&noarg;
$macro{'PD'}=\&untranslated;

# Indexing term (printed on standard error).
# (ms macros)
$macro{'IX'}=\&translate_each;

###
### groff macros
###
# .br
$macro{'br'}=\&noarg;
# .bp N      Eject current page and begin new page.
$macro{'bp'}=\&untranslated;
# .ad       Begin line adjustment for output lines in current adjust mode.
# .ad c     Start line adjustment in mode c (c=l,r,b,n).
$macro{'ad'}=\&untranslated;
# .de macro Define or redefine macro until .. is encountered.
$macro{'de'}=sub {
    my $self = shift;
    if ($groff_code ne "fail") {
        my $paragraph = "@_";
        my $end = ".";
        if ($paragraph=~/^[.'][\t ]*de[\t ]+([^\t ]+)[\t ]+([^\t ]+)[\t ]$/) {
            $end = $2;
        }
        my ($line, $ref) = $self->SUPER::shiftline();
        chomp $line;
        $paragraph .= "\n".$line;
        while (defined($line) and $line ne ".$end") {
            ($line, $ref) = $self->SUPER::shiftline();
            if (defined $line) {
                chomp $line;
                $paragraph .= "\n".$line;
            }
        }
        $paragraph .= "\n";
        if ($groff_code eq "verbatim") {
            $self->pushline( $self->r($paragraph) );
        } else {
            $self->pushline( $self->translate($paragraph,
                                              $self->{ref},
                                              "groff code",
                                              "wrap" => 0) );
        }
    } else {
        die wrap_ref_mod($self->{ref}, "po4a::man", dgettext("po4a", "This page defines a new macro with '.de'. Since po4a is not a real groff parser, this is not supported."));
    }
};
# .ds stringvar anything
#                 Set stringvar to anything.
$macro{'ds'}=sub {
    my ($self, $m) = (shift,shift);
    my $name = shift;
    my $string = "@_";
    # indicate to which variable this corresponds. The translator can
    # find references to this string in the translation "\*(name" or
    # "\*[name]"
    $self->{type} = "ds $name";
    $self->pushline($m." ".$self->r($name)." ".$self->translate($string)."\n");
};
#       .fam      Return to previous font family.
#       .fam name Set the current font family to name.
$macro{'fam'}=\&untranslated;
# .fc a b   Set field delimiter to a and pad character to b.
$macro{'fc'}=\&untranslated;
# .ft font  Change to font name or number font;
$macro{'ft'}=sub {
    if (defined $_[2]) {
        set_font($_[2]);
    } else {
        set_font("P");
    }
};
# .hc c     Set up additional hyphenation indicator character c.
$macro{'hc'}=\&untranslated;
# .hy       Enable hyphenation (see nh)
# .hy N     Switch to hyphenation mode N.
# .hym n    Set the hyphenation margin to n (default scaling indicator m).
# .hys n    Set the hyphenation space to n.
$macro{'hy'}=$macro{'hym'}=$macro{'hys'}=\&untranslated;

# .ie cond anything  If cond then anything else goto .el.
# .if cond anything  If cond then anything; otherwise do nothing.
$macro{'ie'}=$macro{'if'}=sub {
    my $self = shift;
    if ($groff_code ne "fail") {
        my $m = $_[0];
        my $paragraph = "@_";
        my ($line,$ref);
        my $count = 0;
        $count = 1 if ($paragraph =~ m/(?<!\\)\\\{/s);
        while (   ($paragraph =~ m/(?<!\\)\\$/s)
               or ($count > 0)) {
            ($line,$ref)=$self->SUPER::shiftline();
            chomp $line;
            $paragraph .= "\n".$line;
            $count += 1 if ($line =~ m/(?<!\\)\\\{/s);
            $count -= 1 if ($line =~ m/(?<!\\)\\\}/s);
        }
        if ($m eq '.ie') {
            # The .el line may be preceded by comments
            ($line,$ref)=$self->SUPER::shiftline();
            chomp $line;
            while ($line =~ m/^[.']\\"/) {
                $paragraph .= "\n".$line;
                ($line,$ref)=$self->SUPER::shiftline();
                chomp $line;
            }

            if ($line !~ m/^[.'][ \t]*el(\s|\\\{)/) {
                die wrap_ref_mod($self->{ref}, "po4a::man", dgettext("po4a",
                            "The .ie macro must be followed by a .el macro."));
            }
            my $paragraph2 = $line;
            $count = 0;
            $count = 1 if ($line =~ m/(?<!\\)\\\{/s);
            while (   ($paragraph2 =~ m/(?<!\\)\\$/s)
                   or ($count > 0)) {
                ($line,$ref)=$self->SUPER::shiftline();
                chomp $line;
                $paragraph2 .= "\n".$line;
                $count += 1 if ($line =~ m/(?<!\\)\\\{/s);
                $count -= 1 if ($line =~ m/(?<!\\)\\\}/s);
            }
            $paragraph .= "\n".$paragraph2;
        }
        $paragraph .= "\n";
        if ($groff_code eq "verbatim") {
            $self->pushline( $self->r($paragraph) );
        } else {
            $self->pushline( $self->translate($paragraph,
                                              $self->{ref},
                                              "groff code",
                                              "wrap" => 0) );
        }
    } else {
        die wrap_ref_mod($self->{ref}, "po4a::man", dgettext("po4a",
            "This page uses conditionals with '%s'. Since po4a is not a real groff parser, this is not supported."), $_[0]);
    }
};
# .in  N    Change indent according to N (default scaling indicator m).
$macro{'in'}=\&untranslated;

# .ig end   Ignore text until .end.
$macro{'ig'}=sub {
    my $self = shift;
    $self->pushmacro(@_);
    my ($name,$end) = (shift,shift||'');
    $end='' if ($end =~ m/^\\\"/);
    my ($line,$ref)=$self->shiftline();
    while (defined($line)) {
        $self->pushline($self->r($line));
        last if ($line =~ /^\.$end\./);
        ($line,$ref)=$self->shiftline();
    }
};


# .lf N file  Set input line number to N and filename to file.
$macro{'lf'}=\&untranslated;
# .ll N       Set line length according to N
$macro{'ll'}=\&untranslated;

# .nh         disable hyphenation (see hy)
$macro{'nh'}=\&untranslated;
# .na       No Adjusting (see ad)
$macro{'na'}=\&untranslated;
# .ne N     Need N vertical space
$macro{'ne'}=\&untranslated;
# .nr register N M
#         Define or modify register
$macro{'nr'}=\&untranslated;
# .ps N    Point size; same as \s[N]
$macro{'ps'}=\&untranslated;
# .so filename Include source file.
# .mso groff variant of .so (other search path)
$macro{'so'}= $macro{'mso'} = sub {
    warn wrap_mod("po4a::man", dgettext("po4a",
        "This page includes another file with '%s'. Do not forget to translate this file ('%s')."), $_[1], $_[2]);
    my $self = shift;
    $self->pushmacro(@_);
};
# .sp     Skip one line vertically.
# .sp N   Space  vertical distance N
$macro{'sp'}=\&untranslated;
# .vs [space]
# .vs +space
# .vs -space
# Change (increase, decrease) the vertical spacing by SPACE.  The
# default scaling indicator is `p'.
$macro{'vs'}=\&untranslated;
# .ta T N   Set tabs after every position that is a multiple of N.
# .ta n1 n2 ... nn T r1 r2 ... rn
#           Set  tabs at positions n1, n2, ..., nn, [...]
$macro{'ta'}=sub {
    # In some cases, a ta request can contain a translatable argument.
    # FIXME: detect those cases (something like 5i does not need to be
    # translated)
    my ($self,$m)=(shift,shift);
    my $line = "@_";
    $line =~ s/^ +//;
    $self->pushline($m." ".$self->translate($line,$self->{ref},'ta')."\n");
};
# .ti +N    Temporary indent next line (default scaling indicator m).
$macro{'ti'}=\&untranslated;


###
### tbl macros
###
$macro{'TS'}=sub {
    my $self=shift;
    my ($in_headers,$buffer)=(1,"");
    my ($line,$ref)=$self->shiftline();

    # Push table start
    $self->pushmacro(@_);
    while (defined($line)) {
        if ($line =~ /^\.TE/) {
            # Table end
            $self->pushline($self->r($line));
            return;
        }
        if ($in_headers) {
            if ($line =~ /\.$/) {
                $in_headers = 0;
            }
            $self->pushline($self->r($line));
        } elsif ($line =~ /\\$/) {
            # Lines are continued on \ at the end of line
            $buffer .= $line;
        } else {
            $buffer .= $line;
            # Arguments to translate are separated by \t
            $self->pushline(join("\t",
                                 map { $self->translate($buffer,
                                                        $ref,
                                                        'tbl table')
                                     } split (/\\t/,$line)));
            $buffer = "";
        }
        ($line,$ref)=$self->shiftline();
    }
};

###
### info groff
###

## Builtin register, of course they do not need to be translated

$macro{'F'}=$macro{'H'}=$macro{'V'}=$macro{'A'}=$macro{'T'}=\&untranslated;

## ms package
##
#
# Displays and keeps. None of these macro accept a translated argument
# (they allow to make blocks of text which cannot be broken by new page)

$macro{'DS'}=$macro{'LD'}=$macro{'DE'}=\&untranslated;
$macro{'ID'}=$macro{'BD'}=$macro{'CD'}=\&untranslated;
$macro{'RD'}=$macro{'KS'}=$macro{'KE'}=\&untranslated;
$macro{'KF'}=$macro{'B1'}=$macro{'B2'}=\&untranslated;
$macro{'DA'}=\&translate_joined;

# .pc c  Change page number character
$macro{'pc'}=\&translate_joined;

# .ns    Disable .sp and such
# .rs    Enable them again
$macro{'ns'}=$macro{'rs'}=\&untranslated;

# .cs font [width [em-size]]
# Switch to and from "constant glyph space mode".
$macro{'cs'}=\&untranslated;

# .ss word_space_size [sentence_space_size]
# Change the minimum size of a space between filled words.
$macro{'ss'}=\&untranslated;

# .ce     Center one line horizontally
# .ce N   Center N lines
# .ul N   Underline N lines (but not the spaces)
# .cu N   Underline N lines (even the spaces)
$macro{'ce'}=$macro{'ul'}=$macro{'cu'}=sub {
    my $self=shift;
    if (defined $_[1]) {
        if ($_[1] <= 0) {
            # disable centering, underlining, ...
            $self->pushmacro($_[0]);
        } else {
# All of these are not handled yet because the number of line may change
# during the translation
            die wrap_mod("po4a::man", dgettext("po4a",
                "This page uses the '%s' request with the number of lines in argument. This is not supported yet."), $_[0]);
        }
    } else {
        $self->pushmacro($_[0]);
    }
};

# .ec [c]
# Set the escape character to C.  With no argument the default
# escape character `\' is restored.  It can be also used to
# re-enable the escape mechanism after an `eo' request.
$macro{'ec'}=sub {
    my $self=shift;
    if (defined $_[1]) {
        die wrap_mod("po4a::man", dgettext("po4a",
            "This page uses the '%s' request. This request is only supported when no argument is provided."), $_[0]);
    } else {
        $self->pushmacro($_[0]);
    }
};


###
### BSD compatibility macros: .AT and .UC
### (define the version of Berkley used)
### FIXME: the header ("3rd Berkeley Distribution" or such) declared
###        by this macro isn't translatable we may want to remove
###        this from the generated manpage, and declare our own header
###
$macro{'UC'}=$macro{'AT'}=\&untranslated;

# Request: .hw word1 word2 ...
#   Define how WORD1, WORD2, etc. are to be hyphenated.  The words
#   must be given with hyphens at the hyphenation points.
#
#   If the English page needs to specify how a word must be hyphenated, the
#   translated page may also have this need.
$macro{'hw'}=\&translate_each;


#############################################################################
#
# mdoc macros
#
# The macros are defined in mdoc(7) and groff_mdoc(7)
#
# TBC: Should the font processing be disabled in the mdoc mode?
#############################################################################
# FIXME: Maybe we should verify that the page is an mdoc page
#        (add a flag in Dd, and always check that this flag is set in the
#        other mdoc macros)
sub translate_mdoc {
    my ($self,$macroname)=(shift,shift);
    my $macroarg = "";
    foreach (@_) {
        $macroarg.=" " if (length $macroarg);
        if ($_ =~ m/((?<!\\) |\t|^$)/) {
            $macroarg.="\"$_\"";
        } else {
            $macroarg.=$_;
        }
    }

    $self->pushline("$macroname ".$self->t($macroarg)."\n");
}
sub translate_mdoc_no_quotes {
    my ($self,$macroname, $macroarg)=(shift,shift, join(" ", @_));

    $self->pushline("$macroname ".$self->t($macroarg)."\n");
}
#
# Title Macros
# ============
# .Dd   Month day, year                       Document date.
$macro{'Dd'}=sub {
    my ($self,$macroname,$macroarg)=(shift,shift,join(" ",@_));

    $mdoc_mode = 1;
    $self->push_docheader();

# FIXME: It would be nice if we could switch from one set of macros to the
# other.
#
# This does not work at this time. If we erase the current set of macros,
# po4a fails when a configuration file uses both mdoc and groff pages.
#
#    # Erase the current macro definitions
#    %macro=();
#    %inline=();
#    %no_wrap_begin=();
#    %no_wrap_end=();
    # Use the mdoc macros
    define_mdoc_macros();

    $self->translate_mdoc_no_quotes($macroname,$macroarg);
};

sub define_mdoc_macros {
    # .Dt   DOCUMENT_TITLE [section] [volume]     Title, in upper case.
    $macro{'Dt'}=\&translate_mdoc;
    # .Os   OPERATING_SYSTEM [version/release]    Operating system (BSD).
    $macro{'Os'}=\&translate_each;
    # Keep the quotes e.g. finger.1
    # Don't add quotes e.g. logger.1

    # Page Layout Macros
    # ==================
    # .Sh   Section Headers.
    # (man mdoc indicates only a limited set of valid headers,
    # but it should be OK to translate the header)
    $macro{'Sh'}= sub {
        my ($self,$macroname)=(shift,shift);
        my $macroarg = "";
        foreach (@_) {
            $macroarg.=" " if (length $macroarg);
            if ($_ =~ m/((?<!\\) |\t|^$)/) {
                $macroarg.="\"$_\"";
            } else {
                $macroarg.=$_;
            }
        }
        if ($mdoc{$macroarg}) {
            $self->pushline("$macroname ".$self->r($macroarg)."\n");
        } else {
            $self->pushline("$macroname ".$self->t($macroarg)."\n");
        }
    };
    # .Ss   Subsection Headers.
    $macro{'Ss'}=\&translate_mdoc;
    # .Pp   Paragraph Break.  Vertical space (one line).
    $macro{'Pp'}=\&noarg;
    # .Lp   Same as .Pp
    $macro{'Lp'}=\&noarg;
    # .D1   (D-one) Display-one Indent and display one text line.
    $macro{'D1'}=\&translate_mdoc;
    # .Dl   (D-ell) Display-one literal.
    #       Indent and display one line of literal text
    $macro{'Dl'}=\&translate_mdoc;
    # .Bd   Begin-display block.
    # FIXME: Note: there are some options, some of the options argument
    #        may be translatable (-file <name>, -offset <string>)
    $no_wrap_begin{'Bd'} = 1;
    # .Ed   End-display (matches .Bd).
    $no_wrap_end{'Ed'} = 1;
    # .Bl   Begin-list.  Create lists or columns.
    # FIXME: As for .Bd, there are some options
    $macro{'Bl'}=\&untranslated;
    # .El   End-list.
    $macro{'El'}=\&noarg;
    # .It   List item.
    # FIXME: Maybe we could extract other modifiers
    #        as in .It Fl l Ar num
    $macro{'It'}=\&translate_mdoc;
    # .Lk   html link
    $macro{'Lk'}=\&untranslated;

    # Manual Domain Macros
    # ====================
    # FIXME: I think most Manual and General text domain are in the inline category
    foreach (qw(Ad An Ar Cd Cm Dv Er Ev Fa Fd Fn Ic Li Nm Op Ot Pa St Va Vt Xr)) {
        $inline{$_} = 1;
    }
    # FIXME: some of these macros introduce a line in bold.
    #        Using \fP in these line is not supported.
    #        do_fonts should be called for every inline line

    # General Text Domain
    # ===================
    foreach (qw(%A %B %C %D %I %J %N %O %P %Q %R %T %U %V
                Ac Ao Ap Aq At Bc Bf Bo Bq Brc Bro Brq Bx Db Dc Do Dq Ec Ef Em Eo Eq Fx No Ns
                Pc Pf Po Pq Qc Ql Qo Qq Re Rs Rv Sc So Sq Sm Sx Sy Tn Ux Xc Xo)) {
        $inline{$_} = 1;
    }

    # FIXME: Maybe it should be joined with the preceding .Nm
    $macro{'Nd'}=\&translate_mdoc;

    # Command line flags
    $inline{'Fl'} = 1;
    # Exit status
    $inline{'Ex'} = 1;
    # Opening option bracket
    $inline{'Oo'} = 1;
    # Closing option bracket
    $inline{'Oc'} = 1;
    # Begin keep (keep words in the same line)
    $inline{'Bk'} = 1;
    # End keep
    $inline{'Ek'} = 1;
    # Library Names
    $inline{'Lb'} = 1;
    # Function Types
    $inline{'Ft'} = 1;
    # Function open (for functions with many arguments)
    $inline{'Fo'} = 1;
    # Function close
    $inline{'Fc'} = 1;
    # OpenBSD macro
    $inline{'Ox'} = 1;
    # BSD/OS Macro
    $inline{'Bsx'} = 1;
    # #include statements
    $macro{'In'} = \&translate_mdoc;
    # NetBSD Macro
    $inline{'Nx'} = 1;
    # Math symbol
    $inline{'Ms'} = 1;
    # Prints 'under development'
    $inline{'Ud'} = 1;

    # This macro is a groff macro. I don't know if ot is valid in an mdoc page.
    # But this is used in some pages and seems to work
    $macro{'br'}=\&noarg;

} # end of define_mdoc_macros
