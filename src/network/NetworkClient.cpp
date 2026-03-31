#include "NetworkClient.h"

QDebug operator<<(QDebug debug, const TaoTieChat::Result &result) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    if(result.hasSuccessCode()) {
        debug.nospace() << "Result("
                        << "success code:" << result.successCode() << ", "
                        << "message:"      << result.message()
                        << ")";
    }
    else if(result.hasSuccessCode()) {
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

QDebug operator<<(QDebug debug, const TaoTieChat::SignUpRequest &req) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    debug.nospace() << "SignUpRequest("
                    << "userId:"           << req.userId()           << ", "
                    << "password:"         << req.password()         << ", "
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

QDebug operator<<(QDebug debug, const TaoTieChat::SignInRequest &req) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    debug.nospace() << "SignInRequest("
                    << "userId:"   << req.userId()   << ", "
                    << "password:" << req.password()
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

    m_inited = true;

    return true;
}

// 成功失败都要返回，失败时要返回错误原因
bool NetworkClient::signup(signup_params& params_signup, signup_callback callback) {
    using Result = TaoTieChat::Result;
    using ec = ::TaoTieChat::ErrorCodeGadget::ErrorCode;
    using sc = ::TaoTieChat::SuccessCodeGadget::SuccessCode;
    Result result;

    // ================================================================== //

    // 有用的参数
    QString user_id  = params_signup->userId          ();
    QString password = params_signup->password        ();
    QString confirm  = params_signup->confirm         ();
    QString question = params_signup->securityQuestion();
    QString answer   = params_signup->securityAnswer  ();

    // 默认设为错误
    Utils::set(result, ec::UNSET);

    // 参数简单校验
    ;    if(user_id  .isEmpty()) { Utils::set(result, ec::SIGN_UP_USER_ID_EMPTY          ); }
    else if(password .isEmpty()) { Utils::set(result, ec::SIGN_UP_PASSWORD_EMPTY         ); }
    else if(confirm  .isEmpty()) { Utils::set(result, ec::SIGN_UP_CONFIRM_PASSWORD_EMPTY ); }
    else if(question .isEmpty()) { Utils::set(result, ec::SIGN_UP_SECURITY_QUESTION_EMPTY); }
    else if(answer   .isEmpty()) { Utils::set(result, ec::SIGN_UP_SECURITY_ANSWER_EMPTY  ); }
    else if(password != confirm) { Utils::set(result, ec::SIGN_UP_DIFF_CONFIRM_PASSWORD  ); }
    else if(!m_client_account  ) { Utils::set(result, ec::ERROR_INIT                     ); }
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

    // TODO:在网络交互前应该对密码进行加密

    // 构造参数
    TaoTieChat::SignUpRequest req;
    req.setUserId          (user_id );
    req.setPassword        (password);
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
bool NetworkClient::signin(signin_params& params_signin, signin_callback callback) {
    using Result = TaoTieChat::Result;
    using ec = ::TaoTieChat::ErrorCodeGadget::ErrorCode;
    using sc = ::TaoTieChat::SuccessCodeGadget::SuccessCode;
    Result      result;
    // user_login_reply reply_data = TaoTieChat::SignInReply();
    // reply_data->setResult(Result{});

    // ================================================================== //

    // 有用的参数
    QString user_id  = params_signin->userId  ();
    QString password = params_signin->hasPassword() == false ? QString() : params_signin->password();
    QString token    = params_signin->hasToken   () == false ? QString() : params_signin->token   ();
    QString auth_str = password + token;

    // 默认设为错误
    Utils::set(result, ec::UNSET);

    // 参数简单校验
    ;    if(user_id .isEmpty ()) { Utils::set(result, ec::SIGN_IN_USER_ID_EMPTY ); }
    else if(auth_str.isEmpty ()) { Utils::set(result, ec::SIGN_IN_INVALID_LOGIN_METHOD); }
    else if(!m_client_account  ) { Utils::set(result, ec::ERROR_INIT            ); }
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

    // TODO:在网络交互前应该对密码进行加密

    // 构造参数
    TaoTieChat::SignInRequest req;
    req.setUserId  (user_id );
    req.setPassword(password);

    //
    auto opts = QGrpcCallOptions{}.setDeadlineTimeout(std::chrono::seconds(30));
    std::unique_ptr<QGrpcCallReply> reply_conn = m_client_account->SignIn(req, opts);
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
