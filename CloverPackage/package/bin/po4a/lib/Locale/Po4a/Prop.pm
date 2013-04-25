# Locale::Po4a::Prop -- Convert Java property and OSX/NeXTSTEP strings files
# to PO file, for translation.
#
# This program is free software; you may redistribute it and/or modify it
# under the terms of GPL (see COPYING).
#

############################################################################
# Modules and declarations
############################################################################

package Locale::Po4a::Prop;

use 5.006;
use strict;
use warnings;

require Exporter;

use vars qw(@ISA @EXPORT $AUTOLOAD);
@ISA = qw(Locale::Po4a::TransTractor);
@EXPORT = qw();

use Locale::Po4a::TransTractor;
use Locale::Po4a::Common;

my $debug=0;

sub initialize {
    my $self = shift;
    my %options = @_;

    $self->{options}{'wrap'}=0;

    foreach my $opt (keys %options) {
        if ($options{$opt}) {
            die wrap_mod("po4a::prop",
                dgettext("po4a", "Unknown option: %s"), $opt)
                unless exists $self->{options}{$opt};
            $self->{options}{$opt} = $options{$opt};
        }
    }
}
sub parse {
	my $self=shift;
	my ($line,$ref);
	my $par;

	LINE:
	($line,$ref)=$self->shiftline();

	while (defined($line)) {
		chomp($line);
		print STDERR  "begin:$line\n" if $debug;

		if ($line =~ m/("[^"]*")[^=]*=[^"]*"(.*)/) { # "
			my @paragraph = ();
			my $pre_text = $1;
			print STDERR  "  PreText=".$pre_text."\n" if $debug;
			my $quoted_text = $2;
			# The text for translation
			print STDERR  "  QuotedText=".$quoted_text."\n" if $debug;
		    if ($quoted_text !~ /";[^"]*/) {
				@paragraph = ("$quoted_text\n");
				do {
					($line, undef) = $self->shiftline();
					push @paragraph, $line;
				} while ($line !~ /";[^"]*/);
			}
			if (@paragraph) {
				$quoted_text = join('', @paragraph);
			}

		    # Remove the final ";
		    $quoted_text =~ s/"\s*;[^"]*$//;
		    # Translate the string
            $par = $self->translate($quoted_text, $ref, $pre_text,
                                    'wrap'=>$self->{options}{'wrap'});
			# Now push the result
            $self->pushline($pre_text .' = "'.$par."\";\n");
			print STDERR  "End of line containing \".\n" if $debug;
		}
        else
		{
			print STDERR "Other stuff\n" if $debug;
			$self->pushline("$line\n");
		}
		# Reinit the loop
		($line,$ref)=$self->shiftline();
	}
}

##############################################################################
# Module return value and documentation
##############################################################################

1;
__END__

=encoding UTF-8

=head1 NAME

Locale::Po4a::Prop - convert Java property and OSX/NeXTSTEP strings from/to PO files

=head1 DESCRIPTION

Locale::Po4a::Prop is a module to help the translation of Java property and
OSX/NeXTSTEP strings files into other [human] languages.

The module searches for lines of the following format and extracts the quoted
text:

"identificator"="text than can be translated";

NOTE: If the text is not quoted, it will be ignored.

=head1 SEE ALSO

L<Locale::Po4a::TransTractor(3pm)>, L<po4a(7)|po4a.7>

=head1 AUTHORS

 Yves Blusseau <aphc549cmv@snkmail.com>

=head1 COPYRIGHT AND LICENSE

Copyright 2012 by Yves Blusseau

This program is free software; you may redistribute it and/or modify it
under the terms of GPL (see the COPYING file).

=cut
