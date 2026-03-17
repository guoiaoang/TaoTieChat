#include "NetworkClient.h"

// 重载 QDebug 运算符
QDebug operator<<(QDebug debug, const QGrpcStatus &status) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    debug.nospace() << "QGrpcStatus("
                    << "code: "    << status.code()    << ", "
                    << "message: " << status.message()
                    << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const TaoTieChat::UserRegisterRequest &req) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    debug.nospace() << "TaoTieChat::UserRegisterRequest("
                    << "userId: "           << req.userId()           << ", "
                    << "password: "         << req.password()         << ", "
                    << "securityQuestion: " << req.securityQuestion() << ", "
                    << "securityAnswer: "   << req.securityAnswer()
                    << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const TaoTieChat::UserRegisterReply &rep) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    debug.nospace() << "TaoTieChat::UserRegisterReply("
                    << "UserRegisterRequest: " << rep.request()      << ", "
                    << "success: "             << rep.success()      << ", "
                    << "accountId: "           << rep.accountId()    << ", "
                    << "errorMessage: "        << rep.errorMessage()
                    << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const TaoTieChat::UserLoginRequest &req) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    debug.nospace() << "TaoTieChat::UserLoginRequest("
                    << "userId: "   << req.userId()   << ", "
                    << "password: " << req.password()
                    << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const TaoTieChat::UserLoginReply &rep) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    debug.nospace() << "TaoTieChat::UserLoginReply("
                    << "UserLoginRequest: " << rep.request()      << ", "
                    << "success: "          << rep.success()      << ", "
                    << "accountId: "        << rep.accountId()    << ", "
                    << "token: "            << rep.token()        << ", "
                    << "errorMessage: "     << rep.errorMessage()
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

bool NetworkClient::user_register(user_register_params params_register, user_register_callback callback) {
    QString&user_id  = params_register->user_id          ;
    QString&password = params_register->password         ;
    QString&confirm  = params_register->confirm          ;
    QString&question = params_register->security_question;
    QString&answer   = params_register->security_answer  ;

    // 只有 0 才表示成功 // 具体的错误码暂不列举
    int     error_code = 11;
    QString error_message;

    do{
        if(user_id.isEmpty() || password.isEmpty() || confirm.isEmpty() || question.isEmpty() || answer.isEmpty()) {
            // 请输入以上所有信息
            error_code = 12;
            error_message = tr("Please enter all of the above information.");
            break;
        }
        if(password != confirm) {
            // 密码与确认密码不一致
            error_code = 13;
            error_message = tr("The password and confirmation password do not match.");
            break;
        }
        if (!m_client_account) {
            error_code = 14;
            error_message = tr("service not attached.");
            break;
        }

        // 继续而不直接返回
        error_code = 0;
    } while(false);

    if(error_code != 0) {
        if(callback) {
            callback(error_code, error_message, params_register, std::nullopt);
        }
        return false;
    }

    // TODO:在网络交互前应该对密码进行加密
    TaoTieChat::UserRegisterRequest req;
    req.setUserId          (user_id );
    req.setPassword        (password);
    req.setSecurityQuestion(question);
    req.setSecurityAnswer  (answer  );
    std::unique_ptr<QGrpcCallReply> rep = m_client_account->UserRegister(req, QGrpcCallOptions{}.setDeadlineTimeout(std::chrono::seconds(30)));
    const auto *rep_ptr = rep.get();
    // 捕获输出返回信息  // 单次模型需要使用 `Qt::SingleShotConnection` 以便结束后销毁
    connect(rep_ptr, &QGrpcCallReply::finished, rep_ptr,
        [this,  reply = std::move(rep), params_register, callback](const QGrpcStatus &status) {
            int     error_code = 21;
            QString error_message;
            user_register_reply rep = std::nullopt;

            do {
                if(!status.isOk()) {
                    error_code = 22;
                    error_message = "rep status not ok.";
                    qDebug().noquote().nospace() << "rep status not ok. status : " << status;
                    break;
                }

                rep = reply->read<TaoTieChat::UserRegisterReply>();
                if(!rep) {
                    error_code = 23;
                    error_message = "rep deserialization failed.";
                    qDebug().noquote().nospace() << "rep deserialization failed. status : " << status;
                    break;
                }

                error_code = 0;
                qDebug().noquote().nospace() << "rep : " << rep;
            } while(false);

            if(callback) {
                callback(error_code, error_message, params_register, rep);
            }
        }, Qt::SingleShotConnection);
    return true;
}

bool NetworkClient::user_login(user_login_params params_login, user_login_callback callback) {
    QString&user_id  = params_login->user_id ;
    QString&password = params_login->password;

    // 只有 0 才表示成功 // 具体的错误码暂不列举
    int     error_code = 11;
    QString error_message;

    do{
        if(user_id.isEmpty() || password.isEmpty()) {
            // 请输入以上所有信息
            error_code = 12;
            error_message = tr("Please enter all of the above information.");
            break;
        }
        if (!m_client_account) {
            error_code = 14;
            error_message = tr("service not attached.");
            break;
        }

        // 继续而不直接返回
        error_code = 0;
    } while(false);

    if(error_code != 0) {
        if(callback) {
            callback(error_code, error_message, params_login, std::nullopt);
        }
        return false;
    }

    // TODO:在网络交互前应该对密码进行加密
    TaoTieChat::UserLoginRequest req;
    req.setUserId(user_id);
    req.setPassword(password);
    std::unique_ptr<QGrpcCallReply> rep = m_client_account->UserLogin(req, QGrpcCallOptions{}.setDeadlineTimeout(std::chrono::seconds(30)));
    const auto *rep_ptr = rep.get();
    // 捕获输出返回信息  // 单次模型需要使用 `Qt::SingleShotConnection` 以便结束后销毁
    connect(rep_ptr, &QGrpcCallReply::finished, rep_ptr,
            [this,  reply = std::move(rep), params_login, callback](const QGrpcStatus &status) {
                int     error_code = 21;
                QString error_message;
                user_login_reply rep = std::nullopt;

                do {
                    if(!status.isOk()) {
                        error_code = 22;
                        error_message = "rep status not ok.";
                        qDebug().noquote().nospace() << "rep status not ok. status : " << status;
                        break;
                    }

                    rep = reply->read<TaoTieChat::UserLoginReply>();
                    if(!rep) {
                        error_code = 23;
                        error_message = "rep deserialization failed.";
                        qDebug().noquote().nospace() << "rep deserialization failed. status : " << status;
                        break;
                    }

                    error_code = 0;
                    qDebug().noquote().nospace() << "rep : " << rep;
                } while(false);

                if(callback) {
                    callback(error_code, error_message, params_login, rep);
                }
            }, Qt::SingleShotConnection);
    return true;
}
