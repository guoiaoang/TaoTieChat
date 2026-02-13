#pragma once

#include "Utils.h"

// ========================================================================== //

namespace Rebuild{
class QIcon : public QObject {
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
}

// ========================================================================== //

// 开发测试代码
class MediaWindow;
class ChatWindow;
namespace Dev{

class MediaWindowDev : public QMainWindow {
    Q_OBJECT
public:
    explicit MediaWindowDev(MediaWindow*media_window, QWidget *parent = nullptr);
    ~MediaWindowDev();
};

class ChatWindowDev : public QDialog {
    Q_OBJECT
public:
    explicit ChatWindowDev(ChatWindow*chat_window, QWidget *parent = nullptr);
    ~ChatWindowDev();
};

}

// ========================================================================== //

#include "data.h"

// 测试数据
namespace Test{
class TestData{
public:
    static TestData *instance() {
        static TestData s_test_data;
        return &s_test_data;
    }
private:
    TestData() {}
    ~TestData() {}
    TestData(TestData&) = delete;
    TestData *operator=(const TestData&) = delete;
public:
    // 用户列表
    Contacts &contacts() { return m_contacts; }
    // 添加用户
    void add_contact(Contact contact) {
        contacts()->append(contact);
    }
    void update_pixmap(){
        if(0)
        {
            // QImage
            if(0)
            {
                QImage image;
            }
            // QPicture
            if(0)
            {
                QPicture picture;
                // picture.load(QPicture()); // IODriver
                // picture.save(QImage()); // QIODevice
                QPainter painter;
                painter.begin(&picture);           // paint in picture
                painter.drawEllipse(10,20, 80,70); // draw an ellipse
                painter.end();                     // painting done
                picture.save("drawing.pic");       // save picture
                // painter.drawPicture();
                // painter.drawImage();
                // painter.drawPixmap();
            }
            if(0)
            {
                QPixmap pixmap;
                QByteArray bytes;
                QBuffer buffer(&bytes);
                buffer.open(QIODevice::WriteOnly);
                pixmap.save(&buffer, "PNG"); // writes pixmap into bytes in PNG format
            }
        }
    }
private:
    Contacts m_contacts;  // 可搜索用户
};

static void test_data_add_users() {
    TestData::instance()->add_contact(ContactManager("Test Contact ID 01", QPixmap(), "Test Contact Avatar 01", "Test Contact NickName 01", "Test Notes 01").contact());
    TestData::instance()->add_contact(ContactManager("Test Contact ID 02", QPixmap(), "Test Contact Avatar 02", "Test Contact NickName 02", "Test Notes 02").contact());
    TestData::instance()->add_contact(ContactManager("Test Contact ID 03", QPixmap(), "Test Contact Avatar 03", "Test Contact NickName 03", "Test Notes 03").contact());
    TestData::instance()->add_contact(
        ContactManager(
            "Test Contact ID 04",
            QPixmap(":/qt-project.org/qmessagebox/images/qtlogo-64.png"),
            ":/qt-project.org/qmessagebox/images/qtlogo-64.png",
            "Test Contact NickName 04", "Test Notes 04"
            ).contact()
        );

    {
        // ":/"
        {
            for(QDirListing::DirEntry file : QDirListing(":/", QDirListing::IteratorFlag::Recursive | QDirListing::IteratorFlag::FilesOnly)) {
                QString path = file.absoluteFilePath();
                // if(!QPixmap(path).isNull()) {
                QString count = QString::number(TestData::instance()->contacts()->size());
                TestData::instance()->add_contact(ContactManager("Test Contact ID " + count, QPixmap(path), path, "Test Contact NickName " + count, "Test Notes " + count).contact());
                // }
            }
        }
    }
}
}
