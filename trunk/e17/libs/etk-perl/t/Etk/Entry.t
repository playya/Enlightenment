use Test::More 'no_plan'; # tests => 1;
use Etk;

my $b = Etk::Entry->new();

ok( defined $b, 	"Entry new()");
ok( $b->isa("Etk::Entry"),	"Class Check");

$b->PasswordModeSet(1);
is($b->PasswordModeGet(), 1, 	"Password set/get");

$b->TextSet("test");
is($b->TextGet(), "test", 	"Text set/get");

