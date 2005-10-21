typedef struct _Ecore_DirectFB_Key_Symbols Ecore_DirectFB_Key_Symbols;

struct _Ecore_DirectFB_Key_Symbols
{
	char *keysymbol;
	unsigned int keycode;
} _ecore_directfb_key_symbols[] = {
	{"BackSpace", DIKS_BACKSPACE},
	{"Tab", DIKS_TAB},
	{"Return", DIKS_RETURN},
	{"Cancel", DIKS_CANCEL},
	{"Escape", DIKS_ESCAPE},
	{"space", DIKS_SPACE},
	{"exclam", DIKS_EXCLAMATION_MARK},
	{"quotedbl", DIKS_QUOTATION},
	{"numbersign", DIKS_NUMBER_SIGN},
	{"dollar", DIKS_DOLLAR_SIGN},
	{"percent", DIKS_PERCENT_SIGN},
	{"ampersand", DIKS_AMPERSAND},
	{"apostrophe", DIKS_APOSTROPHE},
	{"parenleft", DIKS_PARENTHESIS_LEFT},
	{"parenright", DIKS_PARENTHESIS_RIGHT},
	{"asterisk", DIKS_ASTERISK},
	{"plus", DIKS_PLUS_SIGN},
	{"comma", DIKS_COMMA},
	{"minus", DIKS_MINUS_SIGN},
	{"period", DIKS_PERIOD},
	{"slash", DIKS_SLASH},
	{"0", DIKS_0},
	{"1", DIKS_1},
	{"2", DIKS_2},
	{"3", DIKS_3},
	{"4", DIKS_4},
	{"5", DIKS_5},
	{"6", DIKS_6},
	{"7", DIKS_7},
	{"8", DIKS_8},
	{"9", DIKS_9},
	{"colon", DIKS_COLON},
	{"semicolon", DIKS_SEMICOLON},
	{"less", DIKS_LESS_THAN_SIGN},
	{"equal", DIKS_EQUALS_SIGN},
	{"greater", DIKS_GREATER_THAN_SIGN},
	{"question", DIKS_QUESTION_MARK},
	{"at", DIKS_AT},
	{"A", DIKS_CAPITAL_A },
	{"B", DIKS_CAPITAL_B },
	{"C", DIKS_CAPITAL_C },
	{"D", DIKS_CAPITAL_D },
	{"E", DIKS_CAPITAL_E },
	{"F", DIKS_CAPITAL_F },
	{"G", DIKS_CAPITAL_G },
	{"H", DIKS_CAPITAL_H },
	{"I", DIKS_CAPITAL_I },
	{"J", DIKS_CAPITAL_J },
	{"K", DIKS_CAPITAL_K },
	{"L", DIKS_CAPITAL_L },
	{"M", DIKS_CAPITAL_M },
	{"N", DIKS_CAPITAL_N },
	{"O", DIKS_CAPITAL_O },
	{"P", DIKS_CAPITAL_P },
	{"Q", DIKS_CAPITAL_Q },
	{"R", DIKS_CAPITAL_R },
	{"S", DIKS_CAPITAL_S },
	{"T", DIKS_CAPITAL_T },
	{"U", DIKS_CAPITAL_U },
	{"V", DIKS_CAPITAL_V },
	{"W", DIKS_CAPITAL_W },
	{"X", DIKS_CAPITAL_X },
	{"Y", DIKS_CAPITAL_Y },
	{"Z", DIKS_CAPITAL_Z },
	{"bracketleft", DIKS_SQUARE_BRACKET_LEFT },
	{"backslash", DIKS_BACKSLASH },
	{"bracketright", DIKS_SQUARE_BRACKET_RIGHT },
	{"asciicircum", DIKS_CIRCUMFLEX_ACCENT },
	{"underscore", DIKS_UNDERSCORE },
	{"grave", DIKS_GRAVE_ACCENT},
	{"a", DIKS_SMALL_A },
	{"b", DIKS_SMALL_B },
	{"c", DIKS_SMALL_C },
	{"d", DIKS_SMALL_D },
	{"e", DIKS_SMALL_E },
	{"f", DIKS_SMALL_F },
	{"g", DIKS_SMALL_G },
	{"h", DIKS_SMALL_H },
	{"i", DIKS_SMALL_I },
	{"j", DIKS_SMALL_J },
	{"k", DIKS_SMALL_K },
	{"l", DIKS_SMALL_L },
	{"m", DIKS_SMALL_M },
	{"n", DIKS_SMALL_N },
	{"o", DIKS_SMALL_O },
	{"p", DIKS_SMALL_P },
	{"q", DIKS_SMALL_Q },
	{"r", DIKS_SMALL_R },
	{"s", DIKS_SMALL_S },
	{"t", DIKS_SMALL_T },
	{"u", DIKS_SMALL_U },
	{"v", DIKS_SMALL_V },
	{"w", DIKS_SMALL_W },
	{"x", DIKS_SMALL_X },
	{"y", DIKS_SMALL_Y },
	{"z", DIKS_SMALL_Z },
	{"braceleft", DIKS_CURLY_BRACKET_LEFT },
	{"bar", DIKS_VERTICAL_BAR },
	{"braceright", DIKS_CURLY_BRACKET_RIGHT },
	{"asciitilde", DIKS_TILDE },
	{"Delete", DIKS_DELETE },
	{"Left", DIKS_CURSOR_LEFT },
	{"Right", DIKS_CURSOR_RIGHT},
	{"Up", DIKS_CURSOR_UP},
	{"Down", DIKS_CURSOR_DOWN},
	{"Insert", DIKS_INSERT},
	{"Home", DIKS_HOME},
	{"End", DIKS_END},
	{"Page_Up", DIKS_PAGE_UP},
	{"Page_Down", DIKS_PAGE_DOWN},
	{"Print", DIKS_PRINT},
	{"Pause", DIKS_PAUSE},
	/* ok */
	{"Select", DIKS_SELECT},
	/* goto */
	{"Clear", DIKS_CLEAR},
	/* power */
	/* power 2 */
	/* option */
	{"Menu", DIKS_MENU},
	{"Help", DIKS_HELP},
	/* info */
	/* time */
	/* vendor */
	/* archive */
	/* program */
	/* channel */
	/* favorites */
	/* hasta next */
	{"Next", DIKS_NEXT},
	{"Begin", DIKS_BEGIN},
	/* digits */
	/* teen */
	/* twen */
	{"Break", DIKS_BREAK},
	/* exit */
	/* setup */
	{"upleftcorner", DIKS_CURSOR_LEFT_UP },
	{"lowleftcorner", DIKS_CURSOR_LEFT_DOWN },
	{"uprightcorner", DIKS_CURSOR_UP_RIGHT },
	{"lowrightcorner", DIKS_CURSOR_DOWN_RIGHT },
	{"F1", DIKS_F1},
	{"F2", DIKS_F2},
	{"F3", DIKS_F3},
	{"F4", DIKS_F4},
	{"F5", DIKS_F5},
	{"F6", DIKS_F6},
	{"F7", DIKS_F7},
	{"F8", DIKS_F8},
	{"F9", DIKS_F9},
	{"F10", DIKS_F10},
	{"F11", DIKS_F11},
	{"F12", DIKS_F12},
	/* this are only mapped to one, not left right */
	{"Shift_L", DIKS_SHIFT},
	/*{"Shift_R", 0xFFE2}, */
	{"Control_L", DIKS_CONTROL},
	/*{"Control_R", 0xFFE4}, */
	{"Meta_L", DIKS_META},
	/* {"Meta_R", 0xFFE8}, */
	{"Alt_L", DIKS_ALT},
	{"Alt_R", DIKS_ALTGR},
	{"Super_L", DIKS_SUPER},
	/*{"Super_R", 0xFFEC}, */
	{"Hyper_L", DIKS_HYPER},
	/*{"Hyper_R", 0xFFEE}, */
		
	{"Caps_Lock", DIKS_CAPS_LOCK},
	{"Num_Lock", DIKS_NUM_LOCK},
	{"Scroll_Lock", DIKS_SCROLL_LOCK},
	/* not included the dead keys */
	/* not included the custom keys */
	{"VoidSymbol", DIKS_NULL}
};
