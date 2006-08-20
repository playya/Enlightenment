#ifndef EWL_ICON_THEME_H
#define EWL_ICON_THEME_H

/**
 * @addtogroup Ewl_Icon_Theme Ewl_Icon_Theme: The icon theme code
 * @brief Provides code to retrieve the path to icon theme icons
 *
 * @{
 */

void	 	 ewl_icon_theme_theme_change(void);

const char	*ewl_icon_theme_icon_path_get(const char *icon, 
						const char *size);

#define EWL_ICON_SIZE_SMALL "16x16"	/**< 16x16 icons */
#define EWL_ICON_SIZE_MEDIUM "22x22"	/**< 22x22 icons */
#define EWL_ICON_SIZE_LARGE "24x24"	/**< 24x24 icons */

#define EWL_ICON_ADDRESS_BOOK_NEW "address-book-new"	/**< */
#define EWL_ICON_APPLICATION_EXIT "application-exit"	/**< */
#define EWL_ICON_APPOINTMENT_NEW "appointment-new"	/**< */
#define EWL_ICON_CONTACT_NEW "contact-new"	/**< */
#define EWL_ICON_DIALOG_CANCEL "dialog-cancel"	/**< */
#define EWL_ICON_DIALOG_CLOSE "dialog-close"	/**< */
#define EWL_ICON_DIALOG_OK "dialog-ok"	/**< */
#define EWL_ICON_DOCUMENT_NEW "document-new"	/**< */
#define EWL_ICON_DOCUMENT_OPEN "document-open"	/**< */
#define EWL_ICON_DOCUMENT_OPEN_RECENT "document-open-recent"	/**< */
#define EWL_ICON_DOCUMENT_PAGE_SETUP "document-page-setup"	/**< */
#define EWL_ICON_DOCUMENT_PRINT "document-print"	/**< */
#define EWL_ICON_DOCUMENT_PRINT_PREVIEW "document-print-preview"	/**< */
#define EWL_ICON_DOCUMENT_PROPERTIES "document-properties"	/**< */
#define EWL_ICON_DOCUMENT_REVERT "document-revert"	/**< */
#define EWL_ICON_DOCUMENT_SAVE "document-save"	/**< */
#define EWL_ICON_DOCUMENT_SAVE_AS "document-save-as"	/**< */
#define EWL_ICON_EDIT_COPY "edit-copy"	/**< */
#define EWL_ICON_EDIT_CUT "edit-cut"	/**< */
#define EWL_ICON_EDIT_DELETE "edit-delete"	/**< */
#define EWL_ICON_EDIT_FIND "edit-find"	/**< */
#define EWL_ICON_EDIT_FIND_REPLACE "edit-find-replace"	/**< */
#define EWL_ICON_EDIT_PASTE "edit-paste"	/**< */
#define EWL_ICON_EDIT_REDO "edit-redo"	/**< */
#define EWL_ICON_EDIT_SELECT_ALL "edit-select-all"	/**< */
#define EWL_ICON_EDIT_UNDO "edit-undo"	/**< */
#define EWL_ICON_FORMAT_INDENT_LESS "format-indent-less"	/**< */
#define EWL_ICON_FORMAT_INDENT_MORE "format-indent-more"	/**< */
#define EWL_ICON_FORMAT_JUSTIFY_CENTER "format-justify-center"	/**< */
#define EWL_ICON_FORMAT_JUSTIFY_FILL "format-justify-fill"	/**< */
#define EWL_ICON_FORMAT_JUSTIFY_LEFT "format-justify-left"	/**< */
#define EWL_ICON_FORMAT_JUSTIFY_RIGHT "format-justify-right"	/**< */
#define EWL_ICON_FORMAT_TEXT_DIRECTION_LTR "format-text-direction-ltr"	/**< */
#define EWL_ICON_FORMAT_TEXT_DIRECTION_RTL "format-text-direction-rtl"	/**< */
#define EWL_ICON_FORMAT_TEXT_BOLD "format-text-bold"	/**< */
#define EWL_ICON_FORMAT_TEXT_ITALIC "format-text-italic"	/**< */
#define EWL_ICON_FORMAT_TEXT_UNDERLINE "format-text-underline"	/**< */
#define EWL_ICON_FORMAT_TEXT_STRIKETHROUGH "format-text-strikethrough"	/**< */
#define EWL_ICON_GO_BOTTOM "go-bottom"	/**< */
#define EWL_ICON_GO_DOWN "go-down"	/**< */
#define EWL_ICON_GO_FIRST "go-first"	/**< */
#define EWL_ICON_GO_HOME "go-home"	/**< */
#define EWL_ICON_GO_JUMP "go-jump"	/**< */
#define EWL_ICON_GO_LAST "go-last"	/**< */
#define EWL_ICON_GO_NEXT "go-next"	/**< */
#define EWL_ICON_GO_PREVIOUS "go-previous"	/**< */
#define EWL_ICON_GO_TOP "go-top"	/**< */
#define EWL_ICON_GO_UP "go-up"	/**< */
#define EWL_ICON_HELP_ABOUT "help-about"	/**< */
#define EWL_ICON_HELP_CONTENTS "help-contents"	/**< */
#define EWL_ICON_HELP_FAQ "help-faq"	/**< */
#define EWL_ICON_INSERT_IMAGE "insert-image"	/**< */
#define EWL_ICON_INSERT_LINK "insert-link"	/**< */
#define EWL_ICON_INSERT_OBJECT "insert-object"	/**< */
#define EWL_ICON_INSERT_TEXT "insert-text"	/**< */
#define EWL_ICON_LIST_ADD "list-add"	/**< */
#define EWL_ICON_LIST_REMOVE "list-remove"	/**< */
#define EWL_ICON_MAIL_FORWARD "mail-forward"	/**< */
#define EWL_ICON_MAIL_MARK_IMPORTANT "mail-mark-important"	/**< */
#define EWL_ICON_MAIL_MARK_JUNK "mail-mark-junk"	/**< */
#define EWL_ICON_MAIL_MARK_NOTJUNK "mail-mark-notjunk"	/**< */
#define EWL_ICON_MAIL_MARK_READ "mail-mark-read"	/**< */
#define EWL_ICON_MAIL_MARK_UNREAD "mail-mark-unread"	/**< */
#define EWL_ICON_MAIL_MESSAGE_NEW "mail-message-new"	/**< */
#define EWL_ICON_MAIL_REPLY_ALL "mail-reply-all"	/**< */
#define EWL_ICON_MAIL_REPLY_SENDER "mail-reply-sender"	/**< */
#define EWL_ICON_MAIL_SEND_RECEIVE "mail-send-receive"	/**< */
#define EWL_ICON_MEDIA_EJECT "media-eject"	/**< */
#define EWL_ICON_MEDIA_PLAYBACK_PAUSE "media-playback-pause"	/**< */
#define EWL_ICON_MEDIA_PLAYBACK_START "media-playback-start"	/**< */
#define EWL_ICON_MEDIA_PLAYBACK_STOP "media-playback-stop"	/**< */
#define EWL_ICON_MEDIA_RECORD "media-record"	/**< */
#define EWL_ICON_MEDIA_SEEK_BACKWARD "media-seek-backward"	/**< */
#define EWL_ICON_MEDIA_SEEK_FORWARD "media-seek-forward"	/**< */
#define EWL_ICON_MEDIA_SKIP_BACKWARD "media-skip-backward"	/**< */
#define EWL_ICON_MEDIA_SKIP_FORWARD "media-skip-forward"	/**< */
#define EWL_ICON_SYSTEM_LOCK_SCREEN "system-lock-screen"	/**< */
#define EWL_ICON_SYSTEM_LOG_OUT "system-log-out"	/**< */
#define EWL_ICON_SYSTEM_RUN "system-run"	/**< */
#define EWL_ICON_SYSTEM_SEARCH "system-search"	/**< */
#define EWL_ICON_TOOLS_CHECK_SPELLING "tools-check-spelling"	/**< */
#define EWL_ICON_VIEW_FULLSCREEN "view-fullscreen"	/**< */
#define EWL_ICON_VIEW_REFRESH "view-refresh"	/**< */
#define EWL_ICON_VIEW_SORT_ASCENDING "view-sort-ascending"	/**< */
#define EWL_ICON_VIEW_SORT_DESCENDING "view-sort-descending"	/**< */
#define EWL_ICON_WINDOW_CLOSE "window-close"	/**< */
#define EWL_ICON_WINDOW_NEW "window-new"	/**< */
#define EWL_ICON_ZOOM_BEST_FIT "zoom-best-fit"	/**< */
#define EWL_ICON_ZOOM_IN "zoom-in"	/**< */
#define EWL_ICON_ZOOM_ORIGINAL "zoom-original"	/**< */
#define EWL_ICON_ZOOM_OUT "zoom-out"	/**< */

#define EWL_ICON_PROCESS_WORKING "process-working"	/**< */

#define EWL_ICON_ACCESSORIES_CALCULATOR "accessories-calculator"	/**< */
#define EWL_ICON_ACCESSORIES_CHARACTER_MAP "accessories-character-map"	/**< */
#define EWL_ICON_ACCESSORIES_DICTIONARY "accessories-dictionary"	/**< */
#define EWL_ICON_ACCESSORIES_TEXT_EDITOR "accessories-text-editor"	/**< */
#define EWL_ICON_HELP_BROWSER "help-browser"	/**< */
#define EWL_ICON_MULTIMEDIA_VOLUME_CONTROL "multimedia-volume-control"	/**< */
#define EWL_ICON_PREFERENCES_DESKTOP_ACCESSIBILITY "preferences-desktop-accessibility"	/**< */
#define EWL_ICON_PREFERENCES_DESKTOP_FONT "preferences-desktop-font"	/**< */
#define EWL_ICON_PREFERENCES_DESKTOP_KEYBOARD "preferences-desktop-keyboard"	/**< */
#define EWL_ICON_PREFERENCES_DESKTOP_LOCALE "preferences-desktop-locale"	/**< */
#define EWL_ICON_PREFERENCES_DESKTOP_MULTIMEDIA "preferences-desktop-multimedia"	/**< */
#define EWL_ICON_PREFERENCES_DESKTOP_SCREENSAVER "preferences-desktop-screensaver"	/**< */
#define EWL_ICON_PREFERENCES_DESKTOP_THEME "preferences-desktop-theme"	/**< */
#define EWL_ICON_PREFERENCES_DESKTOP_WALLPAPER "preferences-desktop-wallpaper"	/**< */
#define EWL_ICON_SYSTEM_FILE_MANAGER "system-file-manager"	/**< */
#define EWL_ICON_SYSTEM_SOFTWARE_UPDATE "system-software-update"	/**< */
#define EWL_ICON_UTILITIES_TERMINAL "utilities-terminal"	/**< */

#define EWL_ICON_APPLICATIONS_ACCESSORIES "applications-accessories"	/**< */
#define EWL_ICON_APPLICATIONS_DEVELOPMENT "applications-development"	/**< */
#define EWL_ICON_APPLICATIONS_GAMES "applications-games"	/**< */
#define EWL_ICON_APPLICATIONS_GRAPHICS "applications-graphics"	/**< */
#define EWL_ICON_APPLICATIONS_INTERNET "applications-internet"	/**< */
#define EWL_ICON_APPLICATIONS_MULTIMEDIA "applications-multimedia"	/**< */
#define EWL_ICON_APPLICATIONS_OFFICE "applications-office"	/**< */
#define EWL_ICON_APPLICATIONS_OTHER "applications-other"	/**< */
#define EWL_ICON_APPLICATIONS_SYSTEM "applications-system"	/**< */
#define EWL_ICON_APPLICATIONS_UTILITIES "applications-utilities"	/**< */
#define EWL_ICON_PREFERENCES_DESKTOP "preferences-desktop"	/**< */
#define EWL_ICON_PREFERENCES_DESKTOP_ACCESSIBILITY "preferences-desktop-accessibility"	/**< */
#define EWL_ICON_PREFERENCES_DESKTOP_PERIPHERALS "preferences-desktop-peripherals"	/**< */
#define EWL_ICON_PREFERENCES_DESKTOP_PERSONAL "preferences-desktop-personal"	/**< */
#define EWL_ICON_PREFERENCES_OTHER "preferences-other"	/**< */
#define EWL_ICON_PREFERENCES_SYSTEM "preferences-system"	/**< */
#define EWL_ICON_PREFERENCES_SYSTEM_NETWORK "preferences-system-network"	/**< */
#define EWL_ICON_SYSTEM_HELP "system-help"	/**< */

#define EWL_ICON_AUDIO_CARD "audio-card"	/**< */
#define EWL_ICON_AUDIO_INPUT_MICROPHONE "audio-input-microphone"	/**< */
#define EWL_ICON_BATTERY "battery"	/**< */
#define EWL_ICON_CAMERA_PHOTO "camera-photo"	/**< */
#define EWL_ICON_CAMERA_VIDEO "camera-video"	/**< */
#define EWL_ICON_COMPUTER "computer"	/**< */
#define EWL_ICON_DRIVE_CDROM "drive-cdrom"	/**< */
#define EWL_ICON_DRIVE_HARDDISK "drive-harddisk"	/**< */
#define EWL_ICON_DRIVE_REMOVABLE_MEDIA "drive-removable-media"	/**< */
#define EWL_ICON_INPUT_GAMING "input-gaming"	/**< */
#define EWL_ICON_INPUT_KEYBOARD "input-keyboard"	/**< */
#define EWL_ICON_INPUT_MOUSE "input-mouse"	/**< */
#define EWL_ICON_MEDIA_CDROM "media-cdrom"	/**< */
#define EWL_ICON_MEDIA_FLOPPY "media-floppy"	/**< */
#define EWL_ICON_MULTIMEDIA_PLAYER "multimedia-player"	/**< */
#define EWL_ICON_NETWORK_WIRED "network-wired"	/**< */
#define EWL_ICON_NETWORK_WIRELESS "network-wireless"	/**< */
#define EWL_ICON_PRINTER "printer"	/**< */

#define EWL_ICON_EMBLEM_DEFAULT "emblem-default"	/**< */
#define EWL_ICON_EMBLEM_DOCUMENTS "emblem-documents"	/**< */
#define EWL_ICON_EMBLEM_DOWNLOADS "emblem-downloads"	/**< */
#define EWL_ICON_EMBLEM_FAVORITE "emblem-favorite"	/**< */
#define EWL_ICON_EMBLEM_IMPORTANT "emblem-important"	/**< */
#define EWL_ICON_EMBLEM_MAIL "emblem-mail"	/**< */
#define EWL_ICON_EMBLEM_PHOTOS "emblem-photos"	/**< */
#define EWL_ICON_EMBLEM_READONLY "emblem-readonly"	/**< */
#define EWL_ICON_EMBLEM_SHARED "emblem-shared"	/**< */
#define EWL_ICON_EMBLEM_SYMBOLIC_LINK "emblem-symbolic-link"	/**< */
#define EWL_ICON_EMBLEM_SYNCHRONIZED "emblem-synchronized"	/**< */
#define EWL_ICON_EMBLEM_SYSTEM "emblem-system"	/**< */
#define EWL_ICON_EMBLEM_UNREADABLE "emblem-unreadable"	/**< */

#define EWL_ICON_FACE_ANGEL "face-angel"	/**< */
#define EWL_ICON_FACE_CRYING "face-crying"	/**< */
#define EWL_ICON_FACE_DEVIL_GRIN "face-devil-grin"	/**< */
#define EWL_ICON_FACE_DEVIL_SAD "face-devil-sad"	/**< */
#define EWL_ICON_FACE_GLASSES "face-glasses"	/**< */
#define EWL_ICON_FACE_KISS "face-kiss"	/**< */
#define EWL_ICON_FACE_MONKEY "face-monkey"	/**< */
#define EWL_ICON_FACE_PLAIN "face-plain"	/**< */
#define EWL_ICON_FACE_SAD "face-sad"	/**< */
#define EWL_ICON_FACE_SMILE "face-smile"	/**< */
#define EWL_ICON_FACE_SMILE_BIG "face-smile-big"	/**< */
#define EWL_ICON_FACE_SMIRK "face-smirk"	/**< */
#define EWL_ICON_FACE_SURPRISE "face-surprise"	/**< */
#define EWL_ICON_FACE_WINK "face-wink"	/**< */

#define EWL_ICON_APPLICATION_X_EXECUTABLE "application-x-executable"	/**< */
#define EWL_ICON_AUDIO_X_GENERIC "audio-x-generic"	/**< */
#define EWL_ICON_FONT_X_GENERIC "font-x-generic"	/**< */
#define EWL_ICON_IMAGE_X_GENERIC "image-x-generic"	/**< */
#define EWL_ICON_PACKAGE_X_GENERIC "package-x-generic"	/**< */
#define EWL_ICON_TEXT_HTML "text-html"	/**< */
#define EWL_ICON_TEXT_X_GENERIC "text-x-generic"	/**< */
#define EWL_ICON_TEXT_X_GENERIC_TEMPLATE "text-x-generic-template"	/**< */
#define EWL_ICON_TEXT_X_SCRIPT "text-x-script"	/**< */
#define EWL_ICON_VIDEO_X_GENERIC "video-x-generic"	/**< */
#define EWL_ICON_X_OFFICE_ADDRESS_BOOK "x-office-address-book"	/**< */
#define EWL_ICON_X_OFFICE_CALENDAR "x-office-calendar"	/**< */
#define EWL_ICON_X_OFFICE_DOCUMENT "x-office-document"	/**< */
#define EWL_ICON_X_OFFICE_PRESENTATION "x-office-presentation"	/**< */
#define EWL_ICON_X_OFFICE_SPREADSHEET "x-office-spreadsheet"	/**< */

#define EWL_ICON_FOLDER "folder"	/**< */
#define EWL_ICON_FOLDER_REMOTE "folder-remote"	/**< */
#define EWL_ICON_NETWORK_SERVER "network-server"	/**< */
#define EWL_ICON_NETWORK_WORKGROUP "network-workgroup"	/**< */
#define EWL_ICON_START_HERE "start-here"	/**< */
#define EWL_ICON_USER_DESKTOP "user-desktop"	/**< */
#define EWL_ICON_USER_HOME "user-home"	/**< */
#define EWL_ICON_USER_TRASH "user-trash"	/**< */

#define EWL_ICON_APPOINTMENT_MISSED "appointment-missed"	/**< */
#define EWL_ICON_APPOINTMENT_SOON "appointment-soon"	/**< */
#define EWL_ICON_AUDIO_VOLUME_HIGH "audio-volume-high"	/**< */
#define EWL_ICON_AUDIO_VOLUME_LOW "audio-volume-low"	/**< */
#define EWL_ICON_AUDIO_VOLUME_MEDIUM "audio-volume-medium"	/**< */
#define EWL_ICON_AUDIO_VOLUME_MUTED "audio-volume-muted"	/**< */
#define EWL_ICON_BATTERY_CAUTION "battery-caution"	/**< */
#define EWL_ICON_BATTERY_LOW "battery-low"	/**< */
#define EWL_ICON_DIALOG_ERROR "dialog-error"	/**< */
#define EWL_ICON_DIALOG_INFORMATION "dialog-information"	/**< */
#define EWL_ICON_DIALOG_PASSWORD "dialog-password"	/**< */
#define EWL_ICON_DIALOG_QUESTION "dialog-question"	/**< */
#define EWL_ICON_DIALOG_WARNING "dialog-warning"	/**< */
#define EWL_ICON_FOLDER_DRAG_ACCEPT "folder-drag-accept"	/**< */
#define EWL_ICON_FOLDER_OPEN "folder-open"	/**< */
#define EWL_ICON_FOLDER_VISITING "folder-visiting"	/**< */
#define EWL_ICON_IMAGE_LOADING "image-loading"	/**< */
#define EWL_ICON_IMAGE_MISSING "image-missing"	/**< */
#define EWL_ICON_MAIL_ATTACHMENT "mail-attachment"	/**< */
#define EWL_ICON_MAIL_UNREAD "mail-unread"	/**< */
#define EWL_ICON_MAIL_READ "mail-read"	/**< */
#define EWL_ICON_MAIL_REPLIED "mail-replied"	/**< */
#define EWL_ICON_MAIL_SIGNED "mail-signed"	/**< */
#define EWL_ICON_MAIL_SIGNED_VERIFIED "mail-signed-verified"	/**< */
#define EWL_ICON_MEDIA_PLAYLIST_REPEAT "media-playlist-repeat"	/**< */
#define EWL_ICON_MEDIA_PLAYLIST_SHUFFLE "media-playlist-shuffle"	/**< */
#define EWL_ICON_NETWORK_ERROR "network-error"	/**< */
#define EWL_ICON_NETWORK_IDLE "network-idle"	/**< */

#define EWL_ICON_NETWORK_OFFLINE "network-offline"	/**< */
#define EWL_ICON_NETWORK_RECEIVE "network-receive"	/**< */
#define EWL_ICON_NETWORK_TRANSMIT "network-transmit"	/**< */
#define EWL_ICON_NETWORK_TRANSMIT_RECEIVE "network-transmit-receive"	/**< */
#define EWL_ICON_PRINTER_ERROR "printer-error"	/**< */
#define EWL_ICON_PRINTER_PRINTING "printer-printing"	/**< */
#define EWL_ICON_SOFTWARE_UPDATE_AVAILABLE "software-update-available"	/**< */
#define EWL_ICON_SOFTWARE_UPDATE_URGENT "software-update-urgent"	/**< */
#define EWL_ICON_SYNC_ERROR "sync-error"	/**< */
#define EWL_ICON_SYNC_SYNCHRONIZING "sync-synchronizing"	/**< */
#define EWL_ICON_TASK_DUE "task-due"	/**< */
#define EWL_ICON_TASK_PASSED_DUE "task-passed-due"	/**< */
#define EWL_ICON_USER_AWAY "user-away"	/**< */
#define EWL_ICON_USER_IDLE "user-idle"	/**< */
#define EWL_ICON_USER_OFFLINE "user-offline"	/**< */
#define EWL_ICON_USER_ONLINE "user-online"	/**< */
#define EWL_ICON_USER_TRASH_FULL "user-trash-full"	/**< */
#define EWL_ICON_WEATHER_CLEAR "weather-clear"	/**< */
#define EWL_ICON_WEATHER_CLEAR_NIGHT "weather-clear-night"	/**< */
#define EWL_ICON_WEATHER_FEW_CLOUDS "weather-few-clouds"	/**< */
#define EWL_ICON_WEATHER_FEW_CLOUDS_NIGHT "weather-few-clouds-night"	/**< */
#define EWL_ICON_WEATHER_FOG "weather-fog"	/**< */
#define EWL_ICON_WEATHER_OVERCAST "weather-overcast"	/**< */
#define EWL_ICON_WEATHER_SEVERE_ALERT "weather-severe-alert"	/**< */
#define EWL_ICON_WEATHER_SHOWERS "weather-showers"	/**< */
#define EWL_ICON_WEATHER_SHOWERS_SCATTERED "weather-showers-scattered"	/**< */
#define EWL_ICON_WEATHER_SNOW "weather-snow"	/**< */
#define EWL_ICON_WEATHER_STORM "weather-storm"	/**< */

/**
 * @}
 */

#endif

