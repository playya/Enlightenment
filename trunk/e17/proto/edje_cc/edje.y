%{
#include <stdio.h>
#include <string.h>
#include "Etcher.h"

	#define YYDEBUG 1

	void yyerror(const char *s);
	void parse_error(void);

	extern char *cur_file;
	extern int lnum;
	extern int col;

	int section;

%}

%union {
	char *string;
	float val;
	Etcher_Action prog_actions;
	Etcher_Transition transition_type;
	Etcher_Part_Type part_type;
	Etcher_Image_Type image_type;
	Etcher_Text_Effect text_effect;
	Etcher_Aspect_Preference aspect_pref;
}

%token STRING FLOAT
%token ACTION AFTER ALIGN ASPECT ASPECT_PREFERENCE BORDER
%token CLIP_TO COLLECTIONS COLOR COLOR2 COLOR3 COLOR_CLASS
%token CONFINE DATA DESCRIPTION DRAGABLE EFFECT FILL FIT
%token FONT FONTS GROUP GROUPS IMAGE IMAGES IN ITEM MAX MIN MOUSE_EVENTS
%token NAME NORMAL OFFSET ORIGIN PART PARTS PROGRAM PROGRAMS
%token REL1 REL2 RELATIVE REPEAT_EVENTS SCRIPT SIGNAL SIZE
%token SMOOTH SOURCE STATE STEP TARGET TEXT TEXT_CLASS TO
%token TO_X TO_Y TRANSITION TWEEN TYPE VISIBLE X Y
%token OPEN_BRACE CLOSE_BRACE RAW COMP LOSSY
%token COLON QUOTE SEMICOLON STATE_SET ACTION_STOP SIGNAL_EMIT
%token DRAG_VAL_SET DRAG_VAL_STEP DRAG_VAL_PAGE LINEAR
%token SINUSOIDAL ACCELERATE DECELERATE IMAGE RECT SWALLOW
%token NONE PLAIN OUTLINE SOFT_OUTLINE SHADOW SOFT_SHADOW USER
%token OUTLINE_SHADOW OUTLINE_SOFT_SHADOW VERTICAL HORIZONTAL BOTH
%left MINUS PLUS
%left TIMES DIVIDE
%left NEG     /* negation--unary minus */
%token OPEN_PAREN CLOSE_PAREN DOT

%type <string> STRING 
%type <val> FLOAT
%type <prog_actions> action_type
%type <transition_type> transition_type
%type <part_type> part_type
%type <image_type> image_type
%type <text_effect> effect_type
%type <aspect_pref> aspect_pref_type
%type <val> exp

%%

edjes: /* blank */
	| images edjes 
	| fonts edjes
	| collections edjes
	| data edjes
	| error {
		parse_error();
		yyerrok;
		yyclearin;
	}
	;

collections:  COLLECTIONS OPEN_BRACE {section = GROUPS; } collection_statement CLOSE_BRACE { section = BASE; }
	;

fonts:  FONTS OPEN_BRACE { section = FONTS; } font_statement CLOSE_BRACE { section = BASE; }
	;

font_statement: /* empty */
	| font
	| font font_statement
	;

font: FONT COLON STRING STRING SEMICOLON {
                etcher_parse_font($3, $4);
	}
	;

images:  IMAGES OPEN_BRACE { section = IMAGES; } image_statement CLOSE_BRACE { section = BASE; }
	;

image_statement: /* empty */
	| image
	| image image_statement
	;

image: IMAGE COLON STRING image_type SEMICOLON {
                etcher_parse_image($3, $4, 0);
	}
	| IMAGE COLON STRING image_type exp SEMICOLON {
                etcher_parse_image($3, $4, $5);
	}
	;

image_type: RAW { $$ = ETCHER_IMAGE_TYPE_RAW; }
	| COMP { $$ = ETCHER_IMAGE_TYPE_COMP; }
	| LOSSY { $$ = ETCHER_IMAGE_TYPE_LOSSY; }
	| USER { $$ = ETCHER_IMAGE_TYPE_EXTERNAL; }
	;

/* don't set a section here yet (since BASE and GROUP have data sects) */
data:  DATA OPEN_BRACE data_statement CLOSE_BRACE
	;

data_statement: /* empty */
	| item 
	| item data_statement
	;

item: ITEM COLON STRING STRING SEMICOLON {
                switch (section)
                {
                  case BASE:
                    etcher_parse_data($3, $4, 0);
                    break;
                  case GROUP:
                    etcher_parse_group_data($3, $4, 0);
                    break;
                  default:
                    break;
                }
	}
	| ITEM COLON STRING exp SEMICOLON {
                switch (section)
                {
                  case BASE:
                    etcher_parse_data($3, NULL, (int)$4);
                    break;
                  case GROUP:
                    etcher_parse_group_data($3, NULL, (int)$4);
                    break;
                  default:
                    break;
                }
	}
	;

programs: PROGRAMS OPEN_BRACE { section = PROGRAMS; } program_statement CLOSE_BRACE { section = BASE; }
	;

program_statement: /* empty */
	| program
	| program program_statement
	;

program: PROGRAM OPEN_BRACE { etcher_parse_program(); section = PROGRAM; } program_body CLOSE_BRACE { section = PROGRAMS; }
	;

program_body: /* blank */ 
	| program_cmd
	| program_cmd program_body
	;

program_cmd: name
	| program_signal
	| program_source
	| program_in
	| program_action
	| program_transition
	| program_target
	| program_after
	| script
	;

name: NAME COLON STRING SEMICOLON {
                switch(section)
                {
                  case GROUP:
                    etcher_parse_group_name($3);
                    break;
                  case PART:
                    etcher_parse_part_name($3);
                    break;
                  case PROGRAM:
                    etcher_parse_program_name($3);
                    break;
                  default:
                    break;
                }
	}
	;

program_signal: SIGNAL COLON STRING SEMICOLON {
                etcher_parse_program_signal($3);
	}
	;

program_source: SOURCE COLON STRING SEMICOLON {
                etcher_parse_program_source($3);
	}
	;

program_in: IN COLON exp exp SEMICOLON {
                etcher_parse_program_in($3, $4);
	}
	;

program_action: ACTION COLON action_type STRING exp SEMICOLON {
                etcher_parse_program_action($3, $4, NULL, $5, 0);
	}
	| ACTION COLON action_type STRING STRING SEMICOLON {
                etcher_parse_program_action($3, $4, $5, 0, 0);
	}
	| ACTION COLON action_type exp exp SEMICOLON {
                etcher_parse_program_action($3, NULL, NULL, $4, $5);
	}
	| ACTION COLON action_type SEMICOLON {
                etcher_parse_program_action($3, NULL, NULL, 0, 0);
	}
	;

action_type: SIGNAL_EMIT { $$ = ETCHER_ACTION_SIGNAL_EMIT; }
	| STATE_SET { $$ = ETCHER_ACTION_STATE_SET; }
	| ACTION_STOP { $$ = ETCHER_ACTION_STOP; }
	| DRAG_VAL_SET { $$ = ETCHER_ACTION_DRAG_VAL_SET; }
	| DRAG_VAL_STEP { $$ = ETCHER_ACTION_DRAG_VAL_STEP; }
	| DRAG_VAL_PAGE { $$ = ETCHER_ACTION_DRAG_VAL_PAGE; }
	;

program_transition: TRANSITION COLON transition_type exp SEMICOLON {
                etcher_parse_program_transition($3, $4);
	}
	;

transition_type: LINEAR { $$ = ETCHER_TRANSITION_LINEAR; }
	| SINUSOIDAL { $$ = ETCHER_TRANSITION_SINUSOIDAL; }
	| ACCELERATE { $$ = ETCHER_TRANSITION_ACCELERATE; }
	| DECELERATE { $$ = ETCHER_TRANSITION_DECELERATE; }
	;

program_target: TARGET COLON STRING SEMICOLON {
                etcher_parse_program_target($3);
	}
	;

program_after: AFTER COLON STRING SEMICOLON {
                etcher_parse_program_after($3);
	}
	;

collection_statement: /* empty */
	| group
	| group collection_statement
	;

group: GROUP OPEN_BRACE { etcher_parse_group(); section = GROUP; } group_foo CLOSE_BRACE { section = GROUPS; }
	;

group_foo: 
	| group_preamble group_foo
	| group_body group_foo
	;

group_body: data
	| script
	| parts
	| programs
	;

script: SCRIPT {
                switch (section)
                {
                  case GROUP:
                    etcher_parse_group_script(yylval.string);
                    break;
                  case PROGRAM:
                    etcher_parse_program_script(yylval.string);
                    break;
                  default:
                    break;
                }
                free(yylval.string);
                yylval.string = NULL;
	}
	;

group_preamble: group_preamble_entry
	| group_preamble_entry group_preamble
	;

group_preamble_entry: name
	| min
	| max
	;

min: MIN COLON exp exp SEMICOLON {
                switch(section)
                {
                  case GROUP:
                    etcher_parse_group_min((int)$3, (int)$4);
                    break;
                  case STATE:
                    etcher_parse_state_min((int)$3, (int)$4);
                    break;
                  case TEXT:
                    etcher_parse_state_text_min((int)$3, (int)$4);
                    break;
                  default: 
                    break;
                }
	}
	;

max: MAX COLON exp exp SEMICOLON {
                switch(section)
                {
                  case GROUP:
                    etcher_parse_group_max((int)$3, (int)$4);
                    break;
                  case STATE:
                    etcher_parse_state_max((int)$3, (int)$4);
                    break;
                  default: 
                    break;
                }
	}
	;

parts: PARTS OPEN_BRACE { section = PARTS; } parts_statement CLOSE_BRACE { section = BASE; }
	;

parts_statement: /* empty */
	| part
	| part parts_statement
	;

part: PART OPEN_BRACE { etcher_parse_part(); section = PART; } part_foo CLOSE_BRACE { section = PARTS; }
	;

part_foo: 
	| part_preamble part_foo
	| part_body part_foo
	;

part_preamble: part_preamble_entry
	| part_preamble_entry part_preamble

part_preamble_entry: name
	| type
	| effect
	| mouse_events
	| repeat_events
	| clip_to
	| color_class
	| text_class
    ; 

type: TYPE COLON part_type SEMICOLON {
                etcher_parse_part_type($3);
	}
	;

part_type: IMAGE { $$ = ETCHER_PART_TYPE_IMAGE; }
	| RECT { $$ = ETCHER_PART_TYPE_RECT; }
	| TEXT { $$ = ETCHER_PART_TYPE_TEXT; }
	| SWALLOW { $$ = ETCHER_PART_TYPE_SWALLOW; }
	;

effect: EFFECT COLON effect_type SEMICOLON {
                etcher_parse_part_effect($3);
	}
	;

effect_type: NONE { $$ = ETCHER_TEXT_EFFECT_NONE; }
	| PLAIN { $$ = ETCHER_TEXT_EFFECT_PLAIN; }
	| OUTLINE { $$ = ETCHER_TEXT_EFFECT_OUTLINE; }
	| SOFT_OUTLINE { $$ = ETCHER_TEXT_EFFECT_SOFT_OUTLINE; }
	| SHADOW { $$ = ETCHER_TEXT_EFFECT_SHADOW; }
	| SOFT_SHADOW { $$ = ETCHER_TEXT_EFFECT_SOFT_SHADOW; }
	| OUTLINE_SOFT_SHADOW { $$ = ETCHER_TEXT_EFFECT_OUTLINE_SOFT_SHADOW; }
	;

mouse_events: MOUSE_EVENTS COLON exp SEMICOLON {
                etcher_parse_part_mouse_events((int)$3);
	}
	;

repeat_events: REPEAT_EVENTS COLON exp SEMICOLON {
                etcher_parse_part_repeat_events((int)$3);
	}
	;

clip_to: CLIP_TO COLON STRING SEMICOLON {
                etcher_parse_part_clip_to($3);
	}
	;

color_class: COLOR_CLASS COLON STRING SEMICOLON {
                etcher_parse_state_color_class($3);
	}
	;

text_class: TEXT_CLASS COLON STRING SEMICOLON {
                etcher_parse_state_text_text_class($3);
	}
	;

part_body: part_body_entry
	| part_body_entry part_body
	;

part_body_entry: dragable
	| description
	;

dragable: DRAGABLE OPEN_BRACE { section = DRAGABLE; } dragable_statement CLOSE_BRACE { section = PART; }
	;

dragable_statement: /* empty */
	| dragable_body
	| dragable_body dragable_statement
	;

dragable_body: x
	| y
	| confine
	;

x: X COLON exp exp exp SEMICOLON {
                etcher_parse_part_dragable_x((int)$3, (int)$4, (int)$5);
	}
	;

y: Y COLON exp exp exp SEMICOLON {
                etcher_parse_part_dragable_y((int)$3, (int)$4, (int)$5);
	}
	;

confine: CONFINE COLON STRING SEMICOLON {
                etcher_parse_part_dragable_confine($3);
	}
	;

description: DESCRIPTION OPEN_BRACE { etcher_parse_state(); section = STATE; } desc_foo CLOSE_BRACE { section = PART; }
	;

desc_foo:
	| desc_preamble desc_foo
	| desc_body desc_foo
	;

desc_preamble: desc_preamble_entry
	| desc_preamble_entry desc_preamble
	;

desc_preamble_entry: state
	| visible
	| align
	| min
	| max
	| step
	| aspect
	| aspect_preference
	;

state: STATE COLON STRING exp SEMICOLON {
                etcher_parse_state_name($3, $4);
	}
	;

visible: VISIBLE COLON exp SEMICOLON {
                etcher_parse_state_visible((int)$3);
	}
	;

align: ALIGN COLON exp exp SEMICOLON {
                switch(section)
                {
                  case STATE:
                    etcher_parse_state_align($3, $4);
                    break;
                  case TEXT:
                    etcher_parse_state_text_align($3, $4);
                    break;
                  default:
                    break;
                }
	}
	;

step: STEP COLON exp exp SEMICOLON {
                etcher_parse_state_step($3, $4);
	}
	;

aspect: ASPECT COLON exp exp SEMICOLON {
                etcher_parse_state_aspect($3, $4);
	}
	;

aspect_preference: ASPECT_PREFERENCE COLON aspect_pref_type SEMICOLON {
                etcher_parse_state_aspect_preference($3);
	}
	;

aspect_pref_type: NONE { $$ = ETCHER_ASPECT_PREFERENCE_NONE; }
	| VERTICAL { $$ = ETCHER_ASPECT_PREFERENCE_VERTICAL; }
	| HORIZONTAL { $$ = ETCHER_ASPECT_PREFERENCE_HORIZONTAL; }
	| BOTH { $$ = ETCHER_ASPECT_PREFERENCE_BOTH; }
	;

desc_body: desc_body_entry
	| desc_body_entry desc_body
	;

desc_body_entry: rel1
	| rel2
	| image
	| border
	| fill
	| color_class
	| color
	| color2
	| color3
	| text
	;

rel1: REL1 OPEN_BRACE {section = REL1;} rel_statement CLOSE_BRACE {section = STATE;}
	| REL1 DOT {section = REL1;} rel_body {section = STATE;}
	;

rel2: REL2 OPEN_BRACE {section = REL2;} rel_statement CLOSE_BRACE {section = STATE;}
	| REL2 DOT {section = REL2;} rel_body {section = STATE;}
	;

rel_statement: /* empty */ 
	| rel_body
	| rel_body rel_statement
	;

rel_body: relative
	| offset
	| to
	| to_x
	| to_y
	;

relative: RELATIVE COLON exp exp SEMICOLON {
                switch(section)
                {
                  case REL1:
                    etcher_parse_state_rel1_relative($3, $4);
                    break;
                  case REL2:
                    etcher_parse_state_rel2_relative($3, $4);
                    break;
                  case ORIGIN:
                    etcher_parse_state_fill_origin_relative($3, $4);
                    break;
                  case SIZE:
                    etcher_parse_state_fill_size_relative($3, $4);
                    break;
                  default: 
                    break;
                }
	}
	;

offset: OFFSET COLON exp exp SEMICOLON {
                switch(section)
                {
                  case REL1:
                    etcher_parse_state_rel1_offset((int)$3, (int)$4);
                    break;
                  case REL2:
                    etcher_parse_state_rel2_offset((int)$3, (int)$4);
                    break;
                  case ORIGIN:
                    etcher_parse_state_fill_origin_offset((int)$3, (int)$4);
                    break;
                  case SIZE:
                    etcher_parse_state_fill_size_offset((int)$3, (int)$4);
                    break;
                  default: 
                    break;
                }
	}
	;

to: TO COLON STRING SEMICOLON {
                switch(section)
                {
                  case REL1:
                    etcher_parse_state_rel1_to($3);
                    break;
                  case REL2:
                    etcher_parse_state_rel2_to($3);
                    break;
                  default: 
                    fprintf(stderr, "Error: \"to\" not allowed here %d, %d", lnum, col);
                }
	}
	;

to_x: TO_X COLON STRING SEMICOLON {
                switch(section)
                {
                  case REL1:
                    etcher_parse_state_rel1_to_x($3);
                    break;
                  case REL2:
                    etcher_parse_state_rel2_to_x($3);
                    break;
                  default: 
                    break;
                }
	}
	;

to_y: TO_Y COLON STRING SEMICOLON {
                switch(section)
                {
                  case REL1:
                    etcher_parse_state_rel1_to_y($3);
                    break;
                  case REL2:
                    etcher_parse_state_rel2_to_y($3);
                    break;
                  default: 
                    break;
                }
	}
	;

image: IMAGE OPEN_BRACE { section = IMAGE; } image_statement CLOSE_BRACE { section = STATE; }
	| IMAGE DOT { section = IMAGE; } image_body { section = STATE; }
	;

image_statement: /* empty */ 
	| image_body
	| image_body image_statement
	;

image_body: normal
	| tween
	;

normal: NORMAL COLON STRING SEMICOLON {
                etcher_parse_state_image_normal($3);
	}
	;

tween: TWEEN COLON STRING SEMICOLON {
                etcher_parse_state_image_tween($3);
	}
	;

border: BORDER COLON exp exp exp exp SEMICOLON {
                etcher_parse_state_border((int)$3, (int)$4, (int)$5, (int)$6);
	}
	;

fill: FILL OPEN_BRACE { section = FILL; } fill_statement CLOSE_BRACE { section = STATE; }
	| FILL DOT {section = FILL; } fill_body { section = STATE; }
	;

fill_statement: /* empty */
	| fill_body
	| fill_body fill_statement
	;

fill_body: smooth
	| origin
	| size
	;

smooth: SMOOTH COLON exp SEMICOLON {
                etcher_parse_state_fill_smooth((int)$3);
	}
	;

origin: ORIGIN OPEN_BRACE { section = ORIGIN; } origin_statement CLOSE_BRACE { section = FILL; }
	| ORIGIN DOT { section = ORIGIN; } origin_body { section = FILL; }
	;

origin_statement: /* empty */
	| origin_body
	| origin_statement origin_body
	;

origin_body: relative
	| offset
	;

size: SIZE OPEN_BRACE { section = SIZE; } origin_statement CLOSE_BRACE { section = FILL; }
	| SIZE DOT { section = SIZE; } origin_body { section = FILL; }
	;

color_class: COLOR_CLASS COLON STRING SEMICOLON {
                etcher_parse_state_color_class($3);
	}
	;

color: COLOR COLON exp exp exp exp SEMICOLON {
                etcher_parse_state_color((int)$3, (int)$4, (int)$5, (int)$6);
	}
	;

color2: COLOR2 COLON exp exp exp exp SEMICOLON {
                etcher_parse_state_color2((int)$3, (int)$4, (int)$5, (int)$6);
	}
	;
		
color3: COLOR3 COLON exp exp exp exp SEMICOLON {
                etcher_parse_state_color3((int)$3, (int)$4, (int)$5, (int)$6);
	}
	;

text: TEXT OPEN_BRACE { section = TEXT; } text_statement CLOSE_BRACE { section = STATE; }
	| TEXT DOT { section = TEXT; } text_body { section = STATE; }
	;

text_statement: /* empty */
	| text_body
	| text_body text_statement
	;

text_body: text_entry
	| text_class
	| font_entry
	| size_entry
	| fit
	| min
	| align
	;

text_entry: TEXT COLON STRING SEMICOLON {
                etcher_parse_state_text_text($3);
	}
	;

text_class: TEXT_CLASS COLON STRING SEMICOLON {
                etcher_parse_state_text_text_class($3);
	}
	;

font_entry: FONT COLON STRING SEMICOLON {
                etcher_parse_state_text_font($3);
	}
	;

size_entry: SIZE COLON exp SEMICOLON {
                etcher_parse_state_text_size((int)$3);
	}
	;

fit: FIT COLON exp exp SEMICOLON {
                etcher_parse_state_text_fit((int)$3, (int)$4);
	}
	;

exp: FLOAT                          { $$ = $1;          }
	| exp PLUS exp                  { $$ = $1 + $3;     }
	| exp MINUS exp                 { $$ = $1 - $3;     }
	| exp TIMES exp                 { $$ = $1 * $3;     }
	| exp DIVIDE exp                { $$ = $1 / $3;     }
	| MINUS exp %prec NEG           { $$ = -$2;         }
	| OPEN_PAREN exp CLOSE_PAREN    { $$ = $2;          }
	;

%%

void
yyerror(const char *str)
{
	fprintf(stderr, "yyerror: %s\n", str);
}

int
yywrap()
{
	return 1;
}

void
parse_error(void)
{
	fprintf(stderr, "file: %s, line: %d, column: %d\n\n",
                                     cur_file, lnum, col);
	exit(-1);
}


