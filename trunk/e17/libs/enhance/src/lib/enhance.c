#include "enhance_private.h"

static void      _e_property_handle(Enhance *en, EXML_Node *node);
static void      _e_signal_handle(Enhance *en, EXML_Node *node);
static void      _e_traverse_packing_xml(Enhance *en, E_Widget *widget);
static void      _e_traverse_property_xml(Enhance *en);
static void      _e_traverse_signal_xml(Enhance *en);
static void      _e_traverse_child_xml(Enhance *en);
static E_Widget *_e_traverse_widget_xml(Enhance *en);
static void      _e_traverse_xml(Enhance *en);
static Ecore_Hash*_en_stock_items_hash=NULL;
static void      _en_stock_items_hash_init(void);

typedef struct _En_Stock_Item
{
   char *str;
   Etk_Stock_Id id;
} En_Stock_Item;

static const En_Stock_Item en_stock_items[] =
{
   { "ETK_STOCK_ADDRESS_BOOK_NEW", ETK_STOCK_ADDRESS_BOOK_NEW },
   { "ETK_STOCK_APPOINTMENT_NEW", ETK_STOCK_APPOINTMENT_NEW },
   { "ETK_STOCK_BOOKMARK_NEW", ETK_STOCK_BOOKMARK_NEW },
   { "ETK_STOCK_CONTACT_NEW", ETK_STOCK_CONTACT_NEW },
   { "ETK_STOCK_DIALOG_OK", ETK_STOCK_DIALOG_OK },     
   { "ETK_STOCK_DIALOG_CANCEL", ETK_STOCK_DIALOG_CANCEL },
   { "ETK_STOCK_DIALOG_YES", ETK_STOCK_DIALOG_YES },     
   { "ETK_STOCK_DIALOG_NO", ETK_STOCK_DIALOG_NO },
   { "ETK_STOCK_DIALOG_CLOSE", ETK_STOCK_DIALOG_CLOSE },
   { "ETK_STOCK_DOCUMENT_NEW", ETK_STOCK_DOCUMENT_NEW },
   { "ETK_STOCK_DOCUMENT_OPEN", ETK_STOCK_DOCUMENT_OPEN },
   { "ETK_STOCK_DOCUMENT_PRINT", ETK_STOCK_DOCUMENT_PRINT },
   { "ETK_STOCK_DOCUMENT_PRINT_PREVIEW", ETK_STOCK_DOCUMENT_PRINT_PREVIEW },
   { "ETK_STOCK_DOCUMENT_PROPERTIES", ETK_STOCK_DOCUMENT_PROPERTIES },
   { "ETK_STOCK_DOCUMENT_SAVE_AS", ETK_STOCK_DOCUMENT_SAVE_AS },
   { "ETK_STOCK_DOCUMENT_SAVE", ETK_STOCK_DOCUMENT_SAVE },
   { "ETK_STOCK_EDIT_CLEAR", ETK_STOCK_EDIT_CLEAR },
   { "ETK_STOCK_EDIT_COPY", ETK_STOCK_EDIT_COPY },
   { "ETK_STOCK_EDIT_CUT", ETK_STOCK_EDIT_CUT },
   { "ETK_STOCK_EDIT_FIND", ETK_STOCK_EDIT_FIND },
   { "ETK_STOCK_EDIT_PASTE", ETK_STOCK_EDIT_PASTE },
   { "ETK_STOCK_EDIT_REDO", ETK_STOCK_EDIT_REDO },
   { "ETK_STOCK_EDIT_UNDO", ETK_STOCK_EDIT_UNDO },
   { "ETK_STOCK_EDIT_DELETE", ETK_STOCK_EDIT_DELETE },
   { "ETK_STOCK_EDIT_FIND_REPLACE", ETK_STOCK_EDIT_FIND_REPLACE },
   { "ETK_STOCK_FOLDER_NEW", ETK_STOCK_FOLDER_NEW },
   { "ETK_STOCK_FORMAT_INDENT_LESS", ETK_STOCK_FORMAT_INDENT_LESS },
   { "ETK_STOCK_FORMAT_INDENT_MORE", ETK_STOCK_FORMAT_INDENT_MORE },
   { "ETK_STOCK_FORMAT_JUSTIFY_CENTER", ETK_STOCK_FORMAT_JUSTIFY_CENTER },
   { "ETK_STOCK_FORMAT_JUSTIFY_FILL", ETK_STOCK_FORMAT_JUSTIFY_FILL },
   { "ETK_STOCK_FORMAT_JUSTIFY_LEFT", ETK_STOCK_FORMAT_JUSTIFY_LEFT },
   { "ETK_STOCK_FORMAT_JUSTIFY_RIGHT", ETK_STOCK_FORMAT_JUSTIFY_RIGHT },
   { "ETK_STOCK_FORMAT_TEXT_BOLD", ETK_STOCK_FORMAT_TEXT_BOLD },
   { "ETK_STOCK_FORMAT_TEXT_ITALIC", ETK_STOCK_FORMAT_TEXT_ITALIC },
   { "ETK_STOCK_FORMAT_TEXT_STRIKETHROUGH", ETK_STOCK_FORMAT_TEXT_STRIKETHROUGH },
   { "ETK_STOCK_FORMAT_TEXT_UNDERLINE", ETK_STOCK_FORMAT_TEXT_UNDERLINE },
   { "ETK_STOCK_GO_BOTTOM", ETK_STOCK_GO_BOTTOM },
   { "ETK_STOCK_GO_DOWN", ETK_STOCK_GO_DOWN },
   { "ETK_STOCK_GO_FIRST", ETK_STOCK_GO_FIRST },
   { "ETK_STOCK_GO_HOME", ETK_STOCK_GO_HOME },
   { "ETK_STOCK_GO_JUMP", ETK_STOCK_GO_JUMP },
   { "ETK_STOCK_GO_LAST", ETK_STOCK_GO_LAST },
   { "ETK_STOCK_GO_NEXT", ETK_STOCK_GO_NEXT },
   { "ETK_STOCK_GO_PREVIOUS", ETK_STOCK_GO_PREVIOUS },
   { "ETK_STOCK_GO_TOP", ETK_STOCK_GO_TOP },
   { "ETK_STOCK_GO_UP", ETK_STOCK_GO_UP },
   { "ETK_STOCK_LIST_ADD", ETK_STOCK_LIST_ADD },
   { "ETK_STOCK_LIST_REMOVE", ETK_STOCK_LIST_REMOVE },
   { "ETK_STOCK_MAIL_MESSAGE_NEW", ETK_STOCK_MAIL_MESSAGE_NEW },
   { "ETK_STOCK_MAIL_FORWARD", ETK_STOCK_MAIL_FORWARD },
   { "ETK_STOCK_MAIL_MARK_JUNK", ETK_STOCK_MAIL_MARK_JUNK },
   { "ETK_STOCK_MAIL_REPLY_ALL", ETK_STOCK_MAIL_REPLY_ALL },
   { "ETK_STOCK_MAIL_REPLY_SENDER", ETK_STOCK_MAIL_REPLY_SENDER },
   { "ETK_STOCK_MAIL_SEND_RECEIVE", ETK_STOCK_MAIL_SEND_RECEIVE },
   { "ETK_STOCK_MEDIA_EJECT", ETK_STOCK_MEDIA_EJECT },  
   { "ETK_STOCK_MEDIA_PLAYBACK_PAUSE", ETK_STOCK_MEDIA_PLAYBACK_PAUSE },
   { "ETK_STOCK_MEDIA_PLAYBACK_START", ETK_STOCK_MEDIA_PLAYBACK_START },
   { "ETK_STOCK_MEDIA_PLAYBACK_STOP", ETK_STOCK_MEDIA_PLAYBACK_STOP },
   { "ETK_STOCK_MEDIA_RECORD", ETK_STOCK_MEDIA_RECORD },
   { "ETK_STOCK_MEDIA_SEEK_BACKWARD", ETK_STOCK_MEDIA_SEEK_BACKWARD },
   { "ETK_STOCK_MEDIA_SEEK_FORWARD", ETK_STOCK_MEDIA_SEEK_FORWARD },
   { "ETK_STOCK_MEDIA_SKIP_BACKWARD", ETK_STOCK_MEDIA_SKIP_BACKWARD },
   { "ETK_STOCK_MEDIA_SKIP_FORWARD", ETK_STOCK_MEDIA_SKIP_FORWARD },
   { "ETK_STOCK_PROCESS_STOP", ETK_STOCK_PROCESS_STOP },
   { "ETK_STOCK_SYSTEM_LOCK_SCREEN", ETK_STOCK_SYSTEM_LOCK_SCREEN },
   { "ETK_STOCK_SYSTEM_LOG_OUT", ETK_STOCK_SYSTEM_LOG_OUT },
   { "ETK_STOCK_SYSTEM_SEARCH", ETK_STOCK_SYSTEM_SEARCH },
   { "ETK_STOCK_SYSTEM_SHUTDOWN", ETK_STOCK_SYSTEM_SHUTDOWN },
   { "ETK_STOCK_TAB_NEW", ETK_STOCK_TAB_NEW },
   { "ETK_STOCK_VIEW_REFRESH", ETK_STOCK_VIEW_REFRESH },
   { "ETK_STOCK_WINDOW_NEW", ETK_STOCK_WINDOW_NEW },
   { "ETK_STOCK_ACCESSORIES_CALCULATOR", ETK_STOCK_ACCESSORIES_CALCULATOR },
   { "ETK_STOCK_ACCESSORIES_CHARACTER_MAP", ETK_STOCK_ACCESSORIES_CHARACTER_MAP },
   { "ETK_STOCK_ACCESSORIES_TEXT_EDITOR", ETK_STOCK_ACCESSORIES_TEXT_EDITOR },
   { "ETK_STOCK_HELP_BROWSER", ETK_STOCK_HELP_BROWSER },
   { "ETK_STOCK_INTERNET_GROUP_CHAT", ETK_STOCK_INTERNET_GROUP_CHAT },
   { "ETK_STOCK_INTERNET_MAIL", ETK_STOCK_INTERNET_MAIL },
   { "ETK_STOCK_INTERNET_NEWS_READER", ETK_STOCK_INTERNET_NEWS_READER },
   { "ETK_STOCK_INTERNET_WEB_BROWSER", ETK_STOCK_INTERNET_WEB_BROWSER },
   { "ETK_STOCK_MULTIMEDIA_VOLUME_CONTROL", ETK_STOCK_MULTIMEDIA_VOLUME_CONTROL },
   { "ETK_STOCK_OFFICE_CALENDAR", ETK_STOCK_OFFICE_CALENDAR },
   { "ETK_STOCK_PREFERENCES_DESKTOP_ACCESSIBILITY", ETK_STOCK_PREFERENCES_DESKTOP_ACCESSIBILITY },
   { "ETK_STOCK_PREFERENCES_DESKTOP_ASSISTIVE_TECHNOLOGY", ETK_STOCK_PREFERENCES_DESKTOP_ASSISTIVE_TECHNOLOGY },
   { "ETK_STOCK_PREFERENCES_DESKTOP_FONT", ETK_STOCK_PREFERENCES_DESKTOP_FONT },
   { "ETK_STOCK_PREFERENCES_DESKTOP_KEYBOARD_SHORTCUTS", ETK_STOCK_PREFERENCES_DESKTOP_KEYBOARD_SHORTCUTS },
   { "ETK_STOCK_PREFERENCES_DESKTOP_LOCALE", ETK_STOCK_PREFERENCES_DESKTOP_LOCALE },
   { "ETK_STOCK_PREFERENCES_DESKTOP_REMOTE_DESKTOP", ETK_STOCK_PREFERENCES_DESKTOP_REMOTE_DESKTOP },
   { "ETK_STOCK_PREFERENCES_DESKTOP_SOUND", ETK_STOCK_PREFERENCES_DESKTOP_SOUND },
   { "ETK_STOCK_PREFERENCES_DESKTOP_SCREENSAVER", ETK_STOCK_PREFERENCES_DESKTOP_SCREENSAVER },
   { "ETK_STOCK_PREFERENCES_DESKTOP_THEME", ETK_STOCK_PREFERENCES_DESKTOP_THEME },
   { "ETK_STOCK_PREFERENCES_DESKTOP_WALLPAPER", ETK_STOCK_PREFERENCES_DESKTOP_WALLPAPER },
   { "ETK_STOCK_PREFERENCES_SYSTEM_NETWORK_PROXY", ETK_STOCK_PREFERENCES_SYSTEM_NETWORK_PROXY },  
   { "ETK_STOCK_PREFERENCES_SYSTEM_SESSION", ETK_STOCK_PREFERENCES_SYSTEM_SESSION },
   { "ETK_STOCK_PREFERENCES_SYSTEM_WINDOWS", ETK_STOCK_PREFERENCES_SYSTEM_WINDOWS },
   { "ETK_STOCK_SYSTEM_FILE_MANAGER", ETK_STOCK_SYSTEM_FILE_MANAGER },
   { "ETK_STOCK_SYSTEM_INSTALLER", ETK_STOCK_SYSTEM_INSTALLER },
   { "ETK_STOCK_SYSTEM_SOFTWARE_UPDATE", ETK_STOCK_SYSTEM_SOFTWARE_UPDATE },
   { "ETK_STOCK_SYSTEM_USERS", ETK_STOCK_SYSTEM_USERS },
   { "ETK_STOCK_UTILITIES_SYSTEM_MONITOR", ETK_STOCK_UTILITIES_SYSTEM_MONITOR },
   { "ETK_STOCK_UTILITIES_TERMINAL", ETK_STOCK_UTILITIES_TERMINAL },
   { "ETK_STOCK_APPLICATIONS_ACCESSORIES", ETK_STOCK_APPLICATIONS_ACCESSORIES },
   { "ETK_STOCK_APPLICATIONS_DEVELOPMENT", ETK_STOCK_APPLICATIONS_DEVELOPMENT },
   { "ETK_STOCK_APPLICATIONS_GAMES", ETK_STOCK_APPLICATIONS_GAMES },
   { "ETK_STOCK_APPLICATIONS_GRAPHICS", ETK_STOCK_APPLICATIONS_GRAPHICS },
   { "ETK_STOCK_APPLICATIONS_INTERNET", ETK_STOCK_APPLICATIONS_INTERNET },
   { "ETK_STOCK_APPLICATIONS_MULTIMEDIA", ETK_STOCK_APPLICATIONS_MULTIMEDIA },
   { "ETK_STOCK_APPLICATIONS_OFFICE", ETK_STOCK_APPLICATIONS_OFFICE },
   { "ETK_STOCK_APPLICATIONS_OTHER", ETK_STOCK_APPLICATIONS_OTHER },
   { "ETK_STOCK_APPLICATIONS_SYSTEM", ETK_STOCK_APPLICATIONS_SYSTEM },
   { "ETK_STOCK_PREFERENCES_DESKTOP_PERIPHERALS", ETK_STOCK_PREFERENCES_DESKTOP_PERIPHERALS },
   { "ETK_STOCK_PREFERENCES_DESKTOP", ETK_STOCK_PREFERENCES_DESKTOP },
   { "ETK_STOCK_PREFERENCES_SYSTEM", ETK_STOCK_PREFERENCES_SYSTEM },
   { "ETK_STOCK_AUDIO_CARD", ETK_STOCK_AUDIO_CARD },
   { "ETK_STOCK_AUDIO_INPUT_MICROPHONE", ETK_STOCK_AUDIO_INPUT_MICROPHONE },
   { "ETK_STOCK_BATTERY", ETK_STOCK_BATTERY },
   { "ETK_STOCK_CAMERA_PHOTO", ETK_STOCK_CAMERA_PHOTO },
   { "ETK_STOCK_CAMERA_VIDEO", ETK_STOCK_CAMERA_VIDEO },
   { "ETK_STOCK_COMPUTER", ETK_STOCK_COMPUTER },
   { "ETK_STOCK_DRIVE_CDROM", ETK_STOCK_DRIVE_CDROM },
   { "ETK_STOCK_DRIVE_HARDDISK", ETK_STOCK_DRIVE_HARDDISK },
   { "ETK_STOCK_DRIVE_REMOVABLE_MEDIA", ETK_STOCK_DRIVE_REMOVABLE_MEDIA },
   { "ETK_STOCK_INPUT_GAMING", ETK_STOCK_INPUT_GAMING },
   { "ETK_STOCK_INPUT_KEYBOARD", ETK_STOCK_INPUT_KEYBOARD },
   { "ETK_STOCK_INPUT_MOUSE", ETK_STOCK_INPUT_MOUSE },
   { "ETK_STOCK_MEDIA_CDROM", ETK_STOCK_MEDIA_CDROM },
   { "ETK_STOCK_MEDIA_FLOPPY", ETK_STOCK_MEDIA_FLOPPY },
   { "ETK_STOCK_MULTIMEDIA_PLAYER", ETK_STOCK_MULTIMEDIA_PLAYER },
   { "ETK_STOCK_NETWORK", ETK_STOCK_NETWORK },
   { "ETK_STOCK_NETWORK_WIRELESS", ETK_STOCK_NETWORK_WIRELESS },
   { "ETK_STOCK_NETWORK_WIRED", ETK_STOCK_NETWORK_WIRED },
   { "ETK_STOCK_PRINTER", ETK_STOCK_PRINTER },
   { "ETK_STOCK_PRINTER_REMOTE", ETK_STOCK_PRINTER_REMOTE },
   { "ETK_STOCK_VIDEO_DISPLAY", ETK_STOCK_VIDEO_DISPLAY },
   { "ETK_STOCK_EMBLEM_FAVORITE", ETK_STOCK_EMBLEM_FAVORITE },
   { "ETK_STOCK_EMBLEM_IMPORTANT", ETK_STOCK_EMBLEM_IMPORTANT },
   { "ETK_STOCK_EMBLEM_PHOTOS", ETK_STOCK_EMBLEM_PHOTOS },
   { "ETK_STOCK_EMBLEM_READONLY", ETK_STOCK_EMBLEM_READONLY },
   { "ETK_STOCK_EMBLEM_SYMBOLIC_LINK", ETK_STOCK_EMBLEM_SYMBOLIC_LINK },
   { "ETK_STOCK_EMBLEM_SYSTEM", ETK_STOCK_EMBLEM_SYSTEM },
   { "ETK_STOCK_EMBLEM_UNREADABLE", ETK_STOCK_EMBLEM_UNREADABLE },
   { "ETK_STOCK_FACE_ANGEL", ETK_STOCK_FACE_ANGEL },
   { "ETK_STOCK_FACE_CRYING", ETK_STOCK_FACE_CRYING },
   { "ETK_STOCK_FACE_DEVIL_GRIN", ETK_STOCK_FACE_DEVIL_GRIN },
   { "ETK_STOCK_FACE_GLASSES", ETK_STOCK_FACE_GLASSES },
   { "ETK_STOCK_FACE_GRIN", ETK_STOCK_FACE_GRIN },
   { "ETK_STOCK_FACE_KISS", ETK_STOCK_FACE_KISS },
   { "ETK_STOCK_FACE_PLAIN", ETK_STOCK_FACE_PLAIN },
   { "ETK_STOCK_FACE_SAD", ETK_STOCK_FACE_SAD },
   { "ETK_STOCK_FACE_SMILE_BIG", ETK_STOCK_FACE_SMILE_BIG },
   { "ETK_STOCK_FACE_SMILE", ETK_STOCK_FACE_SMILE },
   { "ETK_STOCK_FACE_SURPRISE", ETK_STOCK_FACE_SURPRISE },
   { "ETK_STOCK_FACE_WINK", ETK_STOCK_FACE_WINK },
   { "ETK_STOCK_APPLICATION_CERTIFICATE", ETK_STOCK_APPLICATION_CERTIFICATE },
   { "ETK_STOCK_APPLICATION_X_EXECUTABLE", ETK_STOCK_APPLICATION_X_EXECUTABLE },
   { "ETK_STOCK_AUDIO_X_GENERIC", ETK_STOCK_AUDIO_X_GENERIC },
   { "ETK_STOCK_FONT_X_GENERIC", ETK_STOCK_FONT_X_GENERIC },
   { "ETK_STOCK_IMAGE_X_GENERIC", ETK_STOCK_IMAGE_X_GENERIC },
   { "ETK_STOCK_PACKAGE_X_GENERIC", ETK_STOCK_PACKAGE_X_GENERIC },
   { "ETK_STOCK_TEXT_HTML", ETK_STOCK_TEXT_HTML },
   { "ETK_STOCK_TEXT_X_GENERIC", ETK_STOCK_TEXT_X_GENERIC },
   { "ETK_STOCK_TEXT_X_GENERIC_TEMPLATE", ETK_STOCK_TEXT_X_GENERIC_TEMPLATE },
   { "ETK_STOCK_TEXT_X_SCRIPT", ETK_STOCK_TEXT_X_SCRIPT },
   { "ETK_STOCK_VIDEO_X_GENERIC", ETK_STOCK_VIDEO_X_GENERIC },
   { "ETK_STOCK_X_DIRECTORY_DESKTOP", ETK_STOCK_X_DIRECTORY_DESKTOP },
   { "ETK_STOCK_X_DIRECTORY_NORMAL_DRAG_ACCEPT", ETK_STOCK_X_DIRECTORY_NORMAL_DRAG_ACCEPT },
   { "ETK_STOCK_X_DIRECTORY_NORMAL_HOME", ETK_STOCK_X_DIRECTORY_NORMAL_HOME },
   { "ETK_STOCK_X_DIRECTORY_NORMAL_OPEN", ETK_STOCK_X_DIRECTORY_NORMAL_OPEN },
   { "ETK_STOCK_X_DIRECTORY_NORMAL", ETK_STOCK_X_DIRECTORY_NORMAL },
   { "ETK_STOCK_X_DIRECTORY_NORMAL_VISITING", ETK_STOCK_X_DIRECTORY_NORMAL_VISITING },
   { "ETK_STOCK_X_DIRECTORY_REMOTE", ETK_STOCK_X_DIRECTORY_REMOTE },
   { "ETK_STOCK_X_DIRECTORY_REMOTE_SERVER", ETK_STOCK_X_DIRECTORY_REMOTE_SERVER },
   { "ETK_STOCK_X_DIRECTORY_REMOTE_WORKGROUP", ETK_STOCK_X_DIRECTORY_REMOTE_WORKGROUP },
   { "ETK_STOCK_X_DIRECTORY_TRASH_FULL", ETK_STOCK_X_DIRECTORY_TRASH_FULL },
   { "ETK_STOCK_X_DIRECTORY_TRASH", ETK_STOCK_X_DIRECTORY_TRASH },
   { "ETK_STOCK_X_OFFICE_ADDRESS_BOOK", ETK_STOCK_X_OFFICE_ADDRESS_BOOK },
   { "ETK_STOCK_X_OFFICE_CALENDAR", ETK_STOCK_X_OFFICE_CALENDAR },   
   { "ETK_STOCK_X_OFFICE_DOCUMENT", ETK_STOCK_X_OFFICE_DOCUMENT },
   { "ETK_STOCK_X_OFFICE_PRESENTATION", ETK_STOCK_X_OFFICE_PRESENTATION },
   { "ETK_STOCK_X_OFFICE_SPREADSHEET", ETK_STOCK_X_OFFICE_SPREADSHEET },
   { "ETK_STOCK_PLACES_FOLDER", ETK_STOCK_PLACES_FOLDER },
   { "ETK_STOCK_PLACES_FOLDER_REMOTE", ETK_STOCK_PLACES_FOLDER_REMOTE },
   { "ETK_STOCK_PLACES_FOLDER_SAVED_SEARCH", ETK_STOCK_PLACES_FOLDER_SAVED_SEARCH },
   { "ETK_STOCK_PLACES_NETWORK_SERVER", ETK_STOCK_PLACES_NETWORK_SERVER },
   { "ETK_STOCK_PLACES_NETWORK_WORKGROUP", ETK_STOCK_PLACES_NETWORK_WORKGROUP },
   { "ETK_STOCK_PLACES_START_HERE", ETK_STOCK_PLACES_START_HERE },
   { "ETK_STOCK_PLACES_USER_DESKTOP", ETK_STOCK_PLACES_USER_DESKTOP },
   { "ETK_STOCK_PLACES_USER_HOME", ETK_STOCK_PLACES_USER_HOME },
   { "ETK_STOCK_PLACES_USER_TRASH", ETK_STOCK_PLACES_USER_TRASH },
   { "ETK_STOCK_AUDIO_VOLUME_HIGH", ETK_STOCK_AUDIO_VOLUME_HIGH },
   { "ETK_STOCK_AUDIO_VOLUME_LOW", ETK_STOCK_AUDIO_VOLUME_LOW },
   { "ETK_STOCK_AUDIO_VOLUME_MEDIUM", ETK_STOCK_AUDIO_VOLUME_MEDIUM },
   { "ETK_STOCK_AUDIO_VOLUME_MUTED", ETK_STOCK_AUDIO_VOLUME_MUTED },
   { "ETK_STOCK_BATTERY_CAUTION", ETK_STOCK_BATTERY_CAUTION },
   { "ETK_STOCK_DIALOG_ERROR", ETK_STOCK_DIALOG_ERROR },
   { "ETK_STOCK_DIALOG_INFORMATION", ETK_STOCK_DIALOG_INFORMATION },
   { "ETK_STOCK_DIALOG_WARNING", ETK_STOCK_DIALOG_WARNING },
   { "ETK_STOCK_DIALOG_QUESTION", ETK_STOCK_DIALOG_QUESTION },     
   { "ETK_STOCK_FOLDER_DRAG_ACCEPT", ETK_STOCK_FOLDER_DRAG_ACCEPT },
   { "ETK_STOCK_FOLDER_OPEN", ETK_STOCK_FOLDER_OPEN },
   { "ETK_STOCK_FOLDER_VISITING", ETK_STOCK_FOLDER_VISITING },
   { "ETK_STOCK_IMAGE_LOADING", ETK_STOCK_IMAGE_LOADING },
   { "ETK_STOCK_IMAGE_MISSING", ETK_STOCK_IMAGE_MISSING },
   { "ETK_STOCK_MAIL_ATTACHMENT", ETK_STOCK_MAIL_ATTACHMENT },
   { "ETK_STOCK_NETWORK_ERROR", ETK_STOCK_NETWORK_ERROR },
   { "ETK_STOCK_NETWORK_IDLE", ETK_STOCK_NETWORK_IDLE },
   { "ETK_STOCK_NETWORK_OFFLINE", ETK_STOCK_NETWORK_OFFLINE },
   { "ETK_STOCK_NETWORK_ONLINE", ETK_STOCK_NETWORK_ONLINE },
   { "ETK_STOCK_NETWORK_RECEIVE", ETK_STOCK_NETWORK_RECEIVE },
   { "ETK_STOCK_NETWORK_TRANSMIT", ETK_STOCK_NETWORK_TRANSMIT },
   { "ETK_STOCK_NETWORK_TRANSMIT_RECEIVE", ETK_STOCK_NETWORK_TRANSMIT_RECEIVE },
   { "ETK_STOCK_NETWORK_WIRELESS_ENCRYPTED", ETK_STOCK_NETWORK_WIRELESS_ENCRYPTED },
   { "ETK_STOCK_PRINTER_ERROR", ETK_STOCK_PRINTER_ERROR },
   { "ETK_STOCK_USER_TRASH_FULL", ETK_STOCK_USER_TRASH_FULL },
   { "gtk-home", ETK_STOCK_GO_HOME},
   { "gtk-go-up", ETK_STOCK_GO_UP},
   { "gtk-stock-timer", ETK_STOCK_APPOINTMENT_NEW},
   { "gtk-apply", ETK_STOCK_DIALOG_OK},
   { "gtk-ok", ETK_STOCK_DIALOG_OK},
   { "gtk-cancel", ETK_STOCK_DIALOG_CANCEL},
   { "gtk-yes", ETK_STOCK_DIALOG_YES},
   { "gtk-no", ETK_STOCK_DIALOG_NO},
   { "gtk-close", ETK_STOCK_DIALOG_CLOSE},
   { "gtk-new", ETK_STOCK_DOCUMENT_NEW},
   { "gtk-open", ETK_STOCK_DOCUMENT_OPEN},
   { "gtk-print", ETK_STOCK_DOCUMENT_PRINT},
   { "gtk-print-preview", ETK_STOCK_DOCUMENT_PRINT_PREVIEW},
   { "gtk-properties", ETK_STOCK_DOCUMENT_PROPERTIES},
   { "gtk-save-as", ETK_STOCK_DOCUMENT_SAVE_AS},
   { "gtk-save", ETK_STOCK_DOCUMENT_SAVE},
   { "gtk-clear", ETK_STOCK_EDIT_CLEAR},
   { "gtk-copy", ETK_STOCK_EDIT_COPY},
   { "gnome-stock-multiple-file", ETK_STOCK_EDIT_COPY},
   { "gtk-cut", ETK_STOCK_EDIT_CUT},
   { "gtk-find", ETK_STOCK_EDIT_FIND},
   { "gtk-paste", ETK_STOCK_EDIT_PASTE},
   { "gtk-redo", ETK_STOCK_EDIT_REDO},
   { "gtk-delete", ETK_STOCK_EDIT_DELETE},
   { "gtk-find-and-replace", ETK_STOCK_EDIT_FIND_REPLACE},
   { "gtk-unindent", ETK_STOCK_FORMAT_INDENT_LESS},
   { "gtk-indent", ETK_STOCK_FORMAT_INDENT_MORE},
   { "gtk-justify-center", ETK_STOCK_FORMAT_JUSTIFY_CENTER},
   { "gtk-justify-fill", ETK_STOCK_FORMAT_JUSTIFY_FILL},
   { "gtk-justify-left", ETK_STOCK_FORMAT_JUSTIFY_LEFT},
   { "gtk-justify-right", ETK_STOCK_FORMAT_JUSTIFY_RIGHT},
   { "gtk-bold", ETK_STOCK_FORMAT_TEXT_BOLD},
   { "gtk-italic", ETK_STOCK_FORMAT_TEXT_ITALIC},
   { "gtk-strikethrough", ETK_STOCK_FORMAT_TEXT_STRIKETHROUGH},
   { "gtk-underline", ETK_STOCK_FORMAT_TEXT_UNDERLINE},
   { "gtk-goto-bottom", ETK_STOCK_GO_BOTTOM},
   { "gtk-go-down", ETK_STOCK_GO_DOWN},
   { "gtk-goto-first", ETK_STOCK_GO_FIRST},
   { "gtk-home", ETK_STOCK_GO_HOME},
   { "gtk-jump-to", ETK_STOCK_GO_JUMP},
   { "gtk-goto-last", ETK_STOCK_GO_LAST},
   { "gtk-go-forward", ETK_STOCK_GO_NEXT},
   { "gtk-go-back", ETK_STOCK_GO_PREVIOUS},
   { "gtk-goto-top", ETK_STOCK_GO_TOP},
   { "gtk-go-up", ETK_STOCK_GO_UP},
   { "gtk-add", ETK_STOCK_LIST_ADD},
   { "gtk-remove", ETK_STOCK_LIST_REMOVE},
   { "gnome-stock-mail-new", ETK_STOCK_MAIL_MESSAGE_NEW},
   { "gtk-stock-mail-fwd", ETK_STOCK_MAIL_FORWARD},
   { "gnome-stock-mail-rpl", ETK_STOCK_MAIL_REPLY_SENDER},
   { "gtk-media-pause", ETK_STOCK_MEDIA_PLAYBACK_PAUSE},
   { "gtk-media-play", ETK_STOCK_MEDIA_PLAYBACK_START},
   { "gtk-media-record", ETK_STOCK_MEDIA_RECORD},
   { "gtk-media-rewind", ETK_STOCK_MEDIA_SEEK_BACKWARD},
   { "gtk-media-forward", ETK_STOCK_MEDIA_SEEK_FORWARD},
   { "gtk-media-previous", ETK_STOCK_MEDIA_SKIP_BACKWARD},
   { "gtk-media-next", ETK_STOCK_MEDIA_SKIP_FORWARD},
   { "gtk-stop", ETK_STOCK_PROCESS_STOP},
   { "gtk-quit", ETK_STOCK_SYSTEM_LOG_OUT},
   { "gtk-refresh", ETK_STOCK_VIEW_REFRESH},
   { "gtk-about", ETK_STOCK_HELP_BROWSER},
   { "gnome-stock-about", ETK_STOCK_HELP_BROWSER},
   { "gtk-help", ETK_STOCK_HELP_BROWSER},
   { "gtk-info", ETK_STOCK_HELP_BROWSER},
   { "gnome-stock-mail", ETK_STOCK_INTERNET_MAIL},
   { "gnome-stock-volume", ETK_STOCK_MULTIMEDIA_VOLUME_CONTROL},
   { "gtk-select-font", ETK_STOCK_PREFERENCES_DESKTOP_FONT},
   { "gtk-execute", ETK_STOCK_APPLICATIONS_SYSTEM},
   { "gtk-preferences", ETK_STOCK_PREFERENCES_SYSTEM},
   { "gnome-stock-mic", ETK_STOCK_AUDIO_INPUT_MICROPHONE},
   { "gtk-harddrive", ETK_STOCK_DRIVE_HARDDISK},
   { "gtk-cdrom", ETK_STOCK_MEDIA_CDROM},
   { "gtk-floppy", ETK_STOCK_MEDIA_FLOPPY},
   { "gtk-network", ETK_STOCK_NETWORK},
   { "gtk-file", ETK_STOCK_TEXT_X_GENERIC},
   { "gtk-directory", ETK_STOCK_X_DIRECTORY_NORMAL_OPEN},
   { "gnome-stock-trash-full", ETK_STOCK_X_DIRECTORY_TRASH_FULL},
   { "gnome-stock-trash", ETK_STOCK_X_DIRECTORY_TRASH},
   { "gtk-dialog-error", ETK_STOCK_DIALOG_ERROR},
   { "gtk-dialog-info", ETK_STOCK_DIALOG_INFORMATION},
   { "gtk-dialog-warning", ETK_STOCK_DIALOG_WARNING},
   { "gtk-dialog-question", ETK_STOCK_DIALOG_QUESTION},
   { "gnome-stock-attach", ETK_STOCK_MAIL_ATTACHMENT},
   { "gtk-missing-image", ETK_STOCK_IMAGE_MISSING},
};


#ifdef ENHANCE_MEM_DEBUG
Evas_Hash *mem_objects = NULL;
long int   mem_size = 0;
long int   mem_total = 0;
long int   mem_calloc = 0;
long int   mem_strdup = 0;

void *
mem_alloc(size_t count, size_t size, char *file, int line)
{
   void *ptr;
   char *ptrstr;
      
   ptr = calloc(count, size);
   ptrstr = calloc(64, sizeof(char));
   snprintf(ptrstr, 64 * sizeof(char), "%p", ptr);   
   mem_objects = evas_hash_add(mem_objects, ptrstr, ((void*)(size * count)));
   mem_size += (size * count);
   mem_total += (size * count);
   mem_calloc += (size * count);   
   printf("%s %d : (calloc) %ld bytes, total = %ld\n", file, line, (long int)(size * count), mem_size);
   free(ptrstr);   
   return ptr;
}

char *
strdup2(const char *str, char *file, int line)
{
   char *ptr;
   int   length;
   char *ptrstr;
   
   ptrstr = calloc(64, sizeof(char));
   length = strlen(str) + 1;
   ptr = calloc(length, sizeof(char));
   snprintf(ptrstr, 64 * sizeof(char), "%p", ptr);
   mem_objects = evas_hash_add(mem_objects, ptrstr, ((void*)(length * sizeof(char))));
   mem_size += (length * sizeof(char));
   mem_total += (length * sizeof(char));
   mem_strdup += (length * sizeof(char));   
   snprintf(ptr, length * sizeof(char), "%s", str);   
   printf("%s %d : (strdup) %ld bytes, total = %ld\n", file, line, (long int)(length * sizeof(char)), mem_size);
   free(ptrstr);
   return ptr;
}

#endif

static void
_e_property_handle(Enhance *en, EXML_Node *node)
{
   char *name;
   char *parent_id = NULL;
   char *parent_class = NULL;
   E_Widget *wid = NULL;
   
#define IF_PARENT_CLASS(class) \
   if(parent_class) \
       if(!strcmp(parent_class, class)) \
     
   
   if(!node->value)
     return;
   
   name = ecore_hash_get(node->attributes, "name");
   if(!name) return;
   
#if DEBUG   
   printf("Handling property: %s=%s (parent=%s)\n",
	  name, node->value,
	  (char*)ecore_hash_get(node->parent->attributes, "class"));
#endif
   
   if(node->parent)
     {
	parent_class = ecore_hash_get(node->parent->attributes, "class");
	parent_id = ecore_hash_get(node->parent->attributes, "id");
	wid = evas_hash_find(en->widgets, parent_id);
	if(!wid) return;
     }
   
#define PROPERTY_BOOL \
   Etk_Bool  value = ETK_TRUE; \
   \
   if(!strcasecmp(node->value, "true")) \
     value = ETK_TRUE; \
   else if (!strcasecmp(node->value, "false")) \
     value = ETK_FALSE;
   
#define PROPERTY_DOUBLE \
   double value = 0.0; \
   \
   value = atof(node->value);
   
#define PROPERTY_INT \
   int value = 0; \
   \
   value = atoi(node->value);

#define PROPERTY_STR \
   char *value = NULL; \
   \
   value = node->value;

   if(!strcmp(name, "visible"))
     {		
	PROPERTY_BOOL;
	etk_object_properties_set(ETK_OBJECT(wid->wid), "visible", value, NULL);
     }
      
   else if(!strcmp(name, "homogeneous"))
     {
	PROPERTY_BOOL;
	etk_object_properties_set(ETK_OBJECT(wid->wid), "homogeneous", value, NULL);
     }      
   
   else if(!strcmp(name, "spacing"))
     {
	PROPERTY_INT;
	etk_object_properties_set(ETK_OBJECT(wid->wid), "spacing", value, NULL);
     }

   else if(!strcmp(name, "label"))
     {
	PROPERTY_STR;
	etk_object_properties_set(ETK_OBJECT(wid->wid), "label", value, NULL);
     }
   
   else if(!strcmp(name, "text"))
     {
	PROPERTY_STR;
	etk_object_properties_set(ETK_OBJECT(wid->wid), "text", value, NULL);
     }   
   
   else if (!strcmp(name, "xalign"))
     {
	PROPERTY_DOUBLE;
	IF_PARENT_CLASS("GtkImage")
	  return;
	etk_object_properties_set(ETK_OBJECT(wid->wid), "xalign", value, NULL);
     }
         
   else if(!strcmp(name, "yalign"))
     {
	PROPERTY_DOUBLE;
	IF_PARENT_CLASS("GtkImage")
	  return;	
	etk_object_properties_set(ETK_OBJECT(wid->wid), "yalign", value, NULL);
     }
   
   else if(!strcmp(name, "xscale"))
     {
	PROPERTY_DOUBLE;
	etk_object_properties_set(ETK_OBJECT(wid->wid), "xscale", value, NULL);
     }
   
   else if(!strcmp(name, "yscale"))
     {
	PROPERTY_DOUBLE;
	etk_object_properties_set(ETK_OBJECT(wid->wid), "yscale", value, NULL);
     }
   
   else if(!strcmp(name, "title"))
     {
	IF_PARENT_CLASS("GtkWindow")	  
	  etk_window_title_set(ETK_WINDOW(wid->wid), node->value);	  
     }

   else if(!strcmp(name, "decorated"))
     {
	PROPERTY_BOOL;
	etk_window_decorated_set(ETK_WINDOW(wid->wid), value);
     }
   
   else if(!strcmp(name, "headers_visible"))
     {
	PROPERTY_BOOL;
	etk_object_properties_set(ETK_OBJECT(wid->wid), "headers_visible", value, NULL);	
     }
   
   else if(!strcmp(name, "has_resize_grip"))
     {
	PROPERTY_BOOL;
	etk_object_properties_set(ETK_OBJECT(wid->wid), "has_resize_grip", value, NULL);	
     }   
   
   else if(!strcmp(name, "n_columns"))
     {
	PROPERTY_INT;
	etk_object_properties_set(ETK_OBJECT(wid->wid), "num_cols", value, NULL);
     }
   
   else if(!strcmp(name, "n_rows"))
     {
	PROPERTY_INT;
	etk_object_properties_set(ETK_OBJECT(wid->wid), "num_rows", value, NULL);
     }
   
   else if(!strcmp(name, "fraction"))
     {
	PROPERTY_DOUBLE;
	etk_object_properties_set(ETK_OBJECT(wid->wid), "fraction", value, NULL);
     }
   
   else if(!strcmp(name, "pulse_step"))
     {
	PROPERTY_DOUBLE;
	etk_object_properties_set(ETK_OBJECT(wid->wid), "pulse_step", value, NULL);
     }   
   
   else if(!strcmp(name, "skip_taskbar_hint"))
     {
	PROPERTY_BOOL;
	etk_window_skip_taskbar_hint_set(ETK_WINDOW(wid-wid), value);
     }
   
   else if(!strcmp(name, "skip_pager_hint"))
     {
	PROPERTY_BOOL;
	etk_window_skip_pager_hint_set(ETK_WINDOW(wid-wid), value);
     }   

   else if(!strcmp(name, "label"))
     {
	IF_PARENT_CLASS("GtkButton")
	  etk_button_label_set(ETK_BUTTON(wid->wid), node->value);	  
     }
   
   else if(!strcmp(name, "items"))
     {
	IF_PARENT_CLASS("GtkComboBox")
	  {
	     char *tok;
	     char *value;
	     
	     etk_combobox_column_add(ETK_COMBOBOX(wid->wid), ETK_COMBOBOX_LABEL, 15, ETK_TRUE, ETK_FALSE, ETK_FALSE, 0.0, 0.5);
	     etk_combobox_build(ETK_COMBOBOX(wid->wid));
	     value = strdup(node->value);
	     tok = strtok(value, "\n");
	     do
	       {
		  etk_combobox_item_append(ETK_COMBOBOX(wid->wid), tok);
	       }
	     while ((tok = strtok(NULL, "\n")) != NULL);	     
	  }
     }	  
   
   else if(!strcmp(name, "height_request"))
     {
	PROPERTY_INT;
	etk_object_properties_set(ETK_OBJECT(wid->wid), "height_request", value, NULL);
     }
   
   else if(!strcmp(name, "width_request"))
     {
	PROPERTY_INT;
	etk_object_properties_set(ETK_OBJECT(wid->wid), "width_request", value, NULL);
     }     
   
   else if(!strcmp(name, "stock"))
     {
	Etk_Stock_Id id;
	PROPERTY_STR;

	_en_stock_items_hash_init();      
	id = (Etk_Stock_Id)ecore_hash_get(_en_stock_items_hash, value);
	etk_object_properties_set(ETK_OBJECT(wid->wid), "stock_id", (Etk_Stock_Id)id, NULL);
     }

   else if(!strcmp(name, "icon_size"))
     {
	PROPERTY_INT;
	Etk_Stock_Id id = ETK_STOCK_MEDIUM;
	if (value <= 2) id = ETK_STOCK_SMALL;
	else if (value >= 5) id = ETK_STOCK_BIG;
	etk_object_properties_set(ETK_OBJECT(wid->wid), "stock_size", id, NULL);
     }

   else if(!strcmp(name, "use_stock"))
     {
	PROPERTY_BOOL;
	char *label;
	Etk_Stock_Id id = ETK_STOCK_NO_STOCK;
	
	if (value)
	  {
	     IF_PARENT_CLASS("GtkButton")
	       {
						 
		  _en_stock_items_hash_init();
		  label = (char *)etk_button_label_get(ETK_BUTTON(wid->wid));
		  id = (Etk_Stock_Id)ecore_hash_get(_en_stock_items_hash, label);
		  if (id != ETK_STOCK_NO_STOCK)
		    etk_button_set_from_stock(ETK_BUTTON(wid->wid), (Etk_Stock_Id)id);
	       }
	     IF_PARENT_CLASS("GtkImageMenuItem")
	       {

                  _en_stock_items_hash_init();
		  label = (char *)etk_menu_item_label_get(ETK_MENU_ITEM(wid->wid));
		  id = (Etk_Stock_Id)ecore_hash_get(_en_stock_items_hash, label);
		  if (id != ETK_STOCK_NO_STOCK)
		    etk_menu_item_set_from_stock(ETK_MENU_ITEM(wid->wid), (Etk_Stock_Id)id);
	       }
	  }
     }
}

static void
_e_signal_handle(Enhance *en, EXML_Node *node)
{
   E_Widget *wid;
   char     *name;   
   char     *parent_id;
   char     *handler;
   void     *handle;
   void     *data = NULL;
   etk_callback_type func;   
   
   name = ecore_hash_get(node->attributes, "name");
   if(!name) return;
      
   handler = ecore_hash_get(node->attributes, "handler");
   if(!handler) return;
   
   handle = dlopen(NULL, RTLD_NOW | RTLD_LOCAL);
   if(!handle)
     return;
   
   func = dlsym(handle, handler);
   if(!func)
     {
	printf("ENHANCE ERROR!!!\n"
	       "Error loading dynamic callback: %s\n"
	       "%s\n",
	       handler, dlerror());
	return;
     }
   
   parent_id = ecore_hash_get(node->parent->attributes, "id");
   if(!parent_id) return;
   wid = evas_hash_find(en->widgets, parent_id);
   if(!wid) return;
   
   data = evas_hash_find(en->callback_data, handler);
   
   etk_signal_connect(name, ETK_OBJECT(wid->wid), 
		      ETK_CALLBACK(func), data);
}   

static void
_e_traverse_packing_xml(Enhance *en, E_Widget *widget)
{
   EXML *xml;
   char *tag;
   EXML_Node *node;
   E_Widget_Packing *packing;
         
   xml = en->xml;
   
   if((tag = exml_down(xml)) == NULL)
     return;
   
   node = exml_get(xml);
   
   packing = E_NEW(1, E_Widget_Packing);
   packing->padding       = 0;
   packing->expand        = ETK_TRUE;
   packing->fill          = ETK_TRUE;
   packing->left_attach   = 0;
   packing->right_attach  = 0;
   packing->top_attach    = 0;
   packing->bottom_attach = 0;
   packing->x_padding     = 0;
   packing->y_padding     = 0;
   packing->x_options     = NULL;
   packing->y_options     = NULL;
   packing->shrink        = ETK_FALSE;
   
#define IF_TRUE_FALSE_ASSIGN(value, variable) \
      do \
	{ \
	   if(!strcasecmp(value, "true")) \
	     variable = ETK_TRUE; \
	   else if(!strcasecmp(value, "false")) \
	     variable = ETK_FALSE; \
	} \
      while(0)
      
   do
     {
	char *str = NULL;
	
	node = exml_get(xml);
	
	if((str = ecore_hash_get(node->attributes, "name")) != NULL
	   && node->value)
	  {
	     if(!strcmp("padding", str))
	       packing->padding = atoi(node->value);	     
	     else if(!strcmp("expand", str))	       
	       IF_TRUE_FALSE_ASSIGN(node->value, packing->expand);
	     else if(!strcmp("fill", str))
	       IF_TRUE_FALSE_ASSIGN(node->value, packing->fill);
	     else if(!strcmp("left_attach", str))
	       packing->left_attach = atoi(node->value);
	     else if(!strcmp("right_attach", str))
	       packing->right_attach = atoi(node->value);
	     else if(!strcmp("top_attach", str))
	       packing->top_attach = atoi(node->value);
	     else if(!strcmp("bottom_attach", str))
	       packing->bottom_attach = atoi(node->value);
	     else if(!strcmp("x_options", str))
	       packing->x_options = strdup(node->value);
	     else if(!strcmp("y_options", str))
	       packing->y_options = strdup(node->value);
	     else if(!strcmp("x_padding", str))
	       packing->x_padding = atoi(node->value);
	     else if(!strcmp("y_padding", str))
	       packing->y_padding = atoi(node->value);
	     else if(!strcmp("type", str))
	       packing->type = strdup(node->value);
	     else if(!strcmp("shrink", str))
	       IF_TRUE_FALSE_ASSIGN(node->value, packing->shrink);
	  }
     }
   while((tag = exml_next_nomove(xml)) != NULL);
   
   widget->packing = packing;   
   
   exml_up(xml);   
}

static void   
_e_traverse_property_xml(Enhance *en)
{
   EXML *xml;  
   EXML_Node *node;
   
   xml = en->xml;
   node = exml_get(xml);
	
   _e_property_handle(en, node);
}

static void   
_e_traverse_signal_xml(Enhance *en)
{
   EXML *xml;
   EXML_Node *node;
   
   xml = en->xml;
   node = exml_get(xml);
	
   _e_signal_handle(en, node);
}


static void
_e_traverse_child_xml(Enhance *en)
{
   EXML *xml;  
   char *tag;
   EXML_Node *node;
   char *parent_id;
   E_Widget *widget = NULL;
   E_Widget *parent = NULL;
   
   xml = en->xml;
   
#if DEBUG   
   printf("entering child!\n");
#endif
   
   if((tag = exml_down(xml)) == NULL)
     return;   
   
   do
     {	
	node = exml_get(xml);
	if(!strcmp(tag, "widget"))
	  {
#if DEBUG   	    
	     printf("widget = %s\n", 
		    (char*)ecore_hash_get(node->attributes, "class"));
#endif	     
	     widget = _e_traverse_widget_xml(en);
	  }
	else if(!strcmp(tag, "packing"))
	  {
	     if(widget != NULL)
	       _e_traverse_packing_xml(en, widget);
	  }	
     }
   while((tag = exml_next_nomove(xml)) != NULL);
   
   if(widget != NULL)
     {
	if(widget->node != NULL)
	  if(widget->node->parent != NULL)
	    if(widget->node->parent->parent != NULL)
	      if(widget->node->parent->parent->attributes > 0)
		{		 
		   parent_id = ecore_hash_get(widget->node->parent->parent->attributes, "id");
		   if(parent_id)
		     {
			parent = evas_hash_find(en->widgets, parent_id);
			if(parent)
			  _e_widget_parent_add(parent, widget);
		     }
		}
     }
   
   exml_up(xml);      
}
  
static E_Widget *
_e_traverse_widget_xml(Enhance *en)
{
   EXML *xml;
   char *tag;
   EXML_Node *node;
   E_Widget *widget;

   xml = en->xml;
   
#if DEBUG      
   printf("entering widget!\n");
#endif   
   node = exml_get(xml);   
   widget = _e_widget_handle(en, node);
   
   if((tag = exml_down(xml)) == NULL)
     return widget;
   
   do
     {
	node = exml_get(xml);
	
	if(!strcmp(tag, "property"))
	  {
	     _e_traverse_property_xml(en);
	  }
	else if(!strcmp(tag, "signal"))
	  {
	     _e_traverse_signal_xml(en);
	  }
	else if(!strcmp(tag, "child"))
	  {
	     _e_traverse_child_xml(en);
	  }	
     }
   while((tag = exml_next_nomove(xml)) != NULL);
   exml_up(xml);
   
   return widget;
}

static void
_e_traverse_xml(Enhance *en)
{
   EXML *xml;
   char *tag;
   EXML_Node *node;
   
   xml = en->xml;
   
   if((tag = exml_down(xml)) == NULL)
     return;
   
   do
     {
	node = exml_get(xml);
		
	if(!strcmp(tag, "widget"))
	  {
	     _e_traverse_widget_xml(en);
	  }		
     }
   while((tag = exml_next_nomove(xml)) != NULL);
   exml_up(xml);
}


void
enhance_file_load(Enhance *en, char *main_window, char *file)
{
   EXML *xml;
   EXML_Node *node;
   
   xml = exml_new();
   en->xml = xml;
   
   if(!exml_file_read(xml, file))
     {
	printf("Cant read file: %s\n", file);       
     }
   
   en->main_window = strdup(main_window);
   node = exml_get(xml);
   
   _e_traverse_xml(en);
}

Etk_Widget *
enhance_var_get(Enhance *en, char *string)
{
   E_Widget *widget;
   
   if(!string) return NULL;
   
   if((widget = evas_hash_find(en->widgets, string)) != NULL)
     {
	return widget->wid;
     }

   return NULL;
}

void
enhance_callback_data_set(Enhance *en, char *cb_name, void *data)
{
   en->callback_data = evas_hash_add(en->callback_data, cb_name, data);
}

void *
enhance_callback_data_get(Enhance *en, char *cb_name)
{
   return evas_hash_find(en->callback_data, cb_name);
}

Enhance *
enhance_new()
{
   Enhance *en;
   
   en = E_NEW(1, Enhance);
   
   return en;
}

static Evas_Bool
_e_widget_hash_free(Evas_Hash *hash, const char *key, void *data, void *fdata)
{
   E_Widget *widget;
   
   widget = data;
   evas_hash_del(hash, key, data);
   if(widget->packing)
     {
	E_FREE(widget->packing->x_options);
	E_FREE(widget->packing->y_options);
	E_FREE(widget->packing->type);
	E_FREE(widget->packing);
     }
   E_FREE(widget);
   
   return 1;
}

void
enhance_free(Enhance *en)
{
   if(!en) return;
   exml_destroy(en->xml);
   evas_hash_foreach(en->widgets, _e_widget_hash_free, en);
	ecore_hash_destroy(_en_stock_items_hash);
   E_FREE(en->main_window);
   E_FREE(en);   
}

void
enhance_init()
{
   ecore_init();
   evas_init();
}

void
enhance_shutdown()
{   
#ifdef ENHANCE_MEM_DEBUG
   printf("\n\n*** MEMORY DEBUG STATISTICS ***\n"
	  "Total memory used:\t %ld bytes\n"
	  "Memory used by calloc:\t %ld bytes\n"
	  "Memory used by strdup:\t %ld bytes\n"
	  "Unfreed memory:\t %ld bytes\n",
	  mem_total, mem_calloc, mem_strdup, mem_size);
#endif  
}

static void
_en_stock_items_hash_init(void)
{
	int size, i;

	/* the hash table is build only once */
	if (!_en_stock_items_hash)
	{
		_en_stock_items_hash = ecore_hash_new(ecore_str_hash, ecore_str_compare);
		size = sizeof(en_stock_items)/sizeof(en_stock_items[0]);
		for (i=0; i<size; i++)
		{
			ecore_hash_set(_en_stock_items_hash, en_stock_items[i].str, (Etk_Stock_Id *)en_stock_items[i].id);
		}
	}
}
