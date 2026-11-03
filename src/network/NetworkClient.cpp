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

// 不要同时下载一个文件
// true 代表调用成功，不代表下载结果
bool NetworkClient::download_file(const QString &file_id, network_file_io_notify_callback callback_download_file_notify) {
    using namespace Utils::Wrapper;
    using Request  = FileDownloadRequest;
    using Reply    = FileDownloadReply;
    using ReplyOpt = std::optional<Reply>;
    using Params   = file_download_params;
    using Notify   = Network::FileDownloadNotify;
    using Callback = network_file_io_notify_callback;
    using Stream   = QGrpcServerStream;

    // 需要传出的函数
    struct FileDownloadArgs {
        using BytesMd5 = Utils::Algorithm::BytesMd5;
        Notify                 notify;
        std::shared_ptr<QFile> file_download;  // 用于传递文件句柄
        FileDownloadHeader     header;
        bool                   initialized = false;  // 已经完成初始化的标记
        qint64                 downloaded_size = 0;  // 已经下载的大小，支持断点续传时有用
        QString                real_md5;  // hash 下载文件得出的结果
        BytesMd5               calculated_md5;  // hash 下载的文件的工具
    };
    // 获取下载起始位置
    std::shared_ptr<FileDownloadArgs> file_download_args = std::make_shared<FileDownloadArgs>();
    auto& notify = file_download_args->notify;

    // 只修改与默认值不同的变量
    notify.id   = file_id;
    notify.path = Database::instance()->get_local_file_id_path(file_id);

    const auto& file_info = QFileInfo(notify.path);
    file_download_args->downloaded_size = qMax<qint64>(0, file_info.size());

    // 路径不为空，代表有下载记录，此时可以打开文件。打开文件失败直接返回，成功时在下载过程中再校验哈希。
    if (!notify.path.isEmpty()) {
        Utils::create_dirs(Utils::get_dir(notify.path));  // 确保目录存在
        file_download_args->file_download = std::make_shared<QFile>(notify.path);
        // 文件打开失败返回错误
        if (!file_download_args->file_download->open(QIODevice::ReadWrite)) {
            // 只修改与默认/当前值不同的变量
            notify.finished = true;
            notify.ok       = false;
            notify.code     = int64_t(ErrorCode::FILE_READ_ERROR);
            notify.message  = tr("Failed to open file for md5 calculation: %1").arg(file_download_args->file_download->errorString());
            callback_download_file_notify(notify);
            return false;
        }
    }
    // 业务逻辑
    // ===============                                                    //
    Request request_download_file_param;
    request_download_file_param.setRequestUserAuth(*Accounts::instance()->get_auth_request().get());
    request_download_file_param.setFileId(notify.id);
    request_download_file_param.setDownloadedBytes(file_download_args->downloaded_size);  // 断点续传
    std::unique_ptr<Stream> reply_conn = m_client_file->FileDownload(request_download_file_param, m_grpc_call_options);
    //                                                    =============== //
    // 将 unique_ptr 转为 shared_ptr 供多个 Lambda 安全共享生命周期
    std::shared_ptr<Stream> stream_ptr = std::move(reply_conn);
    Stream* raw_stream = stream_ptr.get();

    // const auto *rep_conn_ptr = reply_conn.get();  // 必须
    if (!raw_stream) {
        notify.finished = true;
        notify.ok       = false;
        notify.code     = int64_t(ErrorCode::ERROR_CONN);
        notify.message  = "Error in connect.";
        callback_download_file_notify(notify);
        return true;
    }

    notify.finished = false;  // 防止提前结束

    // 收到数据时触发的消息  // 服务器每次 while 写数据到客户端都会触发一次
    connect(raw_stream, &Stream::messageReceived, this,
        [stream_ptr, file_download_args, callback_download_file_notify]() {
            auto& notify = file_download_args->notify;
            // 发生错误时已经通知，后续接收到数据时忽略即可，不必通知
            // if (file_download_args->error_occued) {
            if (notify.finished) {
                callback_download_file_notify(notify);  // 可能不需要再次广播
                return;
            }

            // 读取失败
            auto reply = stream_ptr->read<FileDownloadReply>();
            if (!reply) {
                notify.finished = true;
                notify.ok       = false;
                notify.code     = int64_t(ErrorCode::ERROR_RECV);
                notify.message  = "Error in received data.";
                callback_download_file_notify(notify);
                return;
            }

            // 文件头
            if (!file_download_args->initialized) {
                if (!reply->hasHeader()) {
                    notify.finished = true;
                    notify.ok       = false;
                    notify.code     = int64_t(ErrorCode::ERROR_RECV);
                    notify.message  = "Error in read reply : no header info found.";
                    callback_download_file_notify(notify);
                    return;
                }

                file_download_args->header = reply->header();  // 导出
                if (!file_download_args->header.result().hasSuccessCode()) {
                    notify.finished = true;
                    notify.ok       = false;
                    notify.code     = int64_t(ErrorCode::FILE_DOWNLOAD_ERROR);
                    notify.message  = "Header return error.";
                    callback_download_file_notify(notify);
                    return;
                }

                // 如果是有效的文件头信息，将有效信息保存到用于通知的结构体中
                const auto& file_metadata = file_download_args->header.fileMetadata();
                notify.name = file_metadata.fileName();
                notify.size = file_metadata.fileSize();
                notify.md5  = file_metadata.fileMd5 ();

                // 校验参数是否有效
                if (notify.name.isEmpty() || notify.size < 0 || notify.md5.isEmpty()) {
                    notify.finished = true;
                    notify.ok       = false;
                    notify.code     = int64_t(ErrorCode::FILE_DOWNLOAD_ERROR);
                    notify.message  = "Header's filemeta info not valid.";
                    callback_download_file_notify(notify);
                    return;
                }

                // TODO:先检查最终目标，然后再检查临时文件
                // 存储路径为空且文件未打开时，进行初始化
                if ( notify.path.isEmpty() && !file_download_args->file_download) {
                    notify.path = QString("%1/%2/%3.part").arg(Utils::user_cache_path()).arg(notify.md5).arg(notify.name);  //  未下载完成时添加标记
                    // 保存 file_id 和 本地路径的映射关系，以便后续根据 file_id 获取本地文件路径进行删除等操作
                    Database::instance()->save_local_file_id_path(notify.id, notify.path); // 未下载完成时也进行存储，下载完成后再重命名及更新记录
                    Utils::create_dirs(Utils::get_dir(notify.path));  // 确保目录存在
                    file_download_args->file_download = std::make_shared<QFile>(notify.path);
                    if (!file_download_args->file_download || !file_download_args->file_download->open(QIODevice::ReadWrite)) {
                        notify.finished = true;
                        notify.ok       = false;
                        notify.code     = int64_t(ErrorCode::FILE_WRITE_ERROR);
                        notify.message  = "Open file error.";
                        callback_download_file_notify(notify);
                        return;
                    }
                }

                // 文件大小异常时可以跳转到截断位置
                int64_t file_offset = file_download_args->header.offset();
                // 计算已有文件的 md5  // 只读取到下载起始位置与实际文件大小中的下限位置处即可
                int64_t unread_bytes = file_offset;
                file_download_args->calculated_md5.reset();
                while (file_download_args->file_download && unread_bytes > 0 && !file_download_args->file_download->atEnd()) {
                    const size_t chunk_size = 128 * 1024;  // 128KB
                    QByteArray chunk_data = file_download_args->file_download->read(chunk_size);
                    file_download_args->calculated_md5.update(chunk_data);
                    unread_bytes -= chunk_data.size();
                }
                // 截断之后的数据
                file_download_args->file_download->resize(file_download_args->file_download->pos());
                file_download_args->initialized = true ;  // 已经成功初始化

                notify.finished = false;  // 没有发生错误或下载完成，后面还需要继续操作
                notify.ok       = true;  // 未发生错误

                // 第一个包是文件信息时，走到这里就已经处理完成
                return;
            }

            // 写入数据
            // 文件上传完成后最后一条消息也会触发该判断，但实际并不是错误，不需要报错
            if (!reply->hasChunk()) {
                // notify.finished = true;
                // notify.ok       = false;
                // notify.code     = int64_t(ErrorCode::FILE_DOWNLOAD_ERROR);
                // notify.message  = "Error in read reply : no chunk data found.";
                // callback_download_file_notify(notify);
                return;
            }

            auto& chunk_data = reply->chunk().chunkData();
            qint64 chunk_size = chunk_data.size();
            // 有数据时进行写入
            if (!chunk_data.isEmpty()) {
                qint64 bytes_written = file_download_args->file_download->write(chunk_data);

                // 写入失败
                if (bytes_written != chunk_size) {
                    notify.finished = true;
                    notify.ok       = false;
                    notify.code     = int64_t(ErrorCode::FILE_WRITE_ERROR);
                    notify.message  = QString("Write file chunk error.\n\npath: %1.\nerror message: %2")
                        .arg(notify.path).arg(file_download_args->file_download->errorString());
                    callback_download_file_notify(notify);
                    return;
                }

                // md5 校验
                file_download_args->calculated_md5.update(chunk_data);
                // 防止 uploaded_bytes 为默认的无效值 -1
                notify.uploaded_bytes = qMax(0, notify.uploaded_bytes);
                notify.uploaded_bytes += bytes_written;

                // 写入成功时的进度通知
                notify.finished = false;  // 不是下载结束的通知，不要设置为 true
                notify.ok       = true;
                notify.code     = int64_t(SuccessCode::SUCCESS);
                notify.message  = "Write chunk data success.";
                notify.status   = FileStatusCode::FILE_STATUS_UPLOADING;
                callback_download_file_notify(notify);
            }
        }
    );

    // 捕获输出返回信息  // 单次模型需要使用 `Qt::SingleShotConnection` 以便结束后销毁
    // lamda 表达式不可使用引用，会造成生命周期问题
    // 流结束时发送的消息
    connect(raw_stream, &QGrpcCallReply::finished, this,
        [this, stream_ptr, file_download_args, callback_download_file_notify](const QGrpcStatus &status) mutable {
            auto& notify = file_download_args->notify;
            // 关闭文件句柄
            if (file_download_args->file_download && file_download_args->file_download->isOpen()) {
                file_download_args->file_download->close();
            }

            // 测试读取是否异常
            if (!QFile(notify.path).open(QIODevice::ReadWrite)) {
                qDebug().noquote().nospace() << QString("failed to open file %1").arg(notify.path);
            }

            // 服务器返回错误
            if (!status.isOk()) {
                notify.finished = true;
                notify.ok       = false;
                notify.code     = int64_t(ErrorCode::ERROR_CONN);
                notify.message  = "Error in received data.";
                callback_download_file_notify(notify);
                return;
            }

            // 文件校验失败  // 先注释掉通知，下载出错时已经发送过通知了
            // file_download_params->real_md5 = Utils::Algorithm::calculate_file_md5(notify.path);
            if ( notify.md5 != file_download_args->calculated_md5.get_hex_string()) {
                // notify.finished = true;
                // notify.ok       = false;
                // notify.code     = int64_t(ErrorCode::FILE_DOWNLOAD_ERROR);
                // notify.message  = "Error in received data.";
                // callback_download_file_notify(notify);
                return;
            }

            // 移动文件及更新记录
            // Utils::rename(...) 会自动创建父目录
            QString new_file_path = QString("%1/%2/%3").arg(Utils::user_download_path()).arg(notify.md5).arg(notify.name);  //  下载完成后的保存位置
            if (Utils::rename(notify.path, new_file_path)) {
                Database::instance()->save_local_file_id_path(notify.id, new_file_path); // 下载完成后需要更新记录
                notify.path = new_file_path;  // 改名后需要同步修改路径
            }

            // 文件下载完成时的结果通知
            // emit SignalInner::instance()->signal_file_downloaded(notify.id);
            notify.finished = true;  // 任务结束(下载完成)
            notify.ok       = true;  // 没有发生错误
            notify.code     = int64_t(SuccessCode::SUCCESS);  // 成功代码
            notify.message  = tr("Download completed: %1").arg(notify.path);
            notify.status   = FileStatusCode::FILE_STATUS_UPLOADED;  // 文件下载完成
            callback_download_file_notify(notify);
        }, Qt::SingleShotConnection
    );
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
        if (!notify.finished) {
            emit SignalInner::instance()->signal_download_file(notify);  // 广播下载通知
        }
        else if ( notify.finished) {
            // qDebug().noquote().nospace() << "trace message file download." << "NetworkClient::download_file_notify_signal() finished " << notify.id << " pending.";
            // 下载出错
            if (!notify.ok) {
                // qDebug().noquote().nospace() << "trace message file download." << "NetworkClient::download_file_notify_signal() finished " << notify.id << " error.";
                emit SignalInner::instance()->signal_file_download_error(notify);
            }
            // 非下载过程中的通知
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
