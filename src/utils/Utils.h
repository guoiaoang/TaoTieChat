#pragma once

#include "headers.h"
#include "taotie_chat.qpb.h"
#include "taotie_chat_client.grpc.qpb.h"

// using ::TaoTieChat::ResultCodeGadget::ResultCode;

// 文件及路径操作
namespace Utils {
QString app_path();
QString app_dir_path();
QString app_data_path();
QString app_config_path();
QString users_data_path();
QString user_home_path    (QString user_id = QString());
QString user_storage_path (QString user_id = QString());
QString user_download_path(QString user_id = QString());
QString user_cache_path   (QString user_id = QString());
QString get_dir(const QString& path);
bool create_dirs(QString full_path);
bool remove_dirs(QString full_path);
bool rename(QString src, QString dst);
} // namespace Utils

namespace Utils {
void init_debug_output_format();

bool load_translator(QApplication&application);

// 遍历指定路径获取所有文件和文件夹列表
// path: 目标路径; recursive: 递归子目录
QList<QFileInfo> dir_contents(const QString &path, bool recursive = false);

// 获取给定大小内指定字体最适合的最大大小
// 字体名称，矩形大小，绘制文本，精度
QFont get_suitable_font_size(const QString&font_name, const QSizeF&size, const QString&text, const qreal precision = 1);

// 字体，矩形大小，绘制文本，精度
QFont get_suitable_font_size(QFont&font, const QSizeF&size, const QString&text, const qreal precision = 1);

// 注册指定路径的字体文件
bool load_font(const QString&font_path);

// 用于按钮
QIcon to_icon(const QString&text, const QPen&pen = QPen(Qt::white), const QSize size = QSize(128, 128));
// 用于头像
QPixmap to_pixmap(const QString&text, const QSize size = QSize(128, 128));

// ====================================================================== //
// 内存中展开布局器 In-Memory Layout Expander  // 窗口不需要显示即可应用实际布局
// 1. 不要使用 QSplitter，会导致自动调整大小
// 2. 不要调用 widget->adjustSize() 自适应大小，会按照控件所需实际大小进行缩小
// 3. ui 中修改后要确保编译也已经更新，否则界面也看不到变化
void in_memory_layout_expander(const QWidget*widget);
// 窗口前置显示
void widget_raise(QWidget*widget);
// 添加圆角遮罩
QPixmap to_round_pixmap(const QPixmap& src, int radius = 10);
// 添加图形遮罩
QPixmap apply_mask(const QPixmap& source, const QPixmap& mask);

// ====================================================================== //
// 获取随机字符串作为全局唯一的 ID
// QUuid::WithBraces 形如：{00000000-0000-0000-0000-000000000000}  // 8-4-4-4-12=32 位
// QUuid::Id128 : 不使用间隔符
QString get_new_uuid_string();

// ====================================================================== //
// 获取错误信息
QString to_string(::TaoTieChat::SuccessCodeGadget::SuccessCode code);
QString to_string(::TaoTieChat::ErrorCodeGadget::ErrorCode     code);
void set(TaoTieChat::Result&result, const ::TaoTieChat::SuccessCodeGadget::SuccessCode&success_code);
void set(TaoTieChat::Result&result, const ::TaoTieChat::ErrorCodeGadget   ::ErrorCode &error_code  );

// ====================================================================== //
// 查找指定 id 的好友
std::shared_ptr<ContactInfo> get_contact_info(const std::shared_ptr<ContactInfos>&contact_infos, const QString& contact_account_id);
// 从好友列表中获取第 index 个好友
std::shared_ptr<ContactInfo> get_contact_info(const std::shared_ptr<ContactInfos>&contact_infos, int index);

} // namespace Utils

// 将 protobuf 生成的 ContactInfo 转换为更易用的结构体，方便在 UI 层使用
namespace Utils::Wrapper {
    using namespace ::TaoTieChat;

struct FileMetadataWrapper {
    QString file_name = QString{ };
    qint64  file_size = -1        ;
    QString file_md5  = QString{ };
    FileMetadataWrapper parse(const FileMetadata&other) {
        file_name = other.fileName();
        file_size = other.fileSize();
        file_md5  = other.fileMd5 ();
        return *this;
    }
    FileMetadata operator()() const {
        FileMetadata file_metadata;
        file_metadata.setFileName(file_name);
        file_metadata.setFileSize(file_size);
        file_metadata.setFileMd5 (file_md5 );
        return file_metadata;
    }
    bool operator!=(const FileMetadataWrapper& other) const {
        return  file_name != other.file_name ||
                file_size != other.file_size ||
                file_md5  != other.file_md5  ;
    }
};

// Result 结果消息
struct ResultWrapper {
    SuccessCode success_code = SuccessCode::SUCCESS_UNSET;
    ErrorCode   error_code   = ErrorCode  ::ERROR_UNSET  ;
    QString     message      = QString{ };
    Result operator()() const {
        Result result;
        // code
        ;    if(success_code != SuccessCode::SUCCESS_UNSET) { result.setSuccessCode(success_code); }
        else if(error_code   != ErrorCode  ::ERROR_UNSET  ) { result.setErrorCode  (error_code  ); }
        // msg
        if(!message.isEmpty()) { result.setMessage(message); }
        else {
            ;    if(success_code != SuccessCode::SUCCESS_UNSET) { result.setMessage(Utils::to_string(success_code)); }
            else if(error_code   != ErrorCode  ::ERROR_UNSET  ) { result.setMessage(Utils::to_string(error_code)); }
        }
        return result;
    }
};

// 自己相关的信息。
// 目前仅用于标志及存储，不用于信息的修改，仅作为账号登陆记录
struct ContactInfoSelfWrapper {
    bool    modify_self_info  = false    ;  // 是否是修改个人信息
    QString user_id           = QString{};  // 用户 ID
    QString password          = QString{};  // 密码
    QString token             = QString{};  // 认证令牌
    QString security_question = QString{};  // 安全问题
    QString security_answer   = QString{};  // 安全答案
    ContactInfoSelfWrapper parse(const ContactInfoSelf&other) {
        modify_self_info  = other.modifySelfInfo  ();
        user_id           = other.userId          ();
        password          = other.password        ();
        token             = other.token           ();
        security_question = other.securityQuestion();
        security_answer   = other.securityAnswer  ();
        return *this;
    }
    ContactInfoSelf operator()() const {
        ContactInfoSelf self_info;
        self_info.setModifySelfInfo  (modify_self_info );
        self_info.setUserId          (user_id          );
        self_info.setPassword        (password         );
        self_info.setToken           (token            );
        self_info.setSecurityQuestion(security_question);
        self_info.setSecurityAnswer  (security_answer  );
        return self_info;
    }
    bool operator!=(const ContactInfoSelfWrapper& other) const {
        return  modify_self_info  != other.modify_self_info  ||
                user_id           != other.user_id           ||
                password          != other.password          ||
                token             != other.token             ||
                security_question != other.security_question ||
                security_answer   != other.security_answer   ;
    }
};

// 好友相关的信息
struct ContactInfoFriendWrapper {
    QString     account_id   = QString    { };  // 账号 ID  // 获取好友信息时由服务器填充该字段
    QString     user_id      = QString    { };  // 用户 ID  // 修改好友信息时使用该字段查找好友账号 ID
    int64_t     status       = int64_t    {0};  // 是否被拉黑
    bool        starred      = false         ;  // 是否标星
    bool        silent       = false         ;  // 是否免打扰
    QString     notes        = QString    { };  // 备注
    QString     added_source = QString    { };  // 添加来源
    QString     added_time   = QString    { };  // 添加时间
    QStringList tags         = QStringList{ };  // 标签，逗号分隔
    ContactInfoFriendWrapper parse(const ContactInfoFriend&other) {
        account_id   = other.accountId   ();
        user_id      = other.userId      ();
        status       = other.status      ();
        starred      = other.starred     ();
        silent       = other.silent      ();
        notes        = other.notes       ();
        added_source = other.addedSource ();
        added_time   = other.addedTime   ();
        tags         = other.tags        ();
        return *this;
    }
    ContactInfoFriend operator()() const {
        ContactInfoFriend friend_info;
        friend_info.setAccountId   (account_id  );
        friend_info.setUserId      (user_id     );
        friend_info.setStatus      (status      );
        friend_info.setStarred     (starred     );
        friend_info.setSilent      (silent      );
        friend_info.setNotes       (notes       );
        friend_info.setAddedSource (added_source);
        friend_info.setAddedTime   (added_time  );
        friend_info.setTags        (tags        );
        return friend_info;
    }
    bool operator!=(const ContactInfoFriendWrapper& other) const {
        return  account_id   != other.account_id   ||
                user_id      != other.user_id      ||
                status       != other.status       ||
                starred      != other.starred      ||
                silent       != other.silent       ||
                notes        != other.notes        ||
                added_source != other.added_source ||
                added_time   != other.added_time   ||
                tags         != other.tags         ;
    }
};

// 用户的基本信息  // 公共信息
struct ContactInfoBaseWrapper {
    QString     account_id   = QString    {};  // 账号 ID
    QString     user_id      = QString    {};  // 用户 ID  // 修改个人用户 ID 时使用该字段作为目标用户 ID，修改好友 ID 时不使用该字段（不作为校验及目标 ID）
    QString     avatar_id    = QString    {};  // 头像
    QString     nickname     = QString    {};  // 昵称
    int64_t     gender       = 0            ;  // 性别  // 0: 未知 1: 男 2: 女
    QString     birthday     = QString    {};  // 生日
    QString     signature    = QString    {};  // 签名
    QString     region       = QString    {};  // 地区
    QString     description  = QString    {};  // 个人简介
    QStringList phones       = QStringList{};  // 手机号
    QStringList emails       = QStringList{};  // 邮箱
    QStringList social_links = QStringList{};  // 社交链接，逗号分隔
    QStringList moments_id   = QStringList{};  // 朋友圈 ID 列表
    QStringList groups_id    = QStringList{};  // 群组 ID 列表
    ContactInfoBaseWrapper parse(const ContactInfoBase&other) {
        account_id   = other.accountId  ();
        user_id      = other.userId     ();
        avatar_id    = other.avatarId   ();
        nickname     = other.nickname   ();
        gender       = other.gender     ();
        birthday     = other.birthday   ();
        signature    = other.signature  ();
        region       = other.region     ();
        description  = other.description();
        phones       = other.phones     ();
        emails       = other.emails     ();
        social_links = other.socialLinks();
        moments_id   = other.momentsId  ();
        groups_id    = other.groupsId   ();
        return *this;
    }
    ContactInfoBase operator()() const {
        ContactInfoBase base_info;
        base_info.setAccountId   (account_id  );
        base_info.setUserId      (user_id     );
        base_info.setAvatarId    (avatar_id   );
        base_info.setNickname    (nickname    );
        base_info.setGender      (gender      );
        base_info.setBirthday    (birthday    );
        base_info.setSignature   (signature   );
        base_info.setRegion      (region      );
        base_info.setDescription (description );
        base_info.setPhones      (phones      );
        base_info.setEmails      (emails      );
        base_info.setSocialLinks (social_links);
        base_info.setMomentsId   (moments_id  );
        base_info.setGroupsId    (groups_id   );
        return base_info;
    }
    bool operator!=(const ContactInfoBaseWrapper& other) const {
        return  account_id   != other.account_id   ||
                user_id      != other.user_id      ||
                avatar_id    != other.avatar_id    ||
                nickname     != other.nickname     ||
                gender       != other.gender       ||
                birthday     != other.birthday     ||
                signature    != other.signature    ||
                region       != other.region       ||
                description  != other.description  ||
                phones       != other.phones       ||
                emails       != other.emails       ||
                social_links != other.social_links ||
                moments_id   != other.moments_id   ||
                groups_id    != other.groups_id    ;
    }
};

// 用户信息
struct ContactInfoWrapper{
    ContactInfoBaseWrapper   base_info   = ContactInfoBaseWrapper  {};  // 基本信息
    ContactInfoSelfWrapper   self_info   = ContactInfoSelfWrapper  {};  // 自己相关的信息
    ContactInfoFriendWrapper friend_info = ContactInfoFriendWrapper{};  // 好友相关的信息
    ContactInfoWrapper parse(const ContactInfo&other) {
        base_info  .parse(other.baseInfo());
        if(other.hasSelfInfo  ()) { self_info  .parse(other.selfInfo  ()); }
        if(other.hasFriendInfo()) { friend_info.parse(other.friendInfo()); }
        return *this;
    }
    ContactInfo operator()() const {
        ContactInfo contact_info;
        contact_info.setBaseInfo(base_info());
        if(self_info   != ContactInfoSelfWrapper  {}) { contact_info.setSelfInfo  (self_info  ()); }
        if(friend_info != ContactInfoFriendWrapper{}) { contact_info.setFriendInfo(friend_info()); }
        return contact_info;
    }
    bool operator!=(const ContactInfoWrapper& other) const {
        return  base_info   != other.base_info   ||
                self_info   != other.self_info   ||
                friend_info != other.friend_info   ;
    }
};

} // namespace Utils::Wrapper

// FontIcons
namespace Utils {
// 必须在加载字体后创建该结构体的实例，以确保图标能够正确显示
struct FontIcons {
    struct FontIconsSize {
        QSize avatar;
        QSize button;
        FontIconsSize& reset() {
            avatar = QSize();
            button = QSize();
            return *this;
        }
        FontIconsSize& update() {
            if(this->avatar == QSize()) { this->avatar = QSize(48, 48); }
            if(this->button == QSize()) { this->button = QSize(36, 36); }
            return *this;
        }
    } size;
    struct FontIconsColor {
        QColor normal  ;
        QColor disabled;
        FontIconsColor& reset() {
            normal   = QColor();
            disabled = QColor();
            return *this;
        }
        FontIconsColor& update() {
            if(this->normal   == QColor()) { this->normal   = QApplication::palette().color(QPalette::Normal  , QPalette::WindowText); }
            if(this->disabled == QColor()) { this->disabled = QApplication::palette().color(QPalette::Disabled, QPalette::WindowText); }
            return *this;
        }
    } color;
    struct FontIconsIcons {
        QIcon  profile           ;
        QIcon  chat_room_normal  ;
        QIcon  chat_room_disabled;
        QIcon  contacts_normal   ;
        QIcon  contacts_disabled ;
        QIcon  settings          ;
        QIcon  chat_room         ;
        QIcon  contacts          ;
        // MediaWindow 中的图标
        QIcon pin            ; //  = Utils::to_icon("󰤱", icon_color);  // icon_nf_md_pin_outline               //
        QIcon pin_off        ; //  = Utils::to_icon("󰤰", icon_color);  // icon_nf_md_pin_off_outline           //
        QIcon menu           ; //  = Utils::to_icon("󰍜", icon_color);  // icon_nf_md_menu                      //
        QIcon menu_open      ; //  = Utils::to_icon("󰮫", icon_color);  // icon_nf_md_menu_open                 //
        QIcon fullscreen     ; //  = Utils::to_icon("󰊓", icon_color);  // icon_nf_md_fullscreen                //
        QIcon window_minimize; //  = Utils::to_icon("󰖰", icon_color);  // icon_nf_md_window_minimize           //
        QIcon window_maximize; //  = Utils::to_icon("󰖯", icon_color);  // icon_nf_md_window_maximize           //
        QIcon window_restore ; //  = Utils::to_icon("󰖲", icon_color);  // icon_nf_md_window_restore            //
        QIcon zoom_in        ; //  = Utils::to_icon("", icon_color);  // icon_nf_oct_zoom_in                  //
        QIcon zoom_out       ; //  = Utils::to_icon("", icon_color);  // icon_nf_oct_zoom_out                 //
        QIcon folder_open    ; //  = Utils::to_icon("󰷏", icon_color);  // icon_nf_md_folder_open_outline       //
        QIcon save           ; //  = Utils::to_icon("󰠘", icon_color);  // icon_nf_md_content_save_outline      //
        QIcon save_edit      ; //  = Utils::to_icon("󰳼", icon_color);  // icon_nf_md_content_save_edit_outlin  //
        QIcon arrow_right    ; //  = Utils::to_icon("", icon_color);  // icon_nf_fa_arrow_right               //
        QIcon arrow_left     ; //  = Utils::to_icon("", icon_color);  // icon_nf_fa_arrow_left                //
        QIcon fit_to_screen  ; //  = Utils::to_icon("󱣵", icon_color);  // icon_nf_md_fit_to_screen_outline     //
        QIcon restore        ; //  = Utils::to_icon("󰦛", icon_color);  // icon_nf_md_restore                   //
        QIcon oct_pin        ; //  = Utils::to_icon("", icon_color);  // icon_nf_oct_pin                      // 备用
        QIcon cod_zoom_in    ; //  = Utils::to_icon("", icon_color);  // icon_nf_cod_zoom_in                  // 备用
        QIcon cod_zoom_out   ; //  = Utils::to_icon("", icon_color);  // icon_nf_cod_zoom_out                 // 备用
        FontIconsIcons& reset() {
            profile            = QIcon();
            chat_room_normal   = QIcon();
            chat_room_disabled = QIcon();
            contacts_normal    = QIcon();
            contacts_disabled  = QIcon();
            settings           = QIcon();
            chat_room          = QIcon();
            contacts           = QIcon();
            // MediaWindow 中的图标
            pin             = QIcon();
            pin_off         = QIcon();
            menu            = QIcon();
            menu_open       = QIcon();
            fullscreen      = QIcon();
            window_minimize = QIcon();
            window_maximize = QIcon();
            window_restore  = QIcon();
            zoom_in         = QIcon();
            zoom_out        = QIcon();
            folder_open     = QIcon();
            save            = QIcon();
            save_edit       = QIcon();
            arrow_right     = QIcon();
            arrow_left      = QIcon();
            fit_to_screen   = QIcon();
            restore         = QIcon();
            oct_pin         = QIcon();
            cod_zoom_in     = QIcon();
            cod_zoom_out    = QIcon();
            return *this;
        }
        FontIconsIcons& update(const FontIconsSize&size, const FontIconsColor&color) {
            auto icon_set = [](QIcon &icon, const QString &text_icon, const QColor &color) {
                if(icon.isNull()) { icon = to_icon(text_icon, color); }
            };
            // 生成图标
            icon_set(profile           , "󰙄", color.normal  );
            icon_set(chat_room_normal  , "󰻞", color.normal  );
            icon_set(chat_room_disabled, "󰻞", color.disabled);
            icon_set(contacts_normal   , "󰖸", color.normal  );
            icon_set(contacts_disabled , "󰖸", color.disabled);
            icon_set(settings          , "", color.normal  );
            if( chat_room.isNull()) {
                chat_room.addPixmap(chat_room_normal  .pixmap(size.button), QIcon::Normal  , QIcon::Off);
                chat_room.addPixmap(chat_room_disabled.pixmap(size.button), QIcon::Disabled, QIcon::Off);
            }
            if( contacts.isNull()) {
                contacts.addPixmap(contacts_normal   .pixmap(size.button), QIcon::Normal  , QIcon::Off);
                contacts.addPixmap(contacts_disabled .pixmap(size.button), QIcon::Disabled, QIcon::Off);
            }
            icon_set(pin            , "󰤱", color.normal);
            icon_set(pin_off        , "󰤰", color.normal);
            icon_set(menu           , "󰍜", color.normal);
            icon_set(menu_open      , "󰮫", color.normal);
            icon_set(fullscreen     , "󰊓", color.normal);
            icon_set(window_minimize, "󰖰", color.normal);
            icon_set(window_maximize, "󰖯", color.normal);
            icon_set(window_restore , "󰖲", color.normal);
            icon_set(zoom_in        , "", color.normal);
            icon_set(zoom_out       , "", color.normal);
            icon_set(folder_open    , "󰷏", color.normal);
            icon_set(save           , "󰠘", color.normal);
            icon_set(save_edit      , "󰳼", color.normal);
            icon_set(arrow_right    , "", color.normal);
            icon_set(arrow_left     , "", color.normal);
            icon_set(fit_to_screen  , "󱣵", color.normal);
            icon_set(restore        , "󰦛", color.normal);
            icon_set(oct_pin        , "", color.normal);
            icon_set(cod_zoom_in    , "", color.normal);
            icon_set(cod_zoom_out   , "", color.normal);
            return *this;
        }
    } icon;
    // 重置为初始状态，以便完全重新生成
    FontIcons& reset() {
        size .reset();
        color.reset();
        icon .reset();
        return *this;
    }
    // 根据当前属性生成
    FontIcons& update() {
        size .update();
        color.update();
        icon .update(size, color);
        return *this;
    }
};

namespace TestFontIcons {
    static void test() {
        {
            FontIcons icons;
            icons.size.avatar = QSize(64, 64);
            icons.size.button = QSize(48, 48);
            icons.color.normal = Qt::blue;
            icons.color.disabled = Qt::red;
            icons.update();
        }
        {
            FontIcons icons{
                .size = {
                    .avatar = QSize(64, 64),
                    .button = QSize(48, 48)
                },
                .color = {
                    .normal = Qt::blue,
                    .disabled = Qt::red
                }
            };
            icons.update();
        }
    }
} // namespace TestFontIcons

} // namespace Utils

// 提供对 grpc 消息的输出支持
namespace Utils::Transform {
QString to_string(const QProtobufMessage * msg);
}

// 将支持 qDebug() 的对象转换为字符串
namespace Utils::Transform {
// C++ 20
// #include <QDebug>
// #include <QString>
// #include <concepts>
// 1. 定义一个概念：判断类型 T 是否可以输入到 QDebug 中
template<typename T>
concept PrintableByQDebug = requires(QDebug dbg, const T& value) {
    // { dbg << value } -> std::same_as<QDebug&>;  // 严格要求返回类型为 `QDebug&`，与实际 Qt 默认的 `QDebug` 类型不符
    { dbg << value } -> std::convertible_to<QDebug>;
};
// 2. 使用该概念约束模板函数
template<PrintableByQDebug T>
QString to_string(const T& value) {
    QString result;
    QDebug debug(&result);
    debug.noquote().nospace() << value;
    return result;
}
// C++ 11/14，利用SFINAE规则对模板参数进行约束
// #include <QDebug>
// #include <QString>
// #include <type_traits>
// // 模板定义：只有当表达式 "debug << value" 合法时，这个模板才会被编译器启用
// template<typename T>
// typename std::enable_if<std::is_same<decltype(std::declval<QDebug&>() << std::declval<const T&>()), QDebug&>::value, QString>::type
// to_debug_string(const T& value) {
//     QString result;
//     QDebug debug(&result);
//     debug.noquote().nospace() << value;
//     return result;
// }
}

// 支持智能指针及非智能指针的输出
namespace Utils::Transform {
    // 支持所有指针/智能指针类型的 to_string 模板函数
    //   得益于 PointerLike 概念（Concept）的泛型设计，
    //   它不仅支持 std::shared_ptr，还无缝支持 std::unique_ptr、
    //   QSharedPointer 以及 C++ 原生的普通裸指针（如 ContactInfo*）。
template<typename T>
concept PointerLike = requires(const T& ptr) {
    {  ptr ? true : false }; // 可以做布尔判断
    { *ptr };                // 可以解引用
};
template<PointerLike T>
QString to_string(const T& ptr) {
    // 如果指针为空，直接返回 "[nullptr]"
    if (!ptr) {
        return QStringLiteral("[nullptr]");
    }
    // 如果不为空，解引用并复用上面的基础 to_string 模板
    return to_string(*ptr);
}

}

// 算法相关的工具函数
namespace Utils::Algorithm {
// 计算字符串的 MD5 哈希值，返回 32 字符的十六进制字符串
QString md5(const QString& input);
QString calculate_file_md5(const QString& file_path);

// 流式即时计算 MD5 的工具类
class BytesMd5 {
public:
    void reset() {
        m_md5.reset();
    }
    void update(const QByteArrayView&data_view) {
        m_md5.addData(data_view);
    }
    void update(const char*data, int64_t size) {
        QByteArrayView data_view(data, size);
        m_md5.addData(data_view);
    }
    QString get_hex_string() const {
        return m_md5.result().toHex();
    }
private:
    QCryptographicHash m_md5{QCryptographicHash::Algorithm::Md5};
};

}  // namespace Utils::Algorithm

// tasks
namespace Utils{
template<class T = QString>
class SimpleTasks {
public:
    bool record(const T& id) {
        QMutexLocker locker(&m_mutex);
        if(m_active_tasks.contains(id)) { return false; }
        m_active_tasks.insert(id);
        return true;
    }
    void remove(const T& id) {
        QMutexLocker locker(&m_mutex);
        if (m_active_tasks.contains(id)) {
            m_active_tasks.remove(id);
        }
    }
    bool has_task(const QString&id) {
        return m_active_tasks.contains(id);
    }
private:
    QSet<T> m_active_tasks;
    QMutex  m_mutex       ;
};

}

// class Utils::WidgetTools::LineEdit;
namespace Utils::WidgetTools{
class LineEdit : public QObject{
    Q_OBJECT
public:
    // 要操作的控件
    void add_long_text_tip_button(QLineEdit* line_edit);
    // 封装我们上一题实现的“文本是否全部显示”的精准判断函数
    bool is_text_fully_visible(QLineEdit* lineEdit);
    // 动态刷新 Action 显隐的函数
    void update_elide_action_status(QLineEdit* line_edit);
private:
    bool eventFilter(QObject *watched, QEvent *event);
private:
    std::map<QLineEdit*, QAction*> m_map_edit_action;
};

}

// 系统相关的操作
namespace Utils::OS {
// 检查账号操作是否已经被占用
// windows 管道可以被多个进程同时访问
bool try_grab_account_pipe(const QString& user_id, bool grab = true);
}

// 重载 QDebug 运算符
QDebug operator<<(QDebug debug, const AuthRequest &request);
QDebug operator<<(QDebug debug, const Result &result);
// register
QDebug operator<<(QDebug debug, const SignUpRequest &req);
QDebug operator<<(QDebug debug, const SignUpReply   &rep);
// login
QDebug operator<<(QDebug debug, const SignInRequest    &req);
QDebug operator<<(QDebug debug, const SignInReply      &rep);
// contact info
QDebug operator<<(QDebug debug, const ContactInfo      &contact_info);
QDebug operator<<(QDebug debug, const ContactInfoBase  &contact_info_base);
QDebug operator<<(QDebug debug, const ContactInfoSelf  &contact_info_self);
QDebug operator<<(QDebug debug, const ContactInfoFriend&contact_info_friend);
// search users
QDebug operator<<(QDebug debug, const SearchUserRequest&req);
QDebug operator<<(QDebug debug, const SearchUserReply  &rep);

// auto callback = std::function<void(int error_code, const QString&error_message, ...)>;
// using ResultInfo             = std::shared_ptr<SignUpReply>;
// user register
// rpc SignUp(SignUpRequest) returns (SignUpReply) {}
using signup_params   = std::shared_ptr<SignUpRequest>;
using signup_reply    = std::optional  <SignUpReply  >;
using signup_callback = std::function  <void(const signup_params&, const signup_reply&)>;
// user login
// rpc SignIn(SignInRequest) returns (SignInReply) {}
using signin_params   = std::shared_ptr<SignInRequest>;
using signin_reply    = std::optional  <SignInReply  >;
using signin_callback = std::function  <void(const signin_params&, const signin_reply&)>;
// search users
// rpc SearchUser(SearchUserRequest) returns (SearchUserReply) {}
using search_user_params   = std::shared_ptr<SearchUserRequest>;
using search_user_reply    = std::optional  <SearchUserReply  >;
using search_user_callback = std::function  <void(const search_user_params&, const search_user_reply&)>;
// update profile
// rpc UpdateProfile(UpdateProfileRequest) returns (UpdateProfileReply) {}
using update_profile_params   = std::shared_ptr<UpdateProfileRequest>;
using update_profile_reply    = std::optional  <UpdateProfileReply  >;
using update_profile_callback = std::function  <void(const update_profile_params&, const update_profile_reply&)>;
// get file upload id
// rpc GetFileID(GetFileIDRequest) returns (GetFileIDReply) {}
using get_file_id_params   = std::shared_ptr<GetFileIDRequest>;
using get_file_id_reply    = std::optional  <GetFileIDReply  >;
using get_file_id_callback = std::function  <void(const get_file_id_params&, const get_file_id_reply&)>;
// file upload
// rpc FileUpload(stream FileUploadRequest) returns (stream FileUploadReply);
using file_upload_params   = std::shared_ptr<FileUploadRequest>;
using file_upload_reply    = std::optional  <FileUploadReply  >;
using file_upload_callback = std::function  <void(const file_upload_params&, const file_upload_reply&)>;
// file download
// rpc FileDownload(FileDownloadRequest) returns (stream FileDownloadReply);
using file_download_params   = std::shared_ptr<FileDownloadRequest>;
using file_download_reply    = std::optional  <FileDownloadReply  >;
using file_download_callback = std::function  <void(const file_download_params&, const file_download_reply&)>;
// ContactRequest 好友申请
// rpc ContactRequest(ContactRequestRequest) returns (ContactRequestReply) {}
using contact_request_params   = std::shared_ptr<ContactRequestRequest>;
using contact_request_reply    = std::optional  <ContactRequestReply  >;
using contact_request_callback = std::function  <void(const contact_request_params&, const contact_request_reply&)>;
// 添加联系人的回复
// rpc ContactReply(ContactReplyRequest) returns (ContactReplyReply) {}
using contact_reply_params   = std::shared_ptr<ContactReplyRequest>;
using contact_reply_reply    = std::optional  <ContactReplyReply  >;
using contact_reply_callback = std::function  <void(const contact_reply_params&, const contact_reply_reply&)>;
// 获取添加联系人申请的列表
// rpc GetContactRequests(AuthRequest) returns (GetContactRequestsReply) {}
using get_contact_request_params   = std::shared_ptr<AuthRequest            >;
using get_contact_request_reply    = std::optional  <GetContactRequestsReply>;
using get_contact_request_callback = std::function  <void(const get_contact_request_params&, const get_contact_request_reply&)>;
