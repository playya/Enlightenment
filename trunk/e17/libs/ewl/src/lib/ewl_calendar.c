#include <Ewl.h>
#include "ewl_debug.h"
#include "ewl_macros.h"
#include "ewl_private.h"

static void ewl_calendar_grid_setup(Ewl_Calendar *cal);
static int ewl_calendar_leap_year_detect(unsigned int year);
static void ewl_calendar_highlight_today(struct tm *now, Ewl_Label *day, 
						Ewl_Calendar *cal);
static void ewl_calendar_day_select(Ewl_Widget *w, void *ev_data, 
					void *user_data);
static void ewl_calendar_day_pick(Ewl_Widget *w, void *ev_data, 
					void *user_data);
static void ewl_calendar_prev_month_cb(Ewl_Widget *w, void *ev_data, 
					void *user_data);
static void ewl_calendar_next_month_cb(Ewl_Widget *w, void *ev_data, 
					void *user_data);
static void ewl_calendar_add_day_labels(Ewl_Calendar *ib);


static char *months[] = {"January", "February", "March", "April", "May",
			"June", "July", "August", "September", "October",
			"November", "December"};

static int mdays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/**
 * @return Returns NULL on failure, a new Ewl_Calendar on success
 * @brief Creates a new Ewl_Calendar
 *
 * Creates a new Ewl_Calendar object
 */
Ewl_Widget *
ewl_calendar_new(void) 
{
	Ewl_Calendar* ib;
	DENTER_FUNCTION(DLEVEL_STABLE);

	ib = NEW(Ewl_Calendar, 1);
	if (!ib) {
		DRETURN_PTR(NULL, DLEVEL_STABLE);
	}

	if (!ewl_calendar_init(ib)) {
		ewl_widget_destroy(EWL_WIDGET(ib));
		ib = NULL;
	}

	DRETURN_PTR(EWL_WIDGET(ib), DLEVEL_STABLE);
}

/**
 * @param ib: The calendar widget to initialize
 * @return Returns FALSE on failure, a TRUE on success
 * @brief Init a new Ewl_Calendar to default values and callbacks, and set date to today
 */
int
ewl_calendar_init(Ewl_Calendar* ib) 
{
	Ewl_Widget *w, *vbox, *top_hbox, *prev_button, *next_button;
	struct tm *ptr;
	time_t tm;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("ib", ib, FALSE);

	w = EWL_WIDGET(ib);

	if (!ewl_box_init(EWL_BOX(ib))) {
		DRETURN_INT(FALSE, DLEVEL_STABLE);
	}

	ewl_box_orientation_set(EWL_BOX(ib), EWL_ORIENTATION_HORIZONTAL);
	ewl_widget_appearance_set(EWL_WIDGET(ib), "calendar");
	ewl_widget_inherit(EWL_WIDGET(w), "calendar");
	ewl_object_fill_policy_set(EWL_OBJECT(ib), EWL_FLAG_FILL_FILL);

	vbox = ewl_vbox_new();
	ewl_container_child_append(EWL_CONTAINER(ib), vbox);
	ewl_object_minimum_w_set(EWL_OBJECT(vbox), 150);
	ewl_object_fill_policy_set(EWL_OBJECT(vbox), EWL_FLAG_FILL_VSHRINK);
	ewl_widget_show(vbox);

	top_hbox = ewl_hbox_new();
	ewl_container_child_append(EWL_CONTAINER(vbox), top_hbox);
	ewl_widget_show(top_hbox);

	prev_button = ewl_button_new();
	ewl_container_child_append(EWL_CONTAINER(top_hbox), prev_button);
	ewl_button_label_set(EWL_BUTTON(prev_button), "<");
	ewl_object_maximum_size_set(EWL_OBJECT(prev_button), 20,10);
	ewl_callback_append(prev_button, EWL_CALLBACK_MOUSE_DOWN, ewl_calendar_prev_month_cb, ib);
	ewl_widget_show(prev_button);

	ib->month_label = ewl_label_new();
	ewl_container_child_append(EWL_CONTAINER(top_hbox), ib->month_label);
	ewl_label_text_set(EWL_LABEL(ib->month_label), "Disp");
	ewl_object_maximum_h_set(EWL_OBJECT(ib->month_label), 10);
	ewl_widget_show(ib->month_label);

	next_button = ewl_button_new();
	ewl_container_child_append(EWL_CONTAINER(top_hbox), next_button);	
	ewl_button_label_set(EWL_BUTTON(next_button), ">");
	ewl_object_maximum_size_set(EWL_OBJECT(next_button), 20,10);
	ewl_callback_append(next_button, EWL_CALLBACK_MOUSE_DOWN, ewl_calendar_next_month_cb, ib);
	ewl_widget_show(next_button);

	ib->grid = ewl_grid_new(7, 7);
	ewl_container_child_append(EWL_CONTAINER(vbox), ib->grid);
	ewl_object_fill_policy_set(EWL_OBJECT(ib->grid), EWL_FLAG_FILL_FILL);
	ewl_object_minimum_h_set(EWL_OBJECT(ib->grid), 100);
	ewl_widget_show(ib->grid);

	/* Get the start time.. */
	tm = time(NULL);
	ptr = localtime(&tm);

	ib->cur_month = ptr->tm_mon;
	ib->cur_day  = ptr->tm_mday;
	ib->cur_year = ptr->tm_year + 1900;

	ewl_calendar_grid_setup(ib);

	DRETURN_INT(TRUE, DLEVEL_STABLE);
}

/**
 * @param cal: The calendar to get the date frm
 * @param str: a pre-initialized char * pointer to insert the date into
 * @return none 
 * @brief Returns an ASCII formatted representation of the selected date
 *
 * Inserts an ASCII formatted string of the currently selected date into the char* str pointer
 */
void
ewl_calendar_ascii_time_get(Ewl_Calendar *cal, char *str) 
{
	time_t tm;
	struct tm* month_start;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("cal", cal);
	DCHECK_PARAM_PTR("str", str);
	DCHECK_TYPE("cal", cal, "calendar");

	tm = time(NULL);
	month_start = localtime(&tm);
	month_start->tm_mday = cal->cur_day;
	month_start->tm_mon = cal->cur_month;
	month_start->tm_year = cal->cur_year - 1900;
	mktime(month_start);

	strcpy(str, asctime(month_start));
	
	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param c: The Ewl_Calendar to get the day from
 * @return Returns the day currently selected in the calendar
 */
int
ewl_calendar_day_get(Ewl_Calendar *c) 
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("c", c, FALSE);
	DCHECK_TYPE_RET("c", c, "calendar", FALSE);

	DRETURN_INT(c->cur_day, DLEVEL_STABLE);
}

/**
 * @param c: The Ewl_Calendar to get the month from
 * @return Returns the month currently selected in the calendar
 */
int
ewl_calendar_month_get(Ewl_Calendar *c) 
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("c", c, FALSE);
	DCHECK_TYPE_RET("c", c, "calendar", FALSE);

	DRETURN_INT(c->cur_month, DLEVEL_STABLE);
}

/**
 * @param c: The Ewl_Calendar to get the year from
 * @return Returns the current year selected in the calendar
 */
int
ewl_calendar_year_get(Ewl_Calendar *c) 
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("c", c, FALSE);
	DCHECK_TYPE_RET("c", c, "calendar", FALSE);

	DRETURN_INT(c->cur_year, DLEVEL_STABLE);
}

static void
ewl_calendar_grid_setup(Ewl_Calendar *cal) 
{
	struct tm *month_start;
	struct tm *now;
	char display_top[50];
	time_t tm;
	time_t now_tm;
	int today = 0;
	int cur_row, cur_col, cur_day, days = 30;
	Ewl_Widget *day_label;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("cal", cal);
	DCHECK_TYPE("cal", cal, "calendar");

	ewl_grid_reset(EWL_GRID(cal->grid), 7, 7);
	ewl_calendar_add_day_labels(cal);

	/* Make the initial display.. */
	sprintf(display_top, "%s %d", months[cal->cur_month], cal->cur_year);
	ewl_label_text_set(EWL_LABEL(cal->month_label), display_top);
	today = cal->cur_day;

	/* Get the DOW of the first day of this month */
	tm = time(NULL);
	month_start = localtime(&tm);
	month_start->tm_mday = 0;
	month_start->tm_mon = cal->cur_month;
	month_start->tm_year = cal->cur_year - 1900;
	mktime(month_start);

	/* Now add the days to this month */
	cur_row = 2;
	cur_col = month_start->tm_wday + 1;
	if (cur_col > 7) {
		cur_row = 2;
		cur_col = 1;
	}

	cur_day = 0;
	now_tm = time(NULL);
	now = localtime(&now_tm);

	days = mdays[cal->cur_month];
	/* If february, do leap years... */
	if (cal->cur_month == 1) {
		if (ewl_calendar_leap_year_detect(cal->cur_year))
			days = 29;
		else
			days = 28;
	}

	while (cur_day < days) {
		char day[3];

		if (cur_col > 7) {
			cur_row++;
			cur_col = 1;
		}

		sprintf(day, "%d", cur_day + 1); 
		day_label = ewl_label_new();
		ewl_label_text_set(EWL_LABEL(day_label), day);
		ewl_callback_append(EWL_WIDGET(day_label), 
					EWL_CALLBACK_MOUSE_DOWN, 
					ewl_calendar_day_select, cal);
		ewl_callback_append(EWL_WIDGET(day_label), 
					EWL_CALLBACK_CLICKED,
					ewl_calendar_day_pick, cal);

		ewl_grid_add(EWL_GRID(cal->grid), day_label, cur_col, cur_col,
							cur_row, cur_row);
		ewl_calendar_highlight_today(now, EWL_LABEL(day_label), cal);
		ewl_widget_show(day_label);

		cur_col++;
		cur_day++;
	}

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static int
ewl_calendar_leap_year_detect(unsigned int year)
{
	int leap;

	DENTER_FUNCTION(DLEVEL_STABLE);

	assert(year > 1581);
	leap = (((year % 4 == 0) && (year % 100)) || (year % 400 == 0)); 

	DRETURN_INT(leap, DLEVEL_STABLE);
}

static void
ewl_calendar_highlight_today(struct tm *now, Ewl_Label *day, 
					Ewl_Calendar *cal) 
{
	int i;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("now", now);
	DCHECK_PARAM_PTR("day", day);
	DCHECK_PARAM_PTR("cal", cal);
	DCHECK_TYPE("day", day, "label");
	DCHECK_TYPE("cal", cal, "calendar");

	/* Get the day */
	i = atoi(ewl_label_text_get(EWL_LABEL(day)));
	if ((i == now->tm_mday) && ((now->tm_year + 1900) == cal->cur_year) 
			&& (now->tm_mon == cal->cur_month)) {
		ewl_widget_color_set(EWL_WIDGET(day), 0, 0, 255, 255);
	}

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ewl_calendar_day_select(Ewl_Widget *w, void *ev_data __UNUSED__, 
						void *user_data)
{
	struct tm *now;
	time_t now_tm;
	int i;
	Ewl_Widget* it;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_PARAM_PTR("user_data", user_data);
	DCHECK_TYPE("w", w, "widget");

	now_tm = time(NULL);
	now = localtime(&now_tm);
	i = atoi(ewl_label_text_get(EWL_LABEL(w)));

	ewl_container_child_iterate_begin(EWL_CONTAINER(EWL_CALENDAR(user_data)->grid));
	while ((it = ewl_container_child_next(EWL_CONTAINER(EWL_CALENDAR(user_data)->grid))) != NULL) {
		ewl_widget_color_set(EWL_WIDGET(it), 255, 255, 255, 255);
		ewl_calendar_highlight_today(now, EWL_LABEL(it), EWL_CALENDAR(user_data));
	}

	ewl_widget_color_set(w, 255, 0, 0, 255);
	EWL_CALENDAR(user_data)->cur_day = i;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ewl_calendar_day_pick(Ewl_Widget *w __UNUSED__, void *ev_data, 
						void *user_data) 
{
	Ewl_Event_Mouse_Down *ev;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("user_data", user_data);

	ev = ev_data;
	if (ev->clicks == 2)
		ewl_callback_call(EWL_WIDGET(user_data),
				EWL_CALLBACK_VALUE_CHANGED);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ewl_calendar_prev_month_cb(Ewl_Widget *w __UNUSED__, void *ev_data __UNUSED__,
						void *user_data) 
{
	Ewl_Calendar *ib;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("user_data", user_data);

	ib = EWL_CALENDAR(user_data);
	ib->cur_month -= 1;
	if (ib->cur_month < 0) { 
		ib->cur_month = 11; 
		ib->cur_year--; 
	}
	ewl_calendar_grid_setup(ib);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ewl_calendar_next_month_cb(Ewl_Widget *w __UNUSED__, void *ev_data __UNUSED__,
						void *user_data) 
{
	Ewl_Calendar *ib;
	
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("user_data", user_data);

	ib = EWL_CALENDAR(user_data);
	ib->cur_month += 1;
	if (ib->cur_month > 11) { 
		ib->cur_month = 0; 
		ib->cur_year++; 
	}
	ewl_calendar_grid_setup(ib);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ewl_calendar_add_day_labels(Ewl_Calendar *ib) 
{
	Ewl_Widget* day_label;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("ib", ib);
	DCHECK_TYPE("ib", ib, "calendar");

	/* Add the days*/
	day_label = ewl_label_new();
	ewl_label_text_set(EWL_LABEL(day_label), "M");
	ewl_grid_add(EWL_GRID(ib->grid), day_label, 1, 1, 1, 1);
	ewl_widget_show(day_label);

	day_label = ewl_label_new();
	ewl_label_text_set(EWL_LABEL(day_label), "T");
	ewl_grid_add(EWL_GRID(ib->grid), day_label, 2, 2, 1, 1);
	ewl_widget_show(day_label);

	day_label = ewl_label_new();
	ewl_label_text_set(EWL_LABEL(day_label), "W");
	ewl_grid_add(EWL_GRID(ib->grid), day_label, 3, 3, 1, 1);
	ewl_widget_show(day_label);

	day_label = ewl_label_new();
	ewl_label_text_set(EWL_LABEL(day_label), "T");
	ewl_grid_add(EWL_GRID(ib->grid), day_label, 4, 4, 1, 1);
	ewl_widget_show(day_label);

	day_label = ewl_label_new();
	ewl_label_text_set(EWL_LABEL(day_label), "F");
	ewl_grid_add(EWL_GRID(ib->grid), day_label, 5, 5, 1, 1);
	ewl_widget_show(day_label);

	day_label = ewl_label_new();
	ewl_label_text_set(EWL_LABEL(day_label), "S");
	ewl_grid_add(EWL_GRID(ib->grid), day_label, 6, 6, 1, 1);
	ewl_widget_show(day_label);

	day_label = ewl_label_new();
	ewl_label_text_set(EWL_LABEL(day_label), "S");
	ewl_grid_add(EWL_GRID(ib->grid), day_label, 7, 7, 1, 1);
	ewl_widget_show(day_label);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

