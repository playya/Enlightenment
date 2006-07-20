package Etk::Tree;
use strict;
use vars qw(@ISA);
require Etk::Widget;
@ISA = ("Etk::Widget");

use Etk::Tree::Col;
use Etk::Tree::Row;
use Etk::Tree::Model;

use Etk::Tree::Model::Checkbox;
use Etk::Tree::Model::Double;
use Etk::Tree::Model::IconText;
use Etk::Tree::Model::Image;
use Etk::Tree::Model::Int;
use Etk::Tree::Model::ProgressBar;
use Etk::Tree::Model::Text;


sub new
{
    my $class = shift;
    my $self = $class->SUPER::new();
    $self->{WIDGET} = Etk::etk_tree_new();
    bless($self, $class);
    return $self;
}

sub TypeGet
{
    return Etk::etk_tree_type_get();
}

sub ModeSet
{
   my $self = shift;
   my $mode = shift;
   Etk::etk_tree_mode_set($self->{WIDGET}, $mode);
   return $self;
}

sub ModeGet
{
   my $self = shift;
   return Etk::etk_tree_mode_get($self->{WIDGET});
}

sub NumColsGet
{
   my $self = shift;
   return Etk::etk_tree_num_cols_get($self->{WIDGET});
}

sub NthColGet
{
   my $self = shift;
   my $nth = shift;
   return $self->{COLS}->[$nth];
}

sub HeadersVisibleSet
{
   my $self = shift;
   my $headers_visible = shift;
   Etk::etk_tree_headers_visible_set($self->{WIDGET}, $headers_visible);
   return $self;
}

sub HeadersVisibleGet
{
   my $self = shift;
   return Etk::etk_tree_headers_visible_get($self->{WIDGET});
}

sub Build
{
   my $self = shift;
   Etk::etk_tree_build($self->{WIDGET});
   return $self;
}

sub Freeze
{
   my $self = shift;
   Etk::etk_tree_freeze($self->{WIDGET});
   return $self;
}

sub Thaw
{
   my $self = shift;
   Etk::etk_tree_thaw($self->{WIDGET});
   return $self;
}

sub RowHeightSet
{
   my $self = shift;
   my $height = shift;
   Etk::etk_tree_row_height_set($self->{WIDGET}, $height);
   return $self;
}

sub RowHeightGet
{
   my $self = shift;
   return Etk::etk_tree_row_height_get($self->{WIDGET});
}

sub MultipleSelectSet
{
   my $self = shift;
   my $multiple_select = shift;
   Etk::etk_tree_multiple_select_set($self->{WIDGET}, $multiple_select);
   return $self;
}

sub MultipleSelectGet
{
   my $self = shift;
   return Etk::etk_tree_multiple_select_get($self->{WIDGET});
}

sub SelectAll
{
   my $self = shift;
   Etk::etk_tree_select_all($self->{WIDGET});
   return $self;
}

sub UnselectAll
{
   my $self = shift;
   Etk::etk_tree_unselect_all($self->{WIDGET});
   return $self;
}

# NOTE: since we cant transparently pass a variable number of arguments
# from perl to xs to c, we have to do things differently. This function
# will append an empty row, then we use perl model functions to set the
# data of the row's columns.
sub Append
{
    my $self = shift;
    my $row = Etk::Tree::Row->new();
    $row->{WIDGET} = Etk::etk_tree_append($self->{WIDGET});
    return $row;
}

# NOTE: this isnt working because of variable argument lists
sub Append2
{
    my $self = shift;
    my $row = Etk::Tree::Row->new();
    my @args;
    for my $arg (@_)
    {
	if($arg->isa("Etk::Object"))
	{
	    push @args, $arg->{WIDGET};
	}
	else
	{
	    push @args, $arg;
	}
    }
    $row->{WIDGET} = Etk::etk_tree_append($self->{WIDGET}, @args, undef);
    return $row;
}

sub AppendToRow
{
   my $self = shift;
   # TODO: figure out how to implement this
}

sub Clear
{
   my $self = shift;
   Etk::etk_tree_clear($self->{WIDGET});
   return $self;
}

sub Sort
{
    my $self = shift;
    my $callback = shift;
    my $asc = shift;
    my $col = shift;
    my $data = shift;
    Etk::etk_tree_sort($self->{WIDGET}, $callback, $asc, $col->{WIDGET}, 
	$data);
    return $self;
}

sub FirstRowGet
{
   my $self = shift;
   my $row = Etk::Tree::Row->new();
   $row->{WIDGET} = Etk::etk_tree_first_row_get($self->{WIDGET});
   return $row;
}

sub LastRowGet
{
   my $self = shift;
   my $row = Etk::Tree::Row->new();
   $row->{WIDGET} = Etk::etk_tree_last_row_get($self->{WIDGET});
   return $row;
}

sub SelectedRowGet
{
   my $self = shift;
   my $row = Etk::Tree::Row->new();
   $row->{WIDGET} = Etk::etk_tree_selected_row_get($self->{WIDGET});
   return $row;
}

sub SelectedRowsGet
{
   my $self = shift;
   return map {
	   my $widget = Etk::Tree::Row->new();
	   $widget->{WIDGET} = $_;
	   $_ = $widget;
   } Etk::etk_tree_selected_rows_get($self->{WIDGET});
}

sub AddCol
{
    my $self = shift;
    my ($title, $model, $width) = @_;

    my $data;
    my $model_widget;

    if (ref $model eq "ARRAY") {
	    $data = $model->[1];
	    $model = $model->[0];
    }
    
    if ($model eq "Text") { 
	    $model_widget = Etk::Tree::Model::Text->new($self);
    } elsif ($model eq "ProgressBar") {
	    $model_widget = Etk::Tree::Model::ProgressBar->new($self);
    } elsif ($model eq "IconText") {
	    $model_widget = Etk::Tree::Model::IconText->new($self, $data);
    } elsif ($model eq "Image") {
	    $model_widget = Etk::Tree::Model::Image->new($self, $data);
    } elsif ($model eq "Double") {
	    $model_widget = Etk::Tree::Model::Double->new($self);
    } elsif ($model eq "Checkbox") {
	    $model_widget = Etk::Tree::Model::Checkbox->new($self);
    } elsif ($model eq "Int") {
	    $model_widget = Etk::Tree::Model::Int->new($self);
    }

    my $widget = Etk::Tree::Col->new($self, $title, $model_widget, $width);
    $widget->{MODEL} = $model;
    
    push @{$self->{COLS}}, $widget;
    return $widget;

}

sub AddCols
{
    my $self = shift;
    my @cols = @_;
    foreach (@cols) {
	    $self->AddCol(@$_);
    }
    return $self;
}

sub AddRow
{
    my $self = shift;
    my @data = @_;
    my @cols = @{$self->{COLS}};
    my $row = $self->Append();
    foreach my $col (@cols) {
	if ($col->{MODEL} eq "Text") {
		my $text = shift @data;
		$row->FieldTextSet($col, $text);
	} elsif ($col->{MODEL} eq "ProgressBar") {
		my $prog = shift @data;
		$row->FieldProgressBarSet($col, $prog->[0], $prog->[1]);
	} elsif ($col->{MODEL} eq "IconText") {
		my $d = shift @data;
		if (@$d == 3) {
			$row->FieldIconEdjeTextSet($col, $d->[0], $d->[1], $d->[2]);
		} else {
			$row->FieldIconFileTextSet($col, $d->[0], $d->[1]);
		}
	} elsif ($col->{MODEL} eq "Image") {
		my $path = shift @data;
		$row->FieldImageFileSet($col, $path);
	} elsif ($col->{MODEL} eq "Double") {
		my $value = shift @data;
		$row->FieldDoubleSet($col, $value);
	} elsif ($col->{MODEL} eq "Checkbox") {
		my $checked = shift @data;
		$row->FieldCheckboxSet($col, $checked);
	} elsif ($col->{MODEL} eq "Int") {
		my $value = shift @data;
		$row->FieldIntSet($col, $value);
	}
    }
    return $row;
}
   
sub AddRows
{
    my $self = shift;
    my @rows = @_;
    foreach (@rows) {
	    $self->AddRow(@$_);
    }
    return $self;
}

sub cols
{
    my $self = shift;
    return $self->{COLS};
}
   
1;
