package Etk::Stock;
use strict;
require Exporter;

our @ISA = qw/Exporter/;

our %EXPORT_TAGS = (
    size => [qw/SizeSmall SizeMedium SizeBig/],

    stock => [
    qw/  
    NoStock AddressBookNew AppointmentNew BookmarkNew ContactNew 
    DialogOk DialogCancel DialogYes DialogNo DialogClose DocumentNew 
    DocumentOpen DocumentPrint DocumentPrintPreview DocumentProperties 
    DocumentSaveAs DocumentSave EditClear EditCopy EditCut EditFind 
    EditPaste EditRedo EditUndo EditDelete EditFindReplace FolderNew 
    FormatIndentLess FormatIndentMore FormatJustifyCenter FormatJustifyFill 
    FormatJustifyLeft FormatJustifyRight FormatTextBold FormatTextItalic 
    FormatTextStrikethrough FormatTextUnderline GoBottom GoDown GoFirst 
    GoHome GoJump GoLast GoNext GoPrevious GoTop GoUp ListAdd ListRemove 
    MailMessageNew MailForward MailMarkJunk MailReplyAll MailReplySender 
    MailSendReceive MediaEject MediaPlaybackPause MediaPlaybackStart 
    MediaPlaybackStop MediaRecord MediaSeekBackward MediaSeekForward 
    MediaSkipBackward MediaSkipForward ProcessStop SystemLockScreen 
    SystemLogOut SystemSearch SystemShutdown TabNew ViewRefresh WindowNew 
    AccessoriesCalculator AccessoriesCharacterMap AccessoriesTextEditor 
    HelpBrowser InternetGroupChat InternetMail InternetNewsReader 
    InternetWebBrowser MultimediaVolumeControl OfficeCalendar 
    PreferencesDesktopAccessibility PreferencesDesktopAssistiveTechnology 
    PreferencesDesktopFont PreferencesDesktopKeyboardShortcuts 
    PreferencesDesktopLocale PreferencesDesktopRemoteDesktop 
    PreferencesDesktopSound PreferencesDesktopScreensaver 
    PreferencesDesktopTheme PreferencesDesktopWallpaper 
    PreferencesSystemNetworkProxy PreferencesSystemSession 
    PreferencesSystemWindows SystemFileManager SystemInstaller 
    SystemSoftwareUpdate SystemUsers UtilitiesSystemMonitor UtilitiesTerminal 
    ApplicationsAccessories ApplicationsDevelopment ApplicationsGames 
    ApplicationsGraphics ApplicationsInternet ApplicationsMultimedia 
    ApplicationsOffice ApplicationsOther ApplicationsSystem 
    PreferencesDesktopPeripherals PreferencesDesktop PreferencesSystem 
    AudioCard AudioInputMicrophone Battery CameraPhoto CameraVideo Computer 
    DriveCdrom DriveHarddisk DriveRemovableMedia InputGaming InputKeyboard 
    InputMouse MediaCdrom MediaFloppy MultimediaPlayer Network NetworkWireless 
    NetworkWired Printer PrinterRemote VideoDisplay EmblemFavorite EmblemImportant 
    EmblemPhotos EmblemReadonly EmblemSymbolicLink EmblemSystem EmblemUnreadable 
    FaceAngel FaceCrying FaceDevilGrin FaceGlasses FaceGrin FaceKiss FacePlain 
    FaceSad FaceSmileBig FaceSmile FaceSurprise FaceWink ApplicationCertificate 
    ApplicationXExecutable AudioXGeneric FontXGeneric ImageXGeneric PackageXGeneric 
    TextHtml TextXGeneric TextXGenericTemplate TextXScript VideoXGeneric 
    XDirectoryDesktop XDirectoryNormalDragAccept XDirectoryNormalHome 
    XDirectoryNormalOpen XDirectoryNormal XDirectoryNormalVisiting XDirectoryRemote 
    XDirectoryRemoteServer XDirectoryRemoteWorkgroup XDirectoryTrashFull 
    XDirectoryTrash XOfficeAddressBook XOfficeCalendar XOfficeDocument 
    XOfficePresentation XOfficeSpreadsheet PlacesFolder PlacesFolderRemote 
    PlacesFolderSavedSearch PlacesNetworkServer PlacesNetworkWorkgroup PlacesStartHere 
    PlacesUserDesktop PlacesUserHome PlacesUserTrash AudioVolumeHigh AudioVolumeLow 
    AudioVolumeMedium AudioVolumeMuted BatteryCaution DialogError DialogInformation 
    DialogWarning DialogQuestion FolderDragAccept FolderOpen FolderVisiting 
    ImageLoading ImageMissing MailAttachment NetworkError NetworkIdle NetworkOffline 
    NetworkOnline NetworkReceive NetworkTransmit NetworkTransmitReceive 
    NetworkWirelessEncrypted PrinterError UserTrashFull humStockIds/]
    );

$EXPORT_TAGS{all} = [@{$EXPORT_TAGS{size}} , @{$EXPORT_TAGS{stock}}];

our @EXPORT_OK = @{$EXPORT_TAGS{all}};

use constant
{
    SizeSmall => 0,
    SizeMedium => 1,
    SizeBig => 2,
      
    NoStock => 0,
    AddressBookNew => 1,
    AppointmentNew => 2,
    BookmarkNew => 3,
    ContactNew => 4,
    DialogOk => 5,
    DialogCancel => 6,
    DialogYes => 7,
    DialogNo => 8,
    DialogClose => 9,
    DocumentNew => 10,
    DocumentOpen => 11,
    DocumentPrint => 12,
    DocumentPrintPreview => 13,
    DocumentProperties => 14,
    DocumentSaveAs => 15,
    DocumentSave => 16,
    EditClear => 17,
    EditCopy => 18,
    EditCut => 19,
    EditFind => 20,
    EditPaste => 21,
    EditRedo => 22,
    EditUndo => 23,
    EditDelete => 24,
    EditFindReplace => 25,
    FolderNew => 26,
    FormatIndentLess => 27,
    FormatIndentMore => 28,
    FormatJustifyCenter => 29,
    FormatJustifyFill => 30,
    FormatJustifyLeft => 31,
    FormatJustifyRight => 32,
    FormatTextBold => 33,
    FormatTextItalic => 34,
    FormatTextStrikethrough => 35,
    FormatTextUnderline => 36,
    GoBottom => 37,
    GoDown => 38,
    GoFirst => 39,
    GoHome => 40,
    GoJump => 41,
    GoLast => 42,
    GoNext => 43,
    GoPrevious => 44,
    GoTop => 45,
    GoUp => 46,
    ListAdd => 47,
    ListRemove => 48,
    MailMessageNew => 49,
    MailForward => 50,
    MailMarkJunk => 51,
    MailReplyAll => 52,
    MailReplySender => 53,
    MailSendReceive => 54,
    MediaEject => 55,
    MediaPlaybackPause => 56,
    MediaPlaybackStart => 57,
    MediaPlaybackStop => 58,
    MediaRecord => 59,
    MediaSeekBackward => 60,
    MediaSeekForward => 61,
    MediaSkipBackward => 62,
    MediaSkipForward => 63,
    ProcessStop => 64,
    SystemLockScreen => 65,
    SystemLogOut => 66,
    SystemSearch => 67,
    SystemShutdown => 68,
    TabNew => 69,
    ViewRefresh => 70,
    WindowNew => 71,
    AccessoriesCalculator => 72,
    AccessoriesCharacterMap => 73,
    AccessoriesTextEditor => 74,
    HelpBrowser => 75,
    InternetGroupChat => 76,
    InternetMail => 77,
    InternetNewsReader => 78,
    InternetWebBrowser => 79,
    MultimediaVolumeControl => 80,
    OfficeCalendar => 81,
    PreferencesDesktopAccessibility => 82,
    PreferencesDesktopAssistiveTechnology => 83,
    PreferencesDesktopFont => 84,
    PreferencesDesktopKeyboardShortcuts => 85,
    PreferencesDesktopLocale => 86,
    PreferencesDesktopRemoteDesktop => 87,
    PreferencesDesktopSound => 88,
    PreferencesDesktopScreensaver => 89,
    PreferencesDesktopTheme => 90,
    PreferencesDesktopWallpaper => 91,
    PreferencesSystemNetworkProxy => 92,
    PreferencesSystemSession => 93,
    PreferencesSystemWindows => 94,
    SystemFileManager => 95,
    SystemInstaller => 96,
    SystemSoftwareUpdate => 97,
    SystemUsers => 98,
    UtilitiesSystemMonitor => 99,
    UtilitiesTerminal => 100,
    ApplicationsAccessories => 101,
    ApplicationsDevelopment => 102,
    ApplicationsGames => 103,
    ApplicationsGraphics => 104,
    ApplicationsInternet => 105,
    ApplicationsMultimedia => 106,
    ApplicationsOffice => 107,
    ApplicationsOther => 108,
    ApplicationsSystem => 109,
    PreferencesDesktopPeripherals => 110,
    PreferencesDesktop => 111,
    PreferencesSystem => 112,
    AudioCard => 113,
    AudioInputMicrophone => 114,
    Battery => 115,
    CameraPhoto => 116,
    CameraVideo => 117,
    Computer => 118,
    DriveCdrom => 119,
    DriveHarddisk => 120,
    DriveRemovableMedia => 121,
    InputGaming => 122,
    InputKeyboard => 123,
    InputMouse => 124,
    MediaCdrom => 125,
    MediaFloppy => 126,
    MultimediaPlayer => 127,
    Network => 128,
    NetworkWireless => 129,
    NetworkWired => 130,
    Printer => 131,
    PrinterRemote => 132,
    VideoDisplay => 133,
    EmblemFavorite => 134,
    EmblemImportant => 135,
    EmblemPhotos => 136,
    EmblemReadonly => 137,
    EmblemSymbolicLink => 138,
    EmblemSystem => 139,
    EmblemUnreadable => 140,
    FaceAngel => 141,
    FaceCrying => 142,
    FaceDevilGrin => 143,
    FaceGlasses => 144,
    FaceGrin => 145,
    FaceKiss => 146,
    FacePlain => 147,
    FaceSad => 148,
    FaceSmileBig => 149,
    FaceSmile => 150,
    FaceSurprise => 151,
    FaceWink => 152,
    ApplicationCertificate => 153,
    ApplicationXExecutable => 154,
    AudioXGeneric => 155,
    FontXGeneric => 156,
    ImageXGeneric => 157,
    PackageXGeneric => 158,
    TextHtml => 159,
    TextXGeneric => 160,
    TextXGenericTemplate => 161,
    TextXScript => 162,
    VideoXGeneric => 163,
    XDirectoryDesktop => 164,
    XDirectoryNormalDragAccept => 165,
    XDirectoryNormalHome => 166,
    XDirectoryNormalOpen => 167,
    XDirectoryNormal => 168,
    XDirectoryNormalVisiting => 169,
    XDirectoryRemote => 170,
    XDirectoryRemoteServer => 171,
    XDirectoryRemoteWorkgroup => 172,
    XDirectoryTrashFull => 173,
    XDirectoryTrash => 174,
    XOfficeAddressBook => 175,
    XOfficeCalendar => 176,
    XOfficeDocument => 177,
    XOfficePresentation => 178,
    XOfficeSpreadsheet => 179,
    PlacesFolder => 180,
    PlacesFolderRemote => 181,
    PlacesFolderSavedSearch => 182,
    PlacesNetworkServer => 183,
    PlacesNetworkWorkgroup => 184,
    PlacesStartHere => 185,
    PlacesUserDesktop => 186,
    PlacesUserHome => 187,
    PlacesUserTrash => 188,
    AudioVolumeHigh => 189,
    AudioVolumeLow => 190,
    AudioVolumeMedium => 191,
    AudioVolumeMuted => 192,
    BatteryCaution => 193,
    DialogError => 194,
    DialogInformation => 195,
    DialogWarning => 196,
    DialogQuestion => 197,
    FolderDragAccept => 198,
    FolderOpen => 199,
    FolderVisiting => 200,
    ImageLoading => 201,
    ImageMissing => 202,
    MailAttachment => 203,
    NetworkError => 204,
    NetworkIdle => 205,
    NetworkOffline => 206,
    NetworkOnline => 207,
    NetworkReceive => 208,
    NetworkTransmit => 209,
    NetworkTransmitReceive => 210,
    NetworkWirelessEncrypted => 211,
    PrinterError => 212,
    UserTrashFull => 213,
    humStockIds => 214,
};
    
1;
