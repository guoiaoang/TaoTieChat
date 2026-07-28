#pragma once

#include "headers.h"
#include "Utils.h"
#include "Defines.h"

struct NetworkFileIONotify{
    using Status = FileStatusCode;
    bool    ok      = false    ;  // 是否上传成功
    int64_t code    = -1       ;  // 成功或失败的代码
    QString message = QString();  // 成功或失败的消息
    QString id      = QString();  // 文件 ID
    Status  status  = FileStatusCode::FILE_STATUS_NONE;  // 文件状态
    //
    QString path     = QString();  // 文件路径 // 上传及下载
    QString name     = QString();  // 文件名称 // 用于下载完成时重命名
    int64_t size     = -1       ;  // 文件大小
    QString md5      = QString();  // 文件 md5
    int64_t uploaded_bytes = -1 ;  // 文件已经上传/下载完成的大小
    bool    uploading = false   ;  // 是否是正在上传/下载时的通知消息
};

inline QDebug operator<<(QDebug debug, const NetworkFileIONotify &notify) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    debug.nospace() << "NetworkFileIONotify{"
        << "ok:"              << notify.ok
        << ",code:"           << (notify.ok ? Utils::Transform::to_string(SuccessCode(notify.code)) : Utils::Transform::to_string(ErrorCode(notify.code)))
        << ",message:"        << notify.message
        << ",id:"             << notify.id
        << ",status:"         << Utils::Transform::to_string(notify.status)
        << ",path:"           << notify.path
        << ",name:"           << notify.name
        << ",size:"           << notify.size
        << ",md5:"            << notify.md5
        << ",uploaded_bytes:" << notify.uploaded_bytes
        << ",uploading:"      << notify.uploading
        << "}";
    return debug;
}

using network_file_io_notify_callback = std::function<void(const NetworkFileIONotify&)>;

class NetworkClient : public QObject{
    Q_OBJECT
private:
    NetworkClient() = default;
    NetworkClient(const NetworkClient&) = delete;
    NetworkClient operator=(const NetworkClient&) = delete;
public:
    ~NetworkClient() = default;
    static NetworkClient& instance() {
        static NetworkClient s_network_client;
        return s_network_client;
    }

    void set_server_url(const QUrl&url) { m_url_server = url; }
    QUrl server_url() const { return m_url_server; }
    bool init();
    // bool reset_connection();
    bool disconnect_sync(const QString& account_id);

public:
    // 直接接口
    bool signup         (const signup_params        & params, const signup_callback        & callback);  // 注册账号
    bool signin         (const signin_params        & params, const signin_callback        & callback);  // 登录账号
    bool search_user    (const search_user_params   & params, const search_user_callback   & callback);  // 搜索用户 ID // 精准匹配
    bool update_profile (const update_profile_params& params, const update_profile_callback& callback);  // 更新个人资料
    bool get_file_id    (const get_file_id_params   & params, const get_file_id_callback   & callback);  // 获取文件上传 ID
    bool contact_request    (const contact_request_params    & params, const contact_request_callback    & callback);  // 好友申请
    bool contact_reply      (const contact_reply_params      & params, const contact_reply_callback      & callback);  // 被好友申请的回复
    bool get_contact_request(const get_contact_request_params& params, const get_contact_request_callback& callback);  // 获取被好友申请的列表
    bool get_last_id        (const get_last_id_params        & params, const get_last_id_callback        & callback);  // pts
    bool sync               (const sync_params               & params, const sync_callback               & callback);  // pts
    // 接口封装
    bool search_group (const QString&group) { return false; } // 搜索群聊 ID
    // TODO:get_file_id 应该封装在子线程中
    bool get_file_id  (const QString&file_path, network_file_io_notify_callback& callback_get_file_id_notiify );
    // TODO:upload_file 应该封装在子线程中
    bool upload_file  (const QString&file_path, network_file_io_notify_callback& callback_upload_file_notify  );
    // TODO:download_file 应该封装在子线程中
    bool download_file(const QString&file_id  , qint64 downloaded_size, network_file_io_notify_callback& callback_download_file_notify);

signals:
    // 文件下载成功通知
    void signal_file_id_downloaded(const QString&file_id);

private:
    bool m_inited = false;
    QUrl m_url_server;
    // ::TaoTieChat::XxxService::Client;
    std::unique_ptr<AccountClient> m_client_account;
    std::unique_ptr<FileClient   > m_client_file   ;
    std::unique_ptr<MessageClient> m_client_message;
    std::unique_ptr<SearchClient > m_client_search ;
    std::unique_ptr<StatusClient > m_client_status ;
    std::unique_ptr<SyncClient   > m_client_sync   ;
    // 共享的 gRPC HTTP/2 通道
    std::shared_ptr<QGrpcHttp2Channel> m_channel;  // 相同 channel 代表长连接
    // 公共的调用选项  // 长连接保活  // 心跳包并不能保活
    QGrpcCallOptions m_grpc_call_options = QGrpcCallOptions{}.setDeadlineTimeout(std::chrono::years(9999));
    // 用于断开同步线程
    std::unordered_map<QString, QPointer<QGrpcBidiStream>> m_cache_connection_syncs;
};
