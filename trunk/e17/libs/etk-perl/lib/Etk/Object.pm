package Etk::Object;
use strict;
require Etk;

use AutoLoader;

sub AUTOLOAD {
    our $AUTOLOAD;
#    print "Attempted call to: $AUTOLOAD\n";
    my ($func, $package);
    ($func = $AUTOLOAD) =~ s/(.*::)//;
    ($package = $AUTOLOAD) =~ s/::$func//;

#    print "DECODED AS Package=$package FUNCTION=$func\n";

   if ($func =~ /[a-z]/ && $func =~ /[A-Z]/) {
	    $func =~ s/([A-Z][a-z0-9]+)/lc($1) . "_"/eg; 
	    $func =~ s/_$//;

#	    print "\tTranslate to $func\n";
	    my $obj = shift;
#	    print "Object: $obj -> $func (@_)\n";

	    return $obj->$func(@_) if $obj->can($func);
	    die "Object $obj does not have method $func\n"; # fail with some noise
	    
    }

}


1;
__END__

