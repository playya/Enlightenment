/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

/*
    Concerning the EDC reference:

    The formatting for blocks and properties has been implemented as a table
    which is filled using ALIASES.
    For maximum flexibility I implemented them in the \@code/\@encode style,
    this means that missing one or changing the order most certainly cause
    formatting errors.

    \@block
        block name
    \@context
        code sample of the block
    \@description
        the block's description
    \@endblock

    \@property
        property name
    \@parameters
        property's parameter list
    \@effect
        the property description (lol)
    \@endproperty
*/

/**
 * @page edcref Edje Data Collection reference
 * An Edje Data Collection, it's a plain text file (normally identified with the
 * .edc extension),consisting of instructions for the Edje Compiler.
 *
 * The syntax for the edje data collection files follows a simple structure of
 * "blocks { .. }" that can contain "properties: ..", more blocks, or both.
 *
 * @author Andres Blanc (dresb) andresblanc@gmail.com
 *
 * <table class="edcref" border="0">
 */

#include "edje_cc.h"

static void st_images_image(void);

static void st_fonts_font(void);

static void st_data_item(void);
static void st_data_file(void);

static void ob_styles_style(void);
static void st_styles_style_name(void);
static void st_styles_style_base(void);
static void st_styles_style_tag(void);

static void ob_color_class(void);
static void st_color_class_name(void);
static void st_color_class_color(void);
static void st_color_class_color2(void);
static void st_color_class_color3(void);

static void ob_spectrum(void);
/*static void st_spectrum(void);*/
static void st_spectrum_name(void);
static void st_spectrum_color(void);

static void ob_collections(void);

static void ob_collections_group(void);
static void st_collections_group_name(void);
static void st_collections_group_script_only(void);
static void st_collections_group_alias(void);
static void st_collections_group_min(void);
static void st_collections_group_max(void);
static void st_collections_group_data_item(void);

static void ob_collections_group_script(void);

static void ob_collections_group_parts_part(void);
static void st_collections_group_parts_part_name(void);
static void st_collections_group_parts_part_type(void);
static void st_collections_group_parts_part_effect(void);
static void st_collections_group_parts_part_mouse_events(void);
static void st_collections_group_parts_part_repeat_events(void);
static void st_collections_group_parts_part_ignore_flags(void);
static void st_collections_group_parts_part_scale(void);
static void st_collections_group_parts_part_pointer_mode(void);
static void st_collections_group_parts_part_precise_is_inside(void);
static void st_collections_group_parts_part_use_alternate_font_metrics(void);
static void st_collections_group_parts_part_clip_to_id(void);
static void st_collections_group_parts_part_source(void);
static void st_collections_group_parts_part_source2(void);
static void st_collections_group_parts_part_source3(void);
static void st_collections_group_parts_part_source4(void);
static void st_collections_group_parts_part_dragable_x(void);
static void st_collections_group_parts_part_dragable_y(void);
static void st_collections_group_parts_part_dragable_confine(void);
static void st_collections_group_parts_part_dragable_events(void);
static void st_collections_group_parts_part_entry_mode(void);
static void st_collections_group_parts_part_multiline(void);

static void ob_collections_group_parts_part_description(void);
static void st_collections_group_parts_part_description_inherit(void);
static void st_collections_group_parts_part_description_state(void);
static void st_collections_group_parts_part_description_visible(void);
static void st_collections_group_parts_part_description_align(void);
static void st_collections_group_parts_part_description_fixed(void);
static void st_collections_group_parts_part_description_min(void);
static void st_collections_group_parts_part_description_max(void);
static void st_collections_group_parts_part_description_step(void);
static void st_collections_group_parts_part_description_aspect(void);
static void st_collections_group_parts_part_description_aspect_preference(void);
static void st_collections_group_parts_part_description_rel1_relative(void);
static void st_collections_group_parts_part_description_rel1_offset(void);
static void st_collections_group_parts_part_description_rel1_to(void);
static void st_collections_group_parts_part_description_rel1_to_x(void);
static void st_collections_group_parts_part_description_rel1_to_y(void);
static void st_collections_group_parts_part_description_rel2_relative(void);
static void st_collections_group_parts_part_description_rel2_offset(void);
static void st_collections_group_parts_part_description_rel2_to(void);
static void st_collections_group_parts_part_description_rel2_to_x(void);
static void st_collections_group_parts_part_description_rel2_to_y(void);
static void st_collections_group_parts_part_description_image_normal(void);
static void st_collections_group_parts_part_description_image_tween(void);
static void st_collections_group_parts_part_description_image_border(void);
static void st_collections_group_parts_part_description_image_middle(void);
static void st_collections_group_parts_part_description_fill_smooth(void);
static void st_collections_group_parts_part_description_fill_origin_relative(void);
static void st_collections_group_parts_part_description_fill_origin_offset(void);
static void st_collections_group_parts_part_description_fill_size_relative(void);
static void st_collections_group_parts_part_description_fill_size_offset(void);
static void st_collections_group_parts_part_description_fill_angle(void);
static void st_collections_group_parts_part_description_fill_spread(void);
static void st_collections_group_parts_part_description_fill_type(void);
static void st_collections_group_parts_part_description_color_class(void);
static void st_collections_group_parts_part_description_color(void);
static void st_collections_group_parts_part_description_color2(void);
static void st_collections_group_parts_part_description_color3(void);
static void st_collections_group_parts_part_description_text_text(void);
static void st_collections_group_parts_part_description_text_text_class(void);
static void st_collections_group_parts_part_description_text_font(void);
static void st_collections_group_parts_part_description_text_style(void);
static void st_collections_group_parts_part_description_text_size(void);
static void st_collections_group_parts_part_description_text_fit(void);
static void st_collections_group_parts_part_description_text_min(void);
static void st_collections_group_parts_part_description_text_max(void);
static void st_collections_group_parts_part_description_text_align(void);
static void st_collections_group_parts_part_description_text_source(void);
static void st_collections_group_parts_part_description_text_text_source(void);
static void st_collections_group_parts_part_description_text_elipsis(void);
static void st_collections_group_parts_part_description_gradient_type(void);
static void st_collections_group_parts_part_description_gradient_spectrum(void);
static void st_collections_group_parts_part_description_gradient_rel1_relative(void);
static void st_collections_group_parts_part_description_gradient_rel1_offset(void);
static void st_collections_group_parts_part_description_gradient_rel2_relative(void);
static void st_collections_group_parts_part_description_gradient_rel2_offset(void);

static void ob_collections_group_programs_program(void);
static void st_collections_group_programs_program_name(void);
static void st_collections_group_programs_program_signal(void);
static void st_collections_group_programs_program_source(void);
static void st_collections_group_programs_program_in(void);
static void st_collections_group_programs_program_action(void);
static void st_collections_group_programs_program_transition(void);
static void st_collections_group_programs_program_target(void);
static void st_collections_group_programs_program_after(void);

static void ob_collections_group_programs_program_script(void);

/*****/

New_Statement_Handler statement_handlers[] =
{
     {"images.image", st_images_image},
     {"fonts.font", st_fonts_font},
     {"data.item", st_data_item},
     {"data.file", st_data_file},
     {"styles.style.name", st_styles_style_name},
     {"styles.style.base", st_styles_style_base},
     {"styles.style.tag", st_styles_style_tag},
     {"color_classes.color_class.name", st_color_class_name},
     {"color_classes.color_class.color", st_color_class_color},
     {"color_classes.color_class.color2", st_color_class_color2},
     {"color_classes.color_class.color3", st_color_class_color3},
     /*{"spectra.spectrum", st_spectrum},*/
     {"spectra.spectrum.name", st_spectrum_name},
     {"spectra.spectrum.color", st_spectrum_color},
     {"collections.image", st_images_image}, /* dup */
     {"collections.images.image", st_images_image}, /* dup */
     {"collections.font", st_fonts_font}, /* dup */
     {"collections.fonts.font", st_fonts_font}, /* dup */
     {"collections.styles.style.name", st_styles_style_name}, /* dup */
     {"collections.styles.style.base", st_styles_style_base}, /* dup */
     {"collections.styles.style.tag", st_styles_style_tag}, /* dup */
     {"collections.color_classes.color_class.name", st_color_class_name}, /* dup */
     {"collections.color_classes.color_class.color", st_color_class_color}, /* dup */
     {"collections.color_classes.color_class.color2", st_color_class_color2}, /* dup */
     {"collections.color_classes.color_class.color3", st_color_class_color3}, /* dup */
     {"collections.group.name", st_collections_group_name},
     {"collections.group.script_only", st_collections_group_script_only},
     {"collections.group.alias", st_collections_group_alias},
     {"collections.group.min", st_collections_group_min},
     {"collections.group.max", st_collections_group_max},
     {"collections.group.data.item", st_collections_group_data_item},
     {"collections.group.image", st_images_image}, /* dup */
     {"collections.group.images.image", st_images_image}, /* dup */
     {"collections.group.font", st_fonts_font}, /* dup */
     {"collections.group.fonts.font", st_fonts_font}, /* dup */
     {"collections.group.styles.style.name", st_styles_style_name}, /* dup */
     {"collections.group.styles.style.base", st_styles_style_base}, /* dup */
     {"collections.group.styles.style.tag", st_styles_style_tag}, /* dup */
     {"collections.group.color_classes.color_class.name", st_color_class_name}, /* dup */
     {"collections.group.color_classes.color_class.color", st_color_class_color}, /* dup */
     {"collections.group.color_classes.color_class.color2", st_color_class_color2}, /* dup */
     {"collections.group.color_classes.color_class.color3", st_color_class_color3}, /* dup */
     {"collections.group.parts.image", st_images_image}, /* dup */
     {"collections.group.parts.images.image", st_images_image}, /* dup */
     {"collections.group.parts.font", st_fonts_font}, /* dup */
     {"collections.group.parts.fonts.font", st_fonts_font}, /* dup */
     {"collections.group.parts.styles.style.name", st_styles_style_name}, /* dup */
     {"collections.group.parts.styles.style.base", st_styles_style_base}, /* dup */
     {"collections.group.parts.styles.style.tag", st_styles_style_tag}, /* dup */
     {"collections.group.parts.color_classes.color_class.name", st_color_class_name}, /* dup */
     {"collections.group.parts.color_classes.color_class.color", st_color_class_color}, /* dup */
     {"collections.group.parts.color_classes.color_class.color2", st_color_class_color2}, /* dup */
     {"collections.group.parts.color_classes.color_class.color3", st_color_class_color3}, /* dup */
     {"collections.group.parts.part.name", st_collections_group_parts_part_name},
     {"collections.group.parts.part.type", st_collections_group_parts_part_type},
     {"collections.group.parts.part.effect", st_collections_group_parts_part_effect},
     {"collections.group.parts.part.mouse_events", st_collections_group_parts_part_mouse_events},
     {"collections.group.parts.part.repeat_events", st_collections_group_parts_part_repeat_events},
     {"collections.group.parts.part.ignore_flags", st_collections_group_parts_part_ignore_flags},
     {"collections.group.parts.part.scale", st_collections_group_parts_part_scale},
     {"collections.group.parts.part.pointer_mode", st_collections_group_parts_part_pointer_mode},
     {"collections.group.parts.part.precise_is_inside", st_collections_group_parts_part_precise_is_inside},
     {"collections.group.parts.part.use_alternate_font_metrics", st_collections_group_parts_part_use_alternate_font_metrics},
     {"collections.group.parts.part.clip_to", st_collections_group_parts_part_clip_to_id},
     {"collections.group.parts.part.source", st_collections_group_parts_part_source},
     {"collections.group.parts.part.source2", st_collections_group_parts_part_source2},
     {"collections.group.parts.part.source3", st_collections_group_parts_part_source3},
     {"collections.group.parts.part.source4", st_collections_group_parts_part_source4},
     {"collections.group.parts.part.dragable.x", st_collections_group_parts_part_dragable_x},
     {"collections.group.parts.part.dragable.y", st_collections_group_parts_part_dragable_y},
     {"collections.group.parts.part.dragable.confine", st_collections_group_parts_part_dragable_confine},
     {"collections.group.parts.part.dragable.events", st_collections_group_parts_part_dragable_events},
     {"collections.group.parts.part.entry_mode", st_collections_group_parts_part_entry_mode},
     {"collections.group.parts.part.multiline", st_collections_group_parts_part_multiline},
     {"collections.group.parts.part.image", st_images_image}, /* dup */
     {"collections.group.parts.part.images.image", st_images_image}, /* dup */
     {"collections.group.parts.part.font", st_fonts_font}, /* dup */
     {"collections.group.parts.part.fonts.font", st_fonts_font}, /* dup */
     {"collections.group.parts.part.styles.style.name", st_styles_style_name}, /* dup */
     {"collections.group.parts.part.styles.style.base", st_styles_style_base}, /* dup */
     {"collections.group.parts.part.styles.style.tag", st_styles_style_tag}, /* dup */
     {"collections.group.parts.part.color_classes.color_class.name", st_color_class_name}, /* dup */
     {"collections.group.parts.part.color_classes.color_class.color", st_color_class_color}, /* dup */
     {"collections.group.parts.part.color_classes.color_class.color2", st_color_class_color2}, /* dup */
     {"collections.group.parts.part.color_classes.color_class.color3", st_color_class_color3}, /* dup */
     {"collections.group.parts.part.description.inherit", st_collections_group_parts_part_description_inherit},
     {"collections.group.parts.part.description.state", st_collections_group_parts_part_description_state},
     {"collections.group.parts.part.description.visible", st_collections_group_parts_part_description_visible},
     {"collections.group.parts.part.description.align", st_collections_group_parts_part_description_align},
     {"collections.group.parts.part.description.fixed", st_collections_group_parts_part_description_fixed},
     {"collections.group.parts.part.description.min", st_collections_group_parts_part_description_min},
     {"collections.group.parts.part.description.max", st_collections_group_parts_part_description_max},
     {"collections.group.parts.part.description.step", st_collections_group_parts_part_description_step},
     {"collections.group.parts.part.description.aspect", st_collections_group_parts_part_description_aspect},
     {"collections.group.parts.part.description.aspect_preference", st_collections_group_parts_part_description_aspect_preference},
     {"collections.group.parts.part.description.rel1.relative", st_collections_group_parts_part_description_rel1_relative},
     {"collections.group.parts.part.description.rel1.offset", st_collections_group_parts_part_description_rel1_offset},
     {"collections.group.parts.part.description.rel1.to", st_collections_group_parts_part_description_rel1_to},
     {"collections.group.parts.part.description.rel1.to_x", st_collections_group_parts_part_description_rel1_to_x},
     {"collections.group.parts.part.description.rel1.to_y", st_collections_group_parts_part_description_rel1_to_y},
     {"collections.group.parts.part.description.rel2.relative", st_collections_group_parts_part_description_rel2_relative},
     {"collections.group.parts.part.description.rel2.offset", st_collections_group_parts_part_description_rel2_offset},
     {"collections.group.parts.part.description.rel2.to", st_collections_group_parts_part_description_rel2_to},
     {"collections.group.parts.part.description.rel2.to_x", st_collections_group_parts_part_description_rel2_to_x},
     {"collections.group.parts.part.description.rel2.to_y", st_collections_group_parts_part_description_rel2_to_y},
     {"collections.group.parts.part.description.image.normal", st_collections_group_parts_part_description_image_normal},
     {"collections.group.parts.part.description.image.tween", st_collections_group_parts_part_description_image_tween},
     {"collections.group.parts.part.description.image.image", st_images_image}, /* dup */
     {"collections.group.parts.part.description.image.images.image", st_images_image}, /* dup */
     {"collections.group.parts.part.description.image.border", st_collections_group_parts_part_description_image_border},
     {"collections.group.parts.part.description.image.middle", st_collections_group_parts_part_description_image_middle},
     {"collections.group.parts.part.description.fill.smooth", st_collections_group_parts_part_description_fill_smooth},
     {"collections.group.parts.part.description.fill.origin.relative", st_collections_group_parts_part_description_fill_origin_relative},
     {"collections.group.parts.part.description.fill.origin.offset", st_collections_group_parts_part_description_fill_origin_offset},
     {"collections.group.parts.part.description.fill.size.relative", st_collections_group_parts_part_description_fill_size_relative},
     {"collections.group.parts.part.description.fill.size.offset", st_collections_group_parts_part_description_fill_size_offset},
     {"collections.group.parts.part.description.fill.angle", st_collections_group_parts_part_description_fill_angle},
     {"collections.group.parts.part.description.fill.spread", st_collections_group_parts_part_description_fill_spread},
     {"collections.group.parts.part.description.fill.type", st_collections_group_parts_part_description_fill_type},
     {"collections.group.parts.part.description.color_class", st_collections_group_parts_part_description_color_class},
     {"collections.group.parts.part.description.color", st_collections_group_parts_part_description_color},
     {"collections.group.parts.part.description.color2", st_collections_group_parts_part_description_color2},
     {"collections.group.parts.part.description.color3", st_collections_group_parts_part_description_color3},
     {"collections.group.parts.part.description.text.text", st_collections_group_parts_part_description_text_text},
     {"collections.group.parts.part.description.text.text_class", st_collections_group_parts_part_description_text_text_class},
     {"collections.group.parts.part.description.text.font", st_collections_group_parts_part_description_text_font},
     {"collections.group.parts.part.description.text.style", st_collections_group_parts_part_description_text_style},
     {"collections.group.parts.part.description.text.size", st_collections_group_parts_part_description_text_size},
     {"collections.group.parts.part.description.text.fit", st_collections_group_parts_part_description_text_fit},
     {"collections.group.parts.part.description.text.min", st_collections_group_parts_part_description_text_min},
     {"collections.group.parts.part.description.text.max", st_collections_group_parts_part_description_text_max},
     {"collections.group.parts.part.description.text.align", st_collections_group_parts_part_description_text_align},
     {"collections.group.parts.part.description.text.source", st_collections_group_parts_part_description_text_source},
     {"collections.group.parts.part.description.text.text_source", st_collections_group_parts_part_description_text_text_source},
     {"collections.group.parts.part.description.text.font", st_fonts_font}, /* dup */
     {"collections.group.parts.part.description.text.fonts.font", st_fonts_font}, /* dup */
     {"collections.group.parts.part.description.text.elipsis", st_collections_group_parts_part_description_text_elipsis},
     {"collections.group.parts.part.description.gradient.type", st_collections_group_parts_part_description_gradient_type},
     {"collections.group.parts.part.description.gradient.rel1.relative", st_collections_group_parts_part_description_gradient_rel1_relative},
     {"collections.group.parts.part.description.gradient.rel1.offset", st_collections_group_parts_part_description_gradient_rel1_offset},
     {"collections.group.parts.part.description.gradient.rel2.relative", st_collections_group_parts_part_description_gradient_rel2_relative},
     {"collections.group.parts.part.description.gradient.rel2.offset", st_collections_group_parts_part_description_gradient_rel2_offset},
     {"collections.group.parts.part.description.gradient.spectrum", st_collections_group_parts_part_description_gradient_spectrum},
     {"collections.group.parts.part.description.images.image", st_images_image}, /* dup */
     {"collections.group.parts.part.description.font", st_fonts_font}, /* dup */
     {"collections.group.parts.part.description.fonts.font", st_fonts_font}, /* dup */
     {"collections.group.parts.part.description.styles.style.name", st_styles_style_name}, /* dup */
     {"collections.group.parts.part.description.styles.style.base", st_styles_style_base}, /* dup */
     {"collections.group.parts.part.description.styles.style.tag", st_styles_style_tag}, /* dup */
     {"collections.group.parts.part.description.color_classes.color_class.name", st_color_class_name}, /* dup */
     {"collections.group.parts.part.description.color_classes.color_class.color", st_color_class_color}, /* dup */
     {"collections.group.parts.part.description.color_classes.color_class.color2", st_color_class_color2}, /* dup */
     {"collections.group.parts.part.description.color_classes.color_class.color3", st_color_class_color3}, /* dup */
     {"collections.group.parts.part.description.programs.image", st_images_image}, /* dup */
     {"collections.group.parts.part.description.programs.images.image", st_images_image}, /* dup */
     {"collections.group.parts.part.description.programs.font", st_fonts_font}, /* dup */
     {"collections.group.parts.part.description.programs.fonts.font", st_fonts_font}, /* dup */
     {"collections.group.parts.part.description.programs.program.name", st_collections_group_programs_program_name}, /* dup */
     {"collections.group.parts.part.description.programs.program.signal", st_collections_group_programs_program_signal}, /* dup */
     {"collections.group.parts.part.description.programs.program.source", st_collections_group_programs_program_source}, /* dup */
     {"collections.group.parts.part.description.programs.program.in", st_collections_group_programs_program_in}, /* dup */
     {"collections.group.parts.part.description.programs.program.action", st_collections_group_programs_program_action}, /* dup */
     {"collections.group.parts.part.description.programs.program.transition", st_collections_group_programs_program_transition}, /* dup */
     {"collections.group.parts.part.description.programs.program.target", st_collections_group_programs_program_target}, /* dup */
     {"collections.group.parts.part.description.programs.program.after", st_collections_group_programs_program_after}, /* dup */
     {"collections.group.parts.part.description.program.name", st_collections_group_programs_program_name}, /* dup */
     {"collections.group.parts.part.description.program.signal", st_collections_group_programs_program_signal}, /* dup */
     {"collections.group.parts.part.description.program.source", st_collections_group_programs_program_source}, /* dup */
     {"collections.group.parts.part.description.program.in", st_collections_group_programs_program_in}, /* dup */
     {"collections.group.parts.part.description.program.action", st_collections_group_programs_program_action}, /* dup */
     {"collections.group.parts.part.description.program.transition", st_collections_group_programs_program_transition}, /* dup */
     {"collections.group.parts.part.description.program.target", st_collections_group_programs_program_target}, /* dup */
     {"collections.group.parts.part.description.program.after", st_collections_group_programs_program_after}, /* dup */
     {"collections.group.parts.part.programs.image", st_images_image}, /* dup */
     {"collections.group.parts.part.programs.images.image", st_images_image}, /* dup */
     {"collections.group.parts.part.programs.font", st_fonts_font}, /* dup */
     {"collections.group.parts.part.programs.fonts.font", st_fonts_font}, /* dup */
     {"collections.group.parts.part.programs.program.name", st_collections_group_programs_program_name}, /* dup */
     {"collections.group.parts.part.programs.program.signal", st_collections_group_programs_program_signal}, /* dup */
     {"collections.group.parts.part.programs.program.source", st_collections_group_programs_program_source}, /* dup */
     {"collections.group.parts.part.programs.program.in", st_collections_group_programs_program_in}, /* dup */
     {"collections.group.parts.part.programs.program.action", st_collections_group_programs_program_action}, /* dup */
     {"collections.group.parts.part.programs.program.transition", st_collections_group_programs_program_transition}, /* dup */
     {"collections.group.parts.part.programs.program.target", st_collections_group_programs_program_target}, /* dup */
     {"collections.group.parts.part.programs.program.after", st_collections_group_programs_program_after}, /* dup */
     {"collections.group.parts.part.program.name", st_collections_group_programs_program_name}, /* dup */
     {"collections.group.parts.part.program.signal", st_collections_group_programs_program_signal}, /* dup */
     {"collections.group.parts.part.program.source", st_collections_group_programs_program_source}, /* dup */
     {"collections.group.parts.part.program.in", st_collections_group_programs_program_in}, /* dup */
     {"collections.group.parts.part.program.action", st_collections_group_programs_program_action}, /* dup */
     {"collections.group.parts.part.program.transition", st_collections_group_programs_program_transition}, /* dup */
     {"collections.group.parts.part.program.target", st_collections_group_programs_program_target}, /* dup */
     {"collections.group.parts.part.program.after", st_collections_group_programs_program_after}, /* dup */
     {"collections.group.parts.programs.image", st_images_image}, /* dup */
     {"collections.group.parts.programs.images.image", st_images_image}, /* dup */
     {"collections.group.parts.programs.font", st_fonts_font}, /* dup */
     {"collections.group.parts.programs.fonts.font", st_fonts_font}, /* dup */
     {"collections.group.parts.programs.program.name", st_collections_group_programs_program_name}, /* dup */
     {"collections.group.parts.programs.program.signal", st_collections_group_programs_program_signal}, /* dup */
     {"collections.group.parts.programs.program.source", st_collections_group_programs_program_source}, /* dup */
     {"collections.group.parts.programs.program.in", st_collections_group_programs_program_in}, /* dup */
     {"collections.group.parts.programs.program.action", st_collections_group_programs_program_action}, /* dup */
     {"collections.group.parts.programs.program.transition", st_collections_group_programs_program_transition}, /* dup */
     {"collections.group.parts.programs.program.target", st_collections_group_programs_program_target}, /* dup */
     {"collections.group.parts.programs.program.after", st_collections_group_programs_program_after},
     {"collections.group.parts.program.name", st_collections_group_programs_program_name}, /* dup */
     {"collections.group.parts.program.signal", st_collections_group_programs_program_signal}, /* dup */
     {"collections.group.parts.program.source", st_collections_group_programs_program_source}, /* dup */
     {"collections.group.parts.program.in", st_collections_group_programs_program_in}, /* dup */
     {"collections.group.parts.program.action", st_collections_group_programs_program_action}, /* dup */
     {"collections.group.parts.program.transition", st_collections_group_programs_program_transition}, /* dup */
     {"collections.group.parts.program.target", st_collections_group_programs_program_target}, /* dup */
     {"collections.group.parts.program.after", st_collections_group_programs_program_after}, /* dup */
     {"collections.group.program.name", st_collections_group_programs_program_name}, /* dup */
     {"collections.group.program.signal", st_collections_group_programs_program_signal}, /* dup */
     {"collections.group.program.source", st_collections_group_programs_program_source}, /* dup */
     {"collections.group.program.in", st_collections_group_programs_program_in}, /* dup */
     {"collections.group.program.action", st_collections_group_programs_program_action}, /* dup */
     {"collections.group.program.transition", st_collections_group_programs_program_transition}, /* dup */
     {"collections.group.program.target", st_collections_group_programs_program_target}, /* dup */
     {"collections.group.program.after", st_collections_group_programs_program_after}, /* dup */
     {"collections.group.programs.program.name", st_collections_group_programs_program_name},
     {"collections.group.programs.program.signal", st_collections_group_programs_program_signal},
     {"collections.group.programs.program.source", st_collections_group_programs_program_source},
     {"collections.group.programs.program.in", st_collections_group_programs_program_in},
     {"collections.group.programs.program.action", st_collections_group_programs_program_action},
     {"collections.group.programs.program.transition", st_collections_group_programs_program_transition},
     {"collections.group.programs.program.target", st_collections_group_programs_program_target},
     {"collections.group.programs.program.after", st_collections_group_programs_program_after},
     {"collections.group.programs.image", st_images_image}, /* dup */
     {"collections.group.programs.images.image", st_images_image}, /* dup */
     {"collections.group.programs.font", st_fonts_font}, /* dup */
     {"collections.group.programs.fonts.font", st_fonts_font} /* dup */
};

New_Object_Handler object_handlers[] =
{
     {"images", NULL},
     {"fonts", NULL},
     {"data", NULL},
     {"styles", NULL},
     {"styles.style", ob_styles_style},
     {"color_classes", NULL},
     {"color_classes.color_class", ob_color_class},
     {"spectra", NULL},
     {"spectra.spectrum", ob_spectrum},
     {"collections", ob_collections},
     {"collections.images", NULL}, /* dup */
     {"collections.fonts", NULL}, /* dup */
     {"collections.styles", NULL}, /* dup */
     {"collections.styles.style", ob_styles_style}, /* dup */
     {"collections.color_classes", NULL}, /* dup */
     {"collections.color_classes.color_class", ob_color_class}, /* dup */
     {"collections.group", ob_collections_group},
     {"collections.group.data", NULL},
     {"collections.group.script", ob_collections_group_script},
     {"collections.group.images", NULL}, /* dup */
     {"collections.group.fonts", NULL}, /* dup */
     {"collections.group.styles", NULL}, /* dup */
     {"collections.group.styles.style", ob_styles_style}, /* dup */
     {"collections.group.color_classes", NULL}, /* dup */
     {"collections.group.color_classes.color_class", ob_color_class}, /* dup */
     {"collections.group.parts", NULL},
     {"collections.group.parts.images", NULL}, /* dup */
     {"collections.group.parts.fonts", NULL}, /* dup */
     {"collections.group.parts.styles", NULL}, /* dup */
     {"collections.group.parts.styles.style", ob_styles_style}, /* dup */
     {"collections.group.parts.color_classes", NULL}, /* dup */
     {"collections.group.parts.color_classes.color_class", ob_color_class}, /* dup */
     {"collections.group.parts.part", ob_collections_group_parts_part},
     {"collections.group.parts.part.dragable", NULL},
     {"collections.group.parts.part.images", NULL}, /* dup */
     {"collections.group.parts.part.fonts", NULL}, /* dup */
     {"collections.group.parts.part.styles", NULL}, /* dup */
     {"collections.group.parts.part.styles.style", ob_styles_style}, /* dup */
     {"collections.group.parts.part.color_classes", NULL}, /* dup */
     {"collections.group.parts.part.color_classes.color_class", ob_color_class}, /* dup */
     {"collections.group.parts.part.description", ob_collections_group_parts_part_description},
     {"collections.group.parts.part.description.rel1", NULL},
     {"collections.group.parts.part.description.rel2", NULL},
     {"collections.group.parts.part.description.image", NULL}, /* dup */
     {"collections.group.parts.part.description.image.images", NULL}, /* dup */
     {"collections.group.parts.part.description.fill", NULL},
     {"collections.group.parts.part.description.fill.origin", NULL},
     {"collections.group.parts.part.description.fill.size", NULL},
     {"collections.group.parts.part.description.text", NULL},
     {"collections.group.parts.part.description.text.fonts", NULL}, /* dup */
     {"collections.group.parts.part.description.images", NULL}, /* dup */
     {"collections.group.parts.part.description.fonts", NULL}, /* dup */
     {"collections.group.parts.part.description.styles", NULL}, /* dup */
     {"collections.group.parts.part.description.styles.style", ob_styles_style}, /* dup */
     {"collections.group.parts.part.description.gradient", NULL},
     {"collections.group.parts.part.description.gradient.rel1", NULL},
     {"collections.group.parts.part.description.gradient.rel2", NULL},
     {"collections.group.parts.part.description.color_classes", NULL}, /* dup */
     {"collections.group.parts.part.description.color_classes.color_class", ob_color_class}, /* dup */
     {"collections.group.parts.part.description.program", ob_collections_group_programs_program}, /* dup */
     {"collections.group.parts.part.description.program.script", ob_collections_group_programs_program_script}, /* dup */
     {"collections.group.parts.part.description.programs", NULL}, /* dup */
     {"collections.group.parts.part.description.programs.images", NULL}, /* dup */
     {"collections.group.parts.part.description.programs.fonts", NULL}, /* dup */
     {"collections.group.parts.part.description.programs.program", ob_collections_group_programs_program}, /* dup */
     {"collections.group.parts.part.description.programs.program.script", ob_collections_group_programs_program_script}, /* dup */
     {"collections.group.parts.part.description.script", ob_collections_group_script}, /* dup */
     {"collections.group.parts.part.program", ob_collections_group_programs_program}, /* dup */
     {"collections.group.parts.part.program.script", ob_collections_group_programs_program_script}, /* dup */
     {"collections.group.parts.part.programs", NULL}, /* dup */
     {"collections.group.parts.part.programs.images", NULL}, /* dup */
     {"collections.group.parts.part.programs.fonts", NULL}, /* dup */
     {"collections.group.parts.part.programs.program", ob_collections_group_programs_program}, /* dup */
     {"collections.group.parts.part.programs.program.script", ob_collections_group_programs_program_script}, /* dup */
     {"collections.group.parts.part.script", ob_collections_group_script}, /* dup */
     {"collections.group.parts.program", ob_collections_group_programs_program}, /* dup */
     {"collections.group.parts.program.script", ob_collections_group_programs_program_script}, /* dup */
     {"collections.group.parts.programs", NULL}, /* dup */
     {"collections.group.parts.programs.images", NULL}, /* dup */
     {"collections.group.parts.programs.fonts", NULL}, /* dup */
     {"collections.group.parts.programs.program", ob_collections_group_programs_program}, /* dup */
     {"collections.group.parts.programs.program.script", ob_collections_group_programs_program_script}, /* dup */
     {"collections.group.parts.script", ob_collections_group_script}, /* dup */
     {"collections.group.program", ob_collections_group_programs_program}, /* dup */
     {"collections.group.program.script", ob_collections_group_programs_program_script}, /* dup */
     {"collections.group.programs", NULL},
     {"collections.group.programs.images", NULL}, /* dup */
     {"collections.group.programs.fonts", NULL}, /* dup */
     {"collections.group.programs.program", ob_collections_group_programs_program},
     {"collections.group.programs.program.script", ob_collections_group_programs_program_script},
     {"collections.group.programs.script", ob_collections_group_script} /* dup */
};

/*****/

int
object_handler_num(void)
{
   return sizeof(object_handlers) / sizeof (New_Object_Handler);
}

int
statement_handler_num(void)
{
   return sizeof(statement_handlers) / sizeof (New_Object_Handler);
}

/*****/

/**
    @page edcref

    @block
        images
    @context
        images {
            image: "filename1.ext" COMP;
            image: "filename2.ext" LOSSY 99;
            ..
        }
    @description
        The "images" block is used to list each image file that will be used in
        the theme along with its compression method (if any).
        Besides the domcument's root, additional "images" blocks can be
        included inside other blocks, normally "collections", "group" and
        "part", easing mantienance of the file list when the theme is split
        among multiple files.
    @endblock

    @property
        image
    @parameters
        [image file] [compression method] (compression level)
    @effect
        Used to include each image file. The full path to the directory holding
        the images can be defined later with edje_cc's "-id" option.
        Compression methods:
        @li RAW: Uncompressed.
        @li COMP: Lossless compression.
        @li LOSSY [0-100]: Lossy comression with quality from 0 to 100.
    @endproperty
 */
static void
st_images_image(void)
{
   Edje_Image_Directory_Entry *img;
   int v;

   if (!edje_file->image_dir)
     edje_file->image_dir = mem_alloc(SZ(Edje_Image_Directory));
   img = mem_alloc(SZ(Edje_Image_Directory_Entry));
   img->entry = parse_str(0);
     {
	Evas_List *l;

	for (l = edje_file->image_dir->entries; l; l = l->next)
	  {
	     Edje_Image_Directory_Entry *limg;

	     limg = l->data;
	     if (!strcmp(limg->entry, img->entry))
	       {
		  free(img->entry);
		  free(img);
		  return;
	       }
	  }
     }
   edje_file->image_dir->entries = evas_list_append(edje_file->image_dir->entries, img);
   img->id = evas_list_count(edje_file->image_dir->entries) - 1;
   v = parse_enum(1,
		  "RAW", 0,
		  "COMP", 1,
		  "LOSSY", 2,
		  "USER", 3,
		  NULL);
   if (v == 0)
     {
	img->source_type = EDJE_IMAGE_SOURCE_TYPE_INLINE_PERFECT;
	img->source_param = 0;
     }
   else if (v == 1)
     {
	img->source_type = EDJE_IMAGE_SOURCE_TYPE_INLINE_PERFECT;
	img->source_param = 1;
     }
   else if (v == 2)
     {
	img->source_type = EDJE_IMAGE_SOURCE_TYPE_INLINE_LOSSY;
	img->source_param = 0;
     }
   else if (v == 3)
     {
	img->source_type = EDJE_IMAGE_SOURCE_TYPE_EXTERNAL;
	img->source_param = 0;
     }
   if (img->source_type != EDJE_IMAGE_SOURCE_TYPE_INLINE_LOSSY)
	check_arg_count(2);
   else
     {
	img->source_param = parse_int_range(2, 0, 100);
	check_arg_count(3);
     }
}

/**
    @page edcref

    @block
        fonts
    @context
        fonts {
            font: "filename1.ext" "fontname";
            font: "filename2.ext" "otherfontname";
            ..
        }
    @description
        The "fonts" block is used to list each font file with an alias used later
        in the theme. As with the "images" block, additional "fonts" blocks can
        be included inside other blocks.
    @endblock

    @property
        font
    @parameters
        [font filename] [font alias]
    @effect
        Defines each font "file" and "alias", the full path to the directory
        holding the font files can be defined with edje_cc's "-fd" option.
    @endproperty
 */
static void
st_fonts_font(void)
{
   Font *fn;
   Edje_Font_Directory_Entry *fnt;

   check_arg_count(2);

   if (!edje_file->font_dir)
     edje_file->font_dir = mem_alloc(SZ(Edje_Font_Directory));

   fn = mem_alloc(SZ(Font));
   fn->file = parse_str(0);
   fn->name = parse_str(1);
     {
	Evas_List *l;

	for (l = fonts; l; l = l->next)
	  {
	     Font *lfn;

	     lfn = l->data;
	     if (!strcmp(lfn->name, fn->name))
	       {
		  free(fn->file);
		  free(fn->name);
		  free(fn);
		  return;
	       }
	  }
     }
   fonts = evas_list_append(fonts, fn);

   if (edje_file->font_dir)
     {
	fnt = mem_alloc(SZ(Edje_Font_Directory_Entry));
	fnt->entry = mem_strdup(fn->name);
	edje_file->font_dir->entries = evas_list_append(edje_file->font_dir->entries, fnt);
     }
}

/**
    @page edcref
    @block
        data
    @context
        data {
            item: "arbitraryname" "arbitraryvalue";
            item: "othername" "othervalue";
            ..
        }
    @description
        The "data" block is used to pass arbitrary parameters from the theme to
        the application. Unlike the "images" and "fonts" blocks, additional
        "data" blocks can only be included inside the "group" block.
    @endblock

    @property
        item
    @parameters
        [parameter name] [parameter value]
    @effect
        Defines each additional parameter.
    @endproperty
 */
static void
st_data_item(void)
{
   Edje_Data *di;

   check_arg_count(2);

   di = mem_alloc(SZ(Edje_Data));
   di->key = parse_str(0);
   di->value = parse_str(1);
   edje_file->data = evas_list_append(edje_file->data, di);
}

/**
    @page edcref
    @block
        data
    @context
        data {
            file: "arbitraryname" "filename";
            file: "othername" "otherfilename";
            ..
        }
    @description
        The "data" block is used to pass arbitrary parameters from the theme to
        the application. Unlike the "images" and "fonts" blocks, additional
        "data" blocks can only be included inside the "group" block.
    @endblock

    @property
        file
    @parameters
        [parameter name] [parameter filename]
    @effect
        Defines each additional parameter.
    @endproperty
 */
static void
st_data_file(void)
{
   const char *data;
   const char *over;
   Edje_Data *di;
   char *filename;
   char *value;
   int fd;
   int i;
   struct stat buf;

   check_arg_count(2);

   di = mem_alloc(SZ(Edje_Data));
   di->key = parse_str(0);
   filename = parse_str(1);

   fd = open(filename, O_RDONLY | O_BINARY);
   if (fd < 0)
     {
        fprintf(stderr, "%s: Error. %s:%i when opening file \"%s\": \"%s\"\n",
                progname, file_in, line, filename, strerror(errno));
        exit(-1);
     }

   if (fstat(fd, &buf))
     {
        fprintf(stderr, "%s: Error. %s:%i when stating file \"%s\": \"%s\"\n",
                progname, file_in, line, filename, strerror(errno));
        exit(-1);
     }

   data = mmap(NULL, buf.st_size, PROT_READ, MAP_SHARED, fd, 0);
   if (data == MAP_FAILED)
     {
        fprintf(stderr, "%s: Error. %s:%i when mapping file \"%s\": \"%s\"\n",
                progname, file_in, line, filename, strerror(errno));
        exit(-1);
     }

   over = data;
   for (i = 0; i < buf.st_size; ++i, ++over)
     if (*over == '\0')
       {
          fprintf(stderr, "%s: Error. %s:%i file \"%s\" is a binary file.\n",
                  progname, file_in, line, filename);
          exit(-1);
       }

   value = malloc(sizeof (char) * buf.st_size + 1);
   snprintf(value, buf.st_size + 1, "%s", data);

   munmap((void*)data, buf.st_size);
   close(fd);

   di->value = value;
   edje_file->data = evas_list_append(edje_file->data, di);

   free(filename);
}

/**
    @page edcref
    @block
        color_classes
    @context
        color_classes {
            color_class {
                name:  "colorclassname";
                color:  [0-255] [0-255] [0-255] [0-255];
                color2: [0-255] [0-255] [0-255] [0-255];
                color3: [0-255] [0-255] [0-255] [0-255]
            }
            ..
        }
    @description
        The "color_classes" block contains a list of one or more "color_class"
        blocks. Each "color_class" allows the designer to name an arbitrary
        group of colors to be used in the theme, the application can use that
        name to alter the color values at runtime.
    @endblock
*/
static void
ob_color_class(void)
{
   Edje_Color_Class *cc;

   cc = mem_alloc(SZ(Edje_Color_Class));
   edje_file->color_classes = evas_list_append(edje_file->color_classes, cc);

   cc->r = 0;
   cc->g = 0;
   cc->b = 0;
   cc->a = 0;
   cc->r2 = 0;
   cc->g2 = 0;
   cc->b2 = 0;
   cc->a2 = 0;
   cc->r3 = 0;
   cc->g3 = 0;
   cc->b3 = 0;
   cc->a3 = 0;
}

/**
    @page edcref

    @property
        name
    @parameters
        [color class name]
    @effect
        Sets the name for the color class, used as reference by both the theme
        and the application.
    @endproperty
*/
static void
st_color_class_name(void)
{
   Edje_Color_Class *cc, *tcc;
   Evas_List *l;

   cc = evas_list_data(evas_list_last(edje_file->color_classes));
   cc->name = parse_str(0);
   for (l = edje_file->color_classes; l; l = l->next)
     {
	tcc = l->data;
	if ((cc != tcc) && (!strcmp(cc->name, tcc->name)))
	  {
	     fprintf(stderr, "%s: Error. parse error %s:%i. There is already a color class named \"%s\"\n",
		     progname, file_in, line - 1, cc->name);
	     exit(-1);
	  }
     }
}

/**
    @page edcref
    @property
        color
    @parameters
        [red] [green] [blue] [alpha]
    @effect
        The main color.
    @endproperty
*/
static void
st_color_class_color(void)
{
   Edje_Color_Class *cc;

   check_arg_count(4);

   cc = evas_list_data(evas_list_last(edje_file->color_classes));
   cc->r = parse_int_range(0, 0, 255);
   cc->g = parse_int_range(1, 0, 255);
   cc->b = parse_int_range(2, 0, 255);
   cc->a = parse_int_range(3, 0, 255);
}

/**
    @page edcref
    @property
        color2
    @parameters
        [red] [green] [blue] [alpha]
    @effect
        Used as shadow in text and textblock parts.
    @endproperty
*/
static void
st_color_class_color2(void)
{
   Edje_Color_Class *cc;

   check_arg_count(4);

   cc = evas_list_data(evas_list_last(edje_file->color_classes));
   cc->r2 = parse_int_range(0, 0, 255);
   cc->g2 = parse_int_range(1, 0, 255);
   cc->b2 = parse_int_range(2, 0, 255);
   cc->a2 = parse_int_range(3, 0, 255);
}

/**
    @page edcref
    @property
        color3
    @parameters
        [red] [green] [blue] [alpha]
    @effect
        Used as outline in text and textblock parts.
    @endproperty
*/
static void
st_color_class_color3(void)
{
   Edje_Color_Class *cc;

   check_arg_count(4);

   cc = evas_list_data(evas_list_last(edje_file->color_classes));
   cc->r3 = parse_int_range(0, 0, 255);
   cc->g3 = parse_int_range(1, 0, 255);
   cc->b3 = parse_int_range(2, 0, 255);
   cc->a3 = parse_int_range(3, 0, 255);
}

/**
    @page edcref
    @block
        spectra
    @context
        spectra {
            spectrum {
                name: "colorspectrumname";
                color: [0-255] [0-255] [0-255] [0-255] [0-?]
                color: [0-255] [0-255] [0-255] [0-255] [0-?]
                ..
            }
            ..
        }
    @description
        The "spectra" block contains a list of one or more "spectrum" blocks.
        Each "spectrum" block defines a color range used to fill GRADIENT
        parts. The colors are defined with the red, green, blue, alpha, delta
        format.
    @endblock
*/
static void
ob_spectrum(void)
{
   Edje_Spectrum_Directory_Entry *se;

   if (!edje_file->spectrum_dir)
     edje_file->spectrum_dir = mem_alloc(SZ(Edje_Spectrum_Directory));
   se = mem_alloc(SZ(Edje_Spectrum_Directory_Entry));
   edje_file->spectrum_dir->entries = evas_list_append(edje_file->spectrum_dir->entries, se);
   se->id = evas_list_count(edje_file->spectrum_dir->entries) - 1;
   se->entry = NULL;
   se->filename = NULL;
   se->color_list = NULL;
}

/**
    @page edcref
    @property
        name
    @parameters
        [spectrum name]
    @effect
        The name of the spectrum used as reference later in the theme.
    @endproperty
*/
static void
st_spectrum_name(void)
{
   Edje_Spectrum_Directory_Entry *se;

   se = evas_list_data(evas_list_last(edje_file->spectrum_dir->entries));
   se->entry = parse_str(0);
}

/**
    @page edcref
    @property
        color
    @parameters
        [red] [green] [blue] [alpha] [delta]
    @effect
        Each color declaration represents a stop point in the color range. The
        last parameter (delta) is used to set the proportion of a given stop
        point higher or lower in contrast with the other color's delta value.
    @endproperty
*/
static void
st_spectrum_color(void)
{
   Edje_Spectrum_Directory_Entry *se;
   Edje_Spectrum_Color *sc;

   se = evas_list_data(evas_list_last(edje_file->spectrum_dir->entries));

   sc = mem_alloc(SZ(Edje_Spectrum_Color));
   se->color_list = evas_list_append(se->color_list, sc);
   sc->r = parse_int_range(0, 0, 255);
   sc->g = parse_int_range(1, 0, 255);
   sc->b = parse_int_range(2, 0, 255);
   sc->a = parse_int_range(3, 0, 255);
   sc->d = parse_int(4);
}

/**
    @page edcref
    @block
        styles
    @context
        styles {
            style {
                name: "stylename";
                base: "..default style properties..";

                tag:  "tagname" "..style properties..";
                ..
            }
            ..
        }
    @description
        The "styles" block contains a list of one or more "style" blocks. A
        "style" block is used to create style \<tags\> for advanced TEXTBLOCK
        formatting.
    @endblock
*/
static void
ob_styles_style(void)
{
   Edje_Style *stl;

   stl = mem_alloc(SZ(Edje_Style));
   edje_file->styles = evas_list_append(edje_file->styles, stl);
}

/**
    @page edcref
    @property
        name
    @parameters
        [style name]
    @effect
        The name of  the style to be used as reference later in the theme.
    @endproperty
*/
static void
st_styles_style_name(void)
{
   Edje_Style *stl, *tstl;
   Evas_List *l;

   stl = evas_list_data(evas_list_last(edje_file->styles));
   stl->name = parse_str(0);
   for (l = edje_file->styles; l; l = l->next)
     {
	tstl = l->data;
	if ((stl != tstl) && (!strcmp(stl->name, tstl->name)))
	  {
	     fprintf(stderr, "%s: Error. parse error %s:%i. There is already a style named \"%s\"\n",
		     progname, file_in, line - 1, stl->name);
	     exit(-1);
	  }
     }
}

/**
    @page edcref
    @property
        base
    @parameters
        [style properties string]
    @effect
        The default style properties that will be applied to the complete
        text.
    @endproperty
*/
static void
st_styles_style_base(void)
{
   Edje_Style *stl;
   Edje_Style_Tag *tag;

   stl = evas_list_data(evas_list_last(edje_file->styles));
   if (stl->tags)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. There is already a basic format for the style\n",
		progname, file_in, line - 1);
	exit(-1);
     }
   tag = mem_alloc(SZ(Edje_Style_Tag));
   tag->key = mem_strdup("DEFAULT");
   tag->value = parse_str(0);
   stl->tags = evas_list_append(stl->tags, tag);
}

/**
    @page edcref
    @property
        tag
    @parameters
        [tag name] [style properties string]
    @effect
        Style to be applied only to text between style \<tags\>..\</tags\>.
    @endproperty
*/
static void
st_styles_style_tag(void)
{
   Edje_Style *stl;
   Edje_Style_Tag *tag;

   stl = evas_list_data(evas_list_last(edje_file->styles));
   tag = mem_alloc(SZ(Edje_Style_Tag));
   tag->key = parse_str(0);
   tag->value = parse_str(1);
   stl->tags = evas_list_append(stl->tags, tag);
}

/**
    @page edcref
    @block
        collections
    @context
        collections {
            ..
            group { }
            group { }
            ..
        }
    @description
        The "collections" block is used to list the groups that compose the
        theme. Additional "collections" blocks do not prevent overriding group
        names.
    @endblock
*/
static void
ob_collections(void)
{
   if (!edje_file->collection_dir)
     edje_file->collection_dir = mem_alloc(SZ(Edje_Part_Collection_Directory));
}

/**
    @page edcref
    @block
        group
    @context
        collections {
            ..
            group {
                name: "nameusedbytheapplication";
                alias: "anothername";
                min: width height;
                max: width height;

                data { }
                script { }
                parts { }
                programs { }
            }
            ..
        }
    @description
        A "group" block contains the list of parts and programs that compose a
        given Edje Object.
    @endblock
*/
static void
ob_collections_group(void)
{
   Edje_Part_Collection_Directory_Entry *de;
   Edje_Part_Collection *pc;
   Code *cd;
   
   de = mem_alloc(SZ(Edje_Part_Collection_Directory_Entry));
   edje_file->collection_dir->entries = evas_list_append(edje_file->collection_dir->entries, de);
   de->id = evas_list_count(edje_file->collection_dir->entries) - 1;

   pc = mem_alloc(SZ(Edje_Part_Collection));
   edje_collections = evas_list_append(edje_collections, pc);
   pc->id = evas_list_count(edje_collections) - 1;

   cd = mem_alloc(SZ(Code));
   codes = evas_list_append(codes, cd);
}

/**
    @page edcref
    @property
        name
    @parameters
        [group name]
    @effect
        The name that will be used by the application to load the resulting
        Edje object, must be unique within the theme.
    @endproperty
*/
static void
st_collections_group_name(void)
{
   Edje_Part_Collection_Directory_Entry *de;
   Evas_List *l;
   
   check_arg_count(1);

   de = evas_list_data(evas_list_last(edje_file->collection_dir->entries));
   de->entry = parse_str(0);
   for (l = edje_file->collection_dir->entries; l; l = l->next)
     {
	Edje_Part_Collection_Directory_Entry *de_other;
	
	de_other = l->data;
	if ((de_other != de) && (de_other->entry) && 
	    (!strcmp(de->entry, de_other->entry)))
	  {
	     Edje_Part_Collection *pc;
	     Code *cd;
	     int i;
	     
	     pc = evas_list_nth(edje_collections, de_other->id);
	     cd = evas_list_nth(codes, de_other->id);
	     
	     edje_file->collection_dir->entries = 
	       evas_list_remove(edje_file->collection_dir->entries, de_other);
	     edje_collections = 
	       evas_list_remove(edje_collections, pc);
	     codes =
	       evas_list_remove(codes, cd);
	     
	     for (i = 0, l = edje_file->collection_dir->entries; l; l = l->next, i++)
	       {
		  de_other = l->data;
		  de_other->id = i;
	       }
	     for (i = 0, l = edje_collections; l; l = l->next, i++)
	       {
		  pc = l->data;
		  pc->id = i;
	       }
	     break;
	  }
     }
}

/**
    @page edcref
    @property
        script_only
    @parameters
        [on/off]
    @effect
        The flag (on/off) as to if this group is defined ONLY by script
        callbacks such as init(), resize() and shutdown()
    @endproperty
*/
static void
st_collections_group_script_only(void)
{
   Edje_Part_Collection *pc;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   pc->script_only = parse_bool(0);
}

/**
    @page edcref
    @property
        alias
    @parameters
        [aditional group name]
    @effect
        Additional name to serve as identifier. Defining multiple aliases is
        supported.
    @endproperty
*/
static void
st_collections_group_alias(void)
{
   Edje_Part_Collection_Directory_Entry *de, *alias;

   check_arg_count(1);
   de = evas_list_data(evas_list_last(edje_file->collection_dir->entries));

   alias = mem_alloc(SZ(Edje_Part_Collection_Directory_Entry));
   alias->id = de->id;
   alias->entry = parse_str(0);

   aliases = evas_list_append(aliases, alias);
}

/**
    @page edcref
    @property
        min
    @parameters
        [width] [height]
    @effect
        The minimum size for the container defined by the composition of the
        parts.
    @endproperty
*/
static void
st_collections_group_min(void)
{
   Edje_Part_Collection *pc;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   pc->prop.min.w = parse_int_range(0, 0, 0x7fffffff);
   pc->prop.min.h = parse_int_range(1, 0, 0x7fffffff);
}

/**
    @page edcref
    @property
        max
    @parameters
        [width] [height]
    @effect
        The maximum size for the container defined by the totality of the
        parts.
    @endproperty
*/
static void
st_collections_group_max(void)
{
   Edje_Part_Collection *pc;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   pc->prop.max.w = parse_int_range(0, 0, 0x7fffffff);
   pc->prop.max.h = parse_int_range(1, 0, 0x7fffffff);
}

/**
    @page edcref
    @block
        script
    @context
        ..
        group {
            script {
                //embryo script
            }
            ..
            program {
                script {
                    //embryo script
                }
            }
            ..
        }
        ..
    @description
        This block is used to "inject" embryo scripts to a given Edje theme and
        it functions in two modalities. When it's included inside a "program"
        block, the script will be executed every time the program is run, on
        the other hand, when included directly into a "group", "part" or
        "description" block, it will be executed once at load time, in the
        load order.
    @endblock
*/
static void
ob_collections_group_script(void)
{
   Edje_Part_Collection *pc;
   Code *cd;

   pc = evas_list_data(evas_list_last(edje_collections));
   cd = evas_list_data(evas_list_last(codes));

   if (!is_verbatim()) track_verbatim(1);
   else
     {
	char *s;

	s = get_verbatim();
	if (s)
	  {
	     cd->l1 = get_verbatim_line1();
	     cd->l2 = get_verbatim_line2();
	     if (cd->shared)
	       {
		  fprintf(stderr, "%s: Error. parse error %s:%i. There is already an existing script section for the group\n",
			  progname, file_in, line - 1);
		  exit(-1);
	       }
	     cd->shared = s;
	     set_verbatim(NULL, 0, 0);
	  }
     }
}

static void
st_collections_group_data_item(void)
{
   Edje_Part_Collection *pc;
   Edje_Data *di;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   di = mem_alloc(SZ(Edje_Data));
   di->key = parse_str(0);
   di->value = parse_str(1);
   pc->data = evas_list_append(pc->data, di);
}

/**
    @page edcref
    @block
        part
    @context
        group {
            parts {
                ..
                part {
                    name: "partname";
                    type: IMAGE;
                    mouse_events:  1;
                    repeat_events: 0;
                    ignore_flags: NONE;
                    clip_to: "anotherpart";
                    source:  "groupname";
                    pointer_mode: AUTOGRAB;
                    use_alternate_font_metrics: 0;

                    description { }
                    dragable { }
                }
                ..
            }
        }
    @description
        Parts are used to represent the most basic design elements of the
        theme, for example, a part can represent a line in a border or a label
        on a button.
    @endblock
*/
static void
ob_collections_group_parts_part(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;

   ep = mem_alloc(SZ(Edje_Part));
   pc = evas_list_data(evas_list_last(edje_collections));
   pc->parts = evas_list_append(pc->parts, ep);
   ep->id = evas_list_count(pc->parts) - 1;
   ep->type = EDJE_PART_TYPE_IMAGE;
   ep->mouse_events = 1;
   ep->repeat_events = 0;
   ep->ignore_flags = EVAS_EVENT_FLAG_NONE;
   ep->scale = 0;
   ep->pointer_mode = EVAS_OBJECT_POINTER_MODE_AUTOGRAB;
   ep->precise_is_inside = 0;
   ep->use_alternate_font_metrics = 0;
   ep->clip_to_id = -1;
   ep->dragable.confine_id = -1;
   ep->dragable.events_id = -1;
}

/**
    @page edcref
    @property
        name
    @parameters
        [part name]
    @effect
        The part's name will be used as reference in the theme's relative
        positioning system, by programs and in some cases by the application.
        It must be unique within the group.
    @endproperty
*/
static void
st_collections_group_parts_part_name(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ep->name = parse_str(0);

   {
	Evas_List *l;

	for (l = pc->parts; l; l = l->next)
	  {
	     Edje_Part *lep;

	     lep = l->data;
	     if ((lep != ep) && (lep->name) && (!strcmp(lep->name, ep->name)))
	       {
		  fprintf(stderr, "%s: Error. parse error %s:%i. There is already a part of the name %s\n",
			  progname, file_in, line - 1, ep->name);
		  exit(-1);
	       }
	  }
   }
}

/**
    @page edcref
    @property
        type
    @parameters
        [TYPE]
    @effect
        Set the type (all caps) from among the available types, it's set to
        IMAGE by default. Valid types:
            @li RECT
            @li TEXT
            @li IMAGE
            @li SWALLOW
            @li TEXTBLOCK
            @li GRADIENT
            @li GROUP
    @endproperty
*/
static void
st_collections_group_parts_part_type(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ep->type = parse_enum(0,
			 "NONE", EDJE_PART_TYPE_NONE,
			 "RECT", EDJE_PART_TYPE_RECTANGLE,
			 "TEXT", EDJE_PART_TYPE_TEXT,
			 "IMAGE", EDJE_PART_TYPE_IMAGE,
			 "SWALLOW", EDJE_PART_TYPE_SWALLOW,
			 "TEXTBLOCK", EDJE_PART_TYPE_TEXTBLOCK,
			 "GRADIENT", EDJE_PART_TYPE_GRADIENT,
			 "GROUP", EDJE_PART_TYPE_GROUP,
			 NULL);
}

/**
    @page edcref
    @property
        mouse_events
    @parameters
        [1 or 0]
    @effect
        Specifies whether the part will emit signals, altought is named
        "mouse_events", disabling it (0) will prevent the part from emitting
        any type of signal at all. Its set to 1 by default.
    @endproperty
*/
static void
st_collections_group_parts_part_mouse_events(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ep->mouse_events = parse_bool(0);
}

/**
    @page edcref
    @property
        repeat_events
    @parameters
        [1 or 0]
    @effect
        Specifies whether a part echoes a mouse event to other parts below the
        pointer (1), or not (0). Its set to 0 by default.
    @endproperty
*/
static void
st_collections_group_parts_part_repeat_events(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ep->repeat_events = parse_bool(0);
}

/**
    @page edcref
    @property
        ignore_flags
    @parameters
        [FLAG] ...
    @effect
        Specifies whether events with the given flags should be ignored,
	i.e., will not have the signals emitted to the parts. Multiple flags
	must be separated by spaces, the effect will be ignoring all events
	with one of the flags specified. Possible flags:
            @li NONE (default value, no event will be ignored)
            @li ON_HOLD
    @endproperty
*/
static void
st_collections_group_parts_part_ignore_flags(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;

   check_min_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ep->ignore_flags = parse_flags(0,
				  "NONE", EVAS_EVENT_FLAG_NONE,
				  "ON_HOLD", EVAS_EVENT_FLAG_ON_HOLD,
				  NULL);
}

/**
    @page edcref
    @property
        scale
    @parameters
        [1 or 0]
    @effect
        Specifies whether the part will scale its size with an edje scaling
        factor. By default scale is off (0) and the default scale factor is
        1.0 - that means no scaling. This would be used to scale properties
        such as font size, min/max size of the part, and possibly can be used
        to scale based on DPI of the target device. The reason to be selective
        is that some things work well being scaled, others do not, so the
        designer gets to choose what works best.
    @endproperty
*/
static void
st_collections_group_parts_part_scale(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ep->scale = parse_bool(0);
}

/**
    @page edcref
    @property
        pointer_mode
    @parameters
        [MODE]
    @effect
        Sets the mouse pointer behavior for a given part. The default value is
        AUTOGRAB. Aviable modes:
            @li AUTOGRAB, when the part is clicked and the button remains
                pressed, the part will be the source of all future mouse
                signals emitted, even outside the object, until the button is
                released.
            @li NOGRAB, the effect will be limited to the part's container.
        container.
    @endproperty
*/
static void
st_collections_group_parts_part_pointer_mode(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ep->pointer_mode = parse_enum(0,
				 "AUTOGRAB", EVAS_OBJECT_POINTER_MODE_AUTOGRAB,
				 "NOGRAB", EVAS_OBJECT_POINTER_MODE_NOGRAB,
				 NULL);
}

/**
    @page edcref
    @property
        precise_is_inside
    @parameters
        [1 or 0]
    @effect
        Enables precise point collision detection for the part, which is more
        resource intensive. Disabled by default.
    @endproperty
*/
static void
st_collections_group_parts_part_precise_is_inside(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ep->precise_is_inside = parse_bool(0);
}

/**
    @page edcref
    @property
        use_alternate_font_metrics
    @parameters
        [1 or 0]
    @effect
        Only affects text and textblock parts, when enabled Edje will use
        different size measurement functions. Disabled by default. (note from
        the author: I don't know what this is exactlu useful for?)
    @endproperty
*/
static void
st_collections_group_parts_part_use_alternate_font_metrics(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ep->use_alternate_font_metrics = parse_bool(0);
}

/**
    @page edcref
    @property
        clip_to
    @parameters
        [another part's name]
    @effect
        Only renders the area of part that coincides with another part's
        container. Overflowing content will not be displayed.
    @endproperty
*/
static void
st_collections_group_parts_part_clip_to_id(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
     {
	char *name;

	name = parse_str(0);
	data_queue_part_lookup(pc, name, &(ep->clip_to_id));
	free(name);
     }
}

/**
    @page edcref
    @property
        source
    @parameters
        [another group's name]
    @effect
        Only available to GROUP or TEXTBLOCK parts. Swallows the specified 
        group into the part's container if a GROUP. If TEXTBLOCK it is used
        for the group to be loaded and used for selection display UNDER the
        selected text. source2 is used for on top of the selected text, if
        source2 is specified.
    @endproperty
*/
static void
st_collections_group_parts_part_source(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   //FIXME: validate this somehow (need to decide on the format also)
   ep->source = parse_str(0);
}

/**
    @page edcref
    @property
        source2
    @parameters
        [another group's name]
    @effect
        Only available to TEXTBLOCK parts. It is used for the group to be 
        loaded and used for selection display OVER the selected text. source
        is used for under of the selected text, if source is specified.
    @endproperty
*/
static void
st_collections_group_parts_part_source2(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   //FIXME: validate this somehow (need to decide on the format also)
   ep->source2 = parse_str(0);
}

/**
    @page edcref
    @property
        source3
    @parameters
        [another group's name]
    @effect
        Only available to TEXTBLOCK parts. It is used for the group to be 
        loaded and used for cursor display UNDER the cursor position. source4
        is used for over the cursor text, if source4 is specified.
    @endproperty
*/
static void
st_collections_group_parts_part_source3(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   //FIXME: validate this somehow (need to decide on the format also)
   ep->source3 = parse_str(0);
}

/**
    @page edcref
    @property
        source4
    @parameters
        [another group's name]
    @effect
        Only available to TEXTBLOCK parts. It is used for the group to be 
        loaded and used for cursor display OVER the cursor position. source3
        is used for under the cursor text, if source4 is specified.
    @endproperty
*/
static void
st_collections_group_parts_part_source4(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   //FIXME: validate this somehow (need to decide on the format also)
   ep->source4 = parse_str(0);
}

/**
    @page edcref

    @property
        effect
    @parameters
        [EFFECT]
    @effect
        Causes Edje to draw the selected effect among:
        @li PLAIN
        @li OUTLINE
        @li SOFT_OUTLINE
        @li SHADOW
        @li SOFT_SHADOW
        @li OUTLINE_SHADOW
        @li OUTLINE_SOFT_SHADOW
        @li FAR_SHADOW
        @li FAR_SOFT_SHADOW
        @li GLOW
    @endproperty
*/
static void
st_collections_group_parts_part_effect(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ep->effect = parse_enum(0,
               "NONE", EDJE_TEXT_EFFECT_NONE,
               "PLAIN", EDJE_TEXT_EFFECT_PLAIN,
               "OUTLINE", EDJE_TEXT_EFFECT_OUTLINE,
               "SOFT_OUTLINE", EDJE_TEXT_EFFECT_SOFT_OUTLINE,
               "SHADOW", EDJE_TEXT_EFFECT_SHADOW,
               "SOFT_SHADOW", EDJE_TEXT_EFFECT_SOFT_SHADOW,
               "OUTLINE_SHADOW", EDJE_TEXT_EFFECT_OUTLINE_SHADOW,
               "OUTLINE_SOFT_SHADOW", EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW,
               "FAR_SHADOW", EDJE_TEXT_EFFECT_FAR_SHADOW,
               "FAR_SOFT_SHADOW", EDJE_TEXT_EFFECT_FAR_SOFT_SHADOW,
               "GLOW", EDJE_TEXT_EFFECT_GLOW,
               NULL);
}

/**
    @page edcref
    @block
        dragable
    @context
        part {
            ..
            dragable {
                confine: "another part";
                events:  "another dragable part";
                x: 0 0 0;
                y: 0 0 0;
            }
            ..
        }
    @description
        When this block is used the resulting part can be dragged around the
        interface, do not confuse with external drag & drop. By default Edje
        (and most applications) will attempt to use the minimal size possible
        for a dragable part. If the min property is not set in the description
        the part will be (most likely) set to 0px width and 0px height, thus
        invisible.
    @endblock

    @property
        x
    @parameters
        [enable/disable] [step] [count]
    @effect
        Used to setup dragging events for the X axis. The first parameter is
        used to enable (1 or -1) and disable (0) dragging along the axis. When
        enabled, 1 will set the starting point at 0.0 and -1 at 1.0. The second
        parameter takes any integer and will limit movement to values
        divisibles by it, causing the part to jump from position to position.
        The third parameter, (question from the author: What is count for?).
    @endproperty
*/
static void
st_collections_group_parts_part_dragable_x(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;

   check_arg_count(3);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ep->dragable.x = parse_int_range(0, -1, 1);
   ep->dragable.step_x = parse_int_range(1, 0, 0x7fffffff);
   ep->dragable.count_x = parse_int_range(2, 0, 0x7fffffff);
}

/**
    @page edcref
    @property
        y
    @parameters
        [enable/disable] [step] [count]
    @effect
        Used to setup dragging events for the Y axis. The first parameter is
        used to enable (1 or -1) and disable (0) dragging along the axis. When
        enabled, 1 will set the starting point at 0.0 and -1 at 1.0. The second
        parameter takes any integer and will limit movement to values
        divisibles by it, causing the part to jump from position to position.
        The third parameter, (question from the author: What is count for?).
    @endproperty
*/
static void
st_collections_group_parts_part_dragable_y(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;

   check_arg_count(3);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ep->dragable.y = parse_int_range(0, -1, 1);
   ep->dragable.step_y = parse_int_range(1, 0, 0x7fffffff);
   ep->dragable.count_y = parse_int_range(2, 0, 0x7fffffff);
}

/**
    @page edcref
    @property
        confine
    @parameters
        [another part's name]
    @effect
        When set, limits the movement of the dragged part to another part's
        container. When you use confine don't forget to set a min size for the
        part, or the draggie will not show up.
    @endproperty
*/
static void
st_collections_group_parts_part_dragable_confine(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
     {
	char *name;

	name = parse_str(0);
	data_queue_part_lookup(pc, name, &(ep->dragable.confine_id));
	free(name);
     }
}

/**
    @page edcref
    @property
        events
    @parameters
        [another dragable part's name]
    @effect
        It causes the part to forward the drag events to another part, thus
        ignoring them for itself.
    @endproperty
*/
static void
st_collections_group_parts_part_dragable_events(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
     {
	char *name;

	name = parse_str(0);
	data_queue_part_lookup(pc, name, &(ep->dragable.events_id));
	free(name);
     }
}

/**
    @page edcref
    @property
        entry_mode
    @parameters
        Sets the edit mode for a textblock part to one of:
        @li NONE
        @li SELECTABLE
        @li EDITABLE
        @li PASSWORD
    @effect
        It causes the part be editable if the edje object has the keyboard
        focus AND the part has the edje focus (or selectable always
        regardless of focus) and in the event of password mode, not
        selectable and all text chars replaced with *'s but editable and
        pastable.
    @endproperty
*/
static void
st_collections_group_parts_part_entry_mode(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ep->entry_mode = parse_enum(0,
			       "NONE", EDJE_ENTRY_EDIT_MODE_NONE,
			       "PLAIN", EDJE_ENTRY_EDIT_MODE_SELECTABLE,
			       "EDITABLE", EDJE_ENTRY_EDIT_MODE_EDITABLE,
			       "PASSOWRD", EDJE_ENTRY_EDIT_MODE_PASSWORD,
			       NULL);
}

/**
    @page edcref
    @property
        multiline
    @parameters
        0 or 1 (boolean) off/on
    @effect
        It causes a textblock that is editable to allow multiple lines for
        editing.
    @endproperty
*/
static void
st_collections_group_parts_part_multiline(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ep->multiline = parse_bool(0);
}

/**
    @page edcref
    @block
        description
    @context
        description {
            inherit: "another_description" INDEX;
            state: "description_name" INDEX;
            visible: 1;
            min: 0 0;
            max: -1 -1;
            align: 0.5 0.5;
            fixed: 0 0;
            step: 0 0;
            aspect: 1 1;

            rel1 {
                ..
            }

            rel2 {
                ..
            }
        }
    @description
        Every part can have one or more description blocks. Each description is
        used to define style and layout properties of a part in a given
        "state".
    @endblock
*/
static void
ob_collections_group_parts_part_description(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = mem_alloc(SZ(Edje_Part_Description));
   if (!ep->default_desc)
     ep->default_desc = ed;
   else
     ep->other_desc = evas_list_append(ep->other_desc, ed);
   ed->visible = 1;
   ed->align.x = 0.5;
   ed->align.y = 0.5;
   ed->min.w = 0;
   ed->min.h = 0;
   ed->fixed.w = 0;
   ed->fixed.h = 0;
   ed->max.w = -1;
   ed->max.h = -1;
   ed->rel1.relative_x = 0.0;
   ed->rel1.relative_y = 0.0;
   ed->rel1.offset_x = 0;
   ed->rel1.offset_y = 0;
   ed->rel1.id_x = -1;
   ed->rel1.id_y = -1;
   ed->rel2.relative_x = 1.0;
   ed->rel2.relative_y = 1.0;
   ed->rel2.offset_x = -1;
   ed->rel2.offset_y = -1;
   ed->rel2.id_x = -1;
   ed->rel2.id_y = -1;
   ed->image.id = -1;
   ed->fill.smooth = 1;
   ed->fill.pos_rel_x = 0.0;
   ed->fill.pos_abs_x = 0;
   ed->fill.rel_x = 1.0;
   ed->fill.abs_x = 0;
   ed->fill.pos_rel_y = 0.0;
   ed->fill.pos_abs_y = 0;
   ed->fill.rel_y = 1.0;
   ed->fill.abs_y = 0;
   ed->fill.angle = 0;
   ed->fill.spread = 0;
   ed->fill.type = EDJE_FILL_TYPE_SCALE;
   ed->color_class = NULL;
   ed->color.r = 255;
   ed->color.g = 255;
   ed->color.b = 255;
   ed->color.a = 255;
   ed->color2.r = 0;
   ed->color2.g = 0;
   ed->color2.b = 0;
   ed->color2.a = 255;
   ed->color3.r = 0;
   ed->color3.g = 0;
   ed->color3.b = 0;
   ed->color3.a = 128;
   ed->text.align.x = 0.5;
   ed->text.align.y = 0.5;
   ed->text.id_source = -1;
   ed->text.id_text_source = -1;
   ed->gradient.rel1.relative_x = 0;
   ed->gradient.rel1.relative_y = 0;
   ed->gradient.rel1.offset_x = 0;
   ed->gradient.rel1.offset_y = 0;
   ed->gradient.rel2.relative_x = 1;
   ed->gradient.rel2.relative_y = 1;
   ed->gradient.rel2.offset_x = -1;
   ed->gradient.rel2.offset_y = -1;
}

/**
    @page edcref
    @property
        inherit
    @parameters
        [another description's name] [another description's index]
    @effect
        When set, the description will inherit all the properties from the
        named description. The properties defined in this part will override
        the inherited properties, reducing the amount of necessary code for
        simple state changes. Note: inheritance in Edje is single level only.
    @endproperty
*/
static void
st_collections_group_parts_part_description_inherit(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed, *parent = NULL;
   Evas_List *l;
   char *parent_name, *state_name;
   double parent_val, state_val;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   /* inherit may not be used in the default description */
   if (!ep->other_desc)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"inherit may not be used in the default description\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = evas_list_data(evas_list_last(ep->other_desc));

   if (!ed->state.name)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"inherit may only be used after state\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   /* find the description that we inherit from */
   parent_name = parse_str(0);
   parent_val = parse_float_range(1, 0.0, 1.0);

   if (!strcmp (parent_name, "default") && parent_val == 0.0)
     parent = ep->default_desc;
   else
     {
	double min_dst = 999.0;

	if (!strcmp(parent_name, "default"))
	  {
	     parent = ep->default_desc;
	     min_dst = ABS(ep->default_desc->state.value - parent_val);
	  }

	for (l = ep->other_desc; l; l = l->next)
	  {
	     Edje_Part_Description *d = l->data;

	     if (!strcmp (d->state.name, parent_name))
	       {
		  double dst;

		  dst = ABS(d->state.value - parent_val);
		  if (dst < min_dst)
		    {
		       parent = d;
		       min_dst = dst;
		    }
	       }
	  }
     }

   if (!parent)
     {
	fprintf (stderr, "%s: Error. parse error %s:%i. "
	      "cannot find referenced part state %s %lf\n",
	      ep->name, file_in, line - 1, parent_name, parent_val);
	exit(-1);
     }

   free (parent_name);

   /* now do a full copy, only state info will be kept */
   state_name = ed->state.name;
   state_val = ed->state.value;

   *ed = *parent;

   ed->state.name = state_name;
   ed->state.value = state_val;

   data_queue_part_slave_lookup(&parent->rel1.id_x, &ed->rel1.id_x);
   data_queue_part_slave_lookup(&parent->rel1.id_y, &ed->rel1.id_y);
   data_queue_part_slave_lookup(&parent->rel2.id_x, &ed->rel2.id_x);
   data_queue_part_slave_lookup(&parent->rel2.id_y, &ed->rel2.id_y);
   data_queue_image_slave_lookup(&parent->image.id, &ed->image.id);
   data_queue_spectrum_slave_lookup(&parent->gradient.id, &ed->gradient.id);

   /* make sure all the allocated memory is getting copied, not just
    * referenced
    */
   ed->image.tween_list = NULL;

   for (l = parent->image.tween_list; l; l = l->next)
     {
	Edje_Part_Image_Id *iid, *iid_new;

	iid = l->data;
	iid_new = mem_alloc(SZ(Edje_Part_Image_Id));
	data_queue_image_slave_lookup(&(iid->id), &(iid_new->id));
	ed->image.tween_list = evas_list_append(ed->image.tween_list, iid_new);
     }

#define STRDUP(x) x ? strdup(x) : NULL

   ed->color_class = STRDUP(ed->color_class);
   ed->text.text = STRDUP(ed->text.text);
   ed->text.text_class = STRDUP(ed->text.text_class);
   ed->text.font = STRDUP(ed->text.font);
#undef STRDUP

   data_queue_part_slave_lookup(&(parent->text.id_source), &(ed->text.id_source));
   data_queue_part_slave_lookup(&(parent->text.id_text_source), &(ed->text.id_text_source));
}

/**
    @page edcref
    @property
        state
    @parameters
        [a name for the description] [an index]
    @effect
        Sets a name used to identify a description inside a given part.
        Multiple descriptions are used to declare different states of the same
        part, like "clicked" or "invisible". All states declarations are also
        coupled with an index number between 0.0 and 1.0. All parts must have
        at least one description named "default 0.0".
    @endproperty
*/
static void
st_collections_group_parts_part_description_state(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;
   char *s;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));

   s = parse_str(0);
   if (!strcmp (s, "custom"))
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"invalid state name: '%s'.\n",
		progname, file_in, line - 1, s);
	exit(-1);
     }

   ed->state.name = s;
   ed->state.value = parse_float_range(1, 0.0, 1.0);
}

/**
    @page edcref
    @property
        visible
    @parameters
        [0 or 1]
    @effect
        Takes a boolean value specifying whether part is visible (1) or not
        (0). Non-visible parts do not emit signals. The default value is 1.
    @endproperty
*/
static void
st_collections_group_parts_part_description_visible(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->visible = parse_bool(0);
}

/**
    @page edcref
    @property
        align
    @parameters
        [X axis] [Y axis]
    @effect
        When the displayed object's size is smaller than its container, this
        property moves it relatively along both axis inside its container. The
        default value is "0.5 0.5".
    @endproperty
*/
static void
st_collections_group_parts_part_description_align(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->align.x = parse_float_range(0, 0.0, 1.0);
   ed->align.y = parse_float_range(1, 0.0, 1.0);
}

/**
    @page edcref
    @property
        fixed
    @parameters
        [width, 0 or 1] [height, 0 or 1]
    @effect
        When the "min" or "max" properties are set, fixed enables or disables
        resizing for each dimension. The default value is "0 0"
    @endproperty
*/
static void
st_collections_group_parts_part_description_fixed(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->fixed.w = parse_float_range(0, 0, 1);
   ed->fixed.h = parse_float_range(1, 0, 1);
}

/**
    @page edcref
    @property
        min
    @parameters
        [width] [height]
    @effect
        The minimum size of the state.
    @endproperty
*/
static void
st_collections_group_parts_part_description_min(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->min.w = parse_float_range(0, 0, 0x7fffffff);
   ed->min.h = parse_float_range(1, 0, 0x7fffffff);
}

/**
    @page edcref
    @property
        max
    @parameters
        [width] [height]
    @effect
        The maximum size of the state.
    @endproperty
*/
static void
st_collections_group_parts_part_description_max(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->max.w = parse_float_range(0, 0, 0x7fffffff);
   ed->max.h = parse_float_range(1, 0, 0x7fffffff);
}

/**
    @page edcref
    @property
        step
    @parameters
        [width] [height]
    @effect
        Restricts resizing of each dimension to values divisibles by its value.
        This causes the part to jump from value to value while resizing. The
        default value is "0 0" disabling stepping.
    @endproperty
*/
static void
st_collections_group_parts_part_description_step(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->step.x = parse_float_range(0, 0, 0x7fffffff);
   ed->step.y = parse_float_range(1, 0, 0x7fffffff);
}

/**
    @page edcref
    @property
        aspect
    @parameters
        [min] [max]
    @effect
        Normally width and height can be resized to any values independently.
        The aspect property forces the width to height ratio to be kept between
        the minimum and maximum set. For example, "1.0 1.0" will increase the
        width a pixel for every pixel added to heigh. The default value is
        "0.0 0.0" disabling aspect.
    @endproperty
*/
static void
st_collections_group_parts_part_description_aspect(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->aspect.min = parse_float_range(0, 0.0, 999999999.0);
   ed->aspect.max = parse_float_range(1, 0.0, 999999999.0);
}

/**
    @page edcref
    @property
        aspect_preference
    @parameters
        [DIMENSION]
    @effect
        Sets the scope of the "aspect" property to a given dimension. Available
        options are BOTH, VERTICAL, HORIZONTAL and NONE
    @endproperty
*/
static void
st_collections_group_parts_part_description_aspect_preference(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->aspect.prefer =  parse_enum(0,
				   "NONE", EDJE_ASPECT_PREFER_NONE,
				   "VERTICAL", EDJE_ASPECT_PREFER_VERTICAL,
				   "HORIZONTAL", EDJE_ASPECT_PREFER_HORIZONTAL,
				   "BOTH", EDJE_ASPECT_PREFER_BOTH,
				   NULL);
}

/**
    @page edcref
    @property
        color_class
    @parameters
        [color class name]
    @effect
        The part will use the color values of the named color_class, these
        values can be overrided by the "color", "color2" and "color3"
        properties set below.
    @endproperty
*/
static void
st_collections_group_parts_part_description_color_class(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->color_class = parse_str(0);
}

/**
    @page edcref
    @property
        color
    @parameters
        [red] [green] [blue] [alpha]
    @effect
        Sets the main color to the specified values (between 0 and 255).
    @endproperty
*/
static void
st_collections_group_parts_part_description_color(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(4);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->color.r = parse_int_range(0, 0, 255);
   ed->color.g = parse_int_range(1, 0, 255);
   ed->color.b = parse_int_range(2, 0, 255);
   ed->color.a = parse_int_range(3, 0, 255);
}

/**
    @page edcref
    @property
        color2
    @parameters
        [red] [green] [blue] [alpha]
    @effect
        Sets the text shadow color to the specified values (0 to 255).
    @endproperty
*/
static void
st_collections_group_parts_part_description_color2(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(4);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->color2.r = parse_int_range(0, 0, 255);
   ed->color2.g = parse_int_range(1, 0, 255);
   ed->color2.b = parse_int_range(2, 0, 255);
   ed->color2.a = parse_int_range(3, 0, 255);
}

/**
    @page edcref
    @property
        color3
    @parameters
        [red] [green] [blue] [alpha]
    @effect
        Sets the text outline color to the specified values (0 to 255).
    @endproperty
*/
static void
st_collections_group_parts_part_description_color3(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(4);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->color3.r = parse_int_range(0, 0, 255);
   ed->color3.g = parse_int_range(1, 0, 255);
   ed->color3.b = parse_int_range(2, 0, 255);
   ed->color3.a = parse_int_range(3, 0, 255);
}

/**
    @page edcref
    @block
        rel1/rel2
    @context
        description {
            ..
            rel1 {
                relative: 0.0 0.0;
                offset:     0   0;
            }
            ..
            rel2 {
                relative: 1.0 1.0;
                offset:    -1  -1;
            }
            ..
        }
    @description
        The rel1 and rel2 blocks are used to define the position of each corner
        of the part's container. With rel1 being the left-up corner and rel2
        being the right-down corner.
    @endblock

    @property
        relative
    @parameters
        [X axis] [Y axis]
    @effect
        Moves a corner to a relative position inside the container of the
        relative "to" part. Values from 0.0 (0%, begining) to 1.0 (100%, end)
        of each axis.
    @endproperty
*/
static void
st_collections_group_parts_part_description_rel1_relative(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->rel1.relative_x = parse_float(0);
   ed->rel1.relative_y = parse_float(1);
}

/**
    @page edcref
    @property
        offset
    @parameters
        [X axis] [Y axis]
    @effect
        Affects the corner postion a fixed number of pixels along each axis.
    @endproperty
*/
static void
st_collections_group_parts_part_description_rel1_offset(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->rel1.offset_x = parse_int(0);
   ed->rel1.offset_y = parse_int(1);
}

/**
    @page edcref
    @property
        to
    @parameters
        [another part's name]
    @effect
        Causes a corner to be positioned relatively to another part's
        container.
    @endproperty
*/
static void
st_collections_group_parts_part_description_rel1_to(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
     {
	char *name;

	name = parse_str(0);
	data_queue_part_lookup(pc, name, &(ed->rel1.id_x));
	data_queue_part_lookup(pc, name, &(ed->rel1.id_y));
	free(name);
     }
}

/**
    @page edcref
    @property
        to_x
    @parameters
        [another part's name]
    @effect
        Causes a corner to be positioned relatively to the X axis of another
        part's container. Simply put affects the first parameter of "relative".
    @endproperty
*/
static void
st_collections_group_parts_part_description_rel1_to_x(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
     {
	char *name;

	name = parse_str(0);
	data_queue_part_lookup(pc, name, &(ed->rel1.id_x));
	free(name);
     }
}

/**
    @page edcref
    @property
        to_y
    @parameters
        [another part's name]
    @effect
        Causes a corner to be positioned relatively to the Y axis of another
        part's container. Simply put, affects the second parameter of
        "relative".
    @endproperty
*/
static void
st_collections_group_parts_part_description_rel1_to_y(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
     {
	char *name;

	name = parse_str(0);
	data_queue_part_lookup(pc, name, &(ed->rel1.id_y));
	free(name);
     }
}

static void
st_collections_group_parts_part_description_rel2_relative(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->rel2.relative_x = parse_float(0);
   ed->rel2.relative_y = parse_float(1);
}

static void
st_collections_group_parts_part_description_rel2_offset(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->rel2.offset_x = parse_int(0);
   ed->rel2.offset_y = parse_int(1);
}

static void
st_collections_group_parts_part_description_rel2_to(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
     {
	char *name;

	name = parse_str(0);
	data_queue_part_lookup(pc, name, &(ed->rel2.id_x));
	data_queue_part_lookup(pc, name, &(ed->rel2.id_y));
	free(name);
     }
}

static void
st_collections_group_parts_part_description_rel2_to_x(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
     {
	char *name;

	name = parse_str(0);
	data_queue_part_lookup(pc, name, &(ed->rel2.id_x));
	free(name);
     }
}

static void
st_collections_group_parts_part_description_rel2_to_y(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
     {
	char *name;

	name = parse_str(0);
	data_queue_part_lookup(pc, name, &(ed->rel2.id_y));
	free(name);
     }
}

/**
    @page edcref
    @block
        image
    @context
        description {
            ..
            image {
                normal: "filename.ext";
                tween:  "filename2.ext";
                ..
                tween:  "filenameN.ext";
                border:  left right top bottom;
                middle:  0-1;
            }
            ..
        }
    @description
    @endblock

    @property
        normal
    @parameters
        [image's filename]
    @effect
        Name of image to be used as previously declared in the  images block.
        In an animation, this is the first and last image displayed. It's
        required in any image part
    @endproperty
*/
static void
st_collections_group_parts_part_description_image_normal(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if (ep->type != EDJE_PART_TYPE_IMAGE)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"image attributes in non-IMAGE part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
     {
	char *name;

	name = parse_str(0);
	data_queue_image_lookup(name, &(ed->image.id));
	free(name);
     }
}

/**
    @page edcref
    @property
        tween
    @parameters
        [image's filename]
    @effect
        Name of an image to be used in an animation loop, an image block can
        have none, one or multiple tween declarations. Images are displayed in
        the order they are listed.
    @endproperty
*/
static void
st_collections_group_parts_part_description_image_tween(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if (ep->type != EDJE_PART_TYPE_IMAGE)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"image attributes in non-IMAGE part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
     {
	char *name;
	Edje_Part_Image_Id *iid;

	iid = mem_alloc(SZ(Edje_Part_Image_Id));
	ed->image.tween_list = evas_list_append(ed->image.tween_list, iid);
	name = parse_str(0);
	data_queue_image_lookup(name, &(iid->id));
	free(name);
     }
}

/**
    @page edcref
    @property
        border
    @parameters
        [left] [right] [top] [bottom]
    @effect
        If set, the area (in pixels) of each side of the image will be
        displayed as a fixed size border, from the side -> inwards, preventing
        the corners from being changed on a resize.
    @endproperty
*/
static void
st_collections_group_parts_part_description_image_border(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(4);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if (ep->type != EDJE_PART_TYPE_IMAGE)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"image attributes in non-IMAGE part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->border.l = parse_int_range(0, 0, 0x7fffffff);
   ed->border.r = parse_int_range(1, 0, 0x7fffffff);
   ed->border.t = parse_int_range(2, 0, 0x7fffffff);
   ed->border.b = parse_int_range(3, 0, 0x7fffffff);
}

/**
    @page edcref
    @property
        middle
    @parameters
        [0 or 1]
    @effect
        If border is set, this boolean value tells Edje if the rest of the
        image (not covered by the defined border) will be displayed or not.
        The default value is 1.
    @endproperty
*/
static void
st_collections_group_parts_part_description_image_middle(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if (ep->type != EDJE_PART_TYPE_IMAGE)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"image attributes in non-IMAGE part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->border.no_fill = !parse_bool(0);
}

/**
    @page edcref
    @block
        fill
    @context
        description {
            ..
            fill {
                smooth: 0-1;
                origin {
                    relative: X-axis Y-axis;
                    offset:   X-axis Y-axis;
                }
                size {
                    relative: width  height;
                    offset:   width  height;
                }
            }
            ..
        }
    @description
        The fill method is an optional block that defines the way an IMAGE or
        GRADIENT part is going to be displayed inside its container.
    @endblock

    @property
        smooth
    @parameters
        [0 or 1]
    @effect
        The smooth property takes a boolean value to decide if the image will
        be smoothed on scaling (1) or not (0). The default value is 1.
    @endproperty
*/
static void
st_collections_group_parts_part_description_fill_smooth(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if (ep->type != EDJE_PART_TYPE_IMAGE)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"fill.type attribute in non-IMAGE part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->fill.smooth = parse_bool(0);
}

/**
    @page edcref

    @property
        spread
    @parameters
        TODO
    @effect
        TODO
    @endproperty
*/
static void
st_collections_group_parts_part_description_fill_spread(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   /* XXX this will need to include IMAGES when spread support is added to evas images */
   if (ep->type != EDJE_PART_TYPE_GRADIENT)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"gradient attributes in non-GRADIENT part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->fill.spread = parse_int_range(0, 0, 1);
}

/**
    @page edcref

    @property
        angle
    @parameters
        [angle]
    @effect
        The angle of rotation of a GRADIENT part. It is invalid in any other
        part type. The angle is espressed as an int, in the range 0 - 360.
    @endproperty
*/
static void
st_collections_group_parts_part_description_fill_angle(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   /* XXX this will need to include IMAGES when angle support is added to evas images */
   if (ep->type != EDJE_PART_TYPE_GRADIENT)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"gradient attributes in non-GRADIENT part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->fill.angle = parse_int_range(0, 0, 360);
}

/**
    @page edcref

    @property
        type
    @parameters
        TODO
    @effect
        TODO
    @endproperty
*/
static void
st_collections_group_parts_part_description_fill_type(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));

   if (ep->type != EDJE_PART_TYPE_IMAGE)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"fill attributes in non-IMAGE part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed->fill.type = parse_enum(0,
                              "SCALE", EDJE_FILL_TYPE_SCALE,
                              "TILE", EDJE_FILL_TYPE_TILE,
                              NULL);
}

/**
    @page edcref
    @block
        origin
    @context
        description {
            ..
            fill {
                ..
                origin {
                    relative: 0.0 0.0;
                    offset:   0   0;
                }
                ..
            }
            ..
        }
    @description
        The origin block is used to place the starting point, inside the
        displayed element, that will be used to render the tile. By default,
        the origin is set at the element's left-up corner.
    @endblock

    @property
        relative
    @parameters
        [X axis] [Y axis]
    @effect
        Sets the starting point relatively to displayed element's content.
    @endproperty
*/
static void
st_collections_group_parts_part_description_fill_origin_relative(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if (ep->type != EDJE_PART_TYPE_IMAGE && ep->type != EDJE_PART_TYPE_GRADIENT)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"fill attributes in non-IMAGE part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->fill.pos_rel_x = parse_float_range(0, -999999999.0, 999999999.0);
   ed->fill.pos_rel_y = parse_float_range(1, -999999999.0, 999999999.0);
}

/**
    @page edcref
    @property
        offset
    @parameters
        [X axis] [Y axis]
    @effect
        Affects the starting point a fixed number of pixels along each axis.
    @endproperty
*/
static void
st_collections_group_parts_part_description_fill_origin_offset(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if (ep->type != EDJE_PART_TYPE_IMAGE && ep->type != EDJE_PART_TYPE_GRADIENT)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"fill attributes in non-IMAGE part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->fill.pos_abs_x = parse_int(0);
   ed->fill.pos_abs_y = parse_int(1);
}

/**
    @page edcref
    @block
        size
    @context
        description {
            ..
            fill {
                ..
                size {
                    relative: 1.0 1.0;
                    offset:  -1  -1;
                }
                ..
            }
            ..
        }
    @description
        The size block defines the tile size of the content that will be
        displayed.
    @endblock

    @property
        relative
    @parameters
        [width] [height]
    @effect
        Takes a pair of decimal values that represent the a percentual value
        of the original size of the element. For example, "0.5 0.5" represents
        half the size, while "2.0 2.0" represents the double. The default
        value is "1.0 1.0".
    @endproperty
*/
static void
st_collections_group_parts_part_description_fill_size_relative(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));

   if (ep->type != EDJE_PART_TYPE_IMAGE && ep->type != EDJE_PART_TYPE_GRADIENT)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"fill attributes in non-IMAGE part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed->fill.rel_x = parse_float_range(0, 0.0, 999999999.0);
   ed->fill.rel_y = parse_float_range(1, 0.0, 999999999.0);
}

/**
    @page edcref
    @property
        offset
    @parameters
        [X axis] [Y axis]
    @effect
        Affects the size of the tile a fixed number of pixels along each axis.
    @endproperty
*/
static void
st_collections_group_parts_part_description_fill_size_offset(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));
   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));

   if (ep->type != EDJE_PART_TYPE_IMAGE && ep->type != EDJE_PART_TYPE_GRADIENT)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"fill attributes in non-IMAGE part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed->fill.abs_x = parse_int(0);
   ed->fill.abs_y = parse_int(1);
}

/**
    @page edcref

    @block
        text
    @context
        part {
            description {
                ..
                text {
                    text:        "some string of text to display";
                    font:        "font_name";
                    size:         SIZE;
                    text_class:  "class_name";
                    fit:          horizontal vertical;
                    min:          horizontal vertical;
                    max:          horizontal vertical;
                    align:        X-axis     Y-axis;
                    source:      "part_name";
                    text_source: "text_part_name";
                    elipsis:      0.0-1.0;
                    style:       "stylename";
                }
                ..
            }
        }
    @description
    @endblock

    @property
        text
    @parameters
        [a string of text, or nothing]
    @effect
        Sets the default content of a text part, normally the application is
        the one changing its value.
    @endproperty
*/
static void
st_collections_group_parts_part_description_text_text(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;
   char *str = NULL;
   int i;

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if ((ep->type != EDJE_PART_TYPE_TEXT) &&
       (ep->type != EDJE_PART_TYPE_TEXTBLOCK))
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"text attributes in non-TEXT part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   for (i = 0; ;i++)
     {
	char *s;

	if (!is_param(i)) break;
	s = parse_str(i);
	if (!str) str = s;
	else
	  {
	     str = realloc(str, strlen(str) + strlen(s) + 1);
	     strcat(str, s);
	     free(s);
	  }
     }
   ed->text.text = str;
}

/**
    @page edcref

    @property
        text_class
    @parameters
        [text class name]
    @effect
        Similar to color_class, this is the name used by the application
        to alter the font family and size at runtime.
    @endproperty
*/
static void
st_collections_group_parts_part_description_text_text_class(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if ((ep->type != EDJE_PART_TYPE_TEXT) &&
       (ep->type != EDJE_PART_TYPE_TEXTBLOCK))
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"text attributes in non-TEXT part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->text.text_class = parse_str(0);
}

/**
    @page edcref

    @property
        font
    @parameters
        [font alias]
    @effect
        This sets the font family to one of the aliases set up in the "fonts"
        block. Can be overrided by the application.
    @endproperty
*/
static void
st_collections_group_parts_part_description_text_font(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if (ep->type != EDJE_PART_TYPE_TEXT)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"text attributes in non-TEXT part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->text.font = parse_str(0);
}

/**
    @page edcref

    @property
        style
    @parameters
        [the style name]
    @effect
        Causes the part to use the default style and tags defined in the
        "style" block with the specified name.
    @endproperty
*/
static void
st_collections_group_parts_part_description_text_style(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if (ep->type != EDJE_PART_TYPE_TEXTBLOCK)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"text attributes in non-TEXTBLOCK part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->text.style = parse_str(0);
}

/**
    @page edcref

    @property
        size
    @parameters
        [font size in points (pt)]
    @effect
        Sets the default font size for the text part. Can be overrided by the
        application.
    @endproperty
*/
static void
st_collections_group_parts_part_description_text_size(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if (ep->type != EDJE_PART_TYPE_TEXT)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"text attributes in non-TEXT part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->text.size = parse_int_range(0, 0, 255);
}

/**
    @page edcref

    @property
        fit
    @parameters
        [horizontal] [vertical]
    @effect
        When any of the parameters is set to 1 edje will resize the text for it
        to fit in it's container. Both are disabled by default.
    @endproperty
*/
static void
st_collections_group_parts_part_description_text_fit(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if (ep->type != EDJE_PART_TYPE_TEXT)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"text attributes in non-TEXT part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->text.fit_x = parse_bool(0);
   ed->text.fit_y = parse_bool(1);
}

/**
    @page edcref

    @property
        min
    @parameters
        [horizontal] [vertical]
    @effect
        When any of the parameters is enabled (1) it forces the minimum size of
        the container to be equal to the minimum size of the text. The default
        value is "0 0".
    @endproperty
*/
static void
st_collections_group_parts_part_description_text_min(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if ((ep->type != EDJE_PART_TYPE_TEXT) &&
       (ep->type != EDJE_PART_TYPE_TEXTBLOCK))
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"text attributes in non-TEXT part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->text.min_x = parse_bool(0);
   ed->text.min_y = parse_bool(1);
}

/**
    @page edcref

    @property
        max
    @parameters
        [horizontal] [vertical]
    @effect
        When any of the parameters is enabled (1) it forces the maximum size of
        the container to be equal to the maximum size of the text. The default
        value is "0 0".
    @endproperty
*/
static void
st_collections_group_parts_part_description_text_max(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if ((ep->type != EDJE_PART_TYPE_TEXT) &&
       (ep->type != EDJE_PART_TYPE_TEXTBLOCK))
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"text attributes in non-TEXT part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->text.max_x = parse_bool(0);
   ed->text.max_y = parse_bool(1);
}

/**
    @page edcref

    @property
        align
    @parameters
        [horizontal] [vertical]
    @effect
        Change the position of the point of balance inside the container. The
        default value is 0.5 0.5.
    @endproperty
*/
static void
st_collections_group_parts_part_description_text_align(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if (ep->type != EDJE_PART_TYPE_TEXT)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"text attributes in non-TEXT part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->text.align.x = parse_float_range(0, 0.0, 1.0);
   ed->text.align.y = parse_float_range(1, 0.0, 1.0);
}

/**
    @page edcref

    @property
        source
    @parameters
        [another TEXT part's name]
    @effect
        Causes the part to use the text properties (like font and size) of
        another part and update them as they change.
    @endproperty
*/
static void
st_collections_group_parts_part_description_text_source(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if ((ep->type != EDJE_PART_TYPE_TEXT) &&
       (ep->type != EDJE_PART_TYPE_TEXTBLOCK))
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"text attributes in non-TEXT part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
     {
	char *name;

	name = parse_str(0);
	data_queue_part_lookup(pc, name, &(ed->text.id_source));
	free(name);
     }
}

/**
    @page edcref

    @property
        text_source
    @parameters
        [another TEXT part's name]
    @effect
        Causes the part to display the text content of another part and update
        them as they change.
    @endproperty
*/
static void
st_collections_group_parts_part_description_text_text_source(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if ((ep->type != EDJE_PART_TYPE_TEXT) &&
       (ep->type != EDJE_PART_TYPE_TEXTBLOCK))
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"text attributes in non-TEXT part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
     {
	char *name;

	name = parse_str(0);
	data_queue_part_lookup(pc, name, &(ed->text.id_text_source));
	free(name);
     }
}

/**
    @page edcref

    @property
        text_elipsis
    @parameters
        [point of balance]
    @effect
        Used to balance the text in a relative point from 0.0 to 1.0, this
        point is the last section of the string to be cut out in case of a
        resize that is smaller than the text itself. The default value is 0.0.
    @endproperty
*/
static void
st_collections_group_parts_part_description_text_elipsis(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if (ep->type != EDJE_PART_TYPE_TEXT)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"text attributes in non-TEXT part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->text.elipsis = parse_float_range(0, 0.0, 1.0);
}

/**
    @page edcref

    @block
        gradient
    @context
        part {
            description {
                ..
                gradient {
                    type:    "linear";
                    spectrum "spectrumName";
                    rel1 {
                        relative: 0.0 0.0;
                        offset:     0   0;
                    }
                    rel2
                        relative: 1.0 1.0;
                        offset:    -1  -1;
                    }
                }
                ..
            }
        }
    @description
        A gradient block is used to display a given "spectrum" inside a 
        container. The container's shape is a rect but this not mean the
        gradient is restricted to a rectangular shape.  Gradients can use 
        "rel1" and "rel2" blocks to layout the initial and final point 
        relatively inside the container.
    @endblock

    @property
        type
    @parameters
        [the name of the type]
    @effect
        Alters the gradient's rendering algorithm between:
            @li linear (default)  
            @li radial
            @li rectangular
            @li angular
            @li sinusoidal
    @endproperty
*/
static void
st_collections_group_parts_part_description_gradient_type(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if (ep->type != EDJE_PART_TYPE_GRADIENT)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"gradient attributes in non-GRADIENT part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));
   ed->gradient.type  = parse_str(0);
}

/**
    @page edcref

    @property
        spectrum
    @parameters
        [an existing spectrum name]
    @effect
        Causes the gradient to display the colors as defined by a given 
        "spectrum" in the "spectra" block.
    @endproperty
*/
static void
st_collections_group_parts_part_description_gradient_spectrum(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if (ep->type != EDJE_PART_TYPE_GRADIENT)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"gradient attributes in non-GRADIENT part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));

     {
	char *name;

	name = parse_str(0);
	data_queue_spectrum_lookup(name, &(ed->gradient.id));
	free(name);
     }
}

/**
    @page edcref

    @property
        relative
    @parameters
        [a relative X coordinate] [a relative Y coordinate]
    @effect
        Inside rel1 places the initial point, or first color, of the gradient 
        relatively to the gradient's container. Inside rel2 places the final 
        point, or last color.
    @endproperty
*/
static void
st_collections_group_parts_part_description_gradient_rel1_relative(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if (ep->type != EDJE_PART_TYPE_GRADIENT)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"gradient attributes in non-GRADIENT part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));

     {
	ed->gradient.use_rel = 1;
	ed->gradient.rel1.relative_x = parse_float(0);
	ed->gradient.rel1.relative_y = parse_float(1);
     }
}

/**
    @page edcref

    @property
        offset
    @parameters
        [X axis] [Y axis]
    @effect
        Inside rel1 moves the initial point, or first color, of the gradient
        a fixed number of pixels along either axis. Inside rel2 moves the final
        point, or last color.
    @endproperty
*/
static void
st_collections_group_parts_part_description_gradient_rel1_offset(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if (ep->type != EDJE_PART_TYPE_GRADIENT)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"gradient attributes in non-GRADIENT part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));

     {
	ed->gradient.use_rel = 1;
	ed->gradient.rel1.offset_x = parse_int(0);
	ed->gradient.rel1.offset_y = parse_int(1);
     }
}

static void
st_collections_group_parts_part_description_gradient_rel2_relative(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if (ep->type != EDJE_PART_TYPE_GRADIENT)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"gradient attributes in non-GRADIENT part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));

     {
	ed->gradient.use_rel = 1;
	ed->gradient.rel2.relative_x = parse_float(0);
	ed->gradient.rel2.relative_y = parse_float(1);
     }
}

static void
st_collections_group_parts_part_description_gradient_rel2_offset(void)
{
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   Edje_Part_Description *ed;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->parts));

   if (ep->type != EDJE_PART_TYPE_GRADIENT)
     {
	fprintf(stderr, "%s: Error. parse error %s:%i. "
		"gradient attributes in non-GRADIENT part.\n",
		progname, file_in, line - 1);
	exit(-1);
     }

   ed = ep->default_desc;
   if (ep->other_desc) ed = evas_list_data(evas_list_last(ep->other_desc));

     {
	ed->gradient.use_rel = 1;
	ed->gradient.rel2.offset_x = parse_int(0);
	ed->gradient.rel2.offset_y = parse_int(1);
     }
}

/**
    @page edcref
    @block
        program
    @context
        group {
            programs {
               ..
                  program {
                     name: "programname";
                     signal: "signalname";
                     source: "partname";
                     in: 0.3 0.0;
                     action: STATE_SET "statename" state_value;
                     transition: LINEAR 0.5;
                     target: "partname";
                     target: "anotherpart";
                     after: "programname";
                     after: "anotherprogram";
                  }
               ..
            }
        }
    @description
        Programs define how your interface reacts to events.
        Programs can change the state of parts, react to events or trigger
        other events.
    @endblock
*/
static void
ob_collections_group_programs_program(void)
{
   Edje_Part_Collection *pc;
   Edje_Program *ep;

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = mem_alloc(SZ(Edje_Program));
   pc->programs = evas_list_append(pc->programs, ep);
   ep->id = evas_list_count(pc->programs) - 1;
   ep->tween.mode = EDJE_TWEEN_MODE_LINEAR;
   ep->after = NULL;
}

/**
    @page edcref
    @property
        name
    @parameters
        [program name]
    @effect
        Symbolic name of program as a unique identifier.
    @endproperty
*/
static void
st_collections_group_programs_program_name(void)
{
   Edje_Part_Collection *pc;
   Edje_Program *ep;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->programs));
   ep->name = parse_str(0);
     {
	Evas_List *l;

	for (l = pc->programs; l; l = l->next)
	  {
	     Edje_Program *lep;

	     lep = l->data;
	     if ((lep != ep) && (lep->name) && (!strcmp(lep->name, ep->name)))
	       {
		  fprintf(stderr, "%s: Error. parse error %s:%i. There is already a program of the name %s\n",
			  progname, file_in, line - 1, ep->name);
		  exit(-1);
	       }
	  }
     }
}

/**
    @page edcref
    @property
        signal
    @parameters
        [signal name]
    @effect
        Specifies signal(s) that should cause the program to run. The signal
        received must match the specified source to run.
        Signals may be globbed, but only one signal keyword per program
        may be used. ex: signal: "mouse,clicked,*"; (clicking any mouse button
        that matches source starts program).
    @endproperty
*/
static void
st_collections_group_programs_program_signal(void)
{
   Edje_Part_Collection *pc;
   Edje_Program *ep;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->programs));
   ep->signal = parse_str(0);
}

/**
    @page edcref
    @property
        source
    @parameters
        [source name]
    @effect
        Source of accepted signal. Sources may be globbed, but only one source
        keyword per program may be used. ex:source: "button-*"; (Signals from
        any part or program named "button-*" are accepted)
    @endproperty
*/
static void
st_collections_group_programs_program_source(void)
{
   Edje_Part_Collection *pc;
   Edje_Program *ep;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->programs));
   ep->source = parse_str(0);
}

/**
    @page edcref
    @property
        in
    @parameters
        [from] [range]
    @effect
        Wait 'from' seconds before executing the program. And add a random
        number of seconds (from 0 to 'range') to the total waiting time.
    @endproperty
*/
static void
st_collections_group_programs_program_in(void)
{
   Edje_Part_Collection *pc;
   Edje_Program *ep;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->programs));
   ep->in.from = parse_float_range(0, 0.0, 999999999.0);
   ep->in.range = parse_float_range(1, 0.0, 999999999.0);
}

/**
    @page edcref
    @property
        action
    @parameters
        [type] [param1] [param2]
    @effect
        Action to be performed by the program. Valid actions are: STATE_SET,
        ACTION_STOP, SIGNAL_EMIT, DRAG_VAL_SET, DRAG_VAL_STEP, DRAG_VAL_PAGE,
        FOCUS_SET.
        Only one action can be specified per program. Examples:\n
           action: STATE_SET "statename" 0.5;\n
           action: ACTION_STOP "programname";\n
           action: SIGNAL_EMIT "signalname" "emitter";
    @endproperty
*/
static void
st_collections_group_programs_program_action(void)
{
   Edje_Part_Collection *pc;
   Edje_Program *ep;

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->programs));
   ep->action = parse_enum(0,
			   "STATE_SET", EDJE_ACTION_TYPE_STATE_SET,
			   "ACTION_STOP", EDJE_ACTION_TYPE_ACTION_STOP,
			   "SIGNAL_EMIT", EDJE_ACTION_TYPE_SIGNAL_EMIT,
			   "DRAG_VAL_SET", EDJE_ACTION_TYPE_DRAG_VAL_SET,
			   "DRAG_VAL_STEP", EDJE_ACTION_TYPE_DRAG_VAL_STEP,
			   "DRAG_VAL_PAGE", EDJE_ACTION_TYPE_DRAG_VAL_PAGE,
			   "SCRIPT", EDJE_ACTION_TYPE_SCRIPT,
			   "FOCUS_SET", EDJE_ACTION_TYPE_FOCUS_SET,
			   NULL);
   if (ep->action == EDJE_ACTION_TYPE_STATE_SET)
     {
	ep->state = parse_str(1);
	ep->value = parse_float_range(2, 0.0, 1.0);
     }
   else if (ep->action == EDJE_ACTION_TYPE_SIGNAL_EMIT)
     {
	ep->state = parse_str(1);
	ep->state2 = parse_str(2);
     }
   else if (ep->action == EDJE_ACTION_TYPE_DRAG_VAL_SET)
     {
	ep->value = parse_float(1);
	ep->value2 = parse_float(2);
     }
   else if (ep->action == EDJE_ACTION_TYPE_DRAG_VAL_STEP)
     {
	ep->value = parse_float(1);
	ep->value2 = parse_float(2);
     }
   else if (ep->action == EDJE_ACTION_TYPE_DRAG_VAL_PAGE)
     {
	ep->value = parse_float(1);
	ep->value2 = parse_float(2);
     }
   
   switch (ep->action)
     {
      case EDJE_ACTION_TYPE_ACTION_STOP:
	check_arg_count(1);
	break;
      case EDJE_ACTION_TYPE_SCRIPT:
	/* this is implicitly set by script {} so this is here just for
	 * completeness */
	break;
      case EDJE_ACTION_TYPE_FOCUS_SET:
	check_arg_count(1);
	break;
      default:
	check_arg_count(3);
     }
}

/**
    @page edcref
    @property
        transition
    @parameters
        [type] [length]
    @effect
        Defines how transistions occur using STATE_SET action.\n
        Where 'type' is the style of the transistion and 'length' is a double
        specifying the number of seconds in which to preform the transistion.\n
        Valid types are: LINEAR, SINUSOIDAL, ACCELERATE, and DECELERATE.
    @endproperty
*/
static void
st_collections_group_programs_program_transition(void)
{
   Edje_Part_Collection *pc;
   Edje_Program *ep;

   check_arg_count(2);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->programs));
   ep->tween.mode = parse_enum(0,
			       "LINEAR", EDJE_TWEEN_MODE_LINEAR,
			       "SINUSOIDAL", EDJE_TWEEN_MODE_SINUSOIDAL,
			       "ACCELERATE", EDJE_TWEEN_MODE_ACCELERATE,
			       "DECELERATE", EDJE_TWEEN_MODE_DECELERATE,
			       NULL);
   ep->tween.time = parse_float_range(1, 0.0, 999999999.0);
}

/**
    @page edcref
    @property
        target
    @parameters
        [target]
    @effect
        Program or part on which the specified action acts. Multiple target
        keywords may be specified, one per target. SIGNAL_EMITs do not have
        targets.
    @endproperty
*/
static void
st_collections_group_programs_program_target(void)
{
   Edje_Part_Collection *pc;
   Edje_Program *ep;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->programs));
     {
	Edje_Program_Target *et;
	char *name;

	et = mem_alloc(SZ(Edje_Program_Target));
	ep->targets = evas_list_append(ep->targets, et);

	name = parse_str(0);
	if (ep->action == EDJE_ACTION_TYPE_STATE_SET)
	  data_queue_part_lookup(pc, name, &(et->id));
	else if (ep->action == EDJE_ACTION_TYPE_ACTION_STOP)
	  data_queue_program_lookup(pc, name, &(et->id));
	else if (ep->action == EDJE_ACTION_TYPE_DRAG_VAL_SET)
	  data_queue_part_lookup(pc, name, &(et->id));
	else if (ep->action == EDJE_ACTION_TYPE_DRAG_VAL_STEP)
	  data_queue_part_lookup(pc, name, &(et->id));
	else if (ep->action == EDJE_ACTION_TYPE_DRAG_VAL_PAGE)
	  data_queue_part_lookup(pc, name, &(et->id));
	else if (ep->action == EDJE_ACTION_TYPE_FOCUS_SET)
	  data_queue_part_lookup(pc, name, &(et->id));
	else
	  {
	     fprintf(stderr, "%s: Error. parse error %s:%i. "
		   "target may only be used after action\n",
		   progname, file_in, line - 1);
	     exit(-1);
	  }
	free(name);
     }
}

/**
    @page edcref
    @property
        after
    @parameters
        [after]
    @effect
        Specifies a program to run after the current program completes. The
        source and signal parameters of a program run as an "after" are ignored.
        Multiple "after" statements can be specified per program.
    @endproperty
*/
static void
st_collections_group_programs_program_after(void)
{
   Edje_Part_Collection *pc;
   Edje_Program *ep;

   check_arg_count(1);

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->programs));
     {
	Edje_Program_After *pa;
	char *name;

	name = parse_str(0);

	pa = mem_alloc(SZ(Edje_Program_After));
	pa->id = -1;
	ep->after = evas_list_append(ep->after, pa);

	data_queue_program_lookup(pc, name, &(pa->id));
	free(name);
     }
}

static void
ob_collections_group_programs_program_script(void)
{
   Edje_Part_Collection *pc;
   Edje_Program *ep;
   Code *cd;

   pc = evas_list_data(evas_list_last(edje_collections));
   ep = evas_list_data(evas_list_last(pc->programs));
   cd = evas_list_data(evas_list_last(codes));

   if (!is_verbatim()) track_verbatim(1);
   else
     {
	char *s;

	s = get_verbatim();
	if (s)
	  {
	     Code_Program *cp;

	     cp = mem_alloc(SZ(Code_Program));
	     cp->l1 = get_verbatim_line1();
	     cp->l2 = get_verbatim_line2();
	     cp->id = ep->id;
	     cp->script = s;
	     cd->programs = evas_list_append(cd->programs, cp);
	     set_verbatim(NULL, 0, 0);
	     ep->action = EDJE_ACTION_TYPE_SCRIPT;
	  }
     }
}
/**
    @page edcref
    </table>
*/
