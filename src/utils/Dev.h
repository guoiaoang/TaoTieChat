#pragma once

#include "Headers.h"
#include "data.h"
#include "Utils.h"
#include "RegistryFactory.h"

#ifdef DEBUG
class MediaWindow;
class ChatWindow;

namespace Dev {

// ========================================================================== //

class QIconDev : public QObject {
    Q_OBJECT
public:
    enum class ThemeIcon {
        AddressBookNew,
        ApplicationExit,
        AppointmentNew,
        CallStart,
        CallStop,
        ContactNew,
        DocumentNew,
        DocumentOpen,
        DocumentOpenRecent,
        DocumentPageSetup,
        DocumentPrint,
        DocumentPrintPreview,
        DocumentProperties,
        DocumentRevert,
        DocumentSave,
        DocumentSaveAs,
        DocumentSend,
        EditClear,
        EditCopy,
        EditCut,
        EditDelete,
        EditFind,
        EditPaste,
        EditRedo,
        EditSelectAll,
        EditUndo,
        FolderNew,
        FormatIndentLess,
        FormatIndentMore,
        FormatJustifyCenter,
        FormatJustifyFill,
        FormatJustifyLeft,
        FormatJustifyRight,
        FormatTextDirectionLtr,
        FormatTextDirectionRtl,
        FormatTextBold,
        FormatTextItalic,
        FormatTextUnderline,
        FormatTextStrikethrough,
        GoDown,
        GoHome,
        GoNext,
        GoPrevious,
        GoUp,
        HelpAbout,
        HelpFaq,
        InsertImage,
        InsertLink,
        InsertText,
        ListAdd,
        ListRemove,
        MailForward,
        MailMarkImportant,
        MailMarkRead,
        MailMarkUnread,
        MailMessageNew,
        MailReplyAll,
        MailReplySender,
        MailSend,
        MediaEject,
        MediaPlaybackPause,
        MediaPlaybackStart,
        MediaPlaybackStop,
        MediaRecord,
        MediaSeekBackward,
        MediaSeekForward,
        MediaSkipBackward,
        MediaSkipForward,
        ObjectRotateLeft,
        ObjectRotateRight,
        ProcessStop,
        SystemLockScreen,
        SystemLogOut,
        SystemSearch,
        SystemReboot,
        SystemShutdown,
        ToolsCheckSpelling,
        ViewFullscreen,
        ViewRefresh,
        ViewRestore,
        WindowClose,
        WindowNew,
        ZoomFitBest,
        ZoomIn,
        ZoomOut,

        AudioCard,
        AudioInputMicrophone,
        Battery,
        CameraPhoto,
        CameraVideo,
        CameraWeb,
        Computer,
        DriveHarddisk,
        DriveOptical,
        InputGaming,
        InputKeyboard,
        InputMouse,
        InputTablet,
        MediaFlash,
        MediaOptical,
        MediaTape,
        MultimediaPlayer,
        NetworkWired,
        NetworkWireless,
        Phone,
        Printer,
        Scanner,
        VideoDisplay,

        AppointmentMissed,
        AppointmentSoon,
        AudioVolumeHigh,
        AudioVolumeLow,
        AudioVolumeMedium,
        AudioVolumeMuted,
        BatteryCaution,
        BatteryLow,
        DialogError,
        DialogInformation,
        DialogPassword,
        DialogQuestion,
        DialogWarning,
        FolderDragAccept,
        FolderOpen,
        FolderVisiting,
        ImageLoading,
        ImageMissing,
        MailAttachment,
        MailUnread,
        MailRead,
        MailReplied,
        MediaPlaylistRepeat,
        MediaPlaylistShuffle,
        NetworkOffline,
        PrinterPrinting,
        SecurityHigh,
        SecurityLow,
        SoftwareUpdateAvailable,
        SoftwareUpdateUrgent,
        SyncError,
        SyncSynchronizing,
        UserAvailable,
        UserOffline,
        WeatherClear,
        WeatherClearNight,
        WeatherFewClouds,
        WeatherFewCloudsNight,
        WeatherFog,
        WeatherShowers,
        WeatherSnow,
        WeatherStorm,

        NThemeIcons  // 最后这个不能使用，会导致程序静默退出
    };
    Q_ENUM(ThemeIcon)
};

// ========================================================================== //

class LocalUserDataTest : public LocalUserData{
public:
    LocalUserDataTest()
        : m_contacts(ContactsManeger::create()) {
        // 添加测试数据

        // add_contact
        {
            // 自定义数据
            {
                ContactManager cm(ContactManager::create());
                cm.set_account_id ("Contact ID 01"      );
                // cm.set_avatar_pixmap(QPixmap());
                cm.set_avatar_path("Contact Avatar 01"  );
                cm.set_nick_name  ("Contact NickName 01");
                cm.set_notes      ("Contact Notes 01"   );
                cm.set_contact    (cm.contact()         );
                add_contact(cm.contact());

                add_contact(ContactManager("Contact ID 02", QPixmap(), "Contact Avatar 02", "Contact NickName 02", "Contact Notes 02").contact());
                add_contact(ContactManager("Contact ID 03", QPixmap(), "Contact Avatar 03", "Contact NickName 03", "Contact Notes 03").contact());
                add_contact(
                    ContactManager(
                        "Contact ID 04",
                        QPixmap(":/qt-project.org/qmessagebox/images/qtlogo-64.png"),
                        ":/qt-project.org/qmessagebox/images/qtlogo-64.png",
                        "Contact NickName 04",
                        "Contact Notes 04"
                        ).contact()
                    );
            }
            // 资源
            // ":/"
            {
                for(QDirListing::DirEntry file : QDirListing(":/", QDirListing::IteratorFlag::Recursive | QDirListing::IteratorFlag::FilesOnly)) {
                    QString path = file.absoluteFilePath();
                    // if(!pixmap.isNull()) {
                    QString count = QString::number(get_contacts_count());
                    add_contact(ContactManager("Contact :/ ID " + count, QPixmap(path), path, "Contact :/ NickName " + count, path).contact());
                }
            }
            // enum QStyle::StandardPixmap
            {
                QMetaEnum metaEnum = QMetaEnum::fromType<QStyle::StandardPixmap>();
                for(int index = 0; index < metaEnum.keyCount(); ++index) {
                    QPixmap pixmap = QApplication::style()->standardPixmap(QStyle::StandardPixmap(index));
                    QString path   = QString("QStyle::StandardPixmap::%1").arg(metaEnum.valueToKey(index));
                    QString count  = QString::number(get_contacts_count());
                    add_contact(ContactManager("Contact standardPixmap ID " + count, pixmap, path, "Contact standardPixmap NickName " + count, path).contact());
                }
            }
            // [since 6.7] enum class QIcon::ThemeIcon
            {
                // 最后一个元素不能使用，会造成程序静默退出
                QMetaEnum metaEnum = QMetaEnum::fromType<QIconDev::ThemeIcon>();
                // int count_len = QString::number(metaEnum.keyCount()).length();
                for(int index = 0; index < metaEnum.keyCount() - 1; ++index) {
                    // QPixmap::fromImage(QImage(64, 64, QImage::Format_ARGB32_Premultiplied));
                    QIcon icon = QIcon::fromTheme(QIcon::ThemeIcon(index));
                    QPixmap pixmap = icon.pixmap(256);
                    QString path   = QString("QIcon::ThemeIcon::%1").arg(metaEnum.valueToKey(index));
                    QString count  = QString::number(get_contacts_count());
                    add_contact(ContactManager("Contact ThemeIcon ID " + count, pixmap, path, "Contact ThemeIcon NickName " + count, path).contact());
                }
            }
        }
    }
    virtual ~LocalUserDataTest(){}
public:
    // 添加好友  // 要求 id 不为空且不存在
    virtual bool add_contact(Contact contact) override {
        if(
            contact
            && !ContactManager(contact).get_account_id().isEmpty()  // ID 不为空
            && !LocalUserDataTest::get_contact(ContactManager(contact).get_account_id())  // ID 未添加
            ) {
            LocalUserDataTest::get_contacts()->append(contact);
            return true;
        }
        return false;
    }
    // 获取好友列表
    virtual Contact get_contact(const QString&contact_id) override {
        for(auto contact : *LocalUserDataTest::get_contacts()) {
            if(ContactManager(contact).get_account_id() == contact_id) {
                return contact;
            }
        }
        return Contact();
    }
    virtual Contacts get_contacts() override { return m_contacts; }
    int get_contacts_count(){ return LocalUserDataTest::get_contacts()->size(); }
private:
    Contacts m_contacts = nullptr;
};
REG_FAC(LocalUserData, LocalUserDataTest, test_local)
// static QSharedPointer<LocalUserData> g_local_user_data = Registr/yFactory<LocalUserData>::create("test_local");;

// ========================================================================== //

}
#endif // DEBUG
