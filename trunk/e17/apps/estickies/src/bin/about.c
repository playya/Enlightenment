#include "stickies.h"

extern E_Stickies *ss;
    
void
_e_about_show()
{
   static Etk_Widget *win = NULL;
   Etk_Widget *frame;
   Etk_Widget *vbox;
   Etk_Widget *logo;
   Etk_Widget *desctext;   
   Etk_Widget *abouttext;
   
   if(win)
     {
	etk_widget_show_all(win);
	return;
     }
   
   win = etk_dialog_new();
   etk_window_title_set(ETK_WINDOW(win), "About E Stickies");
   etk_signal_connect_swapped("delete_event", ETK_OBJECT(win),
			      ETK_CALLBACK(etk_window_hide_on_delete), win);
   etk_signal_connect_swapped("response", ETK_OBJECT(win),
			      ETK_CALLBACK(etk_window_hide_on_delete), win);
   
   vbox = etk_vbox_new(ETK_FALSE, 0);
   
   /* Logo */
   logo = etk_image_new_from_file(PACKAGE_DATA_DIR"/images/estickies.png");
   etk_box_pack_start(ETK_BOX(vbox), logo, ETK_TRUE, ETK_FALSE, 0);
      
   /* Description */
   frame = etk_frame_new("What is E Stickies?");
   desctext = etk_text_view_new();
   etk_widget_size_request_set(desctext, -1, 75);
   etk_object_properties_set(ETK_OBJECT(desctext),
			     "focusable", ETK_FALSE, NULL);   
   etk_textblock_text_set(ETK_TEXT_VIEW(desctext)->textblock,
			  "E Stickies is a sticky notes application that "
			  "uses Etk. It uses Etk's runtime theming support "
			  "to change the look and feel of the windows and "
			  "buttons.\n"
			  "<b><p align=\"center\"><style effect=\"glow\">"
			  VERSION
			  "</style></p></b>",
			  ETK_TRUE);
   etk_container_add(ETK_CONTAINER(frame), desctext);
   etk_box_pack_start(ETK_BOX(vbox), frame, ETK_FALSE, ETK_FALSE, 0);   
   
   /* Authors */
   frame = etk_frame_new("Authors");
   abouttext = etk_text_view_new();
   etk_widget_size_request_set(abouttext, -1, 75);
   etk_object_properties_set(ETK_OBJECT(abouttext),
			     "focusable", ETK_FALSE, NULL);   
   etk_textblock_text_set(ETK_TEXT_VIEW(abouttext)->textblock,
			  "<b>Code:</b>\n"
			  "Hisham '<b>CodeWarrior</b>' Mardam Bey"
			  "\n\n"
			  "<b>Themes:</b>\n"
			  "Brian 'morlenxus' Miculcy",
			  ETK_TRUE);
   etk_container_add(ETK_CONTAINER(frame), abouttext);
   etk_box_pack_start(ETK_BOX(vbox), frame, ETK_FALSE, ETK_FALSE, 0);   

								 
   etk_dialog_pack_in_main_area(ETK_DIALOG(win), vbox, ETK_FALSE, ETK_FALSE,
				0, ETK_FALSE);
   etk_dialog_button_add(ETK_DIALOG(win), "Close", ETK_RESPONSE_CLOSE);
   etk_container_border_width_set(ETK_CONTAINER(win), 7);
   etk_widget_show_all(win);
   
   etk_textblock_object_cursor_visible_set(ETK_TEXT_VIEW(abouttext)->textblock_object,
					   ETK_FALSE);
   etk_textblock_object_cursor_visible_set(ETK_TEXT_VIEW(desctext)->textblock_object,
					   ETK_FALSE);   
}
