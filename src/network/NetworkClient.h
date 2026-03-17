#pragma once

#include "Headers.h"
#include "Utils.h"
#include <QGrpcChannelOptions>
#include <QGrpcHttp2Channel>
#include <QtGrpc/QGrpcStatus>
#include <QDebug>


struct UserRegisterParameters {
    QString user_id;
    QString password;
    QString confirm;
    QString security_question;
    QString security_answer;
};

struct UserLoginParameters {
    QString user_id;
    QString password;
};

// 重载 QDebug 运算符
inline QDebug operator<<(QDebug debug, const QGrpcStatus &status);
// register
inline QDebug operator<<(QDebug debug, const TaoTieChat::UserRegisterRequest &req);
inline QDebug operator<<(QDebug debug, const TaoTieChat::UserRegisterReply   &rep);
// login
inline QDebug operator<<(QDebug debug, const TaoTieChat::UserLoginRequest    &req);
inline QDebug operator<<(QDebug debug, const TaoTieChat::UserLoginReply      &rep);

// auto callback = std::function<void(int error_code, const QString&error_message, ...)>;
// register
using user_register_params   = QSharedPointer<UserRegisterParameters>;
using user_register_reply    = std::optional<TaoTieChat::UserRegisterReply>;
using user_register_callback = std::function<void(int, const QString&, user_register_params, user_register_reply)>;
// login
using user_login_params   = QSharedPointer<UserLoginParameters>;
using user_login_reply    = std::optional<TaoTieChat::UserLoginReply>;
using user_login_callback = std::function<void(int, const QString&, user_login_params, user_login_reply)>;

class NetworkClient : public QObject{
    Q_OBJECT
private:
    NetworkClient() = default;
    NetworkClient(const NetworkClient&) = delete;
public:
    static NetworkClient& instance() {
        static NetworkClient s_network_client;
        return s_network_client;
    }

    void set_server_url(const QUrl&url) { m_url_server = url; }
    QUrl server_url() const { return m_url_server; }
    bool init();

public:
    bool user_register(user_register_params params_register, user_register_callback callback = nullptr);
    bool user_login   (user_login_params    params_login   , user_login_callback    callback = nullptr);

private:
    bool m_inited = false;
    QUrl m_url_server;
    std::unique_ptr<TaoTieChat::AccountService::Client> m_client_account;
    std::shared_ptr<QGrpcHttp2Channel> m_channel;
};
