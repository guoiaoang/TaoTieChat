#include "NetworkClient.h"

// 重载 QDebug 运算符
QDebug operator<<(QDebug debug, const TaoTieChat::Result &result) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    if(result.hasSuccessCode()) {
        debug.nospace() << "Result("
                        << "success code:" << result.successCode() << ", "
                        << "message:"      << result.message()
                        << ")";
    }
    else if(result.hasErrorCode()) {
        debug.nospace() << "Result("
                        << "error code:" << result.errorCode() << ", "
                        << "message:"    << result.message()
                        << ")";
    }
    else {
        debug.nospace() << "Result(...)";
    }
    return debug;
}

// register
QDebug operator<<(QDebug debug, const TaoTieChat::SignUpRequest &req) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    debug.nospace() << "SignUpRequest("
                    << "userId:"           << req.userId()           << ", "
                    << "password:"         << req.password()         << ", "
                    << "confirm:"          << req.confirm()          << ", "
                    << "securityQuestion:" << req.securityQuestion() << ", "
                    << "securityAnswer:"   << req.securityAnswer()
                    << ")";
    return debug;
}
QDebug operator<<(QDebug debug, const TaoTieChat::SignUpReply &rep) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    debug.nospace() << "SignUpReply("
                    << "Request:"   << rep.request  () << ", "
                    << "result:"    << rep.result   () << ", "
                    << "accountId:" << rep.accountId()
                    << ")";
    return debug;
}

// login
QDebug operator<<(QDebug debug, const TaoTieChat::SignInRequest &req) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    const TaoTieChat::AuthRequest auth = req.hasRequestUserAuth()
                                         ? req.requestUserAuth()
                                         : TaoTieChat::AuthRequest();
    debug.nospace() << "SignInRequest("
                    << "userId:"   << (auth.hasUserId() ? auth.userId() : QString()) << ", "
                    << "password:" << (auth.hasPassword() ? auth.password() : QString()) << ", "
                    << "token:"    << (auth.hasToken() ? auth.token() : QString())
                    << ")";
    return debug;
}
QDebug operator<<(QDebug debug, const TaoTieChat::SignInReply &rep) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    debug.nospace() << "SignInReply("
                    << "Request:"   << rep.request  () << ", "
                    << "result:"    << rep.result   () << ", "
                    << "accountId:" << rep.accountId() << ", "
                    << "token:"     << rep.token    ()
                    << ")";
    return debug;
}

// search users
QDebug operator<<(QDebug debug, const TaoTieChat::Contact& contact) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    debug.nospace() << "Contact("
                    << "userId:"   << contact.userId  () << ", "
                    << "nickname:" << contact.nickname() << ", "
                    << "avatar:"   << contact.avatar  () << ", "
                    << "sign:"     << contact.sign    () << ", "
                    << "gender:"   << contact.gender  () << ", "
                    << "region:"   << contact.region  () << ", "
                    << "birthday:" << contact.birthday() << ", "
                    << "phones:"   << contact.phones  () << ", "
                    << "emails:"   << contact.emails  ()
                    << ")";
    return debug;
}
QDebug operator<<(QDebug debug, const TaoTieChat::SearchUsersRequest& req) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    debug.nospace() << "SearchUsersRequest("
                    << "userId:"     << req.userId()     << ", "
                    << "userFilter:" << req.userFilter() << ", "
                    << ")";
    return debug;
}
QDebug operator<<(QDebug debug, const TaoTieChat::SearchUsersReply& rep) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    debug.nospace() << "SearchUsersReply("
                    << "Request:"  << rep.request () << ", "
                    << "result:"   << rep.result  () << ", "
                    << "contacts.size:" << rep.contacts().contact().size() << ", "
                    << ")";
    return debug;
}

bool NetworkClient::init() {
    if(m_inited) {
        return true;
    }

    if(!server_url().isValid()) {
        set_server_url(QUrl("http://127.0.0.1:50051"));
    }

    // TODO:这里可以设置使用 https 加密链接，待客户端界面完善后再完成
    QGrpcChannelOptions opts;
    m_channel = std::make_shared<QGrpcHttp2Channel>(server_url(), opts);
    m_client_account = std::make_unique<TaoTieChat::AccountService::Client>();
    if (!m_client_account->attachChannel(m_channel)) {
        qDebug().nospace().noquote() << "attach AccountService Channel failed.";
        return false;
    }
    m_client_search = std::make_unique<TaoTieChat::SearchService::Client>();
    if (!m_client_search->attachChannel(m_channel)) {
        qDebug().nospace().noquote() << "attach SearchService Channel failed.";
        return false;
    }

    m_inited = true;

    return true;
}

// 成功失败都要返回，失败时要返回错误原因
bool NetworkClient::signup(const signup_params& params_signup, const signup_callback& callback) {
    using Result = TaoTieChat::Result;
    using ec = ::TaoTieChat::ErrorCodeGadget::ErrorCode;
    using sc = ::TaoTieChat::SuccessCodeGadget::SuccessCode;
    Result result;
    Utils::set(result, ec::UNSET);  // 默认设为错误

    // ================================================================== //

    // 有用的参数
    QString user_id  = params_signup->userId          ();
    QString password = params_signup->password        ();
    QString confirm  = params_signup->confirm         ();
    QString question = params_signup->securityQuestion();
    QString answer   = params_signup->securityAnswer  ();

    // 参数简单校验
    ;    if(user_id  .isEmpty()) { Utils::set(result, ec::USER_ID_EMPTY             ); }
    else if(password .isEmpty()) { Utils::set(result, ec::PASSWORD_EMPTY            ); }
    else if(confirm  .isEmpty()) { Utils::set(result, ec::CONFIRM_PASSWORD_EMPTY    ); }
    else if(question .isEmpty()) { Utils::set(result, ec::SECURITY_QUESTION_EMPTY   ); }
    else if(answer   .isEmpty()) { Utils::set(result, ec::SECURITY_ANSWER_EMPTY     ); }
    else if(password != confirm) { Utils::set(result, ec::DIFF_PASSWORD_WITH_CONFIRM); }
    else if(!m_client_account  ) { Utils::set(result, ec::ERROR_INIT                ); }
    // else { Utils::set(result, sc::SUCCESS); }

    signup_reply reply_data = TaoTieChat::SignUpReply();
    reply_data->setResult(result);

    // 失败时也要发送通知
    if(result.hasErrorCode() && result.errorCode() != ec::UNSET){
        if(callback) {
            callback(params_signup, reply_data);
        }
        return false;
    }

    // ================================================================== //

    // TODO:在网络交互前应该对密码进行加密

    // 构造参数
    TaoTieChat::SignUpRequest req;
    req.setUserId          (user_id );
    req.setPassword        (password);
    req.setConfirm         (confirm );
    req.setSecurityQuestion(question);
    req.setSecurityAnswer  (answer  );

    //
    auto opts = QGrpcCallOptions{}.setDeadlineTimeout(std::chrono::seconds(30));
    std::unique_ptr<QGrpcCallReply> reply_conn = m_client_account->SignUp(req, opts);
    const auto *rep_conn_ptr = reply_conn.get();  // 必须
    if (!rep_conn_ptr) {
        Utils::set(result, ec::ERROR_CONN);
        reply_data->setResult(result);
        if (callback) {
            callback(params_signup, reply_data);
        }
        return false;
    }
    // 捕获输出返回信息  // 单次模型需要使用 `Qt::SingleShotConnection` 以便结束后销毁
    // lamda 表达式不可使用引用，会造成生命周期问题
    connect(rep_conn_ptr, &QGrpcCallReply::finished, rep_conn_ptr,
        [reply_conn = std::move(reply_conn), params_signup, callback](const QGrpcStatus &status) mutable {
            signup_reply reply_data;
            if(status.isOk()) {
                reply_data = reply_conn->read<TaoTieChat::SignUpReply>();
            }
            if(callback) {
                // 发送参数，返回参数
                // 1. 返回参数为 nullopt 时代表返回错误，仅提示失败即可？？
                // 2. 即使失败也要调用会回调，以便通知界面
                callback(params_signup, reply_data);
            }
        }, Qt::SingleShotConnection);
    return true;
}

// 成功失败都要返回，失败时要返回错误原因
bool NetworkClient::signin(const signin_params& params_signin, const signin_callback& callback) {
    using Result = TaoTieChat::Result;
    using ec = ::TaoTieChat::ErrorCodeGadget::ErrorCode;
    using sc = ::TaoTieChat::SuccessCodeGadget::SuccessCode;
    Result      result;
    Utils::set(result, ec::UNSET);  // 默认设为错误
    // user_login_reply reply_data = TaoTieChat::SignInReply();
    // reply_data->setResult(Result{});

    // ================================================================== //

    // 有用的参数
    const TaoTieChat::AuthRequest auth = (params_signin && params_signin->hasRequestUserAuth())
                                         ? params_signin->requestUserAuth()
                                         : TaoTieChat::AuthRequest();
    QString user_id  = auth.hasUserId  () ? auth.userId  () : QString();
    QString password = auth.hasPassword() ? auth.password() : QString();
    QString token    = auth.hasToken   () ? auth.token   () : QString();
    QString auth_str = password + token;

    // 参数简单校验
    ;    if(user_id .isEmpty ()) { Utils::set(result, ec::USER_ID_EMPTY    ); }
    else if(auth_str.isEmpty ()) { Utils::set(result, ec::AUTH_METHOD_EMPTY); }
    else if(!m_client_account  ) { Utils::set(result, ec::ERROR_INIT       ); }
    // else { Utils::set(result, sc::SUCCESS); }

    signin_reply reply_data = TaoTieChat::SignInReply();
    reply_data->setResult(result);
    // reply_data->result().setErrorCode(result.errorCode());
    // reply_data->result().setMessage(result.message());

    // 失败时也要发送通知
    if(result.hasErrorCode() && result.errorCode() != ec::UNSET) {
        if(callback) {
            callback(params_signin, reply_data);
        }
        return false;
    }

    // ================================================================== //

    // TODO:在网络交互前应该对密码进行加密

    //
    auto opts = QGrpcCallOptions{}.setDeadlineTimeout(std::chrono::seconds(30));
    std::unique_ptr<QGrpcCallReply> reply_conn = m_client_account->SignIn(*params_signin, opts);
    const auto *rep_conn_ptr = reply_conn.get();  // 必须
    if (!rep_conn_ptr) {
        Utils::set(result, ec::ERROR_CONN);
        reply_data->setResult(result);
        if (callback) {
            callback(params_signin, reply_data);
        }
        return false;
    }
    // 捕获输出返回信息  // 单次模型需要使用 `Qt::SingleShotConnection` 以便结束后销毁
    // lamda 表达式不可使用引用，会造成生命周期问题
    connect(rep_conn_ptr, &QGrpcCallReply::finished, rep_conn_ptr,
        [reply_conn = std::move(reply_conn), params_signin, callback](const QGrpcStatus &status) mutable {
            signin_reply reply_data;
            if(status.isOk()) {
                reply_data = reply_conn->read<TaoTieChat::SignInReply>();
            }
            if(callback) {
                // 发送参数，返回参数
                // 1. 返回参数为 nullopt 时代表返回错误，仅提示失败即可？？
                // 2. 即使失败也要调用会回调，以便通知界面
                callback(params_signin, reply_data);
            }
        }, Qt::SingleShotConnection);
    return true;
}

// 精准匹配，不返回包含查询词的内容
bool NetworkClient::search_users (const search_users_params&params_search_users, const search_users_callback&callback) {
    using Result = TaoTieChat::Result;
    using ec = ::TaoTieChat::ErrorCodeGadget::ErrorCode;
    using sc = ::TaoTieChat::SuccessCodeGadget::SuccessCode;
    Result result;
    Utils::set(result, ec::UNSET);  // 默认设为错误

    // ================================================================== //

    // 有用的参数
    QString user_keyword = params_search_users->userId    ();
    QString user_filter  = params_search_users->userFilter();

    // 参数简单校验
    ;    if(user_keyword.isEmpty()) { Utils::set(result, ec::TARGET_USER_ID_EMPTY); }
    else if(!m_client_search) { Utils::set(result, ec::ERROR_INIT); }
    // else { Utils::set(result, sc::SUCCESS); }

    search_users_reply reply_data = TaoTieChat::SearchUsersReply();
    reply_data->setResult(result);

    // 失败时也要发送通知
    if(result.hasErrorCode() && result.errorCode() != ec::UNSET){
        if(callback) {
            callback(params_search_users, reply_data);
        }
        return false;
    }

    // ================================================================== //

    //
    auto opts = QGrpcCallOptions{}.setDeadlineTimeout(std::chrono::seconds(30));
    std::unique_ptr<QGrpcCallReply> reply_conn = m_client_search->SearchUsers(*params_search_users, opts);
    const auto *rep_conn_ptr = reply_conn.get();  // 必须
    if (!rep_conn_ptr) {
        Utils::set(result, ec::ERROR_CONN);
        reply_data->setResult(result);
        if (callback) {
            callback(params_search_users, reply_data);
        }
        return false;
    }
    // 捕获输出返回信息  // 单次模型需要使用 `Qt::SingleShotConnection` 以便结束后销毁
    // lamda 表达式不可使用引用，会造成生命周期问题
    connect(rep_conn_ptr, &QGrpcCallReply::finished, rep_conn_ptr,
        [reply_conn = std::move(reply_conn), params_search_users, callback](const QGrpcStatus &status) mutable {
            search_users_reply reply_data;
            if(status.isOk()) {
                reply_data = reply_conn->read<TaoTieChat::SearchUsersReply>();
            }
            if(callback) {
                // 发送参数，返回参数
                // 1. 返回参数为 nullopt 时代表返回错误，仅提示失败即可？？
                // 2. 即使失败也要调用会回调，以便通知界面
                callback(params_search_users, reply_data);
            }
        }, Qt::SingleShotConnection);
    return true;
}
