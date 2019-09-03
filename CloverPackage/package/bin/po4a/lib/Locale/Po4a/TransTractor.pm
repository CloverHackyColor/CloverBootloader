#!/usr/bin/perl -w

require Exporter;

package Locale::Po4a::TransTractor;
use DynaLoader;

use 5.006;
use strict;
use warnings;

use subs qw(makespace);
use vars qw($VERSION @ISA @EXPORT);
$VERSION="0.45";
@ISA = qw(DynaLoader);
@EXPORT = qw(new process translate
             read write readpo writepo
             getpoout setpoout get_out_charset);

# Try to use a C extension if present.
eval("bootstrap Locale::Po4a::TransTractor $VERSION");

use Carp qw(croak);
use Locale::Po4a::Po;
use Locale::Po4a::Common;

use File::Path; # mkdir before write

use Encode;
use Encode::Guess;

=encoding UTF-8

=head1 NAME

Locale::Po4a::TransTractor - generic trans(lator ex)tractor.

=head1 DESCRIPTION

The po4a (PO for anything) project goal is to ease translations (and more
interestingly, the maintenance of translations) using gettext tools on
areas where they were not expected like documentation.

This class is the ancestor of every po4a parser used to parse a document, to
search translatable strings, to extract them to a PO file and to replace them by
their translation in the output document.

More formally, it takes the following arguments as input:

=over 2

=item -

a document to translate;

=item -

a PO file containing the translations to use.

=back

As output, it produces:

=over 2

=item -

another PO file, resulting of the extraction of translatable strings from
the input document;

=item -

a translated document, with the same structure than the one in input, but
with all translatable strings replaced with the translations found in the
PO file provided in input.

=back

Here is a graphical representation of this:

   Input document --\                             /---> Output document
                     \                           /       (translated)
                      +-> parse() function -----+
                     /                           \
   Input PO --------/                             \---> Output PO
                                                         (extracted)

=head1 FUNCTIONS YOUR PARSER SHOULD OVERRIDE

=over 4

=item parse()

This is where all the work takes place: the parsing of input documents, the
generation of output, and the extraction of the translatable strings. This
is pretty simple using the provided functions presented in the section
B<INTERNAL FUNCTIONS> below. See also the B<SYNOPSIS>, which presents an
example.

This function is called by the process() function below, but if you choose
to use the new() function, and to add content manually to your document,
you will have to call this function yourself.

=item docheader()

This function returns the header we should add to the produced document,
quoted properly to be a comment in the target language.  See the section
B<Educating developers about translations>, from L<po4a(7)|po4a.7>, for what
it is good for.

=back

=cut

sub docheader {}

sub parse {}

=head1 SYNOPSIS

The following example parses a list of paragraphs beginning with "<p>". For the sake
of simplicity, we assume that the document is well formatted, i.e. that '<p>'
tags are the only tags present, and that this tag is at the very beginning
of each paragraph.

 sub parse {
   my $self = shift;

   PARAGRAPH: while (1) {
       my ($paragraph,$pararef)=("","");
       my $first=1;
       my ($line,$lref)=$self->shiftline();
       while (defined($line)) {
           if ($line =~ m/<p>/ && !$first--; ) {
               # Not the first time we see <p>.
               # Reput the current line in input,
               #  and put the built paragraph to output
               $self->unshiftline($line,$lref);

               # Now that the document is formed, translate it:
               #   - Remove the leading tag
               $paragraph =~ s/^<p>//s;

               #   - push to output the leading tag (untranslated) and the
               #     rest of the paragraph (translated)
               $self->pushline(  "<p>"
                               . $document->translate($paragraph,$pararef)
                               );

               next PARAGRAPH;
           } else {
               # Append to the paragraph
               $paragraph .= $line;
               $pararef = $lref unless(length($pararef));
           }

           # Reinit the loop
           ($line,$lref)=$self->shiftline();
       }
       # Did not get a defined line? End of input file.
       return;
   }
 }

Once you've implemented the parse function, you can use your document
class, using the public interface presented in the next section.

=head1 PUBLIC INTERFACE for scripts using your parser

=head2 Constructor

=over 4

=item process(%)

This function can do all you need to do with a po4a document in one
invocation. Its arguments must be packed as a hash. ACTIONS:

=over 3

=item a.

Reads all the PO files specified in po_in_name

=item b.

Reads all original documents specified in file_in_name

=item c.

Parses the document

=item d.

Reads and applies all the addenda specified

=item e.

Writes the translated document to file_out_name (if given)

=item f.

Writes the extracted PO file to po_out_name (if given)

=back

ARGUMENTS, beside the ones accepted by new() (with expected type):

=over 4

=item file_in_name (@)

List of filenames where we should read the input document.

=item file_in_charset ($)

Charset used in the input document (if it isn't specified, it will try
to detect it from the input document).

=item file_out_name ($)

Filename where we should write the output document.

=item file_out_charset ($)

Charset used in the output document (if it isn't specified, it will use
the PO file charset).

=item po_in_name (@)

List of filenames where we should read the input PO files from, containing
the translation which will be used to translate the document.

=item po_out_name ($)

Filename where we should write the output PO file, containing the strings
extracted from the input document.

=item addendum (@)

List of filenames where we should read the addenda from.

=item addendum_charset ($)

Charset for the addenda.

=back

=item new(%)

Create a new po4a document. Accepted options (but be in a hash):

=over 4

=item verbose ($)

Sets the verbosity.

=item debug ($)

Sets the debugging.

=back

=cut

sub process {
    ## Determine if we were called via an object-ref or a classname
    my $self = shift;

    ## Any remaining arguments are treated as initial values for the
    ## hash that is used to represent this object.
    my %params = @_;

    # Build the args for new()
    my %newparams = ();
    foreach (keys %params) {
        next if ($_ eq 'po_in_name' ||
                 $_ eq 'po_out_name' ||
                 $_ eq 'file_in_name' ||
                 $_ eq 'file_in_charset' ||
                 $_ eq 'file_out_name' ||
                 $_ eq 'file_out_charset' ||
                 $_ eq 'addendum' ||
                 $_ eq 'addendum_charset');
        $newparams{$_}=$params{$_};
    }

    $self->detected_charset($params{'file_in_charset'});
    $self->{TT}{'file_out_charset'}=$params{'file_out_charset'};
    if (defined($self->{TT}{'file_out_charset'}) and
        length($self->{TT}{'file_out_charset'})) {
        $self->{TT}{'file_out_encoder'} = find_encoding($self->{TT}{'file_out_charset'});
    }
    $self->{TT}{'addendum_charset'}=$params{'addendum_charset'};

    chdir $params{'srcdir'}
        if (defined $params{'srcdir'});
    foreach my $file (@{$params{'po_in_name'}}) {
        print STDERR "readpo($file)... " if $self->debug();
        $self->readpo($file);
        print STDERR "done.\n" if $self->debug()
    }
    foreach my $file (@{$params{'file_in_name'}}) {
        print STDERR "read($file)..." if $self->debug();
        $self->read($file);
        print STDERR "done.\n"  if $self->debug();
    }
    print STDERR "parse..." if $self->debug();
    $self->parse();
    print STDERR "done.\n" if $self->debug();
    foreach my $file (@{$params{'addendum'}}) {
        print STDERR "addendum($file)..." if $self->debug();
        $self->addendum($file) || die "An addendum failed\n";
        print STDERR "done.\n" if $self->debug();
    }
    chdir $params{'destdir'}
        if (defined $params{'destdir'});
    if (defined $params{'file_out_name'}) {
        print STDERR "write(".$params{'file_out_name'}.")... "
            if $self->debug();
        $self->write($params{'file_out_name'});
        print STDERR "done.\n" if $self->debug();
    }
    chdir $params{'srcdir'}
        if (defined $params{'srcdir'});
    if (defined $params{'po_out_name'}) {
        print STDERR "writepo(".$params{'po_out_name'}.")... "
             if $self->debug();
        $self->writepo($params{'po_out_name'});
        print STDERR "done.\n" if $self->debug();
    }
    chdir $params{'calldir'}
        if (defined $params{'calldir'});
    return $self;
}

sub new {
    ## Determine if we were called via an object-ref or a classname
    my $this = shift;
    my $class = ref($this) || $this;
    my $self = { };
    my %options=@_;
    ## Bless ourselves into the desired class and perform any initialization
    bless $self, $class;

    ## initialize the plugin
    # prevent the plugin from croaking on the options intended for Po.pm
    $self->{options}{'porefs'} = '';
    $self->{options}{'copyright-holder'} = '';
    $self->{options}{'msgid-bugs-address'} = '';
    $self->{options}{'package-name'} = '';
    $self->{options}{'package-version'} = '';
    # let the plugin parse the options and such
    $self->initialize(%options);

    ## Create our private data
    my %po_options;
    $po_options{'porefs'} = $self->{options}{'porefs'};
    $po_options{'copyright-holder'} = $options{'copyright-holder'};
    $po_options{'msgid-bugs-address'} = $options{'msgid-bugs-address'};
    $po_options{'package-name'} = $options{'package-name'};
    $po_options{'package-version'} = $options{'package-version'};

    # private data
    $self->{TT}=();
    $self->{TT}{po_in}=Locale::Po4a::Po->new(\%po_options);
    $self->{TT}{po_out}=Locale::Po4a::Po->new(\%po_options);
    # Warning, this is an array of array:
    #  The document is splited on lines, and for each
    #  [0] is the line content, [1] is the reference [2] the type
    $self->{TT}{doc_in}=();
    $self->{TT}{doc_out}=();
    if (defined $options{'verbose'}) {
        $self->{TT}{verbose}  =  $options{'verbose'};
    }
    if (defined $options{'debug'}) {
        $self->{TT}{debug}  =  $options{'debug'};
    }
    # Input document is in ascii until we prove the opposite (in read())
    $self->{TT}{ascii_input}=1;
    # We try not to use utf unless it's forced from the outside (in case the
    # document isn't in ascii)
    $self->{TT}{utf_mode}=0;

    return $self;
}

=back

=head2 Manipulating document files

=over 4

=item read($)

Add another input document at the end of the existing one. The argument is
the filename to read.

Please note that it does not parse anything. You should use the parse()
function when you're done with packing input files into the document.

=cut

#'
sub read() {
    my $self=shift;
    my $filename=shift
        or croak wrap_msg(dgettext("po4a", "Can't read from file without having a filename"));
    my $linenum=0;

    open INPUT,"<$filename"
        or croak wrap_msg(dgettext("po4a", "Can't read from %s: %s"), $filename, $!);
    while (defined (my $textline = <INPUT>)) {
        $linenum++;
        my $ref="$filename:$linenum";
        $textline =~ s/\r$//;
        my @entry=($textline,$ref);
        push @{$self->{TT}{doc_in}}, @entry;

        if (!defined($self->{TT}{'file_in_charset'})) {
            # Detect if this file has non-ascii characters
            if($self->{TT}{ascii_input}) {
                my $decoder = guess_encoding($textline);
                if (!ref($decoder) or $decoder !~ /Encode::XS=/) {
                    # We have detected a non-ascii line
                    $self->{TT}{ascii_input} = 0;
                    # Save the reference for future error message
                    $self->{TT}{non_ascii_ref} ||= $ref;
                }
            }
        }
    }
    close INPUT
        or croak wrap_msg(dgettext("po4a", "Can't close %s after reading: %s"), $filename, $!);

}

=item write($)

Write the translated document to the given filename.

=cut

sub write {
    my $self=shift;
    my $filename=shift
        or croak wrap_msg(dgettext("po4a", "Can't write to a file without filename"));

    my $fh;
    if ($filename eq '-') {
        $fh=\*STDOUT;
    } else {
        # make sure the directory in which we should write the localized file exists
        my $dir = $filename;
        if ($dir =~ m|/|) {
            $dir =~ s|/[^/]*$||;

            File::Path::mkpath($dir, 0, 0755) # Croaks on error
              if (length ($dir) && ! -e $dir);
        }
        open $fh,">$filename"
            or croak wrap_msg(dgettext("po4a", "Can't write to %s: %s"), $filename, $!);
    }

    map { print $fh $_ } $self->docheader();
    map { print $fh $_ } @{$self->{TT}{doc_out}};

    if ($filename ne '-') {
        close $fh or croak wrap_msg(dgettext("po4a", "Can't close %s after writing: %s"), $filename, $!);
    }

}

=back

=head2 Manipulating PO files

=over 4

=item readpo($)

Add the content of a file (which name is passed as argument) to the
existing input PO. The old content is not discarded.

=item writepo($)

Write the extracted PO file to the given filename.

=item stats()

Returns some statistics about the translation done so far. Please note that
it's not the same statistics than the one printed by msgfmt
--statistic. Here, it's stats about recent usage of the PO file, while
msgfmt reports the status of the file. It is a wrapper to the
Locale::Po4a::Po::stats_get function applied to the input PO file. Example
of use:

    [normal use of the po4a document...]

    ($percent,$hit,$queries) = $document->stats();
    print "We found translations for $percent\%  ($hit from $queries) of strings.\n";

=back

=cut

sub getpoout {
    return $_[0]->{TT}{po_out};
}
sub setpoout {
    $_[0]->{TT}{po_out} = $_[1];
}
sub readpo  {
    $_[0]->{TT}{po_in}->read($_[1]);
}
sub writepo {
    $_[0]->{TT}{po_out}->write( $_[1] );
}
sub stats   {
    return $_[0]->{TT}{po_in}->stats_get();
}

=head2 Manipulating addenda

=over 4

=item addendum($)

Please refer to L<po4a(7)|po4a.7> for more information on what addenda are,
and how translators should write them. To apply an addendum to the translated
document, simply pass its filename to this function and you are done ;)

This function returns a non-null integer on error.

=cut

# Internal function to read the header.
sub addendum_parse {
    my ($filename,$header)=shift;

    my ($errcode,$mode,$position,$boundary,$bmode,$content)=
        (1,"","","","","");

    unless (open (INS, "<$filename")) {
        warn wrap_msg(dgettext("po4a", "Can't read from %s: %s"), $filename, $!);
        goto END_PARSE_ADDFILE;
    }

    unless (defined ($header=<INS>) && $header)  {
        warn wrap_msg(dgettext("po4a", "Can't read po4a header from %s."), $filename);
        goto END_PARSE_ADDFILE;
    }

    unless ($header =~ s/PO4A-HEADER://i) {
        warn wrap_msg(dgettext("po4a", "First line of %s does not look like a po4a header."), $filename);
        goto END_PARSE_ADDFILE;
    }
    foreach my $part (split(/;/,$header)) {
        unless ($part =~ m/^\s*([^=]*)=(.*)$/) {
            warn wrap_msg(dgettext("po4a", "Syntax error in po4a header of %s, near \"%s\""), $filename, $part);
            goto END_PARSE_ADDFILE;
        }
        my ($key,$value)=($1,$2);
        $key=lc($key);
        if ($key eq 'mode') {
            $mode=lc($value);
        } elsif ($key eq 'position') {
            $position=$value;
        } elsif ($key eq 'endboundary') {
            $boundary=$value;
            $bmode='after';
        } elsif ($key eq 'beginboundary') {
            $boundary=$value;
            $bmode='before';
        } else {
            warn wrap_msg(dgettext("po4a", "Invalid argument in the po4a header of %s: %s"), $filename, $key);
            goto END_PARSE_ADDFILE;
        }
    }

    unless (length($mode)) {
        warn wrap_msg(dgettext("po4a", "The po4a header of %s does not define the mode."), $filename);
        goto END_PARSE_ADDFILE;
    }
    unless ($mode eq "before" || $mode eq "after") {
        warn wrap_msg(dgettext("po4a", "Mode invalid in the po4a header of %s: should be 'before' or 'after' not %s."), $filename, $mode);
        goto END_PARSE_ADDFILE;
    }

    unless (length($position)) {
        warn wrap_msg(dgettext("po4a", "The po4a header of %s does not define the position."), $filename);
        goto END_PARSE_ADDFILE;
    }
    unless ($mode eq "before" || length($boundary)) {
        warn wrap_msg(dgettext("po4a", "No ending boundary given in the po4a header, but mode=after."));
        goto END_PARSE_ADDFILE;
    }

    while (defined(my $line = <INS>)) {
        $content .= $line;
    }
    close INS;

    $errcode=0;
  END_PARSE_ADDFILE:
      return ($errcode,$mode,$position,$boundary,$bmode,$content);
}

sub mychomp {
    my ($str) = shift;
    chomp($str);
    return $str;
}

sub addendum {
    my ($self,$filename) = @_;

    print STDERR "Apply addendum $filename..." if $self->debug();
    unless ($filename) {
        warn wrap_msg(dgettext("po4a",
            "Can't apply addendum when not given the filename"));
        return 0;
    }
    die wrap_msg(dgettext("po4a", "Addendum %s does not exist."), $filename)
      unless -e $filename;

    my ($errcode,$mode,$position,$boundary,$bmode,$content)=
        addendum_parse($filename);
    return 0 if ($errcode);

    print STDERR "mode=$mode;pos=$position;bound=$boundary;bmode=$bmode;ctn=$content\n"
      if $self->debug();

    # We only recode the addendum if an origin charset is specified, else we
    # suppose it's already in the output document's charset
    if (defined($self->{TT}{'addendum_charset'}) &&
        length($self->{TT}{'addendum_charset'})) {
        Encode::from_to($content,$self->{TT}{'addendum_charset'},
            $self->get_out_charset);
    }

    my $found = scalar grep { /$position/ } @{$self->{TT}{doc_out}};
    if ($found == 0) {
        warn wrap_msg(dgettext("po4a",
            "No candidate position for the addendum %s."), $filename);
        return 0;
    }
    if ($found > 1) {
        warn wrap_msg(dgettext("po4a",
            "More than one candidate position found for the addendum %s."), $filename);
        return 0;
    }

    if ($mode eq "before") {
        if ($self->verbose() > 1 || $self->debug() ) {
            map { print STDERR wrap_msg(dgettext("po4a", "Addendum '%s' applied before this line: %s"), $filename, $_) if (/$position/);
                } @{$self->{TT}{doc_out}};
        }
        @{$self->{TT}{doc_out}} = map { /$position/ ? ($content,$_) : $_
                                        }  @{$self->{TT}{doc_out}};
    } else {
        my @newres=();

        do {
            # make sure it doesn't whine on empty document
            my $line = scalar @{$self->{TT}{doc_out}} ? shift @{$self->{TT}{doc_out}} : "";
            push @newres,$line;
            my $outline=mychomp($line);
            $outline =~ s/^[ \t]*//;

            if ($line =~ m/$position/) {
                while ($line=shift @{$self->{TT}{doc_out}}) {
                    last if ($line=~/$boundary/);
                    push @newres,$line;
                }
                if (defined $line) {
                    if ($bmode eq 'before') {
                        print wrap_msg(dgettext("po4a",
                            "Addendum '%s' applied before this line: %s"),
                            $filename, $outline)
                          if ($self->verbose() > 1 || $self->debug());
                        push @newres,$content;
                        push @newres,$line;
                    } else {
                        print wrap_msg(dgettext("po4a",
                            "Addendum '%s' applied after the line: %s."),
                            $filename, $outline)
                          if ($self->verbose() > 1 || $self->debug());
                        push @newres,$line;
                        push @newres,$content;
                    }
                } else {
                    print wrap_msg(dgettext("po4a", "Addendum '%s' applied at the end of the file."), $filename)
                      if ($self->verbose() > 1 || $self->debug());
                    push @newres,$content;
                }
            }
        } while (scalar @{$self->{TT}{doc_out}});
        @{$self->{TT}{doc_out}} = @newres;
    }
    print STDERR "done.\n" if $self->debug();
    return 1;
}

=back

=head1 INTERNAL FUNCTIONS used to write derivated parsers

=head2 Getting input, providing output

Four functions are provided to get input and return output. They are very
similar to shift/unshift and push/pop. The first pair is about input, while
the second is about output. Mnemonic: in input, you are interested in the
first line, what shift gives, and in output you want to add your result at
the end, like push does.

=over 4

=item shiftline()

This function returns the next line of the doc_in to be parsed and its
reference (packed as an array).

=item unshiftline($$)

Unshifts a line of the input document and its reference.

=item pushline($)

Push a new line to the doc_out.

=item popline()

Pop the last pushed line from the doc_out.

=back

=cut

sub shiftline   {
    my ($line,$ref)=(shift @{$_[0]->{TT}{doc_in}},
                     shift @{$_[0]->{TT}{doc_in}});
    return ($line,$ref);
}
sub unshiftline {
        my $self = shift;
        unshift @{$self->{TT}{doc_in}},@_;
}

sub pushline    {  push @{$_[0]->{TT}{doc_out}}, $_[1] if defined $_[1]; }
sub popline     {  return pop @{$_[0]->{TT}{doc_out}};            }

=head2 Marking strings as translatable

One function is provided to handle the text which should be translated.

=over 4

=item translate($$$)

Mandatory arguments:

=over 2

=item -

A string to translate

=item -

The reference of this string (i.e. position in inputfile)

=item -

The type of this string (i.e. the textual description of its structural role;
used in Locale::Po4a::Po::gettextization(); see also L<po4a(7)|po4a.7>,
section B<Gettextization: how does it work?>)

=back

This function can also take some extra arguments. They must be organized as
a hash. For example:

  $self->translate("string","ref","type",
                   'wrap' => 1);

=over

=item B<wrap>

boolean indicating whether we can consider that whitespaces in string are
not important. If yes, the function canonizes the string before looking for
a translation or extracting it, and wraps the translation.

=item B<wrapcol>

the column at which we should wrap (default: 76).

=item B<comment>

an extra comment to add to the entry.

=back

Actions:

=over 2

=item -

Pushes the string, reference and type to po_out.

=item -

Returns the translation of the string (as found in po_in) so that the
parser can build the doc_out.

=item -

Handles the charsets to recode the strings before sending them to
po_out and before returning the translations.

=back

=back

=cut

sub translate {
    my $self=shift;
    my ($string,$ref,$type)=(shift,shift,shift);
    my (%options)=@_;

    # my $validoption="wrap wrapcol";
    # my %validoption;

    return "" unless defined($string) && length($string);

    # map { $validoption{$_}=1 } (split(/ /,$validoption));
    # foreach (keys %options) {
    #        Carp::confess "internal error: translate() called with unknown arg $_. Valid options: $validoption"
    #            unless $validoption{$_};
    # }

    my $in_charset;
    if ($self->{TT}{ascii_input}) {
        $in_charset = "ascii";
    } else {
        if (defined($self->{TT}{'file_in_charset'}) and
            length($self->{TT}{'file_in_charset'}) and
            $self->{TT}{'file_in_charset'} !~ m/ascii/i) {
            $in_charset=$self->{TT}{'file_in_charset'};
        } else {
            # FYI, the document charset have to be determined *before* we see the first
            # string to recode.
            die wrap_mod("po4a", dgettext("po4a", "Couldn't determine the input document's charset. Please specify it on the command line. (non-ASCII char at %s)"), $self->{TT}{non_ascii_ref})
        }
    }

    if ($self->{TT}{po_in}->get_charset ne "CHARSET") {
        $string = encode_from_to($string,
                                 $self->{TT}{'file_in_encoder'},
                                 $self->{TT}{po_in}{encoder});
    }

    if (defined $options{'wrapcol'} && $options{'wrapcol'} < 0) {
# FIXME: should be the parameter given with --width
        $options{'wrapcol'} = 76 + $options{'wrapcol'};
    }
    my $transstring = $self->{TT}{po_in}->gettext($string,
                                        'wrap'      => $options{'wrap'}||0,
                                        'wrapcol'   => $options{'wrapcol'});

    if ($self->{TT}{po_in}->get_charset ne "CHARSET") {
        my $out_encoder = $self->{TT}{'file_out_encoder'};
        unless (defined $out_encoder) {
            $out_encoder = find_encoding($self->get_out_charset)
        }
        $transstring = encode_from_to($transstring,
                                      $self->{TT}{po_in}{encoder},
                                      $out_encoder);
    }

    # If the input document isn't completely in ascii, we should see what to
    # do with the current string
    unless ($self->{TT}{ascii_input}) {
        my $out_charset = $self->{TT}{po_out}->get_charset;
        # We set the output po charset
        if ($out_charset eq "CHARSET") {
            if ($self->{TT}{utf_mode}) {
                $out_charset="UTF-8";
            } else {
                $out_charset=$in_charset;
            }
            $self->{TT}{po_out}->set_charset($out_charset);
        }
        if ( $in_charset !~ /^$out_charset$/i ) {
            Encode::from_to($string,$in_charset,$out_charset);
            if (defined($options{'comment'}) and length($options{'comment'})) {
                Encode::from_to($options{'comment'},$in_charset,$out_charset);
            }
        }
    }

    # the comments provided by the modules are automatic comments from the PO point of view
    $self->{TT}{po_out}->push('msgid'     => $string,
                              'reference' => $ref,
                              'type'      => $type,
                              'automatic' => $options{'comment'},
                              'wrap'      => $options{'wrap'}||0,
                              'wrapcol'   => $options{'wrapcol'});

#    if ($self->{TT}{po_in}->get_charset ne "CHARSET") {
#        Encode::from_to($transstring,$self->{TT}{po_in}->get_charset,
#            $self->get_out_charset);
#    }

    if ($options{'wrap'}||0) {
        $transstring =~ s/( *)$//s;
        my $trailing_spaces = $1||"";
        $transstring =~ s/(?<!\\) +$//gm;
        $transstring .= $trailing_spaces;
    }

    return $transstring;
}

=head2 Misc functions

=over 4

=item verbose()

Returns if the verbose option was passed during the creation of the
TransTractor.

=cut

sub verbose {
    if (defined $_[1]) {
        $_[0]->{TT}{verbose} = $_[1];
    } else {
        return $_[0]->{TT}{verbose} || 0; # undef and 0 have the same meaning, but one generates warnings
    }
}

=item debug()

Returns if the debug option was passed during the creation of the
TransTractor.

=cut

sub debug {
    return $_[0]->{TT}{debug};
}

=item detected_charset($)

This tells TransTractor that a new charset (the first argument) has been
detected from the input document. It can usually be read from the document
header. Only the first charset will remain, coming either from the
process() arguments or detected from the document.

=cut

sub detected_charset {
    my ($self,$charset)=(shift,shift);
    unless (defined($self->{TT}{'file_in_charset'}) and
            length($self->{TT}{'file_in_charset'}) ) {
        $self->{TT}{'file_in_charset'}=$charset;
        if (defined $charset) {
            $self->{TT}{'file_in_encoder'}=find_encoding($charset);
        } else {
            $self->{TT}{ascii_input}=1;
            $self->{TT}{utf_mode}=0;
        }
    }

    if (defined $self->{TT}{'file_in_charset'} and
        length $self->{TT}{'file_in_charset'} and
        $self->{TT}{'file_in_charset'} !~ m/ascii/i) {
        $self->{TT}{ascii_input}=0;
    }
}

=item get_out_charset()

This function will return the charset that should be used in the output
document (usually useful to substitute the input document's detected charset
where it has been found).

It will use the output charset specified in the command line. If it wasn't
specified, it will use the input PO's charset, and if the input PO has the
default "CHARSET", it will return the input document's charset, so that no
encoding is performed.

=cut

sub get_out_charset {
    my $self=shift;
    my $charset;

    # Use the value specified at the command line
    if (defined($self->{TT}{'file_out_charset'}) and
        length($self->{TT}{'file_out_charset'})) {
        $charset=$self->{TT}{'file_out_charset'};
    } else {
        if ($self->{TT}{utf_mode} && $self->{TT}{ascii_input}) {
            $charset="UTF-8";
        } else {
            $charset=$self->{TT}{po_in}->get_charset;
            $charset=$self->{TT}{'file_in_charset'}
                if $charset eq "CHARSET" and
                    defined($self->{TT}{'file_in_charset'}) and
                    length($self->{TT}{'file_in_charset'});
            $charset="ascii"
                if $charset eq "CHARSET";
        }
    }
    return $charset;
}

=item recode_skipped_text($)

This function returns the recoded text passed as argument, from the input
document's charset to the output document's one. This isn't needed when
translating a string (translate() recodes everything itself), but it is when
you skip a string from the input document and you want the output document to
be consistent with the global encoding.

=cut

sub recode_skipped_text {
    my ($self,$text)=(shift,shift);
    unless ($self->{TT}{'ascii_input'}) {
        if(defined($self->{TT}{'file_in_charset'}) and
            length($self->{TT}{'file_in_charset'}) ) {
            $text = encode_from_to($text,
                                   $self->{TT}{'file_in_encoder'},
                                   find_encoding($self->get_out_charset));
        } else {
            die wrap_mod("po4a", dgettext("po4a", "Couldn't determine the input document's charset. Please specify it on the command line. (non-ASCII char at %s)"), $self->{TT}{non_ascii_ref})
        }
    }
    return $text;
}


# encode_from_to($,$,$)
#
# Encode the given text from one encoding to another one.
# It differs from Encode::from_to because it does not take the name of the
# encoding in argument, but the encoders (as returned by the
# Encode::find_encoding(<name>) method). Thus it permits to save a bunch
# of call to find_encoding.
#
# If the "from" encoding is undefined, it is considered as UTF-8 (or
# ascii).
# If the "to" encoding is undefined, it is considered as UTF-8.
#
sub encode_from_to {
    my ($text,$from,$to) = (shift,shift,shift);

    if (not defined $from) {
        # for ascii and UTF-8, no conversion needed to get an utf-8
        # string.
    } else {
        $text = $from->decode($text, 0);
    }

    if (not defined $to) {
        # Already in UTF-8, no conversion needed
    } else {
        $text = $to->encode($text, 0);
    }

    return $text;
}

=back

=head1 FUTURE DIRECTIONS

One shortcoming of the current TransTractor is that it can't handle
translated document containing all languages, like debconf templates, or
.desktop files.

To address this problem, the only interface changes needed are:

=over 2

=item -

take a hash as po_in_name (a list per language)

=item -

add an argument to translate to indicate the target language

=item -

make a pushline_all function, which would make pushline of its content for
all language, using a map-like syntax:

    $self->pushline_all({ "Description[".$langcode."]=".
                          $self->translate($line,$ref,$langcode)
                        });

=back

Will see if it's enough ;)

=head1 AUTHORS

 Denis Barbier <barbier@linuxfr.org>
 Martin Quinson (mquinson#debian.org)
 Jordi Vilalta <jvprat@gmail.com>

=cut

1;
