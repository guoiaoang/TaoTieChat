#pragma once

#include "Headers.h"
#include "Utils.h"
#include <QGrpcChannelOptions>
#include <QGrpcHttp2Channel>
#include <QtGrpc/QGrpcStatus>
#include <QDebug>

// 重载 QDebug 运算符
QDebug operator<<(QDebug debug, const TaoTieChat::Result &result);
// register
QDebug operator<<(QDebug debug, const TaoTieChat::SignUpRequest &req);
QDebug operator<<(QDebug debug, const TaoTieChat::SignUpReply   &rep);
// login
QDebug operator<<(QDebug debug, const TaoTieChat::SignInRequest    &req);
QDebug operator<<(QDebug debug, const TaoTieChat::SignInReply      &rep);

// auto callback = std::function<void(int error_code, const QString&error_message, ...)>;
// using ResultInfo             = std::shared_ptr<TaoTieChat::SignUpReply>;
// user register
using signup_params   = std::shared_ptr<TaoTieChat::SignUpRequest>;
using signup_reply    = std::optional  <TaoTieChat::SignUpReply  >;
using signup_callback = std::function  <void(signup_params&, signup_reply&)>;
// user login
using signin_params   = std::shared_ptr<TaoTieChat::SignInRequest>;
using signin_reply    = std::optional  <TaoTieChat::SignInReply  >;
using signin_callback = std::function  <void(signin_params&, signin_reply&)>;
// search users
using search_users_params   = std::shared_ptr<TaoTieChat::SearchUsersRequest>;
using search_users_reply    = std::optional  <TaoTieChat::SearchUsersReply  >;
using search_users_callback = std::function  <void(search_users_params &, search_users_reply&)>;

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
    bool signup(signup_params& params_signup, signup_callback callback = nullptr);
    bool signin(signin_params& params_signin, signin_callback callback = nullptr);
    //
    bool search_users (const QString&user ) { return false; }
    bool search_groups(const QString&group) { return false; }

private:
    bool m_inited = false;
    QUrl m_url_server;
    std::unique_ptr<TaoTieChat::AccountService::Client> m_client_account;
    std::shared_ptr<QGrpcHttp2Channel> m_channel;
};
