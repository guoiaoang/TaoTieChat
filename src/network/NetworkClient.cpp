#include "NetworkClient.h"
#include "Accounts.h"
#include "Database.h"
#include "SignalInner.h"

bool NetworkClient::init() {
    if (m_inited) { return true; }

    if (!server_url().isValid()) { set_server_url(QUrl("http://127.0.0.1:50051")); }

    // TODO:这里可以设置使用 https 加密链接，待客户端界面完善后再完成
    QGrpcChannelOptions grpc_channel_opts;
    m_channel = std::make_shared<QGrpcHttp2Channel>(server_url(), grpc_channel_opts);

    // 初始化各个服务的客户端
    m_client_account = std::make_unique<AccountClient>();
    m_client_file    = std::make_unique<FileClient   >();
    m_client_message = std::make_unique<MessageClient>();
    m_client_search  = std::make_unique<SearchClient >();
    m_client_status  = std::make_unique<StatusClient >();
    m_client_sync    = std::make_unique<SyncClient   >();

    // 将客户端连接到共享的 gRPC HTTP/2 通道
    if (!m_client_account->attachChannel(m_channel)) { qDebug().nospace().noquote() << "attach AccountService Channel failed."; return false; }
    if (!m_client_file   ->attachChannel(m_channel)) { qDebug().nospace().noquote() << "attach FileService Channel failed."   ; return false; }
    if (!m_client_message->attachChannel(m_channel)) { qDebug().nospace().noquote() << "attach MessageService Channel failed."; return false; }
    if (!m_client_search ->attachChannel(m_channel)) { qDebug().nospace().noquote() << "attach SearchService Channel failed." ; return false; }
    if (!m_client_status ->attachChannel(m_channel)) { qDebug().nospace().noquote() << "attach StatusService Channel failed." ; return false; }
    if (!m_client_sync   ->attachChannel(m_channel)) { qDebug().nospace().noquote() << "attach SyncService Channel failed."   ; return false; }

    m_inited = true;

    return true;
}

// bool NetworkClient::reset_connection() {
//     m_inited = false;
//     m_channel       .reset();
//     m_client_account.reset();
//     m_client_file   .reset();
//     m_client_message.reset();
//     m_client_search .reset();
//     m_client_status .reset();
//     m_client_sync   .reset();
//     m_client_sync   .reset();
//     init();
//     return true;
// }

bool NetworkClient::disconnect_sync(const QString& account_id) {
    if (!m_cache_connection_syncs.contains(account_id)) { return false; }
    if (!m_cache_connection_syncs[account_id]) { return false; }

    auto&stream = m_cache_connection_syncs[account_id];

    // 1. 断开该 Stream 上的所有信号槽（切断回调，防止退出后还会触发 messageReceived）
    stream->disconnect();

    // 2. 尝试关闭/取消底层 gRPC/Qt-Grpc 流通信
    // 注：QGrpcStream 自身通常提供 cancel() 或 close() 方法
    stream->cancel(); // 或者 m_activeStream->close(); 依据你使用的封装库而定

    // 3. 投递延时销毁（销毁底层 socket 并安全释放内存）
    stream->deleteLater();

    // 4. 将成员指针置空，防止二次操作
    stream = nullptr;

    m_cache_connection_syncs.erase(account_id);

    return true;
}

// 成功失败都要返回，失败时要返回错误原因
bool NetworkClient::signup(signup_params params, signup_callback callback) {
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
    do {
        Result result(ResultWrapper{.error_code = ErrorCode::ERROR_UNSET}()); // 默认设为错误
        // 参数简单校验
        // ===============                                                    //
        ;    if (user_id  .isEmpty()) { result = ResultWrapper{.error_code = ErrorCode::USER_ID_EMPTY             }(); }
        else if (password .isEmpty()) { result = ResultWrapper{.error_code = ErrorCode::PASSWORD_EMPTY            }(); }
        else if (confirm  .isEmpty()) { result = ResultWrapper{.error_code = ErrorCode::CONFIRM_PASSWORD_EMPTY    }(); }
        else if (question .isEmpty()) { result = ResultWrapper{.error_code = ErrorCode::SECURITY_QUESTION_EMPTY   }(); }
        else if (answer   .isEmpty()) { result = ResultWrapper{.error_code = ErrorCode::SECURITY_ANSWER_EMPTY     }(); }
        else if (password != confirm) { result = ResultWrapper{.error_code = ErrorCode::DIFF_PASSWORD_WITH_CONFIRM}(); }
        else if (!m_client_account  ) { result = ResultWrapper{.error_code = ErrorCode::ERROR_INIT                }(); }
        //                                                    =============== //
        // 失败时也要发送通知
        if (result.hasErrorCode() && result.errorCode() != ErrorCode::ERROR_UNSET) {
            ReplyOpt reply_opt = Reply();
            reply_opt->setResult(result);
            if (callback) { callback(params, reply_opt); }
            return false;
        }
        break;
    } while (false);
    // 业务逻辑
    do {
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
                if (status.isOk()) { reply_opt = reply_conn->read<Reply>(); }
                // 发送参数，返回参数
                // 1. 返回参数为 nullopt 时代表返回错误，仅提示失败即可？？
                // 2. 即使失败也要调用会回调，以便通知界面
                if (callback) { callback(params, reply_opt); }
            }, Qt::SingleShotConnection);
        break;
    } while (false);
    return true;
}

// 成功失败都要返回，失败时要返回错误原因
bool NetworkClient::signin(signin_params params, signin_callback callback) {
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
    do {
        Result result(ResultWrapper{.error_code = ErrorCode::ERROR_UNSET}()); // 默认设为错误
        // 参数简单校验
        // ===============                                                    //
        ;    if (account_id.isEmpty() && user_id.isEmpty()) { result = ResultWrapper{.error_code = ErrorCode::AUTH_ID_EMPTY    }(); }
        else if (password  .isEmpty() && token  .isEmpty()) { result = ResultWrapper{.error_code = ErrorCode::AUTH_METHOD_EMPTY}(); }
        else if (!m_client_account                        ) { result = ResultWrapper{.error_code = ErrorCode::ERROR_INIT       }(); }
        //                                                    =============== //
        // 失败时也要发送通知
        if (result.hasErrorCode() && result.errorCode() != ErrorCode::ERROR_UNSET) {
            ReplyOpt reply_opt = Reply();
            reply_opt->setResult(result);
            if (callback) { callback(params, reply_opt); }
            return false;
        }
        break;
    } while (false);
    // 业务逻辑
    do {
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
                if (status.isOk()) { reply_opt = reply_conn->read<Reply>(); }
                if (callback) { callback(params, reply_opt); }
            }, Qt::SingleShotConnection);
        break;
    } while (false);
    return true;
}

// 精准匹配，不返回包含查询词的内容
bool NetworkClient::search_user (search_user_params params, search_user_callback callback) {
    using namespace Utils::Wrapper;
    using Request  = SearchUserRequest;
    using Reply    = SearchUserReply;
    using ReplyOpt = std::optional<Reply>;
    // 有用的参数
    QString user_keyword = params->userId();
    // ====================================================================== //
    // 参数校验
    do {
        Result result(ResultWrapper{.error_code = ErrorCode::ERROR_UNSET}()); // 默认设为错误
        // 参数简单校验
        // ===============                                                    //
        ;    if (user_keyword.isEmpty()) { result = ResultWrapper{.error_code = ErrorCode::TARGET_USER_ID_EMPTY}(); }
        else if (!m_client_search      ) { result = ResultWrapper{.error_code = ErrorCode::ERROR_INIT          }(); }
        //                                                    =============== //
        // 失败时也要发送通知
        if (result.hasErrorCode() && result.errorCode() != ErrorCode::ERROR_UNSET) {
            ReplyOpt reply_opt = Reply();
            reply_opt->setResult(result);
            if (callback) { callback(params, reply_opt); }
            return false;
        }
        break;
    } while (false);
    // 业务逻辑
    do {
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
                if (status.isOk()) { reply_opt = reply_conn->read<Reply>(); }
                // 发送参数，返回参数
                // 1. 返回参数为 nullopt 时代表返回错误，仅提示失败即可？？
                // 2. 即使失败也要调用会回调，以便通知界面
                if (callback) { callback(params, reply_opt); }
            }, Qt::SingleShotConnection);
        break;
    } while (false);
    return true;
}

// 更新个人资料，服务端支持同时修改头像 ID，不包括鉴权信息
bool NetworkClient::update_profile(update_profile_params params, update_profile_callback callback) {
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
    do {
        Result result(ResultWrapper{.error_code = ErrorCode::ERROR_UNSET}()); // 默认设为错误
        // 参数简单校验
        // ===============                                                    //
        ;    if (account_id.isEmpty() && user_id.isEmpty()) { result = ResultWrapper{.error_code = ErrorCode::AUTH_ID_EMPTY    }(); }
        else if (password  .isEmpty() && token  .isEmpty()) { result = ResultWrapper{.error_code = ErrorCode::AUTH_METHOD_EMPTY}(); }
        //                                                    =============== //
        // 失败时也要发送通知
        if (result.hasErrorCode() && result.errorCode() != ErrorCode::ERROR_UNSET) {
            ReplyOpt reply_opt = Reply();
            reply_opt->setResult(result);
            if (callback) { callback(params, reply_opt); }
            return false;
        }
        break;
    } while (false);
    // 业务逻辑
    do {
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
                if (status.isOk()) { reply_opt = reply_conn->read<Reply>(); }
                // 发送参数，返回参数
                // 1. 返回参数为 nullopt 时代表返回错误，仅提示失败即可？？
                // 2. 即使失败也要调用会回调，以便通知界面
                if (callback) { callback(params, reply_opt); }
            }, Qt::SingleShotConnection);
        break;
    } while (false);
    return true;
}

// 获取文件上传 ID
bool NetworkClient::get_file_id(get_file_id_params params, get_file_id_callback callback) {
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
    do {
        Result result(ResultWrapper{.error_code = ErrorCode::ERROR_UNSET}()); // 默认设为错误
        // 参数简单校验
        // ===============                                                    //
        ;    if (account_id.isEmpty() && user_id.isEmpty()) { result = ResultWrapper{.error_code = ErrorCode::AUTH_ID_EMPTY    }(); }
        else if (password  .isEmpty() && token  .isEmpty()) { result = ResultWrapper{.error_code = ErrorCode::AUTH_METHOD_EMPTY}(); }
        else if (file_name .isEmpty()                     ) { result = ResultWrapper{.error_code = ErrorCode::FILE_NAME_EMPTY  }(); }
        else if (file_size  <= 0                          ) { result = ResultWrapper{.error_code = ErrorCode::FILE_SIZE_INVALID}(); }
        else if (file_md5  .isEmpty()                     ) { result = ResultWrapper{.error_code = ErrorCode::FILE_MD5_EMPTY   }(); }
        else if (!m_client_file                           ) { result = ResultWrapper{.error_code = ErrorCode::ERROR_INIT       }(); }
        //                                                    =============== //
        // 失败时也要发送通知
        if (result.hasErrorCode() && result.errorCode() != ErrorCode::ERROR_UNSET) {
            ReplyOpt reply_opt = Reply();
            reply_opt->setResult(result);
            if (callback) { callback(params, reply_opt); }
            return false;
        }
        break;
    } while (false);
    // 业务逻辑
    do {
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
                if (status.isOk()) { reply_opt = reply_conn->read<Reply>(); }
                // 发送参数，返回参数
                // 1. 返回参数为 nullopt 时代表返回错误，仅提示失败即可？？
                // 2. 即使失败也要调用会回调，以便通知界面
                if (callback) { callback(params, reply_opt); }
            }, Qt::SingleShotConnection);
        break;
    } while (false);
    return true;
}

bool NetworkClient::contact_request(contact_request_params params, contact_request_callback callback) {
    using namespace Utils::Wrapper;
    using Request  = ContactRequestRequest;
    using Reply    = ContactRequestReply;
    using ReplyOpt = std::optional<Reply>;
    // 有用的参数
    const AuthRequest auth = (params && params->hasRequestUserAuth()) ? params->requestUserAuth() : AuthRequest();
    QString account_id = auth.hasAccountId() ? auth.accountId() : QString();
    QString user_id    = auth.hasUserId   () ? auth.userId   () : QString();
    QString password   = auth.hasPassword () ? auth.password () : QString();
    QString token      = auth.hasToken    () ? auth.token    () : QString();
    QString target_user_id = params->targetUserId();
    // ====================================================================== //
    // 参数校验
    do {
        Result result(ResultWrapper{.error_code = ErrorCode::ERROR_UNSET}()); // 默认设为错误
        // 参数简单校验
        // ===============                                                    //
        ;    if (account_id.isEmpty() && user_id.isEmpty()) { result = ResultWrapper{.error_code = ErrorCode::AUTH_ID_EMPTY       }(); }
        else if (password  .isEmpty() && token  .isEmpty()) { result = ResultWrapper{.error_code = ErrorCode::AUTH_METHOD_EMPTY   }(); }
        else if (target_user_id.isEmpty()                 ) { result = ResultWrapper{.error_code = ErrorCode::TARGET_USER_ID_EMPTY}(); }
        else if (!m_client_account                        ) { result = ResultWrapper{.error_code = ErrorCode::ERROR_INIT          }(); }
        //                                                    =============== //
        // 失败时也要发送通知
        if (result.hasErrorCode() && result.errorCode() != ErrorCode::ERROR_UNSET) {
            ReplyOpt reply_opt = Reply();
            reply_opt->setResult(result);
            if (callback) { callback(params, reply_opt); }
            return false;
        }
        break;
    } while (false);
    // 业务逻辑
    do {
        // ===============                                                    //
        std::unique_ptr<QGrpcCallReply> reply_conn = m_client_account->ContactRequest(*params, m_grpc_call_options);
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
                    if (status.isOk()) { reply_opt = reply_conn->read<Reply>(); }
                    if (callback) { callback(params, reply_opt); }
                }, Qt::SingleShotConnection);
        break;
    } while (false);
    return true;
}

bool NetworkClient::contact_reply(contact_reply_params params, contact_reply_callback callback) {
    using namespace Utils::Wrapper;
    using Request  = ContactReplyRequest;
    using Reply    = ContactReplyReply;
    using ReplyOpt = std::optional<Reply>;
    // 有用的参数
    const AuthRequest auth = (params && params->hasRequestUserAuth()) ? params->requestUserAuth() : AuthRequest();
    QString account_id = auth.hasAccountId() ? auth.accountId() : QString();
    QString user_id    = auth.hasUserId   () ? auth.userId   () : QString();
    QString password   = auth.hasPassword () ? auth.password () : QString();
    QString token      = auth.hasToken    () ? auth.token    () : QString();
    QString target_user_id = params->targetUserId();
    // ====================================================================== //
    // 参数校验
    do {
        Result result(ResultWrapper{.error_code = ErrorCode::ERROR_UNSET}()); // 默认设为错误
        // 参数简单校验
        // ===============                                                    //
        ;    if (account_id.isEmpty() && user_id.isEmpty()) { result = ResultWrapper{.error_code = ErrorCode::AUTH_ID_EMPTY       }(); }
        else if (password  .isEmpty() && token  .isEmpty()) { result = ResultWrapper{.error_code = ErrorCode::AUTH_METHOD_EMPTY   }(); }
        else if (target_user_id.isEmpty()                 ) { result = ResultWrapper{.error_code = ErrorCode::TARGET_USER_ID_EMPTY}(); }
        else if (!m_client_account                        ) { result = ResultWrapper{.error_code = ErrorCode::ERROR_INIT          }(); }
        //                                                    =============== //
        // 失败时也要发送通知
        if (result.hasErrorCode() && result.errorCode() != ErrorCode::ERROR_UNSET) {
            ReplyOpt reply_opt = Reply();
            reply_opt->setResult(result);
            if (callback) { callback(params, reply_opt); }
            return false;
        }
        break;
    } while (false);
    // 业务逻辑
    do {
        // ===============                                                    //
        std::unique_ptr<QGrpcCallReply> reply_conn = m_client_account->ContactReply(*params, m_grpc_call_options);
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
                    if (status.isOk()) { reply_opt = reply_conn->read<Reply>(); }
                    if (callback) { callback(params, reply_opt); }
                }, Qt::SingleShotConnection);
        break;
    } while (false);
    return true;
}

bool NetworkClient::get_contact_request(get_contact_request_params params, get_contact_request_callback callback) {
    return false;
}

bool NetworkClient::get_last_id(get_last_id_params params, get_last_id_callback callback) {
    using namespace Utils::Wrapper;
    using Request  = AuthRequest;
    using Reply    = GetLastIDReply;
    using ReplyOpt = std::optional<Reply>;
    // 有用的参数
    const AuthRequest auth = *params.get(); // (params && params->hasRequestUserAuth()) ? params->requestUserAuth() : AuthRequest();
    QString account_id = auth.hasAccountId() ? auth.accountId() : QString();
    QString user_id    = auth.hasUserId   () ? auth.userId   () : QString();
    QString password   = auth.hasPassword () ? auth.password () : QString();
    QString token      = auth.hasToken    () ? auth.token    () : QString();
    // ====================================================================== //
    // 参数校验
    do {
        Result result(ResultWrapper{.error_code = ErrorCode::ERROR_UNSET}()); // 默认设为错误
        // 参数简单校验
        // ===============                                                    //
        ;    if (account_id.isEmpty() && user_id.isEmpty()) { result = ResultWrapper{.error_code = ErrorCode::AUTH_ID_EMPTY    }(); }
        else if (password  .isEmpty() && token  .isEmpty()) { result = ResultWrapper{.error_code = ErrorCode::AUTH_METHOD_EMPTY}(); }
        else if (!m_client_sync                           ) { result = ResultWrapper{.error_code = ErrorCode::ERROR_INIT       }(); }
        //                                                    =============== //
        // 失败时也要发送通知
        if (result.hasErrorCode() && result.errorCode() != ErrorCode::ERROR_UNSET) {
            ReplyOpt reply_opt = Reply();
            reply_opt->setResult(result);
            if (callback) { callback(params, reply_opt); }
            return false;
        }
        break;
    } while (false);
    // 业务逻辑
    do {
        // ===============                                                    //
        std::unique_ptr<QGrpcCallReply> reply_conn = m_client_sync->GetLastID(*params, m_grpc_call_options);
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
                if (status.isOk()) { reply_opt = reply_conn->read<Reply>(); }
                if (callback) { callback(params, reply_opt); }
            }, Qt::SingleShotConnection);
        break;
    } while (false);
    return true;
}

bool NetworkClient::sync(sync_params params, sync_callback callback) {
    using namespace Utils::Wrapper;
    using Request  = SyncRequest;
    using Reply    = SyncReply;
    using ReplyOpt = std::optional<Reply>;
    using Params   = sync_params;
    //
    using Header   = SyncReplyHeader;
    using Callback = sync_callback;
    using Stream   = QGrpcBidiStream;

    auto&request = params;
    // 先添加认证信息
    if (!request->hasRequestUserAuth()) { return false; }
    auto auth = request->requestUserAuth();
    QString account_id = auth.hasAccountId() ? auth.accountId() : QString();
    QString user_id    = auth.hasUserId   () ? auth.userId   () : QString();
    QString password   = auth.hasPassword () ? auth.password () : QString();
    QString token      = auth.hasToken    () ? auth.token    () : QString();
    // ====================================================================== //
    // 参数校验
    do {
        Result result(ResultWrapper{.error_code = ErrorCode::ERROR_UNSET}()); // 默认设为错误
        // 参数简单校验
        // ===============                                                    //
        ;    if (account_id.isEmpty() && user_id.isEmpty()) { result = ResultWrapper{.error_code = ErrorCode::AUTH_ID_EMPTY    }(); }
        else if (password  .isEmpty() && token  .isEmpty()) { result = ResultWrapper{.error_code = ErrorCode::AUTH_METHOD_EMPTY}(); }
        else if (!m_client_sync                           ) { result = ResultWrapper{.error_code = ErrorCode::ERROR_INIT       }(); }
        //                                                    =============== //
        // 失败时也要发送通知
        if (result.hasErrorCode() && result.errorCode() != ErrorCode::ERROR_UNSET) {
            ReplyOpt reply_opt = Reply();
            Header header;
            header.setRequest(*params.get());
            header.setResult(result);
            reply_opt->setHeader(header);
            if (callback) { callback(params, reply_opt); }
            return false;
        }
        break;
    } while (false);
    // 网络操作
    // 业务逻辑
    do {
        std::unique_ptr<Stream> reply_conn = m_client_sync->Sync(*request.get(), m_grpc_call_options);
        //                                                    =============== //
        const auto *rep_conn_ptr = reply_conn.get();  // 必须
        if (!rep_conn_ptr) {
            ReplyOpt reply_opt = Reply();
            Header header;
            header.setRequest(*params.get());
            Result result(ResultWrapper{.error_code = ErrorCode::ERROR_CONN}()); // 网络错误
            header.setResult(result);
            reply_opt->setHeader(header);
            if (callback) { callback(params, reply_opt); }
            return false;
        }

        Stream* raw_stream_ptr = reply_conn.release(); // 释放所有权，由信号槽挂载生命周期
        m_cache_connection_syncs[account_id] = raw_stream_ptr;

        // QPointer 是专门为 QObject 及其子类设计的智能指针。当其指向的 QObject 被真正析构时，QPointer 会自动将自己重置为 nullptr。

        // 收到数据时触发的消息
        // 服务器每次 while 写数据到客户端都会触发一次
        // 监听服务器下发的数据 (长连接核心)
        QPointer<Stream> safeStreamPtr = raw_stream_ptr;
        connect(raw_stream_ptr, &Stream::messageReceived, this, [safeStreamPtr, params, callback]() {
            if (!safeStreamPtr) { return; }

            // 读取服务器推过来的长连接通知
            std::optional<Reply> reply = safeStreamPtr->read<Reply>();
            if (reply && callback) {
                // 将数据异步回调给外面
                callback(params, reply);
            }
        });

        connect(raw_stream_ptr, &Stream::finished, raw_stream_ptr, [this, raw_stream_ptr, account_id](const QGrpcStatus &status) {
            if (m_cache_connection_syncs.contains(account_id)) { m_cache_connection_syncs.erase(account_id); }
            raw_stream_ptr->deleteLater(); // 优雅销毁流对象
        });
        break;
    } while (false);
    return true;
}

bool NetworkClient::send_message(send_message_params params, send_message_callback callback) {
    using namespace Utils::Wrapper;
    using Request  = MessageContext;
    using Reply    = MessageReply;
    using ReplyOpt = std::optional<Reply>;
    // 有用的参数
    const AuthRequest auth = (params && params->hasRequestUserAuth()) ? params->requestUserAuth() : AuthRequest();
    QString account_id = auth.hasAccountId() ? auth.accountId() : QString();
    QString user_id    = auth.hasUserId   () ? auth.userId   () : QString();
    QString password   = auth.hasPassword () ? auth.password () : QString();
    QString token      = auth.hasToken    () ? auth.token    () : QString();
    // ====================================================================== //
    // 参数校验
    do {
        Result result(ResultWrapper{.error_code = ErrorCode::ERROR_UNSET}()); // 默认设为错误
        // 参数简单校验
        // ===============                                                    //
        ;    if (account_id.isEmpty() && user_id.isEmpty()) { result = ResultWrapper{.error_code = ErrorCode::AUTH_ID_EMPTY           }(); }
        else if (password  .isEmpty() && token  .isEmpty()) { result = ResultWrapper{.error_code = ErrorCode::AUTH_METHOD_EMPTY       }(); }
        // else if (!params->hasShortText               ()) { result = ResultWrapper{.error_code = ErrorCode::MESSAGE_TEXT_EMPTY      }(); }
        else if (!params->hasAccountId                  ()) { result = ResultWrapper{.error_code = ErrorCode::TARGET_ACCOUNT_ID_EMPTY }(); }
        else if ( params->clientMessageId().isEmpty     ()) { result = ResultWrapper{.error_code = ErrorCode::CONTEXT_REQUIRED        }(); }
        else if (!m_client_message                        ) { result = ResultWrapper{.error_code = ErrorCode::ERROR_INIT              }(); }
        else if (!params->hasTimestamp() || !params->timestamp().hasTimestamp()) { result = ResultWrapper{.error_code = ErrorCode::CONTEXT_REQUIRED }(); }
        //                                                    =============== //
        // 失败时也要发送通知
        if (result.hasErrorCode() && result.errorCode() != ErrorCode::ERROR_UNSET) {
            ReplyOpt reply_opt = Reply();
            reply_opt->setResult(result);
            if (callback) { callback(params, reply_opt); }
            return false;
        }
        break;
    } while (false);
    // 业务逻辑
    do {
        // ===============                                                    //
        std::unique_ptr<QGrpcCallReply> reply_conn = m_client_message->TTCSendMessage(*params.get(), m_grpc_call_options);
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
                if (status.isOk()) { reply_opt = reply_conn->read<Reply>(); }
                if (callback) { callback(params, reply_opt); }
            }, Qt::SingleShotConnection);
        break;
    } while (false);
    return true;
}

bool NetworkClient::get_file_id(const QString&file_path, network_file_io_notify_callback callback_get_file_id_notiify) {
    using Wrapper  = Utils::Wrapper::FileMetadataWrapper;
    using Notify   = Network::FileDownloadNotify;
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
    auto& notify = get_file_id_args->notify;
    notify.path  = file_path;
    if (file_path.isEmpty()) {
        notify.finished = true;
        notify.ok       = false;
        notify.code     = int64_t(ErrorCode::FILE_NAME_EMPTY);
        notify.message  = "file path empty.";
        callback_get_file_id_notiify(notify);
        return true;
    }
    auto file_info = QFileInfo(file_path);
    auto file_name = file_info.fileName();
    auto file_size = file_info.size();
    auto file_md5  = Utils::Algorithm::calculate_file_md5(file_path);
    // 可能部分操作成功
    if (file_name.isEmpty() || file_size < 0 || file_md5.isEmpty()) {
        notify.finished = true;
        notify.ok       = false;
        notify.code     = int64_t(ErrorCode::FILE_READ_ERROR);
        notify.message  = "file not exists.";
        notify.size     = file_size;
        notify.md5      = file_md5;
        callback_get_file_id_notiify(notify);
        return true;
    }
    // 用于参数传递，可避免多次 md5 同一文件
    notify.name = file_name;
    notify.size = file_size;
    notify.md5  = file_md5;
    // 构造参数
    get_file_id_params params_get_file_id = std::make_shared<GetFileIDRequest>();
    params_get_file_id->setRequestUserAuth(*Accounts::instance()->get_auth_request().get());
    params_get_file_id->setFileMetadata(Wrapper{.file_name = file_name, .file_size = file_size, .file_md5 = file_md5}());

    // qDebug().noquote().nospace() << Utils::Transform::to_string(params_get_file_id->requestUserAuth());

    notify.finished = false;  // 防止提前结束

    get_file_id_callback callback_get_file_id = [callback_get_file_id_notiify, get_file_id_args](get_file_id_params request, get_file_id_reply reply) {
        auto& notify = get_file_id_args->notify;
        // 网络超时或其他错误导致没有收到回复
        if (reply == std::nullopt || !reply->hasResult()) {
            notify.finished = true;
            notify.ok       = false;
            notify.code     = int64_t(ErrorCode::ERROR_CONN);
            notify.message  = "No reply or no result.";
            callback_get_file_id_notiify(notify);
            return;
        }
        //
        notify.finished = true;
        notify.ok       = reply->result().hasSuccessCode();
        notify.code     = notify.ok ? int64_t(reply->result().successCode()) : int64_t(reply->result().errorCode());
        notify.message  = reply->result().message();
        notify.id       = reply->fileId    ();
        notify.status   = reply->fileStatus();
        callback_get_file_id_notiify(notify);
    };
    get_file_id(params_get_file_id, callback_get_file_id);
    return true;
}

bool NetworkClient::upload_file(const QString &file_path, network_file_io_notify_callback callback_upload_file_notify) {
    using namespace Utils::Wrapper;
    using Request  = FileUploadRequest;
    using Reply    = FileUploadReply;
    using ReplyOpt = std::optional<Reply>;
    using Params   = file_upload_params;
    //
    using Header   = FileUploadHeader;
    using Notify   = Network::FileDownloadNotify;
    using Callback = network_file_io_notify_callback;
    using Stream   = QGrpcBidiStream;

    Callback callback_get_file_id_notify = [this, callback_upload_file_notify](const Notify notify_get_file_id) {
        // 需要传出的函数
        struct FileUploadArgs {
            Notify notify;
            std::shared_ptr<QFile> file_upload;  // 用于传递文件句柄
            // FileDownloadHeader header;
            bool initialized  = false;  // 已经完成初始化的标记  // 接收到的是文件上传信息还是结果
            // bool error_occued = false;  // 是否已经发生错误
            // QString download_path;
            // QString real_md5;
        };
        std::shared_ptr<FileUploadArgs> file_upload_args = std::make_shared<FileUploadArgs>();
        auto& notify = file_upload_args->notify;
        notify = notify_get_file_id;  // 复制，便于修改

        // 获取文件上传 ID 错误
        if (!notify.finished || !notify.ok || notify.id.isEmpty()) {
            callback_upload_file_notify(notify);
            return;
        }

        // 极速上传/服务器上已经有相同的文件了
        if ( notify.status == FileStatusCode::FILE_STATUS_UPLOADED) {
            notify.finished = true;
            notify.ok       = true;
            notify.code     = int64_t(SuccessCode::SUCCESS);
            notify.message  = "File already uploaded.";
            callback_upload_file_notify(notify);
            return;
        }

        // =================================================================== //
        // 业务逻辑
        Request reuest;
        Header  header;
        header.setRequestUserAuth(*Accounts::instance()->get_auth_request().get());
        header.setFileId(notify.id);
        // header.setReason("upload file");
        reuest.setHeader(header);
        std::unique_ptr<Stream> reply_conn = m_client_file->FileUpload(reuest, m_grpc_call_options);
        //                                                    =============== //
        const auto *rep_conn_ptr = reply_conn.get();  // 必须
        if (!rep_conn_ptr) {
            notify.finished = true;
            notify.code     = int64_t(ErrorCode::ERROR_CONN);
            notify.message  = "Error in connect.";
            callback_upload_file_notify(notify);
            return;
        }

        notify.finished = false;  // 防止提前结束

        // 收到数据时触发的消息  // 服务器每次 while 写数据到客户端都会触发一次
        connect(rep_conn_ptr, &Stream::messageReceived, this,
            [reply_conn = std::move(reply_conn), file_upload_args, callback_upload_file_notify]() {
                auto& notify = file_upload_args->notify;
                // 读取失败
                auto reply = reply_conn->read<FileUploadReply>();
                if (!reply) {
                    notify.finished = true;
                    notify.code     = int64_t(ErrorCode::ERROR_RECV);
                    notify.message  = "Error in received data.";
                    callback_upload_file_notify(notify);
                    return;
                }

                // 发生错误或上传完成时已经通知，后续接收到数据时忽略即可，不必通知
                if (notify.finished) {
                    return;
                }

                // 获取文件信息错误
                if (!reply->hasResult() || !reply->result().hasSuccessCode()) {
                    notify.finished = true;
                    notify.code     = int64_t(ErrorCode::FILE_UPLOAD_ERROR);
                    notify.message  = "Result return error.";
                    callback_upload_file_notify(notify);
                    return;
                }

                // 文件已经上传完成则直接返回成功
                FileStatusCode file_status = reply->fileStatus();
                if (file_status == FileStatusCode::FILE_STATUS_UPLOADED) {
                    notify.finished = true;
                    notify.message  = "File already uploaded.";
                    notify.status   = file_status;
                    callback_upload_file_notify(notify);
                    return;
                }

                // 如果不是获取文件信息时返回的文件上传失败消息，
                // 那么就是文件上传完成后服务器返回的上传结果，
                // 此时的文件上传失败就是最终结果
                else if (file_status != FileStatusCode::FILE_STATUS_UPLOADING && file_upload_args->initialized) {
                    notify.finished = true;
                    notify.code     = int64_t(ErrorCode::FILE_UPLOAD_ERROR);
                    notify.message  = "File upload failed.";
                    notify.status   = file_status;
                    callback_upload_file_notify(notify);
                    return;
                }

                // 文件头  // 判断是否需要上传及是否进行文件位置调整
                // 校验已上传大小是否合法
                notify.uploaded_bytes = reply->uploadedBytes();  // 已经上传的字节数，支持断点续传时有用
                notify.uploaded_bytes = std::max<qint64>(0, notify.uploaded_bytes);
                if (notify.uploaded_bytes > notify.size) {
                    notify.finished = true;
                    notify.code     = int64_t(ErrorCode::FILE_READ_ERROR);
                    notify.message  = "Uploaded bytes invalid.";
                    callback_upload_file_notify(notify);
                    return;
                }

                // 发送文件内容

                file_upload_args->file_upload = std::make_shared<QFile>(notify.path);
                // 打开文件
                if (!file_upload_args->file_upload->open(QIODevice::ReadOnly)) {
                    notify.finished = true;
                    notify.code     = int64_t(ErrorCode::FILE_READ_ERROR);
                    notify.message  = tr("open file error.");
                    callback_upload_file_notify(notify);
                    return;
                }

                // 设置文件信息，进行断点续传
                int64_t upload_size = std::max<qint64>(0, notify.uploaded_bytes);
                if (!file_upload_args->file_upload->seek(upload_size)) {
                    file_upload_args->file_upload->close();
                    notify.finished = true;
                    notify.code     = int64_t(ErrorCode::FILE_READ_ERROR);
                    notify.message  = tr("Failed to seek upload file");
                    callback_upload_file_notify(notify);
                    return;
                }

                // 发送文件内容
                const size_t chunk_size = 128 * 1024;  // 128KB
                while (!file_upload_args->file_upload->atEnd()) {
                    QByteArray chunk_data = file_upload_args->file_upload->read(chunk_size);
                    qint64 bytes_read = chunk_data.size();

                    // 读取数据失败时
                    if (bytes_read < 0) {
                        file_upload_args->file_upload->close();
                        notify.finished = true;
                        notify.code     = int64_t(ErrorCode::FILE_READ_ERROR);
                        notify.message  = tr("Failed to read upload file");
                        callback_upload_file_notify(notify);
                        return;
                    }

                    // 文件读取完成
                    if (bytes_read == 0) {
                        break;
                    }

                    // 未上传完成时需要发送数据给服务器
                    Request request;
                    FileChunkData file_chunk_data;
                    file_chunk_data.setChunkOffset(upload_size);
                    file_chunk_data.setChunkData(chunk_data);
                    request.setChunk(file_chunk_data);
                    reply_conn->writeMessage(request);
                    upload_size += bytes_read;

                    // 文件上传进度通知
                    notify.finished = false;
                    notify.ok       = true;
                    notify.code     = int64_t(SuccessCode::SUCCESS);
                    notify.message  = tr("File uploading...");
                    // notify.id    = notify.id  ;  // 获取 ID 时已经设置
                    notify.status   = FileStatusCode::FILE_STATUS_UPLOADING;
                    // notify.path  = notify.path;  // 获取 ID 时已经设置
                    // notify.name  = notify.name;  // 获取 ID 时已经设置
                    // notify.size  = notify.size;  // 获取 ID 时已经设置
                    // notify.md5   = notify.md5 ;  // 获取 ID 时已经设置
                    notify.uploaded_bytes = upload_size;
                    callback_upload_file_notify(notify);
                }

                file_upload_args->file_upload->close();
                reply_conn->writesDone();
            }
        );
    };
    get_file_id(file_path, callback_get_file_id_notify);
    return true;
}

class FileDownloadRecords {
public:
    bool add_record(const QString &file_id) {
        QMutexLocker locker(&m_mutex);
        if ( m_records.contains(file_id)) { return false; }
        m_records.insert(file_id);
        // qDebug().noquote().nospace() << "trace message file download." << "FileDownloadRecords::add_record " << file_id;
        return true;
    };
    bool remove_reord(const QString &file_id) {
        QMutexLocker locker(&m_mutex);
        // qDebug().noquote().nospace() << "trace message file download." << "FileDownloadRecords::remove_reord " << file_id;
        return m_records.remove(file_id);
    };
public:
    QSet<QString> m_records;
    QMutex m_mutex;
};

// =========================================================================== //
// 文件下载：整条链路放进专用子线程
// --------------------------------------------------------------------------- //
// 关键点：
//   1. QGrpcHttp2Channel 底层是 QNetworkAccessManager(QObject)，QGrpcClientBase、
//      QGrpcServerStream 也都是 QObject，线程亲和性绑定创建它们的线程。
//      NetworkClient::m_channel / m_client_file 属于主线程，绝不能跨线程复用，
//      因此子线程里必须新建一套独立的 channel + client。
//   2. gRPC 的建连、写请求、readyRead，以及 messageReceived / finished 全部靠
//      Qt 事件队列分发，所以线程必须有事件循环。默认 QThread::run() 会 exec()，
//      而 QThread::create() 生成的线程不会 exec()，不能用。
//   3. Accounts / Database / user_cache_path() 等单例只在主线程访问：
//      下载前把参数准备好，下载中的写库与通知排队回主线程。
//   4. 每个任务一个线程，任务结束 quit() + deleteLater 自动回收，互不阻塞。
// =========================================================================== //
namespace {

// 程序退出时统一收尾，避免 "QThread: Destroyed while thread is still running"
static QMutex                    g_download_threads_mutex;
static QList<QPointer<QThread>>  g_download_threads;

static void shutdown_download_threads() {
    QList<QPointer<QThread>> snapshot;
    {
        QMutexLocker locker(&g_download_threads_mutex);
        snapshot = g_download_threads;
    }
    for (const QPointer<QThread> &p : snapshot) {
        if (!p) { continue; }
        p->quit();
        p->wait(5000);
    }
}

static void register_download_thread(QThread *thread) {
    static const bool s_registered = []() {
        qAddPostRoutine(&shutdown_download_threads);
        return true;
    }();
    (void)s_registered;
    QMutexLocker locker(&g_download_threads_mutex);
    for (qsizetype i = g_download_threads.size() - 1; i >= 0; --i) {
        if (g_download_threads.at(i).isNull()) {
            g_download_threads.removeAt(i);
        }
    }
    g_download_threads.append(QPointer<QThread>(thread));
}

class DownloadWorker : public QObject {
public:
    using Notify   = Network::FileDownloadNotify;
    using Callback = network_file_io_notify_callback;
    using Stream   = QGrpcServerStream;
    using BytesMd5 = Utils::Algorithm::BytesMd5;

    // 全部是值类型 / 裸指针，可安全跨线程传递；不含任何 QObject 实例
    struct Params {
        QString             file_id;
        QUrl                server_url;
        QGrpcChannelOptions channel_options;
        QGrpcCallOptions    call_options;
        FileDownloadRequest request;        // 主线程已填好 auth / fileId / downloadedBytes
        QString             cache_dir;      // Utils::user_cache_path()，主线程算好
        QString             download_dir;   // Utils::user_download_path()，主线程算好
        QString             saved_path;     // Database 中记录的本地路径
        qint64              downloaded_size = 0;
        QThread            *thread          = nullptr;
        Callback            callback;
    };

    // 只能通过 QMetaObject::invokeMethod(..., Qt::QueuedConnection) 调用，
    // 保证 channel / client / stream 都在子线程内创建
    void start(Params params) {
        m_params = std::move(params);
        m_thread = m_params.thread;

        m_notify.id   = m_params.file_id;
        m_notify.path = m_params.saved_path;

        // // 有下载记录时先打开已有文件（与原实现一致：打开失败直接结束）
        // if (!m_notify.path.isEmpty()) {
        //     Utils::create_dirs(Utils::get_dir(m_notify.path));
        //     m_file = std::make_shared<QFile>(m_notify.path);
        //     if (!m_file->open(QIODevice::ReadOnly)) {
        //         fail(ErrorCode::FILE_READ_ERROR, NetworkClient::tr("Failed to open file for md5 calculation: %1").arg(m_file->errorString()));
        //         return;
        //     }
        // }

        // 【核心】channel 与 client 必须在本线程创建，不能复用主线程的共享实例
        m_channel = std::make_shared<QGrpcHttp2Channel>(m_params.server_url, m_params.channel_options);
        m_client  = std::make_unique<FileClient>();     // 不带 parent，亲和本线程
        if (!m_client->attachChannel(m_channel)) {
            fail(ErrorCode::ERROR_CONN, "Error in connect.");
            return;
        }

        std::unique_ptr<Stream> reply = m_client->FileDownload(m_params.request, m_params.call_options);
        m_stream = std::shared_ptr<Stream>(std::move(reply));
        if (!m_stream) {
            fail(ErrorCode::ERROR_CONN, "Error in connect.");
            return;
        }
        m_notify.finished = false;  // 防止提前结束

        // connect(m_stream.get(), &Stream::messageReceived, this, &DownloadWorker::on_message);
        connect(m_stream.get(), &Stream::messageReceived, this, [this](){ on_message(); });
        // finished 定义在基类 QGrpcOperation 上
        // connect(m_stream.get(), &QGrpcOperation::finished, this, &DownloadWorker::on_finished, Qt::SingleShotConnection);
        connect(m_stream.get(), &QGrpcOperation::finished, this, [this](const QGrpcStatus &status){ on_finished(status); }, Qt::SingleShotConnection);
    }

private:
    // 所有主线程任务都投递到 NetworkClient 单例（亲和主线程）。
    // 同一个 receiver 的 queued 调用是 FIFO，写库一定早于完成通知。
    template <class Functor>
    void on_main(Functor task) const {
        QMetaObject::invokeMethod(NetworkClient::instance(), task, Qt::QueuedConnection);
    }

    void emit_notify(const Notify &value) const {
        if (value.uploaded_bytes < 0) {
            qDebug().noquote().nospace() << "uploaded_bytes : " << value.uploaded_bytes;
        }
        Callback cb = m_params.callback;
        if (!cb) { return; }
        on_main([cb, value]() { cb(value); });
    }

    void save_path_to_db(const QString &id, const QString &path) const {
        on_main([id, path]() {
            Database::instance()->save_local_file_id_path(id, path);
        });
    }

    void fail(ErrorCode code, const QString &message) {
        Notify n    = m_notify;
        n.finished  = true;
        n.ok        = false;
        n.code      = int64_t(code);
        n.message   = message;
        emit_notify(n);
        stop();
    }

    // 断开信号避免退出前再次触发，然后让主线程执行 quit()
    void stop() {
        if (m_stopped) { return; }
        m_stopped = true;
        if (m_stream) { m_stream->disconnect(this); }
        QThread *thread = m_thread;
        if (thread) {
            QMetaObject::invokeMethod(thread, &QThread::quit, Qt::QueuedConnection);
        }
    }

    void on_message() {
        if (m_stopped) { return; }
        auto &notify = m_notify;
        // 发生错误时已经通知，后续数据忽略即可
        if (notify.finished) { emit_notify(notify); return; }

        auto reply = m_stream->read<FileDownloadReply>();
        if (!reply) {
            fail(ErrorCode::ERROR_RECV, "Error in received data.");
            return;
        }

        // ---- 第一个包：文件头 ----
        if (!m_initialized) {
            if (!reply->hasHeader()) {
                fail(ErrorCode::ERROR_RECV, "Error in read reply : no header info found.");
                return;
            }
            m_header = reply->header();
            if (!m_header.result().hasSuccessCode()) {
                fail(ErrorCode::FILE_DOWNLOAD_ERROR, "Header return error.");
                return;
            }
            const auto &meta = m_header.fileMetadata();
            notify.name = meta.fileName();
            notify.size = meta.fileSize();
            notify.md5  = meta.fileMd5();
            if (notify.name.isEmpty() || notify.size < 0 || notify.md5.isEmpty()) {
                fail(ErrorCode::FILE_DOWNLOAD_ERROR, "Header's filemeta info not valid.");
                return;
            }

            // 从服务器获取到 metadata 后拼接本地路径并检查是否有文件，
            // 有文件的话就认为下载成功，以防误写入或删除文件，
            // 并使用原始 metadata 通知下载完成
            auto local_stroage_path = Database::get_local_stroage_path(m_params.download_dir, notify.md5, notify.name);
            if (!local_stroage_path.isEmpty() && QFile(local_stroage_path).exists()) {
                m_initialized   = true;  // 初始化完成
                notify.finished = true;
                notify.ok       = true;
                notify.code     = int64_t(SuccessCode::SUCCESS);
                notify.message  = NetworkClient::tr("Local file exists : %1").arg(notify.path);
                notify.status   = FileStatusCode::FILE_STATUS_UPLOADED;
                notify.uploaded_bytes = notify.size;  // 下载成功的通知中已下载大小要与文件大小一致
                emit_notify(notify);
                stop();
                return;
            }

            // 临时文件路径：目录用主线程算好的 cache_dir，写库排队回主线程
            if (notify.path.isEmpty() && !m_file) {
                notify.path = Database::get_local_downloading_path(m_params.cache_dir, notify.md5, notify.name);
                // save_path_to_db(notify.id, notify.path);
                Utils::create_dirs(Utils::get_dir(notify.path));
                m_file = std::make_shared<QFile>(notify.path);
                if (!m_file || !m_file->open(QIODevice::ReadWrite)) {
                    fail(ErrorCode::FILE_WRITE_ERROR, "Open file error.");
                    return;
                }
            }

            // 断点续传：把已有文件的前 offset 字节并入 md5，再截断到该位置
            int64_t file_offset  = m_header.offset();
            int64_t unread_bytes = file_offset;
            m_md5.reset();
            while (m_file && unread_bytes > 0 && !m_file->atEnd()) {
                const size_t chunk_size = 128 * 1024;  // 128KB
                QByteArray chunk_data = m_file->read(chunk_size);
                m_md5.update(chunk_data);
                unread_bytes -= chunk_data.size();
            }
            if (m_file) {
                m_file->resize(m_file->pos());  // 截断，清空后面的内容
            }
            m_initialized = true;  // 初始化完成

            notify.finished = false;
            notify.ok       = true;
            return;  // 第一个包是文件信息，处理完成
        }

        // ---- 数据块：落盘 + 进度通知 ----
        if (!reply->hasChunk()) { return; }
        const auto &chunk_data = reply->chunk().chunkData();
        const qint64 chunk_size = chunk_data.size();
        if (chunk_data.isEmpty()) { return; }

        if (!m_file) { return; }
        const qint64 bytes_written = m_file->write(chunk_data);
        if (bytes_written != chunk_size) {
            fail(ErrorCode::FILE_WRITE_ERROR, QString("Write file chunk error.\n\npath: %1.\nerror message: %2").arg(notify.path).arg(m_file->errorString()));
            return;
        }

        m_md5.update(chunk_data);
        notify.uploaded_bytes = qMax<qint64>(0, notify.uploaded_bytes) + bytes_written;
        notify.finished = false;
        notify.ok       = true;
        notify.code     = int64_t(SuccessCode::SUCCESS);
        notify.message  = "Write chunk data success.";
        notify.status   = FileStatusCode::FILE_STATUS_UPLOADING;
        emit_notify(notify);
    }

    void on_finished(const QGrpcStatus &status) {
        if (m_stopped) { return; }
        auto &notify = m_notify;
        if (m_file && m_file->isOpen()) { m_file->close(); }

        if (!QFile(notify.path).open(QIODevice::ReadWrite)) {
            qDebug().noquote().nospace() << QString("failed to open file %1").arg(notify.path);
        }

        if (!status.isOk()) {
            fail(ErrorCode::ERROR_CONN, "Error in received data.");
            return;
        }

        // 与原实现一致：md5 不匹配时静默结束（注意：此时不会发完成通知）
        if (notify.md5 != m_md5.get_hex_string()) {
            stop();
            return;
        }

        auto local_stroage_path = Database::get_local_stroage_path(m_params.download_dir, notify.md5, notify.name);
        if (!QFile(local_stroage_path).exists() && Utils::rename(notify.path, local_stroage_path)) {
            save_path_to_db(notify.id, local_stroage_path);
            notify.path = local_stroage_path;
        }

        notify.finished = true;
        notify.ok       = true;
        notify.code     = int64_t(SuccessCode::SUCCESS);
        notify.message  = NetworkClient::tr("Download completed: %1").arg(notify.path);
        notify.status   = FileStatusCode::FILE_STATUS_UPLOADED;
        notify.uploaded_bytes = notify.size;  // 下载成功的通知中已下载大小要与文件大小一致
        emit_notify(notify);
        stop();
    }

private:
    Params                            m_params;
    QThread                          *m_thread = nullptr;
    Notify                            m_notify;
    FileDownloadHeader                m_header;
    std::shared_ptr<QFile>            m_file;
    BytesMd5                          m_md5;
    bool                              m_initialized = false;
    bool                              m_stopped     = false;

    // 声明顺序决定析构逆序：stream -> client -> channel，全部在子线程销毁
    std::shared_ptr<QGrpcHttp2Channel> m_channel;
    std::unique_ptr<FileClient>        m_client;
    std::shared_ptr<Stream>            m_stream;
};

}  // namespace

// 不要同时下载一个文件
// true 代表调用成功，不代表下载结果
bool NetworkClient::download_file(const QString &file_id, network_file_io_notify_callback callback_download_file_notify) {
    // ---- 1. 主线程：准备参数（Accounts / Database / 用户路径只在主线程访问） ----
    DownloadWorker::Params params;
    params.file_id      = file_id;
    params.server_url   = server_url();
    if (!params.server_url.isValid()) {
        params.server_url = QUrl("http://127.0.0.1:50051");
    }
    params.channel_options = QGrpcChannelOptions{};
    params.call_options    = m_grpc_call_options;  // 值类型，按值带入子线程
    params.saved_path      = Database::instance()->get_local_file_id_path(file_id);
    params.downloaded_size = qMax<qint64>(0, QFileInfo(params.saved_path).size());
    // user_cache_path() / user_download_path() 内部会访问 Accounts 单例
    params.cache_dir    = Utils::user_cache_path();
    params.download_dir = Utils::user_download_path();
    params.callback     = callback_download_file_notify;

    // protobuf 消息是值类型，按值传入子线程即可
    params.request.setRequestUserAuth(*Accounts::instance()->get_auth_request().get());
    params.request.setFileId(file_id);
    params.request.setDownloadedBytes(params.downloaded_size);  // 断点续传

    // ---- 2. 创建带事件循环的专用线程 ----
    auto *thread = new QThread;              // 默认 run() 会 exec()，不要加 parent
    thread->setObjectName(QStringLiteral("FileDownload-%1").arg(file_id));
    auto *worker = new DownloadWorker;       // 不加 parent：QObject 不能跨线程设父子关系
    worker->moveToThread(thread);

    // 线程结束后在各自所属线程里析构
    QObject::connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    QObject::connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    register_download_thread(thread);        // 程序退出时统一收尾

    params.thread = thread;
    thread->start();

    // ---- 3. 排队启动：确保 channel / client / stream 在子线程内创建 ----
    QMetaObject::invokeMethod(worker,
        [worker, params = std::move(params)]() mutable {
            worker->start(std::move(params));
        }, Qt::QueuedConnection);
    return true;
}

bool NetworkClient::download_file_notify_signal(const QString &file_id) {
    static FileDownloadRecords s_records;
    // qDebug().noquote().nospace() << "trace message file download." << "NetworkClient::download_file_notify_signal() add_record " << file_id;
    // 文件ID加入队列
    if(!s_records.add_record(file_id)) {
        return false;
    }
    // qDebug().noquote().nospace() << "trace message file download." << "NetworkClient::download_file_notify_signal() add_record " << file_id << " ok";
    network_file_io_notify_callback callback_download_file_notify = [this](const Network::FileDownloadNotify notify) {
        emit SignalInner::instance()->signal_file_download_notify(notify);  // 广播下载通知
        if (!notify.finished) {
            emit SignalInner::instance()->signal_file_downloading(notify);  // 广播下载通知
        }
        else if ( notify.finished) {
            // qDebug().noquote().nospace() << "trace message file download." << "NetworkClient::download_file_notify_signal() finished " << notify.id << " pending.";
            // 下载出错
            if (!notify.ok) {
                // qDebug().noquote().nospace() << "trace message file download." << "NetworkClient::download_file_notify_signal() finished " << notify.id << " error.";
                emit SignalInner::instance()->signal_file_download_error(notify);
            }
            // 下载完成的通知 // 非下载过程中的通知
            else if (notify.status == FileStatusCode::FILE_STATUS_UPLOADED) {
                // qDebug().noquote().nospace() << "trace message file download." << "NetworkClient::download_file_notify_signal() finished " << notify.id << " ok.";
                emit SignalInner::instance()->signal_file_downloaded(notify);
            }
            s_records.remove_reord(notify.id);
        }
    };
    download_file(file_id, callback_download_file_notify);
    return true;
}
