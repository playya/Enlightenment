package Etk::Widget;
use strict;
use vars qw(@ISA);
require Etk::Object;
@ISA = ("Etk::Object");

use AutoLoader;

sub new
{
    my $class = shift;
    my $self = $class->SUPER::new();
    $self->{WIDGET} = undef;
    bless($self, $class);
    return $self;
}

sub NameSet
{
    my $self = shift;
    my $name = shift;
    Etk::etk_widget_name_set($self->{WIDGET}, $name);
    return $self;
}

sub NameGet
{
    my $self = shift;
    return Etk::etk_widget_name_get($self->{WIDGET});
}

sub ToplevelParentGet
{
    my $self = shift;
    use Etk::ToplevelWidget;
    my $return = Etk::ToplevelWidget->new();
    $return->{WIDGET} = Etk::etk_widget_toplevel_parent_get($self->{WIDGET});
    return $return;
}

sub ParentSet
{
    my $self = shift;
    my $parent = shift;
    Etk::etk_widget_parent_set($self->{WIDGET}, $parent->{WIDGET});
    return $self;
}

sub ParentGet 
{
    my $self = shift;
    my $parent = Etk::Widget->new();
    $parent->{WIDGET} = Etk::etk_widget_parent_get($self->{WIDGET});
    return $parent;
}

sub Show
{
    my $self = shift;
    Etk::etk_widget_show($self->{WIDGET});
    return $self;
}

sub ShowAll
{
    my $self = shift;
    Etk::etk_widget_show_all($self->{WIDGET});
    return $self;
}

sub Hide
{
    my $self = shift;
    Etk::etk_widget_hide($self->{WIDGET});
    return $self;
}

sub HideAll
{
    my $self = shift;
    Etk::etk_widget_hide_all($self->{WIDGET});
    return $self;
}

sub isVisible
{
    my $self = shift;
    return Etk::etk_widget_is_visible($self->{WIDGET});
}

sub VisibilityLockedSet
{
    my $self = shift;
    Etk::etk_widget_visibility_locked_set($self->{WIDGET}, shift);
    return $self;
}

sub VisibilityLockedGet
{
    my $self = shift;
    return Etk::etk_widget_visibility_locked_get($self->{WIDGET});
}

sub Raise
{
    my $self = shift;
    Etk::etk_widget_raise($self->{WIDGET});
    return $self;
}

sub Lower
{
    my $self = shift;
    Etk::etk_widget_lower($self->{WIDGET});
    return $self;
}

sub SizeRecalcQueue
{
    my $self = shift;
    Etk::etk_widget_size_recalc_queue($self->{WIDGET});
    return $self;
}

sub RedrawQueue
{
    my $self = shift;
    Etk::etk_widget_redraw_queue($self->{WIDGET});
    return $self;
}

sub SizeRequestSet
{
    my $self = shift;
    my $width = shift;
    my $height = shift;
    Etk::etk_widget_size_request_set($self->{WIDGET}, $width, $height);
    return $self;
}

sub SizeAllocate
{
    my $self = shift;
    my $geometry = shift; # hashref
    Etk::etk_widget_size_allocate($self->{WIDGET}, $geometry);
    return $self;
}

sub Enter
{
    my $self = shift;
    Etk::etk_widget_enter($self->{WIDGET});
    return $self;
}

sub Leave
{
    my $self = shift;
    Etk::etk_widget_leave($self->{WIDGET});
    return $self;
}

sub Focus
{
    my $self = shift;
    Etk::etk_widget_focus($self->{WIDGET});
    return $self;
}

sub Unfocus
{
    my $self = shift;
    Etk::etk_widget_unfocus($self->{WIDGET});
    return $self;
}

sub PassMouseEventsSet
{
    my $self = shift;
    Etk::etk_widget_pass_mouse_events_set($self->{WIDGET}, shift);
    return $self;
}

sub PassMouseEventsGet
{
    my $self = shift;
    return Etk::etk_widget_pass_mouse_events_get($self->{WIDGET});
}

sub DndDestSet
{
    my $self = shift;
    my $on = shift;
    Etk::etk_widget_dnd_dest_set($self->{WIDGET}, $on);
    return $self;
}

sub DndDestGet
{
    my $self = shift;
    return Etk::etk_widget_dnd_dest_get($self->{WIDGET});
}

sub DndDestWidgetsGet
{
    return map {
	my $widget = Etk::Widget->new();
	$widget->{WIDGET} = $_;
	$_ = $widget;
    } Etk::etk_widget_dnd_dest_widgets_get();

}

sub DndSourceSet
{
    my $self = shift;
    my $on = shift;
    Etk::etk_widget_dnd_source_set($self->{WIDGET}, $on);
    return $self;
}

sub DndSourceGet
{
    my $self = shift;
    return Etk::etk_widget_dnd_source_get($self->{WIDGET});
}

sub DndDragWidgetSet
{
    my $self = shift;
    my $drag = shift;
    Etk::etk_widget_dnd_drag_widget_set($self->{WIDGET}, $drag->{WIDGET});
    return $self;
}

sub DndDragWidgetGet
{
    my $self = shift;
    return Etk::etk_widget_dnd_drag_widget_get($self->{WIDGET});
}

sub DndDragDataSet
{
    # TODO
}

sub DndFilesGet
{
    my $self = shift;
    return Etk::etk_widget_dnd_files_get($self->{WIDGET});
}

sub DndTypesSet
{
    my $self = shift;
    my @types = @_;
    if (@types > 0) {
    	Etk::etk_widget_dnd_types_set($self->{WIDGET}, @types);
    }
    return $self;
}

sub DndTypesGet
{
    my $self = shift;
    return Etk::etk_widget_dnd_types_get($self->{WIDGET});
}

sub DndInternalGet
{
    my $self = shift;
    return Etk::etk_widget_dnd_internal_get($self->{WIDGET});
}

sub DndInternalSet
{
    my $self = shift;
    my $on = shift;
    Etk::etk_widget_dnd_internal_set($self->{WIDGET}, $on);
    return $self;
}

sub DragDrop
{
    my $self = shift;
    my $type = shift;
    my $data = shift;
    # TODO check what dnd has to offer and fix this
#    Etk::etk_widget_drag_drop($self->{WIDGET}, $type, $data);
}

sub DragMotion
{
    my $self = shift;
    Etk::etk_widget_drag_motion($self->{WIDGET});
    return $self;
}

sub DragEnter
{
    my $self = shift;
    Etk::etk_widget_drag_enter($self->{WIDGET});
    return $self;
}

sub DragLeave
{
    my $self = shift;
    Etk::etk_widget_drag_leave($self->{WIDGET});
    return $self;
}

sub DragBegin
{
    my $self = shift;
    Etk::etk_widget_drag_begin($self->{WIDGET});
    return $self;
}

sub DragEnd
{
    my $self = shift;
    Etk::etk_widget_drag_end($self->{WIDGET});
    return $self;
}

sub ThemeFileSet
{
    my $self = shift;
    my $theme_file = shift;
    Etk::etk_widget_theme_file_set($self->{WIDGET}, $theme_file);
    return $self;

}

sub ThemeFileGet
{
    my $self = shift;
    return Etk::etk_widget_theme_file_get($self->{WIDGET});

}

sub ThemeGroupSet
{
    my $self = shift;
    my $theme_group = shift;
    Etk::etk_widget_theme_group_set($self->{WIDGET}, $theme_group);
    return $self;
}

sub ThemeGroupGet
{
    my $self = shift;
    return Etk::etk_widget_theme_group_get($self->{WIDGET});
}

sub ThemeParentSet
{
    my $self = shift;
    my $parent = shift;
    Etk::etk_widget_theme_parent_set($self->{WIDGET}, $parent->{WIDGET});
    return $self;
}

sub ThemeParentGet
{
    my $self = shift;
    return Etk::etk_widget_theme_parent_get($self->{WIDGET});
}

sub GeometryGet
{
    my $self = shift;
    return Etk::etk_widget_geometry_get($self->{WIDGET});
}

sub InnerGeometryGet
{
    my $self = shift;
    return Etk::etk_widget_inner_geometry_get($self->{WIDGET});
}

sub HasEventObjectSet
{
    my $self = shift;
    my $has = shift;
    Etk::etk_widget_has_event_object_set($self->{WIDGET}, $has);
    return $self;
}

sub HasEventObjectGet
{
    my $self = shift;
    return Etk::etk_widget_has_event_object_get($self->{WIDGET});
}

sub RepeatMouseEventsSet
{
    my $self = shift;
    my $repeat = shift;
    Etk::etk_widget_repeat_mouse_events_set($self->{WIDGET}, $repeat);
    return $self;
}

sub RepeatMouseEventsGet
{
    my $self = shift;
    return Etk::etk_widget_repeat_mouse_events_get($self->{WIDGET});
}

sub KeyEventPropagationStop
{
    Etk::etk_widget_key_event_propagation_stop();
}


sub SwallowWidget
{
    my $self = shift;
    my $part = shift;
    my $widget = shift;
    return Etk::etk_widget_swallow_widget($self->{WIDGET}, $part, $widget->{WIDGET});
}

sub UnswallowWidget
{
    my $self = shift;
    my $widget = shift;
    Etk::etk_widget_unswallow_widget($self->{WIDGET}, $widget->{WIDGET});
    return $self;
}

sub IsSwallowingWidget
{
    my $self = shift;
    my $widget = shift;
    return Etk::etk_widget_is_swallowing_widget($self->{WIDGET}, $widget->{WIDGET});
}

sub IsSwallowed
{
    my $self = shift;
    return Etk::etk_widget_is_swallowed($self->{WIDGET});
}

sub ThemeObjectMinSizeCalc
{
    my $self = shift;
    return Etk::etk_widget_theme_object_min_size_calc($self->{WIDGET});
}

sub ThemeObjectSignalEmit
{
    my $self = shift;
    my $signal = shift;
    Etk::etk_widget_theme_object_signal_emit($self->{WIDGET}, $signal);
    return $self;
}

sub ThemeObjectPartTextSet
{
    my $self = shift;
    my $part = shift;
    my $text = shift;
    Etk::etk_widget_theme_object_part_text_set($self->{WIDGET}, $part, $text);
    return $self;
}

# This is just a start.
sub AUTOLOAD
{
    our $AUTOLOAD;

    my $package;
    ($package = $AUTOLOAD) =~ s/.*:://;

    if ($package =~ /^Add(.+)/) 
    {
	my $self = shift;
	my $p = $1;
    	my $return;

    	eval("use Etk::$p");
	die("Cannot load package Etk::$p - $@") if $@;
    	eval("\$return = Etk::${p}->new(\@_);");

	$self->Add($return);
	
    	return $return;
    }
}

sub PackStart
{
    my $self = shift;
    if (ref $self->{PARENT} && $self->{PARENT}->isa("Etk::Box")) {
	    $self->{PARENT}->PackStart($self, @_);
    } else {
	    warn("Parent is not a Box\n");
    }
    return $self;
}

sub PackEnd
{
    my $self = shift;
    if (ref $self->{PARENT} && $self->{PARENT}->isa("Etk::Box")) {
	    $self->{PARENT}->PackEnd($self, @_);
    } else {
	    warn("Parent is not a Box at PackEnd\n");
    }
    return $self;
}


1;

__END__

