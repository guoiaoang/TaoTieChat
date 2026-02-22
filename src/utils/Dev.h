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

// ========================================================================== //

#include "RegistryFactory.h"
#include <QStyle>
#include <QApplication>

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
                QMetaEnum metaEnum = QMetaEnum::fromType<Rebuild::QIcon::ThemeIcon>();
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

#include "data.h"
#include <queue>

namespace SereverBody{
    struct AccountInfoData {
        QString account_id;  // 账号 ID，唯一标识符，仅用于内部账号标识
        QString user_id;     // 用户 ID，唯一标识符，用于搜索及显示
        QString username;    // 用户名，非唯一标识符，也可用于搜索及展示
        QPixmap avatar;      // 头像
        QString password;    // 密码
        QString question;    // 密保问题
        QString answer;      // 密保答案
        QString gender;      // 性别
        QString birthday;    // 生日
        QString email;       // 邮箱
        QString phone;       // 电话
        QString sign;        // 个性签名
        QStringList region;           // 地区
        QDateTime   register_time;    // 注册时间
        QDateTime   last_active_time; // 上次活跃时间
    };
    using AccountInfo = QSharedPointer<AccountInfoData>;
    struct AccountsInfoData {
        QList<AccountInfo> accounts;  // 账号信息列表
    };
    using AccountsInfo = QSharedPointer<AccountsInfoData>;

// ========================================================================== //

    class AccountInfoManager{
    public:
        static AccountInfo create() {
            return QSharedPointer<AccountInfoData>(new AccountInfoData);
        }
    };
    class AccountsInfoManager{
    public:
        static AccountsInfo create() {
            return QSharedPointer<AccountsInfoData>(new AccountsInfoData);
        }
    };
}

class NetworkUserDataTest : public NetworkUserData {
public:
    NetworkUserDataTest()
        : m_accounts_info(SereverBody::AccountsInfoManager::create()) {
    }

    virtual ~NetworkUserDataTest() override {}
    virtual bool init   () override { return true; }
    virtual bool destroy() override { return true; }
public:
    // ============================== 用户管理 ============================== //
    // 注册
    virtual bool request_user_signup (const NetworkBody::UserSignup& request) override {
        // 注册新用户
        // 服务端暂不阻止相同信息的重复注册

        // 读取 uuid 列表并缓存
        SereverBody::AccountsInfo accounts_info = get_accounts_info();
        QSharedPointer<QSet<QString>> cached_uuids = QSharedPointer<QSet<QString>>(new QSet<QString>);
        for(const auto& account_info : accounts_info->accounts) {
            cached_uuids->insert(account_info->account_id);
            cached_uuids->insert(account_info->user_id);
        }

        // 生成账号 id 和用户 id 并校验是否重复
        QString account_id = Utils::get_new_uuid_string();
        QString user_id    = Utils::get_new_uuid_string().left(16); // 用户 ID 取前 16 位即可
        while(cached_uuids->contains(account_id)) {
            account_id = Utils::get_new_uuid_string();
        }
        while(cached_uuids->contains(user_id)) {
            user_id = Utils::get_new_uuid_string();
        }

        // 生成保存的信息
        SereverBody::AccountInfo account_info = SereverBody::AccountInfoManager::create();
        account_info->account_id = account_id;
        account_info->user_id    = user_id;
        account_info->username   = request->username;
        account_info->password   = request->password;
        account_info->question   = request->question;
        account_info->answer     = request->answer;

        // 保存账号
        accounts_info->accounts.append(account_info);

        // 需要返回账号 id 和用户 id，其他信息不返回，以免泄露隐私
        NetworkBody::UserSignup responce = NetworkBody::UserSignup::create();
        responce->account_id = account_info->account_id;
        responce->user_id    = account_info->user_id;
        // responce_user_signup(responce);

        // 异步返回请求结果  // 需要放到线程任务队列中
        std::function<bool(const NetworkBody::UserSignup&)> callback = std::bind(&NetworkUserDataTest::responce_user_signup, this, std::placeholders::_1);
        std::queue<std::function<void(void)>> queue;
        queue.push([callback, responce]() {
            callback(responce);
        });

        return true;
    }
    virtual bool responce_user_signup(const NetworkBody::UserSignup& request) override { return false; }
    // 登陆
    virtual bool request_user_signin (const NetworkBody::UserSignin& request) override { return false; }
    virtual bool responce_user_signin(const NetworkBody::UserSignin& request) override { return false; }
    // 退出
    virtual bool request_user_logout (const NetworkBody::UserLogout& request) override { return false; }
    virtual bool responce_user_logout(const NetworkBody::UserLogout& request) override { return false; }
private:
    SereverBody::AccountsInfo get_accounts_info() { return m_accounts_info; }

private:
    SereverBody::AccountsInfo m_accounts_info = nullptr;  // 账号信息列表
};
REG_FAC(NetworkUserData, NetworkUserDataTest, test_network)
// static QSharedPointer<NetworkUserData> g_network_user_data = RegistryFactory<NetworkUserData>::create("test_network");;
