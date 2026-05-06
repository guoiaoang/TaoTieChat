#pragma once

#include "Headers.h"
#include "Utils.h"

// 重载 QDebug 运算符
QDebug operator<<(QDebug debug, const TaoTieChat::Result &result);
// register
QDebug operator<<(QDebug debug, const TaoTieChat::SignUpRequest &req);
QDebug operator<<(QDebug debug, const TaoTieChat::SignUpReply   &rep);
// login
QDebug operator<<(QDebug debug, const TaoTieChat::SignInRequest    &req);
QDebug operator<<(QDebug debug, const TaoTieChat::SignInReply      &rep);
// search users
QDebug operator<<(QDebug debug, const TaoTieChat::Contact           &contact);
QDebug operator<<(QDebug debug, const TaoTieChat::SearchUsersRequest&req    );
QDebug operator<<(QDebug debug, const TaoTieChat::SearchUsersReply  &rep    );

// auto callback = std::function<void(int error_code, const QString&error_message, ...)>;
// using ResultInfo             = std::shared_ptr<TaoTieChat::SignUpReply>;
// user register
using signup_params   = std::shared_ptr<TaoTieChat::SignUpRequest>;
using signup_reply    = std::optional  <TaoTieChat::SignUpReply  >;
using signup_callback = std::function  <void(const signup_params&, const signup_reply&)>;
// user login
using signin_params   = std::shared_ptr<TaoTieChat::SignInRequest>;
using signin_reply    = std::optional  <TaoTieChat::SignInReply  >;
using signin_callback = std::function  <void(const signin_params&, const signin_reply&)>;
// search users
using search_users_params   = std::shared_ptr<TaoTieChat::SearchUsersRequest>;
using search_users_reply    = std::optional  <TaoTieChat::SearchUsersReply  >;
using search_users_callback = std::function  <void(const search_users_params&, const search_users_reply&)>;

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
    bool signup(const signup_params& params_signup, const signup_callback& callback);
    bool signin(const signin_params& params_signin, const signin_callback& callback);
    // 精准匹配，不返回包含查询词的内容
    bool search_users (const search_users_params&params_search_users, const search_users_callback&callback);
    bool search_groups(const QString&group) { return false; }

private:
    bool m_inited = false;
    QUrl m_url_server;
    std::unique_ptr<TaoTieChat::AccountService::Client> m_client_account;
    std::unique_ptr<TaoTieChat::SearchService ::Client> m_client_search ;
    std::shared_ptr<QGrpcHttp2Channel> m_channel;
};
