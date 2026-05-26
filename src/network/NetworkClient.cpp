#include "NetworkClient.h"
#include "Accounts.h"
#include "ContactWindow.h"

bool NetworkClient::init() {
    if(m_inited) { return true; }

    if(!server_url().isValid()) { set_server_url(QUrl("http://127.0.0.1:50051")); }

    // TODO:这里可以设置使用 https 加密链接，待客户端界面完善后再完成
    QGrpcChannelOptions grpc_channel_opts;
    m_channel = std::make_shared<QGrpcHttp2Channel>(server_url(), grpc_channel_opts);

    // 初始化各个服务的客户端
    m_client_account = std::make_unique<AccountClient>();
    m_client_file    = std::make_unique<FileClient   >();
    m_client_message = std::make_unique<MessageClient>();
    m_client_search  = std::make_unique<SearchClient >();
    m_client_status  = std::make_unique<StatusClient >();

    // 将客户端连接到共享的 gRPC HTTP/2 通道
    if (!m_client_account->attachChannel(m_channel)) { qDebug().nospace().noquote() << "attach AccountService Channel failed."; return false; }
    if (!m_client_file   ->attachChannel(m_channel)) { qDebug().nospace().noquote() << "attach FileService Channel failed."   ; return false; }
    if (!m_client_message->attachChannel(m_channel)) { qDebug().nospace().noquote() << "attach MessageService Channel failed."; return false; }
    if (!m_client_search ->attachChannel(m_channel)) { qDebug().nospace().noquote() << "attach SearchService Channel failed." ; return false; }
    if (!m_client_status ->attachChannel(m_channel)) { qDebug().nospace().noquote() << "attach StatusService Channel failed." ; return false; }

    m_inited = true;

    return true;
}

// 成功失败都要返回，失败时要返回错误原因
bool NetworkClient::signup(const signup_params& params, const signup_callback& callback) {
    using namespace Utils::Wrapper;
    using Request  = SignUpRequest;
    using Reply    = SignUpReply;
    using ReplyOpt = std::optional<Reply>;
    // 有用的参数
    QString user_id  = params->userId          ();
    QString password = params->password        ();
    QString confirm  = params->confirm         ();
    QString question = params->securityQuestion();
    QString answer   = params->securityAnswer  ();
    // ====================================================================== //
    // 参数校验
    do{
        Result result(ResultWrapper{.error_code = ErrorCode::ERROR_UNSET}()); // 默认设为错误
        // 参数简单校验
        // ===============                                                    //
        ;    if(user_id  .isEmpty()) { result = ResultWrapper{.error_code = ErrorCode::USER_ID_EMPTY             }(); }
        else if(password .isEmpty()) { result = ResultWrapper{.error_code = ErrorCode::PASSWORD_EMPTY            }(); }
        else if(confirm  .isEmpty()) { result = ResultWrapper{.error_code = ErrorCode::CONFIRM_PASSWORD_EMPTY    }(); }
        else if(question .isEmpty()) { result = ResultWrapper{.error_code = ErrorCode::SECURITY_QUESTION_EMPTY   }(); }
        else if(answer   .isEmpty()) { result = ResultWrapper{.error_code = ErrorCode::SECURITY_ANSWER_EMPTY     }(); }
        else if(password != confirm) { result = ResultWrapper{.error_code = ErrorCode::DIFF_PASSWORD_WITH_CONFIRM}(); }
        else if(!m_client_account  ) { result = ResultWrapper{.error_code = ErrorCode::ERROR_INIT                }(); }
        //                                                    =============== //
        // 失败时也要发送通知
        if(result.hasErrorCode() && result.errorCode() != ErrorCode::ERROR_UNSET) {
            ReplyOpt reply_opt = Reply();
            reply_opt->setResult(result);
            if(callback) { callback(params, reply_opt); }
            return false;
        }
        break;
    } while(false);
    // 业务逻辑
    do{
        // ===============                                                    //
        std::unique_ptr<QGrpcCallReply> reply_conn = m_client_account->SignUp(*params, m_grpc_call_options);
        //                                                    =============== //
        const auto *rep_conn_ptr = reply_conn.get();  // 必须
        if (!rep_conn_ptr) {
            ReplyOpt reply_opt = Reply();
            reply_opt->setResult(ResultWrapper{.error_code = ErrorCode::ERROR_CONN}());
            if (callback) { callback(params, reply_opt); }
            return false;
        }
        // 捕获输出返回信息  // 单次模型需要使用 `Qt::SingleShotConnection` 以便结束后销毁
        // lamda 表达式不可使用引用，会造成生命周期问题
        connect(rep_conn_ptr, &QGrpcCallReply::finished, rep_conn_ptr,
            [reply_conn = std::move(reply_conn), params, callback](const QGrpcStatus &status) mutable {
                ReplyOpt reply_opt;
                if(status.isOk()) { reply_opt = reply_conn->read<Reply>(); }
                // 发送参数，返回参数
                // 1. 返回参数为 nullopt 时代表返回错误，仅提示失败即可？？
                // 2. 即使失败也要调用会回调，以便通知界面
                if(callback) { callback(params, reply_opt); }
            }, Qt::SingleShotConnection);
        break;
    } while(false);
    return true;
}

// 成功失败都要返回，失败时要返回错误原因
bool NetworkClient::signin(const signin_params& params, const signin_callback& callback) {
    using namespace Utils::Wrapper;
    using Request  = SignInRequest;
    using Reply    = SignInReply;
    using ReplyOpt = std::optional<Reply>;
    // 有用的参数
    const AuthRequest auth = (params && params->hasRequestUserAuth()) ? params->requestUserAuth() : AuthRequest();
    QString account_id = auth.hasAccountId() ? auth.accountId() : QString();
    QString user_id    = auth.hasUserId   () ? auth.userId   () : QString();
    QString password   = auth.hasPassword () ? auth.password () : QString();
    QString token      = auth.hasToken    () ? auth.token    () : QString();
    // ====================================================================== //
    // 参数校验
    do{
        Result result(ResultWrapper{.error_code = ErrorCode::ERROR_UNSET}()); // 默认设为错误
        // 参数简单校验
        // ===============                                                    //
        ;    if(account_id.isEmpty() && user_id.isEmpty()) { result = ResultWrapper{.error_code = ErrorCode::AUTH_ID_EMPTY    }(); }
        else if(password  .isEmpty() && token  .isEmpty()) { result = ResultWrapper{.error_code = ErrorCode::AUTH_METHOD_EMPTY}(); }
        else if(!m_client_account                        ) { result = ResultWrapper{.error_code = ErrorCode::ERROR_INIT       }(); }
        //                                                    =============== //
        // 失败时也要发送通知
        if(result.hasErrorCode() && result.errorCode() != ErrorCode::ERROR_UNSET) {
            ReplyOpt reply_opt = Reply();
            reply_opt->setResult(result);
            if(callback) { callback(params, reply_opt); }
            return false;
        }
        break;
    } while(false);
    // 业务逻辑
    do{
        // ===============                                                    //
        std::unique_ptr<QGrpcCallReply> reply_conn = m_client_account->SignIn(*params, m_grpc_call_options);
        //                                                    =============== //
        const auto *rep_conn_ptr = reply_conn.get();  // 必须
        if (!rep_conn_ptr) {
            ReplyOpt reply_opt = Reply();
            reply_opt->setResult(ResultWrapper{.error_code = ErrorCode::ERROR_CONN}());
            if (callback) { callback(params, reply_opt); }
            return false;
        }
        // 捕获输出返回信息  // 单次模型需要使用 `Qt::SingleShotConnection` 以便结束后销毁
        // lamda 表达式不可使用引用，会造成生命周期问题
        connect(rep_conn_ptr, &QGrpcCallReply::finished, rep_conn_ptr,
            [reply_conn = std::move(reply_conn), params, callback](const QGrpcStatus &status) mutable {
                ReplyOpt reply_opt;
                if(status.isOk()) { reply_opt = reply_conn->read<Reply>(); }
                if(callback) { callback(params, reply_opt); }
            }, Qt::SingleShotConnection);
        break;
    } while(false);
    return true;
}

// 精准匹配，不返回包含查询词的内容
bool NetworkClient::search_user (const search_user_params&params, const search_user_callback&callback) {
    using namespace Utils::Wrapper;
    using Request  = SearchUserRequest;
    using Reply    = SearchUserReply;
    using ReplyOpt = std::optional<Reply>;
    // 有用的参数
    QString user_keyword = params->userId();
    // ====================================================================== //
    // 参数校验
    do{
        Result result(ResultWrapper{.error_code = ErrorCode::ERROR_UNSET}()); // 默认设为错误
        // 参数简单校验
        // ===============                                                    //
        ;    if(user_keyword.isEmpty()) { result = ResultWrapper{.error_code = ErrorCode::TARGET_USER_ID_EMPTY}(); }
        else if(!m_client_search      ) { result = ResultWrapper{.error_code = ErrorCode::ERROR_INIT          }(); }
        //                                                    =============== //
        // 失败时也要发送通知
        if(result.hasErrorCode() && result.errorCode() != ErrorCode::ERROR_UNSET) {
            ReplyOpt reply_opt = Reply();
            reply_opt->setResult(result);
            if(callback) { callback(params, reply_opt); }
            return false;
        }
        break;
    } while(false);
    // 业务逻辑
    do{
        // ===============                                                    //
        std::unique_ptr<QGrpcCallReply> reply_conn = m_client_search->SearchUser(*params, m_grpc_call_options);
        //                                                    =============== //
        const auto *rep_conn_ptr = reply_conn.get();  // 必须
        if (!rep_conn_ptr) {
            ReplyOpt reply_opt = Reply();
            reply_opt->setResult(ResultWrapper{.error_code = ErrorCode::ERROR_CONN}());
            if (callback) { callback(params, reply_opt); }
            return false;
        }
        // 捕获输出返回信息  // 单次模型需要使用 `Qt::SingleShotConnection` 以便结束后销毁
        // lamda 表达式不可使用引用，会造成生命周期问题
        connect(rep_conn_ptr, &QGrpcCallReply::finished, rep_conn_ptr,
            [reply_conn = std::move(reply_conn), params, callback](const QGrpcStatus &status) mutable {
                ReplyOpt reply_opt;
                if(status.isOk()) { reply_opt = reply_conn->read<Reply>(); }
                // 发送参数，返回参数
                // 1. 返回参数为 nullopt 时代表返回错误，仅提示失败即可？？
                // 2. 即使失败也要调用会回调，以便通知界面
                if(callback) { callback(params, reply_opt); }
            }, Qt::SingleShotConnection);
        break;
    } while(false);
    return true;
}

// 更新个人资料
bool NetworkClient::update_profile(const update_profile_params&params, const update_profile_callback&callback) {
    using namespace Utils::Wrapper;
    using Request  = UpdateProfileRequest;
    using Reply    = UpdateProfileReply;
    using ReplyOpt = std::optional<Reply>;
    // 有用的参数
    const AuthRequest auth = (params && params->hasRequestUserAuth()) ? params->requestUserAuth() : AuthRequest();
    QString account_id = auth.hasAccountId() ? auth.accountId() : QString();
    QString user_id    = auth.hasUserId   () ? auth.userId   () : QString();
    QString password   = auth.hasPassword () ? auth.password () : QString();
    QString token      = auth.hasToken    () ? auth.token    () : QString();
    // ====================================================================== //
    // 参数校验
    do{
        Result result(ResultWrapper{.error_code = ErrorCode::ERROR_UNSET}()); // 默认设为错误
        // 参数简单校验
        // ===============                                                    //
        ;    if(account_id.isEmpty() && user_id.isEmpty()) { result = ResultWrapper{.error_code = ErrorCode::AUTH_ID_EMPTY    }(); }
        else if(password  .isEmpty() && token  .isEmpty()) { result = ResultWrapper{.error_code = ErrorCode::AUTH_METHOD_EMPTY}(); }
        //                                                    =============== //
        // 失败时也要发送通知
        if(result.hasErrorCode() && result.errorCode() != ErrorCode::ERROR_UNSET) {
            ReplyOpt reply_opt = Reply();
            reply_opt->setResult(result);
            if(callback) { callback(params, reply_opt); }
            return false;
        }
        break;
    } while(false);
    // 业务逻辑
    do{
        // ===============                                                    //
        std::unique_ptr<QGrpcCallReply> reply_conn = m_client_account->UpdateProfile(*params, m_grpc_call_options);
        //                                                    =============== //
        const auto *rep_conn_ptr = reply_conn.get();  // 必须
        if (!rep_conn_ptr) {
            ReplyOpt reply_opt = Reply();
            reply_opt->setResult(ResultWrapper{.error_code = ErrorCode::ERROR_CONN}());
            if (callback) { callback(params, reply_opt); }
            return false;
        }
        // 捕获输出返回信息  // 单次模型需要使用 `Qt::SingleShotConnection` 以便结束后销毁
        // lamda 表达式不可使用引用，会造成生命周期问题
        connect(rep_conn_ptr, &QGrpcCallReply::finished, rep_conn_ptr,
            [reply_conn = std::move(reply_conn), params, callback](const QGrpcStatus &status) mutable {
                ReplyOpt reply_opt;
                if(status.isOk()) { reply_opt = reply_conn->read<Reply>(); }
                // 发送参数，返回参数
                // 1. 返回参数为 nullopt 时代表返回错误，仅提示失败即可？？
                // 2. 即使失败也要调用会回调，以便通知界面
                if(callback) { callback(params, reply_opt); }
            }, Qt::SingleShotConnection);
        break;
    } while(false);
    return true;
}

// 获取文件上传 ID
bool NetworkClient::get_file_id(const get_file_id_params&params, const get_file_id_callback&callback) {
    using namespace Utils::Wrapper;
    using Request  = GetFileIDRequest;
    using Reply    = GetFileIDReply;
    using ReplyOpt = std::optional<Reply>;
    // 有用的参数
    const AuthRequest auth = (params && params->hasRequestUserAuth()) ? params->requestUserAuth() : AuthRequest();
    QString account_id = auth.hasAccountId() ? auth.accountId() : QString();
    QString user_id    = auth.hasUserId   () ? auth.userId   () : QString();
    QString password   = auth.hasPassword () ? auth.password () : QString();
    QString token      = auth.hasToken    () ? auth.token    () : QString();
    QString file_name  = params->fileMetadata().fileName();
    qint64  file_size  = params->fileMetadata().fileSize();
    QString file_md5   = params->fileMetadata().fileMd5 ();
    // ====================================================================== //
    // 参数校验
    do{
        Result result(ResultWrapper{.error_code = ErrorCode::ERROR_UNSET}()); // 默认设为错误
        // 参数简单校验
        // ===============                                                    //
        ;    if(account_id.isEmpty() && user_id.isEmpty()) { result = ResultWrapper{.error_code = ErrorCode::AUTH_ID_EMPTY    }(); }
        else if(password  .isEmpty() && token  .isEmpty()) { result = ResultWrapper{.error_code = ErrorCode::AUTH_METHOD_EMPTY}(); }
        else if(file_name .isEmpty()                     ) { result = ResultWrapper{.error_code = ErrorCode::FILE_NAME_EMPTY  }(); }
        else if(file_size  <= 0                          ) { result = ResultWrapper{.error_code = ErrorCode::FILE_SIZE_INVALID}(); }
        else if(file_md5  .isEmpty()                     ) { result = ResultWrapper{.error_code = ErrorCode::FILE_MD5_EMPTY   }(); }
        else if(!m_client_file                           ) { result = ResultWrapper{.error_code = ErrorCode::ERROR_INIT       }(); }
        //                                                    =============== //
        // 失败时也要发送通知
        if(result.hasErrorCode() && result.errorCode() != ErrorCode::ERROR_UNSET) {
            ReplyOpt reply_opt = Reply();
            reply_opt->setResult(result);
            if(callback) { callback(params, reply_opt); }
            return false;
        }
        break;
    } while(false);
    // 业务逻辑
    do{
        // ===============                                                    //
        std::unique_ptr<QGrpcCallReply> reply_conn = m_client_file->GetFileID(*params, m_grpc_call_options);
        //                                                    =============== //
        const auto *rep_conn_ptr = reply_conn.get();  // 必须
        if (!rep_conn_ptr) {
            ReplyOpt reply_opt = Reply();
            reply_opt->setResult(ResultWrapper{.error_code = ErrorCode::ERROR_CONN}());
            if (callback) { callback(params, reply_opt); }
            return false;
        }
        // 捕获输出返回信息  // 单次模型需要使用 `Qt::SingleShotConnection` 以便结束后销毁
        // lamda 表达式不可使用引用，会造成生命周期问题
        connect(rep_conn_ptr, &QGrpcCallReply::finished, rep_conn_ptr,
            [reply_conn = std::move(reply_conn), params, callback](const QGrpcStatus &status) mutable {
                ReplyOpt reply_opt;
                if(status.isOk()) { reply_opt = reply_conn->read<Reply>(); }
                // 发送参数，返回参数
                // 1. 返回参数为 nullopt 时代表返回错误，仅提示失败即可？？
                // 2. 即使失败也要调用会回调，以便通知界面
                if(callback) { callback(params, reply_opt); }
            }, Qt::SingleShotConnection);
        break;
    } while(false);
    return true;
}

bool NetworkClient::get_file_id(const QString&file_path, network_file_io_notify_callback& callback_get_file_id_notiify) {
    using Wrapper  = Utils::Wrapper::FileMetadataWrapper;
    using Notify   = NetworkFileIONotify;
    using Callback = network_file_io_notify_callback;
    // 需要传出的函数
    struct GetFileIdArgs {
        Notify notify;
        // std::shared_ptr<QFile> file_upload;  // 用于传递文件句柄
        // FileDownloadHeader header;
        // bool initialized  = false;  // 已经完成初始化的标记  // 接收到的是文件上传信息还是结果
        // bool error_occued = false;  // 是否已经发生错误
        // QString download_path;
        // QString real_md5;
    };
    std::shared_ptr<GetFileIdArgs> get_file_id_args = std::make_shared<GetFileIdArgs>();
    if(file_path.isEmpty()) {
        get_file_id_args->notify = Notify{ .ok = false, .code = int64_t(ErrorCode::FILE_NAME_EMPTY), .message = "file path empty.", .id = "", .path = file_path };
        callback_get_file_id_notiify(get_file_id_args->notify);
        return true;
    }
    auto file_info = QFileInfo(file_path);
    auto file_name = file_info.fileName();
    auto file_size = file_info.size();
    auto file_md5 = Utils::Algorithm::calculate_file_md5(file_path);
    if(file_name.isEmpty() || file_size < 0 || file_md5.isEmpty()) {
        get_file_id_args->notify = Notify{
            .ok = false, .code = int64_t(ErrorCode::FILE_READ_ERROR), .message = "file not exists.", .id = "",
            .path = file_path, .size = file_size, .md5 = file_md5
        };
        callback_get_file_id_notiify(get_file_id_args->notify);
        return true;
    }
    // 用于参数传递，可避免多次 md5 同一文件
    get_file_id_args->notify = Notify{
        .ok = true, .code = 0, .message = "", .id = "",
        .path = file_path, .size = file_size, .md5 = file_md5
    };
    // 构造参数
    get_file_id_params params_get_file_id = std::make_shared<GetFileIDRequest>();
    params_get_file_id->setRequestUserAuth(*Accounts::instance().get_auth_request().get());
    params_get_file_id->setFileMetadata(Wrapper{.file_name = file_name, .file_size = file_size, .file_md5 = file_md5}());

    // qDebug().noquote().nospace() << Utils::Transform::to_string(params_get_file_id->requestUserAuth());

    get_file_id_callback callback_get_file_id = [callback_get_file_id_notiify, get_file_id_args](const get_file_id_params& request, const get_file_id_reply&reply) {
        // 网络超时或其他错误导致没有收到回复
        if(reply == std::nullopt || !reply->hasResult()) {
            get_file_id_args->notify.ok      = false;
            get_file_id_args->notify.code    = int64_t(ErrorCode::ERROR_CONN);
            get_file_id_args->notify.message = "No reply or no result.";
            callback_get_file_id_notiify(get_file_id_args->notify);
            return;
        }
        //
        get_file_id_args->notify.ok      = reply->result().hasSuccessCode();
        get_file_id_args->notify.code    = get_file_id_args->notify.ok ? int64_t(reply->result().successCode()) : int64_t(reply->result().errorCode());
        get_file_id_args->notify.message = reply->result().message   ();
        get_file_id_args->notify.id      = reply->fileId    ();
        get_file_id_args->notify.status  = reply->fileStatus();
        callback_get_file_id_notiify(get_file_id_args->notify);
    };
    get_file_id(params_get_file_id, callback_get_file_id);
    return true;
}

bool NetworkClient::upload_file(const QString&file_path, network_file_io_notify_callback& callback_upload_file_notify) {
    using namespace Utils::Wrapper;
    using Request  = FileUploadRequest;
    using Reply    = FileUploadReply;
    using ReplyOpt = std::optional<Reply>;
    using Params   = file_upload_params;
    //
    using Header   = FileUploadHeader;
    using Notify   = NetworkFileIONotify;
    using Callback = network_file_io_notify_callback;
    using Stream   = QGrpcBidiStream;

    Callback callback_get_file_id_notify_test = [this, callback_upload_file_notify](const Notify&notify_get_file_id) {
        // 需要传出的函数
        struct FileUploadArgs {
            Notify notify;
            std::shared_ptr<QFile> file_upload;  // 用于传递文件句柄
            // FileDownloadHeader header;
            bool initialized  = false;  // 已经完成初始化的标记  // 接收到的是文件上传信息还是结果
            bool error_occued = false;  // 是否已经发生错误
            // QString download_path;
            // QString real_md5;
        };
        std::shared_ptr<FileUploadArgs> file_upload_args = std::make_shared<FileUploadArgs>();
        file_upload_args->notify = notify_get_file_id;  // 复制，便于修改

        // 获取文件上传 ID 错误
        if(!file_upload_args->notify.ok || file_upload_args->notify.id.isEmpty()) {
            callback_upload_file_notify(file_upload_args->notify);
            return;
        }
        if(file_upload_args->notify.status == FileStatusCode::FILE_STATUS_UPLOADED) {
            file_upload_args->notify.ok      = true;
            file_upload_args->notify.code    = int64_t(SuccessCode::SUCCESS);
            file_upload_args->notify.message = "File already uploaded.";
            callback_upload_file_notify(file_upload_args->notify);
            return;
        }

        // 业务逻辑
        do{
            // ===============                                                    //
            Request reuest;
            Header  header;
            header.setRequestUserAuth(*Accounts::instance().get_auth_request().get());
            header.setFileId(file_upload_args->notify.id);
            // header.setReason("upload file");
            reuest.setHeader(header);
            std::unique_ptr<Stream> reply_conn = m_client_file->FileUpload(reuest, m_grpc_call_options);
            //                                                    =============== //
            const auto *rep_conn_ptr = reply_conn.get();  // 必须
            if (!rep_conn_ptr) {
                file_upload_args->notify.ok      = false;
                file_upload_args->notify.code    = int64_t(ErrorCode::ERROR_CONN);
                file_upload_args->notify.message = "Error in connect.";
                callback_upload_file_notify(file_upload_args->notify);
                return;
            }
            // 收到数据时触发的消息  // 服务器每次 while 写数据到客户端都会触发一次
            connect(rep_conn_ptr, &Stream::messageReceived, this,
                [reply_conn = std::move(reply_conn), file_upload_args, callback_upload_file_notify]() {
                    // 读取失败
                    auto reply = reply_conn->read<FileUploadReply>();
                    if(!reply) {
                        file_upload_args->error_occued   = true;
                        file_upload_args->notify.ok      = false;
                        file_upload_args->notify.code    = int64_t(ErrorCode::ERROR_RECV);
                        file_upload_args->notify.message = "Error in received data.";
                        callback_upload_file_notify(file_upload_args->notify);
                        return;
                    }

                    // 发生错误时已经通知，后续接收到数据时忽略即可，不必通知
                    if(file_upload_args->error_occued) {
                        return;
                    }

                    // 获取文件信息错误
                    if(!reply->hasResult() || !reply->result().hasSuccessCode()) {
                        file_upload_args->error_occued = true;
                        file_upload_args->notify.ok      = false;
                        file_upload_args->notify.code    = int64_t(ErrorCode::FILE_UPLOAD_ERROR);
                        file_upload_args->notify.message = "Result return error.";
                        callback_upload_file_notify(file_upload_args->notify);
                        return;
                    }

                    // 文件已经上传完成则直接返回成功
                    FileStatusCode file_status = reply->fileStatus();
                    if(file_status == FileStatusCode::FILE_STATUS_UPLOADED) {
                        file_upload_args->notify.ok      = true;
                        file_upload_args->notify.code    = int64_t(SuccessCode::SUCCESS);
                        file_upload_args->notify.message = "File already uploaded.";
                        file_upload_args->notify.status  = file_status;
                        callback_upload_file_notify(file_upload_args->notify);
                        return;
                    }
                    // 如果不是获取文件信息时返回的文件上传失败消息，
                    // 那么就是文件上传完成后服务器返回的上传结果，
                    // 此时的文件上传失败就是最终结果
                    else if(file_status != FileStatusCode::FILE_STATUS_UPLOADING && file_upload_args->initialized) {
                        file_upload_args->notify.ok      = false;
                        file_upload_args->notify.code    = int64_t(ErrorCode::FILE_UPLOAD_ERROR);
                        file_upload_args->notify.message = "File upload failed.";
                        file_upload_args->notify.status  = file_status;
                        callback_upload_file_notify(file_upload_args->notify);
                        return;
                    }

                    // 文件头  // 判断是否需要上传及是否进行文件位置调整
                    // 校验已上传大小是否合法
                    file_upload_args->notify.uploaded_bytes = reply->uploadedBytes();  // 已经上传的字节数，支持断点续传时有用
                    file_upload_args->notify.uploaded_bytes = std::max<qint64>(0, file_upload_args->notify.uploaded_bytes);
                    if(file_upload_args->notify.uploaded_bytes > file_upload_args->notify.size) {
                        file_upload_args->error_occued   = true;
                        file_upload_args->notify.ok      = false;
                        file_upload_args->notify.code    = int64_t(ErrorCode::FILE_READ_ERROR);
                        file_upload_args->notify.message = "Uploaded bytes invalid.";
                        callback_upload_file_notify(file_upload_args->notify);
                        return;
                    }

                    // 发送文件内容

                    file_upload_args->file_upload = std::make_shared<QFile>(file_upload_args->notify.path);
                    // 打开文件
                    if(!file_upload_args->file_upload->open(QIODevice::ReadOnly)) {
                        file_upload_args->error_occued   = true;
                        file_upload_args->notify.ok      = false;
                        file_upload_args->notify.code    = int64_t(ErrorCode::FILE_READ_ERROR);
                        file_upload_args->notify.message = tr("open file error.");
                        callback_upload_file_notify(file_upload_args->notify);
                        return;
                    }

                    // 设置文件信息，进行断点续传
                    int64_t upload_size = std::max<qint64>(0, file_upload_args->notify.uploaded_bytes);
                    if (!file_upload_args->file_upload->seek(upload_size)) {
                        file_upload_args->file_upload->close();
                        file_upload_args->error_occued   = true;
                        file_upload_args->notify.ok      = false;
                        file_upload_args->notify.code    = int64_t(ErrorCode::FILE_READ_ERROR);
                        file_upload_args->notify.message = tr("Failed to seek upload file");
                        callback_upload_file_notify(file_upload_args->notify);
                        return;
                    }

                    // 发送文件内容
                    const size_t chunk_size = 128 * 1024;  // 128KB
                    while(!file_upload_args->file_upload->atEnd()) {
                        QByteArray chunk_data = file_upload_args->file_upload->read(chunk_size);
                        qint64 bytes_read = chunk_data.size();
                        if(bytes_read < 0) {
                            file_upload_args->file_upload->close();
                            file_upload_args->error_occued   = true;
                            file_upload_args->notify.ok      = false;
                            file_upload_args->notify.code    = int64_t(ErrorCode::FILE_READ_ERROR);
                            file_upload_args->notify.message = tr("Failed to read upload file");
                            callback_upload_file_notify(file_upload_args->notify);
                            return;
                        }
                        if(bytes_read == 0) {
                            break;  // 文件读取完成
                        }
                        Request request;
                        FileChunkData file_chunk_data;
                        file_chunk_data.setChunkOffset(upload_size);
                        file_chunk_data.setChunkData(chunk_data);
                        request.setChunk(file_chunk_data);
                        reply_conn->writeMessage(request);
                        upload_size += bytes_read;
                    }

                    file_upload_args->file_upload->close();
                    reply_conn->writesDone();
                }
            );
            break;
        } while(false);
    };
    get_file_id(file_path, callback_get_file_id_notify_test);
    return true;
}

bool NetworkClient::download_file(const QString&file_id, qint64 downloaded_size, network_file_io_notify_callback& callback_download_file_notify) {
    using namespace Utils::Wrapper;
    using Request  = FileDownloadRequest;
    using Reply    = FileDownloadReply;
    using ReplyOpt = std::optional<Reply>;
    using Params   = file_download_params;
    // 不要同时下载一个文件
    static Utils::SimpleTasks<QString> s_tasks;
    // qDebug().nospace().noquote() << "tasks:" << s_tasks.m_active_tasks;
    if(!s_tasks.record(file_id)) { return false; }
    //
    using Notify   = NetworkFileIONotify;
    using Callback = network_file_io_notify_callback;
    using Stream   = QGrpcServerStream;
    // 需要传出的函数
    struct FileDownloadArgs {
        using BytesMd5 = Utils::Algorithm::BytesMd5;
        Notify notify;
        std::shared_ptr<QFile> file_download;  // 用于传递文件句柄
        FileDownloadHeader header;
        bool     initialized  = false;  // 已经完成初始化的标记
        bool     error_occued = false;  // 是否已经发生错误
        // QString  download_path;
        qint64   downloaded_size = 0;      // 已经下载的大小，支持断点续传时有用
        QString  real_md5      ;  // hash 下载文件得出的结果
        BytesMd5 calculated_md5;  // hash 下载的文件的工具
    };
    // 获取下载起始位置
    std::shared_ptr<FileDownloadArgs> file_download_args = std::make_shared<FileDownloadArgs>();
    file_download_args->notify.id       = file_id;
    file_download_args->notify.path     = Database::instance().get_local_file_id_path(file_id);
    file_download_args->downloaded_size = qMin<qint64>(qMax<qint64>(0, downloaded_size), QFileInfo(file_download_args->notify.path).size());
    // 路径不为空，代表有下载记录，此时可以打开文件
    if(!file_download_args->notify.path.isEmpty()) {
        Utils::create_dirs(Utils::get_dir(file_download_args->notify.path));  // 确保目录存在
        file_download_args->file_download = std::make_shared<QFile>(file_download_args->notify.path);
        if(!file_download_args->file_download->open(QIODevice::ReadWrite)) {
            file_download_args->notify.ok      = false;
            file_download_args->notify.code    = int64_t(ErrorCode::FILE_READ_ERROR);
            file_download_args->notify.message = tr("Failed to open file for md5 calculation: %1").arg(file_download_args->file_download->errorString());
            callback_download_file_notify(file_download_args->notify);
            return false;
        }
    }
    // 计算已有文件的 md5  // 只读取到下载起始位置与实际文件大小中的下限位置处即可
    int64_t unread_bytes = file_download_args->downloaded_size;
    while(file_download_args->file_download && unread_bytes > 0 && !file_download_args->file_download->atEnd()) {
        const size_t chunk_size = 128 * 1024;  // 128KB
        QByteArray chunk_data = file_download_args->file_download->read(chunk_size);
        file_download_args->calculated_md5.update(chunk_data);
        unread_bytes -= chunk_data.size();
    }
    // 业务逻辑
    do{
        // ===============                                                    //
        Request reuest;
        reuest.setRequestUserAuth(*Accounts::instance().get_auth_request().get());
        reuest.setFileId(file_download_args->notify.id);
        std::unique_ptr<Stream> reply_conn = m_client_file->FileDownload(reuest, m_grpc_call_options);
        //                                                    =============== //
        const auto *rep_conn_ptr = reply_conn.get();  // 必须
        if (!rep_conn_ptr) {
            s_tasks.remove(file_download_args->notify.id);
            file_download_args->notify = Notify{ .ok = false, .code = int64_t(ErrorCode::ERROR_CONN), .message = "Error in connect.", .id = file_id};
            callback_download_file_notify(file_download_args->notify);
            return false;
        }
        // 收到数据时触发的消息  // 服务器每次 while 写数据到客户端都会触发一次
        connect(rep_conn_ptr, &Stream::messageReceived, this,
            [reply_conn = std::move(reply_conn), file_download_args, callback_download_file_notify]() {
                // 发生错误时已经通知，后续接收到数据时忽略即可，不必通知
                if(file_download_args->error_occued) {
                    s_tasks.remove(file_download_args->notify.id);
                    return;
                }

                // 读取失败
                auto reply = reply_conn->read<FileDownloadReply>();
                if(!reply) {
                    s_tasks.remove(file_download_args->notify.id);
                    file_download_args->error_occued   = true;
                    file_download_args->notify.ok      = false;
                    file_download_args->notify.code    = int64_t(ErrorCode::ERROR_RECV);
                    file_download_args->notify.message = "Error in received data.";
                    callback_download_file_notify(file_download_args->notify);
                    return;
                }

                // 文件头
                if(!file_download_args->initialized) {
                    if(!reply->hasHeader()) {
                        s_tasks.remove(file_download_args->notify.id);
                        file_download_args->error_occued   = true;
                        file_download_args->notify.ok      = false;
                        file_download_args->notify.code    = int64_t(ErrorCode::ERROR_RECV);
                        file_download_args->notify.message = "Error in read reply : no header info found.";
                        callback_download_file_notify(file_download_args->notify);
                        return;
                    }

                    file_download_args->header = reply->header();  // 导出
                    if(!file_download_args->header.result().hasSuccessCode()) {
                        s_tasks.remove(file_download_args->notify.id);
                        file_download_args->error_occued   = true;
                        file_download_args->notify.ok      = false;
                        file_download_args->notify.code    = int64_t(ErrorCode::FILE_DOWNLOAD_ERROR);
                        file_download_args->notify.message = "Header return error.";
                        callback_download_file_notify(file_download_args->notify);
                        return;
                    }

                    // 如果是有效的文件头信息，将有效信息保存到用于通知的结构体中
                    auto file_metadata = file_download_args->header.fileMetadata();
                    file_download_args->notify.name = file_download_args->header.fileMetadata().fileName();
                    file_download_args->notify.size = file_download_args->header.fileMetadata().fileSize();
                    file_download_args->notify.md5  = file_download_args->header.fileMetadata().fileMd5();

                    // 校验参数是否有效
                    if(file_download_args->notify.name.isEmpty() || file_download_args->notify.size < 0 || file_download_args->notify.md5.isEmpty()) {
                        s_tasks.remove(file_download_args->notify.id);
                        file_download_args->error_occued   = true;
                        file_download_args->notify.ok      = false;
                        file_download_args->notify.code    = int64_t(ErrorCode::FILE_DOWNLOAD_ERROR);
                        file_download_args->notify.message = "Header's filemeta info not valid.";
                        callback_download_file_notify(file_download_args->notify);
                        return;
                    }

                    // 存储路径为空且文件未打开时，进行初始化
                    if( file_download_args->notify.path.isEmpty() && !file_download_args->file_download) {
                        file_download_args->notify.path = QString("%1/%2.part").arg(Utils::user_cache_path()).arg(file_download_args->notify.name);  //  未下载完成时添加标记
                        // 保存 file_id 和 本地路径的映射关系，以便后续根据 file_id 获取本地文件路径进行删除等操作
                        Database::instance().save_local_file_id_path(file_download_args->notify.id, file_download_args->notify.path); // 未下载完成时也进行存储，下载完成后再重命名及更新记录
                        Utils::create_dirs(Utils::get_dir(file_download_args->notify.path));  // 确保目录存在
                        file_download_args->file_download = std::make_shared<QFile>(file_download_args->notify.path);
                        if(!file_download_args->file_download || !file_download_args->file_download->open(QIODevice::ReadWrite)) {
                            s_tasks.remove(file_download_args->notify.id);
                            file_download_args->error_occued   = true;
                            file_download_args->notify.ok      = false;
                            file_download_args->notify.code    = int64_t(ErrorCode::FILE_WRITE_ERROR);
                            file_download_args->notify.message = "Open file error.";
                            callback_download_file_notify(file_download_args->notify);
                            return;
                        }
                    }
                    file_download_args->file_download->seek(file_download_args->downloaded_size);
                    file_download_args->initialized  = true ;  // 已经成功初始化
                    file_download_args->error_occued = false;

                    // 第一个包是文件信息时，走到这里就已经处理完成
                    return;
                }

                // 写入数据
                if(!reply->hasChunk()) {
                    // 服务器最后一次发送消息？？
                    if(reply->hasHeader()) {
                        // 服务器返回的第一个包就是文件信息时，走到这里说明文件信息有问题
                        file_download_args->error_occued = true;
                        file_download_args->notify.ok      = true;
                        file_download_args->notify.code    = int64_t(SuccessCode::SUCCESS);
                        file_download_args->notify.message = QString("Recv header info, downloaded size: %1, file size : %2")
                            .arg(file_download_args->notify.uploaded_bytes).arg(file_download_args->notify.size);
                        callback_download_file_notify(file_download_args->notify);
                        return;
                    }
                    s_tasks.remove(file_download_args->notify.id);
                    file_download_args->error_occued   = true;
                    file_download_args->notify.ok      = false;
                    file_download_args->notify.code    = int64_t(ErrorCode::FILE_DOWNLOAD_ERROR);
                    file_download_args->notify.message = "Error in read reply : no chunk data found.";
                    callback_download_file_notify(file_download_args->notify);
                    return;
                }
                auto chunk_data = reply->chunk().chunkData();
                qint64 chunk_size = chunk_data.size();
                // 有数据时进行写入
                if(!chunk_data.isEmpty()) {
                    qint64 bytes_written = file_download_args->file_download->write(chunk_data);
                    if(bytes_written != chunk_size) {
                        s_tasks.remove(file_download_args->notify.id);
                        file_download_args->error_occued   = true;
                        file_download_args->notify.ok      = false;
                        file_download_args->notify.code    = int64_t(ErrorCode::FILE_WRITE_ERROR);
                        file_download_args->notify.message = QString("Write file chunk error.\n\npath: %1.\nerror message: %2")
                            .arg(file_download_args->notify.path).arg(file_download_args->file_download->errorString());
                        callback_download_file_notify(file_download_args->notify);
                        return;
                    }

                    // md5 校验
                    file_download_args->calculated_md5.update(chunk_data);
                    file_download_args->notify.uploaded_bytes += bytes_written;

                    // 成功时的进度通知
                    // file_download_args->error_occued  = false;
                    file_download_args->notify.ok        = true;
                    file_download_args->notify.code      = int64_t(SuccessCode::SUCCESS);
                    file_download_args->notify.message   = "Write chunk data success.";
                    file_download_args->notify.uploading = true;
                    callback_download_file_notify(file_download_args->notify);
                }
            }
        );
        // 捕获输出返回信息  // 单次模型需要使用 `Qt::SingleShotConnection` 以便结束后销毁
        // lamda 表达式不可使用引用，会造成生命周期问题
        connect(rep_conn_ptr, &QGrpcCallReply::finished, rep_conn_ptr,
            [this, reply_conn = std::move(reply_conn), file_download_args, callback_download_file_notify](const QGrpcStatus &status) mutable {
                s_tasks.remove(file_download_args->notify.id);
                // 关闭文件句柄
                if (file_download_args->file_download && file_download_args->file_download->isOpen()) {
                    file_download_args->file_download->close();
                }

                // 服务器返回错误
                if (!status.isOk()) {
                    file_download_args->error_occued   = true;
                    file_download_args->notify.ok      = false;
                    file_download_args->notify.code    = int64_t(ErrorCode::ERROR_CONN);
                    file_download_args->notify.message = "Error in received data.";
                    callback_download_file_notify(file_download_args->notify);
                    return;
                }

                // 文件校验失败  // 先注释掉通知，下载出错时已经发送过通知了
                // file_download_params->real_md5 = Utils::Algorithm::calculate_file_md5(file_download_params->notify.path);
                if(file_download_args->notify.md5 != file_download_args->calculated_md5.get_hex_string()) {
                    // file_download_args->error_occued   = true;
                    // file_download_args->notify.ok      = false;
                    // file_download_args->notify.code    = int64_t(ErrorCode::FILE_DOWNLOAD_ERROR);
                    // file_download_args->notify.message = "Error in received data.";
                    // callback_download_file_notify(file_download_args->notify);
                    return;
                }

                // 移动文件及更新记录
                QString new_file_path = QString("%1/%2").arg(Utils::user_download_path()).arg(file_download_args->notify.name);  //  下载完成后的保存位置
                Utils::rename(file_download_args->notify.path, new_file_path);  // 会自动创建父目录
                Database::instance().save_local_file_id_path(file_download_args->notify.id, new_file_path); // 下载完成后需要更新记录

                // 成功时的进度通知
                emit signal_file_id_downloaded(file_download_args->notify.id);
                file_download_args->error_occued = false;
                file_download_args->notify.ok      = true;
                file_download_args->notify.code    = int64_t(SuccessCode::SUCCESS);
                file_download_args->notify.message = tr("Download completed: %1").arg(file_download_args->notify.path);
                callback_download_file_notify(file_download_args->notify);
            }
        );
        break;
    } while(false);
    return false;
}
