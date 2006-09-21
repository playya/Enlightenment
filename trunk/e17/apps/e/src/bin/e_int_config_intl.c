/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

typedef struct _E_Intl_Pair E_Intl_Pair;
typedef struct _E_Intl_Langauge_Node E_Intl_Language_Node;
typedef struct _E_Intl_Region_Node E_Intl_Region_Node;

static void *_create_data(E_Config_Dialog *cfd);
static void _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int _advanced_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int _basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);

static void _ilist_basic_language_cb_change(void *data, Evas_Object *obj);

static void _ilist_language_cb_change(void *data, Evas_Object *obj);
static void _ilist_region_cb_change(void *data, Evas_Object *obj);
static void _ilist_codeset_cb_change(void *data, Evas_Object *obj);
static void _ilist_modifier_cb_change(void *data, Evas_Object *obj);

/* Fill the clear lists, fill with language, select */
/* Update lanague */
static void  _cfdata_language_go(const char *lang, const char *region, const char *codeset, const char *modifier, E_Config_Dialog_Data *cfdata);
static Evas_Bool _lang_hash_cb(Evas_Hash *hash, const char *key, void *data, void *fdata);
static Evas_Bool _region_hash_cb(Evas_Hash *hash, const char *key, void *data, void *fdata);

static Evas_Bool _language_hash_free_cb(Evas_Hash *hash, const char *key, void *data, void *fdata);
static Evas_Bool _region_hash_free_cb(Evas_Hash *hash, const char *key, void *data, void *fdata);

struct _E_Intl_Pair
{
   const char	*locale_key;
   const char   *locale_translation;
};

/* We need to store a map of languages -> Countries -> Extra
 *
 * Extra:
 * Each region has its own Encodings
 * Each region has its own Modifiers
 */

struct _E_Intl_Langauge_Node
{
   const char *lang_code;		/* en */
   const char *lang_name;		/* English (trans in ilist) */
   int	       lang_available;		/* defined in e translation */
   Evas_Hash  *region_hash;	        /* US->utf8 */
};

struct _E_Intl_Region_Node
{
   const char *region_code;		/* US */
   const char *region_name;		/* United States */
   Evas_List  *available_codesets;
   Evas_List  *available_modifiers;
};

struct _E_Config_Dialog_Data
{
   E_Config_Dialog *cfd;
   Evas *evas;
   
   /* Current data */
   char	*cur_language;

   char *cur_blang;
   
   char *cur_lang;
   char *cur_reg;
   char *cur_cs;
   char *cur_mod;

   Evas_Hash *locale_hash;

   struct
     {
	Evas_Object     *blang_list;

	Evas_Object	*lang_list;
	Evas_Object     *reg_list;
	Evas_Object	*cs_list;
	Evas_Object	*mod_list;
	
	Evas_Object     *locale_entry;
     } 
   gui;
};

const E_Intl_Pair basic_language_predefined_pairs[ ] = {
       {"en_US.UTF-8", N_("English")},
       {"ja_JP.UTF-8", N_("Japanese")},
       {"fr_FR.UTF-8", N_("French")},
       {"es_AR.UTF-8", N_("Spanish")},
       {"pt_BR.UTF-8", N_("Portuguese")},
       {"fi_FI.UTF-8", N_("Finnish")},
       {"ru_RU.UTF-8", N_("Russian")},
       {"bg_BG.UTF-8", N_("Bulgarian")},
       {"de_DE.UTF-8", N_("German")},
       {"pl_PL.UTF-8", N_("Polish")},
       {"zh_CN.UTF-8", N_("Simplified Chinese")},
       {"zh_TW.UTF-8", N_("Traditional Chinese")}, 
       {"hu_HU.UTF-8", N_("Hungarian")},
       {"sl_SI.UTF-8", N_("Slovenian")},
       {"it_IT.UTF-8", N_("Italian")},
       {"cs_CS.UTF-8", N_("Czech")},
       {"da_DK.UTF-8", N_("Danish")},
       {"sk_SK.UTF-8", N_("Slovak")},
       {"sv_SV.UTF-8", N_("Swedish")},
       {"nb_NO.UTF-8", N_("Norwegian Bokmål")},
       {"nl_NL.UTF-8", N_("Dutch")},
       {"ko_KR.UTF-8", N_("Korean")},
       { NULL, NULL }
};

const E_Intl_Pair language_predefined_pairs[ ] = {
       {"aa", N_("Afar")},
       {"af", N_("Afrikaans")},
       {"ak", N_("Akan")},
       {"am", N_("Amharic")},
       {"an", N_("Aragonese")},
       {"ar", N_("Arabic")},
       {"as", N_("Assamese")},
       {"az", N_("Azerbaijani")},
       {"be", N_("Belarusian")},
       {"bg", N_("Bulgarian")},
       {"bn", N_("Bengali")},
       {"br", N_("Breton")},
       {"bs", N_("Bosnian")},
       {"byn", N_("Blin")},
       {"ca", N_("Catalan")},
       {"cch", N_("Atsam")},
       {"cs", N_("Czech")},
       {"cy", N_("Welsh")},
       {"da", N_("Danish")},
       {"de", N_("German")},
       {"dv", N_("Divehi")},
       {"dz", N_("Dzongkha")},
       {"ee", N_("Ewe")},
       {"el", N_("Greek")},
       {"en", N_("English")},
       {"eo", N_("Esperanto")},
       {"es", N_("Spanish")},
       {"et", N_("Estonian")},
       {"eu", N_("Basque")},
       {"fa", N_("Persian")},
       {"fi", N_("Finnish")},
       {"fo", N_("Faroese")},
       {"fr", N_("French")},
       {"fur", N_("Friulian")},
       {"ga", N_("Irish")},
       {"gaa", N_("Ga")},
       {"gez", N_("Geez")},
       {"gl", N_("Galician")},
       {"gu", N_("Gujarati")},
       {"gv", N_("Manx")},
       {"ha", N_("Hausa")},
       {"haw", N_("Hawaiian")},
       {"he", N_("Hebrew")},
       {"hi", N_("Hindi")},
       {"hr", N_("Croatian")},
       {"hu", N_("Hungarian")},
       {"hy", N_("Armenian")},
       {"ia", N_("Interlingua")},
       {"id", N_("Indonesian")},
       {"ig", N_("Igbo")},
       {"is", N_("Icelandic")},
       {"it", N_("Italian")},
       {"iu", N_("Inuktitut")},
       {"iw", N_("Hebrew")},
       {"ja", N_("Japanese")},
       {"ka", N_("Georgian")},
       {"kaj", N_("Jju")},
       {"kam", N_("Kamba")},
       {"kcg", N_("Tyap")},
       {"kfo", N_("Koro")},
       {"kk", N_("Kazakh")},
       {"kl", N_("Kalaallisut")},
       {"km", N_("Khmer")},
       {"kn", N_("Kannada")},
       {"ko", N_("Korean")},
       {"kok", N_("Konkani")},
       {"ku", N_("Kurdish")},
       {"kw", N_("Cornish")},
       {"ky", N_("Kirghiz")},
       {"ln", N_("Lingala")},
       {"lo", N_("Lao")},
       {"lt", N_("Lithuanian")},
       {"lv", N_("Latvian")},
       {"mi", N_("Maori")},
       {"mk", N_("Macedonian")},
       {"ml", N_("Malayalam")},
       {"mn", N_("Mongolian")},
       {"mr", N_("Marathi")},
       {"ms", N_("Malay")},
       {"mt", N_("Maltese")},
       {"nb", N_("Norwegian Bokmål")},
       {"ne", N_("Nepali")},
       {"nl", N_("Dutch")},
       {"nn", N_("Norwegian Nynorsk")},
       {"no", N_("Norwegian")},
       {"nr", N_("South Ndebele")},
       {"nso", N_("Northern Sotho")},
       {"ny", N_("Nyanja; Chichewa; Chewa")},
       {"oc", N_("Occitan")},
       {"om", N_("Oromo")},
       {"or", N_("Oriya")},
       {"pa", N_("Punjabi")},
       {"pl", N_("Polish")},
       {"ps", N_("Pashto")},
       {"pt", N_("Portuguese")},
       {"ro", N_("Romanian")},
       {"ru", N_("Russian")},
       {"rw", N_("Kinyarwanda")},
       {"sa", N_("Sanskrit")},
       {"se", N_("Northern Sami")},
       {"sh", N_("Serbo-Croatian")},
       {"sid", N_("Sidamo")},
       {"sk", N_("Slovak")},
       {"sl", N_("Slovenian")},
       {"so", N_("Somali")},
       {"sq", N_("Albanian")},
       {"sr", N_("Serbian")},
       {"ss", N_("Swati")},
       {"st", N_("Southern Sotho")},
       {"sv", N_("Swedish")},
       {"sw", N_("Swahili")},
       {"syr", N_("Syriac")},
       {"ta", N_("Tamil")},
       {"te", N_("Telugu")},
       {"tg", N_("Tajik")},
       {"th", N_("Thai")},
       {"ti", N_("Tigrinya")},
       {"tig", N_("Tigre")},
       {"tl", N_("Tagalog")},
       {"tn", N_("Tswana")},
       {"tr", N_("Turkish")},
       {"ts", N_("Tsonga")},
       {"tt", N_("Tatar")},
       {"uk", N_("Ukrainian")},
       {"ur", N_("Urdu")},
       {"uz", N_("Uzbek")},
       {"ve", N_("Venda")},
       {"vi", N_("Vietnamese")},
       {"wa", N_("Walloon")},
       {"wal", N_("Walamo")},
       {"xh", N_("Xhosa")},
       {"yi", N_("Yiddish")},
       {"yo", N_("Yoruba")},
       {"zh", N_("Chinese")},
       {"zu", N_("Zulu")},
       { NULL, NULL}
};

const E_Intl_Pair region_predefined_pairs[ ] = {
       { "AF", N_("Afghanistan")},
       { "AX", N_("Åland Islands")},
       { "AL", N_("Albania")},
       { "DZ", N_("Algeria")},
       { "AS", N_("American Samoa")},
       { "AD", N_("Andorra")},
       { "AO", N_("Angola")},
       { "AI", N_("Anguilla")},
       { "AQ", N_("Antarctica")},
       { "AG", N_("Antigua and Barbuda")},
       { "AR", N_("Argentina")},
       { "AM", N_("Armenia")},
       { "AW", N_("Aruba")},
       { "AU", N_("Australia")},
       { "AT", N_("Austria")},
       { "AZ", N_("Azerbaijan")},
       { "BS", N_("Bahamas")},
       { "BH", N_("Bahrain")},
       { "BD", N_("Bangladesh")},
       { "BB", N_("Barbados")},
       { "BY", N_("Belarus")},
       { "BE", N_("Belgium")},
       { "BZ", N_("Belize")},
       { "BJ", N_("Benin")},
       { "BM", N_("Bermuda")},
       { "BT", N_("Bhutan")},
       { "BO", N_("Bolivia")},
       { "BA", N_("Bosnia and Herzegovina")},
       { "BW", N_("Botswana")},
       { "BV", N_("Bouvet Island")},
       { "BR", N_("Brazil")},
       { "IO", N_("British Indian Ocean Territory")},
       { "BN", N_("Brunei Darussalam")},
       { "BG", N_("Bulgaria")},
       { "BF", N_("Burkina Faso")},
       { "BI", N_("Burundi")},
       { "KH", N_("Cambodia")},
       { "CM", N_("Cameroon")},
       { "CA", N_("Canada")},
       { "CV", N_("Cape Verde")},
       { "KY", N_("Cayman Islands")},
       { "CF", N_("Central African Republic")},
       { "TD", N_("Chad")},
       { "CL", N_("Chile")},
       { "CN", N_("China")},
       { "CX", N_("Christmas Island")},
       { "CC", N_("Cocos (keeling) Islands")},
       { "CO", N_("Colombia")},
       { "KM", N_("Comoros")},
       { "CG", N_("Congo")},
       { "CD", N_("Congo")},
       { "CK", N_("Cook Islands")},
       { "CR", N_("Costa Rica")},
       { "CI", N_("Cote D'ivoire")},
       { "HR", N_("Croatia")},
       { "CU", N_("Cuba")},
       { "CY", N_("Cyprus")},
       { "CZ", N_("Czech Republic")},
       { "DK", N_("Denmark")},
       { "DJ", N_("Djibouti")},
       { "DM", N_("Dominica")},
       { "DO", N_("Dominican Republic")},
       { "EC", N_("Ecuador")},
       { "EG", N_("Egypt")},
       { "SV", N_("El Salvador")},
       { "GQ", N_("Equatorial Guinea")},
       { "ER", N_("Eritrea")},
       { "EE", N_("Estonia")},
       { "ET", N_("Ethiopia")},
       { "FK", N_("Falkland Islands (malvinas)")},
       { "FO", N_("Faroe Islands")},
       { "FJ", N_("Fiji")},
       { "FI", N_("Finland")},
       { "FR", N_("France")},
       { "GF", N_("French Guiana")},
       { "PF", N_("French Polynesia")},
       { "TF", N_("French Southern Territories")},
       { "GA", N_("Gabon")},
       { "GM", N_("Gambia")},
       { "GE", N_("Georgia")},
       { "DE", N_("Germany")},
       { "GH", N_("Ghana")},
       { "GI", N_("Gibraltar")},
       { "GR", N_("Greece")},
       { "GL", N_("Greenland")},
       { "GD", N_("Grenada")},
       { "GP", N_("Guadeloupe")},
       { "GU", N_("Guam")},
       { "GT", N_("Guatemala")},
       { " GG", N_("Guernsey")},
       { "GN", N_("Guinea")},
       { "GW", N_("Guinea-bissau")},
       { "GY", N_("Guyana")},
       { "HT", N_("Haiti")},
       { "HM", N_("Heard Island and Mcdonald Islands")},
       { "VA", N_("Holy See (vatican City State)")},
       { "HN", N_("Honduras")},
       { "HK", N_("Hong Kong")},
       { "HU", N_("Hungary")},
       { "IS", N_("Iceland")},
       { "IN", N_("India")},
       { "ID", N_("Indonesia")},
       { "IR", N_("Iran")},
       { "IQ", N_("Iraq")},
       { "IE", N_("Ireland")},
       { "IM", N_("Isle Of Man")},
       { "IL", N_("Israel")},
       { "IT", N_("Italy")},
       { "JM", N_("Jamaica")},
       { "JP", N_("Japan")},
       { "JE", N_("Jersey")},
       { "JO", N_("Jordan")},
       { "KZ", N_("Kazakhstan")},
       { "KE", N_("Kenya")},
       { "KI", N_("Kiribati")},
       { "KP", N_("Korea")},
       { "KR", N_("Korea")},
       { "KW", N_("Kuwait")},
       { "KG", N_("Kyrgyzstan")},
       { "LA", N_("Lao People's Democratic Republic")},
       { "LV", N_("Latvia")},
       { "LB", N_("Lebanon")},
       { "LS", N_("Lesotho")},
       { "LR", N_("Liberia")},
       { "LY", N_("Libyan Arab Jamahiriya")},
       { "LI", N_("Liechtenstein")},
       { "LT", N_("Lithuania")},
       { "LU", N_("Luxembourg")},
       { "MO", N_("Macao")},
       { "MK", N_("Macedonia")},
       { "MG", N_("Madagascar")},
       { "MW", N_("Malawi")},
       { "MY", N_("Malaysia")},
       { "MV", N_("Maldives")},
       { "ML", N_("Mali")},
       { "MT", N_("Malta")},
       { "MH", N_("Marshall Islands")},
       { "MQ", N_("Martinique")},
       { "MR", N_("Mauritania")},
       { "MU", N_("Mauritius")},
       { "YT", N_("Mayotte")},
       { "MX", N_("Mexico")},
       { "FM", N_("Micronesia")},
       { "MD", N_("Moldova")},
       { "MC", N_("Monaco")},
       { "MN", N_("Mongolia")},
       { "MS", N_("Montserrat")},
       { "MA", N_("Morocco")},
       { "MZ", N_("Mozambique")},
       { "MM", N_("Myanmar")},
       { "NA", N_("Namibia")},
       { "NR", N_("Nauru")},
       { "NP", N_("Nepal")},
       { "NL", N_("Netherlands")},
       { "AN", N_("Netherlands Antilles")},
       { "NC", N_("New Caledonia")},
       { "NZ", N_("New Zealand")},
       { "NI", N_("Nicaragua")},
       { "NE", N_("Niger")},
       { "NG", N_("Nigeria")},
       { "NU", N_("Niue")},
       { "NF", N_("Norfolk Island")},
       { "MP", N_("Northern Mariana Islands")},
       { "NO", N_("Norway")},
       { "OM", N_("Oman")},
       { "PK", N_("Pakistan")},
       { "PW", N_("Palau")},
       { "PS", N_("Palestinian Territory")},
       { "PA", N_("Panama")},
       { "PG", N_("Papua New Guinea")},
       { "PY", N_("Paraguay")},
       { "PE", N_("Peru")},
       { "PH", N_("Philippines")},
       { "PN", N_("Pitcairn")},
       { "PL", N_("Poland")},
       { "PT", N_("Portugal")},
       { "PR", N_("Puerto Rico")},
       { "QA", N_("Qatar")},
       { "RE", N_("Reunion")},
       { "RO", N_("Romania")},
       { "RU", N_("Russian Federation")},
       { "RW", N_("Rwanda")},
       { "SH", N_("Saint Helena")},
       { "KN", N_("Saint Kitts and Nevis")},
       { "LC", N_("Saint Lucia")},
       { "PM", N_("Saint Pierre and Miquelon")},
       { "VC", N_("Saint Vincent and the Grenadines")},
       { "WS", N_("Samoa")},
       { "SM", N_("San Marino")},
       { "ST", N_("Sao Tome and Principe")},
       { "SA", N_("Saudi Arabia")},
       { "SN", N_("Senegal")},
       { "CS", N_("Serbia and Montenegro")},
       { "SC", N_("Seychelles")},
       { "SL", N_("Sierra Leone")},
       { "SG", N_("Singapore")},
       { "SK", N_("Slovakia")},
       { "SI", N_("Slovenia")},
       { "SB", N_("Solomon Islands")},
       { "SO", N_("Somalia")},
       { "ZA", N_("South Africa")},
       { "GS", N_("South Georgia and the South Sandwich Islands")},
       { "ES", N_("Spain")},
       { "LK", N_("Sri Lanka")},
       { "SD", N_("Sudan")},
       { "SR", N_("Suriname")},
       { "SJ", N_("Svalbard and Jan Mayen")},
       { "SZ", N_("Swaziland")},
       { "SE", N_("Sweden")},
       { "CH", N_("Switzerland")},
       { "SY", N_("Syrian Arab Republic")},
       { "TW", N_("Taiwan")},
       { "TJ", N_("Tajikistan")},
       { "TZ", N_("Tanzania")},
       { "TH", N_("Thailand")},
       { "TL", N_("Timor-leste")},
       { "TG", N_("Togo")},
       { "TK", N_("Tokelau")},
       { "TO", N_("Tonga")},
       { "TT", N_("Trinidad and Tobago")},
       { "TN", N_("Tunisia")},
       { "TR", N_("Turkey")},
       { "TM", N_("Turkmenistan")},
       { "TC", N_("Turks and Caicos Islands")},
       { "TV", N_("Tuvalu")},
       { "UG", N_("Uganda")},
       { "UA", N_("Ukraine")},
       { "AE", N_("United Arab Emirates")},
       { "GB", N_("United Kingdom")},
       { "US", N_("United States")},
       { "UM", N_("United States Minor Outlying Islands")},
       { "UY", N_("Uruguay")},
       { "UZ", N_("Uzbekistan")},
       { "VU", N_("Vanuatu")},
       { "VE", N_("Venezuela")},
       { "VN", N_("Viet Nam")},
       { "VG", N_("Virgin Islands")},
       { "VI", N_("Virgin Islands")},
       { "WF", N_("Wallis and Futuna")},
       { "EH", N_("Western Sahara")},
       { "YE", N_("Yemen")},
       { "ZM", N_("Zambia")},
       { "ZW", N_("Zimbabwe")},
       { NULL, NULL}
};

EAPI E_Config_Dialog *
e_int_config_intl(E_Container *con)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;

   v = E_NEW(E_Config_Dialog_View, 1);
   
   v->create_cfdata           = _create_data;
   v->free_cfdata             = _free_data;
   v->advanced.create_widgets    = _advanced_create_widgets;
   v->advanced.apply_cfdata      = _advanced_apply_data;
   v->basic.create_widgets    = _basic_create_widgets;
   v->basic.apply_cfdata      = _basic_apply_data;
   
   cfd = e_config_dialog_new(con,
			     _("Language Configuration"),
			    "E", "_config_intl_dialog",
			     "enlightenment/intl", 0, v, NULL);
   return cfd;
}

/* Build hash tables used for locale navigation. The locale information is 
 * gathered using the locale -a command. 
 *
 * Below the following terms are used:
 * ll - Locale Language Code (Example en)
 * RR - Locale Region code (Example US)
 * enc - Locale Encoding (Example UTF-8)
 * mod - Locale Modifier (Example EURO)
 */
static void
_fill_data(E_Config_Dialog_Data *cfdata)
{
   Evas_List    *e_lang_list;
   FILE		*output;
   
   e_lang_list = e_intl_language_list();
   
   /* Get list of all locales and start making map */
   output = popen("locale -a", "r");
   if ( output ) 
     {
	char line[32];
	while ( fscanf(output, "%[^\n]\n", line) == 1)
	  {
	     char *language;
	     
	     language = e_intl_locale_canonic_get(line, E_INTL_LOC_LANG);

	     if (language && (language[0] == 0))
	       {
		  free(language);
		  language = NULL;
	       }
	     
	     /* If the language is a valid ll_RR[.enc[@mod]] locale add it to the hash */
	     if (language) 
	       {
		  E_Intl_Language_Node *lang_node;
		  E_Intl_Region_Node  *region_node;
		  char *region;
		  char *codeset;
		  char *modifier;

		  /* Separate out ll RR enc and mod parts */
		  region = e_intl_locale_canonic_get(line, E_INTL_LOC_REGION);
		  codeset = e_intl_locale_canonic_get(line, E_INTL_LOC_CODESET);
		  modifier = e_intl_locale_canonic_get(line, E_INTL_LOC_MODIFIER);
   
		  /* Add the language to the new locale properties to the hash */
		  /* First check if the LANGUAGE exists in there already */
		  lang_node  = evas_hash_find(cfdata->locale_hash, language);
		  if (lang_node == NULL) 
		    {
		       Evas_List *next;
		       int i;
		       
		       /* create new node */
		       lang_node = E_NEW(E_Intl_Language_Node, 1);

		       lang_node->lang_code = evas_stringshare_add(language);

		       /* Check if the language list exists */
		       /* Linear Search */
		       for (next = e_lang_list; next; next = next->next) 
			 {
			    char *e_lang;

			    e_lang = next->data;
			    if (!strncmp(e_lang, language, 2) || !strcmp("en", language)) 
			      {
				 lang_node->lang_available = 1;
				 break;
			      }
			 }
		       
		       /* Search for translation */
		       /* Linear Search */
		       i = 0;
		       while (language_predefined_pairs[i].locale_key)
			 {
			    if (!strcmp(language_predefined_pairs[i].locale_key, language))
			      lang_node->lang_name = _(language_predefined_pairs[i].locale_translation);
			      
			    i++;
			 }
		       
		       cfdata->locale_hash = evas_hash_add(cfdata->locale_hash, language, lang_node);
		    }

		  /* If no region data just go next */	  
		  if (region && (region[0] == 0))
		    {
		       free(region);
		       region = NULL;
		    }

		  /* We now have the current language hash node, lets see if there is
		     region data that needs to be added. 
		   */
		  if (region)
		    {
		       region_node = evas_hash_find(lang_node->region_hash, region);

		       if (region_node == NULL)
			 {
			    int i;
			    
			    /* create new node */
			    region_node = E_NEW(E_Intl_Region_Node, 1);
			    region_node->region_code = evas_stringshare_add(region);
		       
			    /* Get the region translation */
			    /* Linear Search */
			    i = 0;
			    while (region_predefined_pairs[i].locale_key)
			      {
				 if (!strcmp(region_predefined_pairs[i].locale_key, region))
				   region_node->region_name = _(region_predefined_pairs[i].locale_translation);
				 i++;
			      }
			    lang_node->region_hash = evas_hash_add(lang_node->region_hash, region, region_node);
			 }

		       /* We now have the current region hash node */
		       /* Add codeset to the region hash node if it exists */
		       if (codeset && (codeset[0] != 0))
			 {
			    const char * cs;
			    
			    cs = evas_stringshare_add(codeset);
			    /* Exclusive */
			    /* Linear Search */
			    if (!evas_list_find(region_node->available_codesets, cs))
			      region_node->available_codesets = evas_list_append(region_node->available_codesets, cs);
			 }

		       /* Add modifier to the region hash node if it exists */
		       if (modifier && (modifier[0] != 0))
			 {
			    const char * mod;
			    
			    /* Find only works here because we are using stringshare*/
			    mod = evas_stringshare_add(modifier);
			    /* Exclusive */
			    /* Linear Search */
			    if (!evas_list_find(region_node->available_modifiers, mod))
			      region_node->available_modifiers = evas_list_append(region_node->available_modifiers, mod);
			 }
		       free(region);
		    }
		  
		  E_FREE(codeset);
		  E_FREE(modifier);
		  free(language);
	       }
	  }

        while (e_lang_list)
	  {
	     free(e_lang_list->data);
	     e_lang_list = evas_list_remove_list(e_lang_list, e_lang_list);
	  }	     
	pclose(output);
     }

   /* Make sure we know the currently configured locale */
   cfdata->cur_language = strdup(e_config->language);
   
   return;
}

static void *
_create_data(E_Config_Dialog *cfd)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   cfdata->cfd = cfd;
   _fill_data(cfdata);
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   E_FREE(cfdata->cur_language);
   E_FREE(cfdata->cur_blang);
   E_FREE(cfdata->cur_lang);
   E_FREE(cfdata->cur_reg);
   E_FREE(cfdata->cur_cs);
   E_FREE(cfdata->cur_mod);

   evas_hash_foreach(cfdata->locale_hash, _language_hash_free_cb, NULL);
   evas_hash_free(cfdata->locale_hash);
   
   free(cfdata);
}

static Evas_Bool 
_language_hash_free_cb(Evas_Hash *hash, const char *key, void *data, void *fdata)
{
   E_Intl_Language_Node *node;

   node = data;   
   if (node->lang_code) evas_stringshare_del(node->lang_code);
   evas_hash_foreach(node->region_hash, _region_hash_free_cb, NULL);
   evas_hash_free(node->region_hash);
   free(node); 
   
   return 1;
}

static Evas_Bool 
_region_hash_free_cb(Evas_Hash *hash, const char *key, void *data, void *fdata)
{ 
   E_Intl_Region_Node *node;

   node = data;   
   if (node->region_code) evas_stringshare_del(node->region_code);
   while (node->available_codesets) 
     {
	const char *str;

	str = node->available_codesets->data;
	if (str) evas_stringshare_del(str);
	node->available_codesets = evas_list_remove_list(node->available_codesets, node->available_codesets);
     }
    
   while (node->available_modifiers) 
     {
	const char *str;

	str = node->available_modifiers->data;
	if (str) evas_stringshare_del(str);
	node->available_modifiers = evas_list_remove_list(node->available_modifiers, node->available_modifiers);
     }
   
   free(node);  
   return 1;
}

static int
_basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{	 
   if (cfdata->cur_language != NULL)
     {
	e_config->language = evas_stringshare_add(cfdata->cur_language);
	e_intl_language_set(e_config->language);
     }
   
   e_config_save_queue();
   
   return 1;
}

static int
_advanced_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{	 
   if (cfdata->cur_language != NULL)
     {
	e_config->language = evas_stringshare_add(cfdata->cur_language);
	e_intl_language_set(e_config->language);
     }
   
   e_config_save_queue();
   
   return 1;
}

static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *o, *of, *ob;
   int i;
   
   cfdata->evas = evas;
   
   o = e_widget_list_add(evas, 0, 0);
  
   of = e_widget_frametable_add(evas, _("Language Selector"), 1);
  
   /* Language List */ 
   ob = e_widget_ilist_add(evas, 16, 16, &(cfdata->cur_blang));
   e_widget_on_change_hook_set(ob, _ilist_basic_language_cb_change, cfdata);
   cfdata->gui.blang_list = ob;

   i = 0;
   while (basic_language_predefined_pairs[i].locale_key)
     {
	const char *key;
	const char *trans;

	key = basic_language_predefined_pairs[i].locale_key;
	trans = basic_language_predefined_pairs[i].locale_translation;
	e_widget_ilist_append(cfdata->gui.blang_list, NULL, trans, NULL, NULL, key);
	i++;
     }
   e_widget_ilist_go(ob);
   e_widget_min_size_set(ob, 100, 100);
   e_widget_frametable_object_append(of, ob, 0, 0, 2, 6, 1, 1, 1, 1);
    
   e_widget_list_object_append(o, of, 1, 1, 0.5);
   
   /* Locale selector */
   of = e_widget_frametable_add(evas, _("Locale Selected"), 0);
  
   ob = e_widget_label_add(evas, _("Locale"));
   e_widget_frametable_object_append(	of, 
					ob, 
					0, 0, 1, 1,
					1, 1, 1, 1);
   
   cfdata->gui.locale_entry = e_widget_entry_add(evas, &(cfdata->cur_language));
   e_widget_disabled_set(cfdata->gui.locale_entry, 1);
   e_widget_min_size_set(cfdata->gui.locale_entry, 100, 25);
   e_widget_frametable_object_append(of, cfdata->gui.locale_entry, 
					0, 1, 1, 1, 
					1, 1, 1, 1);
    
   e_widget_list_object_append(o, of, 1, 1, 0.5);
   
   return o;

}
   
static Evas_Object *
_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *o, *of, *ob;
   char *language;
   char *region;
   char *codeset;
   char *modifier;

   cfdata->evas = evas;
  
   E_FREE(cfdata->cur_lang);
   cfdata->cur_lang = NULL;
  
   E_FREE(cfdata->cur_reg);
   cfdata->cur_reg = NULL;
   
   o = e_widget_list_add(evas, 0, 0);
  
   of = e_widget_frametable_add(evas, _("Language Selector"), 1);
  
   /* Language List */ 
   ob = e_widget_ilist_add(evas, 16, 16, &(cfdata->cur_lang));
   e_widget_on_change_hook_set(ob, _ilist_language_cb_change, cfdata);
   cfdata->gui.lang_list = ob;

   evas_hash_foreach(cfdata->locale_hash, _lang_hash_cb, cfdata);
  
   e_widget_ilist_go(ob);
   e_widget_min_size_set(ob, 100, 100);
   e_widget_frametable_object_append(of, ob, 0, 0, 1, 4, 1, 1, 1, 1);
   
   /* Region List */
   ob = e_widget_ilist_add(evas, 0, 0, &(cfdata->cur_reg));
   e_widget_on_change_hook_set(ob, _ilist_region_cb_change, cfdata); 
   cfdata->gui.reg_list = ob;
 
   e_widget_ilist_go(ob);
   e_widget_min_size_set(ob, 100, 100);
   e_widget_frametable_object_append(of, ob, 1, 0, 1, 4, 1, 1, 1, 1);
 
   /* Codeset List */
   ob = e_widget_ilist_add(evas, 0, 0, &(cfdata->cur_cs));
   e_widget_on_change_hook_set(ob, _ilist_codeset_cb_change, cfdata); 
   cfdata->gui.cs_list = ob;

   e_widget_ilist_go(ob);
   e_widget_min_size_set(ob, 100, 100);
   e_widget_frametable_object_append(of, ob, 2, 0, 1, 4, 1, 1, 1, 1);
   
   /* Modified List */
   ob = e_widget_ilist_add(evas, 0, 0, &(cfdata->cur_mod));
   e_widget_on_change_hook_set(ob, _ilist_modifier_cb_change, cfdata); 
   cfdata->gui.mod_list = ob;

   e_widget_ilist_go(ob);
   e_widget_min_size_set(ob, 100, 100);
   e_widget_frametable_object_append(of, ob, 3, 0, 1, 4, 1, 1, 1, 1);
    
   e_widget_list_object_append(o, of, 1, 1, 0.5);
   
   /* Locale selector */
   of = e_widget_frametable_add(evas, _("Locale Selected"), 0);
  
   ob = e_widget_label_add(evas, _("Locale"));
   e_widget_frametable_object_append(	of, 
					ob, 
					0, 0, 1, 1,
					1, 1, 1, 1);
   
   cfdata->gui.locale_entry = e_widget_entry_add(evas, &(cfdata->cur_language));
   e_widget_disabled_set(cfdata->gui.locale_entry, 1);
   e_widget_min_size_set(cfdata->gui.locale_entry, 100, 25);
   e_widget_frametable_object_append(of, cfdata->gui.locale_entry, 
					0, 1, 1, 1, 
					1, 1, 1, 1);
    
   e_widget_list_object_append(o, of, 1, 1, 0.5);
  
   language = e_intl_locale_canonic_get(cfdata->cur_language, E_INTL_LOC_LANG);
   region = e_intl_locale_canonic_get(cfdata->cur_language, E_INTL_LOC_REGION);
   codeset = e_intl_locale_canonic_get(cfdata->cur_language, E_INTL_LOC_CODESET);
   modifier = e_intl_locale_canonic_get(cfdata->cur_language, E_INTL_LOC_MODIFIER);
  
   _cfdata_language_go(language, region, codeset, modifier, cfdata);
   
   E_FREE(language);
   E_FREE(region);
   E_FREE(codeset);
   E_FREE(modifier);
   
   return o;
}

static void
_ilist_basic_language_cb_change(void *data, Evas_Object *obj)
{
    E_Config_Dialog_Data * cfdata;
    
    cfdata = data;

    e_widget_entry_text_set(cfdata->gui.locale_entry, cfdata->cur_blang);
}

static void
_ilist_language_cb_change(void *data, Evas_Object *obj)
{
    E_Config_Dialog_Data * cfdata;
    
    cfdata = data;
    
    _cfdata_language_go(cfdata->cur_lang, NULL, NULL, NULL, cfdata);

    e_widget_entry_text_set(cfdata->gui.locale_entry, cfdata->cur_lang);
    E_FREE(cfdata->cur_cs);
    E_FREE(cfdata->cur_mod);
}

static void
_ilist_region_cb_change(void *data, Evas_Object *obj)
{
    E_Config_Dialog_Data * cfdata;
    char locale[32];
    
    cfdata = data;
    
    _cfdata_language_go(cfdata->cur_lang, cfdata->cur_reg, NULL, NULL, cfdata);
    
    sprintf(locale, "%s_%s", cfdata->cur_lang, cfdata->cur_reg);
    e_widget_entry_text_set(cfdata->gui.locale_entry, locale);
    E_FREE(cfdata->cur_cs);
    E_FREE(cfdata->cur_mod);
}

static void 
_ilist_codeset_cb_change(void *data, Evas_Object *obj)
{
    E_Config_Dialog_Data * cfdata;
    char locale[32];

    cfdata = data;

    if (cfdata->cur_mod)
      {
	 sprintf(locale, "%s_%s.%s@%s", cfdata->cur_lang, cfdata->cur_reg, cfdata->cur_cs, cfdata->cur_mod);
      }
    else
      {	 
	 sprintf(locale, "%s_%s.%s", cfdata->cur_lang, cfdata->cur_reg, cfdata->cur_cs);
      }
    e_widget_entry_text_set(cfdata->gui.locale_entry, locale);
}

static void 
_ilist_modifier_cb_change(void *data, Evas_Object *obj)
{
    E_Config_Dialog_Data * cfdata;
    char locale[32];
    
    cfdata = data;
    if (cfdata->cur_cs)
      {
	 sprintf(locale, "%s_%s.%s@%s", cfdata->cur_lang, cfdata->cur_reg, cfdata->cur_cs, cfdata->cur_mod);
      }
    else
      {	 
	 sprintf(locale, "%s_%s@%s", cfdata->cur_lang, cfdata->cur_reg, cfdata->cur_mod);
      }
    e_widget_entry_text_set(cfdata->gui.locale_entry, locale);

}

static void 
_cfdata_language_go(const char *lang, const char *region, const char *codeset, const char *modifier, E_Config_Dialog_Data *cfdata)
{
   E_Intl_Language_Node *lang_node;	
   int lang_update;
   int region_update;
   
   /* Check what changed */
   lang_update = 0;
   region_update = 0;
   
   if (cfdata->cur_lang == NULL || (lang && !region)) 
     {
	lang_update = 1;
	region_update = 1;
	e_widget_ilist_clear(cfdata->gui.cs_list);
	e_widget_ilist_clear(cfdata->gui.mod_list);
     }
   if (lang && region) 
     {
	region_update = 1;
	e_widget_ilist_clear(cfdata->gui.cs_list);
	e_widget_ilist_clear(cfdata->gui.mod_list);
     }
   
   if (lang)
     {
	lang_node = evas_hash_find(cfdata->locale_hash, lang);
	
	if (lang_node)
	  {
	     if (lang_update)
	       {
		  e_widget_ilist_clear(cfdata->gui.reg_list);
		  evas_hash_foreach(lang_node->region_hash, _region_hash_cb, cfdata);
	       }
	     if (region && region_update)
	       {
		  E_Intl_Region_Node *reg_node;
		  
		  reg_node = evas_hash_find(lang_node->region_hash, region);
		  if (reg_node)
		    {
		       Evas_List *next;

		       for (next = reg_node->available_codesets; next; next = next->next) 
			 {
			    const char * cs;
			    
			    cs = next->data;
			    e_widget_ilist_append(cfdata->gui.cs_list, NULL, cs, NULL, NULL, cs);
			 }
		       for (next = reg_node->available_modifiers; next; next = next->next) 
			 {
			    const char * mod;
			    
			    mod = next->data;
			    e_widget_ilist_append(cfdata->gui.mod_list, NULL, mod, NULL, NULL, mod);
			 }

		    }
		  e_widget_ilist_go(cfdata->gui.cs_list);
		  e_widget_ilist_go(cfdata->gui.mod_list);
	       }
	  }
	e_widget_ilist_go(cfdata->gui.reg_list);
     }
 
}

static Evas_Bool
_lang_hash_cb(Evas_Hash *hash, const char *key, void *data, void *fdata)
{
   E_Config_Dialog_Data *cfdata; 
   E_Intl_Language_Node *lang_node;
   const char *trans;
   
   cfdata = fdata;
   lang_node = data;
   
   if (lang_node->lang_name)
     {
	trans = lang_node->lang_name;
     }
   else 
     {
	trans = key;
     }
   
   if (lang_node->lang_available)
     {
	Evas_Object *ic;
	
	ic = edje_object_add(cfdata->evas);
	e_util_edje_icon_set(ic, "enlightenment/e");
	e_widget_ilist_append(cfdata->gui.lang_list, ic, trans, NULL, NULL, key);
     }
   else
     {	
	e_widget_ilist_append(cfdata->gui.lang_list, NULL, trans, NULL, NULL, key);
     }
   return 1;
}

static Evas_Bool
_region_hash_cb(Evas_Hash *hash, const char *key, void *data, void *fdata)
{
   E_Config_Dialog_Data *cfdata; 
   E_Intl_Region_Node *reg_node;
   const char *trans;
   
   cfdata = fdata;
   reg_node = data;

   if (reg_node->region_name)
     {
	trans = reg_node->region_name;
     }
   else 
     {
	trans = key;
     }
 
   e_widget_ilist_append(cfdata->gui.reg_list, NULL, trans, NULL, NULL, key); 
   return 1;
}
