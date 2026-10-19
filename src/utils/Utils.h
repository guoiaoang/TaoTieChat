#pragma once

#include "headers.h"

namespace ContactData { struct ItemData; };

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

// 获取显示名称的接口
QString get_display_name(std::shared_ptr<ContactInfo          > contact_info);
QString get_display_name(std::shared_ptr<ContactData::ItemData> contact_data);

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
// 添加圆角遮罩
QImage  to_round_image(const QImage& src, int radius = 10);
// 添加图形遮罩
QPixmap apply_mask(const QPixmap& source, const QPixmap& mask);
// 添加图形遮罩
QImage apply_mask(const QImage& source, const QImage& mask);

// ====================================================================== //
// 获取随机字符串作为全局唯一的 ID
// QUuid::WithBraces 形如：{00000000-0000-0000-0000-000000000000}  // 8-4-4-4-12=32 位
// QUuid::Id128 : 不使用间隔符
QString get_new_uuid_string();

// ====================================================================== //
// 获取错误信息
void set(Result&result, const SuccessCode&success_code);
void set(Result&result, const ErrorCode  &error_code  );

// // ====================================================================== //
// // 查找指定 id 的好友
// std::shared_ptr<ContactData::ItemData> get_contact_data(const QVector<std::shared_ptr<ContactData::ItemData>>&contact_datas, const QString& contact_account_id);
// // 从好友列表中获取第 index 个好友
// std::shared_ptr<ContactData::ItemData> get_contact_data(const QVector<std::shared_ptr<ContactData::ItemData>>&contact_datas, int index);

// ====================================================================== //

// 时间戳转字符串
// 1. [今天, now] -> hh:mm
// 2. [昨天, 今天) -> 昨天 hh:mm
// 3. [一周内， 昨天) -> 周一 hh:mm
// 4. [本年内，一周内) -> MM/dd
// 5. [本年前，本年内) -> yyyy/MM/dd
QString format_smart_time(const QDateTime &datetime);

} // namespace Utils

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
        // collapse
        QIcon collapse;
        QIcon expand  ;
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
            // collapse
            collapse        = QIcon();
            expand          = QIcon();
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
            // collapse
            icon_set(collapse       , "", color.normal);
            icon_set(expand         , "", color.normal);
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
                    .normal   = Qt::blue,
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
QString to_string(SuccessCode code);
QString to_string(ErrorCode   code);
QString to_string(FileStatusCode file_status_code);
int to_int(const FileStatusCode& file_status_code);
TaoTieChat::Timestamp to_timestamp(const QDateTime&datatime);
QString human_size(qint64 bytes); // 字节数 → 人类可读大小（KB/MB/GB）
}

namespace Utils::Transform {
std::shared_ptr<ContactData::ItemData> to_contact_data(std::shared_ptr<ContactInfo          > contact_info);
std::shared_ptr<ContactInfo          > to_contact_info(std::shared_ptr<ContactData::ItemData> contact_data);

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
            ;    if(success_code != SuccessCode::SUCCESS_UNSET) { result.setMessage(Utils::Transform::to_string(success_code)); }
            else if(error_code   != ErrorCode  ::ERROR_UNSET  ) { result.setMessage(Utils::Transform::to_string(error_code)); }
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
    bool has_task(const T&id) {
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

namespace Utils::Transform {
    // static const 是在翻译加载前存储的，因此直接使用的话不会加载翻译
    // static const 会有多个副本，可以使用 inline const (c++17) 代替

    inline const std::map<ErrorCode, const char*> s_map_error_code_strings = {
        // option allow_alias = true;  // 允许不同名但同值的枚举值  // 没有重名会报错
        { ErrorCode::ERROR_UNSET                        , QT_TRANSLATE_NOOP("Utils::Transform", "Unset error code"                  ) },  // = 0x00000000;  // 未设置错误码
        // 通用错误码  // 0x00000000 - 0x00000FFF
        { ErrorCode::UNKNOWN                            , QT_TRANSLATE_NOOP("Utils::Transform", "Unknown error code"                ) },  // = 0x00000001;  // 未知错误
        { ErrorCode::DEFAULT                            , QT_TRANSLATE_NOOP("Utils::Transform", "Default error"                     ) },  // = 0x00000002;  // 发生错误
        // grpc 相关的错误码  // 0x00001001 - 0x00001FFF
        { ErrorCode::ERROR_INIT                         , QT_TRANSLATE_NOOP("Utils::Transform", "Error on init"                     ) },  // = 0x00001001;  // 初始化失败
        { ErrorCode::ERROR_CONN                         , QT_TRANSLATE_NOOP("Utils::Transform", "Error on connect"                  ) },  // = 0x00001002;  // QT 建立连接失败，无法触发 connect 动作
        { ErrorCode::ERROR_RECV                         , QT_TRANSLATE_NOOP("Utils::Transform", "Error on recv or parse"            ) },  // = 0x00001003;  // 接收到的数据不完整或解析失败
        { ErrorCode::ERROR_SEND                         , QT_TRANSLATE_NOOP("Utils::Transform", "Error on send"                     ) },  // = 0x00001004;  // 发送数据失败
        // 数据库相关错误码  // 0x00002001 - 0x00002FFF
        { ErrorCode::DB_QUERY                           , QT_TRANSLATE_NOOP("Utils::Transform", "DB query failed"                   ) },  // = 0x00002001;  // 数据库查询失败
        { ErrorCode::DB_QUERY_EMPTY                     , QT_TRANSLATE_NOOP("Utils::Transform", "DB query empty"                    ) },  // = 0x00002002;  // 数据库查询为空
        { ErrorCode::DB_EXECUTE                         , QT_TRANSLATE_NOOP("Utils::Transform", "DB execute failed"                 ) },  // = 0x00002003;  // 数据库命令执行失败
        { ErrorCode::DB_EXECUTE_EMPTY                   , QT_TRANSLATE_NOOP("Utils::Transform", "DB execute empty"                  ) },  // = 0x00002004;  // 数据库命令执行为空
        { ErrorCode::DB_CHECK                           , QT_TRANSLATE_NOOP("Utils::Transform", "DB check failed"                   ) },  // = 0x00002005;  // 数据库数据插入检查失败
        { ErrorCode::DB_CHECK_EMPTY                     , QT_TRANSLATE_NOOP("Utils::Transform", "DB check empty"                    ) },  // = 0x00002006;  // 数据库数据插入检查为空
        { ErrorCode::DB_RECORD_NOT_FOUND                , QT_TRANSLATE_NOOP("Utils::Transform", "DB record not found"               ) },  // = 0x00002007;  // 数据库记录未找到
        // 状态等相关错误码  // 0x00003001 - 0x00003FFF
        { ErrorCode::UNSUPPORTED                        , QT_TRANSLATE_NOOP("Utils::Transform", "Unsupported"                       ) },  // = 0x00003001;  // 不支持的操作
        { ErrorCode::PENDING                            , QT_TRANSLATE_NOOP("Utils::Transform", "Pending"                           ) },  // = 0x00003002;  // 待处理    // 如：好友请求待处理
        { ErrorCode::ACCEPTED                           , QT_TRANSLATE_NOOP("Utils::Transform", "Accepted"                          ) },  // = 0x00003003;  // 已接受    // 如：同意添加为好友
        { ErrorCode::IGNORED                            , QT_TRANSLATE_NOOP("Utils::Transform", "Ignored"                           ) },  // = 0x00003004;  // 忽略      // 如：不提醒该条消息
        { ErrorCode::REFUSED                            , QT_TRANSLATE_NOOP("Utils::Transform", "Refused"                           ) },  // = 0x00003005;  // 拒绝      // 如：不同意添加为好友
        { ErrorCode::BLOCKED                            , QT_TRANSLATE_NOOP("Utils::Transform", "Blocked"                           ) },  // = 0x00003006;  // 被拉黑    // 如：被对方拉黑
        { ErrorCode::UNASSOCIATED                       , QT_TRANSLATE_NOOP("Utils::Transform", "Unassociated"                      ) },  // = 0x00003101;  // 未关联    // 如：还不是好友
        { ErrorCode::ASSOCIATED                         , QT_TRANSLATE_NOOP("Utils::Transform", "Associated"                        ) },  // = 0x00003102;  // 已关联    // 如：已经是好友
        { ErrorCode::PERMISSION_DENIED                  , QT_TRANSLATE_NOOP("Utils::Transform", "Permission denied"                 ) },  // = 0x00003103;  // 权限不足  // 如：没有权限访问文件
        // 单参数  // 0x00011011 - 0x00011FFF
        { ErrorCode::USER_ID_ERROR                      , QT_TRANSLATE_NOOP("Utils::Transform", "User id error"                     ) },  // = 0x00011011;  // 用户 ID 错误
        { ErrorCode::USER_ID_EMPTY                      , QT_TRANSLATE_NOOP("Utils::Transform", "User id is empty"                  ) },  // = 0x00011012;  // 用户 ID 为空
        { ErrorCode::USER_ID_NOT_FOUND                  , QT_TRANSLATE_NOOP("Utils::Transform", "User id not found"                 ) },  // = 0x00011013;  // 用户 id 不存在
        { ErrorCode::USER_ID_EXISTS                     , QT_TRANSLATE_NOOP("Utils::Transform", "User id already exists"            ) },  // = 0x00011014;  // 用户 id 已存在
        { ErrorCode::ACCOUNT_ID_ERROR                   , QT_TRANSLATE_NOOP("Utils::Transform", "Account id error"                  ) },  // = 0x00011021;  // 账号 ID 错误
        { ErrorCode::ACCOUNT_ID_EMPTY                   , QT_TRANSLATE_NOOP("Utils::Transform", "Account id is empty"               ) },  // = 0x00011022;  // 账号 ID 为空
        { ErrorCode::ACCOUNT_ID_NOT_FOUND               , QT_TRANSLATE_NOOP("Utils::Transform", "Account id not found"              ) },  // = 0x00011023;  // 账号 ID 不存在
        { ErrorCode::PASSWORD_ERROR                     , QT_TRANSLATE_NOOP("Utils::Transform", "Password error"                    ) },  // = 0x00011031;  // 密码错误
        { ErrorCode::PASSWORD_EMPTY                     , QT_TRANSLATE_NOOP("Utils::Transform", "Password is empty"                 ) },  // = 0x00011032;  // 密码为空
        { ErrorCode::PASSWORD_NOT_FOUND                 , QT_TRANSLATE_NOOP("Utils::Transform", "Password not found"                ) },  // = 0x00011033;  // 密码未找到
        { ErrorCode::TOKEN_ERROR                        , QT_TRANSLATE_NOOP("Utils::Transform", "Token error"                       ) },  // = 0x00011041;  // token 错误
        { ErrorCode::TOKEN_EMPTY                        , QT_TRANSLATE_NOOP("Utils::Transform", "Token is empty"                    ) },  // = 0x00011042;  // token 为空
        { ErrorCode::TOKEN_NOT_FOUND                    , QT_TRANSLATE_NOOP("Utils::Transform", "Token not found"                   ) },  // = 0x00011043;  // token 未找到
        { ErrorCode::CONFIRM_PASSWORD_ERROR             , QT_TRANSLATE_NOOP("Utils::Transform", "Confirm password error"            ) },  // = 0x00011051;  // 确认密码错误
        { ErrorCode::CONFIRM_PASSWORD_EMPTY             , QT_TRANSLATE_NOOP("Utils::Transform", "Confirm password is empty"         ) },  // = 0x00011052;  // 确认密码为空
        { ErrorCode::CONFIRM_PASSWORD_NOT_FOUND         , QT_TRANSLATE_NOOP("Utils::Transform", "Confirm password not found"        ) },  // = 0x00011053;  // 确认密码未找到
        { ErrorCode::SECURITY_QUESTION_ERROR            , QT_TRANSLATE_NOOP("Utils::Transform", "Security question error"           ) },  // = 0x00011061;  // 安全问题错误
        { ErrorCode::SECURITY_QUESTION_EMPTY            , QT_TRANSLATE_NOOP("Utils::Transform", "Security question is empty"        ) },  // = 0x00011062;  // 安全问题为空
        { ErrorCode::SECURITY_QUESTION_NOT_FOUND        , QT_TRANSLATE_NOOP("Utils::Transform", "Security question not found"       ) },  // = 0x00011063;  // 安全问题未找到
        { ErrorCode::SECURITY_ANSWER_ERROR              , QT_TRANSLATE_NOOP("Utils::Transform", "Security answer error"             ) },  // = 0x00011071;  // 安全答案错误
        { ErrorCode::SECURITY_ANSWER_EMPTY              , QT_TRANSLATE_NOOP("Utils::Transform", "Security answer is empty"          ) },  // = 0x00011072;  // 安全答案为空
        { ErrorCode::SECURITY_ANSWER_NOT_FOUND          , QT_TRANSLATE_NOOP("Utils::Transform", "Security answer not found"         ) },  // = 0x00011073;  // 安全答案未找到
        // 目标错误  // 0x00012011 - 0x00012FFF
        { ErrorCode::TARGET_USER_ID_ERROR               , QT_TRANSLATE_NOOP("Utils::Transform", "Target user id error"              ) },  // = 0x00012011;  // 目标用户 id 错误
        { ErrorCode::TARGET_USER_ID_EMPTY               , QT_TRANSLATE_NOOP("Utils::Transform", "Target user id is empty"           ) },  // = 0x00012012;  // 目标用户 id 为空
        { ErrorCode::TARGET_USER_ID_NOT_FOUND           , QT_TRANSLATE_NOOP("Utils::Transform", "Target user id not found"          ) },  // = 0x00012013;  // 目标用户 id 未找到
        { ErrorCode::TARGET_USER_ID_EXISTS              , QT_TRANSLATE_NOOP("Utils::Transform", "Target user id exists"             ) },  // = 0x00012014;  // 目标用户 id 已存在
        { ErrorCode::TARGET_ACCOUNT_ID_ERROR            , QT_TRANSLATE_NOOP("Utils::Transform", "Target account id error"           ) },  // = 0x00012021;  // 目标账号 id 错误
        { ErrorCode::TARGET_ACCOUNT_ID_EMPTY            , QT_TRANSLATE_NOOP("Utils::Transform", "Target account id empty"           ) },  // = 0x00012022;  // 目标账号 id 为空
        { ErrorCode::TARGET_ACCOUNT_ID_NOT_FOUND        , QT_TRANSLATE_NOOP("Utils::Transform", "Target account id not found"       ) },  // = 0x00012023;  // 目标账号 id 未找到
        { ErrorCode::TARGET_PASSWORD_ERROR              , QT_TRANSLATE_NOOP("Utils::Transform", "Target password error"             ) },  // = 0x00012031;  // 目标密码错误
        { ErrorCode::TARGET_PASSWORD_EMPTY              , QT_TRANSLATE_NOOP("Utils::Transform", "Target password empty"             ) },  // = 0x00012032;  // 目标密码为空
        { ErrorCode::TARGET_PASSWORD_NOT_FOUND          , QT_TRANSLATE_NOOP("Utils::Transform", "Target password not found"         ) },  // = 0x00012033;  // 目标密码未找到
        { ErrorCode::TARGET_CONFIRM_PASSWORD_ERROR      , QT_TRANSLATE_NOOP("Utils::Transform", "Target confirm password error"     ) },  // = 0x00012051;  // 目标确认密码错误
        { ErrorCode::TARGET_CONFIRM_PASSWORD_EMPTY      , QT_TRANSLATE_NOOP("Utils::Transform", "Target confirm password empty"     ) },  // = 0x00012052;  // 目标确认密码为空
        { ErrorCode::TARGET_CONFIRM_PASSWORD_NOT_FOUND  , QT_TRANSLATE_NOOP("Utils::Transform", "Target confirm password not found" ) },  // = 0x00012053;  // 目标确认密码未找到
        { ErrorCode::TARGET_SECURITY_QUESTION_ERROR     , QT_TRANSLATE_NOOP("Utils::Transform", "Target security question error"    ) },  // = 0x00012061;  // 目标安全问题错误
        { ErrorCode::TARGET_SECURITY_QUESTION_EMPTY     , QT_TRANSLATE_NOOP("Utils::Transform", "Target security question empty"    ) },  // = 0x00012062;  // 目标安全问题为空
        { ErrorCode::TARGET_SECURITY_QUESTION_NOT_FOUND , QT_TRANSLATE_NOOP("Utils::Transform", "Target security question not found") },  // = 0x00012063;  // 目标安全问题未找到
        { ErrorCode::TARGET_SECURITY_ANSWER_ERROR       , QT_TRANSLATE_NOOP("Utils::Transform", "Target security answer error"      ) },  // = 0x00012071;  // 目标安全答案错误
        { ErrorCode::TARGET_SECURITY_ANSWER_EMPTY       , QT_TRANSLATE_NOOP("Utils::Transform", "Target security answer empty"      ) },  // = 0x00012072;  // 目标安全答案为空
        { ErrorCode::TARGET_SECURITY_ANSWER_NOT_FOUND   , QT_TRANSLATE_NOOP("Utils::Transform", "Target security answer not found"  ) },  // = 0x00012073;  // 目标安全答案未找到
        { ErrorCode::TARGET_GROUP_ID_ERROR              , QT_TRANSLATE_NOOP("Utils::Transform", "Target group id error"             ) },  // = 0x00012081;  // 目标群组 ID 错误
        { ErrorCode::TARGET_GROUP_ID_EMPTY              , QT_TRANSLATE_NOOP("Utils::Transform", "Target group id empty"             ) },  // = 0x00012082;  // 目标群组 ID 为空
        { ErrorCode::TARGET_GROUP_ID_NOT_FOUND          , QT_TRANSLATE_NOOP("Utils::Transform", "Target group id not found"         ) },  // = 0x00012083;  // 目标群组 ID 未找到
        // 聊天消息  // 0x00013011 - 0x00013FFF
        { ErrorCode::MESSAGE_TEXT_ERROR                 , QT_TRANSLATE_NOOP("Utils::Transform", "Message text error"                ) },  // = 0x00013011;  // 消息文本错误
        { ErrorCode::MESSAGE_TEXT_EMPTY                 , QT_TRANSLATE_NOOP("Utils::Transform", "Message text empty"                ) },  // = 0x00013012;  // 消息文本为空
        { ErrorCode::MESSAGE_TEXT_NOT_FOUND             , QT_TRANSLATE_NOOP("Utils::Transform", "Message text not found"            ) },  // = 0x00013013;  // 消息文本未找到
        { ErrorCode::MESSAGE_FILE_ERROR                 , QT_TRANSLATE_NOOP("Utils::Transform", "Message file error"                ) },  // = 0x00013021;  // 消息文件错误
        { ErrorCode::MESSAGE_FILE_EMPTY                 , QT_TRANSLATE_NOOP("Utils::Transform", "Message file empty"                ) },  // = 0x00013022;  // 消息文件为空
        { ErrorCode::MESSAGE_FILE_NOT_FOUND             , QT_TRANSLATE_NOOP("Utils::Transform", "Message file not found"            ) },  // = 0x00013023;  // 消息文件未找到
        { ErrorCode::MESSAGE_CONTEXT_ERROR              , QT_TRANSLATE_NOOP("Utils::Transform", "Message context error"             ) },  // = 0x00013031;  // 消息内容错误
        { ErrorCode::MESSAGE_CONTEXT_EMPTY              , QT_TRANSLATE_NOOP("Utils::Transform", "Message context empty"             ) },  // = 0x00013032;  // 消息内容为空
        { ErrorCode::MESSAGE_CONTEXT_NOT_FOUND          , QT_TRANSLATE_NOOP("Utils::Transform", "Message context not found"         ) },  // = 0x00013033;  // 消息内容未找到
        // 文件上传相关错误码  // 0x00004001 - 0x00004FFF
        { ErrorCode::FILE_READ_ERROR                    , QT_TRANSLATE_NOOP("Utils::Transform", "File read error"                   ) },  // = 0x00014001;  // 文件读取错误
        { ErrorCode::FILE_WRITE_ERROR                   , QT_TRANSLATE_NOOP("Utils::Transform", "File write error"                  ) },  // = 0x00014002;  // 文件写入失败
        { ErrorCode::FILE_REQUIRED                      , QT_TRANSLATE_NOOP("Utils::Transform", "File required"                     ) },  // = 0x00014003;  // 文件不能为空
        { ErrorCode::FILE_NOT_FOUND                     , QT_TRANSLATE_NOOP("Utils::Transform", "File not found"                    ) },  // = 0x00014004;  // 文件未找到
        { ErrorCode::FILE_ID_REQUIRED                   , QT_TRANSLATE_NOOP("Utils::Transform", "File id required"                  ) },  // = 0x00014005;  // 需要文件 ID ，但未提供
        { ErrorCode::FILE_HEADER_REQUIRED               , QT_TRANSLATE_NOOP("Utils::Transform", "File header required"              ) },  // = 0x00014006;  // 需要文件信息，但未提供
        { ErrorCode::FILE_NAME_EMPTY                    , QT_TRANSLATE_NOOP("Utils::Transform", "File name empty"                   ) },  // = 0x00014007;  // 文件名不能为空
        { ErrorCode::FILE_SIZE_INVALID                  , QT_TRANSLATE_NOOP("Utils::Transform", "File size invalid"                 ) },  // = 0x00014008;  // 文件大小无效
        { ErrorCode::FILE_MD5_EMPTY                     , QT_TRANSLATE_NOOP("Utils::Transform", "File md5 empty"                    ) },  // = 0x00014009;  // 文件检验码不能为空
        { ErrorCode::FILE_MD5_MISMATCH                  , QT_TRANSLATE_NOOP("Utils::Transform", "File md5 mismatch"                 ) },  // = 0x00014010;  // 文件校验码不匹配
        { ErrorCode::FILE_UPLOAD_ERROR                  , QT_TRANSLATE_NOOP("Utils::Transform", "File upload error"                 ) },  // = 0x00014011;  // 文件上传失败
        { ErrorCode::FILE_DOWNLOAD_ERROR                , QT_TRANSLATE_NOOP("Utils::Transform", "File download error"               ) },  // = 0x00014012;  // 文件下载失败
        // 多参 // 0x00021011 - 0x00021FFF
        // 鉴权
        { ErrorCode::AUTH_INFO_NONE                     , QT_TRANSLATE_NOOP("Utils::Transform", "Auth info none"                    ) },  // = 0x00021011;  // 没有必要的认证信息
        { ErrorCode::AUTH_ID_EMPTY                      , QT_TRANSLATE_NOOP("Utils::Transform", "Auth id empty"                     ) },  // = 0x00021012;  // 认证 id 为空   // 无 user_id 及 account_id
        { ErrorCode::AUTH_METHOD_EMPTY                  , QT_TRANSLATE_NOOP("Utils::Transform", "Auth method empty"                 ) },  // = 0x00021013;  // 认证方法为空  // 无 token 及 password
        { ErrorCode::AUTH_FAILED                        , QT_TRANSLATE_NOOP("Utils::Transform", "Auth failed"                       ) },  // = 0x00021014;  // 认证失败
        { ErrorCode::DIFF_PASSWORD_WITH_CONFIRM         , QT_TRANSLATE_NOOP("Utils::Transform", "Diff password with confirm"        ) },  // = 0x00021021;  // 确认密码与密码不一致
        { ErrorCode::TARGET_ID_REQUESTED                , QT_TRANSLATE_NOOP("Utils::Transform", "Target id requested"               ) },  // = 0x00021022;  // 多参 // 发送聊天消息时，需要提供目标 ID，但未提供
        { ErrorCode::CONTEXT_REQUIRED                   , QT_TRANSLATE_NOOP("Utils::Transform", "Context required"                  ) },  // = 0x00021023;  // 多参 // 发送聊天消息时，需要上下文信息，但未提供
        // 业务  // 0x00031011 - 0x00031FFF
        { ErrorCode::CLIENT_MESSAGE_ID_REQUIRED         , QT_TRANSLATE_NOOP("Utils::Transform", "Client message id is required"     ) },  // = 0x10011011;  // 需要客户端消息 ID，但未提供
        { ErrorCode::DUPLICATE_MESSAGE_ID               , QT_TRANSLATE_NOOP("Utils::Transform", "Duplicate message id"              ) },  // = 0x10011012;  // 重复的客户端消息 id
        { ErrorCode::FRIEND_REQUEST_NOT_FOUND           , QT_TRANSLATE_NOOP("Utils::Transform", "Friend request not found"          ) },  // = 0x10011013;  // 好友请求未找到
    };

    inline const std::map<FileStatusCode, QString> s_map_file_status_code_strings = {
        { FileStatusCode::FILE_STATUS_NONE     , "FILE_STATUS_NONE"      },
        { FileStatusCode::FILE_STATUS_UPLOADED , "FILE_STATUS_UPLOADED"  },
        { FileStatusCode::FILE_STATUS_UPLOADING, "FILE_STATUS_UPLOADING" },
        { FileStatusCode::FILE_STATUS_NOT_FOUND, "FILE_STATUS_NOT_FOUND" },
    };

    inline const std::map<FileStatusCode, int> s_map_file_status_string_ints = {
        { FileStatusCode::FILE_STATUS_NONE     , 0 },
        { FileStatusCode::FILE_STATUS_UPLOADED , 1 },
        { FileStatusCode::FILE_STATUS_UPLOADING, 2 },
        { FileStatusCode::FILE_STATUS_NOT_FOUND, 3 },
    };

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
// rpc Sync(stream SyncRequest) returns (stream SyncReply) {}
QDebug operator<<(QDebug debug, const SyncRequest&req);
QDebug operator<<(QDebug debug, const SyncReply  &rep);
QDebug operator<<(QDebug debug, const SyncTag&tag);
QDebug operator<<(QDebug debug, const SyncReplyHeader&header);
QDebug operator<<(QDebug debug, const SyncContact&sync_contact);
QDebug operator<<(QDebug debug, const SyncReplyBody&body);
QDebug operator<<(QDebug debug, const SyncReply&rep);
//
QDebug operator<<(QDebug debug, const Timestamp&ts);
// 发送消息
// rpc SendMessage(MessageContext) returns (MessageReply) {}
QDebug operator<<(QDebug debug, const MessageContext&req);
QDebug operator<<(QDebug debug, const MessageReply&rep);
