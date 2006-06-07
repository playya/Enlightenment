package Etk::HSlider;
use strict;
use vars qw(@ISA);
require Etk::Range;
@ISA = ("Etk::Range");

sub new
{
    my $class = shift;
    my $self = $class->SUPER::new();
    $self->{WIDGET} = Etk::etk_hslider_new(shift, shift, shift, shift, shift);
    bless($self, $class);
    return $self;
}

1;
