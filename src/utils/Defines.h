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
        GROUP  ,  // 分组
        CONTACT,  // 联系人信息
        MESSAGE,  // 消息
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
    return QObject::tr("[ENUMS::ItemGroupType::%1]").arg(m.value(i));
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
// rpc GetLastID(AuthRequest) returns (GetLastIDReply) {}
using get_last_id_params   = std::shared_ptr<AuthRequest   >;
using get_last_id_reply    = std::optional  <GetLastIDReply>;
using get_last_id_callback = std::function  <void(const get_last_id_params&, const get_last_id_reply&)>;
// rpc Sync(stream SyncRequest) returns (stream SyncReply) {}
using sync_params   = std::shared_ptr<SyncRequest>;
using sync_reply    = std::optional  <SyncReply  >;
using sync_callback = std::function  <void(const sync_params&, const sync_reply&)>;
