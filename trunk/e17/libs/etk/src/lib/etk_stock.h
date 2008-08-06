/* ETK - The Enlightened ToolKit
 * Copyright (C) 2006-2008 Simon Treny, Hisham Mardam-Bey, Vincent Torri, Viktor Kojouharov
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. 
 * If not, see <http://www.gnu.org/licenses/>.
 */

/** @file etk_stock.h */
#ifndef _ETK_STOCK_H_
#define _ETK_STOCK_H_

#include "etk_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup Etk_Stock Etk_Stock
 * @{
 */

/**
 * @enum Etk_Stock_Size
 * @brief The size of a stock icon
 */
typedef enum
{
   ETK_STOCK_SMALL,     /* 16x16 */
   ETK_STOCK_MEDIUM,    /* 22x22 */
   ETK_STOCK_BIG        /* 48x48 */
} Etk_Stock_Size;

/**
 * @enum Etk_Stock_Id
 * @brief The Id of a stock icon
 */
typedef enum
{
   ETK_STOCK_NO_STOCK,
   ETK_STOCK_ADDRESS_BOOK_NEW,
   ETK_STOCK_APPOINTMENT_NEW,
   ETK_STOCK_BOOKMARK_NEW,
   ETK_STOCK_CONTACT_NEW,
   ETK_STOCK_DIALOG_APPLY,
   ETK_STOCK_DIALOG_OK,
   ETK_STOCK_DIALOG_CANCEL,
   ETK_STOCK_DIALOG_YES,
   ETK_STOCK_DIALOG_NO,
   ETK_STOCK_DIALOG_CLOSE,
   ETK_STOCK_DOCUMENT_NEW,
   ETK_STOCK_DOCUMENT_OPEN,
   ETK_STOCK_DOCUMENT_PRINT,
   ETK_STOCK_DOCUMENT_PRINT_PREVIEW,
   ETK_STOCK_DOCUMENT_PROPERTIES,
   ETK_STOCK_DOCUMENT_SAVE_AS,
   ETK_STOCK_DOCUMENT_SAVE,
   ETK_STOCK_EDIT_CLEAR,
   ETK_STOCK_EDIT_COPY,
   ETK_STOCK_EDIT_CUT,
   ETK_STOCK_EDIT_FIND,
   ETK_STOCK_EDIT_PASTE,
   ETK_STOCK_EDIT_REDO,
   ETK_STOCK_EDIT_UNDO,
   ETK_STOCK_EDIT_DELETE,
   ETK_STOCK_EDIT_FIND_REPLACE,
   ETK_STOCK_FOLDER_NEW,
   ETK_STOCK_FORMAT_INDENT_LESS,
   ETK_STOCK_FORMAT_INDENT_MORE,
   ETK_STOCK_FORMAT_JUSTIFY_CENTER,
   ETK_STOCK_FORMAT_JUSTIFY_FILL,
   ETK_STOCK_FORMAT_JUSTIFY_LEFT,
   ETK_STOCK_FORMAT_JUSTIFY_RIGHT,
   ETK_STOCK_FORMAT_TEXT_BOLD,
   ETK_STOCK_FORMAT_TEXT_ITALIC,
   ETK_STOCK_FORMAT_TEXT_STRIKETHROUGH,
   ETK_STOCK_FORMAT_TEXT_UNDERLINE,
   ETK_STOCK_GO_BOTTOM,
   ETK_STOCK_GO_DOWN,
   ETK_STOCK_GO_FIRST,
   ETK_STOCK_GO_HOME,
   ETK_STOCK_GO_JUMP,
   ETK_STOCK_GO_LAST,
   ETK_STOCK_GO_NEXT,
   ETK_STOCK_GO_PREVIOUS,
   ETK_STOCK_GO_TOP,
   ETK_STOCK_GO_UP,
   ETK_STOCK_LIST_ADD,
   ETK_STOCK_LIST_REMOVE,
   ETK_STOCK_MAIL_MESSAGE_NEW,
   ETK_STOCK_MAIL_FORWARD,
   ETK_STOCK_MAIL_MARK_JUNK,
   ETK_STOCK_MAIL_REPLY_ALL,
   ETK_STOCK_MAIL_REPLY_SENDER,
   ETK_STOCK_MAIL_SEND_RECEIVE,
   ETK_STOCK_MEDIA_EJECT,
   ETK_STOCK_MEDIA_PLAYBACK_PAUSE,
   ETK_STOCK_MEDIA_PLAYBACK_START,
   ETK_STOCK_MEDIA_PLAYBACK_STOP,
   ETK_STOCK_MEDIA_RECORD,
   ETK_STOCK_MEDIA_SEEK_BACKWARD,
   ETK_STOCK_MEDIA_SEEK_FORWARD,
   ETK_STOCK_MEDIA_SKIP_BACKWARD,
   ETK_STOCK_MEDIA_SKIP_FORWARD,
   ETK_STOCK_PROCESS_STOP,
   ETK_STOCK_SYSTEM_LOCK_SCREEN,
   ETK_STOCK_SYSTEM_LOG_OUT,
   ETK_STOCK_SYSTEM_SEARCH,
   ETK_STOCK_SYSTEM_SHUTDOWN,
   ETK_STOCK_TAB_NEW,
   ETK_STOCK_VIEW_REFRESH,
   ETK_STOCK_WINDOW_NEW,
   ETK_STOCK_ACCESSORIES_CALCULATOR,
   ETK_STOCK_ACCESSORIES_CHARACTER_MAP,
   ETK_STOCK_ACCESSORIES_TEXT_EDITOR,
   ETK_STOCK_HELP_BROWSER,
   ETK_STOCK_INTERNET_GROUP_CHAT,
   ETK_STOCK_INTERNET_MAIL,
   ETK_STOCK_INTERNET_NEWS_READER,
   ETK_STOCK_INTERNET_WEB_BROWSER,
   ETK_STOCK_MULTIMEDIA_VOLUME_CONTROL,
   ETK_STOCK_OFFICE_CALENDAR,
   ETK_STOCK_PREFERENCES_DESKTOP_ACCESSIBILITY,
   ETK_STOCK_PREFERENCES_DESKTOP_ASSISTIVE_TECHNOLOGY,
   ETK_STOCK_PREFERENCES_DESKTOP_FONT,
   ETK_STOCK_PREFERENCES_DESKTOP_KEYBOARD_SHORTCUTS,
   ETK_STOCK_PREFERENCES_DESKTOP_LOCALE,
   ETK_STOCK_PREFERENCES_DESKTOP_REMOTE_DESKTOP,
   ETK_STOCK_PREFERENCES_DESKTOP_SOUND,
   ETK_STOCK_PREFERENCES_DESKTOP_SCREENSAVER,
   ETK_STOCK_PREFERENCES_DESKTOP_THEME,
   ETK_STOCK_PREFERENCES_DESKTOP_WALLPAPER,
   ETK_STOCK_PREFERENCES_SYSTEM_NETWORK_PROXY,
   ETK_STOCK_PREFERENCES_SYSTEM_SESSION,
   ETK_STOCK_PREFERENCES_SYSTEM_WINDOWS,
   ETK_STOCK_SYSTEM_FILE_MANAGER,
   ETK_STOCK_SYSTEM_INSTALLER,
   ETK_STOCK_SYSTEM_SOFTWARE_UPDATE,
   ETK_STOCK_SYSTEM_USERS,
   ETK_STOCK_UTILITIES_SYSTEM_MONITOR,
   ETK_STOCK_UTILITIES_TERMINAL,
   ETK_STOCK_APPLICATIONS_ACCESSORIES,
   ETK_STOCK_APPLICATIONS_DEVELOPMENT,
   ETK_STOCK_APPLICATIONS_GAMES,
   ETK_STOCK_APPLICATIONS_GRAPHICS,
   ETK_STOCK_APPLICATIONS_INTERNET,
   ETK_STOCK_APPLICATIONS_MULTIMEDIA,
   ETK_STOCK_APPLICATIONS_OFFICE,
   ETK_STOCK_APPLICATIONS_OTHER,
   ETK_STOCK_APPLICATIONS_SYSTEM,
   ETK_STOCK_PREFERENCES_DESKTOP_PERIPHERALS,
   ETK_STOCK_PREFERENCES_DESKTOP,
   ETK_STOCK_PREFERENCES_SYSTEM,
   ETK_STOCK_AUDIO_CARD,
   ETK_STOCK_AUDIO_INPUT_MICROPHONE,
   ETK_STOCK_BATTERY,
   ETK_STOCK_CAMERA_PHOTO,
   ETK_STOCK_CAMERA_VIDEO,
   ETK_STOCK_COMPUTER,
   ETK_STOCK_DRIVE_CDROM,
   ETK_STOCK_DRIVE_HARDDISK,
   ETK_STOCK_DRIVE_REMOVABLE_MEDIA,
   ETK_STOCK_INPUT_GAMING,
   ETK_STOCK_INPUT_KEYBOARD,
   ETK_STOCK_INPUT_MOUSE,
   ETK_STOCK_MEDIA_CDROM,
   ETK_STOCK_MEDIA_FLOPPY,
   ETK_STOCK_MULTIMEDIA_PLAYER,
   ETK_STOCK_NETWORK,
   ETK_STOCK_NETWORK_WIRELESS,
   ETK_STOCK_NETWORK_WIRED,
   ETK_STOCK_PRINTER,
   ETK_STOCK_PRINTER_REMOTE,
   ETK_STOCK_VIDEO_DISPLAY,
   ETK_STOCK_EMBLEM_FAVORITE,
   ETK_STOCK_EMBLEM_IMPORTANT,
   ETK_STOCK_EMBLEM_PHOTOS,
   ETK_STOCK_EMBLEM_READONLY,
   ETK_STOCK_EMBLEM_SYMBOLIC_LINK,
   ETK_STOCK_EMBLEM_SYSTEM,
   ETK_STOCK_EMBLEM_UNREADABLE,
   ETK_STOCK_FACE_ANGEL,
   ETK_STOCK_FACE_CRYING,
   ETK_STOCK_FACE_DEVIL_GRIN,
   ETK_STOCK_FACE_GLASSES,
   ETK_STOCK_FACE_GRIN,
   ETK_STOCK_FACE_KISS,
   ETK_STOCK_FACE_PLAIN,
   ETK_STOCK_FACE_SAD,
   ETK_STOCK_FACE_SMILE_BIG,
   ETK_STOCK_FACE_SMILE,
   ETK_STOCK_FACE_SURPRISE,
   ETK_STOCK_FACE_WINK,
   ETK_STOCK_APPLICATION_CERTIFICATE,
   ETK_STOCK_APPLICATION_X_EXECUTABLE,
   ETK_STOCK_AUDIO_X_GENERIC,
   ETK_STOCK_FONT_X_GENERIC,
   ETK_STOCK_IMAGE_X_GENERIC,
   ETK_STOCK_PACKAGE_X_GENERIC,
   ETK_STOCK_TEXT_HTML,
   ETK_STOCK_TEXT_X_GENERIC,
   ETK_STOCK_TEXT_X_GENERIC_TEMPLATE,
   ETK_STOCK_TEXT_X_SCRIPT,
   ETK_STOCK_VIDEO_X_GENERIC,
   ETK_STOCK_X_DIRECTORY_DESKTOP,
   ETK_STOCK_X_DIRECTORY_NORMAL_DRAG_ACCEPT,
   ETK_STOCK_X_DIRECTORY_NORMAL_HOME,
   ETK_STOCK_X_DIRECTORY_NORMAL_OPEN,
   ETK_STOCK_X_DIRECTORY_NORMAL,
   ETK_STOCK_X_DIRECTORY_NORMAL_VISITING,
   ETK_STOCK_X_DIRECTORY_REMOTE,
   ETK_STOCK_X_DIRECTORY_REMOTE_SERVER,
   ETK_STOCK_X_DIRECTORY_REMOTE_WORKGROUP,
   ETK_STOCK_X_DIRECTORY_TRASH_FULL,
   ETK_STOCK_X_DIRECTORY_TRASH,
   ETK_STOCK_X_OFFICE_ADDRESS_BOOK,
   ETK_STOCK_X_OFFICE_CALENDAR,
   ETK_STOCK_X_OFFICE_DOCUMENT,
   ETK_STOCK_X_OFFICE_PRESENTATION,
   ETK_STOCK_X_OFFICE_SPREADSHEET,
   ETK_STOCK_PLACES_FOLDER,
   ETK_STOCK_PLACES_FOLDER_REMOTE,
   ETK_STOCK_PLACES_FOLDER_SAVED_SEARCH,
   ETK_STOCK_PLACES_NETWORK_SERVER,
   ETK_STOCK_PLACES_NETWORK_WORKGROUP,
   ETK_STOCK_PLACES_START_HERE,
   ETK_STOCK_PLACES_USER_DESKTOP,
   ETK_STOCK_PLACES_USER_HOME,
   ETK_STOCK_PLACES_USER_TRASH,
   ETK_STOCK_AUDIO_VOLUME_HIGH,
   ETK_STOCK_AUDIO_VOLUME_LOW,
   ETK_STOCK_AUDIO_VOLUME_MEDIUM,
   ETK_STOCK_AUDIO_VOLUME_MUTED,
   ETK_STOCK_BATTERY_CAUTION,
   ETK_STOCK_DIALOG_ERROR,
   ETK_STOCK_DIALOG_INFORMATION,
   ETK_STOCK_DIALOG_WARNING,
   ETK_STOCK_DIALOG_QUESTION,
   ETK_STOCK_FOLDER_DRAG_ACCEPT,
   ETK_STOCK_FOLDER_OPEN,
   ETK_STOCK_FOLDER_VISITING,
   ETK_STOCK_IMAGE_LOADING,
   ETK_STOCK_IMAGE_MISSING,
   ETK_STOCK_MAIL_ATTACHMENT,
   ETK_STOCK_NETWORK_ERROR,
   ETK_STOCK_NETWORK_IDLE,
   ETK_STOCK_NETWORK_OFFLINE,
   ETK_STOCK_NETWORK_ONLINE,
   ETK_STOCK_NETWORK_RECEIVE,
   ETK_STOCK_NETWORK_TRANSMIT,
   ETK_STOCK_NETWORK_TRANSMIT_RECEIVE,
   ETK_STOCK_NETWORK_WIRELESS_ENCRYPTED,
   ETK_STOCK_PRINTER_ERROR,
   ETK_STOCK_USER_TRASH_FULL,
   ETK_NUM_STOCK_IDS
} Etk_Stock_Id;

const char *etk_stock_key_get(Etk_Stock_Id stock_id, Etk_Stock_Size size);
const char *etk_stock_label_get(Etk_Stock_Id stock_id);

/** @} */

#ifdef __cplusplus
}
#endif

#endif
