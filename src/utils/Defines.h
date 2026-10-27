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
        REQUEST ,  // 好友申请
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

// 好友申请
namespace ContactRequestData {
// 好友申请消息
struct RequestMessage {
    QString   message_id;
    QString   account_id;
    QDateTime datetime;
    QString   message_text;
};

// 好友申请数据
struct ItemData {
    QString receiver_account_id;
    QString sender_account_id;
    QVector<RequestMessage> messages;
};

};  // namespace Request

namespace ContactData {
// 聊天列表所需的数据
struct ItemData {
    // ContactInfoBase
    QString     account_id   = {};  // 账号 ID  // 查找联系人时需要
    QString     user_id      = {};  // 用户 ID  // 联系人列表排序时需要
    QString     avatar_id    = {};  // 头像
    QString     nickname     = {};  // 昵称
    int         gender       = {};  // 性别
    QString     birthday     = {};  // 生日
    QString     signature    = {};  // 签名
    QString     region       = {};  // 地区
    QString     description  = {};  // 个人简介
    QStringList phones       = {};  // 手机号
    QStringList emails       = {};  // 邮箱
    QStringList social_links = {};  // 社交链接，逗号分隔
    QStringList moments_id   = {};  // 朋友圈 ID 列表
    QStringList groups_id    = {};  // 群组 ID 列表
    // ContactInfoFriend
    int         status       = {};  // 是否被拉黑
    bool        starred      = {};  // 是否标星
    bool        silent       = {};  // 是否免打扰
    QString     notes        = {};  // 备注
    QString     added_source = {};  // 添加来源
    QString     added_time   = {};  // 添加时间
    QStringList tags         = {};  // 标签，逗号分隔
    // ContactInfoSelf
    bool    modify_self_info  = {};  // 是否是修改个人信息
    QString password          = {};  // 密码
    QString token             = {};  // 认证令牌
    QString security_question = {};  // 安全问题
    QString security_answer   = {};  // 安全答案
    //
    std::shared_ptr<ContactRequestData::ItemData> request_messages;
    // cache  // 以下信息在添加时都需要填充
    bool is_me     = false;
    bool is_friend = false;
    QString display_name;  // 联系人列表排序、显示名称时需要
    QString avatar_path;
    std::shared_ptr<QImage> avatar_image;  // Image 与 UI 无关，可以在子线程中操作
    QString info_text;     // 联系人信息中昵称下面显示的内容(签名或消息)
    QString info_time;     // 消息时间(聊天或好友申请的时间)
};

};  // namespace ContactData

namespace ChatroomData {
// 聊天列表所需的数据
struct ItemData {
    QString   chatroom_id; //
    QString   last_message_text    ;  // last_message_text
    QDateTime last_message_datetime;  // last_message_datetime
    // cache
    QString display_datetime;
    QString display_name;
    QString avatar_id;
    QString avatar_path;
    std::shared_ptr<QImage> avatar_image;  // Image 与 UI 无关，可以在子线程中操作
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
    std::shared_ptr<QImage> avatar_image;
};

struct TextData {
    QString text;
};

struct ImageData {
    // QString image_id;
    // QString image_path;
    // QPixmap pixmap;
    std::shared_ptr<QImage> image;
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
    QString    chatroom_id;
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

namespace Network {
// Network::FileDownloadNotify
struct FileDownloadNotify {
    using Status = FileStatusCode;
    // 上传/下载结束后 ok 才应该设置为true，收发包正确时只应设置code
    bool    finished =      true;  // 所有操作完成，任务退出时设置为 true
    bool    ok__     =     false;  // 是否发生错误
    int64_t code     =        -1;  // 成功或失败的代码
    QString message  = QString();  // 成功或失败的消息
    //
    // FileStatusCode::FILE_STATUS_UPLOADING 也可能是其他客户端正在上传
    QString id       = QString();  // 文件 ID
    Status  status   = FileStatusCode::FILE_STATUS_NONE;  // 文件状态
    //
    QString path           = QString();  // 文件路径 // 上传及下载
    QString name           = QString();  // 文件名称 // 用于下载完成时重命名
    int64_t size           =        -1;  // 文件大小
    QString md5            = QString();  // 文件 md5
    int64_t uploaded_bytes =         -1;  // 文件已经上传/下载完成的大小
};


inline QDebug operator<<(QDebug debug, const FileDownloadNotify &notify) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    debug.nospace() << "NetworkFileIONotify{"
        << "finished:"        << notify.finished
        << ",ok:"             << notify.ok__
        << ",code:"           << (notify.ok__ ? Utils::Transform::to_string(SuccessCode(notify.code)) : Utils::Transform::to_string(ErrorCode(notify.code)))
        << ",message:"        << notify.message
        << ",id:"             << notify.id
        << ",status:"         << Utils::Transform::to_string(notify.status)
        << ",path:"           << notify.path
        << ",name:"           << notify.name
        << ",size:"           << notify.size
        << ",md5:"            << notify.md5
        << ",uploaded_bytes:" << notify.uploaded_bytes
        << "}";
    return debug;
}

}

// 数据只保存在列表中的第 0 列
static int get_data_column() { return 0; }

// auto callback = std::function<void(int error_code, const QString&error_message, ...)>;
// using ResultInfo             = std::shared_ptr<SignUpReply>;
// user register
// rpc SignUp(SignUpRequest) returns (SignUpReply) {}
using signup_params   = std::shared_ptr<SignUpRequest>;
using signup_reply    = std::optional  <SignUpReply  >;
using signup_callback = std::function  <void(signup_params, signup_reply)>;
// user login
// rpc SignIn(SignInRequest) returns (SignInReply) {}
using signin_params   = std::shared_ptr<SignInRequest>;
using signin_reply    = std::optional  <SignInReply  >;
using signin_callback = std::function  <void(signin_params, signin_reply)>;
// search users
// rpc SearchUser(SearchUserRequest) returns (SearchUserReply) {}
using search_user_params   = std::shared_ptr<SearchUserRequest>;
using search_user_reply    = std::optional  <SearchUserReply  >;
using search_user_callback = std::function  <void(search_user_params, search_user_reply)>;
// update profile
// rpc UpdateProfile(UpdateProfileRequest) returns (UpdateProfileReply) {}
using update_profile_params   = std::shared_ptr<UpdateProfileRequest>;
using update_profile_reply    = std::optional  <UpdateProfileReply  >;
using update_profile_callback = std::function  <void(update_profile_params, update_profile_reply)>;
// get file upload id
// rpc GetFileID(GetFileIDRequest) returns (GetFileIDReply) {}
using get_file_id_params   = std::shared_ptr<GetFileIDRequest>;
using get_file_id_reply    = std::optional  <GetFileIDReply  >;
using get_file_id_callback = std::function  <void(get_file_id_params, get_file_id_reply)>;
// file upload
// rpc FileUpload(stream FileUploadRequest) returns (stream FileUploadReply);
using file_upload_params   = std::shared_ptr<FileUploadRequest>;
using file_upload_reply    = std::optional  <FileUploadReply  >;
using file_upload_callback = std::function  <void(file_upload_params, file_upload_reply)>;
// file download
// rpc FileDownload(FileDownloadRequest) returns (stream FileDownloadReply);
using file_download_params   = std::shared_ptr<FileDownloadRequest>;
using file_download_reply    = std::optional  <FileDownloadReply  >;
using file_download_callback = std::function  <void(file_download_params, file_download_reply)>;
// ContactRequest 好友申请
// rpc ContactRequest(ContactRequestRequest) returns (ContactRequestReply) {}
using contact_request_params   = std::shared_ptr<ContactRequestRequest>;
using contact_request_reply    = std::optional  <ContactRequestReply  >;
using contact_request_callback = std::function  <void(contact_request_params, contact_request_reply)>;
// 添加联系人的回复
// rpc ContactReply(ContactReplyRequest) returns (ContactReplyReply) {}
using contact_reply_params   = std::shared_ptr<ContactReplyRequest>;
using contact_reply_reply    = std::optional  <ContactReplyReply  >;
using contact_reply_callback = std::function  <void(contact_reply_params, contact_reply_reply)>;
// 获取添加联系人申请的列表
// rpc GetContactRequests(AuthRequest) returns (GetContactRequestsReply) {}
using get_contact_request_params   = std::shared_ptr<AuthRequest            >;
using get_contact_request_reply    = std::optional  <GetContactRequestsReply>;
using get_contact_request_callback = std::function  <void(get_contact_request_params, get_contact_request_reply)>;
// 获取最好的消息 ID
// rpc GetLastID(AuthRequest) returns (GetLastIDReply) {}
using get_last_id_params   = std::shared_ptr<AuthRequest   >;
using get_last_id_reply    = std::optional  <GetLastIDReply>;
using get_last_id_callback = std::function  <void(get_last_id_params, get_last_id_reply)>;
// 消息同步
// rpc Sync(stream SyncRequest) returns (stream SyncReply) {}
using sync_params   = std::shared_ptr<SyncRequest>;
using sync_reply    = std::optional  <SyncReply  >;
using sync_callback = std::function  <void(sync_params, sync_reply)>;
// 发送消息
// rpc SendMessage(MessageContext) returns (MessageReply) {}
using send_message_params   = std::shared_ptr<MessageContext>;
using send_message_reply    = std::optional  <MessageReply  >;
using send_message_callback = std::function  <void(send_message_params, send_message_reply)>;
