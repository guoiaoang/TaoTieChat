#pragma once

#include "headers.h"

// 名字空间中声明 enum
namespace ENUMS {
    Q_NAMESPACE // 告诉 Qt 这是一个有元数据的命名空间
    // 分组
    enum class ItemGroupType : int {
        UNSET  ,
        NONE   ,  // 没有分组标签
        REQUEST,  // 好友申请
        FRIENDS,  // 联系人
    };
    Q_ENUM_NS(ItemGroupType)
    // qRegisterMetaType<ENUMS::ItemGroupType>("ENUMS::ItemGroupType");

    // 声明标志 // Flags::A|Flags::B
    // Q_DECLARE_FLAGS(ItemGroupType, ItemGroupType)
    // Q_DECLARE_OPERATORS_FOR_FLAGS(ItemGroupType)

    enum class ItemDataType : int {
        UNSET,
        NONE ,
        GROUP   ,  // 分组
        CONTACT ,  // 联系人信息
        CHATROOM,  // 对话
        MESSAGE ,  // 消息
    };
    Q_ENUM_NS(ItemDataType)
    static int role_item_type() { return Qt::UserRole + 1; }
    static int role_item_data() { return Qt::UserRole + 2; }
}

namespace Utils::Transform {
// 获取错误信息
static QString to_string(ENUMS::ItemGroupType group_type) {
    static const QMap<ENUMS::ItemGroupType, QString> map_groups = {
        // {ENUMS::ItemGroupType::UNSET  , "UNSET"},
        // {ENUMS::ItemGroupType::NONE   , "NONE"},
        {ENUMS::ItemGroupType::REQUEST, QObject::tr("Contact Request")},
        {ENUMS::ItemGroupType::FRIENDS, QObject::tr("Friends"        )},
    };
    if (map_groups.contains(group_type)) {
        return map_groups[group_type];
    }
    int i = static_cast<int>(group_type);
    auto m = QMetaEnum::fromType<ENUMS::ItemGroupType>();
    return QString("[ENUMS::ItemGroupType::%1]").arg(m.value(i));
}
}

#ifdef _DEBUG
// QObject 类中声明 enum
class MyEnums : public QObject {
    Q_OBJECT

public:
    // 1. 声明强类型枚举
    enum class ItemGroupType : int {
        UNSET  ,
        NONE   ,  // 没有分组标签
        REQUEST,  // 好友申请
        FRIENDS,  // 联系人
    };

    // 2. 关键：向 Qt 元对象系统注册该枚举
    Q_ENUM(ItemGroupType)
};
#endif // _DEBUG

namespace ContactData {
// 聊天列表所需的数据
struct ItemData {
    QString contact_id;
    QString nickname;
    QString notes;
    QString signature;
    // cache
    QString display_name;
    QString avatar_id;
    QString avatar_path;
    QImage  avatar_image;  // Image 与 UI 无关，可以在子线程中操作
};
};

namespace ChatroomData {
// 聊天列表所需的数据
struct ItemData {
    // std::shared_ptr<ContactInfo> contact;
    QString   chatroom_id; //
    QString   last_message_text    ;  // last_message_text
    QDateTime last_message_datetime;  // last_message_datetime
    // cache
    QString display_datetime;
    QString display_name;
    QString avatar_id;
    QString avatar_path;
    QImage  avatar_image;  // Image 与 UI 无关，可以在子线程中操作
};
};

namespace MessageData {

enum class SenderType : short {
    SYSTEM = 11,
    USER   = 21
};

enum class MediaType : short {
    TIMESTAMP = 11,
    TEXT      = 21,
    IMAGE     = 22,
    AUDIO     = 23,
    VIDEO     = 24,
    FILE      = 25,
};

struct SenderData {
    QString account_id;
    QString display_name;
    QString avatar_id;
    QString avatar_path;
    QPixmap avatar_pixmap;
};

struct TextData {
    QString text;
};

struct ImageData {
    // QString image_id;
    // QString image_path;
    QPixmap pixmap;
};

struct AudioData {};
struct VideoData {};
struct FileData  {
    QString file_id;
    QString file_path;
    QString file_hash;
    // cache 数据
    QString file_name;
    QIcon   file_icon;
    qint64  file_size = -1;  // 文件总大小
    QString human_size;      // 文件总大小
    qint64  downloaded_file_size = -1;  // 已下载的文件大小
    QString downloaded_human_size;      // 已下载的文件大小
};

struct ItemCache {
    ;
};

// 消息内容
struct ItemData {
    // std::shared_ptr<MessageContext> context;
    // std::shared_ptr<ItemGeometry  > geometry;
    //
    SenderType sender_type = SenderType::USER;
    MediaType  media_type  = MediaType::TEXT;
    bool       is_me = false;
    QString    message_id;
    QDateTime  message_time;
    std::shared_ptr<SenderData> sender;  // 发送者信息
    std::shared_ptr<FileData  > file;    // 图片视频等也均为文件
    std::shared_ptr<TextData  > text;
    std::shared_ptr<ImageData > image;
    std::shared_ptr<AudioData > audio;
    std::shared_ptr<VideoData > video;
    // cache
    ItemCache cache;
};

/**
 * @brief 主题描述：所有绘制颜色集中于此，深/浅色各一套。
 *
 * 设计要点：QTextDocument 缓存是“主题无关”的——HTML 中不写死任何
 * 正文颜色，颜色在 paint 阶段通过 QAbstractTextDocumentLayout::PaintContext
 * 的调色板注入。因此切换深/浅色主题时缓存完全不用失效重建。
 */
struct ChatTheme {
    QColor viewBg;      ///< 消息列表背景
    QColor windowBg;    ///< 工具栏/输入栏等面板背景
    QColor bubbleMe;    ///< 我的气泡背景
    QColor bubbleOther; ///< 对方气泡背景
    QColor text;        ///< 正文默认颜色
    QColor systemText;  ///< 系统消息颜色
    QColor metaText;    ///< 发送者/时间行颜色
    QColor border;      ///< 气泡描边
    QColor link;        ///< 超链接颜色
    QColor selBg;       ///< 文字选区高亮背景
    QColor selText;     ///< 文字选区高亮前景

    static ChatTheme light();
    static ChatTheme dark();
};

inline ChatTheme ChatTheme::light() {
    ChatTheme t;
    t.viewBg      = QColor(0xF5, 0xF5, 0xF5);
    t.windowBg    = QColor(0xFA, 0xFA, 0xFA);
    t.bubbleMe    = QColor(0x95, 0xEC, 0x69);   // 微信绿
    t.bubbleOther = QColor(0xFF, 0xFF, 0xFF);
    t.text        = QColor(0x1A, 0x1A, 0x1A);
    t.systemText  = QColor(0x8A, 0x8A, 0x8A);
    t.metaText    = QColor(0x9A, 0x9A, 0x9A);
    t.border      = QColor(0, 0, 0, 25);
    t.link        = QColor(0x1E, 0x88, 0xE5);
    t.selBg       = QColor(0x1E, 0x88, 0xE5);
    t.selText     = QColor(0xFF, 0xFF, 0xFF);
    return t;
}

inline ChatTheme ChatTheme::dark() {
    ChatTheme t;
    t.viewBg      = QColor(0x1B, 0x1B, 0x1B);
    t.windowBg    = QColor(0x20, 0x20, 0x20);
    t.bubbleMe    = QColor(0x2E, 0x7D, 0x46);   // 暗化的微信绿
    t.bubbleOther = QColor(0x2C, 0x2C, 0x2C);
    t.text        = QColor(0xE4, 0xE4, 0xE4);
    t.systemText  = QColor(0x8A, 0x8A, 0x8A);
    t.metaText    = QColor(0x8F, 0x8F, 0x8F);
    t.border      = QColor(255, 255, 255, 30);
    t.link        = QColor(0x64, 0xB5, 0xF6);
    t.selBg       = QColor(0x37, 0x62, 0xA8);
    t.selText     = QColor(0xFF, 0xFF, 0xFF);
    return t;
}

struct ChatMessage {
    enum class Type {
        Text,    ///< 普通聊天消息（富文本气泡）
        System,  ///< 系统提示（居中灰色小字）
        Image,   ///< 图片消息（缩放气泡，来自剪贴板粘贴或文件选择）
        File,    ///< 文件消息（文件卡片，点击打开所在位置）
    };
};

}

// 数据只保存在列表中的第 0 列
static int get_data_column() { return 0; }

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
// 获取最好的消息 ID
// rpc GetLastID(AuthRequest) returns (GetLastIDReply) {}
using get_last_id_params   = std::shared_ptr<AuthRequest   >;
using get_last_id_reply    = std::optional  <GetLastIDReply>;
using get_last_id_callback = std::function  <void(const get_last_id_params&, const get_last_id_reply&)>;
// 消息同步
// rpc Sync(stream SyncRequest) returns (stream SyncReply) {}
using sync_params   = std::shared_ptr<SyncRequest>;
using sync_reply    = std::optional  <SyncReply  >;
using sync_callback = std::function  <void(const sync_params&, const sync_reply&)>;
// 发送消息
// rpc SendMessage(MessageContext) returns (MessageReply) {}
using send_message_params   = std::shared_ptr<MessageContext>;
using send_message_reply    = std::optional  <MessageReply  >;
using send_message_callback = std::function  <void(const send_message_params&, const send_message_reply&)>;
