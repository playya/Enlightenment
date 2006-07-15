use strict;
use Etk;
use Etk::Tree::Model::Text;
use Etk::Tree::Model::ProgressBar;
use Etk::Tree::Col;

my $win = Etk::Window->new();
my $button1 = $win->AddButton("click me!"); 
my $button2 = $win->AddButton("click me too!");
my $vbox = $win->AddVBox(0, 0);
my $tree = $win->AddTree();
my $col1 = Etk::Tree::Col->new($tree, "Col 1",
    Etk::Tree::Model::Text->new($tree), 90);
my $col2 = Etk::Tree::Col->new($tree, "Col 2",
    Etk::Tree::Model::ProgressBar->new($tree), "90");

$tree->SizeRequestSet(320, 400);
$tree->Build();

my $row1 = $tree->Append();
$row1->FieldTextSet($col1, "Weee!");
$row1->FieldProgressBarSet($col2, 0.5, " Loading ... ");

my $row2 = $tree->Append();
$row2->FieldTextSet($col1, "Second line");
$row2->FieldProgressBarSet($col2, 0.2, " Reading ... ");

$vbox->PackStart($button1, 1, 1, 5);
$vbox->PackStart($button2, 0, 0, 2);
$vbox->PackStart($tree, 0, 0, 0);

$win->Add($vbox);
$win->ShowAll();

$button1->SignalConnect("clicked", \&click_cb1, "click_cb1_data");
$button2->SignalConnect("clicked", \&click_cb2);
$win->SignalConnect("delete_event", \&quit_cb);

Etk::Main::Run();
Etk::Shutdown();

sub click_cb1
{
    my $data = shift;
    print "click_cb1! (data=$data)\n";
    my ($padding, $expand, $fill, $pack_end) = $vbox->ChildPackingGet($button1);
    print "padding = $padding, expand = $expand, fill = $fill, pack_end = $pack_end\n";
}

sub click_cb2
{
    print "click_cb2!\n";
}

sub quit_cb
{
    print "quit!\n";
    Etk::Main::Quit();
}
