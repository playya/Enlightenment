%{
#include <stdio.h>
#include <string.h>

void yyerror(const char *str) {
	fprintf(stderr, "yyerror: %s\n", str);
}
int yywrap() {
	return 1;
}

%}

%union {
	char *string;
	float val;
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
%token OPEN_BRACE CLOSE_BRACE

%type <string> STRING
%type <val> FLOAT

%%
edjes: images edjes |
       fonts edjes |
       collections edjes |
       ;
collections:  COLLECTIONS OPEN_BRACE group CLOSE_BRACE
	;
fonts:  FONTS OPEN_BRACE statement CLOSE_BRACE
	;
images:  IMAGES OPEN_BRACE statement CLOSE_BRACE
	;
statement: STRING 
	|
	;
group:  statement 
	;
%%
