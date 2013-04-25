#!/usr/bin/perl -w

# Copyright (c) 2004, 2005 by Nicolas FRANÇOIS <nicolas.francois@centraliens.net>
#
# This file is part of po4a.
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
# along with po4a; if not, write to the Free Software
# Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#
########################################################################

=encoding UTF-8

=head1 NAME

Locale::Po4a::LaTeX - convert LaTeX documents and derivates from/to PO files

=head1 DESCRIPTION

The po4a (PO for anything) project goal is to ease translations (and more
interestingly, the maintenance of translations) using gettext tools on
areas where they were not expected like documentation.

Locale::Po4a::LaTeX is a module to help the translation of LaTeX documents into
other [human] languages. It can also be used as a base to build modules for
LaTeX-based documents.

This module contains the definitions of common LaTeX commands and
environments.

See the L<Locale::Po4a::TeX(3pm)|Locale::Po4a::TeX> manpage for the list
of recognized options.

=head1 SEE ALSO

L<Locale::Po4a::TeX(3pm)|Locale::Po4a::TeX>,
L<Locale::Po4a::TransTractor(3pm)|Locale::Po4a::TransTractor>,
L<po4a(7)|po4a.7>

=head1 AUTHORS

 Nicolas François <nicolas.francois@centraliens.net>

=head1 COPYRIGHT AND LICENSE

Copyright 2004, 2005 by Nicolas FRANÇOIS <nicolas.francois@centraliens.net>.

This program is free software; you may redistribute it and/or modify it
under the terms of GPL (see COPYING file).

=cut

package Locale::Po4a::LaTeX;

use 5.006;
use strict;
use warnings;

require Exporter;
use vars qw($VERSION @ISA @EXPORT);
$VERSION= $Locale::Po4a::TeX::VERSION;
@ISA= qw(Locale::Po4a::TeX);
@EXPORT= qw();

use Locale::Po4a::TeX;
use subs qw(&generic_command
            &parse_definition_file
            &register_generic_command
            &register_generic_environment);
*parse_definition_file         = \&Locale::Po4a::TeX::parse_definition_file;
*generic_command               = \&Locale::Po4a::TeX::generic_command;
*register_generic_command      = \&Locale::Po4a::TeX::register_generic_command;
*register_generic_environment  = \&Locale::Po4a::TeX::register_generic_environment;
use vars qw($RE_ESCAPE            $ESCAPE
            $no_wrap_environments
            %commands             %environments
            %separated_command    %separated_environment
            %command_parameters   %environment_parameters
            %env_separators
            @exclude_include);
*RE_ESCAPE                = \$Locale::Po4a::TeX::RE_ESCAPE;
*ESCAPE                   = \$Locale::Po4a::TeX::ESCAPE;
*no_wrap_environments     = \$Locale::Po4a::TeX::no_wrap_environments;
*commands                 = \%Locale::Po4a::TeX::commands;
*environments             = \%Locale::Po4a::TeX::environments;
*separated_command        = \%Locale::Po4a::TeX::separated_command;
*separated_environment    = \%Locale::Po4a::TeX::separated_environment;
*env_separators           = \%Locale::Po4a::TeX::env_separators;
*exclude_include          = \@Locale::Po4a::TeX::exclude_include;
*command_parameters       = \%Locale::Po4a::TeX::command_parameters;
*environment_parameters   = \%Locale::Po4a::TeX::environment_parameters;


# documentclass:
# Only read the documentclass in order to find some po4a directives.
# FIXME: The documentclass could contain translatable strings.
# Maybe it should be implemented as \include{}.
register_generic_command("*documentclass,[]{}");
# We use register_generic_command to define the number and types of
# parameters. The function is then overwritten:
$commands{'documentclass'} = sub {
    my $self = shift;
    my ($command,$variant,$args,$env) = (shift,shift,shift,shift);
    my $no_wrap = shift;

    # Only try to parse the file.  We don't want to fail or parse this file
    # if it is a standard documentclass.
    my $name = ($args->[0] eq '[')? $args->[3]: $args->[1];
    parse_definition_file($self, $name.".cls", 1);

    my ($t,@e) = generic_command($self,$command,$variant,$args,$env,$no_wrap);

    return ($t, @$env);
};

# LaTeX 2
# I chose not to translate files, counters, lengths
register_generic_command("*addcontentsline,{}{}{_}");
register_generic_command("address,{_}");           # lines are seperated by \\
register_generic_command("*addtocontents,{}{_}");
register_generic_command("*addtocounter,{}{}");
register_generic_command("*addtolength,{}{}");
register_generic_command("*addvspace,{}");
register_generic_command("alph,{}");               # another language may not want this alphabet
register_generic_command("arabic,{}");             # another language may not want an arabic numbering
register_generic_command("*author,{_}");           # authors are separated by \and
register_generic_command("bibitem,[]{}");
register_generic_command("*bibliographystyle,{}"); # BibTeX
register_generic_command("*bibliography,{}");      # BibTeX
register_generic_command("*centerline,{_}");
register_generic_command("*caption,[]{_}");
register_generic_command("cc,{_}");
register_generic_command("circle,[]{}");
register_generic_command("cite,[_]{}");
register_generic_command("cline,{}");
register_generic_command("closing,{_}");
register_generic_command("dashbox,{}");            # followed by a (w,h) argument
register_generic_command("date,{_}");
register_generic_command("*enlargethispage,{}");
register_generic_command("ensuremath,{_}");
register_generic_command("*fbox,{_}");
register_generic_command("fnsymbol,{}");
register_generic_command("*footnote,[]{_}");
register_generic_command("*footnotemark,[]");
register_generic_command("*footnotetext,[]{_}");
register_generic_command("frac,{_}{_}");
register_generic_command("*frame,{_}");
register_generic_command("*framebox,[][]{_}");     # There is another form in picture environment
register_generic_command("*hbox,{}");
register_generic_command("*hspace,[]{}");
register_generic_command("*hyphenation,{_}");      # Translators may wish to add/remove words
register_generic_command("*include,{}");
#register_generic_command("includeonly,{}");       # should not be supported for now
register_generic_command("*index,{_}");
register_generic_command("*input,{}");
register_generic_command("*item,[_]");
register_generic_command("*label,{}");
register_generic_command("lefteqn,{_}");
register_generic_command("line,");                 # The first argument is (x,y)
register_generic_command("*linebreak,[]");
register_generic_command("linethickness,{}");
register_generic_command("location,{_}");
register_generic_command("makebox,[][]{_}");       # There's another form in picture environment
register_generic_command("makelabels,{}");
register_generic_command("*markboth,[]{_}{_}");
register_generic_command("*markright,{_}");
register_generic_command("mathcal,{_}");
register_generic_command("mathop,{_}");
register_generic_command("mbox,{_}");
register_generic_command("multicolumn,{}{}{_}");
register_generic_command("multiput,");             # The first arguments are (x,y)(dx,dy)
register_generic_command("name,{_}");
register_generic_command("*newcommand,{}[][]{_}");
register_generic_command("*newcounter,{}[]");
register_generic_command("*newenvironment,{}[]{_}{_}");
register_generic_command("*newfont,{}{}");
register_generic_command("*newlength,{}");
register_generic_command("*newsavebox,{}");
register_generic_command("*newtheorem,{}{_}");     # Two forms, the optionnal arg is not the first one
register_generic_command("nocite,{}");
register_generic_command("nolinebreak,[]");
register_generic_command("*nopagebreak,[]");
register_generic_command("opening,{_}");
register_generic_command("oval,");                 # The first argument is (w,h)
register_generic_command("overbrace,{_}");
register_generic_command("overline,{_}");
register_generic_command("*pagebreak,[]");
register_generic_command("*pagenumbering,{_}");
register_generic_command("pageref,{}");
register_generic_command("*pagestyle,{}");
register_generic_command("*parbox,[][][]{}{_}");
register_generic_command("providecommand,{}[][]{_}");
register_generic_command("put,");                  # The first argument is (x,y)
register_generic_command("raisebox,{}[][]{_}");
register_generic_command("ref,{}");
register_generic_command("*refstepcounter,{}");
register_generic_command("*renewcommand,{}[][]{_}");
register_generic_command("*renewenvironment,{}[]{_}{_}");
register_generic_command("roman,{}");              # another language may not want a roman numbering
register_generic_command("rule,[]{}{}");
register_generic_command("savebox,{}");            # Optional arguments in 2nd & 3rd position
register_generic_command("sbox,{}{_}");
register_generic_command("*setcounter,{}{}");
register_generic_command("*setlength,{}{}");
register_generic_command("*settodepth,{}{_}");
register_generic_command("*settoheight,{}{_}");
register_generic_command("*settowidth,{}{_}");
register_generic_command("shortstack,[]{_}");
register_generic_command("signature,{_}");
register_generic_command("sqrt,[_]{_}");
register_generic_command("stackrel,{_}{_}");
register_generic_command("stepcounter,{}");
register_generic_command("*subfigure,[_]{_}");
register_generic_command("symbol,{_}");
register_generic_command("telephone,{_}");
register_generic_command("thanks,{_}");
register_generic_command("*thispagestyle,{}");
register_generic_command("*title,{_}");
register_generic_command("typeout,{_}");
register_generic_command("typein,[]{_}");
register_generic_command("twocolumn,[_]");
register_generic_command("underbrace,{_}");
register_generic_command("underline,{_}");
register_generic_command("*usebox,{}");
register_generic_command("usecounter,{}");
register_generic_command("*usepackage,[]{}");
register_generic_command("value,{}");
register_generic_command("vector,");             # The first argument is (x,y)
register_generic_command("vphantom,{_}");
register_generic_command("*vspace,[]{}");
register_generic_command("*vbox,{}");
register_generic_command("*vcenter,{}");

register_generic_command("*part,[_]{_}");
register_generic_command("*chapter,[_]{_}");
register_generic_command("*section,[_]{_}");
register_generic_command("*subsection,[_]{_}");
register_generic_command("*subsubsection,[_]{_}");
register_generic_command("*paragraph,[_]{_}");
register_generic_command("*subparagraph,[_]{_}");

register_generic_command("textrm,{_}");
register_generic_command("textit,{_}");
register_generic_command("emph,{_}");
register_generic_command("textmd,{_}");
register_generic_command("textbf,{_}");
register_generic_command("textup,{_}");
register_generic_command("textsl,{_}");
register_generic_command("textsf,{_}");
register_generic_command("textsc,{_}");
register_generic_command("texttt,{_}");
register_generic_command("textnormal,{_}");
register_generic_command("mathrm,{_}");
register_generic_command("mathsf,{_}");
register_generic_command("mathtt,{_}");
register_generic_command("mathit,{_}");
register_generic_command("mathnormal,{_}");
register_generic_command("mathversion,{}");

register_generic_command("*contentspage,");
register_generic_command("*tablelistpage,");
register_generic_command("*figurepage,");

register_generic_command("*PassOptionsToPackage,{}{}");

register_generic_command("*ifthenelse,{}{_}{_}");

# graphics
register_generic_command("*includegraphics,[]{}");
register_generic_command("*graphicspath,{}");
register_generic_command("*resizebox,{}{}{_}");
register_generic_command("*scalebox,{}{_}");
register_generic_command("*rotatebox,{}{_}");

# url
register_generic_command("UrlFont,{}");
register_generic_command("*urlstyle,{}");

# hyperref
register_generic_command("href,{}{_}");            # 1:URL
register_generic_command("url,{}");                # URL
register_generic_command("nolinkurl,{}");          # URL
register_generic_command("hyperbaseurl,{}");       # URL
register_generic_command("hyperimage,{}");         # URL
register_generic_command("hyperdef,{}{}{_}");      # 1:category, 2:name
register_generic_command("hyperref,{}{}{}{_}");    # 1:URL, 2:category, 3:name
register_generic_command("hyperlink,{}{_}");       # 1:name
register_generic_command("*hypersetup,{_}");
register_generic_command("hypertarget,{}{_}");     # 1:name
register_generic_command("autoref,{}");            # 1:label

register_generic_command("*selectlanguage,{}");

# color
register_generic_command("*definecolor,{}{}{}");
register_generic_command("*textcolor,{}{_}");
register_generic_command("*colorbox,{}{_}");
register_generic_command("*fcolorbox,{}{}{_}");
register_generic_command("*pagecolor,{_}");
register_generic_command("*color,{}");

# equations/theorems
register_generic_command("*qedhere,");
register_generic_command("*qedsymbol,");
register_generic_command("*theoremstyle,{}");
register_generic_command("*proclaim,{_}");
register_generic_command("*endproclaim,");
register_generic_command("*shoveleft,{_}");
register_generic_command("*shoveright,{_}");

# commands without arguments. This is better than untranslated or
# translate_joined because the number of arguments will be checked.
foreach (qw(a *appendix *backmatter backslash *baselineskip *baselinestretch bf
            *bigskip boldmath cal cdots *centering *cleardoublepage *clearpage
            ddots dotfill em flushbottom *footnotesize frenchspacing
            *frontmatter *glossary *hfill *hline hrulefill huge Huge indent it
            kill large Large LARGE ldots left linewidth listoffigures
            listoftables *mainmatter *makeatletter *makeglossary *makeindex
            *maketitle *medskip *newline *newpage noindent nonumber *normalsize
            not *null *onecolumn *par parindent *parskip *printindex protect ps
            pushtabs *qquad *quad raggedbottom raggedleft raggedright right rm
            sc scriptsize sf sl small *smallskip *startbreaks *stopbreaks
            *tableofcontents textwidth textheight tiny today tt unitlength
            vdots verb *vfill *vline *fussy *sloppy

            aleph hbar imath jmath ell wp Re Im prime nabla surd angle forall
            exists partial infty triangle Box Diamond flat natural sharp
            clubsuit diamondsuit heartsuit spadesuit dag ddag S P copyright
            pounds Delta ASCII

            rmfamily itshape mdseries bfseries upshape slshape sffamily scshape
            ttfamily *normalfont width height depth totalheight

            *fboxsep *fboxrule
            *itemi *itemii *itemiii *itemiv
            *theitemi *theitemii *theitemiii *theitemiv)) {
    register_generic_command("$_,");
}



# standard environments.
# FIXME: All these definitions should be re-checked
foreach (qw(abstract align align* cases center description displaymath document enumerate
            eqnarray eqnarray* equation equation* flushleft flushright footnotesize itemize
            letter lrbox multline multline* proof quotation quote
            sloppypar tabbing theorem titlepage
            trivlist verbatim verbatim* verse wrapfigure)) {
    register_generic_environment("$_,");
}
register_generic_environment("tabular,[]{}");
register_generic_environment("tabular*,{}{}");
register_generic_environment("tabularx,{}{}");
register_generic_environment("multicols,{}");
register_generic_environment("list,{_}{}");
register_generic_environment("array,[]{}");
register_generic_environment("figure,[]");
register_generic_environment("minipage,[]{}");
register_generic_environment("picture,{}{}");
register_generic_environment("table,[]");
register_generic_environment("thebibliography,{_}");


# Commands and environments with separators.

# & is the cell separator, \\ is the line separator
# '\' is escaped twice
$env_separators{'array'} =
  $env_separators{'tabular'} =
  $env_separators{'tabularx'} = "(?:&|\\\\\\\\|\\\\hline)";

$env_separators{'trivlist'} =
  $env_separators{'list'} =
  $env_separators{'description'} =
  $env_separators{'enumerate'} =
  $env_separators{'itemize'} = "\\\\item";

$env_separators{'thebibliography'} = "\\\\bibitem";

$env_separators{'displaymath'} =
  $env_separators{'eqnarray'} =
  $env_separators{'eqnarray*'} =
  $env_separators{'flushleft'} =
  $env_separators{'flushright'} =
  $env_separators{'center'} =
  $env_separators{'author{#1}'} =
  $env_separators{'title{#1}'} = "\\\\\\\\";

# tabbing

1;
