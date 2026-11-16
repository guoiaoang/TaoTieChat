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

bool NetworkClient::get_file_id(const QString &file_path, network_file_io_notify_callback callback_get_file_id_notiify) {
    using Wrapper  = Utils::Wrapper::FileMetadataWrapper;
    using Notify   = Network::FileDownloadNotify;
    using Callback = network_file_io_notify_callback;

    auto func_file_hash = [this, file_path, callback_get_file_id_notiify](const QString &file_hash){
        // if (file_hash.isEmpty()) { return; }

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
        auto &notify = get_file_id_args->notify;
        notify.path  = file_path;
        if (file_hash.isEmpty() || file_path.isEmpty()) {
            notify.finished = true;
            notify.ok       = false;
            notify.code     = int64_t(ErrorCode::FILE_NAME_EMPTY);
            notify.message  = "file path empty.";
            callback_get_file_id_notiify(notify);
            return;
        }
        auto file_info = QFileInfo(file_path);
        auto file_name = file_info.fileName();
        auto file_size = file_info.size();
        auto file_md5  = file_hash; // Utils::Algorithm::calculate_file_md5(file_path);
        // 可能部分操作成功
        if (file_name.isEmpty() || file_size < 0 || file_md5.isEmpty()) {
            notify.finished = true;
            notify.ok       = false;
            notify.code     = int64_t(ErrorCode::FILE_READ_ERROR);
            notify.message  = "file not exists.";
            notify.size     = file_size;
            notify.md5      = file_md5;
            callback_get_file_id_notiify(notify);
            return;
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
    };
    Database::get_file_hash(file_path, func_file_hash);
    return true;
}

// upload_file 的实现放在文件末尾：与 download_file 一样整条链路放进专用子线程。
// 旧版本在 messageReceived 回调里用 while 循环连续 writeMessage，会占住事件循环，
// HTTP/2 帧只堆在内存缓冲区里发不出去，已整体移除。

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
// 文件上传 / 下载：整条链路放进专用子线程
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
//      上传 / 下载前把参数准备好，过程中的写库与通知排队回主线程。
//   4. 每个任务一个线程，任务结束 quit() + deleteLater 自动回收，互不阻塞。
//   5. 上传按分片排队发送并主动让出事件循环，不能用 while 连发。
// =========================================================================== //
namespace {

// 程序退出时统一收尾，避免 "QThread: Destroyed while thread is still running"
static QMutex                    g_file_threads_mutex;
static QList<QPointer<QThread>>  g_file_threads;

static void shutdown_file_threads() {
    QList<QPointer<QThread>> snapshot;
    {
        QMutexLocker locker(&g_file_threads_mutex);
        snapshot = g_file_threads;
    }
    for (const QPointer<QThread> &p : snapshot) {
        if (!p) { continue; }
        p->quit();
        p->wait(5000);
    }
}

static void register_file_thread(QThread *thread) {
    static const bool s_registered = []() {
        qAddPostRoutine(&shutdown_file_threads);
        return true;
    }();
    (void)s_registered;
    QMutexLocker locker(&g_file_threads_mutex);
    for (qsizetype i = g_file_threads.size() - 1; i >= 0; --i) {
        if (g_file_threads.at(i).isNull()) {
            g_file_threads.removeAt(i);
        }
    }
    g_file_threads.append(QPointer<QThread>(thread));
}

// =========================================================================== //
// 文件上传：整条链路放进专用子线程（与 DownloadWorker 对称）
// --------------------------------------------------------------------------- //
//   1. 子线程内新建 channel + client，绝不复用主线程 m_channel / m_client_file。
//   2. 全文件 MD5、GetFileID、FileUpload 双向流、分片读写全部在子线程。
//   3. 【关键】分片必须限速，不能自投事件连发。
//      Qt 6.10.3 的 QGrpcHttp2Channel 内部是一个无上限的发送队列：
//        Http2Handler::writeMessage() -> processQueue() -> QHttp2Stream::sendDATA()
//      sendDATA() 写完会同步 emit uploadFinished()，而 Qt 用默认
//      AutoConnection 把该信号直连回 Http2Handler::processQueue()（同线程即直接调用）。
//      旧实现每片结束就 invokeMethod 自己投下一片，本地读盘远快于网络发送，
//      队列会堆到成千上万条；一次 WINDOW_UPDATE 触发同步递归排空，
//      递归深度 = 队列长度，直接爆栈（实测 2435 帧 = 174 层同构循环）。
//      这里改为定时器节拍泵：每个节拍最多发 kMaxChunksPerTick 片，节拍之间
//      事件循环有机会处理 socket / WINDOW_UPDATE，队列不会堆积。
//      若服务端会逐片回 uploaded_bytes（异步版服务端），再额外启用 8 MiB
//      在途字节窗口做真正的背压；同步版服务端不回进度，就只靠节拍限速。
//   4. 通知统一排队回主线程，调用方回调与下载保持同一语义。
//   5. 上传线程给 64 MB 栈作为最后兜底：即便极端情况下队列仍被排空，
//      递归也撑不到爆栈（默认 1 MB 栈只能撑几百层）。
// =========================================================================== //
class UploadWorker : public QObject {
public:
    using Notify    = Network::FileDownloadNotify;
    using Callback  = network_file_io_notify_callback;
    using Stream    = QGrpcBidiStream;
    using CallReply = QGrpcCallReply;

    // 全部是值类型 / 裸指针，可安全跨线程传递；不含任何 QObject 实例
    struct Params {
        QString             file_path;
        QUrl                server_url;
        QGrpcChannelOptions channel_options;
        QGrpcCallOptions    call_options;
        AuthRequest         auth;      // 主线程取好，避免子线程访问 Accounts 单例
        QThread            *thread  = nullptr;
        Callback            callback;
    };

    // 只能通过 QMetaObject::invokeMethod(..., Qt::QueuedConnection) 调用，
    // 保证 channel / client / stream 都在子线程内创建
    void start(Params params) {
        m_params = std::move(params);
        m_thread                = m_params.thread;
        m_notify.path           = m_params.file_path;
        m_notify.uploaded_bytes = 0;   // 从 0 计，避免默认值 -1
        m_notify.status         = FileStatusCode::FILE_STATUS_UPLOADING;
        m_progress_timer.start();

        // 注意：看门狗不要在这里启动。下面要同步计算整个文件的 MD5，GB 级文件
        // 会占住本线程事件循环几十秒，恢复后定时器会误判成「服务端无响应」。

        // ---- 1. 参数校验 + 全文件 MD5（O(文件大小)，放在子线程不卡 UI） ----
        if (m_notify.path.isEmpty()) {
            fail(ErrorCode::FILE_NAME_EMPTY, "file path empty.");
            return;
        }
        const QFileInfo file_info(m_notify.path);
        m_notify.name = file_info.fileName();
        m_notify.size = file_info.size();
        m_notify.md5  = Utils::Algorithm::calculate_file_md5(m_notify.path);
        if (m_notify.name.isEmpty() || m_notify.size < 0 || m_notify.md5.isEmpty()) {
            fail(ErrorCode::FILE_READ_ERROR, "file not exists.");
            return;
        }
        emit_progress(true);  // 起始 0%：文件名 / 大小 / MD5 已经确定

        // ---- 2. 【核心】channel 与 client 必须在本线程创建 ----
        m_channel = std::make_shared<QGrpcHttp2Channel>(m_params.server_url, m_params.channel_options);
        m_client  = std::make_unique<FileClient>();     // 不带 parent，亲和本线程
        if (!m_client->attachChannel(m_channel)) {
            fail(ErrorCode::ERROR_CONN, "Error in connect.");
            return;
        }

        // ---- 3. GetFileID：秒传判断 / 取文件 ID ----
        GetFileIDRequest request;
        request.setRequestUserAuth(m_params.auth);
        request.setFileMetadata(Utils::Wrapper::FileMetadataWrapper{
            .file_name = m_notify.name,
            .file_size = m_notify.size,
            .file_md5  = m_notify.md5 }());
        std::unique_ptr<CallReply> reply = m_client->GetFileID(request, m_params.call_options);
        m_call_reply = std::shared_ptr<CallReply>(std::move(reply));  // 不能是局部变量，否则会被提前重置
        if (!m_call_reply) {
            fail(ErrorCode::ERROR_CONN, "Error in connect.");
            return;
        }
        m_notify.finished = false;  // 防止提前结束
        // finished 定义在基类 QGrpcOperation 上
        connect(m_call_reply.get(), &QGrpcOperation::finished, this,
            [this](const QGrpcStatus &status) { on_file_id_reply(status); },
            Qt::SingleShotConnection);
        touch_watchdog();  // MD5 / 建连都已完成，从这里开始计时空闲超时
    }

private:
    // 所有主线程任务都投递到 NetworkClient 单例（亲和主线程），与下载一致
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

    void fail(ErrorCode code, const QString &message) {
        if (m_stopped) { return; }
        Notify n    = m_notify;
        n.finished  = true;
        n.ok        = false;
        n.code      = int64_t(code);
        n.message   = message;
        emit_notify(n);
        stop();
    }

    // 空闲看门狗：任何一次进展都重新计时，服务端长时间无响应则结束任务
    void touch_watchdog() {
        if (!m_watchdog) {
            m_watchdog = new QTimer(this);
            m_watchdog->setSingleShot(true);
            m_watchdog->setInterval(60 * 1000);
            connect(m_watchdog, &QTimer::timeout, this, [this]() {
                if (!m_stopped) { fail(ErrorCode::ERROR_RECV, "Timeout: no response from server."); }
            });
        }
        m_watchdog->start();
    }

    // 断开信号避免退出前再次触发，然后让线程执行 quit()
    void stop() {
        if (m_stopped) { return; }
        m_stopped = true;
        if (m_watchdog)   { m_watchdog->stop(); }
        if (m_chunk_pump) { m_chunk_pump->stop(); }
        if (m_stream) { m_stream->disconnect(this); }
        if (m_call_reply) { m_call_reply->disconnect(this); }
        if (m_file && m_file->isOpen()) { m_file->close(); }
        m_file.reset();
        QThread *thread = m_thread;
        if (thread) {
            QMetaObject::invokeMethod(thread, &QThread::quit, Qt::QueuedConnection);
        }
    }

    // ---- GetFileID 回复：决定秒传、继续上传还是报错 ----
    void on_file_id_reply(const QGrpcStatus &status) {
        if (m_stopped) { return; }
        touch_watchdog();
        auto &n = m_notify;
        std::optional<GetFileIDReply> reply;
        if (status.isOk() && m_call_reply) {
            reply = m_call_reply->read<GetFileIDReply>();  // 从网络流中读取并解析
        }
        if (!status.isOk() || !reply || !reply->hasResult()) {
            const QString detail = status.message();
            fail(ErrorCode::ERROR_CONN, detail.isEmpty() ? QString("No reply or no result.") : QString("GetFileID failed: %1").arg(detail));
            return;
        }
        n.finished = true;
        n.ok       = reply->result().hasSuccessCode();
        n.code     = n.ok ? int64_t(reply->result().successCode()) : int64_t(reply->result().errorCode());
        n.message  = reply->result().message();
        n.id       = reply->fileId();
        n.status   = reply->fileStatus();

        // 获取文件上传 ID 失败
        if (!n.ok || n.id.isEmpty()) {
            emit_notify(n);
            stop();
            return;
        }
        // 极速上传：服务器上已经有相同的文件了
        if (n.status == FileStatusCode::FILE_STATUS_UPLOADED) {
            n.code           = int64_t(SuccessCode::SUCCESS);
            n.message        = "File already uploaded.";
            n.uploaded_bytes = n.size;
            n.status         = FileStatusCode::FILE_STATUS_UPLOADED;
            emit_notify(n);
            stop();
            return;
        }
        n.finished = false;  // 进入上传阶段
        start_upload_stream();
    }

    // ---- 发起 FileUpload 双向流；首包携带 header（鉴权 + 文件 ID） ----
    void start_upload_stream() {
        FileUploadHeader header;
        header.setRequestUserAuth(m_params.auth);
        header.setFileId(m_notify.id);
        FileUploadRequest request;
        request.setHeader(header);

        std::unique_ptr<Stream> reply = m_client->FileUpload(request, m_params.call_options);
        m_stream = std::shared_ptr<Stream>(std::move(reply));
        if (!m_stream) {
            fail(ErrorCode::ERROR_CONN, "Error in connect.");
            return;
        }
        m_notify.finished = false;
        connect(m_stream.get(), &Stream::messageReceived, this, [this]() { on_message(); });
        // finished 定义在基类 QGrpcOperation 上
        connect(m_stream.get(), &QGrpcOperation::finished, this,
            [this](const QGrpcStatus &status) { on_finished(status); }, Qt::SingleShotConnection);
        touch_watchdog();
    }

    // ---- 服务端消息：首包决定是否上传，中间可能是保活/进度，最后一包是结果 ----
    void on_message() {
        if (m_stopped) { return; }
        touch_watchdog();
        auto &n = m_notify;
        if (!m_stream || n.finished) { return; }

        auto reply = m_stream->read<FileUploadReply>();
        if (!reply) {
            fail(ErrorCode::ERROR_RECV, "Error in received data.");
            return;
        }

        // 服务端明确给出错误码才是真正的失败终态，错误码与说明要透传给 UI。
        if (reply->hasResult() && reply->result().hasErrorCode()) {
            n.finished = true;
            n.ok       = false;
            n.code     = int64_t(reply->result().errorCode());
            n.message  = reply->result().message();
            if (n.message.isEmpty()) { n.message = "Server rejected file upload."; }
            n.status   = reply->fileStatus();
            emit_notify(n);
            stop();
            return;
        }

        const FileStatusCode file_status = reply->fileStatus();
        // 上传完成：服务端返回的最终状态
        if (file_status == FileStatusCode::FILE_STATUS_UPLOADED) {
            n.finished       = true;
            n.ok             = true;
            n.code           = int64_t(SuccessCode::SUCCESS);
            n.message        = "File upload finished.";
            n.status         = file_status;
            n.uploaded_bytes = n.size;
            emit_notify(n);
            stop();
            return;
        }
        // 【注意】file_status 不是 RPC 错误码，绝不能在这里拿来判断失败。
        // 服务端 GetFileID 为新文件写入的初始状态就是 FILE_STATUS_NOT_FOUND
        // （DBSQLCipher.cpp 写元数据时写死为 3），FileUpload 的首包会原样回这个
        // 状态，含义是「服务器上还没有内容，请按 uploaded_bytes 开始传」。
        // 旧代码把 NOT_FOUND 直接 fail()，导致任何新文件一片都发不出去就报
        // 「File not found on server.」。真正的错误一律走上面的 error_code 分支。
        // ------------------------------------------------------------------
        // 首包：按服务端确认的已上传字节数决定断点续传起点。
        // NONE / UPLOADING / NOT_FOUND 都表示「继续上传」。
        const qint64 uploaded_bytes = qMax<qint64>(0, reply->uploadedBytes());
        if (!m_upload_started) {
            if (uploaded_bytes > n.size) {
                fail(ErrorCode::FILE_READ_ERROR, "Uploaded bytes invalid.");
                return;
            }
            n.uploaded_bytes = uploaded_bytes;
            begin_send_chunks(uploaded_bytes);
            return;
        }

        // 上传中的消息：可能是逐片进度，也可能只是保活。异步版服务端每收一片
        // 都会回一条 uploaded_bytes；同步版只在首包和最终包回写。只要进度值
        // 真正增长过，就启用「在途字节窗口」做背压；没有增长就保持纯节拍限速，
        // 任务不会因为等不到回复而停摆。
        const qint64 before = m_server_acked;
        m_server_acked = qMax(m_server_acked, uploaded_bytes);
        if (m_server_acked > before) { m_server_reports_progress = true; }
        emit_progress(false);
        pump_chunks();   // 服务端已确认收到，立刻补满在途窗口（若有）
    }

    void begin_send_chunks(qint64 offset) {
        m_file = std::make_shared<QFile>(m_notify.path);
        if (!m_file->open(QIODevice::ReadOnly)) {
            fail(ErrorCode::FILE_READ_ERROR, QObject::tr("open file error."));
            return;
        }
        if (!m_file->seek(offset)) {
            m_file->close();
            fail(ErrorCode::FILE_READ_ERROR, QObject::tr("Failed to seek upload file"));
            return;
        }
        m_next_offset             = offset;
        m_server_acked            = offset;
        m_server_reports_progress = false;
        m_writes_done_sent        = false;
        m_upload_started          = true;
        start_chunk_pump();   // 主驱动：定时器节拍泵
        pump_chunks();        // 立即发第一批，不必等第一个节拍
    }

    // 定时器节拍泵——上传分片的唯一主驱动。
    // 这与「每片写完自投一个 queued 事件」有本质区别：自投事件会跟事件循环
    // 抢跑，一秒能灌几万片；固定节拍把入队速率压到
    // kMaxChunksPerTick * kChunkSize / kPumpIntervalMs（约 64 MB/s），
    // 节拍之间 socket 可读可写事件和 HTTP/2 WINDOW_UPDATE 都有机会被处理。
    void start_chunk_pump() {
        if (!m_chunk_pump) {
            m_chunk_pump = new QTimer(this);
            m_chunk_pump->setTimerType(Qt::PreciseTimer);
            m_chunk_pump->setInterval(kPumpIntervalMs);
            connect(m_chunk_pump, &QTimer::timeout, this, [this]() {
                if (!m_stopped && !m_writes_done_sent) { pump_chunks(); }
            });
        }
        m_chunk_pump->start();
    }

    // 唯一的发片入口。m_pumping 防止 writeMessage() 内部的同步回调造成重入；
    // 单次节拍最多 kMaxChunksPerTick 片，把 QtGrpc HTTP/2 内部队列长度压到个位数。
    void pump_chunks() {
        if (m_stopped || m_writes_done_sent || !m_stream || !m_file || !m_upload_started) { return; }
        if (m_pumping) { return; }
        m_pumping = true;
        struct Guard {
            bool &flag;
            ~Guard() { flag = false; }
        } guard{m_pumping};
        (void)guard;

        int sent = 0;
        while (sent < kMaxChunksPerTick) {
            if (m_stopped || !m_stream || !m_file) { break; }
            // 服务端会逐片回报进度时，额外限制「已交给 Qt 但服务端还没确认」
            // 的字节数，形成真正的窗口背压；否则只靠节拍限速。
            if (m_server_reports_progress &&
                (m_next_offset - m_server_acked) >= kMaxInFlightBytes) { break; }

            if (m_file->atEnd()) { finish_writes(); return; }

            const QByteArray chunk_data = m_file->read(kChunkSize);
            const qint64     bytes_read = chunk_data.size();
            if (bytes_read < 0) {
                fail(ErrorCode::FILE_READ_ERROR, QObject::tr("Failed to read upload file"));
                return;
            }
            if (bytes_read == 0) { finish_writes(); return; }

            FileChunkData chunk;
            chunk.setChunkOffset(m_next_offset);
            chunk.setChunkData(chunk_data);
            FileUploadRequest request;
            request.setChunk(chunk);
            m_stream->writeMessage(request);
            m_next_offset += bytes_read;
            ++sent;
            // 同步版服务端在整个上传过程中一条消息都不回，看门狗必须靠本地
            // 「确实写出了一片」续期，否则大文件会在 60s 后被误判超时。
            touch_watchdog();
            // 同步版服务端没有中间消息，进度只能在本地写片时推进；
            // emit_progress 自带 100ms 节流，不会灌满主线程事件队列。
            emit_progress(false);
        }
        if (m_file && m_file->atEnd()) { finish_writes(); }
    }

    // 文件读完：补一条最终进度，关闭文件并写半关闭，等待服务端最终状态
    void finish_writes() {
        if (m_writes_done_sent) { return; }
        m_writes_done_sent = true;
        emit_progress(true);
        if (m_file) { m_file->close(); m_file.reset(); }
        if (m_chunk_pump) { m_chunk_pump->stop(); }
        if (m_stream) { m_stream->writesDone(); }
    }

    // 进度通知按 100ms 节流，避免灌满主线程事件队列。
    // 会逐片回报的服务端用「服务端确认字节数」（真实收到的量）；
    // 不回报的服务端退回本地已交出的字节数，保证进度条照样会动。
    void emit_progress(bool force) {
        const qint64 value = m_server_reports_progress ? m_server_acked : m_next_offset;
        const qint64 shown = qMax<qint64>(0, value);
        if (!force && m_progress_timer.isValid() && m_progress_timer.elapsed() < 100) { return; }
        if (!force && shown == m_last_progress_bytes) { return; }
        m_progress_timer.restart();
        m_last_progress_bytes = shown;
        auto &n = m_notify;
        n.finished       = false;
        n.ok             = true;
        n.code           = int64_t(SuccessCode::SUCCESS);
        n.message        = QObject::tr("File uploading...");
        n.status         = FileStatusCode::FILE_STATUS_UPLOADING;
        n.uploaded_bytes = (n.size > 0) ? qMin(shown, n.size) : shown;
        emit_notify(n);
    }

    void on_finished(const QGrpcStatus &status) {
        if (m_stopped) { return; }
        if (!status.isOk()) {
            const QString detail = status.message();
            fail(ErrorCode::ERROR_CONN, detail.isEmpty() ? QString("Error in received data.") : QString("Upload stream error: %1").arg(detail));
            return;
        }
        // 服务端直接关闭流却没给最终状态：补一个明确失败，避免界面一直等待
        if (!m_notify.finished) {
            fail(ErrorCode::FILE_UPLOAD_ERROR, "Upload stream finished without result.");
            return;
        }
        stop();
    }

private:
    // 256 KiB/片：相比旧的 128 KiB，2.5 GiB 文件的消息数直接减半（约 1 万条）。
    // 节拍泵把入队速率压到 2*256KiB/8ms ≈ 64 MB/s，队列长度维持个位数；
    // 8 MiB 在途窗口只会在服务端逐片回报进度时额外启用。
    static constexpr qint64 kChunkSize         = 256 * 1024;
    static constexpr int    kPumpIntervalMs    = 8;
    static constexpr int    kMaxChunksPerTick  = 2;
    static constexpr qint64 kMaxInFlightBytes  = 8 * 1024 * 1024;

    Params                  m_params;
    QThread                *m_thread                  = nullptr;
    QTimer                 *m_watchdog                = nullptr;  // parent = this
    QTimer                 *m_chunk_pump              = nullptr;  // parent = this，节拍泵
    QElapsedTimer           m_progress_timer;                     // 进度通知节流
    Notify                  m_notify;
    std::shared_ptr<QFile>  m_file;
    qint64                  m_next_offset              = 0;   // 本地已交给 writeMessage 的字节数
    qint64                  m_server_acked             = 0;   // 服务端确认的字节数
    qint64                  m_last_progress_bytes      = -1;
    bool                    m_server_reports_progress  = false;
    bool                    m_pumping                  = false;
    bool                    m_writes_done_sent         = false;
    bool                    m_upload_started           = false;
    bool                    m_stopped                  = false;

    // 声明顺序决定析构逆序：stream / call_reply -> client -> channel，全部在子线程销毁
    std::shared_ptr<QGrpcHttp2Channel> m_channel;
    std::unique_ptr<FileClient       > m_client;
    std::shared_ptr<CallReply        > m_call_reply;
    std::shared_ptr<Stream           > m_stream;
};

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
    register_file_thread(thread);        // 程序退出时统一收尾

    params.thread = thread;
    thread->start();

    // ---- 3. 排队启动：确保 channel / client / stream 在子线程内创建 ----
    QMetaObject::invokeMethod(worker,
        [worker, params = std::move(params)]() mutable {
            worker->start(std::move(params));
        }, Qt::QueuedConnection);
    return true;
}

// true 代表调用成功，不代表上传结果
// 与 download_file 对称：主线程只准备参数，整条链路放进带事件循环的专用子线程
bool NetworkClient::upload_file(const QString &file_path, network_file_io_notify_callback callback_upload_file_notify) {
    // ---- 1. 主线程：准备参数（Accounts / server_url 只在主线程访问） ----
    UploadWorker::Params params;
    params.file_path       = file_path;
    params.server_url      = server_url();
    if (!params.server_url.isValid()) {
        params.server_url = QUrl("http://127.0.0.1:50051");
    }
    params.channel_options = QGrpcChannelOptions{};
    params.call_options    = m_grpc_call_options;  // 值类型，按值带入子线程
    params.callback        = callback_upload_file_notify;

    const auto auth = Accounts::instance()->get_auth_request();
    if (!auth) {
        Network::FileDownloadNotify notify;
        notify.path     = file_path;
        notify.finished = true;
        notify.ok       = false;
        notify.code     = int64_t(ErrorCode::ERROR_INIT);
        notify.message  = "Auth request not ready.";
        if (callback_upload_file_notify) { callback_upload_file_notify(notify); }
        return true;
    }
    params.auth = *auth;

    // ---- 2. 创建带事件循环的专用线程 ----
    auto *thread = new QThread;              // 默认 run() 会 exec()，不要加 parent
    // 兜底：Qt6Grpc 的 HTTP/2 队列排空是同步递归（每层约十几帧），默认 1 MB
    // 栈只能撑几百层。64 MB 在 Windows 上只是保留虚拟地址、按需提交，代价很低；
    // 即使极端情况下队列仍被一次性排空，也不会再爆栈。真正的修复是节拍限速。
    thread->setStackSize(64 * 1024 * 1024);
    thread->setObjectName(QStringLiteral("FileUpload-%1").arg(QFileInfo(file_path).fileName()));
    auto *worker = new UploadWorker;         // 不加 parent：QObject 不能跨线程设父子关系
    worker->moveToThread(thread);

    // 线程结束后在各自所属线程里析构
    QObject::connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    QObject::connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    register_file_thread(thread);           // 程序退出时统一收尾

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
