#pragma once

#include "Headers.h"
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
QString get_dir(const QString& path);
bool create_dirs(QString full_path);
bool remove_dirs(QString full_path);
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
namespace Utils {
    using namespace ::TaoTieChat;
struct ContactInfoWrapper {
    QString     account_id        = QString    { };
    QString     user_id           = QString    { };
    QString     avatar_id         = QString    { };
    QString     nickname          = QString    { };
    int64_t     gender            = int64_t    {0};
    QString     birthday          = QString    { };
    QString     sign              = QString    { };
    QString     region            = QString    { };
    QString     desc              = QString    { };
    QStringList phones            = QStringList{ };
    QStringList emails            = QStringList{ };
    QStringList social_links      = QStringList{ };
    QStringList moments_id        = QStringList{ };
    QStringList groups_id         = QStringList{ };
    bool        is_friend         = false         ;
    bool        is_blocked        = false         ;
    bool        is_starred        = false         ;
    bool        is_silent         = false         ;
    QString     notes             = QString    { };
    QString     from              = QString    { };
    QString     added_time        = QString    { };
    QStringList tags              = QStringList{ };
    bool        modify_self_info  = false         ;
    QString     password          = QString    { };
    QString     token             = QString    { };
    QString     security_question = QString    { };
    QString     security_answer   = QString    { };
    ContactInfoWrapper(const std::shared_ptr<ContactInfo> contact_info = nullptr) {
        if(contact_info == nullptr) {
            return;
        }
        auto baseInfo   = contact_info->hasBaseInfo  () ? contact_info->baseInfo  () : ContactInfoBase  {};
        auto friendInfo = contact_info->hasFriendInfo() ? contact_info->friendInfo() : ContactInfoFriend{};
        auto selfInfo   = contact_info->hasSelfInfo  () ? contact_info->selfInfo  () : ContactInfoSelf  {};
        // 基本信息
        account_id   = baseInfo.accountId  ();
        user_id      = baseInfo.userId     ();
        avatar_id    = baseInfo.avatarId   ();
        nickname     = baseInfo.nickname   ();
        gender       = baseInfo.gender     ();
        birthday     = baseInfo.birthday   ();
        sign         = baseInfo.sign       ();
        region       = baseInfo.region     ();
        desc         = baseInfo.desc       ();
        phones       = baseInfo.phones     ();
        emails       = baseInfo.emails     ();
        social_links = baseInfo.socialLinks();
        moments_id   = baseInfo.momentsId  ();
        groups_id    = baseInfo.groupsId   ();
        // 好友信息
        is_starred   = friendInfo.starred    ();
        is_silent    = friendInfo.silent     ();
        notes        = friendInfo.notes      ();
        from         = friendInfo.addedSource();
        added_time   = friendInfo.addedTime  ();
        tags         = friendInfo.tags       ();
        // 自己的信息
        modify_self_info  = selfInfo.modifySelfInfo  ();
        password          = selfInfo.password        ();
        token             = selfInfo.token           ();
        security_question = selfInfo.securityQuestion();
        security_answer   = selfInfo.securityAnswer  ();
    }
};
} // namespace Utils

//
namespace Utils {
// 必须在加载字体后创建该结构体的实例，以确保图标能够正确显示
struct UtilsFontIcons {
    QSize  size_avatar            ;
    QSize  size_button            ;
    QColor icon_color             ;
    QColor icon_color_disabled    ;
    QIcon  icon_profile           ;
    QIcon  icon_chat_room_normal  ;
    QIcon  icon_chat_room_disabled;
    QIcon  icon_contacts_normal   ;
    QIcon  icon_contacts_disabled ;
    QIcon  icon_settings          ;
    QIcon  icon_chat_room         ;
    QIcon  icon_contacts          ;
    UtilsFontIcons() {
        this->size_avatar             = QSize(48, 48);
        this->size_button             = QSize(36, 36);
        this->icon_color              = QApplication::palette().color(QPalette::WindowText);
        this->icon_color_disabled     = QApplication::palette().color(QPalette::Disabled, QPalette::WindowText);
        this->icon_profile            = Utils::to_icon("󰙄", this->icon_color);
        this->icon_chat_room_normal   = Utils::to_icon("󰻞", this->icon_color);
        this->icon_chat_room_disabled = Utils::to_icon("󰻞", this->icon_color_disabled);
        this->icon_contacts_normal    = Utils::to_icon("󰖸", this->icon_color);
        this->icon_contacts_disabled  = Utils::to_icon("󰖸", this->icon_color_disabled);
        this->icon_settings           = Utils::to_icon("", this->icon_color);
        this->icon_chat_room.addPixmap(this->icon_chat_room_normal  .pixmap(this->size_button), QIcon::Normal  , QIcon::Off);
        this->icon_chat_room.addPixmap(this->icon_chat_room_disabled.pixmap(this->size_button), QIcon::Disabled, QIcon::Off);
        this->icon_contacts .addPixmap(this->icon_contacts_normal   .pixmap(this->size_button), QIcon::Normal  , QIcon::Off);
        this->icon_contacts .addPixmap(this->icon_contacts_disabled .pixmap(this->size_button), QIcon::Disabled, QIcon::Off);
    }
};

} // namespace Utils

namespace Utils::Transform {
QString to_string(const QProtobufMessage * msg);

}

// 重载 QDebug 运算符
QDebug operator<<(QDebug debug, const Result &result);
// register
QDebug operator<<(QDebug debug, const SignUpRequest &req);
QDebug operator<<(QDebug debug, const SignUpReply   &rep);
// login
QDebug operator<<(QDebug debug, const SignInRequest    &req);
QDebug operator<<(QDebug debug, const SignInReply      &rep);
// search users
QDebug operator<<(QDebug debug, const ContactInfo      &contact_info);
QDebug operator<<(QDebug debug, const SearchUserRequest&req    );
QDebug operator<<(QDebug debug, const SearchUserReply  &rep    );

// auto callback = std::function<void(int error_code, const QString&error_message, ...)>;
// using ResultInfo             = std::shared_ptr<SignUpReply>;
// user register
using signup_params   = std::shared_ptr<SignUpRequest>;
using signup_reply    = std::optional  <SignUpReply  >;
using signup_callback = std::function  <void(const signup_params&, const signup_reply&)>;
// user login
using signin_params   = std::shared_ptr<SignInRequest>;
using signin_reply    = std::optional  <SignInReply  >;
using signin_callback = std::function  <void(const signin_params&, const signin_reply&)>;
// search users
using search_user_params   = std::shared_ptr<SearchUserRequest>;
using search_user_reply    = std::optional  <SearchUserReply  >;
using search_user_callback = std::function  <void(const search_user_params&, const search_user_reply&)>;
// update profile
using update_profile_params   = std::shared_ptr<UpdateProfileRequest>;
using update_profile_reply    = std::optional  <UpdateProfileReply  >;
using update_profile_callback = std::function  <void(const update_profile_params&, const update_profile_reply&)>;
